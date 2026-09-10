/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include <QtTest>
#include "ThemeManager.hpp"
#include "AppSettings.hpp"

class TestThemeManager : public QObject {
    Q_OBJECT

private slots:
    void testDefaultThemeIsSolarized8();
    void testAvailableThemes();
    void testSetTheme();
    void testAccessColorsPerTheme();
    void testColorBlindMode();
    void testAllColorBlindModes();
    void testHighContrastThemes();
    void testPaletteAndStyleSheetGeneration();
    void testAppSettingsConfigFile();
    void testEnsureWindowOnScreen();
    void testJsonSerializationAndDeserialization();
    void testLoadCustomThemeFromJson();
    void testCustomThemesDirScanning();
    void testThemeOverriding();
    void testThemeEdgeCasesAndCoverage();
    void cleanupTestCase();
};

void TestThemeManager::testDefaultThemeIsSolarized8()
{
    ThemeManager &tm = ThemeManager::instance();
    // Default is Solarized 8 (Dark)
    tm.setTheme("solarized8");
    QCOMPARE(tm.currentThemeId(), QString("solarized8"));
    QCOMPARE(tm.currentThemeName(), QString("Solarized 8 (Dark)"));
    QVERIFY(tm.currentTheme().isDark);
    QCOMPARE(tm.currentTheme().windowBg, QColor("#002b36"));
}

void TestThemeManager::testAvailableThemes()
{
    ThemeManager &tm = ThemeManager::instance();
    QStringList ids = tm.themeIds();
    QStringList names = tm.themeNames();

    QVERIFY(ids.contains("solarized8"));
    QVERIFY(ids.contains("solarized8_light"));
    QVERIFY(ids.contains("nord"));
    QVERIFY(ids.contains("dracula"));
    QVERIFY(ids.contains("monokai"));
    QVERIFY(ids.contains("classic"));
    QVERIFY(ids.contains("high_contrast_dark"));
    QVERIFY(ids.contains("high_contrast_light"));

    QCOMPARE(ids.size(), 8);
    QCOMPARE(names.size(), 8);
}

void TestThemeManager::testSetTheme()
{
    ThemeManager &tm = ThemeManager::instance();

    QVERIFY(tm.setTheme("nord"));
    QCOMPARE(tm.currentThemeId(), QString("nord"));
    QCOMPARE(tm.currentTheme().windowBg, QColor("#2e3440"));

    QVERIFY(tm.setTheme("dracula"));
    QCOMPARE(tm.currentThemeId(), QString("dracula"));
    QCOMPARE(tm.currentTheme().windowBg, QColor("#282a36"));

    QVERIFY(tm.setTheme("monokai"));
    QCOMPARE(tm.currentThemeId(), QString("monokai"));
    QCOMPARE(tm.currentTheme().windowBg, QColor("#272822"));

    QVERIFY(tm.setTheme("solarized8_light"));
    QCOMPARE(tm.currentThemeId(), QString("solarized8_light"));
    QVERIFY(!tm.currentTheme().isDark);
    QCOMPARE(tm.currentTheme().windowBg, QColor("#fdf6e3"));

    QVERIFY(tm.setTheme("classic"));
    QCOMPARE(tm.currentThemeId(), QString("classic"));
    QVERIFY(!tm.currentTheme().isDark);

    // Aliases
    QVERIFY(tm.setTheme("solarized_dark"));
    QCOMPARE(tm.currentThemeId(), QString("solarized8"));

    QVERIFY(tm.setTheme("solarized_light"));
    QCOMPARE(tm.currentThemeId(), QString("solarized8_light"));

    // Invalid theme
    QVERIFY(!tm.setTheme("non_existent_theme_123"));

    // Reset to default
    tm.setTheme("solarized8");
}

