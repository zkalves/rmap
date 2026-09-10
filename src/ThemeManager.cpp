/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "ThemeManager.hpp"
#include "AppSettings.hpp"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QCoreApplication>
#include <cstdlib>

QString colorBlindModeToString(ColorBlindMode mode)
{
    switch (mode) {
    case ColorBlindMode::Universal:    return QStringLiteral("universal");
    case ColorBlindMode::Protanopia:   return QStringLiteral("protanopia");
    case ColorBlindMode::Deuteranopia: return QStringLiteral("deuteranopia");
    case ColorBlindMode::Tritanopia:   return QStringLiteral("tritanopia");
    case ColorBlindMode::Achromatopsia: return QStringLiteral("achromatopsia");
    case ColorBlindMode::None:
    default:
        return QStringLiteral("none");
    }
}

ColorBlindMode stringToColorBlindMode(const QString &str)
{
    QString s = str.trimmed().toLower();
    if (s == "universal" || s == "okabe_ito" || s == "barrier_free" || s == "cvd" || s == "true" || s == "1") {
        return ColorBlindMode::Universal;
    } else if (s == "protanopia" || s == "protan" || s == "red_blind") {
        return ColorBlindMode::Protanopia;
    } else if (s == "deuteranopia" || s == "deutan" || s == "green_blind") {
        return ColorBlindMode::Deuteranopia;
    } else if (s == "tritanopia" || s == "tritan" || s == "blue_blind") {
        return ColorBlindMode::Tritanopia;
    } else if (s == "achromatopsia" || s == "monochrome" || s == "grayscale") {
        return ColorBlindMode::Achromatopsia;
    }
    return ColorBlindMode::None;
}

const QList<ColorBlindModeInfo>& availableColorBlindModes()
{
    static const QList<ColorBlindModeInfo> modes = {
        { ColorBlindMode::Universal, QStringLiteral("universal"), QStringLiteral("Universal (Barrier-Free Okabe-Ito)"), QStringLiteral("Universally distinguishable across all cone deficiencies") },
        { ColorBlindMode::Deuteranopia, QStringLiteral("deuteranopia"), QStringLiteral("Deuteranopia (Green-Blind / Weak)"), QStringLiteral("Optimized for medium-wavelength cone deficiency (~6% of males)") },
        { ColorBlindMode::Protanopia, QStringLiteral("protanopia"), QStringLiteral("Protanopia (Red-Blind / Weak)"), QStringLiteral("Optimized for long-wavelength cone deficiency (~2% of males)") },
        { ColorBlindMode::Tritanopia, QStringLiteral("tritanopia"), QStringLiteral("Tritanopia (Blue-Blind / Weak)"), QStringLiteral("Optimized for short-wavelength cone deficiency") },
        { ColorBlindMode::Achromatopsia, QStringLiteral("achromatopsia"), QStringLiteral("Achromatopsia (Monochrome / Grayscale)"), QStringLiteral("High-contrast luminance steps for complete color blindness") }
    };
    return modes;
}

