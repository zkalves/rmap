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

#ifndef IFORMAT_HANDLER_HPP
#define IFORMAT_HANDLER_HPP

#include <QString>
#include <QStringList>
#include <memory>

class RegMapTreeModel;
class RegConfigWindow;

struct FormatResult {
    bool success = false;
    QString errorMessage;
    QStringList warnings;
};

class IFormatHandler {
public:
    virtual ~IFormatHandler() = default;

    virtual QString formatName() const = 0;
    virtual QStringList supportedExtensions() const = 0;
    virtual QString fileFilter() const = 0;

    virtual FormatResult read(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) = 0;
    virtual FormatResult write(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) = 0;
};

#endif // IFORMAT_HANDLER_HPP