void TestThemeManager::testAccessColorsPerTheme()
{
    ThemeManager &tm = ThemeManager::instance();

    // In Solarized 8 (Dark)
    tm.setTheme("solarized8");
    AccessColors cRw = tm.getAccessColors("RW", false);
    QCOMPARE(cRw.border, QColor("#859900")); // Solarized Green

    AccessColors cRo = tm.getAccessColors("RO", false);
    QCOMPARE(cRo.border, QColor("#268bd2")); // Solarized Blue

    AccessColors cWo = tm.getAccessColors("WO", false);
    QCOMPARE(cWo.border, QColor("#cb4b16")); // Solarized Orange

    AccessColors cW1c = tm.getAccessColors("W1C", false);
    QCOMPARE(cW1c.border, QColor("#b58900")); // Solarized Yellow

    AccessColors cRc = tm.getAccessColors("RC", false);
    QCOMPARE(cRc.border, QColor("#6c71c4")); // Solarized Violet

    // In Nord
    tm.setTheme("nord");
    AccessColors cNordRw = tm.getAccessColors("RW", false);
    QCOMPARE(cNordRw.border, QColor("#a3be8c")); // Nord Aurora Green

    AccessColors cNordRo = tm.getAccessColors("RO", false);
    QCOMPARE(cNordRo.border, QColor("#88c0d0")); // Nord Frost Blue

    // In Dracula
    tm.setTheme("dracula");
    AccessColors cDracRw = tm.getAccessColors("RW", false);
    QCOMPARE(cDracRw.border, QColor("#50fa7b")); // Dracula Green

    AccessColors cDracRo = tm.getAccessColors("RO", false);
    QCOMPARE(cDracRo.border, QColor("#8be9fd")); // Dracula Cyan

    // In Monokai
    tm.setTheme("monokai");
    AccessColors cMonoRw = tm.getAccessColors("RW", false);
    QCOMPARE(cMonoRw.border, QColor("#a6e22e")); // Monokai Green

    AccessColors cMonoRo = tm.getAccessColors("RO", false);
    QCOMPARE(cMonoRo.border, QColor("#66d9ef")); // Monokai Cyan

    // Reset to default
    tm.setTheme("solarized8");

    // Test ColorScheme boolean getAccessColors and accessColors inline helpers
    AccessColors acTrue = tm.currentTheme().getAccessColors("RW", true);
    AccessColors acFalse = tm.currentTheme().getAccessColors("RW", false);
    QVERIFY(acTrue.bg.isValid());
    QVERIFY(acFalse.bg.isValid());
    AccessColors acAliasMode = tm.currentTheme().accessColors("RW", ColorBlindMode::None);
    AccessColors acAliasBool = tm.currentTheme().accessColors("RW", true);
    QVERIFY(acAliasMode.bg.isValid());
    QVERIFY(acAliasBool.bg.isValid());
    AccessColors tmAliasMode = tm.accessColors("RW", ColorBlindMode::None);
    AccessColors tmAliasBool = tm.accessColors("RW", true);
    QVERIFY(tmAliasMode.bg.isValid());
    QVERIFY(tmAliasBool.bg.isValid());
}

void TestThemeManager::testColorBlindMode()
{
    ThemeManager &tm = ThemeManager::instance();

    // Color blind mode should return high-contrast CVD colors irrespective of theme
    tm.setTheme("solarized8");
    AccessColors cbRw1 = tm.getAccessColors("RW", true);
    QCOMPARE(cbRw1.border, QColor(0, 96, 100)); // Teal

    tm.setTheme("nord");
    AccessColors cbRw2 = tm.getAccessColors("RW", true);
    QCOMPARE(cbRw2.border, QColor(0, 96, 100)); // Same high contrast Teal

    tm.setTheme("solarized8");
}