AccessColors ColorScheme::getAccessColors(const QString &access, ColorBlindMode mode) const
{
    QString a = access.toUpper().trimmed();

    if (mode == ColorBlindMode::None) {
        if (a == "RW") return rwColors;
        if (a == "RO") return roColors;
        if (a == "WO") return woColors;
        if (a.startsWith("W1C") || a == "W1C" || a.startsWith("W0C") || a == "W0C" || a == "WC") return w1cColors;
        if (a.startsWith("W1S") || a == "W1S" || a.startsWith("W0S") || a == "WS") return w1cColors;
        if (a == "RC" || a == "RS") return rcColors;
        return naColors;
    }

    AccessColors c;

    // Check custom color-blind overrides first if available (for universal mode)
    if (hasCustomColorBlind && mode == ColorBlindMode::Universal) {
        if (a == "RO" || a == "RC" || a == "RS") return cbRo;
        if (a == "WO") return cbWo;
        if (a.startsWith("W1") || a.startsWith("W0") || a == "WC" || a == "WS") return cbW1c;
        if (a == "RW") return cbRw;
        return cbNa;
    }

    switch (mode) {
    case ColorBlindMode::Universal: {
        // Universal Okabe-Ito / Wong barrier-free palette
        if (a == "RW") {
            c.bg = QColor(128, 222, 234);
            c.border = QColor(0, 96, 100);
            c.text = QColor(0, 50, 60);
        } else if (a == "RO") {
            c.bg = QColor(159, 168, 218);
            c.border = QColor(26, 35, 126);
            c.text = QColor(15, 20, 80);
        } else if (a == "WO") {
            c.bg = QColor(206, 147, 216);
            c.border = QColor(74, 20, 140);
            c.text = QColor(50, 10, 80);
        } else if (a == "W1S" || a == "W0S" || a == "WS") {
            c.bg = QColor(255, 224, 130);
            c.border = QColor(183, 129, 3);
            c.text = QColor(74, 48, 0);
        } else if (a.startsWith("W1") || a.startsWith("W0") || a == "WC") {
            c.bg = QColor(255, 138, 101);
            c.border = QColor(191, 54, 12);
            c.text = QColor(100, 20, 0);
        } else if (a == "RC" || a == "RS") {
            c.bg = QColor(179, 157, 219);
            c.border = QColor(49, 27, 146);
            c.text = QColor(26, 0, 75);
        } else {
            c.bg = QColor(207, 216, 220);
            c.border = QColor(55, 71, 79);
            c.text = QColor(40, 40, 40);
        }
        break;
    }

    case ColorBlindMode::Protanopia: {
        // Protanopia: Red-blind / Red-weak (L-cone deficiency)
        if (a == "RW") {
            c.bg = QColor(147, 197, 253); // Sky Blue
            c.border = QColor(29, 78, 216);
            c.text = QColor(23, 37, 84);
        } else if (a == "RO") {
            c.bg = QColor(165, 243, 252); // Pale Cyan
            c.border = QColor(8, 145, 178);
            c.text = QColor(22, 78, 99);
        } else if (a == "WO") {
            c.bg = QColor(254, 240, 138); // Yellow
            c.border = QColor(161, 98, 7);
            c.text = QColor(69, 26, 3);
        } else if (a == "W1S" || a == "W0S" || a == "WS") {
            c.bg = QColor(254, 249, 195);
            c.border = QColor(133, 77, 14);
            c.text = QColor(66, 32, 6);
        } else if (a.startsWith("W1") || a.startsWith("W0") || a == "WC") {
            c.bg = QColor(254, 215, 170); // Warm Peach
            c.border = QColor(194, 65, 12);
            c.text = QColor(67, 20, 7);
        } else if (a == "RC" || a == "RS") {
            c.bg = QColor(199, 210, 254); // Indigo
            c.border = QColor(67, 56, 202);
            c.text = QColor(30, 27, 75);
        } else {
            c.bg = QColor(226, 232, 240);
            c.border = QColor(71, 85, 105);
            c.text = QColor(15, 23, 42);
        }
        break;
    }

    case ColorBlindMode::Deuteranopia: {
        // Deuteranopia: Green-blind / Green-weak (M-cone deficiency)
        if (a == "RW") {
            c.bg = QColor(153, 246, 228); // Teal
            c.border = QColor(15, 118, 110);
            c.text = QColor(19, 78, 74);
        } else if (a == "RO") {
            c.bg = QColor(191, 219, 254); // Blue
            c.border = QColor(30, 64, 175);
            c.text = QColor(23, 37, 84);
        } else if (a == "WO") {
            c.bg = QColor(253, 224, 71); // Amber Yellow
            c.border = QColor(161, 98, 7);
            c.text = QColor(66, 32, 6);
        } else if (a == "W1S" || a == "W0S" || a == "WS") {
            c.bg = QColor(254, 240, 138);
            c.border = QColor(180, 83, 9);
            c.text = QColor(69, 26, 3);
        } else if (a.startsWith("W1") || a.startsWith("W0") || a == "WC") {
            c.bg = QColor(251, 146, 60); // Tangerine
            c.border = QColor(154, 52, 18);
            c.text = QColor(67, 20, 7);
        } else if (a == "RC" || a == "RS") {
            c.bg = QColor(221, 214, 254); // Violet
            c.border = QColor(109, 40, 217);
            c.text = QColor(46, 16, 101);
        } else {
            c.bg = QColor(226, 232, 240);
            c.border = QColor(71, 85, 105);
            c.text = QColor(15, 23, 42);
        }
        break;
    }

    case ColorBlindMode::Tritanopia: {
        // Tritanopia: Blue-blind / Blue-weak (S-cone deficiency)
        if (a == "RW") {
            c.bg = QColor(204, 251, 241); // Mint Cyan
            c.border = QColor(15, 118, 110);
            c.text = QColor(19, 78, 74);
        } else if (a == "RO") {
            c.bg = QColor(254, 205, 211); // Rose Red
            c.border = QColor(190, 18, 60);
            c.text = QColor(76, 5, 25);
        } else if (a == "WO") {
            c.bg = QColor(254, 215, 170); // Warm Peach
            c.border = QColor(194, 65, 12);
            c.text = QColor(67, 20, 7);
        } else if (a == "W1S" || a == "W0S" || a == "WS") {
            c.bg = QColor(255, 228, 230);
            c.border = QColor(159, 18, 57);
            c.text = QColor(76, 5, 25);
        } else if (a.startsWith("W1") || a.startsWith("W0") || a == "WC") {
            c.bg = QColor(251, 207, 232); // Magenta
            c.border = QColor(157, 23, 77);
            c.text = QColor(80, 7, 36);
        } else if (a == "RC" || a == "RS") {
            c.bg = QColor(245, 208, 254); // Fuchsia
            c.border = QColor(134, 25, 143);
            c.text = QColor(74, 4, 78);
        } else {
            c.bg = QColor(226, 232, 240);
            c.border = QColor(51, 65, 85);
            c.text = QColor(15, 23, 42);
        }
        break;
    }

    case ColorBlindMode::Achromatopsia: {
        // Achromatopsia: Complete color blindness (Distinct luminance steps)
        if (a == "RW") {
            c.bg = QColor(224, 224, 224); // 88% luminance
            c.border = QColor(0, 0, 0);
            c.text = QColor(0, 0, 0);
        } else if (a == "RO") {
            c.bg = QColor(255, 255, 255); // 100% luminance
            c.border = QColor(0, 0, 0);
            c.text = QColor(0, 0, 0);
        } else if (a == "WO") {
            c.bg = QColor(34, 34, 34); // 13% luminance
            c.border = QColor(255, 255, 255);
            c.text = QColor(255, 255, 255);
        } else if (a == "W1S" || a == "W0S" || a == "WS") {
            c.bg = QColor(136, 136, 136); // 53% luminance
            c.border = QColor(0, 0, 0);
            c.text = QColor(255, 255, 255);
        } else if (a.startsWith("W1") || a.startsWith("W0") || a == "WC") {
            c.bg = QColor(85, 85, 85); // 33% luminance
            c.border = QColor(255, 255, 255);
            c.text = QColor(255, 255, 255);
        } else if (a == "RC" || a == "RS") {
            c.bg = QColor(170, 170, 170); // 67% luminance
            c.border = QColor(0, 0, 0);
            c.text = QColor(0, 0, 0);
        } else {
            c.bg = QColor(51, 51, 51); // 20% luminance
            c.border = QColor(119, 119, 119);
            c.text = QColor(204, 204, 204);
        }
        break;
    }

    default:
        break;
    }

    return c;
}

AccessColors ColorScheme::getAccessColors(const QString &access, bool colorBlind) const
{
    return getAccessColors(access, colorBlind ? ColorBlindMode::Universal : ColorBlindMode::None);
}

QPalette ColorScheme::generatePalette() const noexcept
{
    QPalette p;
    p.setColor(QPalette::Window, windowBg);
    p.setColor(QPalette::WindowText, textColor);
    p.setColor(QPalette::Base, panelBg);
    p.setColor(QPalette::AlternateBase, altRowBg);
    p.setColor(QPalette::ToolTipBase, headerBg);
    p.setColor(QPalette::ToolTipText, textColor);
    p.setColor(QPalette::Text, textColor);
    p.setColor(QPalette::Button, buttonBg);
    p.setColor(QPalette::ButtonText, textColor);
    p.setColor(QPalette::BrightText, errorText);
    p.setColor(QPalette::Link, roColors.border);
    p.setColor(QPalette::Highlight, selectionBg);
    p.setColor(QPalette::HighlightedText, selectionText);
    p.setColor(QPalette::PlaceholderText, textMuted);
    return p;
}

