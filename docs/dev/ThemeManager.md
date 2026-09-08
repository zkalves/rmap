# ThemeManager

### 1. Class Overview

`ThemeManager` is a singleton service providing multi-theme styling, palette generation, and accessibility color management across **rmap**. It registers 8 comprehensive color schemes (`solarized8`, `solarized8_light`, `nord`, `dracula`, `monokai`, `classic`, `high_contrast_dark`, `high_contrast_light`), generates matching QSS Qt stylesheets and `QPalette` instances, and provides scientifically verified Colour Vision Deficiency (CVD) palettes for Universal (Okabe-Ito), Deuteranopia, Protanopia, Tritanopia, and Achromatopsia accessibility.

### 2. Project Structure and Dependencies

Defined in `src/ThemeManager.hpp` and implemented in `src/ThemeManager.cpp`.
- Provides styling for `RegMapWindow`, `RegBitfieldBarWidget`, `BlockMemoryMapWidget`, and `RegMapDelegate`.
- Persisted across sessions via `AppSettings`.

Build Requirements:
- Qt 6 modules: `QtCore`, `QtGui`

### 3. Class Hierarchy and Role

`ThemeManager` inherits from:
- `QObject` (QtCore) — Provides signal/slot communication (`themeChanged`, `themesUpdated`, and `colorBlindModeChanged` signals).

### 4. Auxiliary Data Structures

#### ColorBlindMode Enum
Represents supported accessibility color modes:

| Value | Name | Description |
|---|---|---|
| `ColorBlindMode::None` | Standard | Standard palette (color-blind mode disabled). |
| `ColorBlindMode::Universal` | Universal | Okabe-Ito / Wong universal barrier-free palette across all cone deficiencies. |
| `ColorBlindMode::Deuteranopia` | Deuteranopia | Medium-wavelength M-cone deficiency (~6% of males; green-blind/weak). |
| `ColorBlindMode::Protanopia` | Protanopia | Long-wavelength L-cone deficiency (~2% of males; red-blind/weak). |
| `ColorBlindMode::Tritanopia` | Tritanopia | Short-wavelength S-cone deficiency (blue-blind/weak). |
| `ColorBlindMode::Achromatopsia` | Achromatopsia | Total color blindness with distinct luminance steps (13%–100%). |

#### AccessColors
Contains text, border, and background `QColor` definitions for an access policy badge:

| Member | Type | Description |
|---|---|---|
| `bg` | `QColor` | Background fill color. |
| `border` | `QColor` | Border stroke color. |
| `text` | `QColor` | Foreground text color. |

#### ColorScheme
Comprehensive theme specification defining 30+ colors for UI surfaces, table borders, selections, errors, and bitfield visualizer elements:

| Member | Type | Description |
|---|---|---|
| `id` | `QString` | Unique theme identifier (e.g. `"nord"`, `"high_contrast_dark"`). |
| `name` | `QString` | Human-readable name (e.g. `"High Contrast (Dark)"`). |
| `isDark` | `bool` | `true` for dark themes, `false` for light themes. |
| `windowBg`, `panelBg`, `border` | `QColor` | Base surface colors. |
| `textColor`, `textMuted` | `QColor` | Typography colors. |
| `rsvdBg`, `rsvdBorder`, `rsvdStripe` | `QColor` | Bitfield slice reserved gap colors. |
| `rwColors`, `roColors`, `woColors` | `AccessColors` | Access policy color palettes. |

Methods on `ColorScheme`:
- `AccessColors getAccessColors(const QString &access, ColorBlindMode mode) const`: Resolves access policy colors for a CVD mode.
- `AccessColors getAccessColors(const QString &access, bool colorBlind) const`: Backward-compatible resolution.
- `QString generateStyleSheet() const`: Generates complete Qt stylesheet CSS string for main window and dialogs.
- `QPalette generatePalette() const`: Constructs a `QPalette` for standard widget rendering.
- `bool fromJson(const QJsonObject &obj)`: Deserializes theme colors from standard JSON schema.
- `QJsonObject toJson() const`: Serializes theme configuration into JSON.
- `static ColorScheme createDefault(const QString &id)`: Returns in-code fallback scheme.
- `static QList<ColorScheme> builtInDefaults()`: Returns all 8 default built-in color schemes.

