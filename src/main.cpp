/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "LanguageManager.hpp"
#include "PathUtils.hpp"
#include "RmapVersion.hpp"
#include "ThemeManager.hpp"
#include "rmap.hpp"
#include <atomic>
#include <csignal>
#include <iostream>

static void signalHandler(int sig) {
  Q_UNUSED(sig);
  if (QCoreApplication::instance()) {
    QMetaObject::invokeMethod(QCoreApplication::instance(),
                              &QCoreApplication::quit, Qt::QueuedConnection);
  }
}

int main(int argc, char *argv[]) {
  std::signal(SIGTERM, signalHandler);
  std::signal(SIGINT, signalHandler);
  Q_INIT_RESOURCE(resources);
  const QString version =
      QStringLiteral("v") + QString::fromLatin1(RMAP_VERSION_STRING);

  bool headless_mode = false;
  const char *disp = std::getenv("DISPLAY");
  const char *wayland = std::getenv("WAYLAND_DISPLAY");
  if ((!disp || disp[0] == '\0') && (!wayland || wayland[0] == '\0')) {
    headless_mode = true;
  }
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--export" || arg == "-e" || arg == "--convert" || arg == "-c" ||
        arg.rfind("--convert=", 0) == 0 || arg.rfind("-c=", 0) == 0 ||
        arg == "--lint" || arg == "-l" || arg == "--diff" || arg == "-d" ||
        arg.rfind("--diff=", 0) == 0 || arg.rfind("-d=", 0) == 0 ||
        arg == "--strict" || arg == "--report-format" ||
        arg.rfind("--report-format=", 0) == 0 || arg == "--out" ||
        arg == "-o" || arg.rfind("--out=", 0) == 0 ||
        arg.rfind("-o=", 0) == 0 || arg == "--help" || arg == "-h" ||
        arg == "--help-all" || arg == "--version" || arg == "-v") {
      headless_mode = true;
      break;
    }
  }
  if (headless_mode) {
    qputenv("QT_QPA_PLATFORM", "offscreen");
  }

  QApplication app(argc, argv);
  app.setApplicationName("rmap");
  app.setApplicationDisplayName("rmap");
  app.setDesktopFileName("rmap");
  app.setApplicationVersion(version);

  QIcon appIcon(":/icons/app_icon.png");
  appIcon.addFile(":/icons/app_icon_512.png", QSize(512, 512));
  appIcon.addFile(":/icons/app_icon_256.png", QSize(256, 256));
  appIcon.addFile(":/icons/app_icon_128.png", QSize(128, 128));
  appIcon.addFile(":/icons/app_icon_64.png", QSize(64, 64));
  appIcon.addFile(":/icons/app_icon_48.png", QSize(48, 48));
  appIcon.addFile(":/icons/app_icon_32.png", QSize(32, 32));
  appIcon.addFile(":/icons/app_icon_16.png", QSize(16, 16));
  app.setWindowIcon(appIcon);
  QCommandLineParser parser;
  QCommandLineOption f_opt({"f", "file"}, "Register Map file to load", "file");
  QCommandLineOption e_opt({"e", "export"},
                           "Run in headless mode and export templates");
  QCommandLineOption o_opt(
      {"o", "out"}, "Override default output directory or output report file",
      "path");
  QCommandLineOption c_opt(
      {"c", "convert"}, "Convert loaded register map to specified output file",
      "out_file");
  QCommandLineOption l_opt(
      {"l", "lint"}, "Run headless linter validation on the register map file");
  QCommandLineOption strict_opt("strict",
                                "Enable strict validation rules (check empty "
                                "descriptions and alignment)");
  QCommandLineOption fmt_opt("report-format",
                             "Report format for linting (text, json, sarif, "
                             "junit) or diff (text, markdown)",
                             "format", "text");
  QCommandLineOption diff_opt(
      {"d", "diff"}, "Compare loaded register map against another file",
      "compare_file");
  const QString availableThemes =
      ThemeManager::instance().themeIds().join(QStringLiteral(", "));
  const QString themeHelp =
      QStringLiteral("Set active colour scheme (%1)").arg(availableThemes);
  const QString availableLangs =
      LanguageManager::instance().languageCodes().join(QStringLiteral(", "));
  const QString langHelp =
      QStringLiteral("Set application language (%1)").arg(availableLangs);

  QCommandLineOption theme_opt({"t", "theme", "colour-scheme", "color-scheme"},
                               themeHelp, "scheme", "default");
  QCommandLineOption lang_opt({"lang", "language"}, langHelp, "language");

  parser.setApplicationDescription(
      "rmap — Hardware Register Map Designer & Model Generator");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument(
      "file",
      "Register map file to load (.rmt, .rmb, .svd, .rdl, .systemrdl, .xml, "
      ".ipxact, .json, .csv, .tsv).",
      "[file]");
  parser.addOption(f_opt);
  parser.addOption(e_opt);
  parser.addOption(o_opt);
  parser.addOption(c_opt);
  parser.addOption(l_opt);
  parser.addOption(strict_opt);
  parser.addOption(fmt_opt);
  parser.addOption(diff_opt);
  parser.addOption(theme_opt);
  parser.addOption(lang_opt);
  parser.process(app);

  // 1. Validate mutual exclusivity of headless action modes
  int actionCount = 0;
  if (parser.isSet(e_opt)) {
    actionCount++;
  }
  if (parser.isSet(c_opt)) {
    actionCount++;
  }
  if (parser.isSet(l_opt)) {
    actionCount++;
  }
  if (parser.isSet(diff_opt)) {
    actionCount++;
  }

  if (actionCount > 1) {
    std::cerr << "Error: The action arguments --export, --convert, --lint, and "
                 "--diff are mutually exclusive."
              << std::endl;
    return 1;
  }

  // 2. Validate input file arguments and positional arguments
  if (parser.isSet(f_opt) && !parser.positionalArguments().isEmpty()) {
    std::cerr << "Error: Conflicting input files specified via --file and "
                 "positional argument."
              << std::endl;
    return 1;
  }
  if (parser.positionalArguments().size() > 1) {
    std::cerr << "Error: Too many positional arguments specified." << std::endl;
    return 1;
  }

  QString regmap_file;
  if (parser.isSet(f_opt)) {
    regmap_file = PathUtils::expandEnvVars(parser.value(f_opt));
    if (regmap_file.trimmed().isEmpty()) {
      std::cerr << "Error: --file argument cannot be empty." << std::endl;
      return 1;
    }
  } else if (!parser.positionalArguments().isEmpty()) {
    regmap_file =
        PathUtils::expandEnvVars(parser.positionalArguments().first());
  }

  // 3. Validate action-specific requirements on input files
  if (parser.isSet(diff_opt)) {
    if (regmap_file.isEmpty()) {
      std::cerr
          << "Error: --diff requires an input register map file to compare."
          << std::endl;
      return 1;
    }
    QString file2 = PathUtils::expandEnvVars(parser.value(diff_opt));
    if (file2.trimmed().isEmpty()) {
      std::cerr << "Error: --diff requires a comparison target file argument."
                << std::endl;
      return 1;
    }
  }

  if (parser.isSet(l_opt)) {
    if (regmap_file.isEmpty()) {
      std::cerr << "Error: --lint requires an input register map file."
                << std::endl;
      return 1;
    }
  }

  if (parser.isSet(c_opt)) {
    if (regmap_file.isEmpty()) {
      std::cerr << "Error: --convert requires an input register map file."
                << std::endl;
      return 1;
    }
    QString out_file = PathUtils::expandEnvVars(parser.value(c_opt));
    if (out_file.trimmed().isEmpty()) {
      std::cerr << "Error: --convert requires a destination file argument."
                << std::endl;
      return 1;
    }
  }

  if (parser.isSet(e_opt)) {
    if (regmap_file.isEmpty()) {
      std::cerr << "Error: --export requires an input register map file."
                << std::endl;
      return 1;
    }
  }

  // 4. Validate modifier option compatibility
  if (parser.isSet(strict_opt) && !parser.isSet(l_opt)) {
    std::cerr << "Error: --strict is only compatible with --lint." << std::endl;
    return 1;
  }

  if (parser.isSet(fmt_opt)) {
    if (!parser.isSet(l_opt) && !parser.isSet(diff_opt)) {
      std::cerr << "Error: --report-format is only compatible with --lint or "
                   "--diff."
                << std::endl;
      return 1;
    }
    QString format = parser.value(fmt_opt).toLower();
    if (parser.isSet(l_opt)) {
      if (format != "text" && format != "json" && format != "sarif" &&
          format != "junit") {
        std::cerr
            << "Error: Invalid --report-format '"
            << parser.value(fmt_opt).toStdString()
            << "' for --lint. Supported formats: text, json, sarif, junit."
            << std::endl;
        return 1;
      }
    } else if (parser.isSet(diff_opt)) {
      if (format != "text" && format != "markdown") {
        std::cerr << "Error: Invalid --report-format '"
                  << parser.value(fmt_opt).toStdString()
                  << "' for --diff. Supported formats: text, markdown."
                  << std::endl;
        return 1;
      }
    }
  }

  bool isHeadlessAction = (actionCount > 0);
  if (parser.isSet(o_opt)) {
    if (!isHeadlessAction) {
      std::cerr << "Error: --out is only valid with headless operations "
                   "(--export, --lint, --diff)."
                << std::endl;
      return 1;
    }
    if (parser.isSet(c_opt)) {
      std::cerr << "Error: --out is incompatible with --convert (target file "
                   "is specified as argument to --convert)."
                << std::endl;
      return 1;
    }
  }

  // 5. Execute actions
  if (parser.isSet(diff_opt)) {
    QString file2 = PathUtils::expandEnvVars(parser.value(diff_opt));
    QString out_path = PathUtils::expandEnvVars(parser.value(o_opt));
    QString format = parser.value(fmt_opt);
    bool ok = RegMapWindow::semanticDiff(regmap_file, file2, format, out_path);
    return ok ? 0 : 1;
  }

  RegMapWindow *mainWin = new RegMapWindow(regmap_file);
  if (parser.isSet(theme_opt)) {
    mainWin->setColourScheme(parser.value(theme_opt));
  }
  if (parser.isSet(lang_opt)) {
    mainWin->setLanguage(parser.value(lang_opt));
  }

  if (parser.isSet(l_opt)) {
    bool strict = parser.isSet(strict_opt);
    QString format = parser.value(fmt_opt);
    QString out_path = PathUtils::expandEnvVars(parser.value(o_opt));
    bool passed = mainWin->headlessLint(strict, format, out_path);
    delete mainWin;
    return passed ? 0 : 1;
  }

  if (parser.isSet(c_opt)) {
    QString out_file = PathUtils::expandEnvVars(parser.value(c_opt));
    bool success = mainWin->fileSave(out_file);
    delete mainWin;
    return success ? 0 : 1;
  }

  if (parser.isSet(e_opt)) {
    QString out_dir = PathUtils::expandEnvVars(parser.value(o_opt));
    bool success = mainWin->headlessExport(out_dir);
    delete mainWin;
    return success ? 0 : 1;
  }

  mainWin->show();
  int ret = app.exec();
  delete mainWin;
  return ret;
}
