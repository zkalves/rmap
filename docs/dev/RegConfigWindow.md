# RegConfigWindow

### 1. Class Overview

`RegConfigWindow` is a non-modal configuration dialog for managing project-level parameters and code generation template mappings in **rmap**. It configures project metadata (name, version, register bus width: 8, 16, 32, 64 bits), manages template-to-output file generation mappings, maintains custom key-value template parameters, supports base directory relativization, and serializes settings directly to Protocol Buffer messages (`protormap::Config`).

### 2. Project Structure and Dependencies

Defined in `src/RegConfigWindow.hpp` and implemented in `src/RegConfigWindow.cpp`.
- Instantiated by `RegMapWindow` and displayed via toolbar/menu action (**Project → Configure**).
- Serializes project parameters to `protormap::Config` schema in `proto/rmap.proto`.
- Interfaced with `PathUtils` and `AppSettings`.

Build Requirements:
- Qt 6 modules: `QtWidgets`, `QtCore`, `QtGui`
- Protocol Buffers runtime library (`libprotobuf`)
- UI template form `ui/config.ui` processed via `uic`

### 3. Class Hierarchy and Role

`RegConfigWindow` inherits from:
- `QDialog` (Qt Widgets) — Provides dialog window behavior, standard button handling (`accept`, `reject`), and modality control.
- `Ui::config` (private multiple inheritance) — Direct compile-time access to widgets in `ui/config.ui`.

### 4. Public Slots

#### void addTemplateRow(const QString &tmpl = "", const QString &out = "")
Appends a new template mapping row to the template configuration table.

#### void addParameterRow(const QString &key = "", const QString &val = "")
Appends a custom key-value parameter row to the user parameter table.

#### void accept() [override]
`QDialog` override. Commits UI values to internal state, updates project settings, and closes the dialog.

#### void reject() [override]
`QDialog` override. Reverts any uncommitted UI modifications back to the last saved state and closes the dialog.

### 5. Public Methods

#### explicit RegConfigWindow(QWidget *parent = nullptr)
Constructs the non-modal configuration dialog, initializes table delegates, loads default values, and connects browse actions.

#### ~RegConfigWindow() override = default
Destructor.

#### protormap::Config* serialize()
Allocates and returns a new `protormap::Config` Protocol Buffer object reflecting current project configuration settings (caller assumes ownership of the returned pointer).

#### void deserialize(const protormap::Config &config)
Loads project settings, template mappings, register width, and custom parameters from `config`.

#### void setRegisterWidth(uint32_t width)
Sets the default register bus bit width (e.g. 32 or 64).

#### uint32_t registerWidth() const
#### uint32_t registerWidth() const
Returns the configured register bit width.

#### void setProjectName(const QString &name)
Sets the project identifier string.

#### QString projectName() const
Returns the project name.

#### void setProjectVersion(const QString &version)
Sets the project version string.

#### QString projectVersion() const
Returns the project version.

#### void setBaseDir(const QString &baseDir)
Sets the base directory used for resolving relative template and output paths.

#### QString baseDir() const
#### QString baseDir() const
Returns the base project directory.

#### void saveWindowStateToSettings()
Persists dialog geometry and column widths to `~/.config/rmap/rmap.conf`.

#### void restoreWindowStateFromSettings()
Restores dialog geometry and column layout from settings.

### 6. Protected Virtual Methods / Event Handlers

#### void showEvent(QShowEvent *event) [override]
`QDialog` override. Auto-adjusts dialog dimensions on first display so all action buttons, table viewports, and input fields are displayed completely without crushed controls or clipping.

#### void closeEvent(QCloseEvent *event) [override]
`QDialog` override. Saves dialog geometry before closing.

#### void resizeEvent(QResizeEvent *event) [override]
`QWidget` override. Captures size changes.

#### void moveEvent(QMoveEvent *event) [override]
`QWidget` override. Captures window movement.

### 7. Ownership and Lifecycle

`RegConfigWindow` is created and owned by `RegMapWindow`. It is designed to be displayed non-modally via `show()` / `raise()` so users can edit the register map while keeping configuration settings open.

### 8. Thread Safety

`RegConfigWindow` is **GUI-thread only**.

### 9. Inter-Class Interactions

- Serialized to Protocol Buffers by `ProtobufHandler` and `RegMapWindow::fileSave()`.
- Provides template mappings directly to `CodeGenerator` during batch or interactive export.

### 10. Usage Example

```cpp
#include "RegConfigWindow.hpp"

void configureProject(QWidget *parent)
{
    auto *configDialog = new RegConfigWindow(parent);
    configDialog->setProjectName("SPI_CONTROLLER");
    configDialog->setProjectVersion("2.1");
    configDialog->setRegisterWidth(32);

    configDialog->addTemplateRow("templates/c/reg_map.h.inja", "work/c/spi_map.h");
    configDialog->addTemplateRow("templates/uvm/reg_model.sv.inja", "work/uvm/spi_uvm.sv");

    configDialog->show();
    configDialog->raise();
}
```
