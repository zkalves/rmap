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

#ifndef LANGUAGEMANAGER_HPP
#define LANGUAGEMANAGER_HPP

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QTranslator>
#include <QHash>

struct LanguageInfo {
    QString code;         // e.g. "en", "es", "de", "fr", "zh_CN", "ja", "pt_BR"
    QString name;         // e.g. "English", "Spanish", "German"
    QString nativeName;   // e.g. "English", "Español", "Deutsch"
    QString resourcePath; // Embedded Qt resource path (e.g. ":/translations/rmap_es.json")
    bool isBuiltIn = true;

    QString displayName() const {
        if (nativeName.isEmpty() || nativeName == name) {
            return name;
        }
        return QStringLiteral("%1 (%2)").arg(nativeName, name);
    }
};

class JsonTranslator : public QTranslator {
    Q_OBJECT

public:
    explicit JsonTranslator(QObject *parent = nullptr);
    ~JsonTranslator() override = default;

    bool loadResource(const QString &resourcePath);
    bool loadData(const QByteArray &jsonData);

    QString translate(const char *context, const char *sourceText,
                      const char *disambiguation = nullptr, int n = -1) const override;
    bool isEmpty() const override;

    QString code() const { return m_code; }
    QString name() const { return m_name; }
    QString nativeName() const { return m_nativeName; }
    int translationCount() const;

private:
    QString m_code;
    QString m_name;
    QString m_nativeName;
    QHash<QString, QString> m_contextMap; // "context\x04sourceText" -> translation
    QHash<QString, QString> m_globalMap;   // "sourceText" -> translation
};

class LanguageManager : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(LanguageManager)

public:
    static LanguageManager& instance();

    const QList<LanguageInfo>& availableLanguages() const;
    QStringList languageCodes() const;
    QStringList languageNames() const;

    QString currentLanguage() const;
    QString currentLanguageName() const;
    LanguageInfo currentLanguageInfo() const;

    bool setLanguage(const QString &codeOrName);
    bool hasLanguage(const QString &codeOrName) const;
    LanguageInfo languageInfo(const QString &codeOrName) const;

signals:
    void languageChanged(const QString &languageCode);

private:
    LanguageManager();
    ~LanguageManager() override;

    void initLanguages();

    QList<LanguageInfo> m_languages;
    QString m_currentLanguage = QStringLiteral("en");
    QTranslator *m_activeTranslator = nullptr;
};

#endif // LANGUAGEMANAGER_HPP
