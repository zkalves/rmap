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
