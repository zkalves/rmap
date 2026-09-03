#include <QDirIterator>
#include "CodeGenerator.hpp"

void CodeGenerator::registerHelpers(Environment &env) {
    // Helper: {{ upper(str) }}
    env.add_callback("upper", 1, [](Arguments& args) {
        std::string str = args.at(0)->is_string() ? args.at(0)->get<std::string>() : args.at(0)->dump();
        if (str.size() >= 2 && str.front() == '"' && str.back() == '"') {
            str = str.substr(1, str.size() - 2);
        }
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        return str;
    });

    // Helper: {{ lower(str) }}
    env.add_callback("lower", 1, [](Arguments& args) {
        std::string str = args.at(0)->is_string() ? args.at(0)->get<std::string>() : args.at(0)->dump();
        if (str.size() >= 2 && str.front() == '"' && str.back() == '"') {
            str = str.substr(1, str.size() - 2);
        }
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    });

    // Helper: {{ to_hex(val, width) }}
    env.add_callback("to_hex", 2, [](Arguments& args) {
        uint64_t val = 0;
        if (args.at(0)->is_number()) {
            val = args.at(0)->get<uint64_t>();
        } else if (args.at(0)->is_string()) {
            try { val = std::stoull(args.at(0)->get<std::string>(), nullptr, 0); } catch (...) { val = 0; }
        }
        int width = args.at(1)->is_number() ? args.at(1)->get<int>() : 0;
        std::stringstream ss;
        ss << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(width) << val;
        return ss.str();
    });

    // Helper: {{ to_dec(val) }}
    env.add_callback("to_dec", 1, [](Arguments& args) {
        uint64_t val = 0;
        if (args.at(0)->is_number()) {
            val = args.at(0)->get<uint64_t>();
        } else if (args.at(0)->is_string()) {
            try { val = std::stoull(args.at(0)->get<std::string>(), nullptr, 0); } catch (...) { val = 0; }
        }
        return std::to_string(val);
    });

    // Helper: {{ bitmask(width, lsb) }} -> Generates ((0x1ULL << width) - 1) << lsb
    env.add_callback("bitmask", 2, [](Arguments& args) {
        uint64_t width = args.at(0)->is_number() ? args.at(0)->get<uint64_t>() : 0;
        uint64_t lsb   = args.at(1)->is_number() ? args.at(1)->get<uint64_t>() : 0;
        uint64_t mask  = 0;
        if (width >= 64) {
            mask = ~0ULL;
        } else if (width > 0) {
            mask = ((1ULL << width) - 1ULL);
        }
        if (lsb < 64) {
            mask <<= lsb;
        } else {
            mask = 0;
        }
        std::stringstream ss;
        ss << "0x" << std::hex << std::uppercase << mask;
        return ss.str();
    });

    // Helper: {{ pad_zero(val, width) }} -> Formats numbers as zero-padded strings
    env.add_callback("pad_zero", 2, [](Arguments& args) {
        uint64_t val = 0;
        if (args.at(0)->is_number()) {
            val = args.at(0)->get<uint64_t>();
        } else if (args.at(0)->is_string()) {
            try { val = std::stoull(args.at(0)->get<std::string>(), nullptr, 0); } catch (...) { val = 0; }
        }
        int width = args.at(1)->is_number() ? args.at(1)->get<int>() : 0;
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(width) << val;
        return ss.str();
    });

    // Helper: {{ camel_case(str) }} -> e.g. "CTRL_STATUS" -> "ctrlStatus"
    env.add_callback("camel_case", 1, [](Arguments& args) {
        std::string str = args.at(0)->is_string() ? args.at(0)->get<std::string>() : args.at(0)->dump();
        if (str.size() >= 2 && str.front() == '"' && str.back() == '"') str = str.substr(1, str.size() - 2);
        std::string res;
        bool capNext = false;
        for (size_t i = 0; i < str.size(); ++i) {
            char c = str[i];
            if (c == '_' || c == '-' || c == ' ') {
                capNext = true;
            } else if (res.empty()) {
                res += std::tolower(c);
            } else if (capNext) {
                res += std::toupper(c);
                capNext = false;
            } else {
                res += std::tolower(c);
            }
        }
        return res;
    });

    // Helper: {{ pascal_case(str) }} -> e.g. "ctrl_status" -> "CtrlStatus"
    env.add_callback("pascal_case", 1, [](Arguments& args) {
        std::string str = args.at(0)->is_string() ? args.at(0)->get<std::string>() : args.at(0)->dump();
        if (str.size() >= 2 && str.front() == '"' && str.back() == '"') str = str.substr(1, str.size() - 2);
        std::string res;
        bool capNext = true;
        for (size_t i = 0; i < str.size(); ++i) {
            char c = str[i];
            if (c == '_' || c == '-' || c == ' ') {
                capNext = true;
            } else if (capNext) {
                res += std::toupper(c);
                capNext = false;
            } else {
                res += std::tolower(c);
            }
        }
        return res;
    });

    // Helper: {{ snake_case(str) }} -> e.g. "CtrlStatus" -> "ctrl_status", "SPI_SYS" -> "spi_sys"
    env.add_callback("snake_case", 1, [](Arguments& args) {
        std::string str = args.at(0)->is_string() ? args.at(0)->get<std::string>() : args.at(0)->dump();
        if (str.size() >= 2 && str.front() == '"' && str.back() == '"') str = str.substr(1, str.size() - 2);
        std::string res;
        for (size_t i = 0; i < str.size(); ++i) {
            char c = str[i];
            if (std::isupper(c)) {
                bool prevIsLower = (i > 0 && std::islower(str[i - 1]));
                bool nextIsLower = (i + 1 < str.size() && std::islower(str[i + 1]) && i > 0 && str[i - 1] != '_');
                if ((prevIsLower || nextIsLower) && !res.empty() && res.back() != '_') {
                    res += '_';
                }
                res += std::tolower(c);
            } else if (c == '-' || c == ' ') {
                if (!res.empty() && res.back() != '_') res += '_';
            } else {
                res += c;
            }
        }
        return res;
    });

    // Helper: {{ c_type(width) }} -> uint8_t, uint16_t, uint32_t, uint64_t
    env.add_callback("c_type", 1, [](Arguments& args) {
        uint64_t w = args.at(0)->is_number() ? args.at(0)->get<uint64_t>() : 32;
        if (w <= 8) return std::string("uint8_t");
        if (w <= 16) return std::string("uint16_t");
        if (w <= 32) return std::string("uint32_t");
        return std::string("uint64_t");
    });

    // Helper: {{ msb(width, lsb) }} or {{ msb(lsb, width) }} -> lsb + width - 1
    env.add_callback("msb", 2, [](Arguments& args) {
        uint64_t arg0 = args.at(0)->is_number() ? args.at(0)->get<uint64_t>() : 0;
        uint64_t arg1 = args.at(1)->is_number() ? args.at(1)->get<uint64_t>() : 1;
        uint64_t m = (arg0 + arg1 > 0) ? (arg0 + arg1 - 1) : 0;
        return std::to_string(m);
    });
}

