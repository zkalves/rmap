/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#ifndef APPSETTINGS_HPP
#define APPSETTINGS_HPP

#include <QObject>
#include <QString>
#include <QSize>
#include <QPoint>
#include <QRect>
#include <QByteArray>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>

class QWidget;

class AppSettings : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(AppSettings)

public:
    static AppSettings& instance();

    QString configFilePath() const;
    void setConfigFilePath(const QString &path);

    QString colourScheme() const { return colorScheme(); }
    void setColourScheme(const QString &scheme) { setColorScheme(scheme); }
    QString colorScheme() const;
    void setColorScheme(const QString &scheme);

    bool colourBlindMode() const { return colorBlindMode(); }
    void setColourBlindMode(bool enabled) { setColorBlindMode(enabled); }
    bool colorBlindMode() const;
    void setColorBlindMode(bool enabled);

    // Main Window Geometry, Pos & Size
    QByteArray mainWindowGeometry() const;
    void setMainWindowGeometry(const QByteArray &geom);
    QPoint mainWindowPos() const;
    void setMainWindowPos(const QPoint &pos);
    QSize mainWindowSize() const;
    void setMainWindowSize(const QSize &size);
    QByteArray mainWindowState() const;
    void setMainWindowState(const QByteArray &state);
    QByteArray mainWindowSplitter() const;
    void setMainWindowSplitter(const QByteArray &splitter);

    // Config Window Geometry, Pos & Size
    QByteArray configWindowGeometry() const;
    void setConfigWindowGeometry(const QByteArray &geom);
    QPoint configWindowPos() const;
    void setConfigWindowPos(const QPoint &pos);
    QSize configWindowSize() const;
    void setConfigWindowSize(const QSize &size);

    // Generic Window Geometry, Pos & Size helpers
    QByteArray windowGeometry(const QString &windowName) const;
    void setWindowGeometry(const QString &windowName, const QByteArray &geom);
    QPoint windowPos(const QString &windowName, const QPoint &defaultPos = QPoint()) const;
    void setWindowPos(const QString &windowName, const QPoint &pos);
    QSize windowSize(const QString &windowName, const QSize &defaultSize = QSize()) const;
    void setWindowSize(const QString &windowName, const QSize &size);

    // Ensure widget is fully visible on an available display and >= minimum size
    static void ensureWindowOnScreen(QWidget *widget, const QSize &minSize = QSize(), const QSize &defaultSize = QSize());

    void load();
    void save();

signals:
    void colorSchemeChanged(const QString &scheme);
    void colorBlindModeChanged(bool enabled);

private:
    AppSettings();
    QString m_configPath;
    QString m_colorScheme = QStringLiteral("solarized8");
    bool m_colorBlindMode = false;

    QByteArray m_mainWindowGeometry;
    QByteArray m_mainWindowState;
    QByteArray m_mainWindowSplitter;
    QPoint m_mainWindowPos;
    QSize m_mainWindowSize;

    QByteArray m_configWindowGeometry;
    QPoint m_configWindowPos;
    QSize m_configWindowSize;
};

#endif // APPSETTINGS_HPP