void TestThemeManager::testAllColorBlindModes()
{
    ThemeManager &tm = ThemeManager::instance();

    // 1. Verify availableColorBlindModes()
    const auto &modes = availableColorBlindModes();
    QCOMPARE(modes.size(), 5);
    QCOMPARE(modes[0].id, QString("universal"));
    QCOMPARE(modes[1].id, QString("deuteranopia"));
    QCOMPARE(modes[2].id, QString("protanopia"));
    QCOMPARE(modes[3].id, QString("tritanopia"));
    QCOMPARE(modes[4].id, QString("achromatopsia"));

    // 2. String conversion helpers round-trip
    QCOMPARE(colorBlindModeToString(ColorBlindMode::Universal), QString("universal"));
    QCOMPARE(colorBlindModeToString(ColorBlindMode::Deuteranopia), QString("deuteranopia"));
    QCOMPARE(colorBlindModeToString(ColorBlindMode::Protanopia), QString("protanopia"));
    QCOMPARE(colorBlindModeToString(ColorBlindMode::Tritanopia), QString("tritanopia"));
    QCOMPARE(colorBlindModeToString(ColorBlindMode::Achromatopsia), QString("achromatopsia"));
    QCOMPARE(colorBlindModeToString(ColorBlindMode::None), QString("none"));

    QCOMPARE(stringToColorBlindMode("universal"), ColorBlindMode::Universal);
    QCOMPARE(stringToColorBlindMode("deuteranopia"), ColorBlindMode::Deuteranopia);
    QCOMPARE(stringToColorBlindMode("protanopia"), ColorBlindMode::Protanopia);
    QCOMPARE(stringToColorBlindMode("tritanopia"), ColorBlindMode::Tritanopia);
    QCOMPARE(stringToColorBlindMode("achromatopsia"), ColorBlindMode::Achromatopsia);
    QCOMPARE(stringToColorBlindMode("none"), ColorBlindMode::None);

    // 3. Test each CVD mode palette covers RW, RO, WO, W1C, W1S, W0C, RC, RS, NA
    const QStringList policies = {"RW", "RO", "WO", "W1C", "W1S", "W0C", "RC", "RS", "NA"};

    for (const auto &info : modes) {
        for (const QString &pol : policies) {
            AccessColors ac = tm.getAccessColors(pol, info.mode);
            QVERIFY(ac.bg.isValid());
            QVERIFY(ac.border.isValid());
            QVERIFY(ac.text.isValid());
        }
    }

    // 4. Test distinct colors for Deuteranopia
    AccessColors deutRw = tm.getAccessColors("RW", ColorBlindMode::Deuteranopia);
    AccessColors deutRo = tm.getAccessColors("RO", ColorBlindMode::Deuteranopia);
    AccessColors deutWo = tm.getAccessColors("WO", ColorBlindMode::Deuteranopia);
    QVERIFY(deutRw.bg != deutRo.bg);
    QVERIFY(deutRw.bg != deutWo.bg);
    QCOMPARE(deutRw.bg, QColor(153, 246, 228)); // Teal

    // 5. Test distinct colors for Protanopia
    AccessColors protRw = tm.getAccessColors("RW", ColorBlindMode::Protanopia);
    AccessColors protRo = tm.getAccessColors("RO", ColorBlindMode::Protanopia);
    AccessColors protWo = tm.getAccessColors("WO", ColorBlindMode::Protanopia);
    QVERIFY(protRw.bg != protRo.bg);
    QVERIFY(protRw.bg != protWo.bg);
    QCOMPARE(protRw.bg, QColor(147, 197, 253)); // Sky Blue

    // 6. Test distinct colors for Tritanopia
    AccessColors tritRw = tm.getAccessColors("RW", ColorBlindMode::Tritanopia);
    AccessColors tritRo = tm.getAccessColors("RO", ColorBlindMode::Tritanopia);
    AccessColors tritWo = tm.getAccessColors("WO", ColorBlindMode::Tritanopia);
    QVERIFY(tritRw.bg != tritRo.bg);
    QVERIFY(tritRw.bg != tritWo.bg);
    QCOMPARE(tritRw.bg, QColor(204, 251, 241)); // Mint Cyan

    // 7. Test Achromatopsia (Luminance steps)
    AccessColors achrRo = tm.getAccessColors("RO", ColorBlindMode::Achromatopsia); // 100% white
    AccessColors achrRw = tm.getAccessColors("RW", ColorBlindMode::Achromatopsia); // 88%
    AccessColors achrWo = tm.getAccessColors("WO", ColorBlindMode::Achromatopsia); // 13% dark
    QCOMPARE(achrRo.bg, QColor(255, 255, 255));
    QCOMPARE(achrRw.bg, QColor(224, 224, 224));
    QCOMPARE(achrWo.bg, QColor(34, 34, 34));

    // 8. ThemeManager active mode manipulation
    QSignalSpy spy(&tm, &ThemeManager::colorBlindModeChanged);
    tm.setColorBlindMode(ColorBlindMode::Deuteranopia);
    QCOMPARE(tm.colorBlindMode(), ColorBlindMode::Deuteranopia);
    QCOMPARE(tm.colorBlindModeId(), QString("deuteranopia"));
    QCOMPARE(spy.count(), 1);

    tm.setColorBlindMode(QString("achromatopsia"));
    QCOMPARE(tm.colorBlindMode(), ColorBlindMode::Achromatopsia);
    QCOMPARE(spy.count(), 2);

    tm.setColorBlindMode(false);
    QCOMPARE(tm.colorBlindMode(), ColorBlindMode::None);
    QCOMPARE(spy.count(), 3);
}