std::string CodeGenerator::resolveTemplatePath(
    const std::string &tmpl_path,
    const std::string &default_folder,
    const std::string &base_dir)
{
    if (tmpl_path.empty()) return "";

    std::string expTmpl = PathUtils::expandEnvVars(tmpl_path);
    std::string expDefault = PathUtils::expandEnvVars(default_folder);
    std::string expBase = PathUtils::expandEnvVars(base_dir);

    return PathUtils::resolvePath(expTmpl, expBase, expDefault);
}

std::string CodeGenerator::resolveOutputPath(
    const std::string &tmpl_path,
    const std::string &out_path,
    const std::string &default_out_folder,
    const std::string &base_dir,
    const json &context)
{
    QString expTmpl = QString::fromStdString(PathUtils::expandEnvVars(tmpl_path)).trimmed();
    expTmpl = PathUtils::normalizeSeparators(expTmpl);

    QString expDefaultTmpl = PathUtils::expandEnvVars(PathUtils::defaultTemplatesDir()).trimmed();
    expDefaultTmpl = PathUtils::normalizeSeparators(expDefaultTmpl);

    // Extract subpath relative to templates folder if applicable
    QString relSubPath;
    if (expTmpl.startsWith(expDefaultTmpl + "/", Qt::CaseInsensitive)) {
        relSubPath = expTmpl.mid(expDefaultTmpl.length() + 1);
    } else if (expTmpl.startsWith("./" + expDefaultTmpl + "/", Qt::CaseInsensitive)) {
        relSubPath = expTmpl.mid(expDefaultTmpl.length() + 3);
    } else if (expTmpl.startsWith("templates/", Qt::CaseInsensitive)) {
        relSubPath = expTmpl.mid(10);
    } else if (expTmpl.startsWith("./templates/", Qt::CaseInsensitive)) {
        relSubPath = expTmpl.mid(12);
    } else {
        QFileInfo tmplInfo(expTmpl);
        QString dirPath = tmplInfo.path();
        if (dirPath != "." && !dirPath.isEmpty() && !expTmpl.startsWith("/")) {
            relSubPath = expTmpl;
        } else {
            relSubPath = tmplInfo.fileName();
        }
    }

    // Determine default base filename by stripping .inja / .tmpl suffix
    if (relSubPath.endsWith(".inja", Qt::CaseInsensitive)) {
        relSubPath.chop(5);
    } else if (relSubPath.endsWith(".tmpl", Qt::CaseInsensitive)) {
        relSubPath.chop(5);
    }

    // Extract category, base filename, template name, and extension
    QString category = "";
    QString baseFileName = relSubPath;
    int slashIdx = relSubPath.lastIndexOf('/');
    if (slashIdx >= 0) {
        category = relSubPath.left(slashIdx);
        baseFileName = relSubPath.mid(slashIdx + 1);
    }
    QString fileExt = "";
    QString tmplName = baseFileName;
    int dotIdx = baseFileName.lastIndexOf('.');
    if (dotIdx >= 0) {
        tmplName = baseFileName.left(dotIdx);
        fileExt = baseFileName.mid(dotIdx + 1);
    }

    QString expDefaultOut = QString::fromStdString(PathUtils::expandEnvVars(default_out_folder)).trimmed();
    if (expDefaultOut.isEmpty()) {
        expDefaultOut = PathUtils::defaultOutputDir();
    }
    expDefaultOut = PathUtils::normalizeSeparators(expDefaultOut);

    QString expOut = QString::fromStdString(PathUtils::expandEnvVars(out_path)).trimmed();
    expOut = PathUtils::normalizeSeparators(expOut);

    // Dynamic variable expansion
    if (!expOut.isEmpty()) {
        std::string rawBlockName;
        if (context.contains("blocks") && context["blocks"].is_array() && !context["blocks"].empty()) {
            rawBlockName = context["blocks"][0].value("name", "");
        }
        if (rawBlockName.empty()) {
            rawBlockName = context.value("name", "");
        }
        QString blockName = QString::fromStdString(rawBlockName).toLower();

        std::string rawProj = context.value("project_name", "");
        if (rawProj.empty()) rawProj = rawBlockName;
        QString projName = QString::fromStdString(rawProj).toLower();

        expOut.replace("{output_folder}", expDefaultOut, Qt::CaseInsensitive);
        expOut.replace("{output_dir}", expDefaultOut, Qt::CaseInsensitive);
        expOut.replace("{out_dir}", expDefaultOut, Qt::CaseInsensitive);
        expOut.replace("{out}", expDefaultOut, Qt::CaseInsensitive);

        expOut.replace("{category}", category, Qt::CaseInsensitive);
        expOut.replace("{cat}", category, Qt::CaseInsensitive);

        expOut.replace("{block_name}", blockName, Qt::CaseInsensitive);
        expOut.replace("{block}", blockName, Qt::CaseInsensitive);
        expOut.replace("{name}", blockName, Qt::CaseInsensitive);

        expOut.replace("{project_name}", projName, Qt::CaseInsensitive);
        expOut.replace("{project}", projName, Qt::CaseInsensitive);

        expOut.replace("{file_extension}", fileExt, Qt::CaseInsensitive);
        expOut.replace("{ext}", fileExt, Qt::CaseInsensitive);

        expOut.replace("{template_name}", tmplName, Qt::CaseInsensitive);
        expOut.replace("{filename}", tmplName, Qt::CaseInsensitive);

        expOut = PathUtils::normalizeSeparators(expOut);
    }

    QString targetDirOrFile = expOut.isEmpty() ? expDefaultOut : expOut;

    // Resolve directory/file against base_dir if relative
    QString resolved = PathUtils::resolvePath(targetDirOrFile, QString::fromStdString(PathUtils::expandEnvVars(base_dir)));
    resolved = PathUtils::normalizeSeparators(resolved);

    QFileInfo outInfo(resolved);
    bool isKnownFileWithoutExt = outInfo.fileName().compare("makefile", Qt::CaseInsensitive) == 0;
    // If output is explicitly a directory, ends with a slash separator, or has no file extension (folder path)
    if (!isKnownFileWithoutExt && (targetDirOrFile.endsWith('/') || targetDirOrFile.endsWith('\\') || (outInfo.exists() && outInfo.isDir()) || !outInfo.fileName().contains('.'))) {
        QString finalSubPath = relSubPath;
        if (!category.isEmpty()) {
            QString normTarget = PathUtils::normalizeSeparators(targetDirOrFile);
            if (normTarget.endsWith("/" + category, Qt::CaseInsensitive) || normTarget.endsWith("/" + category + "/", Qt::CaseInsensitive) || normTarget == category) {
                finalSubPath = baseFileName;
            }
        }
        return PathUtils::normalizeSeparators(QDir(resolved).filePath(finalSubPath)).toStdString();
    }

    return PathUtils::normalizeSeparators(resolved).toStdString();
}

