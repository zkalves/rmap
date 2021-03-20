#include "ThemeManager.hpp"
#include <QApplication>

AccessColors ColorScheme::getAccessColors(const QString &access, bool colorBlind) const
{
    QString a = access.toUpper().trimmed();
    AccessColors c;

    if (colorBlind) {
        // High-Contrast CVD Barrier-Free Palette (Okabe-Ito / Wong)
        if (a == "RO" || a == "RC" || a == "RS") {
            c.bg = QColor(159, 168, 218);
            c.border = QColor(26, 35, 126);
            c.text = QColor(15, 20, 80);
        } else if (a == "WO") {
            c.bg = QColor(206, 147, 216);
            c.border = QColor(74, 20, 140);
            c.text = QColor(50, 10, 80);
        } else if (a.startsWith("W1") || a.startsWith("W0") || a == "WC" || a == "WS") {
            c.bg = QColor(255, 138, 101);
            c.border = QColor(191, 54, 12);
            c.text = QColor(100, 20, 0);
        } else if (a == "RW") {
            c.bg = QColor(128, 222, 234);
            c.border = QColor(0, 96, 100);
            c.text = QColor(0, 50, 60);
        } else {
            c.bg = QColor(207, 216, 220);
            c.border = QColor(55, 71, 79);
            c.text = QColor(40, 40, 40);
        }
        return c;
    }

    if (a == "RW") {
        return rwColors;
    } else if (a == "RO") {
        return roColors;
    } else if (a == "WO") {
        return woColors;
    } else if (a.startsWith("W1") || a.startsWith("W0") || a == "WC" || a == "WS") {
        return w1cColors;
    } else if (a == "RC" || a == "RS") {
        return rcColors;
    }
    return naColors;
}

QPalette ColorScheme::generatePalette() const
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

    qss += QString("QScrollBar:vertical { background: %1; width: 12px; margin: 0px; }\n"
                   "QScrollBar::handle:vertical { background: %2; min-height: 20px; border-radius: 4px; margin: 2px; }\n"
                   "QScrollBar::handle:vertical:hover { background: %3; }\n"
                   "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }\n")
               .arg(windowBg.name(), border.name(), buttonHover.name());

    qss += QString("QScrollBar:horizontal { background: %1; height: 12px; margin: 0px; }\n"
                   "QScrollBar::handle:horizontal { background: %2; min-width: 20px; border-radius: 4px; margin: 2px; }\n"
                   "QScrollBar::handle:horizontal:hover { background: %3; }\n"
                   "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; }\n")
               .arg(windowBg.name(), border.name(), buttonHover.name());

    return qss;
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

    // 1. Solarized 8 (Dark) — Ethan Schoonover / Lifepillar (Default!)
    {
        ColorScheme s;
        s.id = "solarized8";
        s.name = "Solarized 8 (Dark)";
        s.isDark = true;

        s.windowBg       = QColor("#002b36"); // Base03
        s.panelBg        = QColor("#002b36"); // Base03
        s.altRowBg       = QColor("#073642"); // Base02
        s.textColor      = QColor("#839496"); // Base0
        s.textMuted      = QColor("#586e75"); // Base01
        s.headerBg       = QColor("#073642"); // Base02
        s.headerText     = QColor("#93a1a1"); // Base1
        s.border         = QColor("#0d4857"); // Base02/01 mid
        s.selectionBg    = QColor("#1e5666"); // Solarized highlight
        s.selectionText  = QColor("#fdf6e3"); // Base3
        s.buttonBg       = QColor("#073642"); // Base02
        s.buttonHover    = QColor("#0e4654");
        s.inputBg        = QColor("#073642"); // Base02
        s.inputBorder    = QColor("#586e75"); // Base01
        s.errorBg        = QColor("#42161b"); // Soft dark red
        s.errorBorder    = QColor("#dc322f"); // Red
        s.errorText      = QColor("#fdf6e3"); // Base3

        s.rsvdBg         = QColor("#073642"); // Base02
        s.rsvdBorder     = QColor("#586e75"); // Base01
        s.rsvdStripe     = QColor("#002b36"); // Base03
        s.rsvdText       = QColor("#839496"); // Base0
        s.rulerText      = QColor("#586e75"); // Base01

        s.rwColors       = { QColor("#103e2e"), QColor("#859900"), QColor("#859900") }; // Green
        s.roColors       = { QColor("#0e3c54"), QColor("#268bd2"), QColor("#268bd2") }; // Blue
        s.woColors       = { QColor("#44281e"), QColor("#cb4b16"), QColor("#cb4b16") }; // Orange
        s.w1cColors      = { QColor("#3e3518"), QColor("#b58900"), QColor("#b58900") }; // Yellow
        s.rcColors       = { QColor("#2d274c"), QColor("#6c71c4"), QColor("#6c71c4") }; // Violet
        s.naColors       = { QColor("#073642"), QColor("#586e75"), QColor("#839496") }; // Base01

        m_themes.append(s);
    }

    // 2. Solarized 8 (Light)
    {
        ColorScheme s;
        s.id = "solarized8_light";
        s.name = "Solarized 8 (Light)";
        s.isDark = false;

        s.windowBg       = QColor("#fdf6e3"); // Base3
        s.panelBg        = QColor("#fdf6e3"); // Base3
        s.altRowBg       = QColor("#eee8d5"); // Base2
        s.textColor      = QColor("#657b83"); // Base00
        s.textMuted      = QColor("#93a1a1"); // Base1
        s.headerBg       = QColor("#eee8d5"); // Base2
        s.headerText     = QColor("#586e75"); // Base01
        s.border         = QColor("#d3cbb7");
        s.selectionBg    = QColor("#268bd2"); // Blue
        s.selectionText  = QColor("#fdf6e3"); // Base3
        s.buttonBg       = QColor("#eee8d5"); // Base2
        s.buttonHover    = QColor("#e0d7be");
        s.inputBg        = QColor("#ffffff");
        s.inputBorder    = QColor("#93a1a1"); // Base1
        s.errorBg        = QColor("#fadbd8");
        s.errorBorder    = QColor("#dc322f"); // Red
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

        m_themes.append(s);
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

        m_themes.append(s);
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

        m_themes.append(s);
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

        m_themes.append(s);
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

        m_themes.append(s);
    }
}

const QList<ColorScheme>& ThemeManager::availableThemes() const
{
    return m_themes;
}

QStringList ThemeManager::themeIds() const
{
    QStringList ids;
    for (const auto &t : m_themes) {
        ids.append(t.id);
    }
    return ids;
}

QStringList ThemeManager::themeNames() const
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
    QString key = idOrName.trimmed().toLower();
    if (key.isEmpty() || key == "solarized" || key == "solarized8" || key == "solarized8_dark" || key == "solarized_dark" || key == "default" || key == "dark") {
        key = "solarized8";
    } else if (key == "solarized8_light" || key == "solarized_light" || key == "light") {
        key = "solarized8_light";
    }

    for (int i = 0; i < m_themes.size(); ++i) {
        if (m_themes[i].id.toLower() == key || m_themes[i].name.toLower() == key) {
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

AccessColors ThemeManager::getAccessColors(const QString &access, bool colorBlind) const
{
    return currentTheme().getAccessColors(access, colorBlind);
}
