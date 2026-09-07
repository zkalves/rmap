/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include <QtTest>
#include <QLabel>
#include <QTabWidget>
#include <QTextBrowser>
#include <QDialogButtonBox>
#include <QTemporaryDir>
#include "AboutWindow.hpp"
#include "AppSettings.hpp"

class TestAboutWindow : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        QDir("work").removeRecursively();
        QDir("examples/work").removeRecursively();
        QDir().mkpath("work");
    }
    void cleanupTestCase() {
        QDir("work").removeRecursively();
        QDir("examples/work").removeRecursively();
    }
    void testAboutWindowUIElements();
    void testWindowProperties();
    void testTabsAndContent();
    void testApplicationMethods();
    void testAcceptAndClose();
    void testWindowSizePersistence();
};

void TestAboutWindow::testAboutWindowUIElements()
{
    AboutWindow aboutWin;
    auto *logo = aboutWin.findChild<QLabel*>("labelLogo");
    auto *title = aboutWin.findChild<QLabel*>("labelTitle");
    auto *subtitle = aboutWin.findChild<QLabel*>("labelSubtitle");
    auto *version = aboutWin.findChild<QLabel*>("labelVersion");
    auto *copyright = aboutWin.findChild<QLabel*>("labelCopyright");
    auto *tabWidget = aboutWin.findChild<QTabWidget*>("tabWidget");
    auto *buttonBox = aboutWin.findChild<QDialogButtonBox*>("buttonBox");

    QVERIFY(logo != nullptr);
    QVERIFY(title != nullptr);
    QVERIFY(subtitle != nullptr);
    QVERIFY(version != nullptr);
    QVERIFY(copyright != nullptr);
    QVERIFY(tabWidget != nullptr);
    QVERIFY(buttonBox != nullptr);

    QVERIFY(!logo->pixmap().isNull());
    QCOMPARE(title->text(), QString("rmap"));
    QVERIFY(subtitle->text().contains("Hardware Register Map"));
    QVERIFY(version->text().contains("0.2.0"));
    QVERIFY(copyright->text().contains("2026 Ezequiel Alves"));
}

void TestAboutWindow::testWindowProperties()
{
    AboutWindow aboutWin;
    QCOMPARE(aboutWin.windowTitle(), QString("About rmap"));
    QVERIFY(!aboutWin.isModal());
    QVERIFY(aboutWin.windowFlags().testFlag(Qt::Window));
    QVERIFY(aboutWin.windowFlags().testFlag(Qt::WindowCloseButtonHint));
    QVERIFY(aboutWin.minimumWidth() >= 400);
    QVERIFY(aboutWin.minimumHeight() >= 300);
}

void TestAboutWindow::testTabsAndContent()
{
    AboutWindow aboutWin;
    auto *tabWidget = aboutWin.findChild<QTabWidget*>("tabWidget");
    QVERIFY(tabWidget != nullptr);
    QCOMPARE(tabWidget->count(), 4);

    auto *aboutBrowser = aboutWin.findChild<QTextBrowser*>("textBrowserAbout");
    auto *featuresBrowser = aboutWin.findChild<QTextBrowser*>("textBrowserFeatures");
    auto *libBrowser = aboutWin.findChild<QTextBrowser*>("textBrowserLibraries");
    auto *licenseBrowser = aboutWin.findChild<QTextBrowser*>("textBrowserLicense");

    QVERIFY(aboutBrowser != nullptr);
    QVERIFY(featuresBrowser != nullptr);
    QVERIFY(libBrowser != nullptr);
    QVERIFY(licenseBrowser != nullptr);

    // Verify openExternalLinks is enabled
    QVERIFY(aboutBrowser->openExternalLinks());
    QVERIFY(featuresBrowser->openExternalLinks());
    QVERIFY(libBrowser->openExternalLinks());
    QVERIFY(licenseBrowser->openExternalLinks());

    // Verify About tab text
    QString aboutText = aboutBrowser->toPlainText();
    QVERIFY(aboutText.contains("rmap"));
    QVERIFY(aboutText.contains("ASIC") || aboutText.contains("FPGA"));
    QVERIFY(aboutText.contains("Linter") || aboutText.contains("DRC"));

    // Verify Features tab text
    QString featuresText = featuresBrowser->toPlainText();
    QVERIFY(featuresText.contains("SystemRDL"));
    QVERIFY(featuresText.contains("CMSIS-SVD"));
    QVERIFY(featuresText.contains("IP-XACT"));
    QVERIFY(featuresText.contains("UVM"));

    // Verify Libraries tab text
    QString libText = libBrowser->toPlainText();
    QVERIFY(libText.contains("Qt"));
    QVERIFY(libText.contains("Inja"));
    QVERIFY(libText.contains("json"));
    QVERIFY(libText.contains("Protocol Buffers"));

    // Verify License tab text
    QString licText = licenseBrowser->toPlainText();
    QVERIFY(licText.contains("Mozilla Public License"));
    QVERIFY(licText.contains("Ezequiel Alves"));
}

void TestAboutWindow::testApplicationMethods()
{
    AboutWindow aboutWin;
    QVERIFY(!aboutWin.applicationVersion().isEmpty());
    QVERIFY(!aboutWin.applicationName().isEmpty());
}

void TestAboutWindow::testAcceptAndClose()
{
    AboutWindow aboutWin;
    aboutWin.show();
    QVERIFY(aboutWin.isVisible());

    aboutWin.accept();
    QVERIFY(!aboutWin.isVisible());
}

void TestAboutWindow::testWindowSizePersistence()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString origPath = AppSettings::instance().configFilePath();
    AppSettings::instance().setConfigFilePath(tempDir.path() + "/test_rmap.conf");

    {
        AboutWindow aboutWin;
        aboutWin.resize(650, 520);
        aboutWin.saveWindowStateToSettings();
        QCOMPARE(AppSettings::instance().windowSize("AboutWindow"), QSize(650, 520));
    }

    {
        AboutWindow aboutWin2;
        QCOMPARE(AppSettings::instance().windowSize("AboutWindow"), QSize(650, 520));
        QCOMPARE(aboutWin2.size(), QSize(650, 520));
    }

    AppSettings::instance().setConfigFilePath(origPath);
}

QTEST_MAIN(TestAboutWindow)
#include "test_AboutWindow.moc"
