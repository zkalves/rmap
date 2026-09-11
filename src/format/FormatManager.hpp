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

#ifndef FORMAT_MANAGER_HPP
#define FORMAT_MANAGER_HPP

#include <QString>
#include <QStringList>
#include <vector>
#include <memory>
#include "IFormatHandler.hpp"

class FormatManager {
public:
    static FormatManager& instance();

    void registerHandler(std::shared_ptr<IFormatHandler> handler);
    void clearHandlers();

    std::shared_ptr<IFormatHandler> handlerForFile(const QString &filepath) const;
    std::shared_ptr<IFormatHandler> handlerByName(const QString &name) const;

    FormatResult loadFile(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config);
    FormatResult saveFile(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config);

    void registerDefaultHandlers();

    QString allFilterString() const;
    const std::vector<std::shared_ptr<IFormatHandler>>& handlers() const;

private:
    FormatManager();

    std::vector<std::shared_ptr<IFormatHandler>> m_handlers;
};

#endif // FORMAT_MANAGER_HPP
