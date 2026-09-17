/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "ThemeManager.hpp"
#include "AppSettings.hpp"
#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <cstdlib>

QString colorBlindModeToString(ColorBlindMode mode) {
  switch (mode) {
  case ColorBlindMode::Universal:
    return QStringLiteral("universal");
  case ColorBlindMode::Protanopia:
    return QStringLiteral("protanopia");
  case ColorBlindMode::Deuteranopia:
    return QStringLiteral("deuteranopia");
  case ColorBlindMode::Tritanopia:
    return QStringLiteral("tritanopia");
  case ColorBlindMode::Achromatopsia:
    return QStringLiteral("achromatopsia");
  case ColorBlindMode::None:
  default:
    return QStringLiteral("none");
  }
}

ColorBlindMode stringToColorBlindMode(const QString &str) {
  QString s = str.trimmed().toLower();
  if (s == "universal" || s == "okabe_ito" || s == "barrier_free" ||
      s == "cvd" || s == "true" || s == "1") {
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

namespace {
const QList<ColorBlindModeInfo> s_colorBlindModes = {
    {ColorBlindMode::Universal, QStringLiteral("universal"),
     QStringLiteral("Universal (Barrier-Free Okabe-Ito)"),
     QStringLiteral(
         "Universally distinguishable across all cone deficiencies")},
    {ColorBlindMode::Deuteranopia, QStringLiteral("deuteranopia"),
     QStringLiteral("Deuteranopia (Green-Blind / Weak)"),
     QStringLiteral(
         "Optimized for medium-wavelength cone deficiency (~6% of males)")},
    {ColorBlindMode::Protanopia, QStringLiteral("protanopia"),
     QStringLiteral("Protanopia (Red-Blind / Weak)"),
     QStringLiteral(
         "Optimized for long-wavelength cone deficiency (~2% of males)")},
    {ColorBlindMode::Tritanopia, QStringLiteral("tritanopia"),
     QStringLiteral("Tritanopia (Blue-Blind / Weak)"),
     QStringLiteral("Optimized for short-wavelength cone deficiency")},
    {ColorBlindMode::Achromatopsia, QStringLiteral("achromatopsia"),
     QStringLiteral("Achromatopsia (Monochrome / Grayscale)"),
     QStringLiteral(
         "High-contrast luminance steps for complete color blindness")}};

enum class AccessKind { RW, RO, WO, W1S, W1C, RC, NA };

AccessKind parseAccessKind(QStringView a) {
  if (a == u"RW")
    return AccessKind::RW;
  if (a == u"RO")
    return AccessKind::RO;
  if (a == u"WO")
    return AccessKind::WO;
  if (a == u"WO1")
    return AccessKind::WO;
  if (a == u"WRC")
    return AccessKind::RW;
  if (a == u"WRS")
    return AccessKind::RW;
  if (a == u"W1S")
    return AccessKind::W1S;
  if (a == u"W0S")
    return AccessKind::W1S;
  if (a == u"WS")
    return AccessKind::W1S;
  if (a == u"W1C")
    return AccessKind::W1C;
  if (a == u"W0C")
    return AccessKind::W1C;
  if (a == u"WC")
    return AccessKind::W1C;
  if (a == u"W1T")
    return AccessKind::W1C;
  if (a == u"W0T")
    return AccessKind::W1C;
  if (a == u"W1")
    return AccessKind::W1C;
  if (a == u"W0")
    return AccessKind::W1C;
  if (a == u"RC")
    return AccessKind::RC;
  if (a == u"RS")
    return AccessKind::RC;
  if (a == u"WOC")
    return AccessKind::WO;
  if (a == u"WOS")
    return AccessKind::W1S;
  if (a == u"W1SRC")
    return AccessKind::W1S;
  if (a == u"W0SRC")
    return AccessKind::W1S;
  if (a == u"W1CRS")
    return AccessKind::W1C;
  if (a == u"W0CRS")
    return AccessKind::W1C;
  if (a == u"NOACCESS")
    return AccessKind::NA;
  if (a == u"NA")
    return AccessKind::NA;
  return AccessKind::NA;
}
} // namespace

const QList<ColorBlindModeInfo> &availableColorBlindModes() {
  return s_colorBlindModes;
}

AccessColors ColorScheme::getAccessColors(const QString &access,
                                          ColorBlindMode mode) const {
  QString a = access.toUpper().trimmed();
  AccessKind kind = parseAccessKind(a);

  if (mode == ColorBlindMode::None) {
    switch (kind) {
    case AccessKind::RW:
      return rwColors;
    case AccessKind::RO:
      return roColors;
    case AccessKind::WO:
      return woColors;
    case AccessKind::W1C:
    case AccessKind::W1S:
      return w1cColors;
    case AccessKind::RC:
      return rcColors;
    case AccessKind::NA:
    default:
      return naColors;
    }
  }

  AccessColors c;

  // Check custom color-blind overrides first if available (for universal mode)
  if (hasCustomColorBlind && mode == ColorBlindMode::Universal) {
    switch (kind) {
    case AccessKind::RO:
    case AccessKind::RC:
      return cbRo;
    case AccessKind::WO:
      return cbWo;
    case AccessKind::W1C:
    case AccessKind::W1S:
      return cbW1c;
    case AccessKind::RW:
      return cbRw;
    case AccessKind::NA:
    default:
      return cbNa;
    }
  }

  switch (mode) {
  case ColorBlindMode::Universal: {
    // Universal Okabe-Ito / Wong barrier-free palette
    switch (kind) {
    case AccessKind::RW:
      c.bg = QColor(128, 222, 234);
      c.border = QColor(0, 96, 100);
      c.text = QColor(0, 50, 60);
      break;
    case AccessKind::RO:
      c.bg = QColor(159, 168, 218);
      c.border = QColor(26, 35, 126);
      c.text = QColor(15, 20, 80);
      break;
    case AccessKind::WO:
      c.bg = QColor(206, 147, 216);
      c.border = QColor(74, 20, 140);
      c.text = QColor(50, 10, 80);
      break;
    case AccessKind::W1S:
      c.bg = QColor(255, 224, 130);
      c.border = QColor(183, 129, 3);
      c.text = QColor(74, 48, 0);
      break;
    case AccessKind::W1C:
      c.bg = QColor(255, 138, 101);
      c.border = QColor(191, 54, 12);
      c.text = QColor(100, 20, 0);
      break;
    case AccessKind::RC:
      c.bg = QColor(179, 157, 219);
      c.border = QColor(49, 27, 146);
      c.text = QColor(26, 0, 75);
      break;
    case AccessKind::NA:
    default:
      c.bg = QColor(207, 216, 220);
      c.border = QColor(55, 71, 79);
      c.text = QColor(40, 40, 40);
      break;
    }
    break;
  }

  case ColorBlindMode::Protanopia: {
    // Protanopia: Red-blind / Red-weak (L-cone deficiency)
    switch (kind) {
    case AccessKind::RW:
      c.bg = QColor(147, 197, 253);
      c.border = QColor(29, 78, 216);
      c.text = QColor(23, 37, 84);
      break;
    case AccessKind::RO:
      c.bg = QColor(165, 243, 252);
      c.border = QColor(8, 145, 178);
      c.text = QColor(22, 78, 99);
      break;
    case AccessKind::WO:
      c.bg = QColor(254, 240, 138);
      c.border = QColor(161, 98, 7);
      c.text = QColor(69, 26, 3);
      break;
    case AccessKind::W1S:
      c.bg = QColor(254, 249, 195);
      c.border = QColor(133, 77, 14);
      c.text = QColor(66, 32, 6);
      break;
    case AccessKind::W1C:
      c.bg = QColor(254, 215, 170);
      c.border = QColor(194, 65, 12);
      c.text = QColor(67, 20, 7);
      break;
    case AccessKind::RC:
      c.bg = QColor(199, 210, 254);
      c.border = QColor(67, 56, 202);
      c.text = QColor(30, 27, 75);
      break;
    case AccessKind::NA:
    default:
      c.bg = QColor(226, 232, 240);
      c.border = QColor(71, 85, 105);
      c.text = QColor(15, 23, 42);
      break;
    }
    break;
  }

  case ColorBlindMode::Deuteranopia: {
    // Deuteranopia: Green-blind / Green-weak (M-cone deficiency)
    switch (kind) {
    case AccessKind::RW:
      c.bg = QColor(153, 246, 228);
      c.border = QColor(15, 118, 110);
      c.text = QColor(19, 78, 74);
      break;
    case AccessKind::RO:
      c.bg = QColor(191, 219, 254);
      c.border = QColor(30, 64, 175);
      c.text = QColor(23, 37, 84);
      break;
    case AccessKind::WO:
      c.bg = QColor(253, 224, 71);
      c.border = QColor(161, 98, 7);
      c.text = QColor(66, 32, 6);
      break;
    case AccessKind::W1S:
      c.bg = QColor(254, 240, 138);
      c.border = QColor(180, 83, 9);
      c.text = QColor(69, 26, 3);
      break;
    case AccessKind::W1C:
      c.bg = QColor(251, 146, 60);
      c.border = QColor(154, 52, 18);
      c.text = QColor(67, 20, 7);
      break;
    case AccessKind::RC:
      c.bg = QColor(221, 214, 254);
      c.border = QColor(109, 40, 217);
      c.text = QColor(46, 16, 101);
      break;
    case AccessKind::NA:
    default:
      c.bg = QColor(226, 232, 240);
      c.border = QColor(71, 85, 105);
      c.text = QColor(15, 23, 42);
      break;
    }
    break;
  }

  case ColorBlindMode::Tritanopia: {
    // Tritanopia: Blue-blind / Blue-weak (S-cone deficiency)
    switch (kind) {
    case AccessKind::RW:
      c.bg = QColor(204, 251, 241);
      c.border = QColor(15, 118, 110);
      c.text = QColor(19, 78, 74);
      break;
    case AccessKind::RO:
      c.bg = QColor(254, 205, 211);
      c.border = QColor(190, 18, 60);
      c.text = QColor(76, 5, 25);
      break;
    case AccessKind::WO:
      c.bg = QColor(254, 215, 170);
      c.border = QColor(194, 65, 12);
      c.text = QColor(67, 20, 7);
      break;
    case AccessKind::W1S:
      c.bg = QColor(255, 228, 230);
      c.border = QColor(159, 18, 57);
      c.text = QColor(76, 5, 25);
      break;
    case AccessKind::W1C:
      c.bg = QColor(251, 207, 232);
      c.border = QColor(157, 23, 77);
      c.text = QColor(80, 7, 36);
      break;
    case AccessKind::RC:
      c.bg = QColor(245, 208, 254);
      c.border = QColor(134, 25, 143);
      c.text = QColor(74, 4, 78);
      break;
    case AccessKind::NA:
    default:
      c.bg = QColor(226, 232, 240);
      c.border = QColor(51, 65, 85);
      c.text = QColor(15, 23, 42);
      break;
    }
    break;
  }

  case ColorBlindMode::Achromatopsia: {
    // Achromatopsia: Complete color blindness (Distinct luminance steps)
    switch (kind) {
    case AccessKind::RW:
      c.bg = QColor(224, 224, 224);
      c.border = QColor(0, 0, 0);
      c.text = QColor(0, 0, 0);
      break;
    case AccessKind::RO:
      c.bg = QColor(255, 255, 255);
      c.border = QColor(0, 0, 0);
      c.text = QColor(0, 0, 0);
      break;
    case AccessKind::WO:
      c.bg = QColor(34, 34, 34);
      c.border = QColor(255, 255, 255);
      c.text = QColor(255, 255, 255);
      break;
    case AccessKind::W1S:
      c.bg = QColor(136, 136, 136);
      c.border = QColor(0, 0, 0);
      c.text = QColor(255, 255, 255);
      break;
    case AccessKind::W1C:
      c.bg = QColor(85, 85, 85);
      c.border = QColor(255, 255, 255);
      c.text = QColor(255, 255, 255);
      break;
    case AccessKind::RC:
      c.bg = QColor(170, 170, 170);
      c.border = QColor(0, 0, 0);
      c.text = QColor(0, 0, 0);
      break;
    case AccessKind::NA:
    default:
      c.bg = QColor(51, 51, 51);
      c.border = QColor(119, 119, 119);
      c.text = QColor(204, 204, 204);
      break;
    }
    break;
  }

  default:
    break;
  }

  return c;
}

AccessColors ColorScheme::getAccessColors(const QString &access,
                                          bool colorBlind) const {
  return getAccessColors(access, colorBlind ? ColorBlindMode::Universal
                                            : ColorBlindMode::None);
}

QPalette ColorScheme::generatePalette() const noexcept {
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

QString ColorScheme::generateStyleSheet() const {
  QString qss;
  qss += QString("QMainWindow, QDialog { background-color: %1; color: %2; }\n")
             .arg(windowBg.name(), textColor.name());

  qss += QString("QTreeView, QTableView { background-color: %1; "
                 "alternate-background-color: %2; color: %3; gridline-color: "
                 "%4; border: 1px solid %4; selection-background-color: %5; "
                 "selection-color: %6; }\n")
             .arg(panelBg.name(), altRowBg.name(), textColor.name(),
                  border.name(), selectionBg.name(), selectionText.name());

  qss +=
      QString("QHeaderView::section { background-color: %1; color: %2; border: "
              "1px solid %3; padding: 4px 6px; font-weight: bold; }\n")
          .arg(headerBg.name(), headerText.name(), border.name());

  qss += QString("QToolBar { background-color: %1; border-bottom: 1px solid "
                 "%2; spacing: 4px; padding: 3px; }\n")
             .arg(headerBg.name(), border.name());

  qss +=
      QString("QToolButton { background-color: transparent; border: 1px solid "
              "transparent; border-radius: 4px; padding: 3px; color: %1; }\n")
          .arg(textColor.name());
  qss +=
      QString(
          "QToolButton:hover { background-color: %1; border: 1px solid %2; }\n")
          .arg(buttonHover.name(), border.name());
  qss += QString("QToolButton:pressed { background-color: %1; }\n")
             .arg(selectionBg.name());

  qss += QString("QMenuBar { background-color: %1; color: %2; border-bottom: "
                 "1px solid %3; }\n")
             .arg(windowBg.name(), textColor.name(), border.name());
  qss += QString("QMenuBar::item:selected { background-color: %1; color: %2; "
                 "border-radius: 3px; }\n")
             .arg(buttonHover.name(), selectionText.name());

  qss += QString("QMenu { background-color: %1; color: %2; border: 1px solid "
                 "%3; padding: 4px; }\n")
             .arg(headerBg.name(), textColor.name(), border.name());
  qss += QString("QMenu::item:selected { background-color: %1; color: %2; "
                 "border-radius: 3px; }\n")
             .arg(selectionBg.name(), selectionText.name());
  qss += QString("QMenu::separator { height: 1px; background-color: %1; "
                 "margin: 4px 8px; }\n")
             .arg(border.name());

  QString inputTextColor = isDark ? QString("#f1f5f9") : textColor.name();
  qss +=
      QString(
          "QLineEdit, QSpinBox, QComboBox { background-color: %1; color: %2; "
          "border: 1px solid %3; border-radius: 3px; padding: 3px 6px; }\n")
          .arg(inputBg.name(), inputTextColor, inputBorder.name());
  qss += QString("QLineEdit:focus, QSpinBox:focus, QComboBox:focus { border: "
                 "1px solid %1; }\n")
             .arg(selectionBg.name());

  qss += QString("QPushButton { background-color: %1; color: %2; border: 1px "
                 "solid %3; border-radius: 4px; padding: 4px 12px; }\n")
             .arg(buttonBg.name(), textColor.name(), border.name());
  qss += QString("QPushButton:hover { background-color: %1; }\n")
             .arg(buttonHover.name());
  qss += QString("QPushButton:pressed { background-color: %1; color: %2; }\n")
             .arg(selectionBg.name(), selectionText.name());

  qss += QString("QGroupBox { border: 1px solid %1; border-radius: 5px; "
                 "margin-top: 10px; font-weight: bold; color: %2; }\n")
             .arg(border.name(), textColor.name());
  qss += QString("QGroupBox::title { subcontrol-origin: margin; "
                 "subcontrol-position: top left; padding: 0 5px; }\n");

  qss += QString("QStatusBar { background-color: %1; color: %2; border-top: "
                 "1px solid %3; }\n")
             .arg(headerBg.name(), textMuted.name(), border.name());

  qss += QString("QSplitter::handle { background-color: %1; }\n")
             .arg(border.name());

  QString vsb = QStringLiteral(
      "QScrollBar:vertical { background: %1; width: 12px; margin: 0px; }\n"
      "QScrollBar::handle:vertical { background: %2; min-height: 20px; "
      "border-radius: 4px; margin: 2px; }\n"
      "QScrollBar::handle:vertical:hover { background: %3; }\n"
      "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: "
      "0px; }\n");
  qss += vsb.arg(windowBg.name(), border.name(), buttonHover.name());

  QString hsb = QStringLiteral(
      "QScrollBar:horizontal { background: %1; height: 12px; margin: 0px; }\n"
      "QScrollBar::handle:horizontal { background: %2; min-width: 20px; "
      "border-radius: 4px; margin: 2px; }\n"
      "QScrollBar::handle:horizontal:hover { background: %3; }\n"
      "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { "
      "width: 0px; }\n");
  qss += hsb.arg(windowBg.name(), border.name(), buttonHover.name());

  return qss;
}

static QColor parseColor(const QJsonValue &val, const QColor &fallback) {
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

void ColorScheme::initDefaults() {
  isDark = true;
  isDefault = false;
  windowBg = QColor("#1e1e1e");
  panelBg = QColor("#252526");
  altRowBg = QColor("#2a2d2e");
  textColor = QColor("#d4d4d4");
  textMuted = QColor("#808080");
  headerBg = QColor("#333333");
  headerText = QColor("#cccccc");
  border = QColor("#3c3c3c");
  selectionBg = QColor("#264f78");
  selectionText = QColor("#ffffff");
  buttonBg = QColor("#0e639c");
  buttonHover = QColor("#1177bb");
  inputBg = QColor("#3c3c3c");
  inputBorder = QColor("#555555");
  errorBg = QColor("#5a1d1d");
  errorBorder = QColor("#be1100");
  errorText = QColor("#ffffff");

  rsvdBg = QColor("#333333");
  rsvdBorder = QColor("#555555");
  rsvdStripe = QColor("#444444");
  rsvdText = QColor("#888888");
  rulerText = QColor("#888888");

  rwColors = {QColor("#1e3a1e"), QColor("#3a7a3a"), QColor("#88ff88")};
  roColors = {QColor("#1e2a3a"), QColor("#3a5a7a"), QColor("#88ccff")};
  woColors = {QColor("#3a2a1e"), QColor("#7a5a3a"), QColor("#ffcc88")};
  w1cColors = {QColor("#3a1e2a"), QColor("#7a3a5a"), QColor("#ff88cc")};
  rcColors = {QColor("#2a1e3a"), QColor("#5a3a7a"), QColor("#cc88ff")};
  naColors = {QColor("#2a2a2a"), QColor("#4a4a4a"), QColor("#888888")};
  hasCustomColorBlind = false;
}

bool ColorScheme::fromJson(const QJsonObject &obj) {
  QString schemeId = obj.value("id").toString().trimmed();
  if (schemeId.isEmpty()) {
    return false;
  }

  initDefaults();
  this->id = schemeId;

  QJsonValue nameVal = obj.value("name");
  if (nameVal.isString()) {
    this->name = nameVal.toString().trimmed();
  } else {
    this->name = schemeId;
  }
  if (obj.contains("isDark")) {
    this->isDark = obj["isDark"].toBool(this->isDark);
  }
  if (obj.contains("isDefault")) {
    this->isDefault = obj["isDefault"].toBool(this->isDefault);
  }
  if (obj.contains("aliases") && obj["aliases"].isArray()) {
    const QJsonArray arr = obj["aliases"].toArray();
    for (const auto &v : arr) {
      aliases.append(v.toString().trimmed());
    }
  }

  QJsonObject baseObj = obj.value("base").toObject();
  auto getBaseColor = [&](const QString &key, const QColor &def) -> QColor {
    QJsonValue v = baseObj.value(key);
    if (!v.isUndefined())
      return parseColor(v, def);
    v = obj.value(key);
    if (!v.isUndefined())
      return parseColor(v, def);
    return def;
  };

  windowBg = getBaseColor("windowBg", windowBg);
  panelBg = getBaseColor("panelBg", panelBg);
  altRowBg = getBaseColor("altRowBg", altRowBg);
  textColor = getBaseColor("textColor", textColor);
  textMuted = getBaseColor("textMuted", textMuted);
  headerBg = getBaseColor("headerBg", headerBg);
  headerText = getBaseColor("headerText", headerText);
  border = getBaseColor("border", border);
  selectionBg = getBaseColor("selectionBg", selectionBg);
  selectionText = getBaseColor("selectionText", selectionText);
  buttonBg = getBaseColor("buttonBg", buttonBg);
  buttonHover = getBaseColor("buttonHover", buttonHover);
  inputBg = getBaseColor("inputBg", inputBg);
  inputBorder = getBaseColor("inputBorder", inputBorder);
  errorBg = getBaseColor("errorBg", errorBg);
  errorBorder = getBaseColor("errorBorder", errorBorder);
  errorText = getBaseColor("errorText", errorText);

  QJsonObject rsvdObj = obj.value("reserved").toObject();
  auto getRsvdColor = [&](const QString &key, const QString &flatKey,
                          const QColor &def) -> QColor {
    QJsonValue v = rsvdObj.value(key);
    if (!v.isUndefined())
      return parseColor(v, def);
    v = obj.value(flatKey);
    if (!v.isUndefined())
      return parseColor(v, def);
    return def;
  };

  rsvdBg = getRsvdColor("bg", "rsvdBg", rsvdBg);
  rsvdBorder = getRsvdColor("border", "rsvdBorder", rsvdBorder);
  rsvdStripe = getRsvdColor("stripe", "rsvdStripe", rsvdStripe);
  rsvdText = getRsvdColor("text", "rsvdText", rsvdText);
  rulerText = getRsvdColor("rulerText", "rulerText", rulerText);

  QJsonObject apObj = obj.value("accessPolicies").toObject();
  auto getAccess = [&](const QString &key,
                       const AccessColors &def) -> AccessColors {
    QJsonObject o = apObj.value(key).toObject();
    if (o.isEmpty()) {
      QJsonValue v = obj.value(key);
      if (v.isObject())
        o = v.toObject();
    }
    if (o.isEmpty())
      return def;
    AccessColors res = def;
    QJsonValue v = o.value("bg");
    if (!v.isUndefined())
      res.bg = parseColor(v, def.bg);
    v = o.value("border");
    if (!v.isUndefined())
      res.border = parseColor(v, def.border);
    v = o.value("text");
    if (!v.isUndefined())
      res.text = parseColor(v, def.text);
    return res;
  };

  rwColors = getAccess("rw", rwColors);
  roColors = getAccess("ro", roColors);
  woColors = getAccess("wo", woColors);
  w1cColors = getAccess("w1c", w1cColors);
  rcColors = getAccess("rc", rcColors);
  naColors = getAccess("na", naColors);

  QJsonValue cbVal = obj.value("colorBlind");
  if (cbVal.isObject()) {
    QJsonObject cbObj = cbVal.toObject();
    auto getCb = [&](const QString &key,
                     const AccessColors &def) -> AccessColors {
      QJsonObject o = cbObj.value(key).toObject();
      if (o.isEmpty())
        return def;
      AccessColors res = def;
      QJsonValue v = o.value("bg");
      if (!v.isUndefined())
        res.bg = parseColor(v, def.bg);
      v = o.value("border");
      if (!v.isUndefined())
        res.border = parseColor(v, def.border);
      v = o.value("text");
      if (!v.isUndefined())
        res.text = parseColor(v, def.text);
      return res;
    };
    hasCustomColorBlind = true;
    AccessColors defRo = {QColor(159, 168, 218), QColor(26, 35, 126),
                          QColor(15, 20, 80)};
    AccessColors defWo = {QColor(206, 147, 216), QColor(74, 20, 140),
                          QColor(50, 10, 80)};
    AccessColors defW1c = {QColor(255, 138, 101), QColor(191, 54, 12),
                           QColor(100, 20, 0)};
    AccessColors defRw = {QColor(128, 222, 234), QColor(0, 96, 100),
                          QColor(0, 50, 60)};
    AccessColors defNa = {QColor(207, 216, 220), QColor(55, 71, 79),
                          QColor(40, 40, 40)};

    cbRo = getCb("ro", defRo);
    cbWo = getCb("wo", defWo);
    cbW1c = getCb("w1c", defW1c);
    cbRw = getCb("rw", defRw);
    cbRc = getCb("rc", cbRo);
    cbNa = getCb("na", defNa);
  }

  return true;
}

QJsonObject ColorScheme::toJson() const {
  QJsonObject root;
  root["id"] = id;
  root["name"] = name;
  root["isDark"] = isDark;
  if (isDefault) {
    root["isDefault"] = isDefault;
  }
  if (!aliases.isEmpty()) {
    QJsonArray arr;
    for (const auto &a : aliases) {
      arr.append(a);
    }
    root["aliases"] = arr;
  }

  QJsonObject base;
  base["windowBg"] = windowBg.name();
  base["panelBg"] = panelBg.name();
  base["altRowBg"] = altRowBg.name();
  base["textColor"] = textColor.name();
  base["textMuted"] = textMuted.name();
  base["headerBg"] = headerBg.name();
  base["headerText"] = headerText.name();
  base["border"] = border.name();
  base["selectionBg"] = selectionBg.name();
  base["selectionText"] = selectionText.name();
  base["buttonBg"] = buttonBg.name();
  base["buttonHover"] = buttonHover.name();
  base["inputBg"] = inputBg.name();
  base["inputBorder"] = inputBorder.name();
  base["errorBg"] = errorBg.name();
  base["errorBorder"] = errorBorder.name();
  base["errorText"] = errorText.name();
  root["base"] = base;

  QJsonObject rsvd;
  rsvd["bg"] = rsvdBg.name();
  rsvd["border"] = rsvdBorder.name();
  rsvd["stripe"] = rsvdStripe.name();
  rsvd["text"] = rsvdText.name();
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
  ap["rw"] = accessToJson(rwColors);
  ap["ro"] = accessToJson(roColors);
  ap["wo"] = accessToJson(woColors);
  ap["w1c"] = accessToJson(w1cColors);
  ap["rc"] = accessToJson(rcColors);
  ap["na"] = accessToJson(naColors);
  root["accessPolicies"] = ap;

  if (hasCustomColorBlind) {
    QJsonObject cb;
    cb["ro"] = accessToJson(cbRo);
    cb["wo"] = accessToJson(cbWo);
    cb["w1c"] = accessToJson(cbW1c);
    cb["rw"] = accessToJson(cbRw);
    cb["rc"] = accessToJson(cbRc);
    cb["na"] = accessToJson(cbNa);
    root["colorBlind"] = cb;
  }

  return root;
}

QList<ColorScheme> ColorScheme::builtInDefaults() noexcept {
  QList<ColorScheme> list;
  auto loadThemeFile = [&list](const QString &filePath) {
    QFile f(filePath);
    if (f.open(QIODevice::ReadOnly)) {
      QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
      if (doc.isObject()) {
        ColorScheme scheme;
        if (scheme.fromJson(doc.object())) {
          for (const auto &existing : list) {
            if (existing.id.compare(scheme.id, Qt::CaseInsensitive) == 0) {
              return;
            }
          }
          list.append(scheme);
        }
      }
    }
  };

  auto scanDir = [&loadThemeFile](const QString &dirPath) {
    QDir dir(dirPath);
    if (!dir.exists())
      return;
    QFileInfoList files = dir.entryInfoList(
        QStringList() << "*.json", QDir::Files | QDir::Readable, QDir::Name);
    for (const QFileInfo &fi : files) {
      if (fi.fileName().compare("template.json", Qt::CaseInsensitive) == 0)
        continue;
      loadThemeFile(fi.absoluteFilePath());
    }
  };

  scanDir(":/themes");
  if (list.isEmpty()) {
    scanDir("./themes");
  }

  // Ensure default theme (isDefault == true) is at index 0 if present
  for (int i = 0; i < list.size(); ++i) {
    if (list[i].isDefault && i > 0) {
      list.swapItemsAt(0, i);
      break;
    }
  }

  if (list.isEmpty()) {
    ColorScheme s;
    s.initDefaults();
    s.id = "default";
    s.name = "Default";
    s.isDefault = true;
    list.append(s);
  }

  return list;
}

ColorScheme ColorScheme::createDefault(const QString &id) {
  QString key = id.trimmed();
  QString norm = key.toLower();
  norm.replace('-', '_');

  const auto defaults = builtInDefaults();
  for (const auto &s : defaults) {
    if (s.id.compare(key, Qt::CaseInsensitive) == 0 ||
        s.name.compare(key, Qt::CaseInsensitive) == 0 ||
        s.id.compare(norm, Qt::CaseInsensitive) == 0) {
      return s;
    }
    for (const auto &alias : s.aliases) {
      QString aNorm = alias.toLower();
      aNorm.replace('-', '_');
      if (alias.compare(key, Qt::CaseInsensitive) == 0 ||
          alias.compare(norm, Qt::CaseInsensitive) == 0 ||
          aNorm.compare(norm, Qt::CaseInsensitive) == 0) {
        return s;
      }
    }
  }

  if (norm.isEmpty() || norm == "default") {
    for (const auto &s : defaults) {
      if (s.isDefault)
        return s;
    }
    if (!defaults.isEmpty())
      return defaults.first();
  }

  if (norm == "dark") {
    for (const auto &s : defaults) {
      if (s.isDark)
        return s;
    }
  }

  if (norm == "light") {
    for (const auto &s : defaults) {
      if (!s.isDark)
        return s;
    }
  }

  if (!defaults.isEmpty()) {
    ColorScheme fallback = defaults.first();
    fallback.id = id;
    fallback.name = id;
    return fallback;
  }

  ColorScheme fallback;
  fallback.initDefaults();
  fallback.id = id;
  fallback.name = id;
  return fallback;
}

ThemeManager &ThemeManager::instance() {
  static ThemeManager mgr;
  return mgr;
}

ThemeManager::ThemeManager() {
  registerThemes();
  m_currentIndex = 0;
}

void ThemeManager::registerThemes() {
  m_themes.clear();

  // 1. Built-in defaults dynamically loaded from theme JSON catalogs
  for (const auto &s : ColorScheme::builtInDefaults()) {
    m_themes.append(s);
  }

  // 2. Scan user and local search directories
  for (const QString &path : searchPaths()) {
    scanThemesDir(path);
  }
}

void ThemeManager::resetToDefaults() {
  QString curId = currentThemeId();
  registerThemes();
  setTheme(curId);
  emit themesUpdated();
}

const QList<ColorScheme> &ThemeManager::availableThemes() const {
  return m_themes;
}

QStringList ThemeManager::themeIds() const noexcept {
  QStringList ids;
  for (const auto &t : m_themes) {
    ids.append(t.id);
  }
  return ids;
}

QStringList ThemeManager::themeNames() const noexcept {
  QStringList names;
  for (const auto &t : m_themes) {
    names.append(t.name);
  }
  return names;
}

const ColorScheme &ThemeManager::currentTheme() const {
  if (m_currentIndex >= 0 && m_currentIndex < m_themes.size()) {
    return m_themes[m_currentIndex];
  }
  return m_themes.first();
}

QString ThemeManager::currentThemeId() const { return currentTheme().id; }

QString ThemeManager::currentThemeName() const { return currentTheme().name; }

bool ThemeManager::setTheme(const QString &idOrName) {
  QString key = idOrName.trimmed();
  QString norm = key.toLower();
  norm.replace('-', '_');

  if (norm.isEmpty() || norm == "default") {
    for (int i = 0; i < m_themes.size(); ++i) {
      if (m_themes[i].isDefault) {
        m_currentIndex = i;
        if (auto *app =
                qobject_cast<QApplication *>(QCoreApplication::instance())) {
          app->setPalette(m_themes[i].generatePalette());
          app->setStyleSheet(m_themes[i].generateStyleSheet());
        }
        emit themeChanged(m_themes[i]);
        return true;
      }
    }
    if (!m_themes.isEmpty()) {
      m_currentIndex = 0;
      if (auto *app =
              qobject_cast<QApplication *>(QCoreApplication::instance())) {
        app->setPalette(m_themes[0].generatePalette());
        app->setStyleSheet(m_themes[0].generateStyleSheet());
      }
      emit themeChanged(m_themes[0]);
      return true;
    }
    return false;
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
          norm = key.toLower();
          norm.replace('-', '_');
        }
      }
    }
  }

  for (int i = 0; i < m_themes.size(); ++i) {
    bool match = (m_themes[i].id.compare(key, Qt::CaseInsensitive) == 0 ||
                  m_themes[i].name.compare(key, Qt::CaseInsensitive) == 0 ||
                  m_themes[i].id.compare(norm, Qt::CaseInsensitive) == 0);
    if (!match) {
      for (const auto &alias : m_themes[i].aliases) {
        QString aNorm = alias.toLower();
        aNorm.replace('-', '_');
        if (alias.compare(key, Qt::CaseInsensitive) == 0 ||
            alias.compare(norm, Qt::CaseInsensitive) == 0 ||
            aNorm.compare(norm, Qt::CaseInsensitive) == 0) {
          match = true;
          break;
        }
      }
    }
    if (match) {
      m_currentIndex = i;
      if (auto *app =
              qobject_cast<QApplication *>(QCoreApplication::instance())) {
        app->setPalette(m_themes[i].generatePalette());
        app->setStyleSheet(m_themes[i].generateStyleSheet());
      }
      emit themeChanged(m_themes[i]);
      return true;
    }
  }
  return false;
}

ColorBlindMode ThemeManager::colorBlindMode() const { return m_colorBlindMode; }

QString ThemeManager::colorBlindModeId() const {
  return colorBlindModeToString(m_colorBlindMode);
}

void ThemeManager::setColorBlindMode(ColorBlindMode mode) {
  if (m_colorBlindMode != mode) {
    m_colorBlindMode = mode;
    emit colorBlindModeChanged(m_colorBlindMode);
  }
}

void ThemeManager::setColorBlindMode(const QString &modeId) {
  setColorBlindMode(stringToColorBlindMode(modeId));
}

void ThemeManager::setColorBlindMode(bool enabled) {
  setColorBlindMode(enabled ? ColorBlindMode::Universal : ColorBlindMode::None);
}

AccessColors ThemeManager::getAccessColors(const QString &access,
                                           ColorBlindMode mode) const {
  return currentTheme().getAccessColors(access, mode);
}

AccessColors ThemeManager::getAccessColors(const QString &access,
                                           bool colorBlind) const {
  return currentTheme().getAccessColors(
      access, colorBlind ? ColorBlindMode::Universal : ColorBlindMode::None);
}

bool ThemeManager::registerTheme(const ColorScheme &scheme) {
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

bool ThemeManager::loadThemeFromJson(const QString &jsonPathOrContent) {
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

int ThemeManager::scanThemesDir(const QString &dirPath) {
  QDir dir(dirPath);
  if (!dir.exists()) {
    return 0;
  }

  QStringList filters;
  filters << "*.json";
  QFileInfoList files =
      dir.entryInfoList(filters, QDir::Files | QDir::Readable, QDir::Name);

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

void ThemeManager::scanThemes() {
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

QString ThemeManager::userThemesDir() {
  QString configDir =
      QFileInfo(AppSettings::instance().configFilePath()).path();
  return QDir(configDir).filePath("themes");
}

QString ThemeManager::localThemesDir() {
  return QDir::current().filePath("themes");
}

static bool s_systemThemePathsOverride = false;

void ThemeManager::setSystemThemePathsOverride(bool override) {
  s_systemThemePathsOverride = override;
}

bool ThemeManager::systemThemePathsOverride() {
  return s_systemThemePathsOverride;
}

QStringList ThemeManager::searchPaths() const {
  QStringList paths;

  // 1. RMAP_THEMES_PATH or RMAP_THEME_DIR environment variable
  const char *envThemes = std::getenv("RMAP_THEMES_PATH");
  if (!envThemes || envThemes[0] == '\0') {
    envThemes = std::getenv("RMAP_THEME_DIR");
  }
  if (envThemes && envThemes[0] != '\0') {
    for (const QString &p : QString::fromUtf8(envThemes).split(
             QDir::listSeparator(), Qt::SkipEmptyParts)) {
      if (!paths.contains(p))
        paths.append(p);
    }
  }

  // 2. Custom search paths registered via addSearchPath
  for (const QString &p : m_customSearchPaths) {
    if (!paths.contains(p))
      paths.append(p);
  }

  // 3. Installed system paths
  QString appDir = QCoreApplication::applicationDirPath();
  QString installPath = QDir(appDir + "/../share/rmap/themes").canonicalPath();
  if ((!installPath.isEmpty() || s_systemThemePathsOverride) &&
      !paths.contains(installPath)) {
    paths.append(installPath.isEmpty() ? appDir + "/../share/rmap/themes"
                                       : installPath);
  }
  if ((QDir("/usr/local/share/rmap/themes").exists() ||
       s_systemThemePathsOverride) &&
      !paths.contains("/usr/local/share/rmap/themes")) {
    paths.append("/usr/local/share/rmap/themes");
  }
  if ((QDir("/usr/share/rmap/themes").exists() || s_systemThemePathsOverride) &&
      !paths.contains("/usr/share/rmap/themes")) {
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

void ThemeManager::addSearchPath(const QString &path) {
  if (!path.isEmpty() && !m_customSearchPaths.contains(path)) {
    m_customSearchPaths.append(path);
  }
}
