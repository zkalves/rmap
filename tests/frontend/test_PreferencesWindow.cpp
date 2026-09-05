/* This file is part of rmap.
 *
 * rmap is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * rmap is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rmap.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include <QtTest>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include "PreferencesWindow.hpp"
#include "ThemeManager.hpp"
#include "AppSettings.hpp"

class TestPreferencesWindow : public QObject
{
    Q_OBJECT

private slots:
    void testPreferencesDefaults();
    void testColourSchemeChange();
    void testColourBlindModeToggle();
    void testApplyAndReject();
    void testWindowSizePersistence();
};

void TestPreferencesWindow::testPreferencesDefaults()
{
    PreferencesWindow prefWin;
    auto *combo = prefWin.findChild<QComboBox*>("colourSchemeCombo");
    auto *cb = prefWin.findChild<QCheckBox*>("colourBlindMode");

    QVERIFY(combo != nullptr);
    QVERIFY(cb != nullptr);
    QVERIFY(combo->count() >= 6);

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
    prefWin.setColourBlindMode(true);
    QCOMPARE(prefWin.isColourBlindMode(), true);

    prefWin.accept();
    QCOMPARE(AppSettings::instance().colourBlindMode(), true);

    prefWin.setColourBlindMode(false);
    prefWin.accept();
    QCOMPARE(AppSettings::instance().colourBlindMode(), false);
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

QTEST_MAIN(TestPreferencesWindow)
#include "test_PreferencesWindow.moc"
