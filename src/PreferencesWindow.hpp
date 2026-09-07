/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#ifndef PREFERENCESWINDOW_HPP
#define PREFERENCESWINDOW_HPP

#include <QDialog>
#include <QString>
#include "ThemeManager.hpp"
#include "ui_preferences.h"

namespace Ui {
    class preferences;
}

class PreferencesWindow : public QDialog, private Ui::preferences
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(PreferencesWindow)

public:
    explicit PreferencesWindow(QWidget *parent = nullptr);
    ~PreferencesWindow() override = default;

    void setColourScheme(const QString &scheme);
    QString colourScheme() const;
    QString colorScheme() const { return colourScheme(); }

    void setColourBlindMode(bool enabled);
    bool isColourBlindMode() const;
    bool colorBlindMode() const { return isColourBlindMode(); }

    void setColourBlindType(ColorBlindMode mode);
    ColorBlindMode colourBlindType() const;
    void setColorBlindType(ColorBlindMode mode) { setColourBlindType(mode); }
    ColorBlindMode colorBlindType() const { return colourBlindType(); }

    void setLanguage(const QString &lang);
    QString language() const;

    void saveWindowStateToSettings();
    void restoreWindowStateFromSettings();

public slots:
    void accept() override;
    void reject() override;
    void apply();
    void onOpenThemesFolder();

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    QString m_colourScheme = QStringLiteral("solarized8");
    bool m_colourBlindMode = false;
    ColorBlindMode m_colourBlindType = ColorBlindMode::Universal;
    QString m_language = QStringLiteral("en");

    void updateUiFromState();
    void saveStateFromUi();
    void populateThemes();
    void populateLanguages();
    void populateColorBlindModes();
};

#endif // PREFERENCESWINDOW_HPP
