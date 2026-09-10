# rmap — Hardware Register Map Designer & Model Generator

[![CI](https://github.com/zkalves/rmap/actions/workflows/ci.yml/badge.svg)](https://github.com/zkalves/rmap/actions/workflows/ci.yml)
[![License: MPL 2.0](https://img.shields.io/badge/License-MPL_2.0-blue.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Qt 6](https://img.shields.io/badge/Qt-6-green.svg)](https://www.qt.io/)
<!-- COVERAGE_BADGES_START -->
[![Line Coverage](https://img.shields.io/badge/Line_Coverage-100.0%25-brightgreen.svg)](#code-coverage-metrics)
[![Function Coverage](https://img.shields.io/badge/Function_Coverage-100.0%25-brightgreen.svg)](#code-coverage-metrics)
[![Branch Coverage](https://img.shields.io/badge/Branch_Coverage-87.7%25-brightgreen.svg)](#code-coverage-metrics)
[![Condition Coverage](https://img.shields.io/badge/Condition_Coverage-82.5%25-brightgreen.svg)](#code-coverage-metrics)
<!-- COVERAGE_BADGES_END -->

**rmap** is a high-performance GUI & CLI tool for designing hardware register maps, validating register architectures, and generating hardware, verification, firmware, and documentation models.

---

## Key Features

- **Interactive Graphical Bitfield Visualizer**: 32/64-bit segmented slice bar widget displaying bit indexes, colour-coded access policy badges (`RW`, `RO`, `WO`, `W1C`, `W1S`, etc.), reserved gaps, hover tooltips, and bidirectional table selection synchronization.
- **Universal Multi-Format Support**: Read, edit, and write **ARM CMSIS-SVD** (`.svd`), **SystemRDL 1.0 & 2.0** (`.rdl`), **IP-XACT IEEE 1685-2009/2014/2022** (`.xml`, `.ipxact`), **JSON**, **CSV / TSV**, and **Google Protobuf** (`.rmt`, `.rmb`).
- **Cross-Format Conversion**: Load in any format and instantly export to any other format in GUI or via CLI (`rmap -f chip.rdl --convert chip.svd`).
- **Full Undo / Redo System**: Complete multi-level undo/redo (`Ctrl+Z` / `Ctrl+Y`) covering cell edits, additions, and deletions.
- **Real-Time Search & Live Filtering**: Filter through block names, register names, hex addresses, access policies, and descriptions with instant visual feedback.
- **Multi-Target Code & Model Generation**:
  - **Synthesizable RTL**: Bus-agnostic generic SystemVerilog slave register file (`rtl/reg_map.sv.inja`) with address decode, byte enables, and HW/SW strobes.
  - **Verification Models**: Complete UVM SystemVerilog Register Model (`uvm/reg_model.sv.inja`) with backdoor HDL paths and coverage hooks.
  - **Standalone Self-Checking RTL Testbench**: Out-of-the-box SystemVerilog testbench (`rtl_tb/tb_reg_map.sv.inja`) verifying resets, access policies, byte strobes, and hardware sidebands in open-source Icarus Verilog or Verilator.
  - **Open-Source Python UVM Testbench**: Full UVM verification environment in Python (`pyuvm_tb/tb_pyuvm.py.inja`) powered by `pyuvm` and Cocotb.
  - **IEEE 1800.2 (`uvm-ieee`) Verification Suite**: Complete UVM testbench (`uvm_tb/`) including bus VIP agent, driver, monitor, adapter, predictor, environment, and tests for Synopsys VCS, Cadence Xcelium, and Siemens Questa.
  - **Simulation Runner Makefile**: Multi-tool execution Makefile (`sim/Makefile.inja`) for one-command simulation across all simulators.
  - **Firmware & Embedded Headers**: C/C++ packed structs and bit manipulation macros (`c/reg_map.h.inja`).
  - **Rust PAC**: Type-safe Rust Peripheral Access Crate (`rust/reg_map.rs.inja`).
  - **Python Lab Bring-Up Drivers**: Standalone Python register driver class compatible with Cocotb, PyUVM, PyFTDI, and JTAG (`python/reg_map.py.inja`).
  - **Interactive HTML Documentation**: Searchable, responsive HTML register documentation specification (`html/reg_doc.html.inja`).
  - **SystemRDL 2.0 Specification**: Standard SystemRDL 2.0 register file and addrmap model (`systemrdl/reg_map.rdl.inja`).
  - **IP-XACT IEEE 1685**: Component XML register model specification (`ipxact/reg_map.xml.inja`).
  - **ARM CMSIS-SVD**: Cortex-M peripheral description XML for debuggers (`svd/reg_map.xml.inja`).
  - **Markdown Documentation**: Table-driven GitHub-flavored Markdown specification (`markdown/reg_doc.md.inja`).
  - **JSON Schema**: Formatted machine-readable JSON schema export (`json/reg_map.json.inja`).
- **Post-Generation Python Script Execution**: Automatically launch a custom Python script upon code generation with the complete register map context and helper functions injected directly as globals (`name`, `blocks`, `reg_width`, etc.), virtual module (`import rmap`), CLI argument (`sys.argv[1]`), standard input, and environment variables.
- **Automated CI/CD Linter**: Headless validation engine (`--lint`, `--strict`) with machine-readable reports in **SARIF** (GitHub PR code scanning), **JUnit XML** (CI test dashboards), **JSON**, or human-readable **Text**.
- **Semantic Register Map Diff Engine**: Headless structural diffing (`--diff`) comparing registers, addresses, bitfields, and access policies across versions with Text and Markdown reports.
- **Curated Multi-Theme Engine & Configurable Colour Schemes**: Built-in dark, light, and high-contrast colour schemes (**Solarized 8 (Dark)** default, **Solarized 8 (Light)**, **Nord**, **Dracula**, **Monokai**, **Classic Light**, **High Contrast (Dark)**, **High Contrast (Light)**), easily customized and extended via declarative JSON files (`themes/*.json`, `~/.config/rmap/themes/`). Custom themes can be added or edited on the fly with a 1-click **Themes Folder...** shortcut in Preferences. User GUI settings persist in `~/.config/rmap/rmap.conf`.
- **Comprehensive Colour-Blind Modes & Accessibility (WCAG AAA)**: Full support for all 5 major Colour Vision Deficiency (CVD) conditions with scientifically tailored access policy palettes: **Universal (Barrier-Free Okabe-Ito)**, **Deuteranopia** (green-blind/weak), **Protanopia** (red-blind/weak), **Tritanopia** (blue-blind/weak), and **Achromatopsia** (monochromatic luminance steps). Features explicit bracketed access badges (`[RW]`, `[RO]`, `[WO]`, `[W1C]`), quick toggle (`Ctrl+Alt+C`), **View &rarr; Colour-Blind Profile** submenu, and WCAG AAA compliant High Contrast themes.
- **Multi-Language Internationalization & Localization**: Multi-language engine (**English**, **Spanish / Español**, **German / Deutsch**, **French / Français**, **Chinese / 简体中文**, **Japanese / 日本語**, **Portuguese / Português**) with instant live UI and table retranslation. Features a **View &rarr; Language** menu, Preferences dropdown, and `--lang` CLI flag. All languages are configured at build time for determinism and security, with new languages easily added via JSON templates.
- **Relative Paths & Environment Variable Expansion**: Full support for relative paths, absolute paths, and environment variable expansion (`$VAR`, `${VAR}`, Windows `%VAR%`, and `~`) across all CLI options, configuration settings, and template mappings. All saved repository files store clean, portable relative paths.
- **Conflict-Free Keyboard Shortcuts**: Standardized OS-friendly shortcuts (`Ctrl+Shift+M` for Mem, `Ctrl+Shift+S` for Save As, `Ctrl+Shift+F` for Field, `Ctrl+Shift+B` for Block, `Ctrl+Shift+R` for Register, `Ctrl+Z` for Undo, `Ctrl+Y` for Redo, `Ctrl+P` for Configuration, `Ctrl+,` for Preferences, `Ctrl+Alt+C` for Colour-Blind Mode).

---

<!-- COVERAGE_SECTION_START -->
## Code Coverage Metrics

Automated test coverage analysis across all 6 compiler-supported metrics:

| Metric | Covered | Total | Coverage Rate | Status |
| :--- | :---: | :---: | :---: | :---: |
| **Lines** | 7,654 | 7,654 | **100.00%** | ✅ |
| **Functions** | 557 | 557 | **100.00%** | ✅ |
| **Branches (Decision)** | 12,720 | 14,500 | **87.72%** | ✅ |
| **Conditions (MC/DC)** | 5,246 | 6,356 | **82.54%** | ✅ |
| **Calls** | 16,436 | 20,708 | **79.37%** | ✅ |
| **Basic Blocks** | 24,232 | 37,691 | **64.29%** | ✅ |
| *Branches (Raw w/ Unwind)* | 12,728 | 21,815 | *58.35%* | ℹ️ |

> [!NOTE]
> **Branch Coverage Measurement**: In accordance with DO-178C, ISO 26262, and `gcovr` standards, decision branch coverage tracks actual logical control branches (`if`, `switch`, `while`, ternary). Compiler-synthesized exception unwinding landing pads (`throw: true`) are excluded from decision branches and shown transparently in raw metrics.

### Architectural Subsystems Breakdown

| Subsystem | Lines (%) | Functions (%) | Branches (%) | Conditions (%) | Calls (%) | Blocks (%) |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| **Code Generation Engine** | 100.0% (462/462) | 100.0% (20/20) | 91.5% (1004/1097) | 88.0% (403/458) | 80.0% | 63.5% |
| **Core Architecture & Model** | 100.0% (701/701) | 100.0% (77/77) | 91.9% (1125/1224) | 86.0% (447/520) | 78.4% | 61.7% |
| **Dialogs & Configuration** | 100.0% (814/814) | 100.0% (91/91) | 88.9% (1029/1157) | 86.4% (380/440) | 81.3% | 67.6% |
| **Format Parsers & Serializers** | 100.0% (1377/1377) | 100.0% (61/61) | 88.7% (3105/3500) | 84.4% (1386/1642) | 79.2% | 64.8% |
| **GUI Widgets & Main Window** | 100.0% (2556/2556) | 100.0% (180/180) | 84.8% (3894/4591) | 76.2% (1456/1912) | 79.0% | 64.1% |
| **System Services & Utilities** | 100.0% (1744/1744) | 100.0% (128/128) | 87.4% (2563/2931) | 84.8% (1174/1384) | 79.5% | 64.2% |

<details>
<summary><b>Detailed Source Files Coverage (Click to expand)</b></summary>

| Source File | Subsystem | Lines (%) | Functions (%) | Branches (%) | Conditions (%) | Calls (%) | Blocks (%) |
| :--- | :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| `src/AboutWindow.cpp` | Dialogs & Configuration | 100.0% | 100.0% | 96.1% | 85.7% | 80.5% | 69.5% |
| `src/AboutWindow.hpp` | Dialogs & Configuration | 100.0% | 100.0% | 0.0% | 0.0% | 100.0% | 100.0% |
| `src/AppSettings.cpp` | System Services & Utilities | 100.0% | 100.0% | 94.1% | 89.0% | 81.1% | 69.1% |
| `src/BlockMemoryMapWidget.cpp` | GUI Widgets & Main Window | 100.0% | 100.0% | 93.8% | 88.0% | 80.8% | 66.2% |
| `src/BlockMemoryMapWidget.hpp` | GUI Widgets & Main Window | 100.0% | 100.0% | 0.0% | 0.0% | 100.0% | 100.0% |
| `src/CodeGenerator.cpp` | Code Generation Engine | 100.0% | 100.0% | 91.5% | 88.0% | 80.0% | 63.5% |
| `src/CodeGenerator.hpp` | Code Generation Engine | 100.0% | 100.0% | 0.0% | 0.0% | 0.0% | 100.0% |
| `src/LanguageManager.cpp` | System Services & Utilities | 100.0% | 100.0% | 86.0% | 88.1% | 82.7% | 70.4% |
| `src/ObjectFactory.hpp` | Core Architecture & Model | 100.0% | 100.0% | 40.0% | 50.0% | 50.0% | 70.0% |
| `src/PathUtils.cpp` | System Services & Utilities | 100.0% | 100.0% | 87.3% | 81.8% | 76.5% | 63.4% |
| `src/PathUtils.hpp` | System Services & Utilities | 100.0% | 100.0% | 100.0% | 0.0% | 100.0% | 100.0% |
| `src/PreferencesWindow.cpp` | Dialogs & Configuration | 100.0% | 100.0% | 94.1% | 83.3% | 85.6% | 72.6% |
| `src/PreferencesWindow.hpp` | Dialogs & Configuration | 100.0% | 100.0% | 0.0% | 0.0% | 100.0% | 100.0% |
| `src/ProtobufLogCollector.cpp` | System Services & Utilities | 100.0% | 100.0% | 100.0% | 0.0% | 68.0% | 45.2% |
| `src/ProtobufLogCollector.hpp` | System Services & Utilities | 100.0% | 100.0% | 0.0% | 0.0% | 45.5% | 47.1% |
| `src/RegBitfieldBarWidget.cpp` | GUI Widgets & Main Window | 100.0% | 100.0% | 90.6% | 85.3% | 80.9% | 68.3% |
| `src/RegBitfieldBarWidget.hpp` | GUI Widgets & Main Window | 100.0% | 100.0% | 0.0% | 0.0% | 100.0% | 100.0% |
| `src/RegConfigWindow.cpp` | Dialogs & Configuration | 100.0% | 100.0% | 87.6% | 86.9% | 80.2% | 66.2% |
| `src/RegConfigWindow.hpp` | Dialogs & Configuration | 100.0% | 100.0% | 0.0% | 0.0% | 100.0% | 100.0% |
| `src/RegMapDelegate.cpp` | GUI Widgets & Main Window | 100.0% | 100.0% | 86.5% | 82.3% | 79.0% | 66.6% |
| `src/RegMapTreeItem.cpp` | Core Architecture & Model | 100.0% | 100.0% | 91.0% | 93.3% | 82.0% | 75.3% |
| `src/RegMapTreeModel.cpp` | Core Architecture & Model | 100.0% | 100.0% | 92.1% | 87.0% | 78.4% | 59.1% |
| `src/RegMapTreeModel.hpp` | Core Architecture & Model | 100.0% | 100.0% | 0.0% | 0.0% | 100.0% | 25.0% |
| `src/RegMapTreeView.cpp` | Core Architecture & Model | 100.0% | 100.0% | 100.0% | 0.0% | 100.0% | 100.0% |
| `src/RegMapWindow.cpp` | GUI Widgets & Main Window | 100.0% | 100.0% | 83.1% | 73.0% | 78.6% | 63.1% |
| `src/RegMapWindow.hpp` | GUI Widgets & Main Window | 100.0% | 100.0% | 100.0% | 0.0% | 75.0% | 100.0% |
| `src/Serializable.hpp` | Core Architecture & Model | 100.0% | 100.0% | 0.0% | 0.0% | 0.0% | 25.0% |
| `src/SerializationContext.hpp` | Core Architecture & Model | 100.0% | 100.0% | 93.3% | 66.7% | 61.5% | 64.0% |
| `src/ThemeManager.cpp` | System Services & Utilities | 100.0% | 100.0% | 87.5% | 86.9% | 81.0% | 64.3% |
| `src/UndoCommands.cpp` | Core Architecture & Model | 100.0% | 100.0% | 93.2% | 78.6% | 79.1% | 64.6% |
| `src/format/CmsisSvdHandler.cpp` | Format Parsers & Serializers | 100.0% | 100.0% | 90.6% | 85.0% | 76.9% | 62.8% |
| `src/format/CmsisSvdHandler.hpp` | Format Parsers & Serializers | 100.0% | 100.0% | 0.0% | 0.0% | 100.0% | 100.0% |
| `src/format/CsvHandler.cpp` | Format Parsers & Serializers | 100.0% | 100.0% | 85.9% | 78.7% | 76.1% | 60.8% |
| `src/format/FormatManager.cpp` | Format Parsers & Serializers | 100.0% | 100.0% | 91.7% | 79.4% | 76.0% | 65.4% |
| `src/format/IpxactHandler.cpp` | Format Parsers & Serializers | 100.0% | 100.0% | 92.0% | 87.1% | 77.4% | 61.9% |
| `src/format/JsonHandler.cpp` | Format Parsers & Serializers | 100.0% | 100.0% | 85.4% | 86.5% | 78.8% | 60.2% |
| `src/format/ProtobufHandler.cpp` | Format Parsers & Serializers | 100.0% | 100.0% | 90.7% | 85.0% | 79.4% | 64.8% |
| `src/format/SystemRdlHandler.cpp` | Format Parsers & Serializers | 100.0% | 100.0% | 88.5% | 84.6% | 84.6% | 73.6% |
| `src/main.cpp` | System Services & Utilities | 100.0% | 100.0% | 80.8% | 66.7% | 72.2% | 51.3% |

</details>

### Running Coverage Locally

```bash
# Run unit tests with coverage instrumentation and output summary report
make coverage

# Generate HTML and Markdown coverage reports
make coverage-report
```

<!-- COVERAGE_SECTION_END -->

---

## Quickstart & Installation

### Prerequisites

- **C++17 Compiler** (`g++` 9+ or `clang++` 10+)
- **CMake 3.15+**
- **Qt 6** (`qt6-base-dev`, `libqt6test6`)
- **Protocol Buffers** (`libprotobuf-dev`, `protobuf-compiler`)

#### Ubuntu / Debian
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake git \
  qt6-base-dev libqt6test6 \
  libprotobuf-dev protobuf-compiler
```

### Building

```bash
# Clone the repository
git clone https://github.com/zkalves/rmap.git
cd rmap

# Build with Make wrapper
make

# Run automated unit test suites (100% pass rate)
make test

# Run comprehensive template verification tests across all 11 output formats
make test-templates
```

### Installation

Install the `rmap` executable, code generation templates (`<prefix>/share/rmap/templates`), sample models (`<prefix>/share/rmap/examples`), and documentation (`<prefix>/share/doc/rmap`) to your preferred directory using either Make or CMake. The application automatically discovers installed templates without requiring manual configuration:

#### With Make
```bash
# Default system-wide installation (/usr/local/bin/rmap and /usr/local/share/rmap)
sudo make install

# Custom install prefix (e.g. user home directory)
make install PREFIX=$HOME/.local

# Custom staging directory for packagers
make install DESTDIR=/tmp/staging PREFIX=/usr

# Uninstall
sudo make uninstall
# Or for a custom prefix:
make uninstall PREFIX=$HOME/.local
```

#### With CMake
```bash
# Configure installation prefix during setup
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$HOME/.local
cmake --build build -j$(nproc)
cmake --install build

# Or override installation prefix at install time
cmake --install build --prefix $HOME/.local
```

---

## Usage

### 1. GUI Mode

Launch the application to design or inspect register maps interactively:

```bash
# Launch empty window
./build/bin/rmap

# Open an existing register map
./build/bin/rmap -f examples/rmt/peripherals/spi.rmt
```

### 2. Headless CLI Mode & CI/CD Automation

Run code generation, linting, or diffing directly in Makefiles, CI/CD pipelines, or Git pre-commit hooks:

```bash
# 1. Headless Batch Code Generation
./build/bin/rmap -f examples/rmt/peripherals/spi.rmt --export --out ./work

# 2. Format Conversion (e.g. Protobuf -> ARM CMSIS-SVD)
./build/bin/rmap -f examples/rmt/peripherals/spi.rmt --convert work/spi.svd

# 3. CI/CD Linter (SARIF for GitHub PR annotations or JUnit XML for CI test reports)
./build/bin/rmap -f examples/rmt/peripherals/spi.rmt --lint --strict --report-format sarif --out work/lint.sarif
./build/bin/rmap -f examples/rmt/peripherals/spi.rmt --lint --strict --report-format junit --out work/junit.xml

# 4. Semantic Diff Between Revisions
./build/bin/rmap -f base.rmt --diff updated.rmt --report-format markdown --out diff.md
```

---

## Semantic Versioning & Releases

`rmap` adheres to [Semantic Versioning 2.0.0](https://semver.org/) (`MAJOR.MINOR.PATCH`). Releases and version bumps are managed **completely automatically**:

- **Conventional Commits**: Commits using `feat:` trigger minor version bumps, `fix:` triggers patch bumps, and `BREAKING CHANGE:` or `feat!:` triggers major bumps.
- **Automated GitHub Releases**: Google's `release-please` automatically maintains release PRs, generates `CHANGELOG.md`, and creates Git tags on merge to `main`.
- **Dynamic Build Injection**: CMake extracts Git describe tags and commit hashes at compile time into `RmapVersion.hpp`. The binary version (`rmap --version`) always matches the Git revision with zero manual intervention.
- **Local Automation**:
  ```bash
  # Check active version
  make version

  # Auto-bump version based on commits since last release
  make bump-auto

  # Install Conventional Commits git hook to prevent non-conforming commits
  make setup-hooks
  ```

---

## Documentation

Full documentation is available on [GitHub Pages](https://zkalves.github.io/rmap/):

- [Getting Started Guide](https://zkalves.github.io/rmap/user/getting-started.html)
- [GUI & Register Design](https://zkalves.github.io/rmap/user/gui-guide.html)
- [CLI Reference & Automation](https://zkalves.github.io/rmap/user/cli-reference.html)
- [Templates & Code Generation](https://zkalves.github.io/rmap/user/templates-and-codegen.html)
- [Architecture & Internal Data Flow](https://zkalves.github.io/rmap/user/architecture.html)

---

## License

This project is licensed under the Mozilla Public License 2.0 (MPL 2.0) — see the [LICENSE](LICENSE) file for details.
