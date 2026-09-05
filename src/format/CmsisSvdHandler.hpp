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

#ifndef CMSIS_SVD_HANDLER_HPP
#define CMSIS_SVD_HANDLER_HPP

#include "IFormatHandler.hpp"

class CmsisSvdHandler : public IFormatHandler {
public:
    CmsisSvdHandler() = default;
    ~CmsisSvdHandler() override = default;

    QString formatName() const override { return "ARM CMSIS-SVD"; }
    QStringList supportedExtensions() const override { return {"svd"}; }
    QString fileFilter() const override { return "ARM CMSIS-SVD (*.svd)"; }

    FormatResult read(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) override;
    FormatResult write(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) override;
};

#endif // CMSIS_SVD_HANDLER_HPP
