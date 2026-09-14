/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "AppSettings.hpp"
#include <QCoreApplication>

#include <QWidget>
#include <QScreen>
#include <QGuiApplication>

AppSettings& AppSettings::instance()
{
    static AppSettings inst;
    return inst;
}

static int s_screenOverrideMode = 0;

void AppSettings::setScreenOverrideMode(int mode)
{
    s_screenOverrideMode = mode;
}

QString AppSettings::determineConfigPath(const char *envConfig, const char *xdgConfig, const QString &genericConfigLoc)
{
    if (envConfig && envConfig[0] != '\0') {
        return QString::fromUtf8(envConfig);
    }
    QString configDir = (xdgConfig && xdgConfig[0] != '\0')
        ? QString::fromUtf8(xdgConfig)
        : genericConfigLoc;
    if (configDir.isEmpty()) {
        configDir = QDir::homePath() + "/.config";
    }
    return configDir + "/rmap/rmap.conf";
}

AppSettings::AppSettings()
{
    m_configPath = determineConfigPath(
        std::getenv("RMAP_CONFIG_FILE"),
        std::getenv("XDG_CONFIG_HOME"),
        QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation));

    load();
}

QString AppSettings::configFilePath() const
{
    return m_configPath;
}

void AppSettings::setConfigFilePath(const QString &path)
{
    if (!path.isEmpty() && m_configPath != path) {
        m_configPath = path;
        load();
    }
}

QString AppSettings::colorScheme() const
{
    return m_colorScheme;
}

void AppSettings::setColorScheme(const QString &scheme)
{
    QString s = scheme.trimmed().toLower();
    if (s.isEmpty()) s = "solarized8";
    if (m_colorScheme != s) {
        m_colorScheme = s;
        save();
        emit colorSchemeChanged(m_colorScheme);
    }
}

bool AppSettings::colorBlindMode() const
{
    return m_colorBlindMode;
}

void AppSettings::setColorBlindMode(bool enabled)
{
    if (m_colorBlindMode != enabled) {
        m_colorBlindMode = enabled;
        save();
        emit colorBlindModeChanged(m_colorBlindMode);
    }
}

ColorBlindMode AppSettings::colorBlindType() const
{
    return m_colorBlindType;
}

void AppSettings::setColorBlindType(ColorBlindMode mode)
{
    if (m_colorBlindType != mode) {
        m_colorBlindType = mode;
        save();
        emit colorBlindTypeChanged(m_colorBlindType);
    }
}

QString AppSettings::colorBlindTypeString() const
{
    return colorBlindModeToString(m_colorBlindType);
}

void AppSettings::setColorBlindTypeString(const QString &type)
{
    setColorBlindType(stringToColorBlindMode(type));
}

QString AppSettings::language() const
{
    return m_language;
}

void AppSettings::setLanguage(const QString &lang)
{
    QString l = lang.trimmed();
    if (l.isEmpty()) l = QStringLiteral("en");
    if (m_language != l) {
        m_language = l;
        save();
        emit languageChanged(m_language);
    }
}

QByteArray AppSettings::mainWindowGeometry() const
{
    return m_mainWindowGeometry;
}

void AppSettings::setMainWindowGeometry(const QByteArray &geom)
{
    m_mainWindowGeometry = geom;
    save();
}

QPoint AppSettings::mainWindowPos() const
{
    return m_mainWindowPos;
}

void AppSettings::setMainWindowPos(const QPoint &pos)
{
    m_mainWindowPos = pos;
    save();
}

QSize AppSettings::mainWindowSize() const
{
    return m_mainWindowSize;
}

void AppSettings::setMainWindowSize(const QSize &size)
{
    if (size.isValid() && size.width() > 0 && size.height() > 0) {
        m_mainWindowSize = size;
        save();
    }
}

QByteArray AppSettings::mainWindowState() const
{
    return m_mainWindowState;
}

void AppSettings::setMainWindowState(const QByteArray &state)
{
    m_mainWindowState = state;
    save();
}

QByteArray AppSettings::mainWindowSplitter() const
{
    return m_mainWindowSplitter;
}

void AppSettings::setMainWindowSplitter(const QByteArray &splitter)
{
    m_mainWindowSplitter = splitter;
    save();
}

QByteArray AppSettings::configWindowGeometry() const
{
    return m_configWindowGeometry;
}

void AppSettings::setConfigWindowGeometry(const QByteArray &geom)
{
    m_configWindowGeometry = geom;
    save();
}

QPoint AppSettings::configWindowPos() const
{
    return m_configWindowPos;
}

void AppSettings::setConfigWindowPos(const QPoint &pos)
{
    m_configWindowPos = pos;
    save();
}

QSize AppSettings::configWindowSize() const
{
    return m_configWindowSize;
}

void AppSettings::setConfigWindowSize(const QSize &size)
{
    if (size.isValid() && size.width() > 0 && size.height() > 0) {
        m_configWindowSize = size;
        save();
    }
}