GenerationReport CodeGenerator::generate(
    const json &json_data,
    const std::string &default_template_folder,
    const std::string &default_output_folder,
    const std::vector<TemplateMapping> &mappings,
    const std::string &base_dir)
{
    GenerationReport report;

    // If no specific template mappings are defined, scan the default template folder
    if (mappings.empty()) {
        return parseDirectory(json_data, default_template_folder, default_output_folder, base_dir);
    }

    // Ensure memory gap padding is present if raw json data didn't go through model extraction
    json preparedJson = json_data;
    if (!preparedJson.contains("reg_width_bytes")) {
        uint32_t width = preparedJson.value("reg_width", 32U);
        preparedJson["reg_width_bytes"] = width > 0 ? (width / 8) : 4;
    }
    if (preparedJson.contains("blocks") && preparedJson["blocks"].is_array()) {
        uint64_t regBytes = preparedJson.value("reg_width_bytes", 4ULL);
        if (regBytes == 0) regBytes = 4;
        for (auto &blk : preparedJson["blocks"]) {
            if (blk.contains("registers") && blk["registers"].is_array()) {
                uint64_t currentOffset = 0;
                for (auto &r : blk["registers"]) {
                    if (!r.contains("pad_words_before")) {
                        uint64_t regOffset = r.value("offset_lsb", 0ULL);
                        uint64_t padBytes = (regOffset > currentOffset) ? (regOffset - currentOffset) : 0;
                        uint64_t padWords = (regBytes > 0) ? (padBytes / regBytes) : 0;
                        r["pad_bytes_before"] = padBytes;
                        r["pad_words_before"] = padWords;
                    }
                    currentOffset = r.value("offset_lsb", 0ULL) + regBytes;
                }
            }
        }
    }

    // Pre-scan mappings to identify output paths for RTL, UVM, and SIM directory
    QString resolvedRtlOut;
    QString resolvedUvmOut;
    QString resolvedSimDir;

    for (const auto &m : mappings) {
        QString tmplPath = PathUtils::normalizeSeparators(QString::fromStdString(m.template_file));
        std::string resolvedOut = resolveOutputPath(m.template_file, m.output_file, default_output_folder, base_dir, preparedJson);
        QString normOut = PathUtils::normalizeSeparators(QString::fromStdString(resolvedOut));
        QFileInfo outFi(normOut);

        if (tmplPath.contains("/rtl/", Qt::CaseInsensitive) && tmplPath.endsWith(".sv.inja", Qt::CaseInsensitive) && !tmplPath.contains("_tb", Qt::CaseInsensitive)) {
            resolvedRtlOut = normOut;
        } else if (tmplPath.contains("/uvm/", Qt::CaseInsensitive) && tmplPath.endsWith(".sv.inja", Qt::CaseInsensitive) && !tmplPath.contains("_tb", Qt::CaseInsensitive)) {
            resolvedUvmOut = normOut;
        }

        if (tmplPath.contains("/sim/", Qt::CaseInsensitive) || normOut.contains("/sim/", Qt::CaseInsensitive)) {
            resolvedSimDir = outFi.isDir() ? normOut : outFi.dir().absolutePath();
        }
    }

    if (resolvedSimDir.isEmpty()) {
        resolvedSimDir = PathUtils::normalizeSeparators(QDir(QString::fromStdString(default_output_folder.empty() ? PathUtils::DEFAULT_OUTPUT_DIR : default_output_folder)).filePath("sim"));
    }

    QString relRtlPath = "../rtl/reg_map.sv";
    QString relRtlDir  = "../rtl";
    if (!resolvedRtlOut.isEmpty() && !resolvedSimDir.isEmpty()) {
        QDir simD(resolvedSimDir);
        relRtlPath = PathUtils::normalizeSeparators(simD.relativeFilePath(resolvedRtlOut));
        relRtlDir  = PathUtils::normalizeSeparators(simD.relativeFilePath(QFileInfo(resolvedRtlOut).dir().absolutePath()));
    }

    QString relUvmPath = "../uvm/reg_model.sv";
    QString relUvmDir  = "../uvm";
    if (!resolvedUvmOut.isEmpty() && !resolvedSimDir.isEmpty()) {
        QDir simD(resolvedSimDir);
        relUvmPath = PathUtils::normalizeSeparators(simD.relativeFilePath(resolvedUvmOut));
        relUvmDir  = PathUtils::normalizeSeparators(simD.relativeFilePath(QFileInfo(resolvedUvmOut).dir().absolutePath()));
    }

    preparedJson["sim_rel_rtl_path"] = relRtlPath.toStdString();
    preparedJson["sim_rel_rtl_dir"]  = relRtlDir.toStdString();
    preparedJson["sim_rel_uvm_path"] = relUvmPath.toStdString();
    preparedJson["sim_rel_uvm_dir"]  = relUvmDir.toStdString();

    // Process each configured template mapping
    for (const auto &mapping : mappings) {
        std::string resolvedTmpl = resolveTemplatePath(mapping.template_file, default_template_folder, base_dir);
        std::string resolvedOut  = resolveOutputPath(mapping.template_file, mapping.output_file, default_output_folder, base_dir, preparedJson);

        QFileInfo tmplInfo(QString::fromStdString(resolvedTmpl));
        if (!tmplInfo.exists() || !tmplInfo.isFile()) {
            std::string err = "Template file does not exist: " + mapping.template_file;
            qWarning() << "[CodeGenerator]" << err.c_str();
            report.errors.push_back({mapping.template_file, err});
            continue;
        }

        // Ensure output parent directory exists
        QFileInfo outInfo(QString::fromStdString(resolvedOut));
        QDir outDir = outInfo.dir();
        if (!outDir.exists()) {
            outDir.mkpath(".");
        }

        // Use the template file's directory as Inja base so relative includes inside template resolve correctly
        std::string tmplDir = tmplInfo.absolutePath().toStdString();
        std::string tmplFileName = tmplInfo.fileName().toStdString();

        try {
            Environment env(tmplDir + "/", "");
            env.set_line_statement("$$##$$");
            env.set_trim_blocks(true);
            env.set_lstrip_blocks(true);
            registerHelpers(env);

            Template temp = env.parse_template(tmplFileName);
            env.write(temp, preparedJson, resolvedOut);

            qDebug() << "[CodeGenerator] Successfully generated:" << resolvedOut.c_str()
                     << "from template:" << resolvedTmpl.c_str();
            report.success_files.push_back(resolvedOut);
        } catch (const std::exception &e) {
            std::string err = std::string("Template rendering error: ") + e.what();
            qWarning() << "[CodeGenerator]" << err.c_str() << "for template:" << resolvedTmpl.c_str();
            report.errors.push_back({mapping.template_file, err});
        }
    }

    return report;
}

