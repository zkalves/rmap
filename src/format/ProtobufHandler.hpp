/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#ifndef PROTOBUF_HANDLER_HPP
#define PROTOBUF_HANDLER_HPP

#include "IFormatHandler.hpp"

class ProtobufHandler : public IFormatHandler {
public:
    QString formatName() const override;
    QStringList supportedExtensions() const override;
    QString fileFilter() const override;

    FormatResult read(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) override;
    FormatResult write(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) override;
};

#endif // PROTOBUF_HANDLER_HPP
