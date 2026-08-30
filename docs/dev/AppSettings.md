# AppSettings

### 1. Class Overview

`AppSettings` is a singleton configuration service that manages user preferences and GUI state persistence in **rmap**. It stores application settings (active theme, color-blind mode, main window geometry, splitter sizes, dialog positions) in the user configuration directory (`~/.config/rmap/rmap.conf`) using `QSettings` (INI format) and ensures restored windows remain on-screen across multi-monitor setups.

### 2. Project Structure and Dependencies

Defined in `src/AppSettings.hpp` and implemented in `src/AppSettings.cpp`.
- Relies on `QSettings`, `QStandardPaths`, and `QScreen` for platform-independent config resolution.
- Consumed by `RegMapWindow`, `RegConfigWindow`, `PreferencesWindow`, and `ThemeManager`.

Build Requirements:
- Qt 6 modules: `QtCore`, `QtWidgets`, `QtGui`

### 3. Class Hierarchy and Role

`AppSettings` inherits from:
- `QObject` (QtCore) — Provides change notification signals (`colorSchemeChanged`, `colorBlindModeChanged`).

### 4. Signals

#### void colorSchemeChanged(const QString &scheme)
Emitted when the saved color scheme setting changes.

#### void colorBlindModeChanged(bool enabled)
Emitted when the color-blind mode preference changes.

### 5. Public Methods

#### static AppSettings& instance()
Returns the global `AppSettings` singleton instance.

#### QString configFilePath() const
Returns the absolute path to the configuration file (`~/.config/rmap/rmap.conf`).

#### void setConfigFilePath(const QString &path)
Overrides the configuration file destination (primarily used in automated unit tests).

#### QString colorScheme() const / QString colourScheme() const
Returns the persisted color scheme ID.

#### void setColorScheme(const QString &scheme) / void setColourScheme(const QString &scheme)
Updates and persists the active color scheme setting, emitting `colorSchemeChanged`.

#### bool colorBlindMode() const / bool colourBlindMode() const
Returns `true` if color-blind mode is enabled.

#### void setColorBlindMode(bool enabled) / void setColourBlindMode(bool enabled)
Updates and persists the color-blind mode setting, emitting `colorBlindModeChanged`.

#### QByteArray mainWindowGeometry() const
#### void setMainWindowGeometry(const QByteArray &geom)
#### QPoint mainWindowPos() const
#### void setMainWindowPos(const QPoint &pos)
#### QSize mainWindowSize() const
#### void setMainWindowSize(const QSize &size)
#### QByteArray mainWindowState() const
#### void setMainWindowState(const QByteArray &state)
#### QByteArray mainWindowSplitter() const
#### void setMainWindowSplitter(const QByteArray &splitter)
Getters and setters for main window window state, coordinates, sizes, and splitter layout.

#### QByteArray configWindowGeometry() const
#### void setConfigWindowGeometry(const QByteArray &geom)
#### QPoint configWindowPos() const
#### void setConfigWindowPos(const QPoint &pos)
#### QSize configWindowSize() const
#### void setConfigWindowSize(const QSize &size)
Getters and setters for `RegConfigWindow` position and geometry.

#### QByteArray windowGeometry(const QString &windowName) const
#### void setWindowGeometry(const QString &windowName, const QByteArray &geom)
#### QPoint windowPos(const QString &windowName, const QPoint &defaultPos = QPoint()) const
#### void setWindowPos(const QString &windowName, const QPoint &pos)
#### QSize windowSize(const QString &windowName, const QSize &defaultSize = QSize()) const
#### void setWindowSize(const QString &windowName, const QSize &size)
Generic window state helpers for arbitrary dialogs.

#### static void ensureWindowOnScreen(QWidget *widget, const QSize &minSize = QSize(), const QSize &defaultSize = QSize())
Inspects all active screen geometries via `QGuiApplication::screens()` and adjusts `widget` position and size to ensure it is fully visible on a connected monitor and meets minimum dimensions.

#### void load()
Reads all persisted keys from `rmap.conf`.

#### void save()
Flushes in-memory settings to `rmap.conf`.

### 6. Ownership and Lifecycle

`AppSettings` is a process-lifetime singleton created on first invocation of `AppSettings::instance()`.

### 7. Thread Safety

`AppSettings` is **GUI-thread only**.

### 8. Inter-Class Interactions

- Loaded at application startup in `main.cpp` and `RegMapWindow::restoreWindowStateFromSettings()`.
- Updated when closing dialogs or modifying preferences in `PreferencesWindow`.

### 9. Usage Example

```cpp
#include "AppSettings.hpp"
#include <QWidget>

void saveAndRestore(QWidget *window)
{
    AppSettings &settings = AppSettings::instance();

    // Restore size and ensure on-screen
    QSize savedSize = settings.mainWindowSize();
    if (savedSize.isValid()) {
        window->resize(savedSize);
    }
    AppSettings::ensureWindowOnScreen(window, QSize(640, 480));

    // Persist on close
    settings.setMainWindowSize(window->size());
    settings.setMainWindowPos(window->pos());
    settings.save();
}
```
