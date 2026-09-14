# AboutWindow

### 1. Class Overview

`AboutWindow` is a non-modal information and diagnostic dialog in **rmap**. It displays comprehensive application metadata, including the semantic version, application description, architectural features, third-party component licenses, and copyright information across four tabbed views. It persists its window dimensions and position across sessions via `AppSettings`.

### 2. Project Structure and Dependencies

Defined in `src/AboutWindow.hpp` and implemented in `src/AboutWindow.cpp`.
- Instantiated by `RegMapWindow` and opened via **Help -> About rmap** menu action.
- Uses `AppSettings` for dialog geometry persistence.
- Uses `RmapVersion` via `QApplication::applicationVersion()` for SemVer reporting.

Build Requirements:
- Qt 6 modules: `QtWidgets`, `QtCore`, `QtGui`
- UI template form `ui/about.ui` processed via `uic`

### 3. Class Hierarchy and Role

`AboutWindow` inherits from:
- `QDialog` (Qt Widgets) — Standard dialog behavior with non-modal window flags (`Qt::Window`).
- `Ui::about` (private multiple inheritance) — Direct access to UI controls in `ui/about.ui`.

### 4. Public Methods

#### explicit AboutWindow(QWidget *parent = nullptr)
Constructs the about dialog, configures window flags (close, minimize, maximize), sets minimum size constraints, initializes the rich text tab contents, and restores saved geometry from `AppSettings`.

#### ~AboutWindow() override = default
Default virtual destructor.

#### QString applicationVersion() const
Returns the active application version string reported by `QApplication::applicationVersion()`, falling back to a default version tag if unset.

#### QString applicationName() const
Returns the application name string reported by `QApplication::applicationName()`.

#### void saveWindowStateToSettings()
Persists dialog size and screen position to user configuration via `AppSettings`.

#### void restoreWindowStateFromSettings()
Restores dialog size and screen position from `AppSettings`.

### 5. Public Slots

#### void accept() [override]
`QDialog` override. Saves dialog window geometry and closes the dialog.

### 6. Protected Virtual Methods / Event Handlers

#### void closeEvent(QCloseEvent *event) [override]
Captures dialog close events and persists window geometry to settings before accepting the event.

#### void resizeEvent(QResizeEvent *event) [override]
Captures dialog resize events and updates saved geometry.

#### void moveEvent(QMoveEvent *event) [override]
Captures dialog position adjustments and updates saved geometry.

#### void changeEvent(QEvent *event) [override]
Handles dynamic language and palette changes at runtime.

### 7. Ownership and Lifecycle

Owned and parented to `RegMapWindow`. Displayed non-modally so users can reference feature and licensing details while continuing to edit register maps.

### 8. Thread Safety

`AboutWindow` is **GUI-thread only**.

### 9. Inter-Class Interactions

- Interacts with `AppSettings` to persist and restore window geometry (`dialogs/about_width`, `dialogs/about_height`).
- Formats rich HTML content describing supported industry standards (Accellera SystemRDL, IEEE 1685 IP-XACT, ARM CMSIS-SVD, Google Protobuf) and open-source licenses (MPL-2.0, MIT, Apache-2.0).

### 10. Usage Example

```cpp
#include "AboutWindow.hpp"

void showAboutDialog(QWidget *parent)
{
    auto *about = new AboutWindow(parent);
    about->show();
    about->raise();
    about->activateWindow();
}
```
