/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "LanguageManager.hpp"
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QDebug>

// -------------------------------------------------------------------------
// JsonTranslator Implementation
// -------------------------------------------------------------------------

JsonTranslator::JsonTranslator(QObject *parent)
    : QTranslator(parent)
{
}

bool JsonTranslator::loadResource(const QString &resourcePath)
{
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QByteArray data = file.readAll();
    file.close();
    return loadData(data);
}

bool JsonTranslator::loadData(const QByteArray &jsonData)
{
    QJsonParseError parseError{};
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (doc.isNull() || !doc.isObject()) {
        return false;
    }

    QJsonObject root = doc.object();

    m_code = root.value(QStringLiteral("code")).toString();
    if (m_code.isEmpty()) {
        m_code = root.value(QStringLiteral("language")).toString();
    }

    m_name = root.value(QStringLiteral("name")).toString();
    m_nativeName = root.value(QStringLiteral("nativeName")).toString();

    if (m_name.isEmpty() && !m_code.isEmpty()) {
        m_name = QLocale(m_code).languageToString(QLocale(m_code).language());
    }
    if (m_nativeName.isEmpty() && !m_code.isEmpty()) {
        m_nativeName = QLocale(m_code).nativeLanguageName();
    }

    m_globalMap.clear();
    m_contextMap.clear();

    // 1. Global translations map: { "sourceText": "translatedText" }
    QJsonObject transObj = root.value(QStringLiteral("translations")).toObject();
    for (auto it = transObj.begin(); it != transObj.end(); ++it) {
        m_globalMap.insert(it.key(), it.value().toString());
    }

    // 2. Context-specific translations: { "contexts": { "context": { "src": "trans" } } }
    QJsonObject contextsObj = root.value(QStringLiteral("contexts")).toObject();
    for (auto ctxIt = contextsObj.begin(); ctxIt != contextsObj.end(); ++ctxIt) {
        QString ctxName = ctxIt.key();
        QJsonObject ctxTrans = ctxIt.value().toObject();
        for (auto it = ctxTrans.begin(); it != ctxTrans.end(); ++it) {
            m_contextMap.insert(ctxName + QChar(0x04) + it.key(), it.value().toString());
        }
    }

    return (!m_globalMap.isEmpty() || !m_contextMap.isEmpty());
}

int JsonTranslator::translationCount() const
{
    return m_globalMap.size() + m_contextMap.size();
}

bool JsonTranslator::isEmpty() const
{
    return m_globalMap.isEmpty() && m_contextMap.isEmpty();
}

QString JsonTranslator::translate(const char *context, const char *sourceText,
                                  const char *disambiguation, int n) const
{
    Q_UNUSED(disambiguation);
    Q_UNUSED(n);

    if (!sourceText || !sourceText[0]) {
        return QString();
    }

    QString src = QString::fromUtf8(sourceText);

    // 1. Context-specific lookup
    if (context && context[0]) {
        QString ctxKey = QString::fromUtf8(context) + QChar(0x04) + src;
        auto it = m_contextMap.constFind(ctxKey);
        if (it != m_contextMap.constEnd() && !it.value().isEmpty()) {
            return it.value();
        }
    }

    // 2. Global direct lookup
    auto it = m_globalMap.constFind(src);
    if (it != m_globalMap.constEnd() && !it.value().isEmpty()) {
        return it.value();
    }

    // 3. Fallback: try stripping mnemonic '&' from sourceText
    if (src.contains(QLatin1Char('&'))) {
        QString cleanSrc = src;
        cleanSrc.remove(QLatin1Char('&'));
        auto itClean = m_globalMap.constFind(cleanSrc);
        if (itClean != m_globalMap.constEnd() && !itClean.value().isEmpty()) {
            return itClean.value();
        }
    } else {
        // Or if dictionary has '&' version (e.g. key is "&File" and query is "File")
        QString ampersandSrc = QStringLiteral("&") + src;
        auto itAmp = m_globalMap.constFind(ampersandSrc);
        if (itAmp != m_globalMap.constEnd() && !itAmp.value().isEmpty()) {
            QString res = itAmp.value();
            res.remove(QLatin1Char('&'));
            return res;
        }
    }

    return QString();
}

// -------------------------------------------------------------------------
// LanguageManager Implementation (Build-Time Configuration Only)
// -------------------------------------------------------------------------

LanguageManager& LanguageManager::instance()
{
    static LanguageManager inst;
    return inst;
}

LanguageManager::LanguageManager()
{
    initLanguages();
}

LanguageManager::~LanguageManager()
{
    if (m_activeTranslator) {
        if (qApp) {
            qApp->removeTranslator(m_activeTranslator);
        }
        delete m_activeTranslator;
        m_activeTranslator = nullptr;
    }
}