QString ColorScheme::generateStyleSheet() const
{
    QString qss;
    qss += QString("QMainWindow, QDialog { background-color: %1; color: %2; }\n")
               .arg(windowBg.name(), textColor.name());

    qss += QString("QTreeView, QTableView { background-color: %1; alternate-background-color: %2; color: %3; gridline-color: %4; border: 1px solid %4; selection-background-color: %5; selection-color: %6; }\n")
               .arg(panelBg.name(), altRowBg.name(), textColor.name(), border.name(), selectionBg.name(), selectionText.name());

    qss += QString("QHeaderView::section { background-color: %1; color: %2; border: 1px solid %3; padding: 4px 6px; font-weight: bold; }\n")
               .arg(headerBg.name(), headerText.name(), border.name());

    qss += QString("QToolBar { background-color: %1; border-bottom: 1px solid %2; spacing: 4px; padding: 3px; }\n")
               .arg(headerBg.name(), border.name());

    qss += QString("QToolButton { background-color: transparent; border: 1px solid transparent; border-radius: 4px; padding: 3px; color: %1; }\n")
               .arg(textColor.name());
    qss += QString("QToolButton:hover { background-color: %1; border: 1px solid %2; }\n")
               .arg(buttonHover.name(), border.name());
    qss += QString("QToolButton:pressed { background-color: %1; }\n")
               .arg(selectionBg.name());

    qss += QString("QMenuBar { background-color: %1; color: %2; border-bottom: 1px solid %3; }\n")
               .arg(windowBg.name(), textColor.name(), border.name());
    qss += QString("QMenuBar::item:selected { background-color: %1; color: %2; border-radius: 3px; }\n")
               .arg(buttonHover.name(), selectionText.name());

    qss += QString("QMenu { background-color: %1; color: %2; border: 1px solid %3; padding: 4px; }\n")
               .arg(headerBg.name(), textColor.name(), border.name());
    qss += QString("QMenu::item:selected { background-color: %1; color: %2; border-radius: 3px; }\n")
               .arg(selectionBg.name(), selectionText.name());
    qss += QString("QMenu::separator { height: 1px; background-color: %1; margin: 4px 8px; }\n")
               .arg(border.name());

    QString inputTextColor = isDark ? QString("#f1f5f9") : textColor.name();
    qss += QString("QLineEdit, QSpinBox, QComboBox { background-color: %1; color: %2; border: 1px solid %3; border-radius: 3px; padding: 3px 6px; }\n")
               .arg(inputBg.name(), inputTextColor, inputBorder.name());
    qss += QString("QLineEdit:focus, QSpinBox:focus, QComboBox:focus { border: 1px solid %1; }\n")
               .arg(selectionBg.name());

    qss += QString("QPushButton { background-color: %1; color: %2; border: 1px solid %3; border-radius: 4px; padding: 4px 12px; }\n")
               .arg(buttonBg.name(), textColor.name(), border.name());
    qss += QString("QPushButton:hover { background-color: %1; }\n")
               .arg(buttonHover.name());
    qss += QString("QPushButton:pressed { background-color: %1; color: %2; }\n")
               .arg(selectionBg.name(), selectionText.name());

    qss += QString("QGroupBox { border: 1px solid %1; border-radius: 5px; margin-top: 10px; font-weight: bold; color: %2; }\n")
               .arg(border.name(), textColor.name());
    qss += QString("QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 0 5px; }\n");

    qss += QString("QStatusBar { background-color: %1; color: %2; border-top: 1px solid %3; }\n")
               .arg(headerBg.name(), textMuted.name(), border.name());

    qss += QString("QSplitter::handle { background-color: %1; }\n")
               .arg(border.name());

    QString vsb = QStringLiteral("QScrollBar:vertical { background: %1; width: 12px; margin: 0px; }\n"
                                 "QScrollBar::handle:vertical { background: %2; min-height: 20px; border-radius: 4px; margin: 2px; }\n"
                                 "QScrollBar::handle:vertical:hover { background: %3; }\n"
                                 "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }\n");
    qss += vsb.arg(windowBg.name(), border.name(), buttonHover.name());

    QString hsb = QStringLiteral("QScrollBar:horizontal { background: %1; height: 12px; margin: 0px; }\n"
                                 "QScrollBar::handle:horizontal { background: %2; min-width: 20px; border-radius: 4px; margin: 2px; }\n"
                                 "QScrollBar::handle:horizontal:hover { background: %3; }\n"
                                 "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; }\n");
    qss += hsb.arg(windowBg.name(), border.name(), buttonHover.name());

    return qss;
}

static QColor parseColor(const QJsonValue &val, const QColor &fallback)
{
    if (val.isString()) {
        QString s = val.toString().trimmed();
        if (!s.isEmpty()) {
            QColor c(s);
            if (c.isValid()) {
                return c;
            }
        }
    }
    return fallback;
}

