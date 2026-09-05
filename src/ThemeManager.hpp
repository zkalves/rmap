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

#ifndef THEMEMANAGER_HPP
#define THEMEMANAGER_HPP

#include <QObject>
#include <QString>
#include <QColor>
#include <QList>
#include <QPalette>

struct AccessColors {
    QColor bg;
    QColor border;
    QColor text;
};

struct ColorScheme {
    QString id;
    QString name;
    bool isDark = true;

    // Base UI Colors
    QColor windowBg;
    QColor panelBg;
    QColor altRowBg;
    QColor textColor;
    QColor textMuted;
    QColor headerBg;
    QColor headerText;
    QColor border;
    QColor selectionBg;
    QColor selectionText;
    QColor buttonBg;
    QColor buttonHover;
    QColor inputBg;
    QColor inputBorder;
    QColor errorBg;
    QColor errorBorder;
    QColor errorText;

    // Bitfield visualizer reserved slot colors
    QColor rsvdBg;
    QColor rsvdBorder;
    QColor rsvdStripe;
    QColor rsvdText;
    QColor rulerText;

    // Access Policy Colors
    AccessColors rwColors;
    AccessColors roColors;
    AccessColors woColors;
    AccessColors w1cColors;
    AccessColors rcColors;
    AccessColors naColors;

    AccessColors getAccessColors(const QString &access, bool colorBlind) const;
    AccessColors accessColors(const QString &access, bool colorBlind) const { return getAccessColors(access, colorBlind); }
    QString generateStyleSheet() const;
    QPalette generatePalette() const;
};

class ThemeManager : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(ThemeManager)

public:
    static ThemeManager& instance();

    const QList<ColorScheme>& availableThemes() const;
    QStringList themeIds() const;
    QStringList themeNames() const;

    const ColorScheme& currentTheme() const;
    QString currentThemeId() const;
    QString currentThemeName() const;

    bool setTheme(const QString &idOrName);
    AccessColors getAccessColors(const QString &access, bool colorBlind) const;
    AccessColors accessColors(const QString &access, bool colorBlind) const { return getAccessColors(access, colorBlind); }

signals:
    void themeChanged(const ColorScheme &newTheme);

private:
    ThemeManager();
    void registerThemes();

    QList<ColorScheme> m_themes;
    int m_currentIndex = 0;
};

#endif // THEMEMANAGER_HPP
