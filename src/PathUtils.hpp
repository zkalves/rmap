/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#ifndef PATH_UTILS_HPP
#define PATH_UTILS_HPP

#include <QString>
#include <QDir>
#include <QFileInfo>
#include <string>

namespace PathUtils {

/**
 * @brief Default output directory relative path across the application.
 */
constexpr const char* DEFAULT_OUTPUT_DIR = "./work";

/**
 * @brief Default templates directory relative path across the application.
 */
constexpr const char* DEFAULT_TEMPLATES_DIR = "./templates";

/**
 * @brief Returns the application default output directory ("./work").
 */
inline QString defaultOutputDir() { return QString::fromUtf8(DEFAULT_OUTPUT_DIR); }

/**
 * @brief Returns the application default templates directory ("./templates").
 */
inline QString defaultTemplatesDir() { return QString::fromUtf8(DEFAULT_TEMPLATES_DIR); }

/**
 * @brief Expands environment variables ($VAR, ${VAR}, %VAR%) and tilde (~) in the given path.
 *
 * Supports:
 * - Leading '~' or '~/' -> user's home directory.
 * - POSIX syntax '$VAR' and '${VAR}'.
 * - Windows syntax '%VAR%'.
 * - Normalizes path separators to forward slashes ('/').
 */
QString expandEnvVars(const QString &path);
std::string expandEnvVars(const std::string &path);
inline QString expandEnvVars(const char *path) { return expandEnvVars(QString::fromUtf8(path ? path : "")); }

/**
 * @brief Converts an absolute or relative path to a clean relative path anchored to baseDir.
 *
 * If baseDir is empty, QDir::currentPath() is used.
 * If targetPath is already relative, normalizes forward slashes and returns it.
 * If targetPath begins with an environment variable ($ or %), it is left un-relativized.
 */
QString toRelativePath(const QString &targetPath, const QString &baseDir = QString());
std::string toRelativePath(const std::string &targetPath, const std::string &baseDir = "");
inline QString toRelativePath(const char *targetPath, const char *baseDir = nullptr) {
    return toRelativePath(QString::fromUtf8(targetPath ? targetPath : ""), baseDir ? QString::fromUtf8(baseDir) : QString());
}

/**
 * @brief Resolves a path taking into account env var expansion, absolute paths, and relative search order.
 *
 * Resolution order:
 * 1. Expand environment variables and tilde.
 * 2. If the expanded path is absolute and exists, return its canonical/absolute path.
 * 3. If relative, check if it exists in primaryBaseDir.
 * 4. If not found, check if it exists in secondaryBaseDir (e.g. default folder or CWD).
 * 5. If not found, check if it exists in current working directory (CWD).
 * 6. If the file does not exist (e.g. creating a new output file), return the path constructed
 *    from primaryBaseDir (if non-empty), secondaryBaseDir (if non-empty), or CWD.
 */
QString resolvePath(const QString &path, const QString &primaryBaseDir = QString(), const QString &secondaryBaseDir = QString());
std::string resolvePath(const std::string &path, const std::string &primaryBaseDir = "", const std::string &secondaryBaseDir = "");
inline QString resolvePath(const char *path, const char *primaryBaseDir = nullptr, const char *secondaryBaseDir = nullptr) {
    return resolvePath(QString::fromUtf8(path ? path : ""), primaryBaseDir ? QString::fromUtf8(primaryBaseDir) : QString(), secondaryBaseDir ? QString::fromUtf8(secondaryBaseDir) : QString());
}

/**
 * @brief Normalizes path separators to forward slashes ('/') and cleans redundant separators.
 */
QString normalizeSeparators(const QString &path);
std::string normalizeSeparators(const std::string &path);
inline QString normalizeSeparators(const char *path) { return normalizeSeparators(QString::fromUtf8(path ? path : "")); }

} // namespace PathUtils

#endif // PATH_UTILS_HPP
