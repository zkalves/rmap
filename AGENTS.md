# AGENTS.md — rmap Development Guide

Welcome to the **rmap** codebase! This document provides AI agents and human contributors with essential context, build instructions, architectural patterns, feature definitions, and development guidelines for working effectively with this repository.

---

## 1. Project Overview

**rmap** is a high-performance GUI & CLI tool built with **C++17** and **Qt 6** for designing hardware register maps, performing real-time architectural validation, and generating register models across the full hardware/software lifecycle (synthesizable SystemVerilog RTL, UVM verification models, C/C++ headers, Rust PACs, Python drivers, and interactive HTML documentation).

### Key Technologies & Libraries
- **Language**: Modern C++ (C++17 standard)
- **GUI Framework**: Qt 6 (`QtCore`, `QtWidgets`, `QtGui`) with `AUTOMOC`, `AUTORCC`, and `AUTOUIC`
- **Serialization / Storage**: Multi-format engine supporting ARM CMSIS-SVD (`.svd`), SystemRDL 1.0/2.0 (`.rdl`), IP-XACT IEEE 1685-2009/2014/2022 (`.xml`, `.ipxact`), JSON, CSV/TSV, and Google Protocol Buffers (Protobuf v3 text `.rmt` and binary `.rmb`)
- **Template Engine**: [Pantor Inja](https://github.com/pantor/inja) (v3.3.0 fetched via CMake `FetchContent`) and embedded [nlohmann/json](https://github.com/nlohmann/json)
- **Build System**: CMake 3.15+ with a top-level `Makefile` wrapper

---

## 2. Build, Run, and Maintenance Commands

### Prerequisites
- C++17 compliant compiler (`g++` or `clang++`)
- CMake 3.15+
- Qt 6 development libraries (`qt6-base-dev`, `libqt6test6` or equivalent)
- Protobuf compiler and runtime (`protobuf-compiler`, `libprotobuf-dev`)

### Common Build Commands

```bash
# Standard build using Makefile wrapper (all targets + tests)
make

# Build only the application binary (fast, skips tests)
make rmap

# Run all automated test suites (cleans work/ output directory first)
make test

# Clean build directory and all generated test output directories
make clean

# Full rebuild
make rebuild

# Run the compiled rmap application (builds rmap target only)
make run
```

### Direct CMake Commands

```bash
# Configure build directory with tests enabled (Release or Debug)
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON

# Build targets and test binaries in parallel
cmake --build build -j$(nproc)

# Clean up output folders before running tests directly
rm -rf work && mkdir -p work

# Run automated tests headlessly via CTest
QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure

# Run a single specific test suite headlessly (e.g. test_RegMapWindow)
rm -rf work && mkdir -p work
QT_QPA_PLATFORM=offscreen ctest --test-dir build -R "^test_RegMapWindow$" --output-on-failure

# Launch the executable
./build/bin/rmap

# Launch rmap directly opening an existing register map file
./build/bin/rmap -f examples/spi.rmt
```

### CLI Arguments & Capabilities
- `-f, --file <file>`: Open a specified register map file (`.svd`, `.rdl`, `.xml`, `.json`, `.csv`, `.rmt`, `.rmb`).
- `-e, --export`: Run in headless batch mode to generate all configured template outputs.
- `-c, --convert <out_file>`: Headlessly convert the loaded register map to another format (e.g. `--convert spi.svd`).
- `-l, --lint`: Run automated linter validation check on the loaded register map.
- `--strict`: Enable strict linting rules (enforce non-empty descriptions, address alignment).
- `--report-format <format>`: Report format for `--lint` (`text`, `json`, `sarif`, `junit`) or `--diff` (`text`, `markdown`).
- `-d, --diff <compare_file>`: Compare loaded register map against another file.
- `-t, --theme, --colour-scheme <scheme>`: Set active colour scheme (`solarized8`, `solarized8_light`, `nord`, `dracula`, `monokai`, `classic`).
- `-o, --out <path>`: Override output destination directory for code generation, or output file path for lint/diff reports.
- `-h, --help`: Display Qt command line help.
- `-v, --version`: Display application version.

### Maintenance Protocol
- Whenever code is updated, new features are introduced, or features are modified/removed:
  1. Update existing backend and frontend tests in `tests/` to reflect the changes.
  2. Ensure all test suites clean up temporary output folders (`work/`) in `initTestCase()` and `cleanupTestCase()` to maintain deterministic test isolation.
  3. Add new automated test cases covering new capabilities, edge cases, and bug fixes.
  4. Verify that `make test` achieves a 100% pass rate.
  5. Update user and technical documentation in `docs/` and `README.md` to reflect the code changes.
  6. Update `AGENTS.md` to reflect any new build options, architecture changes, dependencies, or test instructions before completing your task.

---

## 3. Repository Directory Structure

```
rmap/
├── CMakeLists.txt              # CMake build configuration with FetchContent, rmap_core, rmap, & CTest
├── Makefile                    # Make convenience wrapper (all, run, test, clean, rebuild)
├── .github/workflows/
│   └── ci.yml                  # GitHub Actions CI matrix workflow (build + 11 parallel test jobs)
├── README.md                   # Project overview, badges, features, and quickstart guide
├── docs/                       # Project documentation portal (Pelican)
│   ├── dev/                    # Developer documentation & C++ architecture Markdown references
│   └── user/                   # User guide, getting started, GUI manual, CLI reference
├── proto/                      # Protocol buffer schema (Config, RegItem, RegModel)
│   └── rmap.proto
├── res/                        # Application resources
│   ├── images/                 # Application icons, toolbar action artwork, and branding assets
│   └── resources.qrc           # Qt Resource file (toolbar icons, actions, branding)
├── src/                        # Core application C++ source code
│   ├── ui/                     # Qt Designer UI layouts
│   │   ├── rmap.ui             # Main window UI layout
│   │   ├── config.ui           # Configuration dialog UI layout
│   │   └── preferences.ui      # Preferences dialog UI layout
│   ├── main.cpp                # Application entry point (main) and CLI argument parsing
│   ├── rmap.hpp                # Main application header declarations
│   ├── RegMapWindow.cpp / .hpp # Main window, menu/toolbar actions, dual-pane UI orchestration
│   ├── RegBitfieldBarWidget.*  # Interactive graphical 32/64-bit register slice visualizer widget
│   ├── BlockMemoryMapWidget.*  # Vertical stacked-block memory map diagram widget with gap detection
│   ├── UndoCommands.hpp        # Modular QUndoCommand subclasses (EditCell, InsertItem, DeleteItem)
│   ├── RegMapTreeModel.cpp/.hpp# QAbstractItemModel 11-column tree model with validation, JSON export, lint, diff
│   ├── RegMapTreeItem.cpp/.hpp # Hierarchical tree nodes (root, block, reg, field, mem, map)
│   ├── RegMapTreeView.cpp/.hpp # Custom QTreeView implementation for register hierarchy
│   ├── RegMapDelegate.cpp/.hpp # Custom item delegates (Hex/Dec/Bin, SW Access, HW Access, Bool checkboxes)
│   ├── RegConfigWindow.cpp/.hpp# Project Configuration dialog (multiple template search folders, scanning, enable/disable, outputs, width)
│   ├── PreferencesWindow.cpp/.hpp# Application Preferences dialog (colour scheme, colour-blind mode)
│   ├── CodeGenerator.cpp/.hpp  # Inja template renderer with custom helper callbacks
│   ├── format/                 # Multi-format serializer and deserializer handlers
│   │   ├── IFormatHandler.hpp  # Base abstract format interface
│   │   ├── FormatManager.*     # Format registry and dispatcher with auto-detection
│   │   ├── CmsisSvdHandler.*   # ARM CMSIS-SVD (.svd) XML handler
│   │   ├── SystemRdlHandler.*  # SystemRDL 1.0 & 2.0 lexer, parser, generator
│   │   ├── IpxactHandler.*     # IP-XACT IEEE 1685-2009/2014/2022 streaming XML handler
│   │   ├── JsonHandler.*       # Standardized JSON schema import/export
│   │   ├── CsvHandler.*        # RFC 4180 spreadsheet table handler
│   │   └── ProtobufHandler.*   # Google Protobuf text (.rmt) & binary (.rmb) handler
│   ├── ThemeManager.cpp/.hpp   # Multi-theme engine (Solarized 8, Nord, Dracula, Monokai, Classic)
│   ├── AppSettings.cpp/.hpp    # User home configuration (~/.config/rmap/rmap.conf) manager
│   ├── PathUtils.cpp/.hpp      # Path resolution, relativization, and env var expansion engine
│   ├── ProtobufLogCollector.*  # Protobuf error logging collector
│   ├── Serializable.hpp        # Serializable base interface
│   ├── SerializationContext.*  # Context manager for serializing tree hierarchy
│   └── ObjectFactory.hpp       # Factory template for tree object instantiation
├── templates/                  # Inja code generation templates organized by output type
│   ├── c/                      # C/C++ firmware headers (reg_map.h.inja)
│   ├── html/                   # Interactive HTML documentation (reg_doc.html.inja)
│   ├── ipxact/                 # IP-XACT IEEE 1685 XML models (reg_map.xml.inja)
│   ├── json/                   # JSON schema exports (reg_map.json.inja)
│   ├── markdown/               # Markdown specification tables (reg_doc.md.inja)
│   ├── python/                 # Python bring-up drivers (reg_map.py.inja)
│   ├── rtl/                    # Synthesizable SystemVerilog RTL (reg_map.sv.inja)
│   ├── rust/                   # Rust PAC crates (reg_map.rs.inja)
│   ├── svd/                    # ARM CMSIS-SVD peripheral XML (reg_map.xml.inja)
│   ├── systemrdl/              # SystemRDL 2.0 specifications (reg_map.rdl.inja)
│   └── uvm/                    # UVM register models (reg_model.sv.inja)
├── examples/                   # Sample register maps (.rmt) and reference files
│   ├── spi.rmt                 # Standard SPI peripheral register map
│   ├── comprehensive.rmt       # Full feature coverage (all 9 access policies, booleans, hex/dec/bin, mem)
│   ├── wide_bus_64bit.rmt      # 64-bit architecture register map
│   ├── invalid_overlap.rmt     # Validation test cases (address collision, field collision, width overflow)
│   └── tiny/                   # Minimal example register map
├── tests/                      # Automated test suites (CTest + QtTest)
│   ├── CMakeLists.txt          # Test target declarations and CTest setup
│   ├── backend/                # Backend unit tests
│   │   ├── test_PathUtils.cpp        # Env var expansion ($VAR, %VAR%, ~), relativization, resolution order
│   │   ├── test_RegMapTreeItem.cpp   # Node kinds, hierarchy, serialization context
│   │   ├── test_RegMapTreeModel.cpp  # Model operations, data mutations, overlap/bounds validation
│   │   ├── test_Serialization.cpp    # Protobuf text (.rmt) & binary (.rmb) round-trip verification
│   │   ├── test_CodeGenerator.cpp    # Inja helpers, naming conversions, RTL/UVM/C/Rust/Python/HTML/RDL codegen
│   │   ├── test_Formats.cpp          # SystemRDL, IP-XACT, CMSIS-SVD, JSON, CSV, cross-conversion tests
│   │   └── test_ThemeManager.cpp     # Theme switching, palettes, CVD barrier-free colours, AppSettings
│   └── frontend/               # Frontend GUI tests (offscreen QtTest)
│       ├── test_RegMapWindow.cpp     # Window orchestration, bitfield bar, proxy filtering, add/delete, export, themes
│       ├── test_RegConfigWindow.cpp  # Config dialog, multi-folder list, template scanning, enable toggles, non-modality
│       ├── test_PreferencesWindow.cpp# Preferences dialog, colour schemes, colour-blind mode, state persistence
│       └── test_Delegates.cpp        # Hex/Dec/Bin, SW access, HW access, boolean delegate tests
├── script/                     # Helper developer scripts
│   ├── compile_ui              # PySide2 UIC/RCC compiler script
│   └── edit_ui                 # Qt Designer shortcut launcher
└── work/                       # Default output directory for generated code and tests
```

---

## 4. Architecture & Data Model

### Tree Model 11-Column Architecture
`RegMapTreeModel` manages the register tree with 11 distinct columns:

| Column Index | Header Name | Data Role / Interpretation | Delegate Editor |
| :-: | :--- | :--- | :--- |
| **0** | `Type` | Node Kind (`blk`, `reg`, `fld`, `mem`, `map`) | Fixed Item |
| **1** | `Offset` | Register byte offset or Field bit position (LSB) | `RegHexDecBinDelegate` |
| **2** | `Size` | Bit width for Fields (Register width is global in Config) | `RegHexDecBinDelegate` |
| **3** | `Name` | Identifier string for C/RTL code generation | `RegStrDelegate` |
| **4** | `Access Policy` | Software / Bus-Side register access policy (`RW`, `RO`, `WO`, etc.) | `RegAccessPolicyDelegate` (1-click cycle) |
| **5** | `HW Access` | Hardware Core-Side access mode (`RO`, `RW`, `WO`, `NA`, etc.) | `RegHwAccessDelegate` (1-click cycle) |
| **6** | `Reset Value` | Reset value in hex, decimal, or binary | `RegHexDecBinDelegate` |
| **7** | `Is Rand` | UVM randomization flag (`rand` property) | `RegBoolDelegate` (1-click toggle) |
| **8** | `Volatile` | Hardware volatile property for C/C++ & Rust | `RegBoolDelegate` (1-click toggle) |
| **9** | `Has Reset` | Whether the bitfield has an explicit reset state | `RegBoolDelegate` (1-click toggle) |
| **10**| `Description` | Human-readable documentation string for registers, fields, and blocks | `RegMapDelegate` |

### Complete Software (SW) Access Policy Reference
- **`RW` (Read / Write)**: Normal read/write storage register.
- **`RO` (Read Only)**: Value driven by hardware; software writes are ignored.
- **`WO` (Write Only)**: Software writes update storage or trigger commands; read returns `0`.
- **`W1C` (Write 1 to Clear)**: Writing `1` clears the bit to `0`; writing `0` leaves it unchanged (standard for interrupt status).
- **`W1S` (Write 1 to Set)**: Writing `1` sets the bit to `1`; writing `0` leaves it unchanged.
- **`W0C` (Write 0 to Clear)**: Writing `0` clears the bit to `0`; writing `1` leaves it unchanged.
- **`RC` (Read Clears)**: Reading the register returns value and clears it to `0` (destructive read).
- **`RS` (Read Sets)**: Reading the register returns value and sets it to `1`.
- **`NA` (No Access)**: Field is unmapped from software address space.

### Complete Hardware (HW) Access Policy Reference
- **`RO` (Hardware Read-Only)**: Core logic reads software configuration; cannot modify register.
- **`RW` (Hardware Read-Write)**: Core logic reads and updates storage dynamically.
- **`WO` (Hardware Write-Only)**: Core logic writes internal status/counters; software reads it.
- **`W1C` (Hardware Write 1 to Clear)**: Internal hardware pulse clears the bit.
- **`W1S` (Hardware Write 1 to Set)**: Internal hardware pulse sets the bit (e.g. interrupt trigger).
- **`W0C` (Hardware Write 0 to Clear)**: Active-low hardware pulse clears the bit.
- **`NA` (No Hardware Access)**: Core logic does not interface with the field directly.

---

## 5. UI Features & Visual Synchronization

1. **Interactive Bitfield Slice Visualizer**:
   - Continuous 32/64-bit horizontal slice bar displaying bit boundaries, field names, and access policy badges.
   - **40% Dark Gray Reserved Slots**: Unmapped bit ranges are rendered in dark gray (`#666666`) with subtle 45° diagonal micro-stripes and a bold `RSVD` / `RESERVED` badge.
   - **Access Policy Colour Palette**: Matches the table cell badges identically (`RW` Green `#A5D6A7`, `RO` Blue `#BBDEFB`, `WO` Orange `#FFCC80`, `W1C` Yellow `#FFF59D`).
   - **Colour-Blind Accessible Mode (`Ctrl+Alt+C`)**: Okabe-Ito / Wong high-contrast CVD barrier-free palette with explicit textual bracket tags (`[RW]`, `[RO]`, `[WO]`, `[W1C]`).
   - **Bidirectional Sync**: Clicking a slice selects its row in the fields table; selecting a row in the table highlights its bit slice in the bar.
2. **Fast 1-Click Table Editing**:
   - Single-click on SW Access cycles `RW` &rarr; `RO` &rarr; `WO` &rarr; `W1C` &rarr; `RW`.
   - Single-click on HW Access cycles `RO` &rarr; `RW` &rarr; `WO` &rarr; `NA` &rarr; `RO`.
   - Single-click on booleans toggles `true` &harr; `false`.
3. **Key Bindings Help (`F1`)**:
   - Built-in interactive shortcut cheatsheet accessible via `F1` or **Help &rarr; Key Bindings**.

---

## 6. Code Generation & Inja Templates

The code generation system uses **Pantor Inja** to render output files from JSON data extracted by `RegMapTreeModel::extractJsonData()`.

### Full Template Suite in `templates/`
1. `c/reg_map.h.inja`: C/C++ firmware header (macros, bitmasks, offsets, packed structs).
2. `html/reg_doc.html.inja`: Searchable interactive single-page HTML register map documentation.
3. `ipxact/reg_map.xml.inja`: IP-XACT IEEE 1685-2014/2022 XML register model.
4. `json/reg_map.json.inja`: Formatted JSON register map schema export.
5. `markdown/reg_doc.md.inja`: GitHub-flavored Markdown register map specification tables.
6. `python/reg_map.py.inja`: Standalone Python bring-up driver class (Cocotb, PyUVM, PyFTDI, JTAG).
7. `rtl/reg_map.sv.inja`: Synthesizable generic bus-agnostic SystemVerilog register file.
8. `rust/reg_map.rs.inja`: Type-safe Rust Peripheral Access Crate (PAC).
9. `svd/reg_map.xml.inja`: ARM CMSIS-SVD Cortex-M peripheral description XML.
10. `systemrdl/reg_map.rdl.inja`: Accellera SystemRDL 2.0 register file and addrmap specification.
11. `uvm/reg_model.sv.inja`: Complete UVM SystemVerilog register model.

### Custom Inja Helpers Registered
- `{{ upper(str) }}`: Converts string to uppercase.
- `{{ lower(str) }}`: Converts string to lowercase.
- `{{ camel_case(str) }}`: Converts string to camelCase (e.g. `TX_FIFO_CTRL` &rarr; `txFifoCtrl`).
- `{{ pascal_case(str) }}`: Converts string to PascalCase (e.g. `tx_fifo_ctrl` &rarr; `TxFifoCtrl`).
- `{{ snake_case(str) }}`: Converts string to snake_case (e.g. `TxFifoCtrl` &rarr; `tx_fifo_ctrl`, `SPI_SYS` &rarr; `spi_sys`).
- `{{ c_type(width) }}`: Formats bit width into exact C type (`uint8_t`, `uint16_t`, `uint32_t`, `uint64_t`).
- `{{ msb(width, lsb) }}` / `{{ msb(lsb, width) }}`: Computes MSB index `lsb + width - 1`.
- `{{ to_hex(val, width) }}`: Formats a number as zero-padded hex with `0x` prefix (e.g. `to_hex(15, 4)` &rarr; `0x000F`).
- `{{ to_dec(val) }}`: Formats an unsigned integer as decimal.
- `{{ bitmask(width, lsb) }}`: Computes bitmask in hex `((1 << width) - 1) << lsb`.
### Dynamic Path Variables in Output Destinations
Output paths support meaningful variables for flexible SoC repository organization:
- `{output_folder}` / `{output_dir}`: Configured global default output folder.
- `{category}` / `{cat}`: Template category subfolder (e.g. `rtl`, `c`, `uvm`, `html`).
- `{block_name}` / `{block}`: Lowercase register block or peripheral name (e.g. `spi_core`).
- `{project_name}` / `{project}`: Project name defined in configuration.
- `{file_extension}` / `{ext}`: Output file extension (stripped of `.inja` / `.tmpl`).
- `{template_name}` / `{filename}`: Base template name (e.g. `reg_map`, `reg_doc`).
*(e.g., `{output_folder}/{category}/{block_name}_regs.{file_extension}`).*

### Deterministic CRC32 & Hardware Safety
- `RegMapTreeModel::extractJsonData()` computes deterministic IEEE 802.3 CRC32 checksums (`regmap_crc32`, `regmap_crc32_hex`, `blk.crc32`, `blk.crc32_hex`) available for optional hardware version/fingerprint registers.
- Non-contiguous register gaps are automatically calculated (`pad_words_before`, `pad_bytes_before`), and `c/reg_map.h.inja` emits `uint32_t _reserved_[...]` words for exact memory-mapped struct alignment.
- Synthesizable RTL (`rtl/reg_map.sv.inja`) qualifies `W1C` and `W1S` write updates with `wstrb_i` byte-lane strobes and supports `RC` (Read Clears) next-state logic.
- UVM models (`uvm/reg_model.sv.inja`) register backdoor HDL paths (`add_hdl_path_slice`) for direct simulation access.

---

## 7. Agent Quick-Check Checklist

Before committing any changes:
1. Run `make` to verify compilation succeeds with zero errors (compiler uses `-Wfatal-errors`).
2. Update or add automated tests in `tests/backend/` and `tests/frontend/` to cover all new/modified features and bug fixes.
3. Clean up output directory (`work/`) and run `make test` to verify all automated test suites pass (100% pass rate).
4. Test loading, saving, and cross-converting sample files (e.g. `examples/spi.rmt`, `examples/comprehensive.rmt`).
5. Ensure no temporary files or build artifacts (`build/`, `.obj/`, `work/`, `work/*.sv`, `work/*.h`) are committed to version control.
6. Verify that template modifications match the JSON schema exported by `RegMapTreeModel`.
