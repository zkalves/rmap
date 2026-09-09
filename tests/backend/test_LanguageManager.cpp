/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include <QtTest>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include "LanguageManager.hpp"
#include "AppSettings.hpp"

class TestLanguageManager : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testDefaultLanguageIsEnglish();
    void testAvailableLanguages();
    void testSetLanguage();
    void testTranslationLookup();
    void testJsonTranslatorDirect();
    void testBuildTimeLanguageEnforcement();
    void testAppSettingsPersistence();
};

void TestLanguageManager::initTestCase()
{
    QDir("work").removeRecursively();
    QDir().mkpath("work");
}

void TestLanguageManager::cleanupTestCase()
{
    QDir("work").removeRecursively();
    LanguageManager::instance().setLanguage("en");
}

void TestLanguageManager::testDefaultLanguageIsEnglish()
{
    LanguageManager &lm = LanguageManager::instance();
    lm.setLanguage("en");
    QCOMPARE(lm.currentLanguage(), QString("en"));
    QCOMPARE(lm.currentLanguageName(), QString("English"));
    QVERIFY(lm.hasLanguage("en"));
    QVERIFY(lm.hasLanguage("English"));
}

void TestLanguageManager::testAvailableLanguages()
{
    LanguageManager &lm = LanguageManager::instance();
    QStringList codes = lm.languageCodes();

    QVERIFY(codes.contains("en"));
    QVERIFY(codes.contains("es"));
    QVERIFY(codes.contains("de"));
    QVERIFY(codes.contains("fr"));
    QVERIFY(codes.contains("zh_CN"));
    QVERIFY(codes.contains("ja"));
    QVERIFY(codes.contains("pt_BR"));

    QVERIFY(codes.size() >= 7);

    LanguageInfo esInfo = lm.languageInfo("es");
    QCOMPARE(esInfo.code, QString("es"));
    QCOMPARE(esInfo.name, QString("Spanish"));
    QCOMPARE(esInfo.nativeName, QString("Español"));
    QCOMPARE(esInfo.displayName(), QString("Español (Spanish)"));

    QStringList names = lm.languageNames();
    QCOMPARE(names.size(), lm.availableLanguages().size());
    LanguageInfo curInfo = lm.currentLanguageInfo();
    QCOMPARE(curInfo.code, QString("en"));
    LanguageInfo missingInfo = lm.languageInfo("nonexistent_code_xyz");
    QVERIFY(missingInfo.code.isEmpty());
}

void TestLanguageManager::testSetLanguage()
{
    LanguageManager &lm = LanguageManager::instance();

    // Switch to Spanish
    QVERIFY(lm.setLanguage("es"));
    QCOMPARE(lm.currentLanguage(), QString("es"));
    QCOMPARE(lm.currentLanguageName(), QString("Spanish"));

    // Switch to German
    QVERIFY(lm.setLanguage("de"));
    QCOMPARE(lm.currentLanguage(), QString("de"));
    QCOMPARE(lm.currentLanguageName(), QString("German"));

    // Switch by Name (case-insensitive)
    QVERIFY(lm.setLanguage("French"));
    QCOMPARE(lm.currentLanguage(), QString("fr"));

    // Switch back to English
    QVERIFY(lm.setLanguage("en"));
    QCOMPARE(lm.currentLanguage(), QString("en"));

    // Invalid language
    QVERIFY(!lm.setLanguage("nonexistent_lang_xyz"));
    QCOMPARE(lm.currentLanguage(), QString("en"));

    // Empty language defaults to English
    QVERIFY(lm.setLanguage(""));
    QCOMPARE(lm.currentLanguage(), QString("en"));
}