void TestThemeManager::testHighContrastThemes()
{
    ThemeManager &tm = ThemeManager::instance();

    // High Contrast Dark
    QVERIFY(tm.setTheme("high_contrast_dark"));
    QCOMPARE(tm.currentThemeId(), QString("high_contrast_dark"));
    const ColorScheme &darkHc = tm.currentTheme();
    QVERIFY(darkHc.isDark);
    QCOMPARE(darkHc.windowBg, QColor("#000000")); // Pure Black
    QCOMPARE(darkHc.textColor, QColor("#ffffff")); // Pure White Text
    QCOMPARE(darkHc.border, QColor("#ffffff")); // White Border

    // High Contrast Light
    QVERIFY(tm.setTheme("high_contrast_light"));
    QCOMPARE(tm.currentThemeId(), QString("high_contrast_light"));
    const ColorScheme &lightHc = tm.currentTheme();
    QVERIFY(!lightHc.isDark);
    QCOMPARE(lightHc.windowBg, QColor("#ffffff")); // Pure White
    QCOMPARE(lightHc.textColor, QColor("#000000")); // Pure Black Text
    QCOMPARE(lightHc.border, QColor("#000000")); // Black Border

    // Reset back to solarized8
    tm.setTheme("solarized8");
}

void TestThemeManager::testPaletteAndStyleSheetGeneration()
{
    ThemeManager &tm = ThemeManager::instance();
    tm.setTheme("solarized8");

    const ColorScheme &s = tm.currentTheme();
    QPalette pal = s.generatePalette();
    QCOMPARE(pal.color(QPalette::Window), s.windowBg);
    QCOMPARE(pal.color(QPalette::Base), s.panelBg);
    QCOMPARE(pal.color(QPalette::Text), s.textColor);

    QString qss = s.generateStyleSheet();
    QVERIFY(!qss.isEmpty());
    QVERIFY(qss.contains("QMainWindow"));
    QVERIFY(qss.contains("QTreeView"));
    QVERIFY(qss.contains("QHeaderView"));
    QVERIFY(qss.contains(s.windowBg.name()));
}

