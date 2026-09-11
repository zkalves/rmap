/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include <QtTest>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include "PreferencesWindow.hpp"
#include "ThemeManager.hpp"
#include "LanguageManager.hpp"
#include "AppSettings.hpp"

static QtMessageHandler s_originalHandler = nullptr;
static void testOffscreenMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (type == QtWarningMsg && msg.contains("This plugin does not support")) {
        return;
    }
    if (s_originalHandler) {
        s_originalHandler(type, context, msg);
    }
}

class TestPreferencesWindow : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        s_originalHandler = qInstallMessageHandler(testOffscreenMessageHandler);
        QDir("work").removeRecursively();
        QDir().mkpath("work");
    }
    void cleanupTestCase() {
        QDir("work").removeRecursively();
        qInstallMessageHandler(s_originalHandler);
    }

    void testPreferencesDefaults();
    void testColourSchemeChange();
    void testColourBlindModeToggle();
    void testApplyAndReject();
    void testWindowSizePersistence();
    void testLanguageSelection();
    void testThemesFolderButton();
    void testAllThemesCycling();
    void testColorBlindExtended();
    void testLanguageExtended();
    void testOnOpenThemesFolder();
    void testLanguageChangeEvent();
    void testMoveAndResizeEvents();
    void testRestoreWindowStateVariations();
    void testShowEventAndApplyButton();
};

void TestPreferencesWindow::testPreferencesDefaults()
{
    PreferencesWindow prefWin;
    auto *combo = prefWin.findChild<QComboBox*>("colourSchemeCombo");
    auto *cb = prefWin.findChild<QCheckBox*>("colourBlindMode");
    auto *cbCombo = prefWin.findChild<QComboBox*>("colourBlindCombo");

    QVERIFY(combo != nullptr);
    QVERIFY(cb != nullptr);
    QVERIFY(cbCombo != nullptr);
    QVERIFY(combo->count() >= 8);
    QCOMPARE(cbCombo->count(), 5);

    QVERIFY(!prefWin.isModal());
    QVERIFY(!prefWin.isSizeGripEnabled());
    QVERIFY(prefWin.windowFlags().testFlag(Qt::Window));
    QVERIFY(prefWin.windowFlags().testFlag(Qt::WindowMinMaxButtonsHint));
    QVERIFY(prefWin.windowFlags().testFlag(Qt::WindowCloseButtonHint));
}

void TestPreferencesWindow::testColourSchemeChange()
{
    PreferencesWindow prefWin;
    prefWin.setColourScheme("nord");
    QCOMPARE(prefWin.colourScheme(), QString("nord"));

    prefWin.accept();
    QCOMPARE(AppSettings::instance().colourScheme(), QString("nord"));
    QCOMPARE(ThemeManager::instance().currentThemeId(), QString("nord"));

    // Reset back
    prefWin.setColourScheme("solarized8");
    prefWin.accept();
    QCOMPARE(AppSettings::instance().colourScheme(), QString("solarized8"));
}

void TestPreferencesWindow::testColourBlindModeToggle()
{
    PreferencesWindow prefWin;
    auto *cbCombo = prefWin.findChild<QComboBox*>("colourBlindCombo");
    QVERIFY(cbCombo != nullptr);

    prefWin.setColourBlindMode(true);
    QCOMPARE(prefWin.isColourBlindMode(), true);
    QVERIFY(cbCombo->isEnabled());

    prefWin.setColourBlindType(ColorBlindMode::Deuteranopia);
    QCOMPARE(prefWin.colourBlindType(), ColorBlindMode::Deuteranopia);

    prefWin.accept();
    QCOMPARE(AppSettings::instance().colourBlindMode(), true);
    QCOMPARE(AppSettings::instance().colorBlindType(), ColorBlindMode::Deuteranopia);
    QCOMPARE(ThemeManager::instance().colorBlindMode(), ColorBlindMode::Deuteranopia);

    prefWin.setColourBlindMode(false);
    QCOMPARE(prefWin.isColourBlindMode(), false);
    QVERIFY(!cbCombo->isEnabled());
    prefWin.accept();
    QCOMPARE(AppSettings::instance().colourBlindMode(), false);
    QCOMPARE(ThemeManager::instance().colorBlindMode(), ColorBlindMode::None);
}

void TestPreferencesWindow::testApplyAndReject()
{
    PreferencesWindow prefWin;
    prefWin.setColourScheme("dracula");
    prefWin.apply();
    QCOMPARE(ThemeManager::instance().currentThemeId(), QString("dracula"));

    // Change combo without apply, then reject
    auto *combo = prefWin.findChild<QComboBox*>("colourSchemeCombo");
    int monokaiIdx = combo->findData("monokai");
    if (monokaiIdx >= 0) {
        combo->setCurrentIndex(monokaiIdx);
    }
    prefWin.reject();

    // After reject, current combo reverts to dracula
    QCOMPARE(prefWin.colourScheme(), QString("dracula"));

    // Reset back to solarized8
    prefWin.setColourScheme("solarized8");
    prefWin.apply();
}