bool ColorScheme::fromJson(const QJsonObject &obj)
{
    QString schemeId = obj.value("id").toString().trimmed();
    if (schemeId.isEmpty()) {
        return false;
    }

    *this = ColorScheme::createDefault(schemeId);
    this->id = schemeId;

    if (obj.contains("name") && obj["name"].isString()) {
        this->name = obj["name"].toString().trimmed();
    }
    if (obj.contains("isDark")) {
        this->isDark = obj["isDark"].toBool(this->isDark);
    }

    QJsonObject baseObj = obj.value("base").toObject();
    auto getBaseColor = [&](const QString &key, const QColor &def) -> QColor {
        if (baseObj.contains(key)) return parseColor(baseObj.value(key), def);
        if (obj.contains(key)) return parseColor(obj.value(key), def);
        return def;
    };

    windowBg      = getBaseColor("windowBg", windowBg);
    panelBg       = getBaseColor("panelBg", panelBg);
    altRowBg      = getBaseColor("altRowBg", altRowBg);
    textColor     = getBaseColor("textColor", textColor);
    textMuted     = getBaseColor("textMuted", textMuted);
    headerBg      = getBaseColor("headerBg", headerBg);
    headerText    = getBaseColor("headerText", headerText);
    border        = getBaseColor("border", border);
    selectionBg   = getBaseColor("selectionBg", selectionBg);
    selectionText = getBaseColor("selectionText", selectionText);
    buttonBg      = getBaseColor("buttonBg", buttonBg);
    buttonHover   = getBaseColor("buttonHover", buttonHover);
    inputBg       = getBaseColor("inputBg", inputBg);
    inputBorder   = getBaseColor("inputBorder", inputBorder);
    errorBg       = getBaseColor("errorBg", errorBg);
    errorBorder   = getBaseColor("errorBorder", errorBorder);
    errorText     = getBaseColor("errorText", errorText);

    QJsonObject rsvdObj = obj.value("reserved").toObject();
    auto getRsvdColor = [&](const QString &key, const QString &flatKey, const QColor &def) -> QColor {
        if (rsvdObj.contains(key)) return parseColor(rsvdObj.value(key), def);
        if (obj.contains(flatKey)) return parseColor(obj.value(flatKey), def);
        return def;
    };

    rsvdBg     = getRsvdColor("bg", "rsvdBg", rsvdBg);
    rsvdBorder = getRsvdColor("border", "rsvdBorder", rsvdBorder);
    rsvdStripe = getRsvdColor("stripe", "rsvdStripe", rsvdStripe);
    rsvdText   = getRsvdColor("text", "rsvdText", rsvdText);
    rulerText  = getRsvdColor("rulerText", "rulerText", rulerText);

    QJsonObject apObj = obj.value("accessPolicies").toObject();
    auto getAccess = [&](const QString &key, const AccessColors &def) -> AccessColors {
        QJsonObject o = apObj.value(key).toObject();
        if (o.isEmpty() && obj.contains(key) && obj.value(key).isObject()) {
            o = obj.value(key).toObject();
        }
        if (o.isEmpty()) return def;
        AccessColors res = def;
        if (o.contains("bg")) res.bg = parseColor(o.value("bg"), def.bg);
        if (o.contains("border")) res.border = parseColor(o.value("border"), def.border);
        if (o.contains("text")) res.text = parseColor(o.value("text"), def.text);
        return res;
    };

    rwColors  = getAccess("rw", rwColors);
    roColors  = getAccess("ro", roColors);
    woColors  = getAccess("wo", woColors);
    w1cColors = getAccess("w1c", w1cColors);
    rcColors  = getAccess("rc", rcColors);
    naColors  = getAccess("na", naColors);

    if (obj.contains("colorBlind") && obj.value("colorBlind").isObject()) {
        QJsonObject cbObj = obj.value("colorBlind").toObject();
        auto getCb = [&](const QString &key, const AccessColors &def) -> AccessColors {
            QJsonObject o = cbObj.value(key).toObject();
            if (o.isEmpty()) return def;
            AccessColors res = def;
            if (o.contains("bg")) res.bg = parseColor(o.value("bg"), def.bg);
            if (o.contains("border")) res.border = parseColor(o.value("border"), def.border);
            if (o.contains("text")) res.text = parseColor(o.value("text"), def.text);
            return res;
        };
        hasCustomColorBlind = true;
        AccessColors defRo  = { QColor(159, 168, 218), QColor(26, 35, 126), QColor(15, 20, 80) };
        AccessColors defWo  = { QColor(206, 147, 216), QColor(74, 20, 140), QColor(50, 10, 80) };
        AccessColors defW1c = { QColor(255, 138, 101), QColor(191, 54, 12), QColor(100, 20, 0) };
        AccessColors defRw  = { QColor(128, 222, 234), QColor(0, 96, 100), QColor(0, 50, 60) };
        AccessColors defNa  = { QColor(207, 216, 220), QColor(55, 71, 79), QColor(40, 40, 40) };

        cbRo  = getCb("ro", defRo);
        cbWo  = getCb("wo", defWo);
        cbW1c = getCb("w1c", defW1c);
        cbRw  = getCb("rw", defRw);
        cbRc  = getCb("rc", cbRo);
        cbNa  = getCb("na", defNa);
    }

    return true;
}

QJsonObject ColorScheme::toJson() const
{
    QJsonObject root;
    root["id"] = id;
    root["name"] = name;
    root["isDark"] = isDark;

    QJsonObject base;
    base["windowBg"]      = windowBg.name();
    base["panelBg"]       = panelBg.name();
    base["altRowBg"]      = altRowBg.name();
    base["textColor"]     = textColor.name();
    base["textMuted"]     = textMuted.name();
    base["headerBg"]      = headerBg.name();
    base["headerText"]    = headerText.name();
    base["border"]        = border.name();
    base["selectionBg"]   = selectionBg.name();
    base["selectionText"] = selectionText.name();
    base["buttonBg"]      = buttonBg.name();
    base["buttonHover"]   = buttonHover.name();
    base["inputBg"]       = inputBg.name();
    base["inputBorder"]   = inputBorder.name();
    base["errorBg"]       = errorBg.name();
    base["errorBorder"]   = errorBorder.name();
    base["errorText"]     = errorText.name();
    root["base"] = base;

    QJsonObject rsvd;
    rsvd["bg"]        = rsvdBg.name();
    rsvd["border"]    = rsvdBorder.name();
    rsvd["stripe"]    = rsvdStripe.name();
    rsvd["text"]      = rsvdText.name();
    rsvd["rulerText"] = rulerText.name();
    root["reserved"] = rsvd;

    auto accessToJson = [](const AccessColors &c) noexcept -> QJsonObject {
        QJsonObject o;
        o["bg"] = c.bg.name();
        o["border"] = c.border.name();
        o["text"] = c.text.name();
        return o;
    };

    QJsonObject ap;
    ap["rw"]  = accessToJson(rwColors);
    ap["ro"]  = accessToJson(roColors);
    ap["wo"]  = accessToJson(woColors);
    ap["w1c"] = accessToJson(w1cColors);
    ap["rc"]  = accessToJson(rcColors);
    ap["na"]  = accessToJson(naColors);
    root["accessPolicies"] = ap;

    if (hasCustomColorBlind) {
        QJsonObject cb;
        cb["ro"]  = accessToJson(cbRo);
        cb["wo"]  = accessToJson(cbWo);
        cb["w1c"] = accessToJson(cbW1c);
        cb["rw"]  = accessToJson(cbRw);
        cb["rc"]  = accessToJson(cbRc);
        cb["na"]  = accessToJson(cbNa);
        root["colorBlind"] = cb;
    }

    return root;
}

