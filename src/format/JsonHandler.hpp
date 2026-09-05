/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#ifndef JSON_HANDLER_HPP
#define JSON_HANDLER_HPP

#include "IFormatHandler.hpp"

class JsonHandler : public IFormatHandler {
public:
    QString formatName() const override { return "JSON Schema"; }
    QStringList supportedExtensions() const override { return {"json"}; }
    QString fileFilter() const override {
        return "JSON Register Map (*.json)";
    }

    FormatResult read(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) override;
    FormatResult write(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) override;
};

#endif // JSON_HANDLER_HPP
