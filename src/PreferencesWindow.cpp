#include "PreferencesWindow.hpp"
#include "ThemeManager.hpp"
#include "AppSettings.hpp"
#include <QPushButton>

PreferencesWindow::PreferencesWindow(QWidget *parent) :
    QDialog(parent, Qt::Window)
{
    setupUi(this);

    setWindowTitle(tr("Preferences"));
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    resize(500, 280);
    setMinimumSize(400, 220);

    // Initialize Colour Scheme combo
    this->colourSchemeCombo->clear();
    for (const auto &t : ThemeManager::instance().availableThemes()) {
        this->colourSchemeCombo->addItem(t.name, t.id);
    }

    QPushButton *applyBtn = this->buttonBox->button(QDialogButtonBox::Apply);
    if (applyBtn) {
        connect(applyBtn, &QPushButton::clicked, this, &PreferencesWindow::apply);
    }

    m_colourBlindMode = AppSettings::instance().colorBlindMode();
    m_colourScheme = AppSettings::instance().colorScheme();
    updateUiFromState();

    restoreWindowStateFromSettings();
}

void PreferencesWindow::saveStateFromUi()
{
    m_colourBlindMode = this->colourBlindMode->isChecked();
    m_colourScheme = this->colourSchemeCombo->currentData().toString();
    if (m_colourScheme.isEmpty()) {
        m_colourScheme = "solarized8";
    }
}

void PreferencesWindow::updateUiFromState()
{
    this->colourBlindMode->setChecked(m_colourBlindMode);
    int idx = this->colourSchemeCombo->findData(m_colourScheme);
    if (idx >= 0) {
        this->colourSchemeCombo->setCurrentIndex(idx);
    }
}

void PreferencesWindow::apply()
{
    saveStateFromUi();
    AppSettings::instance().setColorBlindMode(m_colourBlindMode);
    AppSettings::instance().setColorScheme(m_colourScheme);
    ThemeManager::instance().setTheme(m_colourScheme);
}

void PreferencesWindow::accept()
{
    saveWindowStateToSettings();
    apply();
    done(Accepted);
}

void PreferencesWindow::reject()
{
    saveWindowStateToSettings();
    m_colourBlindMode = AppSettings::instance().colorBlindMode();
    m_colourScheme = AppSettings::instance().colorScheme();
    updateUiFromState();
    done(Rejected);
}

void PreferencesWindow::setColourScheme(const QString &scheme)
{
    m_colourScheme = scheme.isEmpty() ? QString("solarized8") : scheme;
    updateUiFromState();
}

QString PreferencesWindow::colourScheme() const
{
    return m_colourScheme;
}

void PreferencesWindow::setColourBlindMode(bool enabled)
{
    m_colourBlindMode = enabled;
    updateUiFromState();
}

bool PreferencesWindow::isColourBlindMode() const
{
    return m_colourBlindMode;
}

void PreferencesWindow::restoreWindowStateFromSettings()
{
    QByteArray geom = AppSettings::instance().windowGeometry("PreferencesWindow");
    if (!geom.isEmpty()) {
        restoreGeometry(geom);
    } else {
        QSize sz = AppSettings::instance().windowSize("PreferencesWindow", QSize(500, 280));
        QPoint p = AppSettings::instance().windowPos("PreferencesWindow");
        if (sz.isValid() && sz.width() > 0 && sz.height() > 0) {
            resize(sz);
        }
        if (!p.isNull()) {
            move(p);
        }
    }
    AppSettings::ensureWindowOnScreen(this, QSize(400, 220), QSize(500, 280));
}

void PreferencesWindow::saveWindowStateToSettings()
{
    AppSettings::instance().setWindowGeometry("PreferencesWindow", saveGeometry());
    AppSettings::instance().setWindowPos("PreferencesWindow", pos());
    AppSettings::instance().setWindowSize("PreferencesWindow", size());
}

void PreferencesWindow::closeEvent(QCloseEvent *event)
{
    saveWindowStateToSettings();
    QDialog::closeEvent(event);
}

void PreferencesWindow::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    AppSettings::instance().setWindowSize("PreferencesWindow", size());
}

void PreferencesWindow::moveEvent(QMoveEvent *event)
{
    QDialog::moveEvent(event);
    AppSettings::instance().setWindowPos("PreferencesWindow", pos());
}
