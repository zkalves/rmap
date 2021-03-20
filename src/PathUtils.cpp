#include "PathUtils.hpp"
#include <QRegularExpression>
#include <QProcessEnvironment>
#include <cstdlib>

namespace PathUtils {

QString normalizeSeparators(const QString &path)
{
    QString result = path;
    result.replace('\\', '/');

    // Collapse multiple consecutive slashes, except keep leading "//" if UNC path
    bool hasLeadingDoubleSlash = result.startsWith("//") && !result.startsWith("///");
    static QRegularExpression multiSlash("/{2,}");
    result.replace(multiSlash, "/");
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
    }

    // 3. Check secondary base directory (e.g. default templates folder or project root)
    if (!sBase.isEmpty()) {
        QFileInfo fi(QDir(sBase).filePath(expanded));
        if (fi.exists()) {
            return normalizeSeparators(fi.canonicalFilePath());
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
        return normalizeSeparators(QDir(sBase).filePath(expanded));
    }
    return normalizeSeparators(QDir::current().filePath(expanded));
}

std::string resolvePath(const std::string &path, const std::string &primaryBaseDir, const std::string &secondaryBaseDir)
{
    return resolvePath(QString::fromStdString(path), QString::fromStdString(primaryBaseDir), QString::fromStdString(secondaryBaseDir)).toStdString();
}

} // namespace PathUtils