QByteArray AppSettings::windowGeometry(const QString &windowName) const
{
    QSettings settings(m_configPath, QSettings::IniFormat);
    return settings.value(QString("Geometry/%1").arg(windowName)).toByteArray();
}

void AppSettings::setWindowGeometry(const QString &windowName, const QByteArray &geom)
{
    QSettings settings(m_configPath, QSettings::IniFormat);
    settings.setValue(QString("Geometry/%1").arg(windowName), geom);
    settings.sync();
}

QPoint AppSettings::windowPos(const QString &windowName, const QPoint &defaultPos) const
{
    QSettings settings(m_configPath, QSettings::IniFormat);
    bool hasX = settings.contains(QString("Geometry/%1X").arg(windowName));
    bool hasY = settings.contains(QString("Geometry/%1Y").arg(windowName));
    if (hasX && hasY) {
        int x = settings.value(QString("Geometry/%1X").arg(windowName)).toInt();
        int y = settings.value(QString("Geometry/%1Y").arg(windowName)).toInt();
        return QPoint(x, y);
    }
    return defaultPos;
}

void AppSettings::setWindowPos(const QString &windowName, const QPoint &pos)
{
    QSettings settings(m_configPath, QSettings::IniFormat);
    settings.setValue(QString("Geometry/%1X").arg(windowName), pos.x());
    settings.setValue(QString("Geometry/%1Y").arg(windowName), pos.y());
    settings.sync();
}

QSize AppSettings::windowSize(const QString &windowName, const QSize &defaultSize) const
{
    QSettings settings(m_configPath, QSettings::IniFormat);
    int w = settings.value(QString("Geometry/%1Width").arg(windowName), defaultSize.width()).toInt();
    int h = settings.value(QString("Geometry/%1Height").arg(windowName), defaultSize.height()).toInt();
    if (w > 0 && h > 0) {
        return QSize(w, h);
    }
    return defaultSize;
}

void AppSettings::setWindowSize(const QString &windowName, const QSize &size)
{
    if (size.isValid() && size.width() > 0 && size.height() > 0) {
        QSettings settings(m_configPath, QSettings::IniFormat);
        settings.setValue(QString("Geometry/%1Width").arg(windowName), size.width());
        settings.setValue(QString("Geometry/%1Height").arg(windowName), size.height());
        settings.sync();
    }
}

void AppSettings::ensureWindowOnScreen(QWidget *widget, const QSize &minSize, const QSize &defaultSize)
{
    if (!widget) return;

    int minW = minSize.width() > 0 ? minSize.width() : widget->minimumWidth();
    if (minW <= 0) minW = 200;
    int minH = minSize.height() > 0 ? minSize.height() : widget->minimumHeight();
    if (minH <= 0) minH = 150;

    int curW = widget->width();
    int curH = widget->height();

    // If dimensions are too small or invalid, enlarge to minSize or defaultSize
    int effectiveMinW = (minW > 0) ? minW : 1;
    if (curW < effectiveMinW) {
        curW = (defaultSize.width() >= effectiveMinW) ? defaultSize.width() : effectiveMinW;
    }
    int effectiveMinH = (minH > 0) ? minH : 1;
    if (curH < effectiveMinH) {
        curH = (defaultSize.height() >= effectiveMinH) ? defaultSize.height() : effectiveMinH;
    }

    QList<QScreen*> screens = QGuiApplication::screens();
    QScreen *targetScreen = nullptr;

    if (!screens.isEmpty()) {
        // Find screen with largest intersection with window rectangle
        QRect windowRect(widget->pos(), QSize(curW, curH));
        int maxIntersectionArea = 0;
        for (QScreen *s : screens) {
            QRect avail = s->availableGeometry();
            QRect inter = avail.intersected(windowRect);
            int area = inter.width() * inter.height();
            if (area > maxIntersectionArea) {
                maxIntersectionArea = area;
                targetScreen = s;
            }
        }

        // If no overlap, check screen at window position
        if (!targetScreen) {
            targetScreen = QGuiApplication::screenAt(widget->pos());
        }

        // Fallback to primary screen
        if (!targetScreen) {
            if (s_screenOverrideMode != 1 && s_screenOverrideMode != 2) {
                targetScreen = QGuiApplication::primaryScreen();
            }
            if (!targetScreen && !screens.isEmpty() && s_screenOverrideMode != 2) {
                targetScreen = screens.first();
            }
        }
    }

    if (targetScreen) {
        QRect avail = targetScreen->availableGeometry();

        // Clamp size to screen if window is wider/taller than the display
        if (curW > avail.width()) {
            curW = avail.width();
        }
        if (curH > avail.height()) {
            curH = avail.height();
        }

        int curX = widget->x();
        int curY = widget->y();

        // If window extends past right/bottom edges, bring it back
        if (curX + curW > avail.right()) {
            curX = avail.right() - curW;
        }
        // If window extends past left/top edges, bring it back
        if (curX < avail.left()) {
            curX = avail.left();
        }
        if (curY + curH > avail.bottom()) {
            curY = avail.bottom() - curH;
        }
        if (curY < avail.top()) {
            curY = avail.top();
        }

        widget->resize(curW, curH);
        widget->move(curX, curY);
    } else {
        widget->resize(curW, curH);
    }
}

