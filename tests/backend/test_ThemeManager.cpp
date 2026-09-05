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
    void testPaletteAndStyleSheetGeneration();
    void testAppSettingsConfigFile();
    void testEnsureWindowOnScreen();
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

    QCOMPARE(ids.size(), 6);
    QCOMPARE(names.size(), 6);
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
}

QTEST_MAIN(TestThemeManager)
#include "test_ThemeManager.moc"
