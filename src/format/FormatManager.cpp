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

#include "FormatManager.hpp"
#include <QFileInfo>
#include "ProtobufHandler.hpp"
#include "SystemRdlHandler.hpp"
#include "IpxactHandler.hpp"
#include "JsonHandler.hpp"
#include "CsvHandler.hpp"
#include "CmsisSvdHandler.hpp"

IFormatHandler::~IFormatHandler() = default;

FormatManager& FormatManager::instance()
{
    static FormatManager s_instance;
    return s_instance;
}

FormatManager::FormatManager()
{
    registerDefaultHandlers();
}

void FormatManager::registerDefaultHandlers()
{
    registerHandler(std::make_shared<ProtobufHandler>());
    registerHandler(std::make_shared<SystemRdlHandler>());
    registerHandler(std::make_shared<IpxactHandler>());
    registerHandler(std::make_shared<CmsisSvdHandler>());
    registerHandler(std::make_shared<JsonHandler>());
    registerHandler(std::make_shared<CsvHandler>());
}

void FormatManager::clearHandlers()
{
    m_handlers.clear();
}

void FormatManager::registerHandler(std::shared_ptr<IFormatHandler> handler)
{
    if (handler && m_handlers.size() < 128) {
        m_handlers.push_back(handler);
    }
}

const std::vector<std::shared_ptr<IFormatHandler>>& FormatManager::handlers() const
{
    return m_handlers;
}

std::shared_ptr<IFormatHandler> FormatManager::handlerForFile(const QString &filepath) const
{
    QFileInfo info(filepath);
    QString ext = info.suffix().toLower();

    for (const auto &handler : m_handlers) {
        for (const QString &supported : handler->supportedExtensions()) {
            if (supported.toLower() == ext) {
                return handler;
            }
        }
    }

    // Default fallback to ProtobufHandler if unknown
    for (const auto &handler : m_handlers) {
        if (std::dynamic_pointer_cast<ProtobufHandler>(handler)) {
            return handler;
        }
    }

    return nullptr;
}

std::shared_ptr<IFormatHandler> FormatManager::handlerByName(const QString &name) const
{
    for (const auto &handler : m_handlers) {
        if (handler->formatName().contains(name, Qt::CaseInsensitive)) {
            return handler;
        }
    }
    return nullptr;
}

#include "PathUtils.hpp"

FormatResult FormatManager::loadFile(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config)
{
    QString expanded = PathUtils::expandEnvVars(filepath);
    auto handler = handlerForFile(expanded);
    if (!handler) {
        return {false, QString("No format handler found for file: %1").arg(filepath), {}};
    }
    return handler->read(expanded, model, config);
}

FormatResult FormatManager::saveFile(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config)
{
    QString expanded = PathUtils::expandEnvVars(filepath);
    auto handler = handlerForFile(expanded);
    if (!handler) {
        return {false, QString("No format handler found for file: %1").arg(filepath), {}};
    }
    return handler->write(expanded, model, config);
}

QString FormatManager::allFilterString() const
{
    QStringList allExts;
    QStringList filterParts;

    for (const auto &handler : m_handlers) {
        for (const QString &ext : handler->supportedExtensions()) {
            allExts.append(QString("*.%1").arg(ext));
        }
        filterParts.append(handler->fileFilter());
    }

    QString composite = QString("All Supported Formats (%1)").arg(allExts.join(" "));
    filterParts.prepend(composite);
    return filterParts.join(";;");
}