void TestThemeManager::testAppSettingsConfigFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString testConfPath = tempDir.path() + "/test_rmap.conf";

    AppSettings &settings = AppSettings::instance();
    QString origPath = settings.configFilePath();
    settings.setConfigFilePath(testConfPath);

    QCOMPARE(settings.configFilePath(), testConfPath);

    settings.setColorScheme("nord");
    settings.setColorBlindMode(true);
    settings.setMainWindowPos(QPoint(150, 120));
    settings.setMainWindowSize(QSize(1400, 900));
    settings.setConfigWindowPos(QPoint(200, 180));
    settings.setConfigWindowSize(QSize(800, 600));
    settings.setWindowPos("HelpDialog", QPoint(250, 220));
    settings.setWindowSize("HelpDialog", QSize(500, 400));
    settings.save();

    // Verify file was written
    QFileInfo fi(testConfPath);
    QVERIFY(fi.exists());

    // Verify loading state
    settings.load();
    QCOMPARE(settings.colorScheme(), QString("nord"));
    QCOMPARE(settings.colorBlindMode(), true);
    QCOMPARE(settings.mainWindowPos(), QPoint(150, 120));
    QCOMPARE(settings.mainWindowSize(), QSize(1400, 900));
    QCOMPARE(settings.configWindowPos(), QPoint(200, 180));
    QCOMPARE(settings.configWindowSize(), QSize(800, 600));
    QCOMPARE(settings.windowPos("HelpDialog"), QPoint(250, 220));
    QCOMPARE(settings.windowSize("HelpDialog"), QSize(500, 400));

    settings.setColorBlindTypeString("protanopia");
    QCOMPARE(settings.colorBlindTypeString(), QString("protanopia"));
    QCOMPARE(settings.colorBlindType(), ColorBlindMode::Protanopia);
    settings.setColorBlindTypeString("none");
    QCOMPARE(settings.colorBlindTypeString(), QString("none"));

    // Test determineConfigPath when all inputs are empty (triggers line 32)
    QString defPath = AppSettings::determineConfigPath(nullptr, nullptr, QString());
    QCOMPARE(defPath, QDir::homePath() + "/.config/rmap/rmap.conf");

    // Test determineConfigPath when envConfig is set (triggers line 31)
    QString customEnvPath = AppSettings::determineConfigPath("/custom/env/rmap.conf", nullptr, QString());
    QCOMPARE(customEnvPath, QString("/custom/env/rmap.conf"));

    // Test saving to nested non-existent directory (triggers line 410)
    QString nestedPath = tempDir.path() + "/nested_dir_123/rmap.conf";
    settings.setConfigFilePath(nestedPath);
    settings.save();
    QVERIFY(QFileInfo::exists(nestedPath));

    // Test loading empty config values (triggers lines 371, 377, 381)
    {
        QSettings emptyVals(testConfPath, QSettings::IniFormat);
        emptyVals.setValue("Appearance/ColorScheme", "");
        emptyVals.setValue("Appearance/ColorBlindType", "none");
        emptyVals.setValue("Appearance/Language", "");
        emptyVals.sync();
    }
    settings.setConfigFilePath(testConfPath);
    settings.load();
    QCOMPARE(settings.colorScheme(), QString("solarized8"));
    QCOMPARE(settings.colorBlindType(), ColorBlindMode::Universal);
    QCOMPARE(settings.language(), QString("en"));

    // Reset back to defaults and restore original path
    settings.setColorScheme("solarized8");
    settings.setColorBlindMode(false);
    settings.setConfigFilePath(origPath);
}

void TestThemeManager::testEnsureWindowOnScreen()
{
    QWidget widget;

    // Test 1: Window size too small is enlarged
    widget.resize(50, 30);
    AppSettings::ensureWindowOnScreen(&widget, QSize(400, 300), QSize(600, 400));
    QVERIFY(widget.width() >= 400);
    QVERIFY(widget.height() >= 300);

    // Test 2: Window moved far offscreen (negative coordinates) is brought back
    widget.move(-5000, -5000);
    AppSettings::ensureWindowOnScreen(&widget, QSize(400, 300), QSize(600, 400));
    QVERIFY(widget.x() >= 0);
    QVERIFY(widget.y() >= 0);

    // Test 3: Window moved far offscreen (large positive coordinates) is brought back
    widget.move(10000, 10000);
    AppSettings::ensureWindowOnScreen(&widget, QSize(400, 300), QSize(600, 400));
    QScreen *primary = QGuiApplication::primaryScreen();
    if (primary) {
        QRect avail = primary->availableGeometry();
        QVERIFY(widget.x() + widget.width() <= avail.right() + 1);
        QVERIFY(widget.y() + widget.height() <= avail.bottom() + 1);
    }

    // Test 4: Window wider and taller than screen (triggers lines 340 and 343)
    widget.resize(20000, 20000);
    AppSettings::ensureWindowOnScreen(&widget, QSize(400, 300), QSize(600, 400));
    if (primary) {
        QVERIFY(widget.width() <= primary->availableGeometry().width());
        QVERIFY(widget.height() <= primary->availableGeometry().height());
    }

    // Test 5: Screen fallback mode 1 (screens.first() fallback - triggers line 330)
    widget.move(-5000, -5000);
    AppSettings::setScreenOverrideMode(1);
    AppSettings::ensureWindowOnScreen(&widget, QSize(400, 300), QSize(600, 400));
    AppSettings::setScreenOverrideMode(0);

    // Test 6: Screen fallback mode 2 (null screen fallback - triggers line 367)
    widget.move(-5000, -5000);
    AppSettings::setScreenOverrideMode(2);
    AppSettings::ensureWindowOnScreen(&widget, QSize(400, 300), QSize(600, 400));
    AppSettings::setScreenOverrideMode(0);
}

