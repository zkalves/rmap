# Architecture & Internals

**rmap** is built with a modular, decoupled architecture adhering to modern Qt 6 and C++17 best practices.

---

## 1. High-Level Architecture

```text
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

### Dynamic & Parameterizable Data Width
- **Arbitrary Data Width Support**: Register and bus widths are not constrained to fixed 32-bit or 64-bit boundaries. Register widths are dynamically configurable per project, block, or register (supporting 8, 16, 32, 64, 128, 256, 512+ bits).
- **HDL Parameterization**: RTL code generators emit a configurable generic parameter (e.g. `DATA_WIDTH`) with automatically computed byte-strobe widths (`[DATA_WIDTH/8-1:0]`) and address decode logic scaled to native word alignments.

### Access Policy Matrix & Behavioral Semantics

#### Comprehensive Software Access Policies (IEEE 1800.2 UVM Standard)
The tool supports the full standard set of UVM access modes defined in IEEE 1800.2 `uvm_reg_field.svh`:

| Policy | Read Effect | Write Effect | Description |
| :--- | :--- | :--- | :--- |
| **`RW`** | Read current value | Write updates stored value | Standard Read/Write. |
| **`RO`** | Read current value | Writes ignored | Read-Only status register. |
| **`WO`** | Returns 0 / undefined | Write updates stored value | Write-Only command register. |
| **`RC`** | Read current value; clears to 0 | Writes ignored | Read-to-Clear. |
| **`RS`** | Read current value; sets to 1 | Writes ignored | Read-to-Set. |
| **`WC`** | Read current value | All bits cleared to 0 | Write-Clear (clears on any write). |
| **`WS`** | Read current value | All bits set to 1 | Write-Set (sets on any write). |
| **`WRC`** | Read current value; clears to 0 | Write updates stored value | Write Read-Clear. |
| **`WRS`** | Read current value; sets to 1 | Write updates stored value | Write Read-Set. |
| **`W1C`** | Read current value | Write '1' clears bit; '0' no effect | Write-1-to-Clear interrupt status. |
| **`W1S`** | Read current value | Write '1' sets bit; '0' no effect | Write-1-to-Set override flag. |
| **`W1T`** | Read current value | Write '1' toggles bit; '0' no effect | Write-1-to-Toggle. |
| **`W0C`** | Read current value | Write '0' clears bit; '1' no effect | Write-0-to-Clear. |
| **`W0S`** | Read current value | Write '0' sets bit; '1' no effect | Write-0-to-Set. |
| **`W0T`** | Read current value | Write '0' toggles bit; '1' no effect | Write-0-to-Toggle. |
| **`W1SRC`** | Read clears to 0 | Write '1' sets bit; '0' no effect | Write-1-Set, Read-Clear. |
| **`W1CRS`** | Read sets to 1 | Write '1' clears bit; '0' no effect | Write-1-Clear, Read-Set. |
| **`W0SRC`** | Read clears to 0 | Write '0' sets bit; '1' no effect | Write-0-Set, Read-Clear. |
| **`W0CRS`** | Read sets to 1 | Write '0' clears bit; '1' no effect | Write-0-Clear, Read-Set. |
| **`W1`** | Read current value | First write after reset updates; subsequent ignored | Write-Once. |
| **`WO1`** | Returns 0 / undefined | First write updates; subsequent ignored | Write-Only Once. |
| **`WOC`** | Returns 0 / undefined | Clears all bits to 0 | Write-Only Clear. |
| **`WOS`** | Returns 0 / undefined | Sets all bits to 1 | Write-Only Set. |
| **`NOACCESS`** | Read prohibited | Writes prohibited | Unmapped / Reserved slice. |

#### Hardware Access Policies (HW)
Defines how internal peripheral hardware logic interfaces with the register storage:
- **`RO`**: Hardware only observes the field output (`hw_<reg>_<fld>_o`).
- **`RW`**: Hardware observes and writes updates via `hw_<reg>_<fld>_i` when `hw_<reg>_<fld>_we_i` is high.
- **`WO`**: Hardware drives updates directly into the register flip-flops.
- **`W1C` / `W1S` / `W0C` / `RC` / `RS`**: Hardware drives event set/clear/toggle strobes.
- **`NA`**: No hardware connection.

#### Hardware vs. Software Arbitration
- **Configurable Precedence**: Synthesis and simulation models support a parameterizable precedence rule (`PARAM_HW_PRECEDENCE`):
  - `HW_PRECEDENCE = 1` (Default): When software and hardware attempt concurrent writes on the same cycle, hardware updates take priority to preserve safety and interrupt timing.
  - `HW_PRECEDENCE = 0`: Software write takes priority over hardware write.

#### Software Access Strobes
- **Signal Definition**: Synthesizable RTL emits both register-level strobes (`sw_<reg>_wr_strobe_o` / `sw_<reg>_rd_strobe_o`) and field-level strobes (`sw_<reg>_<fld>_wr_strobe_o` / `sw_<reg>_<fld>_rd_strobe_o`).
- **Purpose**: A 1-cycle active-high pulse asserted when software successfully executes a write or read access to a specific register or field.
- **Hardware Integration**: Enables internal peripheral logic to react immediately to software transactions without polling (e.g. triggering an SPI transaction start, acknowledging/clearing an interrupt pending flag, resetting a hardware timer, popping/pushing a hardware FIFO, or latching shadow register updates).

### Multiple Address Maps (`uvm_reg_map`)
- **Multi-Map Support**: In enterprise SoCs, peripherals are frequently accessed through multiple bus interfaces (e.g., AXI4-Lite fast path vs. APB4 debug interface) or across different address offsets and privilege regimes (Secure vs. Non-Secure worlds).
- **Architecture**: `rmap` allows assigning registers to multiple distinct `uvm_reg_map` instances within a `uvm_reg_block` (e.g. `apb_map`, `axi_map`), configuring independent base addresses, offsets, and access privileges per map.

### Multi-Format Architecture & Compatibility Matrix (`src/format/`)
- `FormatManager`: Central format registry and dispatcher supporting automatic format detection from file extension and content inspection. When extensions are ambiguous (such as `.xml` shared by ARM CMSIS-SVD and IP-XACT), missing, or unrecognized, `FormatManager` performs non-destructive content inspection to identify the correct handler (e.g., detecting `<device` for CMSIS-SVD, `<ipxact:` or `<spirit:` for IP-XACT, `addrmap` for SystemRDL, JSON schema tokens, and Protobuf binary wire headers).
- `IFormatHandler`: Abstract base interface defining standard `read()` and `write()` operations for all register map formats.
  - **`CmsisSvdHandler`**: Full reader and writer for **ARM CMSIS-SVD** (`.svd`, `.xml`) microcontroller specifications.
  - **`SystemRdlHandler`**: Custom lexer and recursive-descent parser for **SystemRDL 1.0 & 2.0** (`.rdl`, `.systemrdl`).
  - **`IpxactHandler`**: Streaming XML parser and serializer for **IP-XACT IEEE 1685-2009, 2014, and 2022** (`.xml`, `.ipxact`).
  - **`JsonHandler`**: Structured JSON schema serialization via `nlohmann/json` (`.json`).
  - **`CsvHandler`**: RFC 4180 compliant CSV / TSV spreadsheet format for Excel-based register authoring (`.csv`, `.tsv`).
  - **`ProtobufHandler`**: Protobuf text format (`.rmt`) and high-performance binary serialization (`.rmb`).

#### Format Capabilities, Limitations & Implications Matrix

| Format | Native Capabilities | Known Limitations | Technical Implications |
| :--- | :--- | :--- | :--- |
| **ARM CMSIS-SVD** | Microcontroller peripherals, interrupt vectors, bit ranges, reset values. | Single flat address map; no hardware sideband ports; no memory window SRAM signals. | When converting to SVD, multi-map definitions and hardware sideband configs are omitted; generated SVD is strictly debugger/firmware focused. |
| **SystemRDL 2.0** | Full address map hierarchy, composite access policies, user properties, hardware access. | Complex syntax requiring conformant lexer/parser; some toolchains support only SystemRDL 1.0 subsets. | Full roundtrip fidelity preserved. Target toolchains must support SystemRDL 2.0 or downgrade to 1.0. |
| **IP-XACT IEEE 1685** | Multi-vendor standard, multiple `memoryRemap` and `addressBlock` nodes, bus interfaces. | XML schema version drift (2009 vs 2014 vs 2022); schema strictness varies between vendors. | Translators must specify schema target version; custom vendor extensions outside standard tags may require normalization. |
| **Google Protobuf** | 100% attribute fidelity, fast binary serialization, project metadata, template configs. | Proprietary binary/text format not directly ingestible by commercial 3rd-party EDA tools without rmap. | Primary project interchange format for rmap; requires export to SVD/SystemRDL/IP-XACT for external tool ingestion. |
| **JSON Schema** | Machine-readable, extensible, easy web and CI script integration. | Non-standardized industry schema; differs between EDA vendor implementations. | Standardized within rmap ecosystem; custom JSON schemas require mapping to rmap's JSON schema. |
| **CSV / TSV** | Universal spreadsheet tabular authoring in Excel/LibreOffice. | Flat table structure; cannot represent multi-level nested addrmaps or multi-map configurations natively. | Exporting deep hierarchies to CSV flattens names (e.g. `block_reg_field`); re-import requires hierarchical reconstruction. |

### CodeGenerator
- Wraps the Pantor Inja template engine.
- Manages template file resolution, Inja environment scoping, and destination path creation.
- Registers 12 domain helpers: `upper`, `lower`, `camel_case`, `pascal_case`, `snake_case`, `c_type`, `msb`, `to_hex`, `to_dec`, `bitmask`, `pad_zero`, and `sv_hex`.
- **Dynamic Path Variables**: Output destination paths support dynamic interpolation tokens:
  - `{out_dir}` or `{out}`: Evaluates to the target base export directory.
  - `{name}` or `{block_name}` / `{block}`: Evaluates to the register block name (or register map name).
  - `{project_name}` or `{project}`: Evaluates to the project name.
  - `{category}` or `{cat}`: Evaluates to the template sub-category.
  - `{template_name}` or `{filename}`: Evaluates to the template name.
  - `{file_extension}` or `{ext}`: Evaluates to the target output file extension.

### `PathUtils` & Environment Variables (`src/PathUtils.*`)
- Centralized path resolution and normalization engine:
  - Expands environment variables (`$VAR`, `${VAR}`, Windows `%VAR%`, and `~`).
  - Resolves relative paths prioritizing active register map file directory over CWD.
  - Converts absolute GUI selections and paths to clean, portable relative paths.
- **Environment Variables**:
  - `RMAP_CONFIG_FILE`: Overrides default path to the persistent user configuration file (`~/.config/rmap/rmap.conf`).
  - `RMAP_THEMES_PATH` / `RMAP_THEME_DIR`: Defines filesystem search directories for custom color themes.
  - `RMAP_TRANSLATIONS_PATH` / `RMAP_TRANSLATION_DIR`: Defines filesystem search directories for runtime JSON translation catalogs (`rmap_*.json`).
  - `RMAP_TEMPLATES_DIR`: Overrides default search path for Inja templates.
  - `RMAP_EXAMPLES_DIR`: Overrides default search path for bundled examples.
  - `RMAP_DOCS_DIR`: Overrides default path for offline documentation.
  - `RMAP_PYTHON` / `PYTHON`: Custom Python interpreter binary.
  - `RMAP_PYTHON_TIMEOUT`: Configures execution timeout in milliseconds for Python scripts and generators (default: 60000 ms).
  - `RMAP_TMPDIR`: Custom temporary directory for intermediate JSON context files.

### Protobuf Serialization (`rmap.proto`)
- `protormap::Config`: Stores template paths, output destinations, global register width, project metadata, and custom template key-values.
- `protormap::RegModel` & `protormap::RegItem`: Hierarchical node tree representation supporting both human-readable text (`.rmt`) and high-speed binary (`.rmb`) serialization.

---

## 3. C++ Class Reference Documentation

Detailed C++ API reference documentation for each individual class, delegate, visualizer widget, and format handler is generated automatically by Doxygen from source headers:

- [**C++ Subsystem Architecture Guide**](../dev/index.md)
- [**Complete C++ Class List**](annotated.html) & [**Class Hierarchy**](hierarchy.html)
- [**Source Code File Directory**](files.html)
- [Application Entry Point (`main.cpp`)](main_8cpp.html)
- [Main Window Controller (`RegMapWindow`)](classRegMapWindow.html)
- [About Dialog (`AboutWindow`)](classAboutWindow.html)
- [Bitfield Slice Visualizer (`RegBitfieldBarWidget`)](classRegBitfieldBarWidget.html)
- [Stacked Memory Map (`BlockMemoryMapWidget`)](classBlockMemoryMapWidget.html)
- [Tree Model (`RegMapTreeModel`)](classRegMapTreeModel.html) & [Node Items (`RegMapTreeItem`)](classRegMapTreeItem.html)
- [Item Delegates (`RegMapDelegate`)](classRegMapDelegate.html) & [Navigation Tree (`RegMapTreeView`)](classRegMapTreeView.html)
- [Project Configuration Dialog (`RegConfigWindow`)](classRegConfigWindow.html)
- [Preferences Dialog (`PreferencesWindow`)](classPreferencesWindow.html)
- [Code Generator (`CodeGenerator`)](classCodeGenerator.html)
- [Format Registry (`FormatManager`)](classFormatManager.html) & [Format Base Interface (`IFormatHandler`)](classIFormatHandler.html)
- [Format Handlers: ARM CMSIS-SVD (`CmsisSvdHandler`)](classCmsisSvdHandler.html), [SystemRDL (`SystemRdlHandler`)](classSystemRdlHandler.html), [IP-XACT (`IpxactHandler`)](classIpxactHandler.html), [JSON (`JsonHandler`)](classJsonHandler.html), [CSV (`CsvHandler`)](classCsvHandler.html), [Protobuf (`ProtobufHandler`)](classProtobufHandler.html)
- [Theme Engine (`ThemeManager`)](classThemeManager.html) & [Settings (`AppSettings`)](classAppSettings.html)
- [Internationalization Manager (`LanguageManager`)](classLanguageManager.html) & [JSON Translator (`JsonTranslator`)](classJsonTranslator.html)
- [Path Utilities (`PathUtils`)](namespacePathUtils.html) & [Undo Commands (`UndoCommands`)](UndoCommands_8hpp.html)
- [Object Graph Serialization (`SerializationContext`)](classSerializationContext.html), [Object Factory (`ObjectFactory`)](classObjectFactory.html), [Serializable Interface (`Serializable`)](classSerializable.html) & [Protobuf Log Collector (`ProtobufLogCollector`)](classProtobufLogCollector.html)