void AppSettings::load()
{
    QFileInfo fi(m_configPath);
    if (!fi.exists()) {
        m_colorScheme = "solarized8";
        m_colorBlindMode = false;
        m_mainWindowSize = QSize(1200, 800);
        m_configWindowSize = QSize(780, 700);
        return;
    }

    QSettings settings(m_configPath, QSettings::IniFormat);
    m_colorScheme = settings.value("Appearance/ColorScheme", "solarized8").toString();
    if (m_colorScheme.isEmpty()) {
        m_colorScheme = "solarized8";
    }
    m_colorBlindMode = settings.value("Appearance/ColorBlindMode", false).toBool();
    QString cbType = settings.value("Appearance/ColorBlindType", "universal").toString();
    m_colorBlindType = stringToColorBlindMode(cbType);
    if (m_colorBlindType == ColorBlindMode::None) {
        m_colorBlindType = ColorBlindMode::Universal;
    }
    m_language = settings.value("Appearance/Language", "en").toString();
    if (m_language.isEmpty()) {
        m_language = "en";
    }

    m_mainWindowGeometry = settings.value("Geometry/MainWindow").toByteArray();
    m_mainWindowState = settings.value("Geometry/MainWindowState").toByteArray();
    m_mainWindowSplitter = settings.value("Geometry/MainWindowSplitter").toByteArray();
    if (settings.contains("Geometry/MainWindowX") && settings.contains("Geometry/MainWindowY")) {
        m_mainWindowPos = QPoint(settings.value("Geometry/MainWindowX").toInt(),
                                 settings.value("Geometry/MainWindowY").toInt());
    }
    int mainW = settings.value("Geometry/MainWindowWidth", 1200).toInt();
    int mainH = settings.value("Geometry/MainWindowHeight", 800).toInt();
    m_mainWindowSize = QSize(mainW > 0 ? mainW : 1200, mainH > 0 ? mainH : 800);

    m_configWindowGeometry = settings.value("Geometry/ConfigWindow").toByteArray();
    if (settings.contains("Geometry/ConfigWindowX") && settings.contains("Geometry/ConfigWindowY")) {
        m_configWindowPos = QPoint(settings.value("Geometry/ConfigWindowX").toInt(),
                                   settings.value("Geometry/ConfigWindowY").toInt());
    }
    int cfgW = settings.value("Geometry/ConfigWindowWidth", 780).toInt();
    int cfgH = settings.value("Geometry/ConfigWindowHeight", 700).toInt();
    m_configWindowSize = QSize(cfgW > 0 ? cfgW : 780, cfgH > 0 ? cfgH : 700);
}

void AppSettings::save()
{
    QFileInfo fi(m_configPath);
    QDir dir = fi.dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QSettings settings(m_configPath, QSettings::IniFormat);
    settings.setValue("Appearance/ColorScheme", m_colorScheme);
    settings.setValue("Appearance/ColorBlindMode", m_colorBlindMode);
    settings.setValue("Appearance/ColorBlindType", colorBlindModeToString(m_colorBlindType));
    settings.setValue("Appearance/Language", m_language);

    if (!m_mainWindowGeometry.isEmpty()) {
        settings.setValue("Geometry/MainWindow", m_mainWindowGeometry);
    }
    if (!m_mainWindowState.isEmpty()) {
        settings.setValue("Geometry/MainWindowState", m_mainWindowState);
    }
    if (!m_mainWindowSplitter.isEmpty()) {
        settings.setValue("Geometry/MainWindowSplitter", m_mainWindowSplitter);
    }
    if (!m_mainWindowPos.isNull()) {
        settings.setValue("Geometry/MainWindowX", m_mainWindowPos.x());
        settings.setValue("Geometry/MainWindowY", m_mainWindowPos.y());
    }
    if (m_mainWindowSize.width() > 0 && m_mainWindowSize.height() > 0) {
        settings.setValue("Geometry/MainWindowWidth", m_mainWindowSize.width());
        settings.setValue("Geometry/MainWindowHeight", m_mainWindowSize.height());
    }

    if (!m_configWindowGeometry.isEmpty()) {
        settings.setValue("Geometry/ConfigWindow", m_configWindowGeometry);
    }
    if (!m_configWindowPos.isNull()) {
        settings.setValue("Geometry/ConfigWindowX", m_configWindowPos.x());
        settings.setValue("Geometry/ConfigWindowY", m_configWindowPos.y());
    }
    if (m_configWindowSize.width() > 0 && m_configWindowSize.height() > 0) {
        settings.setValue("Geometry/ConfigWindowWidth", m_configWindowSize.width());
        settings.setValue("Geometry/ConfigWindowHeight", m_configWindowSize.height());
    }
    settings.sync();
}
