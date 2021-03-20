# RegMapWindow

### 1. Class Overview

`RegMapWindow` is the primary top-level window and central controller of the **rmap** application. It orchestrates the dual-pane graphical user interface, manages document lifecycle (opening, saving, exporting, converting, and auto-detecting file formats), manages the undo/redo stack (`QUndoStack`), synchronizes selection and cross-probing between tree views, tables, and graphical visualizers (`RegBitfieldBarWidget` and `BlockMemoryMapWidget`), and executes headless batch CLI operations (code generation, architectural linting, and semantic register map diffing).

### 2. Project Structure and Dependencies

`RegMapWindow` is defined in `src/RegMapWindow.hpp` and implemented in `src/RegMapWindow.cpp`. It interacts with almost every major subsystem in the project:
- `RegMapTreeModel` — Core 11-column data model for registers, fields, and memory regions.
- `RegBitfieldBarWidget` — Interactive horizontal 32/64-bit slice bar visualizer.
- `BlockMemoryMapWidget` — Vertical stacked-block peripheral memory map diagram.
- `RegConfigWindow` — Non-modal project code generation and architecture configuration dialog.
- `PreferencesWindow` — Application preferences, theme selection, and color-blind mode settings dialog.
- `ThemeManager` & `AppSettings` — Global palette switching, stylesheet configuration, and settings persistence.
- `FormatManager` — Multi-format registry supporting SystemRDL, IP-XACT, CMSIS-SVD, JSON, CSV, and Protobuf.
- `CodeGenerator` — Inja template code rendering engine.

Build Requirements:
- Qt 6 modules: `QtWidgets`, `QtCore`, `QtGui`
- Protocol Buffers runtime library (`libprotobuf`)
- UI template form `ui/rmap.ui` processed via `uic`

### 3. Class Hierarchy and Role

`RegMapWindow` inherits directly from:
- `QMainWindow` (Qt Widgets) — Provides main window layout, menu bar, status bar, and central widget hosting.
- `Ui::rmap` (private multiple inheritance) — Provides direct compile-time access to widgets declared in `ui/rmap.ui`.

### 4. Public Methods

#### explicit RegMapWindow(const QString &rmap_filename = QString(), QWidget *parent = nullptr)
Constructs the main application window. Loads and initializes embedded Qt resources, instantiates the configuration and preferences dialogs, creates the undo stack, configures the dual-pane splitter, and automatically loads `rmap_filename` if provided.

#### ~RegMapWindow() override
Destructor. Cleans up internal models, proxy filters, and child dialogs.

#### bool headlessExport(const QString &out_dir)
Executes a headless batch code generation run. Loads configured template mappings and generates all target code files into `out_dir` (or the configured default output directory). Returns `true` if all templates rendered successfully.

#### bool headlessLint(bool strict, const QString &format, const QString &outFile)
Runs headless architectural validation checks on the loaded register map. If `strict` is enabled, warnings (such as missing descriptions or address alignment violations) are treated as errors. Formats the report as `text`, `json`, `sarif`, or `junit` and outputs to stdout or `outFile`. Returns `true` if validation passed with zero errors.

#### static bool semanticDiff(const QString &file1, const QString &file2, const QString &format, const QString &outFile)
Compares two register map files semantically, checking for added, removed, or modified registers, bit offsets, access policies, and fields. Generates a comparison report in `text` or `markdown` format. Returns `true` if the diff execution completed successfully.

#### void fileOpen(QString fname)
Loads a register map from disk into the model, auto-detecting the format via `FormatManager`. Updates the window title and restores selection.

#### bool fileSave(QString fname = "")
Saves the active register map to disk. If `fname` is empty, saves to the current file path. Returns `true` on success.

#### RegMapTreeModel* getModel() const
#### RegMapTreeModel* model() const
Returns a pointer to the underlying `RegMapTreeModel`.

#### QUndoStack* getUndoStack() const
#### QUndoStack* undoStack() const
Returns a pointer to the window's `QUndoStack`.