QList<ColorScheme> ColorScheme::builtInDefaults() noexcept
{
    QList<ColorScheme> list;

    // 1. Solarized 8 (Dark) — Ethan Schoonover / Lifepillar (Default!)
    {
        ColorScheme s;
        s.id = "solarized8";
        s.name = "Solarized 8 (Dark)";
        s.isDark = true;

        s.windowBg       = QColor("#002b36");
        s.panelBg        = QColor("#002b36");
        s.altRowBg       = QColor("#073642");
        s.textColor      = QColor("#839496");
        s.textMuted      = QColor("#586e75");
        s.headerBg       = QColor("#073642");
        s.headerText     = QColor("#93a1a1");
        s.border         = QColor("#0d4857");
        s.selectionBg    = QColor("#1e5666");
        s.selectionText  = QColor("#fdf6e3");
        s.buttonBg       = QColor("#073642");
        s.buttonHover    = QColor("#0e4654");
        s.inputBg        = QColor("#073642");
        s.inputBorder    = QColor("#586e75");
        s.errorBg        = QColor("#42161b");
        s.errorBorder    = QColor("#dc322f");
        s.errorText      = QColor("#fdf6e3");

        s.rsvdBg         = QColor("#073642");
        s.rsvdBorder     = QColor("#586e75");
        s.rsvdStripe     = QColor("#002b36");
        s.rsvdText       = QColor("#839496");
        s.rulerText      = QColor("#586e75");

        s.rwColors       = { QColor("#103e2e"), QColor("#859900"), QColor("#859900") };
        s.roColors       = { QColor("#0e3c54"), QColor("#268bd2"), QColor("#268bd2") };
        s.woColors       = { QColor("#44281e"), QColor("#cb4b16"), QColor("#cb4b16") };
        s.w1cColors      = { QColor("#3e3518"), QColor("#b58900"), QColor("#b58900") };
        s.rcColors       = { QColor("#2d274c"), QColor("#6c71c4"), QColor("#6c71c4") };
        s.naColors       = { QColor("#073642"), QColor("#586e75"), QColor("#839496") };

        list.append(s);
    }

    // 2. Solarized 8 (Light)
    {
        ColorScheme s;
        s.id = "solarized8_light";
        s.name = "Solarized 8 (Light)";
        s.isDark = false;

        s.windowBg       = QColor("#fdf6e3");
        s.panelBg        = QColor("#fdf6e3");
        s.altRowBg       = QColor("#eee8d5");
        s.textColor      = QColor("#657b83");
        s.textMuted      = QColor("#93a1a1");
        s.headerBg       = QColor("#eee8d5");
        s.headerText     = QColor("#586e75");
        s.border         = QColor("#d3cbb7");
        s.selectionBg    = QColor("#268bd2");
        s.selectionText  = QColor("#fdf6e3");
        s.buttonBg       = QColor("#eee8d5");
        s.buttonHover    = QColor("#e0d7be");
        s.inputBg        = QColor("#ffffff");
        s.inputBorder    = QColor("#93a1a1");
        s.errorBg        = QColor("#fadbd8");
        s.errorBorder    = QColor("#dc322f");
        s.errorText      = QColor("#78281f");

        s.rsvdBg         = QColor("#eee8d5");
        s.rsvdBorder     = QColor("#93a1a1");
        s.rsvdStripe     = QColor("#fdf6e3");
        s.rsvdText       = QColor("#657b83");
        s.rulerText      = QColor("#93a1a1");

        s.rwColors       = { QColor("#dcedc8"), QColor("#859900"), QColor("#33691e") };
        s.roColors       = { QColor("#bbdefb"), QColor("#268bd2"), QColor("#0d47a1") };
        s.woColors       = { QColor("#ffe0b2"), QColor("#cb4b16"), QColor("#bf360c") };
        s.w1cColors      = { QColor("#fff9c4"), QColor("#b58900"), QColor("#e65100") };
        s.rcColors       = { QColor("#e1bee7"), QColor("#6c71c4"), QColor("#4a148c") };
        s.naColors       = { QColor("#eee8d5"), QColor("#93a1a1"), QColor("#657b83") };

        list.append(s);
    }

    // 3. Nord (Dark)
    {
        ColorScheme s;
        s.id = "nord";
        s.name = "Nord";
        s.isDark = true;

        s.windowBg       = QColor("#2e3440");
        s.panelBg        = QColor("#2e3440");
        s.altRowBg       = QColor("#3b4252");
        s.textColor      = QColor("#d8dee9");
        s.textMuted      = QColor("#4c566a");
        s.headerBg       = QColor("#3b4252");
        s.headerText     = QColor("#eceff4");
        s.border         = QColor("#434c5e");
        s.selectionBg    = QColor("#4c566a");
        s.selectionText  = QColor("#88c0d0");
        s.buttonBg       = QColor("#3b4252");
        s.buttonHover    = QColor("#434c5e");
        s.inputBg        = QColor("#3b4252");
        s.inputBorder    = QColor("#4c566a");
        s.errorBg        = QColor("#482e34");
        s.errorBorder    = QColor("#bf616a");
        s.errorText      = QColor("#eceff4");

        s.rsvdBg         = QColor("#3b4252");
        s.rsvdBorder     = QColor("#4c566a");
        s.rsvdStripe     = QColor("#2e3440");
        s.rsvdText       = QColor("#d8dee9");
        s.rulerText      = QColor("#4c566a");

        s.rwColors       = { QColor("#263d33"), QColor("#a3be8c"), QColor("#a3be8c") };
        s.roColors       = { QColor("#213945"), QColor("#88c0d0"), QColor("#88c0d0") };
        s.woColors       = { QColor("#3d2b28"), QColor("#d08770"), QColor("#d08770") };
        s.w1cColors      = { QColor("#3d3725"), QColor("#ebcb8b"), QColor("#ebcb8b") };
        s.rcColors       = { QColor("#35263d"), QColor("#b48ead"), QColor("#b48ead") };
        s.naColors       = { QColor("#3b4252"), QColor("#4c566a"), QColor("#d8dee9") };

        list.append(s);
    }

    // 4. Dracula (Dark)
    {
        ColorScheme s;
        s.id = "dracula";
        s.name = "Dracula";
        s.isDark = true;

        s.windowBg       = QColor("#282a36");
        s.panelBg        = QColor("#282a36");
        s.altRowBg       = QColor("#343746");
        s.textColor      = QColor("#f8f8f2");
        s.textMuted      = QColor("#6272a4");
        s.headerBg       = QColor("#44475a");
        s.headerText     = QColor("#f8f8f2");
        s.border         = QColor("#44475a");
        s.selectionBg    = QColor("#44475a");
        s.selectionText  = QColor("#50fa7b");
        s.buttonBg       = QColor("#44475a");
        s.buttonHover    = QColor("#5a5f78");
        s.inputBg        = QColor("#343746");
        s.inputBorder    = QColor("#6272a4");
        s.errorBg        = QColor("#4a2128");
        s.errorBorder    = QColor("#ff5555");
        s.errorText      = QColor("#f8f8f2");

        s.rsvdBg         = QColor("#343746");
        s.rsvdBorder     = QColor("#6272a4");
        s.rsvdStripe     = QColor("#282a36");
        s.rsvdText       = QColor("#f8f8f2");
        s.rulerText      = QColor("#6272a4");

        s.rwColors       = { QColor("#1e3828"), QColor("#50fa7b"), QColor("#50fa7b") };
        s.roColors       = { QColor("#1c353d"), QColor("#8be9fd"), QColor("#8be9fd") };
        s.woColors       = { QColor("#3d2f20"), QColor("#ffb86c"), QColor("#ffb86c") };
        s.w1cColors      = { QColor("#3d3c22"), QColor("#f1fa8c"), QColor("#f1fa8c") };
        s.rcColors       = { QColor("#352542"), QColor("#bd93f9"), QColor("#bd93f9") };
        s.naColors       = { QColor("#343746"), QColor("#6272a4"), QColor("#f8f8f2") };

        list.append(s);
    }

    // 5. Monokai (Dark)
    {
        ColorScheme s;
        s.id = "monokai";
        s.name = "Monokai";
        s.isDark = true;

        s.windowBg       = QColor("#272822");
        s.panelBg        = QColor("#272822");
        s.altRowBg       = QColor("#383830");
        s.textColor      = QColor("#f8f8f2");
        s.textMuted      = QColor("#75715e");
        s.headerBg       = QColor("#3e3d32");
        s.headerText     = QColor("#f8f8f2");
        s.border         = QColor("#49483e");
        s.selectionBg    = QColor("#49483e");
        s.selectionText  = QColor("#a6e22e");
        s.buttonBg       = QColor("#3e3d32");
        s.buttonHover    = QColor("#525043");
        s.inputBg        = QColor("#383830");
        s.inputBorder    = QColor("#75715e");
        s.errorBg        = QColor("#4b1d2b");
        s.errorBorder    = QColor("#f92672");
        s.errorText      = QColor("#f8f8f2");

        s.rsvdBg         = QColor("#383830");
        s.rsvdBorder     = QColor("#75715e");
        s.rsvdStripe     = QColor("#272822");
        s.rsvdText       = QColor("#f8f8f2");
        s.rulerText      = QColor("#75715e");

        s.rwColors       = { QColor("#283a1b"), QColor("#a6e22e"), QColor("#a6e22e") };
        s.roColors       = { QColor("#1b373f"), QColor("#66d9ef"), QColor("#66d9ef") };
        s.woColors       = { QColor("#3f2b1a"), QColor("#fd971f"), QColor("#fd971f") };
        s.w1cColors      = { QColor("#3d3a1f"), QColor("#e6db74"), QColor("#e6db74") };
        s.rcColors       = { QColor("#322045"), QColor("#ae81ff"), QColor("#ae81ff") };
        s.naColors       = { QColor("#383830"), QColor("#75715e"), QColor("#f8f8f2") };

        list.append(s);
    }

    // 6. Classic Light
    {
        ColorScheme s;
        s.id = "classic";
        s.name = "Classic Light";
        s.isDark = false;

        s.windowBg       = QColor("#f5f6f8");
        s.panelBg        = QColor("#ffffff");
        s.altRowBg       = QColor("#f0f2f5");
        s.textColor      = QColor("#212121");
        s.textMuted      = QColor("#757575");
        s.headerBg       = QColor("#e9ecef");
        s.headerText     = QColor("#212121");
        s.border         = QColor("#dcdfe4");
        s.selectionBg    = QColor("#1976d2");
        s.selectionText  = QColor("#ffffff");
        s.buttonBg       = QColor("#e9ecef");
        s.buttonHover    = QColor("#dde1e5");
        s.inputBg        = QColor("#ffffff");
        s.inputBorder    = QColor("#cccccc");
        s.errorBg        = QColor("#ffc8c8");
        s.errorBorder    = QColor("#d32f2f");
        s.errorText      = QColor("#5f1313");

        s.rsvdBg         = QColor("#666666");
        s.rsvdBorder     = QColor("#484848");
        s.rsvdStripe     = QColor("#555555");
        s.rsvdText       = QColor("#ebeef2");
        s.rulerText      = QColor("#646464");

        s.rwColors       = { QColor("#a5d6a7"), QColor("#1b5e20"), QColor("#1b5e20") };
        s.roColors       = { QColor("#bbdefb"), QColor("#0d47a1"), QColor("#0d47a1") };
        s.woColors       = { QColor("#ffcc80"), QColor("#e65100"), QColor("#bf360c") };
        s.w1cColors      = { QColor("#fff59d"), QColor("#f57f17"), QColor("#e65100") };
        s.rcColors       = { QColor("#d1c4e9"), QColor("#512da8"), QColor("#311b92") };
        s.naColors       = { QColor("#e4e7eb"), QColor("#757575"), QColor("#606060") };

        list.append(s);
    }

    // 7. High Contrast (Dark)
    {
        ColorScheme s;
        s.id = "high_contrast_dark";
        s.name = "High Contrast (Dark)";
        s.isDark = true;

        s.windowBg       = QColor("#000000");
        s.panelBg        = QColor("#000000");
        s.altRowBg       = QColor("#121212");
        s.textColor      = QColor("#ffffff");
        s.textMuted      = QColor("#ffff00");
        s.headerBg       = QColor("#1a1a1a");
        s.headerText     = QColor("#ffffff");
        s.border         = QColor("#ffffff");
        s.selectionBg    = QColor("#0055d4");
        s.selectionText  = QColor("#ffffff");
        s.buttonBg       = QColor("#1f1f1f");
        s.buttonHover    = QColor("#333333");
        s.inputBg        = QColor("#000000");
        s.inputBorder    = QColor("#ffffff");
        s.errorBg        = QColor("#550000");
        s.errorBorder    = QColor("#ff0000");
        s.errorText      = QColor("#ffffff");

        s.rsvdBg         = QColor("#1a1a1a");
        s.rsvdBorder     = QColor("#ffffff");
        s.rsvdStripe     = QColor("#000000");
        s.rsvdText       = QColor("#ffff00");
        s.rulerText      = QColor("#ffffff");

        s.rwColors       = { QColor("#003300"), QColor("#00ff00"), QColor("#00ff00") };
        s.roColors       = { QColor("#002244"), QColor("#00ffff"), QColor("#00ffff") };
        s.woColors       = { QColor("#442200"), QColor("#ff8800"), QColor("#ff8800") };
        s.w1cColors      = { QColor("#444400"), QColor("#ffff00"), QColor("#ffff00") };
        s.rcColors       = { QColor("#440044"), QColor("#ff00ff"), QColor("#ff00ff") };
        s.naColors       = { QColor("#222222"), QColor("#aaaaaa"), QColor("#ffffff") };

        list.append(s);
    }

    // 8. High Contrast (Light)
    {
        ColorScheme s;
        s.id = "high_contrast_light";
        s.name = "High Contrast (Light)";
        s.isDark = false;

        s.windowBg       = QColor("#ffffff");
        s.panelBg        = QColor("#ffffff");
        s.altRowBg       = QColor("#f0f0f0");
        s.textColor      = QColor("#000000");
        s.textMuted      = QColor("#000000");
        s.headerBg       = QColor("#e0e0e0");
        s.headerText     = QColor("#000000");
        s.border         = QColor("#000000");
        s.selectionBg    = QColor("#000088");
        s.selectionText  = QColor("#ffffff");
        s.buttonBg       = QColor("#e6e6e6");
        s.buttonHover    = QColor("#cccccc");
        s.inputBg        = QColor("#ffffff");
        s.inputBorder    = QColor("#000000");
        s.errorBg        = QColor("#ffcccc");
        s.errorBorder    = QColor("#cc0000");
        s.errorText      = QColor("#000000");

        s.rsvdBg         = QColor("#cccccc");
        s.rsvdBorder     = QColor("#000000");
        s.rsvdStripe     = QColor("#ffffff");
        s.rsvdText       = QColor("#000000");
        s.rulerText      = QColor("#000000");

        s.rwColors       = { QColor("#c8f7c5"), QColor("#006600"), QColor("#004d00") };
        s.roColors       = { QColor("#cce5ff"), QColor("#0000aa"), QColor("#000088") };
        s.woColors       = { QColor("#ffe0cc"), QColor("#cc4400"), QColor("#993300") };
        s.w1cColors      = { QColor("#fff2b3"), QColor("#997a00"), QColor("#665200") };
        s.rcColors       = { QColor("#edd4ff"), QColor("#660099"), QColor("#4d0073") };
        s.naColors       = { QColor("#e0e0e0"), QColor("#000000"), QColor("#000000") };

        list.append(s);
    }

    return list;
}

