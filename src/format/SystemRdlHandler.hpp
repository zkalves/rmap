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

#ifndef SYSTEM_RDL_HANDLER_HPP
#define SYSTEM_RDL_HANDLER_HPP

#include "IFormatHandler.hpp"

class SystemRdlHandler : public IFormatHandler {
public:
    QString formatName() const override { return "SystemRDL"; }
    QStringList supportedExtensions() const override { return {"rdl", "systemrdl"}; }
    QString fileFilter() const override {
        return "SystemRDL (*.rdl *.systemrdl)";
    }

    FormatResult read(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) override;
    FormatResult write(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) override;
};

#endif // SYSTEM_RDL_HANDLER_HPP
