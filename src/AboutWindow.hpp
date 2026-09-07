/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#ifndef ABOUTWINDOW_HPP
#define ABOUTWINDOW_HPP

#include <QDialog>
#include <QString>
#include "ui_about.h"

namespace Ui {
    class about;
}

class AboutWindow : public QDialog, private Ui::about
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(AboutWindow)

public:
    explicit AboutWindow(QWidget *parent = nullptr);
    ~AboutWindow() override = default;

    QString applicationVersion() const;
    QString applicationName() const;

    void saveWindowStateToSettings();
    void restoreWindowStateFromSettings();

public slots:
    void accept() override;

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void moveEvent(QMoveEvent *event) override;

private:
    void initContent();
};

#endif // ABOUTWINDOW_HPP