void TestThemeManager::testJsonSerializationAndDeserialization()
{
    ThemeManager &tm = ThemeManager::instance();
    tm.setTheme("solarized8");
    const ColorScheme &orig = tm.currentTheme();

    QJsonObject json = orig.toJson();
    QCOMPARE(json["id"].toString(), QString("solarized8"));
    QCOMPARE(json["isDark"].toBool(), true);
    QVERIFY(json.contains("base"));
    QVERIFY(json.contains("reserved"));
    QVERIFY(json.contains("accessPolicies"));

    ColorScheme parsed;
    QVERIFY(parsed.fromJson(json));
    QCOMPARE(parsed.id, orig.id);
    QCOMPARE(parsed.name, orig.name);
    QCOMPARE(parsed.isDark, orig.isDark);
    QCOMPARE(parsed.windowBg, orig.windowBg);
    QCOMPARE(parsed.panelBg, orig.panelBg);
    QCOMPARE(parsed.rsvdBg, orig.rsvdBg);
    QCOMPARE(parsed.rwColors.border, orig.rwColors.border);
    QCOMPARE(parsed.roColors.border, orig.roColors.border);
}

void TestThemeManager::testLoadCustomThemeFromJson()
{
    ThemeManager &tm = ThemeManager::instance();
    QString customJson = QString::fromUtf8(R"json({
        "id": "cyberpunk_neon",
        "name": "Cyberpunk Neon",
        "isDark": true,
        "base": {
            "windowBg": "#120024",
            "panelBg": "#1a0033",
            "textColor": "#00ffcc",
            "border": "#ff007f"
        },
        "reserved": {
            "bg": "#2b004a",
            "text": "#ff77a9"
        },
        "accessPolicies": {
            "rw": { "bg": "#003322", "border": "#00ff99", "text": "#00ff99" },
            "ro": { "bg": "#002233", "border": "#00ccff", "text": "#00ccff" }
        }
    })json");

    QVERIFY(tm.loadThemeFromJson(customJson));
    QVERIFY(tm.themeIds().contains("cyberpunk_neon"));

    QVERIFY(tm.setTheme("cyberpunk_neon"));
    QCOMPARE(tm.currentThemeId(), QString("cyberpunk_neon"));
    QCOMPARE(tm.currentTheme().windowBg, QColor("#120024"));
    QCOMPARE(tm.currentTheme().border, QColor("#ff007f"));
    QCOMPARE(tm.getAccessColors("RW", false).border, QColor("#00ff99"));

    // Reset back
    tm.setTheme("solarized8");
}