ColorScheme ColorScheme::createDefault(const QString &id)
{
    QString key = id.trimmed().toLower();
    if (key.isEmpty() || key == "solarized" || key == "solarized8" || key == "solarized8_dark" || key == "solarized_dark" || key == "default" || key == "dark") {
        key = "solarized8";
    } else if (key == "solarized8_light" || key == "solarized_light" || key == "light") {
        key = "solarized8_light";
    } else if (key == "high_contrast" || key == "high_contrast_dark" || key == "high-contrast" || key == "high-contrast-dark") {
        key = "high_contrast_dark";
    } else if (key == "high_contrast_light" || key == "high-contrast-light") {
        key = "high_contrast_light";
    }

    for (const auto &s : builtInDefaults()) {
        if (s.id.toLower() == key) {
            return s;
        }
    }

    ColorScheme fallback = builtInDefaults().first();
    fallback.id = id;
    fallback.name = id;
    return fallback;
}

ThemeManager& ThemeManager::instance()
{
    static ThemeManager mgr;
    return mgr;
}

ThemeManager::ThemeManager()
{
    registerThemes();
    m_currentIndex = 0; // Default to Solarized 8 (Dark)
}

void ThemeManager::registerThemes()
{
    m_themes.clear();

    // 1. Built-in C++ hardcoded defaults
    for (const auto &s : ColorScheme::builtInDefaults()) {
        m_themes.append(s);
    }

    // 2. Embedded Qt resource files :/themes/*.json
    QDir qrcDir(":/themes");
    if (qrcDir.exists()) {
        for (const QFileInfo &fi : qrcDir.entryInfoList(QStringList() << "*.json", QDir::Files)) {
            if (fi.fileName().compare("template.json", Qt::CaseInsensitive) == 0) {
                continue;
            }
            loadThemeFromJson(fi.absoluteFilePath());
        }
    }

    // 3. Scan user and local search directories
    for (const QString &path : searchPaths()) {
        scanThemesDir(path);
    }
}