GenerationReport CodeGenerator::parseDirectory(
    const json &json_data,
    const std::string &template_folder,
    const std::string &output_folder,
    const std::string &base_dir)
{
    GenerationReport report;
    std::string rawTmplFolder = template_folder.empty() ? PathUtils::DEFAULT_TEMPLATES_DIR : template_folder;
    std::string resolvedTmplFolder = PathUtils::resolvePath(rawTmplFolder, base_dir);
    std::string outFolder  = output_folder.empty() ? PathUtils::DEFAULT_OUTPUT_DIR : output_folder;

    QDir tmplDir(QString::fromStdString(resolvedTmplFolder));
    if (!tmplDir.exists()) {
        report.errors.push_back({resolvedTmplFolder, "Template directory does not exist: " + resolvedTmplFolder});
        return report;
    }

    QStringList filters;
    filters << "*.inja" << "*.tmpl" << "*.txt" << "*.sv" << "*.h" << "*.cpp" << "*.rs" << "*.py" << "*.html" << "*.md" << "*.rdl" << "*.xml" << "*.json";

    std::vector<TemplateMapping> mappings;
    QDirIterator it(QString::fromStdString(resolvedTmplFolder), filters, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        QFileInfo fi = it.fileInfo();
        QString relPath = tmplDir.relativeFilePath(fi.absoluteFilePath());
        QString relOutPath = relPath;
        if (relOutPath.endsWith(".inja", Qt::CaseInsensitive)) {
            relOutPath.chop(5);
        } else if (relOutPath.endsWith(".tmpl", Qt::CaseInsensitive)) {
            relOutPath.chop(5);
        }
        QString targetOut = QDir(QString::fromStdString(outFolder)).filePath(relOutPath);
        mappings.push_back({fi.absoluteFilePath().toStdString(), targetOut.toStdString()});
    }

    if (mappings.empty()) {
        qDebug() << "[CodeGenerator] No template files matching pattern found in directory:" << resolvedTmplFolder.c_str();
        return report;
    }

    return generate(json_data, resolvedTmplFolder, outFolder, mappings, base_dir);
}

void CodeGenerator::parse(json json_data, const std::string &template_folder, const std::string &output_folder) {
    parseDirectory(json_data, template_folder, output_folder);
}

void CodeGenerator::parseCustom(json json_data, const std::string &template_folder, const std::vector<TemplateMapping>& mappings) {
    generate(json_data, template_folder, "", mappings);
}
