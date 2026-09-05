/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#ifndef REGMAPTREEVIEW_HPP
#define REGMAPTREEVIEW_HPP

#include <QTreeView>
#include <QMouseEvent>

namespace Ui {
class RegMapTreeView;
}

class RegMapTreeView : public QTreeView
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(RegMapTreeView)

public:
    explicit RegMapTreeView(QWidget* parent = nullptr);
    ~RegMapTreeView() override = default;

protected:
    void mousePressEvent(QMouseEvent *event) override;
};

#endif // REGMAPTREEVIEW_HPP