### 5. Signals

#### void themeChanged(const ColorScheme &newTheme)
Emitted whenever the active theme is changed via `setTheme()`.

#### void themesUpdated()
Emitted when themes are dynamically registered, rescanned, or updated from files.

#### void colorBlindModeChanged(ColorBlindMode newMode)
Emitted whenever the active colour-blind mode or profile changes.

### 6. Public Methods

#### static ThemeManager& instance()
Returns the global `ThemeManager` singleton reference.

#### const QList<ColorScheme>& availableThemes() const
Returns the list of all available color schemes (built-in defaults plus dynamically loaded JSON themes).

#### QStringList themeIds() const
Returns a list of all active theme IDs.

#### QStringList themeNames() const
Returns human-readable theme names.

#### const ColorScheme& currentTheme() const
Returns the currently active `ColorScheme`.

#### QString currentThemeId() const
Returns the active theme ID.

#### QString currentThemeName() const
Returns the active theme display name.

#### bool setTheme(const QString &idOrName)
Switches the active theme by matching `idOrName` against registered IDs, names, or file paths. Updates internal state and emits `themeChanged`. Returns `true` on match.

#### bool registerTheme(const ColorScheme &scheme)
Registers a new theme or updates an existing theme with the same ID, emitting `themesUpdated`.

#### bool loadThemeFromJson(const QString &jsonPathOrContent)
Loads a theme from a JSON file path or raw JSON string content and registers it.

#### int scanThemesDir(const QString &dirPath)
Scans a directory for `*.json` theme files (ignoring `template.json`) and registers any found themes.

#### void scanThemes()
Scans all directories in `searchPaths()` for custom and installed themes.

#### static QString userThemesDir()
Returns the user theme directory (`~/.config/rmap/themes`).

#### static QString localThemesDir()
Returns the local working directory theme path (`./themes`).

#### QStringList searchPaths() const
Returns the prioritized list of theme search paths (`RMAP_THEMES_PATH`, custom paths, system install, local `./themes`, and user config).

#### void addSearchPath(const QString &path)
Adds a custom directory path to the search paths.

#### ColorBlindMode colorBlindMode() const
Returns the currently active `ColorBlindMode` enum value.

#### QString colorBlindModeId() const
Returns the string identifier of the active colour-blind mode (e.g. `"deuteranopia"`).

#### void setColorBlindMode(ColorBlindMode mode)
#### void setColorBlindMode(const QString &modeId)
#### void setColorBlindMode(bool enabled)
Sets the active colour-blind mode and emits `colorBlindModeChanged`.

#### AccessColors getAccessColors(const QString &access, ColorBlindMode mode) const
#### AccessColors getAccessColors(const QString &access, bool colorBlind) const
#### AccessColors accessColors(const QString &access, ColorBlindMode mode) const
#### AccessColors accessColors(const QString &access, bool colorBlind) const
Convenience methods resolving access policy colors under the current theme for the specified CVD mode.

### 7. Ownership and Lifecycle

`ThemeManager` is a static singleton initialized on first call to `ThemeManager::instance()`. Built-in default schemes are embedded via Qt resources (`:/themes/*.json`) and backed by C++ fallbacks. Custom themes can be added at runtime to `~/.config/rmap/themes/` or loaded directly via CLI.

### 8. Thread Safety

`ThemeManager` is **GUI-thread only** for palette and theme mutations.

### 9. Inter-Class Interactions

- Connected to `RegMapWindow::setColourScheme()`, `PreferencesWindow`, and custom widget painters.
- Notifies views via `themeChanged` and `themesUpdated` to trigger stylesheet re-application, menu rebuilding, and repainting.

### 10. Usage Example

```cpp
#include <QApplication>
#include "ThemeManager.hpp"

void switchApplicationTheme(const QString &themeId)
{
    ThemeManager &tm = ThemeManager::instance();
    if (tm.setTheme(themeId)) {
        const ColorScheme &theme = tm.currentTheme();
        qApp->setStyleSheet(theme.generateStyleSheet());
        qApp->setPalette(theme.generatePalette());
    }
}
```
