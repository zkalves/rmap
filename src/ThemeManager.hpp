/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
#include <QJsonObject>
#include <QJsonDocument>

enum class ColorBlindMode {
    None = 0,         // Standard palette (color-blind mode off)
    Universal,        // Okabe-Ito / Wong universal barrier-free palette
    Protanopia,       // Red-blind / Red-weak (L-cone deficiency)
    Deuteranopia,     // Green-blind / Green-weak (M-cone deficiency)
    Tritanopia,       // Blue-blind / Blue-weak (S-cone deficiency)
    Achromatopsia     // Total color blindness (Monochrome / Grayscale luminance)
};

struct ColorBlindModeInfo {
    ColorBlindMode mode;
    QString id;
    QString name;
    QString description;
};

QString colorBlindModeToString(ColorBlindMode mode);
ColorBlindMode stringToColorBlindMode(const QString &str);
const QList<ColorBlindModeInfo>& availableColorBlindModes();

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

    // Optional custom color-blind overrides
    bool hasCustomColorBlind = false;
    AccessColors cbRw;
    AccessColors cbRo;
    AccessColors cbWo;
    AccessColors cbW1c;
    AccessColors cbRc;
    AccessColors cbNa;

    AccessColors getAccessColors(const QString &access, ColorBlindMode mode) const;
    AccessColors getAccessColors(const QString &access, bool colorBlind) const;
    AccessColors accessColors(const QString &access, ColorBlindMode mode) const { return getAccessColors(access, mode); }
    AccessColors accessColors(const QString &access, bool colorBlind) const { return getAccessColors(access, colorBlind); }
    QString generateStyleSheet() const;
    QPalette generatePalette() const;

    bool fromJson(const QJsonObject &obj);
    QJsonObject toJson() const;
    static ColorScheme createDefault(const QString &id);
    static QList<ColorScheme> builtInDefaults();
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

    ColorBlindMode colorBlindMode() const;
    QString colorBlindModeId() const;
    void setColorBlindMode(ColorBlindMode mode);
    void setColorBlindMode(const QString &modeId);
    void setColorBlindMode(bool enabled);

    AccessColors getAccessColors(const QString &access, ColorBlindMode mode) const;
    AccessColors getAccessColors(const QString &access, bool colorBlind) const;
    AccessColors accessColors(const QString &access, ColorBlindMode mode) const { return getAccessColors(access, mode); }
    AccessColors accessColors(const QString &access, bool colorBlind) const { return getAccessColors(access, colorBlind); }

    bool registerTheme(const ColorScheme &scheme);
    bool loadThemeFromJson(const QString &jsonPathOrContent);
    int scanThemesDir(const QString &dirPath);
    void scanThemes();
    void resetToDefaults();

    static QString userThemesDir();
    static QString localThemesDir();
    QStringList searchPaths() const;
    void addSearchPath(const QString &path);

signals:
    void themeChanged(const ColorScheme &newTheme);
    void themesUpdated();
    void colorBlindModeChanged(ColorBlindMode newMode);

private:
    ThemeManager();
    void registerThemes();

    QList<ColorScheme> m_themes;
    QStringList m_customSearchPaths;
    int m_currentIndex = 0;
    ColorBlindMode m_colorBlindMode = ColorBlindMode::None;
};

#endif // THEMEMANAGER_HPP
