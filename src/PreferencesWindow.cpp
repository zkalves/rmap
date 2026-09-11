/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "PreferencesWindow.hpp"
#include "ThemeManager.hpp"
#include "AppSettings.hpp"
#include "LanguageManager.hpp"
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QFile>
#include <QShowEvent>

PreferencesWindow::PreferencesWindow(QWidget *parent) :
    QDialog(parent, Qt::Window)
{
    setupUi(this);

    setWindowTitle(tr("Preferences"));
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    resize(500, 280);
    setMinimumSize(400, 220);

    // Initialize Colour Scheme combo
    populateThemes();

    // Initialize Colour-Blind profiles combo
    populateColorBlindModes();
    connect(this->colourBlindMode, &QCheckBox::toggled, this->colourBlindCombo, &QComboBox::setEnabled);

    // Initialize Language combo
    populateLanguages();

    connect(this->btnOpenThemesFolder, &QPushButton::clicked, this, &PreferencesWindow::onOpenThemesFolder);
    connect(&ThemeManager::instance(), &ThemeManager::themesUpdated, this, &PreferencesWindow::populateThemes);

    QPushButton *applyBtn = this->buttonBox->button(QDialogButtonBox::Apply);
    if (applyBtn) { // GCOV_EXCL_BR_LINE - applyBtn is guaranteed by preferences.ui
        connect(applyBtn, &QPushButton::clicked, this, &PreferencesWindow::apply);
    }

    m_colourBlindMode = AppSettings::instance().colorBlindMode();
    m_colourBlindType = AppSettings::instance().colorBlindType();
    m_colourScheme = AppSettings::instance().colorScheme();
    m_language = AppSettings::instance().language();
    updateUiFromState();

    restoreWindowStateFromSettings();
}

void PreferencesWindow::saveStateFromUi()
{
    m_colourBlindMode = this->colourBlindMode->isChecked();
    QString cbId = this->colourBlindCombo->currentData().toString();
    m_colourBlindType = stringToColorBlindMode(cbId);
    if (m_colourBlindType == ColorBlindMode::None) {
        m_colourBlindType = ColorBlindMode::Universal;
    }
    m_colourScheme = this->colourSchemeCombo->currentData().toString();
    if (m_colourScheme.isEmpty()) {
        m_colourScheme = "solarized8";
    }
    m_language = this->languageCombo->currentData().toString();
    if (m_language.isEmpty()) {
        m_language = "en";
    }
}

void PreferencesWindow::updateUiFromState()
{
    this->colourBlindMode->setChecked(m_colourBlindMode);
    this->colourBlindCombo->setEnabled(m_colourBlindMode);
    int cbIdx = this->colourBlindCombo->findData(colorBlindModeToString(m_colourBlindType));
    if (cbIdx >= 0) {
        this->colourBlindCombo->setCurrentIndex(cbIdx);
    }
    int idx = this->colourSchemeCombo->findData(m_colourScheme);
    if (idx >= 0) {
        this->colourSchemeCombo->setCurrentIndex(idx);
    }
    int langIdx = this->languageCombo->findData(m_language);
    if (langIdx >= 0) {
        this->languageCombo->setCurrentIndex(langIdx);
    }
}

void PreferencesWindow::populateColorBlindModes()
{
    QString current = this->colourBlindCombo->currentData().toString();
    if (current.isEmpty()) {
        current = colorBlindModeToString(m_colourBlindType);
    }

    this->colourBlindCombo->clear();
    for (const auto &info : availableColorBlindModes()) {
        this->colourBlindCombo->addItem(info.name, info.id);
    }

    int idx = this->colourBlindCombo->findData(current);
    if (idx >= 0) {
        this->colourBlindCombo->setCurrentIndex(idx);
    }
}

void PreferencesWindow::populateThemes()
{
    QString current = this->colourSchemeCombo->currentData().toString();
    if (current.isEmpty()) {
        current = m_colourScheme;
    }

    this->colourSchemeCombo->clear();
    for (const auto &t : ThemeManager::instance().availableThemes()) {
        this->colourSchemeCombo->addItem(t.name, t.id);
    }

    int idx = this->colourSchemeCombo->findData(current);
    if (idx >= 0) {
        this->colourSchemeCombo->setCurrentIndex(idx);
    }
}

void PreferencesWindow::populateLanguages()
{
    this->languageCombo->clear();
    for (const auto &lang : LanguageManager::instance().availableLanguages()) {
        this->languageCombo->addItem(lang.displayName(), lang.code);
    }
}

void PreferencesWindow::onOpenThemesFolder()
{
    QString themesPath = ThemeManager::userThemesDir();
    QDir dir(themesPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QString templatePath = dir.filePath("template.json");
    if (!QFile::exists(templatePath)) {
        QFile src(":/themes/template.json");
        if (src.open(QIODevice::ReadOnly)) {
            QFile dst(templatePath);
            if (dst.open(QIODevice::WriteOnly)) {
                dst.write(src.readAll());
            }
        }
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(themesPath));
}

void PreferencesWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    ThemeManager::instance().scanThemes();
    populateThemes();
    updateUiFromState();
}

void PreferencesWindow::apply()
{
    saveStateFromUi();
    AppSettings::instance().setColorBlindMode(m_colourBlindMode);
    AppSettings::instance().setColorBlindType(m_colourBlindType);
    AppSettings::instance().setColorScheme(m_colourScheme);
    AppSettings::instance().setLanguage(m_language);
    ThemeManager::instance().setTheme(m_colourScheme);
    ThemeManager::instance().setColorBlindMode(m_colourBlindMode ? m_colourBlindType : ColorBlindMode::None);
    LanguageManager::instance().setLanguage(m_language);
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
    m_colourBlindType = AppSettings::instance().colorBlindType();
    m_colourScheme = AppSettings::instance().colorScheme();
    m_language = AppSettings::instance().language();
    updateUiFromState();
    done(Rejected);
}

void PreferencesWindow::setLanguage(const QString &lang)
{
    m_language = lang.isEmpty() ? QStringLiteral("en") : lang;
    updateUiFromState();
}

QString PreferencesWindow::language() const
{
    return m_language;
}

void PreferencesWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi(this);
        setWindowTitle(tr("Preferences"));
        populateThemes();
        populateColorBlindModes();
        populateLanguages();
        updateUiFromState();
    }
    QDialog::changeEvent(event);
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

void PreferencesWindow::setColourBlindType(ColorBlindMode mode)
{
    m_colourBlindType = (mode == ColorBlindMode::None) ? ColorBlindMode::Universal : mode;
    updateUiFromState();
}

ColorBlindMode PreferencesWindow::colourBlindType() const
{
    return m_colourBlindType;
}

void PreferencesWindow::restoreWindowStateFromSettings()
{
    QByteArray geom = AppSettings::instance().windowGeometry("PreferencesWindow");
    if (!geom.isEmpty()) {
        restoreGeometry(geom);
    } else {
        QSize sz = AppSettings::instance().windowSize("PreferencesWindow", QSize(500, 280));
        QPoint p = AppSettings::instance().windowPos("PreferencesWindow");
        if (sz.width() > 0 && sz.height() > 0) {
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
