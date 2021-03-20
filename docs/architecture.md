# Architecture & Internals

**rmap** is built with a modular, decoupled architecture adhering to modern Qt 6 and C++17 best practices.

---

## 1. High-Level Architecture

```
┌────────────────────────────────────────────────────────────────────────┐
│                       Qt 6 GUI Layer                                   │
│                                                                        │
│   RegMapWindow (Main Window)                                           │
│    ├── Search Bar (QLineEdit with live regex & substring matching)     │
│    ├── Undo Stack (QUndoStack: EditCell, InsertItem, DeleteItem)       │
│    ├── Left Panel: QTreeView (TreeFilterProxyModel)                    │
│    │     └─ Displays Blocks (blk), Memories (mem), Maps, & Registers   │
│    └── Right Panel (Stacked Views):                                    │
│          ├── Register View: RegBitfieldBarWidget & Fields Table        │
│          └── Block View: BlockMemoryMapWidget (Stacked Memory Map)     │
└──────────────────────────────┬─────────────────────────────────────────┘
                               │
                               ▼
┌────────────────────────────────────────────────────────────────────────┐
│                 Data Model & Validation Engine                         │
│                                                                        │
│   RegMapTreeModel (QAbstractItemModel)                                 │
│    ├── RegMapTreeItem Hierarchy: Root -> Blk / Mem / Map -> Reg -> Fld │
│    ├── Real-time validation: checks address overlaps & bit bounds      │
│    ├── Linter & Strict Rule Checker (SARIF, JUnit XML, JSON, Text)     │
│    ├── Semantic Diff Engine (AST-level structural comparison)          │
│    └── extractJsonData(): Converts tree to nlohmann::json              │
└──────────────┬───────────────────────────────┬─────────────────────────┘
               │                               │
               ▼ (Multi-Format I/O)            ▼ (Multi-Target Codegen)
┌────────────────────────────────────────┐ ┌─────────────────────────────┐
│       Format Handlers (src/format/)    │ │     Inja Code Generator     │
│                                        │ │                             │
│  - CmsisSvdHandler (ARM CMSIS-SVD)     │ │  CodeGenerator.cpp          │
│  - SystemRdlHandler (SystemRDL 1.0/2.0)│ │  - Custom helpers:          │
│  - IpxactHandler (IEEE 1685-2009/2022) │ │    c_type, msb, camel_case, │
│  - ProtobufHandler (.rmt / .rmb)       │ │    pascal_case, snake_case  │
│  - JsonHandler (Standard JSON schema)  │ │  - Output: Synthesizable SV,│
│  - CsvHandler (RFC 4180 Spreadsheet)   │ │    UVM, C/C++, Rust, Python,│
│                                        │ │    Interactive HTML docs    │
└────────────────────────────────────────┘ └─────────────────────────────┘
```

---

## 2. Core Components

### `RegBitfieldBarWidget` (`src/RegBitfieldBarWidget.*`)
- Custom `QWidget` rendering a vector graphical representation of the selected register's 32/64-bit architecture.
- Calculates and renders occupied field slices and unmapped/reserved slices in **40% dark gray** (`#666666`) with 45° diagonal micro-stripes and a bold `RSVD` badge.
- Provides accessible Color-Blind mode (`Ctrl+Alt+C`) with Okabe-Ito barrier-free palettes and textual tags.
- Provides interactive hover tooltips with complete bitfield metrics (Name, Bit Range, Width, SW Access, HW Access, Reset, Description).
- Provides bidirectional synchronization with the bitfields table.

### `UndoCommands` (`src/UndoCommands.hpp`)
- Modular `QUndoCommand` implementations supporting granular undo/redo:
  - `EditCellCommand`: Handles scalar property mutations.
  - `InsertItemCommand`: Handles row insertions into the tree hierarchy.
  - `DeleteItemCommand`: Recursively captures complete sub-trees to enable non-destructive restoration.

### `RegMapTreeItem` & `RegMapTreeModel`
- Implements an 11-column hierarchical tree node structure (`root` &rarr; `blk` &rarr; `reg` &rarr; `fld`).
- Columns: `Type`, `Offset/LSB`, `Size/Width`, `Name`, `Access Policy (SW)`, `HW Access`, `Reset Value`, `Is Rand`, `Volatile`, `Has Reset`, `Description`.
- `RegMapTreeModel` subclasses `QAbstractItemModel`, providing Qt Views with observable data, row insertion, deletion, and property updates.
- Performs non-blocking validation tracking in `m_invalidCells`.

