#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
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
    const std::string &base_dir)
{
    QFileInfo tmplInfo(QString::fromStdString(tmpl_path));

    // Determine default base filename by stripping .inja / .tmpl suffix
    QString baseName = tmplInfo.fileName();
    if (baseName.endsWith(".inja", Qt::CaseInsensitive)) {
        baseName.chop(5);
    } else if (baseName.endsWith(".tmpl", Qt::CaseInsensitive)) {
        baseName.chop(5);
    }

    QString expOut = QString::fromStdString(PathUtils::expandEnvVars(out_path)).trimmed();
    QString expDefaultOut = QString::fromStdString(PathUtils::expandEnvVars(default_out_folder)).trimmed();
    if (expDefaultOut.isEmpty()) {
        expDefaultOut = PathUtils::defaultOutputDir();
    }

    QString targetDirOrFile = expOut.isEmpty() ? expDefaultOut : expOut;

    // Resolve directory/file against base_dir if relative
    QString resolved = PathUtils::resolvePath(targetDirOrFile, QString::fromStdString(PathUtils::expandEnvVars(base_dir)));

    QFileInfo outInfo(resolved);
    // If output is explicitly a directory, ends with a slash separator, or has no file extension (folder path)
    if (targetDirOrFile.endsWith('/') || targetDirOrFile.endsWith('\\') || (outInfo.exists() && outInfo.isDir()) || !outInfo.fileName().contains('.')) {
        return PathUtils::normalizeSeparators(QDir(resolved).filePath(baseName)).toStdString();
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

    // Process each configured template mapping
    for (const auto &mapping : mappings) {
        std::string resolvedTmpl = resolveTemplatePath(mapping.template_file, default_template_folder, base_dir);
        std::string resolvedOut  = resolveOutputPath(mapping.template_file, mapping.output_file, default_output_folder, base_dir);

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
            env.write(temp, json_data, resolvedOut);

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
    filters << "*.inja" << "*.tmpl" << "*.txt" << "*.sv" << "*.h" << "*.cpp";
    QFileInfoList fileList = tmplDir.entryInfoList(filters, QDir::Files);

    if (fileList.isEmpty()) {
        qDebug() << "[CodeGenerator] No template files matching pattern found in directory:" << resolvedTmplFolder.c_str();
        return report;
    }

    std::vector<TemplateMapping> mappings;
    for (const QFileInfo &fi : fileList) {
        mappings.push_back({fi.absoluteFilePath().toStdString(), outFolder});
    }

    return generate(json_data, resolvedTmplFolder, outFolder, mappings, base_dir);
}

void CodeGenerator::parse(json json_data, const std::string &template_folder, const std::string &output_folder) {
    parseDirectory(json_data, template_folder, output_folder);
}

void CodeGenerator::parseCustom(json json_data, const std::string &template_folder, const std::vector<TemplateMapping>& mappings) {
    generate(json_data, template_folder, "", mappings);
}