void TestLanguageManager::testTranslationLookup()
{
    LanguageManager &lm = LanguageManager::instance();

    // In Spanish
    QVERIFY(lm.setLanguage("es"));
    QString esFile = QCoreApplication::translate("rmap", "File");
    QCOMPARE(esFile, QString("Archivo"));
    QString esUndo = QCoreApplication::translate("rmap", "Undo");
    QCOMPARE(esUndo, QString("Deshacer"));

    // In German
    QVERIFY(lm.setLanguage("de"));
    QString deFile = QCoreApplication::translate("rmap", "File");
    QCOMPARE(deFile, QString("Datei"));
    QString deUndo = QCoreApplication::translate("rmap", "Undo");
    QCOMPARE(deUndo, QString("Rückgängig"));

    // In French
    QVERIFY(lm.setLanguage("fr"));
    QString frFile = QCoreApplication::translate("rmap", "File");
    QCOMPARE(frFile, QString("Fichier"));

    // Reset to English
    QVERIFY(lm.setLanguage("en"));
    QString enFile = QCoreApplication::translate("rmap", "File");
    QCOMPARE(enFile, QString("File"));
}

void TestLanguageManager::testJsonTranslatorDirect()
{
    JsonTranslator jt;
    QByteArray sampleJson = R"({
        "code": "test_lang",
        "name": "Test Language",
        "nativeName": "Test Native",
        "translations": {
            "Hello": "Bonjour",
            "&File": "&Fichier"
        },
        "contexts": {
            "SpecialContext": {
                "Hello": "Salut"
            }
        }
    })";

    QVERIFY(jt.loadData(sampleJson));
    QCOMPARE(jt.code(), QString("test_lang"));
    QCOMPARE(jt.name(), QString("Test Language"));
    QCOMPARE(jt.nativeName(), QString("Test Native"));
    QVERIFY(!jt.isEmpty());

    // Context lookup
    QCOMPARE(jt.translate("SpecialContext", "Hello"), QString("Salut"));

    // Fallback to global map
    QCOMPARE(jt.translate("OtherContext", "Hello"), QString("Bonjour"));

    // Stripping ampersand lookup
    QCOMPARE(jt.translate(nullptr, "File"), QString("Fichier"));

    // Unknown string
    QVERIFY(jt.translate(nullptr, "Unknown String 123").isEmpty());

    QCOMPARE(jt.translationCount(), 3);
    QVERIFY(jt.translate(nullptr, "").isEmpty());
    QVERIFY(jt.translate(nullptr, nullptr).isEmpty());

    // Ampersand in source text mapped to plain key in translations
    QByteArray ampersandJson = R"({
        "language": "es",
        "translations": {
            "Save": "Guardar"
        }
    })";
    JsonTranslator jtAmp;
    QVERIFY(jtAmp.loadData(ampersandJson));
    QCOMPARE(jtAmp.code(), QString("es"));
    QCOMPARE(jtAmp.translate(nullptr, "&Save"), QString("Guardar"));

    // Failure cases: invalid json, non-object, and missing resource
    JsonTranslator jtErr;
    QVERIFY(!jtErr.loadData("{ invalid json"));
    QVERIFY(!jtErr.loadData("[]"));
    QVERIFY(!jtErr.loadResource("/nonexistent/path/file.json"));
}

void TestLanguageManager::testBuildTimeLanguageEnforcement()
{
    LanguageManager &lm = LanguageManager::instance();

    // Verify all build-time languages are present
    const auto &langs = lm.availableLanguages();
    QCOMPARE(langs.size(), 7);

    // Non-build-time languages are rejected
    QVERIFY(!lm.hasLanguage("it"));
    QVERIFY(!lm.hasLanguage("korean"));
    QVERIFY(!lm.setLanguage("it"));
    QVERIFY(!lm.setLanguage("ru"));

    // Current language remains English
    QCOMPARE(lm.currentLanguage(), QString("en"));
}

void TestLanguageManager::testAppSettingsPersistence()
{
    QTemporaryDir tempDir;
    QString configPath = tempDir.filePath("test_rmap.conf");
    AppSettings::instance().setConfigFilePath(configPath);

    AppSettings::instance().setLanguage("de");
    QCOMPARE(AppSettings::instance().language(), QString("de"));

    // Reload settings from disk
    AppSettings::instance().load();
    QCOMPARE(AppSettings::instance().language(), QString("de"));

    // Reset back to English
    AppSettings::instance().setLanguage("en");
    QCOMPARE(AppSettings::instance().language(), QString("en"));
}

QTEST_MAIN(TestLanguageManager)
#include "test_LanguageManager.moc"