void TestThemeManager::testCustomThemesDirScanning()
{
    ThemeManager &tm = ThemeManager::instance();
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    // Exercise addSearchPath with an empty dir so resetToDefaults does not rescan synthwave
    QString emptySearchPath = tempDir.filePath("empty_search_dir");
    QDir().mkpath(emptySearchPath);
    tm.addSearchPath(emptySearchPath);
    tm.addSearchPath(emptySearchPath); // duplicate check
    tm.addSearchPath(""); // empty check

    // Write synthwave.json
    QString synthwaveJson = QString::fromUtf8(R"json({
        "id": "synthwave_80s",
        "name": "Synthwave 80s",
        "isDark": true,
        "base": {
            "windowBg": "#241734",
            "panelBg": "#2e1f42"
        }
    })json");
    QFile f(tempDir.filePath("synthwave.json"));
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(synthwaveJson.toUtf8());
    f.close();

    // Also write a template.json file which must be skipped
    QString templateJson = QString::fromUtf8(R"json({
        "id": "my_template",
        "name": "My Template",
        "isDark": true
    })json");
    QFile fTpl(tempDir.filePath("template.json"));
    QVERIFY(fTpl.open(QIODevice::WriteOnly));
    fTpl.write(templateJson.toUtf8());
    fTpl.close();

    int loaded = tm.scanThemesDir(tempDir.path());
    QCOMPARE(loaded, 1);
    QVERIFY(tm.themeIds().contains("synthwave_80s"));
    QVERIFY(!tm.themeIds().contains("my_template"));

    // Can set theme
    QVERIFY(tm.setTheme("synthwave_80s"));
    QCOMPARE(tm.currentThemeId(), QString("synthwave_80s"));
    QCOMPARE(tm.currentTheme().windowBg, QColor("#241734"));

    // Reset
    tm.resetToDefaults();
    QVERIFY(!tm.themeIds().contains("synthwave_80s"));
}

void TestThemeManager::testThemeOverriding()
{
    ThemeManager &tm = ThemeManager::instance();
    tm.setTheme("solarized8");
    QCOMPARE(tm.currentTheme().windowBg, QColor("#002b36"));

    QString overrideJson = QString::fromUtf8(R"json({
        "id": "solarized8",
        "name": "Solarized 8 (Dark)",
        "isDark": true,
        "base": {
            "windowBg": "#112233"
        }
    })json");

    int origCount = tm.availableThemes().size();
    QVERIFY(tm.loadThemeFromJson(overrideJson));
    QCOMPARE(tm.availableThemes().size(), origCount); // Not duplicated
    QCOMPARE(tm.currentTheme().windowBg, QColor("#112233"));

    // Reset to defaults
    tm.resetToDefaults();
    QCOMPARE(tm.currentTheme().windowBg, QColor("#002b36"));
}

