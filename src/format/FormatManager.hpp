/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
