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

#ifndef IPXACT_HANDLER_HPP
#define IPXACT_HANDLER_HPP

#include "IFormatHandler.hpp"

class IpxactHandler : public IFormatHandler {
public:
    QString formatName() const override { return "IP-XACT (IEEE 1685)"; }
    QStringList supportedExtensions() const override { return {"xml", "ipxact"}; }
    QString fileFilter() const override {
        return "IP-XACT IEEE 1685 (*.xml *.ipxact)";
    }

    FormatResult read(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) override;
    FormatResult write(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) override;
};

#endif // IPXACT_HANDLER_HPP