### `RegMapDelegate` (`src/RegMapDelegate.*`)
- Custom delegates handle data formatting, visual cues, and 1-click editing:
  - `RegHexDecBinDelegate`: Accepts Hex (`0x`), Decimal, and Binary (`0b`) string inputs and renders error styling when cells are marked invalid.
  - `RegAccessPolicyDelegate`: Single-click cycling and combo box for Software access policies (`RW`, `RO`, `WO`, `W1C`, `W1S`, `W0C`, `RC`, `RS`, `NA`).
  - `RegHwAccessDelegate`: Single-click cycling and combo box for Hardware access policies (`RO`, `RW`, `WO`, `NA`, `W1C`, `W1S`, `W0C`, `RS`, `RC`).
  - `RegBoolDelegate`: Single-click instant toggle for boolean flags (`Is Rand`, `Volatile`, `Has Reset`).

### Multi-Format Architecture (`src/format/`)
- `FormatManager`: Central format registry and dispatcher supporting automatic format detection from file extension and content inspection.
- `IFormatHandler`: Abstract base interface defining standard `read()` and `write()` operations for all register map formats.
  - **`CmsisSvdHandler`**: Full reader and writer for **ARM CMSIS-SVD** (`.svd`) microcontroller specifications.
  - **`SystemRdlHandler`**: Custom lexer and recursive-descent parser for **SystemRDL 1.0 & 2.0** (`.rdl`).
  - **`IpxactHandler`**: Streaming XML parser and serializer for **IP-XACT IEEE 1685-2009, 2014, and 2022** (`.xml`, `.ipxact`).
  - **`JsonHandler`**: Structured JSON schema serialization via `nlohmann/json`.
  - **`CsvHandler`**: RFC 4180 compliant CSV / TSV spreadsheet format for Excel-based register authoring.
  - **`ProtobufHandler`**: Protobuf text format (`.rmt`) and high-performance binary serialization (`.rmb`).

### CodeGenerator
- Wraps the Pantor Inja template engine.
- Manages template file resolution, Inja environment scoping, and destination path creation.
- Registers domain helpers: `upper`, `lower`, `camel_case`, `pascal_case`, `snake_case`, `c_type`, `msb`, `to_hex`, `to_dec`, `bitmask`, `pad_zero`.

### `PathUtils` (`src/PathUtils.*`)
- Centralized path resolution and normalization engine:
  - Expands environment variables (`$VAR`, `${VAR}`, Windows `%VAR%`, and `~`).
  - Resolves relative paths prioritizing active register map file directory over CWD.
  - Converts absolute GUI selections and paths to clean, portable relative paths.

### Protobuf Serialization (`rmap.proto`)
- `protormap::Config`: Stores template paths, output destinations, global register width, project metadata, and custom template key-values.
- `protormap::RegModel` & `protormap::RegItem`: Hierarchical node tree representation supporting both human-readable text (`.rmt`) and high-speed binary (`.rmb`) serialization.

---

## 3. C++ Class Reference Documentation

Detailed C++ API reference documentation for each individual class, delegate, visualizer widget, and format handler is maintained in the [`src/doc/`](../src/doc/index.md) directory:

- [**API Documentation Index**](../src/doc/index.md)
- [Application Entry Point (`rmap.cpp`)](../src/doc/rmap.md)
- [Main Window Controller (`RegMapWindow`)](../src/doc/RegMapWindow.md)
- [Bitfield Slice Visualizer (`RegBitfieldBarWidget`)](../src/doc/RegBitfieldBarWidget.md)
- [Stacked Memory Map (`BlockMemoryMapWidget`)](../src/doc/BlockMemoryMapWidget.md)
- [Tree Model (`RegMapTreeModel`)](../src/doc/RegMapTreeModel.md) & [Node Items (`RegMapTreeItem`)](../src/doc/RegMapTreeItem.md)
- [Item Delegates (`RegMapDelegate`)](../src/doc/RegMapDelegate.md) & [Navigation Tree (`RegMapTreeView`)](../src/doc/RegMapTreeView.md)
- [Project Configuration Dialog (`RegConfigWindow`)](../src/doc/RegConfigWindow.md)
- [Preferences Dialog (`PreferencesWindow`)](../src/doc/PreferencesWindow.md)
- [Code Generator (`CodeGenerator`)](../src/doc/CodeGenerator.md)
- [Format Registry (`FormatManager`)](../src/doc/FormatManager.md)
- [Theme Engine (`ThemeManager`)](../src/doc/ThemeManager.md) & [Settings (`AppSettings`)](../src/doc/AppSettings.md)
- [Path Utilities (`PathUtils`)](../src/doc/PathUtils.md) & [Undo Commands (`UndoCommands`)](../src/doc/UndoCommands.md)
- [Object Graph Serialization (`SerializationContext`)](../src/doc/SerializationContext.md)
