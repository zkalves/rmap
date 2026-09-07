/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "rmap.hpp"
#include "ThemeManager.hpp"
#include "LanguageManager.hpp"
#include "PathUtils.hpp"
#include "RmapVersion.hpp"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(resources);
    const QString version = QStringLiteral("v") + QString::fromLatin1(RMAP_VERSION_STRING);

    bool headless_mode = false;
    const char* disp = std::getenv("DISPLAY");
    const char* wayland = std::getenv("WAYLAND_DISPLAY");
    if (!disp && !wayland) {
        headless_mode = true;
    }
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--export" || arg == "-e" ||
            arg == "--convert" || arg == "-c" ||
            arg == "--lint" || arg == "-l" ||
            arg == "--diff" || arg == "-d" ||
            arg == "--help" || arg == "-h" ||
            arg == "--version" || arg == "-v") {
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
    QCommandLineOption f_opt({"f","file"}, "Register Map file to load", "file");
    QCommandLineOption e_opt({"e","export"}, "Run in headless mode and export templates");
    QCommandLineOption o_opt({"o","out"}, "Override default output directory or output report file", "path");
    QCommandLineOption c_opt({"c","convert"}, "Convert loaded register map to specified output file", "out_file");
    QCommandLineOption l_opt({"l","lint"}, "Run headless linter validation on the register map file");
    QCommandLineOption strict_opt("strict", "Enable strict validation rules (check empty descriptions and alignment)");
    QCommandLineOption fmt_opt("report-format", "Report format for linting (text, json, sarif, junit) or diff (text, markdown)", "format", "text");
    QCommandLineOption diff_opt({"d","diff"}, "Compare loaded register map against another file", "compare_file");
    QCommandLineOption theme_opt({"t","theme","colour-scheme","color-scheme"}, "Set active colour scheme (solarized8, solarized8_light, nord, dracula, monokai, classic)", "scheme", "solarized8");
    QCommandLineOption lang_opt({"lang","language"}, "Set application language (e.g. en, es, de, fr, zh_CN, ja, pt_BR)", "language");

    parser.setApplicationDescription("rmap — Hardware Register Map Designer & Model Generator");
    parser.addHelpOption();
    parser.addVersionOption();
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

    QString regmap_file = PathUtils::expandEnvVars(parser.value("file"));

    if (parser.isSet(diff_opt)) {
        QString file2 = PathUtils::expandEnvVars(parser.value("diff"));
        QString out_path = PathUtils::expandEnvVars(parser.value("out"));
        QString format = parser.value("report-format");
        bool ok = RegMapWindow::semanticDiff(regmap_file, file2, format, out_path);
        return ok ? 0 : 1;
    }

    RegMapWindow * mainWin = new RegMapWindow(regmap_file);
    if (parser.isSet(theme_opt)) {
        mainWin->setColourScheme(parser.value(theme_opt));
    }
    if (parser.isSet(lang_opt)) {
        mainWin->setLanguage(parser.value(lang_opt));
    }

    if (parser.isSet(l_opt)) {
        bool strict = parser.isSet(strict_opt);
        QString format = parser.value("report-format");
        QString out_path = PathUtils::expandEnvVars(parser.value("out"));
        bool passed = mainWin->headlessLint(strict, format, out_path);
        delete mainWin;
        return passed ? 0 : 1;
    }

    if (parser.isSet(c_opt)) {
        QString out_file = PathUtils::expandEnvVars(parser.value("convert"));
        bool success = mainWin->fileSave(out_file);
        delete mainWin;
        return success ? 0 : 1;
    }

    if (parser.isSet(e_opt)) {
        QString out_dir = PathUtils::expandEnvVars(parser.value("out"));
        bool success = mainWin->headlessExport(out_dir);
        delete mainWin;
        return success ? 0 : 1;
    }

    mainWin->show();
    return app.exec();
}
