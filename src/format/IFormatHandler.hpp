/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
    virtual ~IFormatHandler();

    virtual QString formatName() const = 0;
    virtual QStringList supportedExtensions() const = 0;
    virtual QString fileFilter() const = 0;

    virtual FormatResult read(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) = 0;
    virtual FormatResult write(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) = 0;
};

#endif // IFORMAT_HANDLER_HPP