void TestThemeManager::testThemeEdgeCasesAndCoverage()
{
    // 1. ColorScheme accessPolicyColors without custom color blind overrides
    ColorScheme s = ColorScheme::createDefault("solarized8");
    s.hasCustomColorBlind = false;

    // Normal mode unknown access policy
    AccessColors na = s.getAccessColors("UNKNOWN_POLICY", ColorBlindMode::None);
    QVERIFY(na.bg.isValid());

    // Universal mode access policies
    QVERIFY(s.getAccessColors("RW", ColorBlindMode::Universal).bg.isValid());
    QVERIFY(s.getAccessColors("RO", ColorBlindMode::Universal).bg.isValid());
    QVERIFY(s.getAccessColors("WO", ColorBlindMode::Universal).bg.isValid());
    QVERIFY(s.getAccessColors("W1S", ColorBlindMode::Universal).bg.isValid());
    QVERIFY(s.getAccessColors("W0S", ColorBlindMode::Universal).bg.isValid());
    QVERIFY(s.getAccessColors("WS", ColorBlindMode::Universal).bg.isValid());
    QVERIFY(s.getAccessColors("W1C", ColorBlindMode::Universal).bg.isValid());
    QVERIFY(s.getAccessColors("W0C", ColorBlindMode::Universal).bg.isValid());
    QVERIFY(s.getAccessColors("WC", ColorBlindMode::Universal).bg.isValid());
    QVERIFY(s.getAccessColors("RC", ColorBlindMode::Universal).bg.isValid());
    QVERIFY(s.getAccessColors("RS", ColorBlindMode::Universal).bg.isValid());
    QVERIFY(s.getAccessColors("UNKNOWN", ColorBlindMode::Universal).bg.isValid());

    // Default mode switch case
    QVERIFY(!s.getAccessColors("RW", static_cast<ColorBlindMode>(999)).bg.isValid());

    // 2. Palette and style sheet generation
    QPalette pal = s.generatePalette();
    QVERIFY(pal.window().color().isValid());
    QString ss = s.generateStyleSheet();
    QVERIFY(!ss.isEmpty());

    // 3. parseColor fallback and fromJson error cases
    QJsonObject badObj;
    badObj["id"] = "bad";
    badObj["windowBg"] = "not-a-valid-color";
    ColorScheme badScheme;
    badScheme.fromJson(badObj);

    QJsonObject emptyObj;
    QVERIFY(!badScheme.fromJson(emptyObj));

    // 4. fromJson with top-level access keys
    QJsonObject topObj;
    topObj["id"] = "top_level_theme";
    topObj["name"] = "Top Level Theme";
    QJsonObject rwObj;
    rwObj["bg"] = "#112233";
    rwObj["border"] = "#445566";
    rwObj["text"] = "#778899";
    topObj["rw"] = rwObj;
    ColorScheme topScheme;
    QVERIFY(topScheme.fromJson(topObj));

    topScheme.hasCustomColorBlind = true;
    QJsonObject topJson = topScheme.toJson();
    QVERIFY(!topJson.isEmpty());

    // 5. builtInDefaults
    QVERIFY(!ColorScheme::builtInDefaults().isEmpty());

    // 6. ThemeManager themeIds, themeNames, and out-of-bounds currentTheme
    ThemeManager &tm = ThemeManager::instance();
    QVERIFY(!tm.themeIds().isEmpty());
    QVERIFY(!tm.themeNames().isEmpty());

    int savedIdx = tm.m_currentIndex;
    tm.m_currentIndex = -1;
    QCOMPARE(tm.currentTheme().id, tm.m_themes.first().id);
    tm.m_currentIndex = savedIdx;

    // 7. setTheme with direct JSON file path
    QTemporaryDir tdir;
    QVERIFY(tdir.isValid());
    QString tjPath = tdir.filePath("direct_theme.json");
    QFile f(tjPath);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(QJsonDocument(topJson).toJson());
    f.close();
    QVERIFY(tm.setTheme(tjPath));

    // 8. registerTheme with empty id
    ColorScheme emptyIdScheme;
    QVERIFY(!tm.registerTheme(emptyIdScheme));

    // 9. loadThemeFromJson error cases
    QVERIFY(!tm.loadThemeFromJson("nonexistent_file_xyz_123.json"));
    QString unreadablePath = tdir.filePath("unreadable.json");
    {
        QFile uf(unreadablePath);
        QVERIFY(uf.open(QIODevice::WriteOnly));
        uf.write("{}");
        uf.close();
        uf.setPermissions(QFileDevice::WriteOwner); // remove read permission
    }
    QVERIFY(!tm.loadThemeFromJson(unreadablePath));
    QVERIFY(!tm.loadThemeFromJson("bad json {"));
    QVERIFY(!tm.loadThemeFromJson("{}"));

    // 10. RMAP_THEMES_PATH environment variable
    qputenv("RMAP_THEMES_PATH", "/tmp/custom_theme_search_123");
    QStringList sPaths = tm.searchPaths();
    QVERIFY(sPaths.contains("/tmp/custom_theme_search_123"));
    qunsetenv("RMAP_THEMES_PATH");

    // 11. System themes override
    ThemeManager::setSystemThemePathsOverride(true);
    QVERIFY(ThemeManager::systemThemePathsOverride());
    sPaths = tm.searchPaths();
    QVERIFY(sPaths.contains("/usr/local/share/rmap/themes"));
    ThemeManager::setSystemThemePathsOverride(false);
    QVERIFY(!ThemeManager::systemThemePathsOverride());

    // 12. scanThemes coverage
    tm.scanThemes();
}

void TestThemeManager::cleanupTestCase()
{
    ThemeManager::instance().resetToDefaults();
    ThemeManager::instance().setTheme("solarized8");
}

QTEST_MAIN(TestThemeManager)
#include "test_ThemeManager.moc"
