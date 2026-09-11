/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "PathUtils.hpp"
#include <QRegularExpression>
#include <QProcessEnvironment>
#include <QCoreApplication>
#include <cstdlib>

namespace PathUtils {

namespace {
static const QRegularExpression multiSlashRegex("/{2,}");
}

QString normalizeSeparators(const QString &path)
{
    if (path.isEmpty()) {
        return path;
    }
    QString result = path;
    result.replace('\\', '/');

    // Collapse multiple consecutive slashes, except keep leading "//" if UNC path
    bool hasLeadingDoubleSlash = result.startsWith("//") && !result.startsWith("///");
    result.replace(multiSlashRegex, "/");
    if (hasLeadingDoubleSlash && !result.startsWith("//")) {
        result.prepend('/');
    }
    return result;
}

std::string normalizeSeparators(const std::string &path)
{
    return normalizeSeparators(QString::fromStdString(path)).toStdString();
}

QString expandEnvVars(const QString &path)
{
    if (path.isEmpty()) return QString();

    QString result = normalizeSeparators(path);

    // 1. Tilde (~) expansion
    if (result == "~") {
        result = QDir::homePath();
    } else if (result.startsWith("~/")) {
        result = QDir::homePath() + result.mid(1);
    }

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    // 2. POSIX bracketed variable: ${VAR_NAME}
    static QRegularExpression reBracketed(R"(\$\{([A-Za-z_][A-Za-z0-9_]*)\})");
    QRegularExpressionMatchIterator itBracket = reBracketed.globalMatch(result);
    // Process replacements from left to right with offset or re-matching
    while (true) {
        QRegularExpressionMatch m = reBracketed.match(result);
        if (!m.hasMatch()) break;
        QString varName = m.captured(1);
        QString varVal = env.value(varName, QString::fromLocal8Bit(qgetenv(varName.toLocal8Bit().constData())));
        result.replace(m.capturedStart(), m.capturedLength(), varVal);
    }

    // 3. POSIX unbracketed variable: $VAR_NAME
    static QRegularExpression rePosix(R"(\$([A-Za-z_][A-Za-z0-9_]*))");
    while (true) {
        QRegularExpressionMatch m = rePosix.match(result);
        if (!m.hasMatch()) break;
        QString varName = m.captured(1);
        QString varVal = env.value(varName, QString::fromLocal8Bit(qgetenv(varName.toLocal8Bit().constData())));
        result.replace(m.capturedStart(), m.capturedLength(), varVal);
    }

    // 4. Windows variable: %VAR_NAME%
    static QRegularExpression reWin(R"(%([A-Za-z_][A-Za-z0-9_]*)%)");
    while (true) {
        QRegularExpressionMatch m = reWin.match(result);
        if (!m.hasMatch()) break;
        QString varName = m.captured(1);
        QString varVal = env.value(varName, QString::fromLocal8Bit(qgetenv(varName.toLocal8Bit().constData())));
        result.replace(m.capturedStart(), m.capturedLength(), varVal);
    }

    return normalizeSeparators(result);
}

std::string expandEnvVars(const std::string &path)
{
    return expandEnvVars(QString::fromStdString(path)).toStdString();
}

QString toRelativePath(const QString &targetPath, const QString &baseDir)
{
    QString trimmed = targetPath.trimmed();
    if (trimmed.isEmpty()) return QString();

    // If it starts with an unexpanded environment variable ($ or %), do not alter it
    if (trimmed.startsWith('$') || trimmed.startsWith('%')) {
        return normalizeSeparators(trimmed);
    }

    QString base = baseDir.trimmed();
    if (base.isEmpty()) {
        base = QDir::currentPath();
    } else {
        base = expandEnvVars(base);
        QFileInfo baseFi(base);
        if (baseFi.isFile()) {
            base = baseFi.absolutePath();
        } else {
            base = baseFi.absoluteFilePath();
        }
    }

    // If target is already relative, return clean relative format
    QFileInfo targetFi(trimmed);
    if (targetFi.isRelative()) {
        return normalizeSeparators(trimmed);
    }

    // Target is absolute, compute relative path from base directory
    QString rel = QDir(base).relativeFilePath(trimmed);
    rel = normalizeSeparators(rel);
    if (!rel.startsWith('.') && !rel.startsWith('/')) {
        rel = "./" + rel;
    }
    return rel;
}

std::string toRelativePath(const std::string &targetPath, const std::string &baseDir)
{
    return toRelativePath(QString::fromStdString(targetPath), QString::fromStdString(baseDir)).toStdString();
}

QString resolvePath(const QString &path, const QString &primaryBaseDir, const QString &secondaryBaseDir)
{
    QString trimmed = path.trimmed();
    if (trimmed.isEmpty()) return QString();

    QString expanded = expandEnvVars(trimmed);
    QFileInfo expFi(expanded);

    // 1. Direct absolute path
    if (expFi.isAbsolute()) {
        if (expFi.exists()) {
            return normalizeSeparators(expFi.canonicalFilePath());
        }
        return normalizeSeparators(expFi.absoluteFilePath());
    }

    // Helper lambda to clean base directory
    auto cleanBase = [](const QString &dir) -> QString {
        if (dir.trimmed().isEmpty()) return QString();
        QString exp = expandEnvVars(dir.trimmed());
        QFileInfo fi(exp);
        return fi.isFile() ? fi.absolutePath() : fi.absoluteFilePath();
    };

    QString pBase = cleanBase(primaryBaseDir);
    QString sBase = cleanBase(secondaryBaseDir);

    // 2. Check primary base directory (e.g. register map file directory)
    if (!pBase.isEmpty()) {
        QFileInfo fi(QDir(pBase).filePath(expanded));
        if (fi.exists()) {
            return normalizeSeparators(fi.canonicalFilePath());
        }
        // If pBase ends with "templates" and expanded begins with "templates/" or "./templates/"
        QString normP = normalizeSeparators(pBase);
        if (normP.endsWith("/templates", Qt::CaseInsensitive) || normP == "templates") {
            QString subTmpl = expanded;
            if (subTmpl.startsWith("./templates/", Qt::CaseInsensitive)) {
                subTmpl = subTmpl.mid(12);
            } else if (subTmpl.startsWith("templates/", Qt::CaseInsensitive)) {
                subTmpl = subTmpl.mid(10);
            }
            if (subTmpl != expanded) {
                QFileInfo fiSub(QDir(pBase).filePath(subTmpl));
                if (fiSub.exists()) {
                    return normalizeSeparators(fiSub.canonicalFilePath());
                }
            }
        }
    }

    // 3. Check secondary base directory (e.g. default templates folder or project root)
    if (!sBase.isEmpty()) {
        QFileInfo fi(QDir(sBase).filePath(expanded));
        if (fi.exists()) {
            return normalizeSeparators(fi.canonicalFilePath());
        }
        // If sBase ends with "templates" and expanded begins with "templates/" or "./templates/"
        QString normS = normalizeSeparators(sBase);
        if (normS.endsWith("/templates", Qt::CaseInsensitive) || normS == "templates") {
            QString subTmpl = expanded;
            if (subTmpl.startsWith("./templates/", Qt::CaseInsensitive)) {
                subTmpl = subTmpl.mid(12);
            } else if (subTmpl.startsWith("templates/", Qt::CaseInsensitive)) {
                subTmpl = subTmpl.mid(10);
            }
            if (subTmpl != expanded) {
                QFileInfo fiSub(QDir(sBase).filePath(subTmpl));
                if (fiSub.exists()) {
                    return normalizeSeparators(fiSub.canonicalFilePath());
                }
            }
        }
    }

    // 4. Check Current Working Directory (CWD)
    QFileInfo fiCwd(QDir::current().filePath(expanded));
    if (fiCwd.exists()) {
        return normalizeSeparators(fiCwd.canonicalFilePath());
    }

    // 5. Fallback for new / non-existent target files (construct path)
    if (!pBase.isEmpty()) {
        return normalizeSeparators(QDir(pBase).filePath(expanded));
    }
    if (!sBase.isEmpty()) {
        QString normS = normalizeSeparators(sBase);
        if ((normS.endsWith("/templates", Qt::CaseInsensitive) || normS == "templates") &&
            (expanded.startsWith("templates/", Qt::CaseInsensitive) || expanded.startsWith("./templates/", Qt::CaseInsensitive))) {
            QString subTmpl = expanded.startsWith("./templates/", Qt::CaseInsensitive) ? expanded.mid(12) : expanded.mid(10);
            return normalizeSeparators(QDir(sBase).filePath(subTmpl));
        }
        return normalizeSeparators(QDir(sBase).filePath(expanded));
    }
    return normalizeSeparators(QDir::current().filePath(expanded));
}

std::string resolvePath(const std::string &path, const std::string &primaryBaseDir, const std::string &secondaryBaseDir)
{
    return resolvePath(QString::fromStdString(path), QString::fromStdString(primaryBaseDir), QString::fromStdString(secondaryBaseDir)).toStdString();
}

static bool s_installedOverride = false;

void setInstalledOverride(bool enable)
{
    s_installedOverride = enable;
}

bool isInstalledOverride()
{
    return s_installedOverride;
}

QString defaultTemplatesDir()
{
    // 1. Explicit environment variable override
    if (qEnvironmentVariableIsSet("RMAP_TEMPLATES_DIR")) {
        QString envDir = QString::fromLocal8Bit(qgetenv("RMAP_TEMPLATES_DIR")).trimmed();
        if (!envDir.isEmpty()) {
            return normalizeSeparators(expandEnvVars(envDir));
        }
    }

    // 2. Local "./templates" if it exists
    if (!s_installedOverride) {
        QDir localTemplates("./templates");
        if (localTemplates.exists() && !localTemplates.isEmpty()) {
            return QString("./templates");
        }
    }

    // 3. Application-relative relocatable directory: <bin_dir>/../share/rmap/templates
    if (!s_installedOverride) {
        QString appDir = QCoreApplication::applicationDirPath();
        if (!appDir.isEmpty()) {
            QDir relShare(appDir + "/../share/rmap/templates");
            if (relShare.exists()) {
                return normalizeSeparators(relShare.canonicalPath().isEmpty() ? relShare.absolutePath() : relShare.canonicalPath());
            }
        }
    }

    // 4. Configured compile-time installation path (e.g. /usr/local/share/rmap/templates)
#ifdef RMAP_INSTALL_TEMPLATES_DIR
    QString installDir = QString::fromUtf8(RMAP_INSTALL_TEMPLATES_DIR).trimmed();
    if (s_installedOverride || (!installDir.isEmpty() && QDir(installDir).exists())) {
        return normalizeSeparators(installDir);
    }
#endif

    // 5. Fallback relative path
    return QString::fromUtf8(DEFAULT_TEMPLATES_DIR);
}

QString defaultExamplesDir()
{
    // 1. Explicit environment variable override
    if (qEnvironmentVariableIsSet("RMAP_EXAMPLES_DIR")) {
        QString envDir = QString::fromLocal8Bit(qgetenv("RMAP_EXAMPLES_DIR")).trimmed();
        if (!envDir.isEmpty()) {
            return normalizeSeparators(expandEnvVars(envDir));
        }
    }

    // 2. Local "./examples" if it exists
    if (!s_installedOverride) {
        QDir localEx("./examples");
        if (localEx.exists() && (localEx.exists("rmt") || !localEx.isEmpty())) {
            return QString("./examples");
        }
    }

    // 3. Application-relative relocatable directory: <bin_dir>/../share/rmap/examples
    if (!s_installedOverride) {
        QString appDir = QCoreApplication::applicationDirPath();
        if (!appDir.isEmpty()) {
            QDir relShare(appDir + "/../share/rmap/examples");
            if (relShare.exists()) {
                return normalizeSeparators(relShare.canonicalPath().isEmpty() ? relShare.absolutePath() : relShare.canonicalPath());
            }
        }
    }

    // 4. Configured compile-time installation path
#ifdef RMAP_INSTALL_EXAMPLES_DIR
    QString installDir = QString::fromUtf8(RMAP_INSTALL_EXAMPLES_DIR).trimmed();
    if (s_installedOverride || (!installDir.isEmpty() && QDir(installDir).exists())) {
        return normalizeSeparators(installDir);
    }
#endif

    // 5. Fallback relative path
    return QString("./examples");
}

QString defaultDocsDir()
{
    // 1. Explicit environment variable override
    if (qEnvironmentVariableIsSet("RMAP_DOCS_DIR")) {
        QString envDir = QString::fromLocal8Bit(qgetenv("RMAP_DOCS_DIR")).trimmed();
        if (!envDir.isEmpty()) {
            return normalizeSeparators(expandEnvVars(envDir));
        }
    }

    // 2. Local "./docs" if it exists
    if (!s_installedOverride) {
        QDir localDocs("./docs");
        if (localDocs.exists() && !localDocs.isEmpty()) {
            return QString("./docs");
        }
    }

    // 3. Application-relative relocatable directory: <bin_dir>/../share/doc/rmap
    if (!s_installedOverride) {
        QString appDir = QCoreApplication::applicationDirPath();
        if (!appDir.isEmpty()) {
            QDir relDoc(appDir + "/../share/doc/rmap");
            if (relDoc.exists()) {
                return normalizeSeparators(relDoc.canonicalPath().isEmpty() ? relDoc.absolutePath() : relDoc.canonicalPath());
            }
        }
    }

    // 4. Configured compile-time installation path
#ifdef RMAP_INSTALL_DOCDIR
    QString installDir = QString::fromUtf8(RMAP_INSTALL_DOCDIR).trimmed();
    if (s_installedOverride || (!installDir.isEmpty() && QDir(installDir).exists())) {
        return normalizeSeparators(installDir);
    }
#endif

    // 5. Fallback relative path
    return QString("./docs");
}

} // namespace PathUtils
