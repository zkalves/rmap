/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
