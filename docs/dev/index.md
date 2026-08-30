# rmap C++ API and Architecture Reference

Welcome to the **rmap** C++ developer reference documentation. This documentation covers the architecture, class contracts, delegates, widgets, format handlers, and utilities that compose the rmap codebase.

---

## Application & UI Controllers

- [**rmap Application Entry Point**](rmap.md): Application startup sequence, Qt application configuration, headless mode auto-detection, and `QCommandLineParser` option handling.
- [**RegMapWindow**](RegMapWindow.md): Main top-level application window, dual-pane UI orchestration, undo/redo stack management, and headless CLI batch workflows.
- [**RegConfigWindow**](RegConfigWindow.md): Non-modal project configuration dialog managing Inja template mappings, output destinations, register widths, and custom parameters.
- [**PreferencesWindow**](PreferencesWindow.md): Non-modal user preferences dialog managing application themes and barrier-free color-blind accessibility modes.

---

## Data Model & Hierarchy

- [**RegMapTreeModel**](RegMapTreeModel.md): Hierarchical 11-column tree model implementing `QAbstractItemModel` with real-time architectural validation, invalid cell tracking, and JSON data extraction.
- [**RegMapTreeItem**](RegMapTreeItem.md): Core tree node class representing blocks, registers, fields, memories, and maps with Protocol Buffer serialization support.
- [**RegMapTreeView**](RegMapTreeView.md): Specialized `QTreeView` subclass managing the left-pane peripheral hierarchy with custom focus and selection handling.
- [**RegMapDelegate**](RegMapDelegate.md): Collection of custom `QStyledItemDelegate` subclasses providing regex validation, access policy pill badges, 1-click cycling, and boolean toggles.
- [**UndoCommands**](UndoCommands.md): Modular `QUndoCommand` implementations for cell modifications, row insertions, and deep recursive subtree deletions.

---

## Interactive Visualizers

- [**RegBitfieldBarWidget**](RegBitfieldBarWidget.md): Interactive continuous 32/64-bit register slice visualizer with unmapped reserved slot hatching, hover tooltips, and bidirectional table selection.
- [**BlockMemoryMapWidget**](BlockMemoryMapWidget.md): Vertical stacked-block memory map diagram with address gap detection and click-to-navigate cross-probing.

---

## Serialization & Format Engine

- [**FormatManager & Multi-Format Handlers**](FormatManager.md): Format registry and dispatcher supporting ARM CMSIS-SVD, Accellera SystemRDL 1.0/2.0, IP-XACT IEEE 1685, JSON, CSV/TSV, and Protobuf (.rmt/.rmb).
- [**SerializationContext & Persistence**](SerializationContext.md): Object graph serialization framework, abstract `Serializable` interface, template `ObjectFactory`, and `ProtobufLogCollector`.

---

## Code Generation & Utilities

- [**CodeGenerator**](CodeGenerator.md): Pantor Inja template rendering engine featuring 11 custom naming and bitwise helper callbacks for multi-target code and documentation generation.
- [**ThemeManager**](ThemeManager.md): Multi-theme styling engine supporting 6 color schemes (Solarized 8, Nord, Dracula, Monokai, Classic) and Okabe-Ito / Wong CVD barrier-free palettes.
- [**AppSettings**](AppSettings.md): Persistent user configuration manager for application geometry, splitter positions, and preferences (`~/.config/rmap/rmap.conf`).
- [**PathUtils**](PathUtils.md): Cross-platform path utility library for environment variable expansion, path relativization, and multi-tiered fallback path resolution.
