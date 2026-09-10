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
    void testEdgeCasesAndFallbacks();
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

    // Test code-only loadData which triggers QLocale fallbacks
    QByteArray localeFallbackJson = R"({
        "code": "es",
        "translations": { "Hello": "Hola", "Empty": "" },
        "contexts": { "Ctx": { "Key": "Val", "EmptyCtx": "" } }
    })";
    JsonTranslator jtLocale;
    QVERIFY(jtLocale.loadData(localeFallbackJson));
    QCOMPARE(jtLocale.code(), QString("es"));
    QVERIFY(!jtLocale.name().isEmpty());
    QVERIFY(!jtLocale.nativeName().isEmpty());
    // empty translation strings in context & global
    QVERIFY(jtLocale.translate("Ctx", "EmptyCtx").isEmpty());
    QVERIFY(jtLocale.translate(nullptr, "Empty").isEmpty());
    QVERIFY(jtLocale.translate("UnknownCtx", "Empty").isEmpty());
    QVERIFY(jtLocale.translate("", "Hello").isEmpty() == false);

    // Ampersand with empty translated string
    QByteArray emptyAmpJson = R"({
        "code": "es",
        "translations": { "Clean": "", "&WithAmp": "" }
    })";
    JsonTranslator jtEmptyAmp;
    QVERIFY(jtEmptyAmp.loadData(emptyAmpJson));
    QVERIFY(jtEmptyAmp.translate(nullptr, "&Clean").isEmpty());
    QVERIFY(jtEmptyAmp.translate(nullptr, "WithAmp").isEmpty());

    // Empty dictionaries
    JsonTranslator jtEmpty;
    QVERIFY(jtEmpty.isEmpty());
    QCOMPARE(jtEmpty.translationCount(), 0);
    QVERIFY(!jtEmpty.loadData(R"({"code": "es"})"));

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

void TestLanguageManager::testEdgeCasesAndFallbacks()
{
    // 1. Destructor coverage with active translator
    {
        LanguageManager localLm;
        localLm.m_activeTranslator = new QTranslator(&localLm);
        if (qApp) {
            qApp->installTranslator(localLm.m_activeTranslator);
        }
    }

    // 2. currentLanguageName & currentLanguageInfo fallback
    LanguageManager &lm = LanguageManager::instance();
    QString origLang = lm.m_currentLanguage;
    lm.m_currentLanguage = "nonexistent_code_123";
    QCOMPARE(lm.currentLanguageName(), QString("English"));
    QCOMPARE(lm.currentLanguageInfo().code, QString("en"));

    // Empty languages list fallback
    QList<LanguageInfo> savedLangs = lm.m_languages;
    lm.m_languages.clear();
    QCOMPARE(lm.currentLanguageInfo().code, QString());
    lm.m_languages = savedLangs;
    lm.m_currentLanguage = origLang;

    // 3. Fallback to dev path translations/rmap_%1.json
    LanguageInfo devLang;
    devLang.code = "es";
    devLang.name = "SpanishDev";
    devLang.nativeName = "Español";
    devLang.resourcePath = "non_existent_res_path_xyz.json";
    lm.m_languages.append(devLang);
    QVERIFY(lm.setLanguage("SpanishDev"));
    lm.m_languages.removeLast();

    // 4. Missing translation file
    LanguageInfo missingLang;
    missingLang.code = "xyz_missing";
    missingLang.name = "MissingLang";
    missingLang.nativeName = "Missing";
    missingLang.resourcePath = "definitely_not_a_valid_file_at_all.json";
    lm.m_languages.append(missingLang);
    QVERIFY(!lm.setLanguage("MissingLang"));
    lm.m_languages.removeLast();

    // 5. Invalid JSON translation resource
    QTemporaryFile badJsonFile;
    QVERIFY(badJsonFile.open());
    badJsonFile.write("{ invalid json ");
    badJsonFile.close();
    LanguageInfo badLang;
    badLang.code = "bad_json_lang";
    badLang.name = "BadLang";
    badLang.nativeName = "Bad";
    badLang.resourcePath = badJsonFile.fileName();
    lm.m_languages.append(badLang);
    QVERIFY(!lm.setLanguage("BadLang"));
    lm.m_languages.removeLast();

    // 6. Native name lookups and LanguageInfo displayName branches
    QVERIFY(lm.hasLanguage("Español"));
    QVERIFY(lm.hasLanguage("Deutsch"));
    QVERIFY(lm.hasLanguage("日本語"));
    QCOMPARE(lm.languageInfo("Español").code, QString("es"));
    QCOMPARE(lm.languageInfo("Deutsch").code, QString("de"));
    QCOMPARE(lm.languageInfo("nonexistent_native_xyz").code, QString());

    LanguageInfo liSame;
    liSame.name = "English";
    liSame.nativeName = "English";
    QCOMPARE(liSame.displayName(), QString("English"));

    LanguageInfo liEmptyNative;
    liEmptyNative.name = "TestOnly";
    liEmptyNative.nativeName = "";
    QCOMPARE(liEmptyNative.displayName(), QString("TestOnly"));

    // 7. Empty and whitespace setLanguage calls
    QVERIFY(lm.setLanguage(""));
    QCOMPARE(lm.currentLanguage(), QString("en"));
    QVERIFY(lm.setLanguage("   "));
    QCOMPARE(lm.currentLanguage(), QString("en"));

    // 8. Switching from one non-English language to another (replaces active translator)
    QVERIFY(lm.setLanguage("es"));
    QCOMPARE(lm.currentLanguage(), QString("es"));
    QVERIFY(lm.setLanguage("de"));
    QCOMPARE(lm.currentLanguage(), QString("de"));
    QVERIFY(lm.setLanguage("en"));
    QCOMPARE(lm.currentLanguage(), QString("en"));

    // Restore to English
    lm.setLanguage("en");
    QCOMPARE(lm.currentLanguage(), QString("en"));
}

QTEST_MAIN(TestLanguageManager)
#include "test_LanguageManager.moc"