void TestPreferencesWindow::testWindowSizePersistence()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString origPath = AppSettings::instance().configFilePath();
    AppSettings::instance().setConfigFilePath(tempDir.path() + "/test_rmap.conf");

    {
        PreferencesWindow prefWin;
        prefWin.resize(550, 320);
        prefWin.saveWindowStateToSettings();
        QCOMPARE(AppSettings::instance().windowSize("PreferencesWindow"), QSize(550, 320));
    }

    {
        PreferencesWindow prefWin2;
        QCOMPARE(AppSettings::instance().windowSize("PreferencesWindow"), QSize(550, 320));
        QCOMPARE(prefWin2.size(), QSize(550, 320));
    }

    AppSettings::instance().setConfigFilePath(origPath);
}

void TestPreferencesWindow::testLanguageSelection()
{
    PreferencesWindow prefWin;
    auto *langCombo = prefWin.findChild<QComboBox*>("languageCombo");
    QVERIFY(langCombo != nullptr);
    QVERIFY(langCombo->count() >= 7);

    // Change to Spanish
    prefWin.setLanguage("es");
    QCOMPARE(prefWin.language(), QString("es"));

    prefWin.accept();
    QCOMPARE(AppSettings::instance().language(), QString("es"));
    QCOMPARE(LanguageManager::instance().currentLanguage(), QString("es"));

    // Reset back to English
    prefWin.setLanguage("en");
    prefWin.accept();
    QCOMPARE(AppSettings::instance().language(), QString("en"));
    QCOMPARE(LanguageManager::instance().currentLanguage(), QString("en"));
}

void TestPreferencesWindow::testThemesFolderButton()
{
    PreferencesWindow prefWin;
    auto *btn = prefWin.findChild<QPushButton*>("btnOpenThemesFolder");
    QVERIFY(btn != nullptr);
    QVERIFY(!btn->text().isEmpty());

    QString themesDir = ThemeManager::userThemesDir();
    QVERIFY(!themesDir.isEmpty());
    QVERIFY(themesDir.endsWith("themes"));
}

void TestPreferencesWindow::testAllThemesCycling()
{
    PreferencesWindow prefWin;
    const auto themes = ThemeManager::instance().availableThemes();
    for (const auto &t : themes) {
        prefWin.setColourScheme(t.id);
        QCOMPARE(prefWin.colourScheme(), t.id);
        QCOMPARE(prefWin.colorScheme(), t.id);
        prefWin.apply();
        QCOMPARE(ThemeManager::instance().currentThemeId(), t.id);
    }

    // Setting empty scheme should default to solarized8
    prefWin.setColourScheme("");
    QCOMPARE(prefWin.colourScheme(), QString("solarized8"));
    prefWin.apply();
    QCOMPARE(ThemeManager::instance().currentThemeId(), QString("solarized8"));
}

void TestPreferencesWindow::testOnOpenThemesFolder()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString origPath = AppSettings::instance().configFilePath();
    AppSettings::instance().setConfigFilePath(tempDir.path() + "/conf/rmap.conf");

    PreferencesWindow prefWin;
    QString themesDir = ThemeManager::userThemesDir();
    QVERIFY(!QDir(themesDir).exists());

    prefWin.onOpenThemesFolder();
    QVERIFY(QDir(themesDir).exists());
    QVERIFY(QFile::exists(QDir(themesDir).filePath("template.json")));

    AppSettings::instance().setConfigFilePath(origPath);
}

void TestPreferencesWindow::testLanguageExtended()
{
    PreferencesWindow prefWin;
    prefWin.setLanguage("");
    QCOMPARE(prefWin.language(), QString("en"));

    prefWin.setLanguage("de");
    QCOMPARE(prefWin.language(), QString("de"));
}