void ThemeManager::resetToDefaults()
{
    QString curId = currentThemeId();
    registerThemes();
    setTheme(curId);
    emit themesUpdated();
}

const QList<ColorScheme>& ThemeManager::availableThemes() const
{
    return m_themes;
}

QStringList ThemeManager::themeIds() const noexcept
{
    QStringList ids;
    for (const auto &t : m_themes) {
        ids.append(t.id);
    }
    return ids;
}

QStringList ThemeManager::themeNames() const noexcept
{
    QStringList names;
    for (const auto &t : m_themes) {
        names.append(t.name);
    }
    return names;
}

const ColorScheme& ThemeManager::currentTheme() const
{
    if (m_currentIndex >= 0 && m_currentIndex < m_themes.size()) {
        return m_themes[m_currentIndex];
    }
    return m_themes.first();
}

QString ThemeManager::currentThemeId() const
{
    return currentTheme().id;
}

QString ThemeManager::currentThemeName() const
{
    return currentTheme().name;
}

bool ThemeManager::setTheme(const QString &idOrName)
{
    QString key = idOrName.trimmed();
    if (key.isEmpty() || key.compare("solarized", Qt::CaseInsensitive) == 0 ||
        key.compare("solarized8", Qt::CaseInsensitive) == 0 ||
        key.compare("solarized8_dark", Qt::CaseInsensitive) == 0 ||
        key.compare("solarized_dark", Qt::CaseInsensitive) == 0 ||
        key.compare("default", Qt::CaseInsensitive) == 0 ||
        key.compare("dark", Qt::CaseInsensitive) == 0) {
        key = "solarized8";
    } else if (key.compare("solarized8_light", Qt::CaseInsensitive) == 0 ||
               key.compare("solarized_light", Qt::CaseInsensitive) == 0 ||
               key.compare("light", Qt::CaseInsensitive) == 0) {
        key = "solarized8_light";
    }

    // If key points to an existing JSON file, load it
    if (key.endsWith(".json", Qt::CaseInsensitive) && QFile::exists(key)) {
        QFile f(key);
        if (f.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
            if (doc.isObject()) {
                ColorScheme scheme;
                if (scheme.fromJson(doc.object())) {
                    registerTheme(scheme);
                    key = scheme.id;
                }
            }
        }
    }

    for (int i = 0; i < m_themes.size(); ++i) {
        if (m_themes[i].id.compare(key, Qt::CaseInsensitive) == 0 ||
            m_themes[i].name.compare(key, Qt::CaseInsensitive) == 0) {
            m_currentIndex = i;
            if (qApp) {
                qApp->setPalette(m_themes[i].generatePalette());
                qApp->setStyleSheet(m_themes[i].generateStyleSheet());
            }
            emit themeChanged(m_themes[i]);
            return true;
        }
    }
    return false;
}

