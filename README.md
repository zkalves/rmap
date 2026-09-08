# rmap — Hardware Register Map Designer & Model Generator

[![CI](https://github.com/zkalves/rmap/actions/workflows/ci.yml/badge.svg)](https://github.com/zkalves/rmap/actions/workflows/ci.yml)
[![License: MPL 2.0](https://img.shields.io/badge/License-MPL_2.0-blue.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Qt 6](https://img.shields.io/badge/Qt-6-green.svg)](https://www.qt.io/)
<!-- COVERAGE_BADGES_START -->
[![Line Coverage](https://img.shields.io/badge/Line_Coverage-80.0%25-green.svg)](#code-coverage-metrics)
[![Function Coverage](https://img.shields.io/badge/Function_Coverage-82.6%25-brightgreen.svg)](#code-coverage-metrics)
[![Branch Coverage](https://img.shields.io/badge/Branch_Coverage-42.9%25-orange.svg)](#code-coverage-metrics)
[![Condition Coverage](https://img.shields.io/badge/Condition_Coverage-54.1%25-yellow.svg)](#code-coverage-metrics)
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
| **Lines** | 6,110 | 7,638 | **79.99%** | ✅ |
| **Functions** | 450 | 545 | **82.57%** | ✅ |
| **Branches** | 9,695 | 22,611 | **42.88%** | ⚠️ |
| **Conditions (MC/DC)** | 3,456 | 6,388 | **54.10%** | ✅ |
| **Calls** | 13,241 | 20,881 | **63.41%** | ✅ |
| **Basic Blocks** | 19,190 | 38,293 | **50.11%** | ✅ |

### Architectural Subsystems Breakdown

| Subsystem | Lines (%) | Functions (%) | Branches (%) | Conditions (%) | Calls (%) | Blocks (%) |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| **Code Generation Engine** | 82.4% (385/467) | 81.8% (18/22) | 45.1% (769/1706) | 55.1% (259/470) | 65.0% | 50.0% |
| **Core Architecture & Model** | 89.6% (640/714) | 87.0% (67/77) | 50.8% (1025/2019) | 71.0% (369/520) | 69.1% | 52.9% |
| **Dialogs & Configuration** | 67.0% (553/825) | 79.1% (72/91) | 30.6% (565/1846) | 41.3% (186/450) | 50.5% | 42.4% |
| **Format Parsers & Serializers** | 82.2% (1142/1389) | 73.8% (45/61) | 46.0% (2550/5546) | 58.1% (962/1656) | 66.8% | 52.9% |
| **GUI Widgets & Main Window** | 74.7% (1893/2536) | 79.2% (137/173) | 38.6% (2763/7156) | 46.3% (900/1942) | 58.4% | 46.1% |
| **System Services & Utilities** | 87.7% (1497/1707) | 91.7% (111/121) | 46.6% (2023/4338) | 57.8% (780/1350) | 68.6% | 53.8% |

<details>
<summary><b>Detailed Source Files Coverage (Click to expand)</b></summary>

| Source File | Subsystem | Lines (%) | Functions (%) | Branches (%) | Conditions (%) | Calls (%) | Blocks (%) |
| :--- | :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| `src/AboutWindow.cpp` | Dialogs & Configuration | 90.8% | 90.9% | 50.0% | 78.6% | 71.7% | 62.6% |
| `src/AboutWindow.hpp` | Dialogs & Configuration | 100.0% | 100.0% | 0.0% | 0.0% | 100.0% | 100.0% |
| `src/AppSettings.cpp` | System Services & Utilities | 96.0% | 97.4% | 53.8% | 66.9% | 77.1% | 65.0% |
| `src/BlockMemoryMapWidget.cpp` | GUI Widgets & Main Window | 71.8% | 66.7% | 38.7% | 47.6% | 57.4% | 44.7% |
| `src/BlockMemoryMapWidget.hpp` | GUI Widgets & Main Window | 100.0% | 100.0% | 0.0% | 0.0% | 100.0% | 100.0% |
| `src/CodeGenerator.cpp` | Code Generation Engine | 82.4% | 80.0% | 45.1% | 55.1% | 65.0% | 50.0% |
| `src/CodeGenerator.hpp` | Code Generation Engine | 100.0% | 100.0% | 0.0% | 0.0% | 0.0% | 100.0% |
| `src/LanguageManager.cpp` | System Services & Utilities | 78.4% | 84.2% | 47.6% | 60.6% | 64.2% | 53.8% |
| `src/LanguageManager.hpp` | System Services & Utilities | 100.0% | 100.0% | 75.0% | 27.8% | 25.9% | 26.1% |
| `src/ObjectFactory.hpp` | Core Architecture & Model | 100.0% | 100.0% | 33.3% | 50.0% | 50.0% | 70.0% |
| `src/PathUtils.cpp` | System Services & Utilities | 72.8% | 91.7% | 36.0% | 44.9% | 51.8% | 41.6% |
| `src/PathUtils.hpp` | System Services & Utilities | 100.0% | 100.0% | 50.0% | 0.0% | 50.0% | 50.0% |
| `src/PreferencesWindow.cpp` | Dialogs & Configuration | 83.8% | 92.0% | 42.5% | 53.3% | 72.3% | 61.1% |
| `src/PreferencesWindow.hpp` | Dialogs & Configuration | 100.0% | 100.0% | 0.0% | 0.0% | 100.0% | 100.0% |
| `src/ProtobufLogCollector.cpp` | System Services & Utilities | 0.0% | 0.0% | 0.0% | 0.0% | 0.0% | 0.0% |
| `src/ProtobufLogCollector.hpp` | System Services & Utilities | 66.7% | 66.7% | 0.0% | 0.0% | 36.4% | 35.3% |
| `src/RegBitfieldBarWidget.cpp` | GUI Widgets & Main Window | 44.0% | 63.2% | 19.7% | 22.6% | 30.2% | 23.7% |
| `src/RegBitfieldBarWidget.hpp` | GUI Widgets & Main Window | 50.0% | 50.0% | 0.0% | 0.0% | 0.0% | 50.0% |
| `src/RegConfigWindow.cpp` | Dialogs & Configuration | 58.7% | 68.6% | 27.2% | 38.0% | 42.6% | 36.1% |
| `src/RegConfigWindow.hpp` | Dialogs & Configuration | 100.0% | 100.0% | 0.0% | 0.0% | 100.0% | 100.0% |
| `src/RegMapDelegate.cpp` | GUI Widgets & Main Window | 89.3% | 92.3% | 44.3% | 48.5% | 69.6% | 57.3% |
| `src/RegMapTreeItem.cpp` | Core Architecture & Model | 87.9% | 94.4% | 50.9% | 64.4% | 71.5% | 61.9% |
| `src/RegMapTreeItem.hpp` | Core Architecture & Model | 100.0% | 75.0% | 0.0% | 0.0% | 25.0% | 29.4% |
| `src/RegMapTreeModel.cpp` | Core Architecture & Model | 92.7% | 90.6% | 52.1% | 76.0% | 70.2% | 51.7% |
| `src/RegMapTreeModel.hpp` | Core Architecture & Model | 100.0% | 100.0% | 0.0% | 0.0% | 100.0% | 25.0% |
| `src/RegMapTreeView.cpp` | Core Architecture & Model | 28.6% | 50.0% | 0.0% | 0.0% | 20.0% | 25.0% |
| `src/RegMapWindow.cpp` | GUI Widgets & Main Window | 78.6% | 81.1% | 39.9% | 49.1% | 60.3% | 47.6% |
| `src/RegMapWindow.hpp` | GUI Widgets & Main Window | 50.0% | 100.0% | 0.0% | 0.0% | 25.0% | 100.0% |
| `src/Serializable.hpp` | Core Architecture & Model | 100.0% | 100.0% | 0.0% | 0.0% | 0.0% | 25.0% |
| `src/SerializationContext.hpp` | Core Architecture & Model | 95.0% | 100.0% | 57.1% | 52.8% | 61.5% | 60.4% |
| `src/ThemeManager.cpp` | System Services & Utilities | 92.3% | 95.3% | 50.9% | 63.4% | 76.0% | 58.6% |
| `src/UndoCommands.hpp` | Core Architecture & Model | 76.8% | 71.4% | 40.4% | 53.6% | 63.2% | 50.0% |
| `src/format/CmsisSvdHandler.cpp` | Format Parsers & Serializers | 87.1% | 100.0% | 50.1% | 67.3% | 67.6% | 55.0% |
| `src/format/CmsisSvdHandler.hpp` | Format Parsers & Serializers | 60.0% | 60.0% | 33.3% | 33.3% | 45.5% | 42.9% |
| `src/format/CsvHandler.cpp` | Format Parsers & Serializers | 85.3% | 100.0% | 44.7% | 52.4% | 68.1% | 52.0% |
| `src/format/CsvHandler.hpp` | Format Parsers & Serializers | 25.0% | 33.3% | 35.7% | 33.3% | 50.0% | 40.9% |
| `src/format/FormatManager.cpp` | Format Parsers & Serializers | 56.0% | 63.6% | 32.1% | 42.1% | 44.4% | 38.6% |
| `src/format/IFormatHandler.hpp` | Format Parsers & Serializers | 100.0% | 100.0% | 0.0% | 0.0% | 0.0% | 4.2% |
| `src/format/IpxactHandler.cpp` | Format Parsers & Serializers | 89.8% | 100.0% | 49.7% | 65.8% | 69.5% | 55.1% |
| `src/format/IpxactHandler.hpp` | Format Parsers & Serializers | 25.0% | 33.3% | 35.7% | 33.3% | 50.0% | 40.9% |
| `src/format/JsonHandler.cpp` | Format Parsers & Serializers | 91.3% | 100.0% | 43.8% | 56.4% | 70.5% | 53.5% |
| `src/format/JsonHandler.hpp` | Format Parsers & Serializers | 25.0% | 33.3% | 33.3% | 33.3% | 42.9% | 38.1% |
| `src/format/ProtobufHandler.cpp` | Format Parsers & Serializers | 85.7% | 100.0% | 45.8% | 58.8% | 61.1% | 51.0% |
| `src/format/ProtobufHandler.hpp` | Format Parsers & Serializers | 25.0% | 33.3% | 35.7% | 33.3% | 50.0% | 40.9% |
| `src/format/SystemRdlHandler.cpp` | Format Parsers & Serializers | 78.5% | 100.0% | 44.9% | 55.5% | 66.5% | 53.2% |
| `src/format/SystemRdlHandler.hpp` | Format Parsers & Serializers | 25.0% | 33.3% | 35.7% | 33.3% | 50.0% | 40.9% |
| `src/main.cpp` | System Services & Utilities | 76.0% | 100.0% | 39.6% | 39.1% | 59.9% | 40.9% |

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