void LanguageManager::initLanguages()
{
    m_languages.clear();

    // Baseline native English (no external translator needed)
    LanguageInfo en;
    en.code = QStringLiteral("en");
    en.name = QStringLiteral("English");
    en.nativeName = QStringLiteral("English");
    en.resourcePath = QString();
    en.isBuiltIn = true;
    m_languages.append(en);

    // Build-time configured languages embedded in Qt resources (res/resources.qrc)
    struct BuildTimeLang {
        const char *code;
        const char *name;
        const char *nativeName;
        const char *resourcePath;
    };
    static const BuildTimeLang buildTimeTable[] = {
        {"es",    "Spanish",    "Español",  ":/translations/rmap_es.json"},
        {"de",    "German",     "Deutsch",  ":/translations/rmap_de.json"},
        {"fr",    "French",     "Français", ":/translations/rmap_fr.json"},
        {"zh_CN", "Chinese",    "简体中文",  ":/translations/rmap_zh_CN.json"},
        {"ja",    "Japanese",   "日本語",    ":/translations/rmap_ja.json"},
        {"pt_BR", "Portuguese", "Português", ":/translations/rmap_pt_BR.json"}
    };

    for (const auto &entry : buildTimeTable) {
        LanguageInfo info;
        info.code = QString::fromLatin1(entry.code);
        info.name = QString::fromLatin1(entry.name);
        info.nativeName = QString::fromUtf8(entry.nativeName);
        info.resourcePath = QString::fromLatin1(entry.resourcePath);
        info.isBuiltIn = true;
        m_languages.append(info);
    }
}

const QList<LanguageInfo>& LanguageManager::availableLanguages() const
{
    return m_languages;
}

QStringList LanguageManager::languageCodes() const noexcept
{
    QStringList list;
    list.reserve(m_languages.size());
    for (const auto &l : m_languages) {
        list.append(l.code);
    }
    return list;
}

QStringList LanguageManager::languageNames() const noexcept
{
    QStringList list;
    list.reserve(m_languages.size());
    for (const auto &l : m_languages) {
        list.append(l.displayName());
    }
    return list;
}

QString LanguageManager::currentLanguage() const
{
    return m_currentLanguage;
}

QString LanguageManager::currentLanguageName() const
{
    for (const auto &l : m_languages) {
        if (l.code == m_currentLanguage) {
            return l.name;
        }
    }
    return QStringLiteral("English");
}

LanguageInfo LanguageManager::currentLanguageInfo() const
{
    for (const auto &l : m_languages) {
        if (l.code == m_currentLanguage) {
            return l;
        }
    }
    return m_languages.isEmpty() ? LanguageInfo{} : m_languages.first();
}

bool LanguageManager::hasLanguage(const QString &codeOrName) const
{
    QString target = codeOrName.trimmed().toLower();
    for (const auto &l : m_languages) {
        if (l.code.toLower() == target || l.name.toLower() == target || l.nativeName.toLower() == target) {
            return true;
        }
    }
    return false;
}

LanguageInfo LanguageManager::languageInfo(const QString &codeOrName) const
{
    QString target = codeOrName.trimmed().toLower();
    for (const auto &l : m_languages) {
        if (l.code.toLower() == target || l.name.toLower() == target || l.nativeName.toLower() == target) {
            return l;
        }
    }
    return LanguageInfo{};
}

bool LanguageManager::setLanguage(const QString &codeOrName)
{
    QString target = codeOrName.trimmed();
    if (target.isEmpty()) {
        target = QStringLiteral("en");
    }

    // Find matching build-time language
    int foundIdx = -1;
    for (int i = 0; i < m_languages.size(); ++i) {
        if (m_languages[i].code.compare(target, Qt::CaseInsensitive) == 0 ||
            m_languages[i].name.compare(target, Qt::CaseInsensitive) == 0) {
            foundIdx = i;
            break;
        }
    }

    if (foundIdx < 0 && target.toLower() != QStringLiteral("en")) {
        return false;
    }

    QString code = (foundIdx >= 0) ? m_languages[foundIdx].code : QStringLiteral("en");

    // English: remove translator to restore default application strings
    if (code.toLower() == QStringLiteral("en")) {
        if (m_activeTranslator) {
            if (qApp) {
                qApp->removeTranslator(m_activeTranslator);
            }
            delete m_activeTranslator;
            m_activeTranslator = nullptr;
        }
        m_currentLanguage = QStringLiteral("en");
        QLocale::setDefault(QLocale(QLocale::English));
        emit languageChanged(m_currentLanguage);
        return true;
    }

    const LanguageInfo &info = m_languages[foundIdx];
    QString resPath = info.resourcePath;

    // Fallback: check alternate resource or filesystem path if running in dev environment
    if (!QFile::exists(resPath)) {
        QString devPath = QStringLiteral("translations/rmap_%1.json").arg(code);
        if (QFile::exists(devPath)) {
            resPath = devPath;
        }
    }

    if (resPath.isEmpty() || !QFile::exists(resPath)) {
        qWarning() << "LanguageManager: Translation resource not found:" << resPath;
        return false;
    }

    JsonTranslator *jt = new JsonTranslator(this);
    if (!jt->loadResource(resPath)) {
        delete jt;
        qWarning() << "LanguageManager: Failed to load translation resource:" << resPath;
        return false;
    }

    if (m_activeTranslator) {
        if (qApp) {
            qApp->removeTranslator(m_activeTranslator);
        }
        delete m_activeTranslator;
        m_activeTranslator = nullptr;
    }

    m_activeTranslator = jt;
    if (qApp) {
        qApp->installTranslator(m_activeTranslator);
    }

    m_currentLanguage = code;
    QLocale::setDefault(QLocale(code));
    emit languageChanged(m_currentLanguage);

    return true;
}
