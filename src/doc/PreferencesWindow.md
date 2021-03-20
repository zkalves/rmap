# PreferencesWindow

### 1. Class Overview

`PreferencesWindow` is a non-modal settings dialog for managing global user preferences in **rmap**. It controls UI themes (Solarized 8 Dark/Light, Nord, Dracula, Monokai, Classic) and barrier-free color-blind accessible mode (Okabe-Ito / Wong CVD palette), persisting selections to user configuration in `~/.config/rmap/rmap.conf`.

### 2. Project Structure and Dependencies

Defined in `src/PreferencesWindow.hpp` and implemented in `src/PreferencesWindow.cpp`.
- Instantiated by `RegMapWindow` and opened via **Preferences** menu action.
- Applies themes via `ThemeManager` and saves preferences via `AppSettings`.

Build Requirements:
- Qt 6 modules: `QtWidgets`, `QtCore`, `QtGui`
- UI template form `ui/preferences.ui` processed via `uic`

### 3. Class Hierarchy and Role

`PreferencesWindow` inherits from:
- `QDialog` (Qt Widgets) — Standard dialog behavior and modality management.
- `Ui::preferences` (private multiple inheritance) — Direct access to UI controls in `ui/preferences.ui`.

### 4. Public Slots

#### void accept() [override]
`QDialog` override. Commits preference changes, updates application theme and color-blind modes, saves settings to disk, and closes the dialog.

#### void reject() [override]
`QDialog` override. Reverts uncommitted changes back to active settings and closes the dialog.

#### void apply()
Applies current selections in the dialog to the application immediately without closing the dialog.

### 5. Public Methods

#### explicit PreferencesWindow(QWidget *parent = nullptr)
Constructs the preferences dialog, populates theme dropdown choices, and initializes state from `AppSettings`.

#### ~PreferencesWindow() override = default
Destructor.

#### void setColourScheme(const QString &scheme)
Selects `scheme` in the preferences dialog UI.

#### QString colourScheme() const
#### QString colourScheme() const
#### QString colorScheme() const
Returns the selected theme identifier.

#### void setColourBlindMode(bool enabled)
Sets the color-blind mode checkbox state.

#### bool isColourBlindMode() const
#### bool colorBlindMode() const
Returns `true` if color-blind mode is selected.

#### void saveWindowStateToSettings()
Persists dialog geometry to `AppSettings`.

#### void restoreWindowStateFromSettings()
Restores dialog geometry from `AppSettings`.

### 6. Protected Virtual Methods / Event Handlers

#### void closeEvent(QCloseEvent *event) [override]
`QDialog` override. Saves dialog geometry before closing.

#### void resizeEvent(QResizeEvent *event) [override]
`QWidget` override. Captures size changes.

#### void moveEvent(QMoveEvent *event) [override]
`QWidget` override. Captures dialog position changes.

### 7. Ownership and Lifecycle

Owned and parented to `RegMapWindow`. Displayed non-modally.

### 8. Thread Safety

`PreferencesWindow` is **GUI-thread only**.

### 9. Inter-Class Interactions

- Notifies `RegMapWindow` to trigger `setColourScheme` and `setColourBlindMode`.
- Persists user preferences globally via `AppSettings`.

### 10. Usage Example

```cpp
#include "PreferencesWindow.hpp"

void openPreferences(QWidget *parent)
{
    auto *prefDialog = new PreferencesWindow(parent);
    prefDialog->setColourScheme("dracula");
    prefDialog->setColourBlindMode(false);
    prefDialog->show();
    prefDialog->raise();
}
```
