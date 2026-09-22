/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "CodeGenerator.hpp"
#include <set>
#include <QDirIterator>
#include <QProcess>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QTemporaryFile>

void CodeGenerator::registerHelpers(Environment &env) {
  // Helper: {{ to_hex(val, width) }}
  env.add_callback("to_hex", 2, [](Arguments &args) {
    uint64_t val = 0;
    if (args.at(0)->is_number()) {
      val = args.at(0)->get<uint64_t>();
    } else if (args.at(0)->is_string()) {
      try {
        val = std::stoull(args.at(0)->get<std::string>(), nullptr, 0);
      } catch (...) {
        val = 0;
      }
    }
    int width = args.at(1)->is_number() ? args.at(1)->get<int>() : 0;
    std::stringstream ss;
    ss << "0x" << std::hex << std::uppercase << std::setfill('0')
       << std::setw(width) << val;
    return ss.str();
  });

  // Helper: {{ to_dec(val) }}
  env.add_callback("to_dec", 1, [](Arguments &args) {
    uint64_t val = 0;
    if (args.at(0)->is_number()) {
      val = args.at(0)->get<uint64_t>();
    } else if (args.at(0)->is_string()) {
      try {
        val = std::stoull(args.at(0)->get<std::string>(), nullptr, 0);
      } catch (...) {
        val = 0;
      }
    }
    return std::to_string(val);
  });

  // Helper: {{ bitmask(width, lsb) }} -> Generates ((0x1ULL << width) - 1) <<
  // lsb
  env.add_callback("bitmask", 2, [](Arguments &args) {
    uint64_t width = args.at(0)->is_number() ? args.at(0)->get<uint64_t>() : 0;
    uint64_t lsb = args.at(1)->is_number() ? args.at(1)->get<uint64_t>() : 0;
    uint64_t mask = 0;
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

  // Helper: {{ pad_zero(val, width) }} -> Formats numbers as zero-padded
  // strings
  env.add_callback("pad_zero", 2, [](Arguments &args) {
    uint64_t val = 0;
    if (args.at(0)->is_number()) {
      val = args.at(0)->get<uint64_t>();
    } else if (args.at(0)->is_string()) {
      try {
        val = std::stoull(args.at(0)->get<std::string>(), nullptr, 0);
      } catch (...) {
        val = 0;
      }
    }
    int width = args.at(1)->is_number() ? args.at(1)->get<int>() : 0;
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(width) << val;
    return ss.str();
  });

  // Helper: {{ camel_case(str) }} -> e.g. "CTRL_STATUS" -> "ctrlStatus"
  env.add_callback("camel_case", 1, [](Arguments &args) {
    std::string str = args.at(0)->is_string() ? args.at(0)->get<std::string>()
                                              : args.at(0)->dump();
    if (str.size() >= 2 && str.front() == '"' && str.back() == '"')
      str = str.substr(1, str.size() - 2);
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
  env.add_callback("pascal_case", 1, [](Arguments &args) {
    std::string str = args.at(0)->is_string() ? args.at(0)->get<std::string>()
                                              : args.at(0)->dump();
    if (str.size() >= 2 && str.front() == '"' && str.back() == '"')
      str = str.substr(1, str.size() - 2);
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

  // Helper: {{ snake_case(str) }} -> e.g. "CtrlStatus" -> "ctrl_status",
  // "SPI_SYS" -> "spi_sys"
  env.add_callback("snake_case", 1, [](Arguments &args) {
    std::string str = args.at(0)->is_string() ? args.at(0)->get<std::string>()
                                              : args.at(0)->dump();
    if (str.size() >= 2 && str.front() == '"' && str.back() == '"')
      str = str.substr(1, str.size() - 2);
    std::string res;
    for (size_t i = 0; i < str.size(); ++i) {
      char c = str[i];
      if (std::isupper(c)) {
        bool prevIsLower = (i > 0 && std::islower(str[i - 1]));
        bool nextIsLower = (i + 1 < str.size() && std::islower(str[i + 1]) &&
                            i > 0 && str[i - 1] != '_');
        if ((prevIsLower || nextIsLower) && !res.empty() && res.back() != '_') {
          res += '_';
        }
        res += std::tolower(c);
      } else if (c == '-' || c == ' ') {
        if (!res.empty() && res.back() != '_')
          res += '_';
      } else {
        res += c;
      }
    }
    return res;
  });

  // Helper: {{ c_type(width) }} -> uint8_t, uint16_t, uint32_t, uint64_t
  env.add_callback("c_type", 1, [](Arguments &args) {
    uint64_t w = args.at(0)->is_number() ? args.at(0)->get<uint64_t>() : 32;
    if (w <= 8)
      return std::string("uint8_t");
    if (w <= 16)
      return std::string("uint16_t");
    if (w <= 32)
      return std::string("uint32_t");
    return std::string("uint64_t");
  });

  // Helper: {{ msb(width, lsb) }} or {{ msb(lsb, width) }} -> lsb + width - 1
  env.add_callback("msb", 2, [](Arguments &args) {
    uint64_t arg0 = args.at(0)->is_number() ? args.at(0)->get<uint64_t>() : 0;
    uint64_t arg1 = args.at(1)->is_number() ? args.at(1)->get<uint64_t>() : 1;
    uint64_t m = (arg0 + arg1 > 0) ? (arg0 + arg1 - 1) : 0;
    return std::to_string(m);
  });

  // Helper: {{ sv_hex(val, width) }} or {{ sv_hex(val) }} -> SystemVerilog hex
  // literal (e.g. 32'h0000, 32'h4D87A9DC, 1'h0)
  auto sv_hex_fn = [](Arguments &args) {
    uint64_t val = 0;
    int width = 32;
    if (args.at(0)->is_number()) {
      val = args.at(0)->get<uint64_t>();
    } else if (args.at(0)->is_string()) {
      std::string s = args.at(0)->get<std::string>();
      if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
        s = s.substr(1, s.size() - 2);
      }
      try {
        val = std::stoull(s, nullptr, 0);
      } catch (...) {
        val = 0;
      }
    }

    if (args.size() > 1) {
      if (args.at(1)->is_number()) {
        width = args.at(1)->get<int>();
      } else if (args.at(1)->is_string()) {
        try {
          width = std::stoi(args.at(1)->get<std::string>());
        } catch (...) {
          width = 32;
        }
      }
    }

    if (width > 0 && width < 64) {
      val &= ((1ULL << width) - 1ULL);
    }

    std::stringstream ss;
    if (width <= 0) {
      ss << "'h" << std::hex << std::uppercase << val;
    } else {
      int digits = (width + 3) / 4;
      if (width >= 32 && val < 0x10000ULL) {
        digits = 4;
      }
      ss << width << "'h" << std::hex << std::uppercase << std::setfill('0')
         << std::setw(digits) << val;
    }
    return ss.str();
  };
  env.add_callback("sv_hex", 1, sv_hex_fn);
  env.add_callback("sv_hex", 2, sv_hex_fn);
}

std::string
CodeGenerator::resolveTemplatePath(const std::string &tmpl_path,
                                   const std::string &default_folder,
                                   const std::string &base_dir) {
  if (tmpl_path.empty())
    return "";

  std::string expTmpl = PathUtils::expandEnvVars(tmpl_path);
  std::string expDefault = PathUtils::expandEnvVars(default_folder);
  std::string expBase = PathUtils::expandEnvVars(base_dir);

  std::string resolved = PathUtils::resolvePath(expTmpl, expBase, expDefault);
  if (QFile::exists(QString::fromStdString(resolved))) {
    return resolved;
  }

  // Fallback: check global/installed default templates directory
  QString defTmplDir = PathUtils::defaultTemplatesDir();
  if (!defTmplDir.isEmpty()) {
    std::string defStr = defTmplDir.toStdString();
    if (defStr != expDefault) {
      std::string resFallback =
          PathUtils::resolvePath(expTmpl, defStr, expBase);
      if (QFile::exists(QString::fromStdString(resFallback))) {
        return resFallback;
      }
    }
  }

  return resolved;
}

std::string CodeGenerator::resolveOutputPath(
    const std::string &tmpl_path, const std::string &out_path,
    const std::string &default_out_folder, const std::string &base_dir,
    const json &context) {
  QString expTmpl =
      QString::fromStdString(PathUtils::expandEnvVars(tmpl_path)).trimmed();
  expTmpl = PathUtils::normalizeSeparators(expTmpl);

  QString expDefaultTmpl =
      PathUtils::expandEnvVars(PathUtils::defaultTemplatesDir()).trimmed();
  expDefaultTmpl = PathUtils::normalizeSeparators(expDefaultTmpl);

  // Extract subpath relative to templates folder if applicable
  QString relSubPath;
  if (expTmpl.startsWith(expDefaultTmpl + "/", Qt::CaseInsensitive)) {
    relSubPath = expTmpl.mid(expDefaultTmpl.length() + 1);
  } else if (expTmpl.startsWith("./" + expDefaultTmpl + "/",
                                Qt::CaseInsensitive)) {
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

  QString expDefaultOut =
      QString::fromStdString(PathUtils::expandEnvVars(default_out_folder))
          .trimmed();
  if (expDefaultOut.isEmpty()) {
    expDefaultOut = PathUtils::defaultOutputDir();
  }
  expDefaultOut = PathUtils::normalizeSeparators(expDefaultOut);

  QString expOut =
      QString::fromStdString(PathUtils::expandEnvVars(out_path)).trimmed();
  expOut = PathUtils::normalizeSeparators(expOut);

  // Dynamic variable expansion
  if (!expOut.isEmpty()) {
    std::string rawBlockName;
    auto ctxBlksIt = context.find("blocks");
    if (ctxBlksIt != context.end() && ctxBlksIt->is_array() &&
        !ctxBlksIt->empty()) {
      rawBlockName = (*ctxBlksIt)[0].value("name", "");
    }
    if (rawBlockName.empty()) {
      rawBlockName = context.value("name", "");
    }
    QString blockName = QString::fromStdString(rawBlockName).toLower();

    std::string rawProj = context.value("project_name", "");
    if (rawProj.empty())
      rawProj = rawBlockName;
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

  QString targetDirOrFile =
      PathUtils::normalizeSeparators(expOut.isEmpty() ? expDefaultOut : expOut);

  // Resolve directory/file against base_dir if relative
  QString resolved = PathUtils::resolvePath(
      targetDirOrFile,
      QString::fromStdString(PathUtils::expandEnvVars(base_dir)));
  resolved = PathUtils::normalizeSeparators(resolved);

  QFileInfo outInfo(resolved);
  bool isKnownFileWithoutExt =
      outInfo.fileName().compare("makefile", Qt::CaseInsensitive) == 0;
  // If output is explicitly a directory, ends with a slash separator, or has no
  // file extension (folder path)
  if (!isKnownFileWithoutExt &&
      (targetDirOrFile.endsWith('/') || (outInfo.exists() && outInfo.isDir()) ||
       !outInfo.fileName().contains('.'))) {
    QString finalSubPath = relSubPath;
    if (!category.isEmpty()) {
      QString normTarget = targetDirOrFile;
      if (normTarget.endsWith('/')) {
        normTarget.chop(1);
      }
      if (normTarget.endsWith("/" + category, Qt::CaseInsensitive) ||
          normTarget.compare(category, Qt::CaseInsensitive) == 0) {
        finalSubPath = baseFileName;
      }
    }
    return PathUtils::normalizeSeparators(QDir(resolved).filePath(finalSubPath))
        .toStdString();
  }

  return resolved.toStdString();
}

GenerationReport CodeGenerator::generate(
    const json &json_data, const std::string &default_template_folder,
    const std::string &default_output_folder,
    const std::vector<TemplateMapping> &mappings, const std::string &base_dir,
    const std::string &python_script) {
  GenerationReport report;

  // Ensure memory gap padding is present if raw json data didn't go through
  // model extraction
  json preparedJson = json_data;
  auto regWidthBytesIt = preparedJson.find("reg_width_bytes");
  if (regWidthBytesIt == preparedJson.end()) {
    uint32_t width = preparedJson.value("reg_width", 32U);
    preparedJson["reg_width_bytes"] = (width > 0) ? (width / 8) : 4;
  }
  auto blksIt = preparedJson.find("blocks");
  if (blksIt != preparedJson.end() && blksIt->is_array()) {
    uint64_t regBytes = preparedJson.value("reg_width_bytes", 4ULL);
    if (regBytes == 0)
      regBytes = 4;
    for (auto &blk : *blksIt) {
      auto regsIt = blk.find("registers");
      if (regsIt != blk.end() && regsIt->is_array()) {
        uint64_t currentOffset = 0;
        for (auto &r : *regsIt) {
          if (!r.contains("pad_words_before")) {
            uint64_t regOffset = r.value("offset_lsb", 0ULL);
            uint64_t padBytes =
                (regOffset > currentOffset) ? (regOffset - currentOffset) : 0;
            uint64_t padWords = padBytes / regBytes;
            r["pad_bytes_before"] = padBytes;
            r["pad_words_before"] = padWords;
          }
          currentOffset = r.value("offset_lsb", 0ULL) + regBytes;
        }
      }
    }
  }

  preparedJson["features"] = extractFeatures(preparedJson);

  // If no specific template mappings are defined, scan the default template
  // folder
  if (mappings.empty()) {
    report = parseDirectory(preparedJson, default_template_folder,
                            default_output_folder, base_dir, "");
  } else {
    // Pre-scan mappings to identify output paths for RTL, UVM, and SIM
    // directory
    QString resolvedRtlOut;
    QString resolvedUvmOut;
    QString resolvedSimDir;

    for (const auto &m : mappings) {
      QString tmplPath = PathUtils::normalizeSeparators(
          QString::fromStdString(m.template_file));
      std::string resolvedOut =
          resolveOutputPath(m.template_file, m.output_file,
                            default_output_folder, base_dir, preparedJson);
      QString normOut =
          PathUtils::normalizeSeparators(QString::fromStdString(resolvedOut));
      QFileInfo outFi(normOut);

      // Identify primary hardware RTL file and UVM model dynamically from
      // selected mappings
      if (normOut.endsWith(".sv", Qt::CaseInsensitive) ||
          normOut.endsWith(".v", Qt::CaseInsensitive) ||
          normOut.endsWith(".vhd", Qt::CaseInsensitive)) {
        if (!normOut.contains("_tb", Qt::CaseInsensitive) &&
            !normOut.contains("tb_", Qt::CaseInsensitive) &&
            !tmplPath.contains("_tb", Qt::CaseInsensitive) &&
            !tmplPath.contains("tb_", Qt::CaseInsensitive)) {
          if ((tmplPath.contains("/rtl/", Qt::CaseInsensitive) ||
               normOut.contains("rtl", Qt::CaseInsensitive)) &&
              resolvedRtlOut.isEmpty()) {
            resolvedRtlOut = outFi.absoluteFilePath();
          } else if ((tmplPath.contains("/uvm/", Qt::CaseInsensitive) ||
                      normOut.contains("uvm", Qt::CaseInsensitive)) &&
                     resolvedUvmOut.isEmpty()) {
            resolvedUvmOut = outFi.absoluteFilePath();
          }
        }
      }

      if (tmplPath.contains("/sim/", Qt::CaseInsensitive) ||
          normOut.contains("/sim/", Qt::CaseInsensitive)) {
        resolvedSimDir = outFi.isDir() ? outFi.absoluteFilePath()
                                       : outFi.dir().absolutePath();
      }
    }

    if (resolvedSimDir.isEmpty() &&
        (!resolvedRtlOut.isEmpty() || !resolvedUvmOut.isEmpty())) {
      resolvedSimDir = PathUtils::normalizeSeparators(
          QDir(QString::fromStdString(default_output_folder.empty()
                                          ? PathUtils::DEFAULT_OUTPUT_DIR
                                          : default_output_folder))
              .filePath("sim"));
    }

    if (!resolvedRtlOut.isEmpty() && !resolvedSimDir.isEmpty()) {
      QDir simD(resolvedSimDir);
      preparedJson["sim_rel_rtl_path"] =
          PathUtils::normalizeSeparators(simD.relativeFilePath(resolvedRtlOut))
              .toStdString();
      preparedJson["sim_rel_rtl_dir"] =
          PathUtils::normalizeSeparators(
              simD.relativeFilePath(
                  QFileInfo(resolvedRtlOut).dir().absolutePath()))
              .toStdString();
    } else {
      preparedJson["sim_rel_rtl_path"] = "";
      preparedJson["sim_rel_rtl_dir"] = "";
    }

    if (!resolvedUvmOut.isEmpty() && !resolvedSimDir.isEmpty()) {
      QDir simD(resolvedSimDir);
      preparedJson["sim_rel_uvm_path"] =
          PathUtils::normalizeSeparators(simD.relativeFilePath(resolvedUvmOut))
              .toStdString();
      preparedJson["sim_rel_uvm_dir"] =
          PathUtils::normalizeSeparators(
              simD.relativeFilePath(
                  QFileInfo(resolvedUvmOut).dir().absolutePath()))
              .toStdString();
    } else {
      preparedJson["sim_rel_uvm_path"] = "";
      preparedJson["sim_rel_uvm_dir"] = "";
    }

    // Process each configured template mapping
    for (const auto &mapping : mappings) {
      std::string resolvedTmpl = resolveTemplatePath(
          mapping.template_file, default_template_folder, base_dir);
      std::string resolvedOut =
          resolveOutputPath(mapping.template_file, mapping.output_file,
                            default_output_folder, base_dir, preparedJson);

      QFileInfo tmplInfo(QString::fromStdString(resolvedTmpl));
      if (!tmplInfo.exists() || !tmplInfo.isFile()) {
        std::string err =
            "Template file does not exist: " + mapping.template_file;
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

      // Use the template file's directory as Inja base so relative includes
      // inside template resolve correctly
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

        qDebug() << "[CodeGenerator] Successfully generated:"
                 << resolvedOut.c_str()
                 << "from template:" << resolvedTmpl.c_str();
        report.success_files.push_back(resolvedOut);
      } catch (const std::exception &e) {
        std::string err = std::string("Template rendering error: ") + e.what();
        qWarning() << "[CodeGenerator]" << err.c_str()
                   << "for template:" << resolvedTmpl.c_str();
        report.errors.push_back({mapping.template_file, err});
      }
    }
  }

  // Launch Python script if configured
  if (!python_script.empty()) {
    std::string pyOut, pyErr;
    bool ok =
        runPythonScript(python_script, preparedJson, base_dir, &pyOut, &pyErr);
    if (ok) {
      report.success_files.push_back("Python Script: " + python_script);
      if (!pyOut.empty()) {
        qDebug() << "[CodeGenerator] Python script output:\n" << pyOut.c_str();
      }
    } else {
      std::string errMsg =
          "Python script execution failed: " + (pyErr.empty() ? pyOut : pyErr);
      qWarning() << "[CodeGenerator]" << errMsg.c_str();
      report.errors.push_back({python_script, errMsg});
    }
  }

  return report;
}

GenerationReport CodeGenerator::parseDirectory(
    const json &json_data, const std::string &template_folder,
    const std::string &output_folder, const std::string &base_dir,
    const std::string &python_script) {
  GenerationReport report;
  std::string rawTmplFolder = template_folder.empty()
                                  ? PathUtils::DEFAULT_TEMPLATES_DIR
                                  : template_folder;
  std::string resolvedTmplFolder =
      PathUtils::resolvePath(rawTmplFolder, base_dir);
  std::string outFolder =
      output_folder.empty() ? PathUtils::DEFAULT_OUTPUT_DIR : output_folder;

  QDir tmplDir(QString::fromStdString(resolvedTmplFolder));
  if (!tmplDir.exists()) {
    report.errors.push_back(
        {resolvedTmplFolder,
         "Template directory does not exist: " + resolvedTmplFolder});
    return report;
  }

  QStringList filters;
  filters << "*.inja" << "*.tmpl";

  std::vector<TemplateMapping> mappings;
  QDirIterator it(QString::fromStdString(resolvedTmplFolder), filters,
                  QDir::Files, QDirIterator::Subdirectories);
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
    QString targetOut =
        QDir(QString::fromStdString(outFolder)).filePath(relOutPath);
    mappings.push_back(
        {fi.absoluteFilePath().toStdString(), targetOut.toStdString()});
  }

  if (mappings.empty()) {
    qDebug() << "[CodeGenerator] No template files matching pattern found in "
                "directory:"
             << resolvedTmplFolder.c_str();
    if (!python_script.empty()) {
      return generate(json_data, resolvedTmplFolder, outFolder, mappings,
                      base_dir, python_script);
    }
    return report;
  }

  return generate(json_data, resolvedTmplFolder, outFolder, mappings, base_dir,
                  python_script);
}

void CodeGenerator::parse(json json_data, const std::string &template_folder,
                          const std::string &output_folder) {
  parseDirectory(json_data, template_folder, output_folder);
}

void CodeGenerator::parseCustom(json json_data,
                                const std::string &template_folder,
                                const std::vector<TemplateMapping> &mappings) {
  generate(json_data, template_folder, "", mappings);
}

bool CodeGenerator::runPythonScript(const std::string &python_script,
                                    const json &json_data,
                                    const std::string &base_dir,
                                    std::string *stdout_str,
                                    std::string *stderr_str) {
  if (python_script.empty()) {
    if (stderr_str)
      *stderr_str = "No Python script specified.";
    return false;
  }

  QString resolvedScript = PathUtils::resolvePath(
      QString::fromStdString(python_script), QString::fromStdString(base_dir));
  QFileInfo scriptInfo(resolvedScript);
  if (!scriptInfo.exists() || !scriptInfo.isFile()) {
    if (stderr_str)
      *stderr_str =
          "Python script file does not exist: " + resolvedScript.toStdString();
    return false;
  }

  QString pythonExe =
      PathUtils::expandEnvVars(QString::fromLocal8Bit(qgetenv("RMAP_PYTHON")))
          .trimmed();
  if (pythonExe.isEmpty()) {
    pythonExe =
        PathUtils::expandEnvVars(QString::fromLocal8Bit(qgetenv("PYTHON")))
            .trimmed();
  }
  if (pythonExe.isEmpty()) {
    pythonExe = QStandardPaths::findExecutable("python3");
  }
  if (pythonExe.isEmpty()) {
    pythonExe = QStandardPaths::findExecutable("python");
  }
  if (pythonExe.isEmpty()) {
    if (stderr_str)
      *stderr_str = "Python interpreter ('python3' or 'python') not found in "
                    "system PATH.";
    return false;
  }

  std::string jsonDump = json_data.dump(2);

  QString tempDirPath = QString::fromLocal8Bit(qgetenv("RMAP_TMPDIR"));
  if (tempDirPath.isEmpty()) {
    tempDirPath = QDir::tempPath();
  }
  QTemporaryFile tempJsonFile(tempDirPath + "/rmap_context_XXXXXX.json");
  if (!tempJsonFile.open()) {
    if (stderr_str)
      *stderr_str =
          "Failed to create temporary file for register map JSON context.";
    return false;
  }
  tempJsonFile.write(jsonDump.data(), static_cast<qint64>(jsonDump.size()));
  tempJsonFile.flush();
  QString tempJsonPath = tempJsonFile.fileName();
  tempJsonFile.close();

  const char *launcherCode =
      R"(import sys, json, os, types

json_path = sys.argv[1]
script_path = os.path.abspath(sys.argv[2])

with open(json_path, 'r', encoding='utf-8') as f:
    data = json.load(f)

sys.argv = [script_path, json_path] + sys.argv[3:]

script_dir = os.path.dirname(script_path)
if script_dir not in sys.path:
    sys.path.insert(0, script_dir)

def to_hex(val, width=8):
    if isinstance(val, str):
        val = int(val, 0)
    return f"0x{int(val):0{width}X}"

def to_dec(val):
    if isinstance(val, str):
        return str(int(val, 0))
    return str(val)

def bitmask(width, lsb):
    mask = ((1 << int(width)) - 1) << int(lsb)
    return f"0x{mask:X}"

rmap_mod = types.ModuleType('rmap')
rmap_mod.__file__ = script_path
for k, v in data.items():
    setattr(rmap_mod, k, v)
setattr(rmap_mod, 'data', data)
setattr(rmap_mod, 'context', data)
setattr(rmap_mod, 'to_hex', to_hex)
setattr(rmap_mod, 'to_dec', to_dec)
setattr(rmap_mod, 'bitmask', bitmask)
sys.modules['rmap'] = rmap_mod

script_globals = {
    '__name__': '__main__',
    '__file__': script_path,
    '__doc__': None,
    '__builtins__': __builtins__,
    'sys': sys,
    'json': json,
    'os': os,
    'data': data,
    'context': data,
    'rmap': data,
    'regmap': data,
    'to_hex': to_hex,
    'to_dec': to_dec,
    'bitmask': bitmask,
}

for k, v in data.items():
    script_globals[k] = v

with open(script_path, 'r', encoding='utf-8') as f:
    code = compile(f.read(), script_path, 'exec')
    exec(code, script_globals)
)";

  QProcess process;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("RMAP_JSON_FILE", tempJsonPath);
  env.insert("RMAP_JSON_DATA", QString::fromUtf8(jsonDump.c_str()));
  auto nameIt = json_data.find("name");
  if (nameIt != json_data.end() && nameIt->is_string()) {
    env.insert("RMAP_NAME", QString::fromStdString(nameIt->get<std::string>()));
  }
  auto projIt = json_data.find("project_name");
  if (projIt != json_data.end() && projIt->is_string()) {
    env.insert("RMAP_PROJECT_NAME",
               QString::fromStdString(projIt->get<std::string>()));
  }
  auto verIt = json_data.find("project_version");
  if (verIt != json_data.end() && verIt->is_string()) {
    env.insert("RMAP_PROJECT_VERSION",
               QString::fromStdString(verIt->get<std::string>()));
  }
  auto widthIt = json_data.find("reg_width");
  if (widthIt != json_data.end() && widthIt->is_number()) {
    env.insert("RMAP_REG_WIDTH", QString::number(widthIt->get<uint32_t>()));
  }
  process.setProcessEnvironment(env);

  QString workDir = QString::fromStdString(base_dir);
  if (workDir.isEmpty() || !QDir(workDir).exists()) {
    workDir = scriptInfo.dir().absolutePath();
  }
  process.setWorkingDirectory(workDir);

  QStringList args;
  args << "-c" << launcherCode << tempJsonPath << scriptInfo.absoluteFilePath();

  process.start(pythonExe, args);
  if (!process.waitForStarted(5000)) {
    if (stderr_str)
      *stderr_str =
          "Failed to start Python interpreter: " + pythonExe.toStdString();
    return false;
  }

  process.write(jsonDump.data(), static_cast<qint64>(jsonDump.size()));
  process.closeWriteChannel();

  int timeoutMs = 60000;
  QByteArray envTimeout = qgetenv("RMAP_PYTHON_TIMEOUT");
  if (!envTimeout.isEmpty()) {
    bool ok = false;
    int parsed = envTimeout.toInt(&ok);
    if (ok && parsed > 0)
      timeoutMs = parsed;
  }

  bool finished = process.waitForFinished(timeoutMs);
  if (!finished) {
    process.kill();
    process.waitForFinished(1000);
    if (stderr_str) {
      if (timeoutMs % 1000 == 0) {
        *stderr_str = QString("Python script execution timed out (%1 seconds).")
                          .arg(timeoutMs / 1000)
                          .toStdString();
      } else {
        *stderr_str = QString("Python script execution timed out (%1 ms).")
                          .arg(timeoutMs)
                          .toStdString();
      }
    }
    return false;
  }

  QByteArray outBytes = process.readAllStandardOutput();
  QByteArray errBytes = process.readAllStandardError();
  if (stdout_str)
    *stdout_str = QString::fromUtf8(outBytes).toStdString();
  if (stderr_str)
    *stderr_str = QString::fromUtf8(errBytes).toStdString();

  int exitCode = process.exitCode();
  return (process.exitStatus() == QProcess::NormalExit && exitCode == 0);
}

json CodeGenerator::extractFeatures(const json &rootJson) {
  json feat = json::object();

  std::set<std::string> uniquePolicies;
  bool hasHwWritable = false;
  bool hasHwReadable = false;
  bool hasVolatile = false;
  bool hasMemories = false;
  bool hasAddressGaps = false;
  bool hasInterrupts = false;
  bool hasSwWritable = false;

  auto blksIt = rootJson.find("blocks");
  size_t blkCount = 0;
  size_t regCount = 0;
  size_t fldCount = 0;

  if (blksIt != rootJson.end() && blksIt->is_array()) {
    blkCount = blksIt->size();
    for (const auto &blk : *blksIt) {
      auto memIt = blk.find("memories");
      if (memIt != blk.end() && memIt->is_array() && !memIt->empty()) {
        hasMemories = true;
      }

      auto regsIt = blk.find("registers");
      if (regsIt != blk.end() && regsIt->is_array()) {
        for (const auto &r : *regsIt) {
          regCount++;
          uint64_t padBytes = r.value("pad_bytes_before", 0ULL);
          if (padBytes > 0) {
            hasAddressGaps = true;
          }

          auto fldsIt = r.find("fields");
          if (fldsIt != r.end() && fldsIt->is_array() && !fldsIt->empty()) {
            for (const auto &fld : *fldsIt) {
              fldCount++;
              std::string acc = fld.value("access", "");
              if (acc.empty()) {
                acc = r.value("access", "RW");
              }
              std::string uAcc = acc;
              std::transform(uAcc.begin(), uAcc.end(), uAcc.begin(), ::toupper);
              if (!uAcc.empty()) {
                uniquePolicies.insert(uAcc);
                if (uAcc != "RO" && uAcc != "NOACCESS") {
                  hasSwWritable = true;
                }
              }

              std::string hwAcc = fld.value("hw_access", "RO");
              std::string uHwAcc = hwAcc;
              std::transform(uHwAcc.begin(), uHwAcc.end(), uHwAcc.begin(), ::toupper);
              if (uHwAcc != "NA" && !uHwAcc.empty()) {
                hasHwReadable = true;
                if (uHwAcc == "WO" || uHwAcc == "RW" || uHwAcc == "W" || uAcc == "RO") {
                  hasHwWritable = true;
                }
              }

              if (fld.value("volatile", false)) {
                hasVolatile = true;
              }

              if (uAcc == "W1C" || uAcc == "W0C" || uAcc == "RC" ||
                  uAcc == "W1SRC" || uAcc == "W1CRS" || uAcc == "W0SRC" || uAcc == "W0CRS") {
                hasInterrupts = true;
              }
              std::string fldName = fld.value("name", "");
              std::string uName = fldName;
              std::transform(uName.begin(), uName.end(), uName.begin(), ::toupper);
              if (uName.find("IRQ") != std::string::npos || uName.find("INT_") != std::string::npos ||
                  uName == "INT" || uName.find("_INT") != std::string::npos) {
                hasInterrupts = true;
              }
            }
          } else {
            std::string acc = r.value("access", "RW");
            std::string uAcc = acc;
            std::transform(uAcc.begin(), uAcc.end(), uAcc.begin(), ::toupper);
            if (!uAcc.empty()) {
              uniquePolicies.insert(uAcc);
              if (uAcc != "RO" && uAcc != "NOACCESS") {
                hasSwWritable = true;
              }
            }
          }
        }
      }
    }
  }

  auto topMemIt = rootJson.find("memories");
  if (topMemIt != rootJson.end() && topMemIt->is_array() && !topMemIt->empty()) {
    hasMemories = true;
  }

  uint32_t regWidth = rootJson.value("reg_width", 32U);
  bool hasByteStrobes = (regWidth > 8);

  feat["access_policies"] = json::array();
  std::string policiesStr;
  for (const auto &p : uniquePolicies) {
    feat["access_policies"].push_back(p);
    std::string key = "has_" + p;
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    feat[key] = true;
    if (!policiesStr.empty()) {
      policiesStr += ", ";
    }
    policiesStr += "`" + p + "`";
  }
  feat["access_policies_str"] = policiesStr;

  static const std::vector<std::string> standardPolicies = {
      "rw", "ro", "wo", "w1c", "w1s", "w1t", "w0c", "rc", "rs",
      "wc", "ws", "w1src", "w1crs", "w0src", "w0crs", "woc", "wos",
      "w1", "wo1", "noaccess"
  };
  for (const auto &sp : standardPolicies) {
    std::string key = "has_" + sp;
    if (!feat.contains(key)) {
      feat[key] = false;
    }
  }

  feat["has_hw_writable"] = hasHwWritable;
  feat["has_hw_readable"] = hasHwReadable;
  feat["has_hw_sidebands"] = (hasHwWritable || hasHwReadable);
  feat["has_hw_precedence"] = (hasHwWritable && hasSwWritable);
  feat["has_volatile"] = hasVolatile;
  feat["has_memories"] = hasMemories;
  feat["has_address_gaps"] = hasAddressGaps;
  feat["has_interrupts"] = hasInterrupts;
  feat["has_byte_strobes"] = hasByteStrobes;
  feat["has_multiple_blocks"] = (blkCount > 1);
  feat["block_count"] = blkCount;
  feat["register_count"] = regCount;
  feat["field_count"] = fldCount;

  return feat;
}