ColorBlindMode ThemeManager::colorBlindMode() const
{
    return m_colorBlindMode;
}

QString ThemeManager::colorBlindModeId() const
{
    return colorBlindModeToString(m_colorBlindMode);
}

void ThemeManager::setColorBlindMode(ColorBlindMode mode)
{
    if (m_colorBlindMode != mode) {
        m_colorBlindMode = mode;
        emit colorBlindModeChanged(m_colorBlindMode);
    }
}

void ThemeManager::setColorBlindMode(const QString &modeId)
{
    setColorBlindMode(stringToColorBlindMode(modeId));
}

void ThemeManager::setColorBlindMode(bool enabled)
{
    setColorBlindMode(enabled ? ColorBlindMode::Universal : ColorBlindMode::None);
}

AccessColors ThemeManager::getAccessColors(const QString &access, ColorBlindMode mode) const
{
    return currentTheme().getAccessColors(access, mode);
}

AccessColors ThemeManager::getAccessColors(const QString &access, bool colorBlind) const
{
    return currentTheme().getAccessColors(access, colorBlind ? ColorBlindMode::Universal : ColorBlindMode::None);
}

bool ThemeManager::registerTheme(const ColorScheme &scheme)
{
    if (scheme.id.isEmpty()) {
        return false;
    }

    for (int i = 0; i < m_themes.size(); ++i) {
        if (m_themes[i].id.compare(scheme.id, Qt::CaseInsensitive) == 0) {
            m_themes[i] = scheme;
            if (m_currentIndex == i && qApp) {
                qApp->setPalette(m_themes[i].generatePalette());
                qApp->setStyleSheet(m_themes[i].generateStyleSheet());
            }
            emit themesUpdated();
            return true;
        }
    }

    m_themes.append(scheme);
    emit themesUpdated();
    return true;
}

bool ThemeManager::loadThemeFromJson(const QString &jsonPathOrContent)
{
    QByteArray data;
    QFileInfo fi(jsonPathOrContent);
    if (fi.exists() && fi.isFile()) {
        QFile f(jsonPathOrContent);
        if (!f.open(QIODevice::ReadOnly)) {
            return false;
        }
        data = f.readAll();
    } else {
        data = jsonPathOrContent.toUtf8();
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return false;
    }

    ColorScheme scheme;
    if (!scheme.fromJson(doc.object())) {
        return false;
    }

    return registerTheme(scheme);
}

int ThemeManager::scanThemesDir(const QString &dirPath)
{
    QDir dir(dirPath);
    if (!dir.exists()) {
        return 0;
    }

    QStringList filters;
    filters << "*.json";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files | QDir::Readable, QDir::Name);

    int loadedCount = 0;
    for (const QFileInfo &fi : files) {
        if (fi.fileName().compare("template.json", Qt::CaseInsensitive) == 0) {
            continue;
        }
        if (loadThemeFromJson(fi.absoluteFilePath())) {
            loadedCount++;
        }
    }
    return loadedCount;
}

void ThemeManager::scanThemes()
{
    bool updated = false;
    for (const QString &path : searchPaths()) {
        if (scanThemesDir(path) > 0) {
            updated = true;
        }
    }
    if (updated) {
        emit themesUpdated();
    }
}

QString ThemeManager::userThemesDir()
{
    QString configDir = QFileInfo(AppSettings::instance().configFilePath()).path();
    return QDir(configDir).filePath("themes");
}

QString ThemeManager::localThemesDir()
{
    return QDir::current().filePath("themes");
}

static bool s_systemThemePathsOverride = false;

void ThemeManager::setSystemThemePathsOverride(bool override)
{
    s_systemThemePathsOverride = override;
}

bool ThemeManager::systemThemePathsOverride()
{
    return s_systemThemePathsOverride;
}

QStringList ThemeManager::searchPaths() const
{
    QStringList paths;

    // 1. RMAP_THEMES_PATH or RMAP_THEME_DIR environment variable
    const char *envThemes = std::getenv("RMAP_THEMES_PATH");
    if (!envThemes || envThemes[0] == '\0') {
        envThemes = std::getenv("RMAP_THEME_DIR");
    }
    if (envThemes && envThemes[0] != '\0') {
        for (const QString &p : QString::fromUtf8(envThemes).split(QDir::listSeparator(), Qt::SkipEmptyParts)) {
            if (!paths.contains(p)) paths.append(p);
        }
    }

    // 2. Custom search paths registered via addSearchPath
    for (const QString &p : m_customSearchPaths) {
        if (!paths.contains(p)) paths.append(p);
    }

    // 3. Installed system paths
    QString appDir = QCoreApplication::applicationDirPath();
    QString installPath = QDir(appDir + "/../share/rmap/themes").canonicalPath();
    if ((!installPath.isEmpty() || s_systemThemePathsOverride) && !paths.contains(installPath)) {
        paths.append(installPath.isEmpty() ? appDir + "/../share/rmap/themes" : installPath);
    }
    if ((QDir("/usr/local/share/rmap/themes").exists() || s_systemThemePathsOverride) && !paths.contains("/usr/local/share/rmap/themes")) {
        paths.append("/usr/local/share/rmap/themes");
    }
    if ((QDir("/usr/share/rmap/themes").exists() || s_systemThemePathsOverride) && !paths.contains("/usr/share/rmap/themes")) {
        paths.append("/usr/share/rmap/themes");
    }

    // 4. Current working directory themes
    QString localPath = localThemesDir();
    if (QDir(localPath).exists() && !paths.contains(localPath)) {
        paths.append(localPath);
    }

    // 5. User config directory themes (~/.config/rmap/themes)
    QString userPath = userThemesDir();
    if (!paths.contains(userPath)) {
        paths.append(userPath);
    }

    return paths;
}

void ThemeManager::addSearchPath(const QString &path)
{
    if (!path.isEmpty() && !m_customSearchPaths.contains(path)) {
        m_customSearchPaths.append(path);
    }
}

