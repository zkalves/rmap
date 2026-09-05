/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "RegMapTreeView.hpp"

RegMapTreeView::RegMapTreeView(QWidget *parent) : QTreeView(parent)
{
}

void RegMapTreeView::mousePressEvent(QMouseEvent *event)
{
        clearSelection();
        setCurrentIndex(rootIndex());
        QTreeView::mousePressEvent(event);
}
