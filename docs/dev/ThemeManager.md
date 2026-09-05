# ThemeManager

### 1. Class Overview

`ThemeManager` is a singleton service providing multi-theme styling, palette generation, and accessibility color management across **rmap**. It registers 6 comprehensive color schemes (`solarized8`, `solarized8_light`, `nord`, `dracula`, `monokai`, `classic`), generates matching QSS Qt stylesheets and `QPalette` instances, and provides Okabe-Ito / Wong CVD barrier-free color mappings for color-blind accessibility.

### 2. Project Structure and Dependencies

Defined in `src/ThemeManager.hpp` and implemented in `src/ThemeManager.cpp`.
- Provides styling for `RegMapWindow`, `RegBitfieldBarWidget`, `BlockMemoryMapWidget`, and `RegMapDelegate`.
- Persisted across sessions via `AppSettings`.

Build Requirements:
- Qt 6 modules: `QtCore`, `QtGui`

### 3. Class Hierarchy and Role

`ThemeManager` inherits from:
- `QObject` (QtCore) — Provides signal/slot communication (`themeChanged` signal).

### 4. Auxiliary Data Structures

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
| `id` | `QString` | Unique theme identifier (e.g. `"nord"`, `"dracula"`). |
| `name` | `QString` | Human-readable name (e.g. `"Nord Dark"`). |
| `isDark` | `bool` | `true` for dark themes, `false` for light themes. |
| `windowBg`, `panelBg`, `border` | `QColor` | Base surface colors. |
| `textColor`, `textMuted` | `QColor` | Typography colors. |
| `rsvdBg`, `rsvdBorder`, `rsvdStripe` | `QColor` | Bitfield slice reserved gap colors. |
| `rwColors`, `roColors`, `woColors` | `AccessColors` | Access policy color palettes. |

Methods on `ColorScheme`:
- `AccessColors getAccessColors(const QString &access, bool colorBlind) const`: Resolves access policy colors.
- `QString generateStyleSheet() const`: Generates complete Qt stylesheet CSS string for main window and dialogs.
- `QPalette generatePalette() const`: Constructs a `QPalette` for standard widget rendering.

### 5. Signals

#### void themeChanged(const ColorScheme &newTheme)
Emitted whenever the active theme is changed via `setTheme()`.

### 6. Public Methods

#### static ThemeManager& instance()
Returns the global `ThemeManager` singleton reference.

#### const QList<ColorScheme>& availableThemes() const
Returns the list of all 6 registered color schemes.

#### QStringList themeIds() const
Returns a list of all theme IDs (`["solarized8", "solarized8_light", "nord", "dracula", "monokai", "classic"]`).

#### QStringList themeNames() const
Returns human-readable theme names.

#### const ColorScheme& currentTheme() const
Returns the currently active `ColorScheme`.

#### QString currentThemeId() const
Returns the active theme ID.

#### QString currentThemeName() const
Returns the active theme display name.

#### bool setTheme(const QString &idOrName)
Switches the active theme by matching `idOrName` against registered IDs or names. Updates internal state and emits `themeChanged`. Returns `true` on match.

#### AccessColors getAccessColors(const QString &access, bool colorBlind) const
#### AccessColors accessColors(const QString &access, bool colorBlind) const
Convenience method resolving access colors under the current theme.

### 7. Ownership and Lifecycle

`ThemeManager` is a static singleton initialized on first call to `ThemeManager::instance()`.

### 8. Thread Safety

`ThemeManager` is **GUI-thread only** for palette and theme mutations.

### 9. Inter-Class Interactions

- Connected to `RegMapWindow::setColourScheme()`, `PreferencesWindow`, and custom widget painters.
- Notifies views via `themeChanged` to trigger stylesheet re-application and repainting.

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