#### RegConfigWindow* configWindow() const
Returns a pointer to the project configuration dialog instance.

#### PreferencesWindow* preferencesWindow() const
Returns a pointer to the preferences dialog instance.

#### RegBitfieldBarWidget* bitfieldWidget() const
Returns a pointer to the interactive register bitfield slice bar widget.

#### BlockMemoryMapWidget* memoryMapWidget() const
Returns a pointer to the vertical block memory map widget.

#### void setColourBlindMode(bool enabled)
#### void setColorBlindMode(bool enabled)
Enables or disables Okabe-Ito / Wong CVD barrier-free color-blind palette across the table delegates, bitfield visualizer, and memory map diagrams.

#### bool isColourBlindMode() const
#### bool isColorBlindMode() const
Returns `true` if color-blind mode is currently active.

#### void setColourScheme(const QString &scheme)
#### void setColorScheme(const QString &scheme)
Applies one of the supported application color themes (`solarized8`, `solarized8_light`, `nord`, `dracula`, `monokai`, `classic`).

#### QString colourScheme() const
#### QString colourScheme() const
#### QString getColorScheme() const
#### QString colorScheme() const
Returns the identifier of the active color scheme.

#### void saveWindowStateToSettings()
Persists window geometry, splitter sizes, and active settings to `~/.config/rmap/rmap.conf`.

#### void restoreWindowStateFromSettings()
Restores window geometry and splitter positions from `~/.config/rmap/rmap.conf`.

### 5. Protected Virtual Methods / Event Handlers

#### void closeEvent(QCloseEvent *event) [override]
`QMainWindow` override. Intercepts window close requests, checks if the active register map has unsaved modifications, and prompts the user to save changes before exiting.

#### void resizeEvent(QResizeEvent *event) [override]
`QWidget` override. Captures window resize events and updates persistent geometry.

#### void moveEvent(QMoveEvent *event) [override]
`QWidget` override. Captures window movement and updates persistent coordinates.

### 6. Ownership and Lifecycle

`RegMapWindow` is typically created as a top-level window on the stack or heap in `main.cpp`. It owns:
- `m_model` (`RegMapTreeModel*`) — Parented to the window.
- `m_config_window` (`RegConfigWindow*`) — Parented to the window, displayed as a non-modal dialog.
- `m_pref_window` (`PreferencesWindow*`) — Parented to the window, displayed as a non-modal dialog.
- `m_undoStack` (`QUndoStack*`) — Parented to the window.
- `m_bitfieldBar` and `m_blockMemoryMapWidget` — Parented through the Qt widget layout hierarchy.

### 7. Thread Safety

`RegMapWindow` is **GUI-thread only**. All widget manipulations, model updates, and UI events must be executed on the main application thread.

### 8. Inter-Class Interactions

- Connects `TreeFilterProxyModel` and `FieldSortProxyModel` to `RegMapTreeView` and `QTableView`.
- Routes selection changes in the left tree to update the right stacked pane (`updateFieldsTable` for registers, `updateBlockView` for blocks).
- Synchronizes selection between the fields table and `RegBitfieldBarWidget`.
- Connects `BlockMemoryMapWidget::registerClicked` to navigate the left tree directly to selected registers.
- Connects `QUndoStack::canUndoChanged` and `canRedoChanged` to toolbar actions.

### 9. External Communication

- Reads and writes register map files across supported formats (`.svd`, `.rdl`, `.xml`, `.json`, `.csv`, `.rmt`, `.rmb`) via `FormatManager`.
- Exports rendered source files (`.sv`, `.h`, `.rs`, `.py`, `.html`, etc.) to the local filesystem via `CodeGenerator`.
- Persists user preferences and geometry to `~/.config/rmap/rmap.conf` via `AppSettings`.

### 10. Usage Example

```cpp
#include <QApplication>
#include "RegMapWindow.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Instantiate main window opening an existing register map
    RegMapWindow window("example/spi.rmt");
    window.setColourScheme("nord");
    window.show();

    return app.exec();
}
```