void TestPreferencesWindow::testColorBlindExtended()
{
    PreferencesWindow prefWin;

    // Setting ColorBlindMode::None should fallback to Universal
    prefWin.setColourBlindType(ColorBlindMode::None);
    QCOMPARE(prefWin.colourBlindType(), ColorBlindMode::Universal);

    // Test alias methods
    prefWin.setColorBlindType(ColorBlindMode::Tritanopia);
    QCOMPARE(prefWin.colorBlindType(), ColorBlindMode::Tritanopia);
    QCOMPARE(prefWin.colourBlindType(), ColorBlindMode::Tritanopia);

    prefWin.setColourBlindMode(true);
    QCOMPARE(prefWin.colorBlindMode(), true);
    QCOMPARE(prefWin.isColourBlindMode(), true);

    prefWin.setColourBlindMode(false);
    QCOMPARE(prefWin.colorBlindMode(), false);

    // Test empty combo fallback in saveStateFromUi
    auto *cbCombo = prefWin.findChild<QComboBox*>("colourBlindCombo");
    auto *schemeCombo = prefWin.findChild<QComboBox*>("colourSchemeCombo");
    auto *langCombo = prefWin.findChild<QComboBox*>("languageCombo");
    QVERIFY(cbCombo && schemeCombo && langCombo);
    cbCombo->setCurrentIndex(-1);
    schemeCombo->setCurrentIndex(-1);
    langCombo->setCurrentIndex(-1);
    prefWin.accept();
    QCOMPARE(prefWin.colourScheme(), QString("solarized8"));
    QCOMPARE(prefWin.language(), QString("en"));
    QCOMPARE(prefWin.colourBlindType(), ColorBlindMode::Universal);
}

void TestPreferencesWindow::testLanguageChangeEvent()
{
    PreferencesWindow prefWin;
    QEvent langChange(QEvent::LanguageChange);
    QApplication::sendEvent(&prefWin, &langChange);
    QCOMPARE(prefWin.windowTitle(), QString("Preferences"));
}

void TestPreferencesWindow::testMoveAndResizeEvents()
{
    PreferencesWindow prefWin;
    QResizeEvent resizeEv(QSize(520, 300), QSize(500, 280));
    QApplication::sendEvent(&prefWin, &resizeEv);

    QMoveEvent moveEv(QPoint(50, 50), QPoint(0, 0));
    QApplication::sendEvent(&prefWin, &moveEv);

    QCloseEvent closeEv;
    QApplication::sendEvent(&prefWin, &closeEv);
}

void TestPreferencesWindow::testRestoreWindowStateVariations()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString origPath = AppSettings::instance().configFilePath();
    AppSettings::instance().setConfigFilePath(tempDir.path() + "/test_pref_restore.conf");

    // Case 1: Non-empty geometry
    {
        PreferencesWindow prefWin;
        prefWin.saveWindowStateToSettings();
        QVERIFY(!AppSettings::instance().windowGeometry("PreferencesWindow").isEmpty());
    }
    {
        PreferencesWindow prefWin2;
        QVERIFY(prefWin2.width() >= 400);
    }

    // Case 2: Empty geometry, valid size and pos
    AppSettings::instance().setWindowGeometry("PreferencesWindow", QByteArray());
    AppSettings::instance().setWindowSize("PreferencesWindow", QSize(510, 290));
    AppSettings::instance().setWindowPos("PreferencesWindow", QPoint(70, 70));
    {
        PreferencesWindow prefWin3;
        QCOMPARE(prefWin3.size(), QSize(510, 290));
        QCOMPARE(prefWin3.pos(), QPoint(70, 70));
    }

    // Case 3: Empty geometry, invalid size and null pos
    AppSettings::instance().setWindowGeometry("PreferencesWindow", QByteArray());
    AppSettings::instance().setWindowSize("PreferencesWindow", QSize(0, 500));
    AppSettings::instance().setWindowPos("PreferencesWindow", QPoint(0, 0));
    {
        PreferencesWindow prefWin4;
        QVERIFY(prefWin4.width() >= 400);
    }
    AppSettings::instance().setWindowSize("PreferencesWindow", QSize(500, 0));
    {
        PreferencesWindow prefWin4b;
        QVERIFY(prefWin4b.height() >= 220);
    }

    // Case 4: Non-existent scheme/lang/colorblind in updateUiFromState
    {
        PreferencesWindow prefWin5;
        prefWin5.setColourScheme("unknown_scheme_xyz");
        prefWin5.setColourBlindType(static_cast<ColorBlindMode>(999));
        prefWin5.setLanguage("unknown_lang_xyz");
        QEvent langEv(QEvent::LanguageChange);
        QApplication::sendEvent(&prefWin5, &langEv);
        prefWin5.onOpenThemesFolder();
        prefWin5.onOpenThemesFolder();
    }

    AppSettings::instance().setConfigFilePath(origPath);
}

void TestPreferencesWindow::testShowEventAndApplyButton()
{
    PreferencesWindow prefWin;
    prefWin.show();

    // Trigger themesUpdated signal
    ThemeManager::instance().scanThemes();

    // Trigger apply button directly via buttonBox
    auto *box = prefWin.findChild<QDialogButtonBox*>("buttonBox");
    QVERIFY(box != nullptr);
    auto *applyBtn = box->button(QDialogButtonBox::Apply);
    if (applyBtn) {
        applyBtn->click();
    }
}

QTEST_MAIN(TestPreferencesWindow)
#include "test_PreferencesWindow.moc"
