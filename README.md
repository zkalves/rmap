# rmap — Hardware Register Map Designer & Model Generator

[![CI](https://github.com/zkalves/rmap/actions/workflows/ci.yml/badge.svg)](https://github.com/zkalves/rmap/actions/workflows/ci.yml)
[![License: MPL 2.0](https://img.shields.io/badge/License-MPL_2.0-blue.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Qt 6](https://img.shields.io/badge/Qt-6-green.svg)](https://www.qt.io/)
<!-- COVERAGE_BADGES_START -->
[![Line Coverage](https://img.shields.io/endpoint?url=https://zkalves.github.io/rmap/coverage/badge_line.json)](https://zkalves.github.io/rmap/coverage/)
[![Function Coverage](https://img.shields.io/endpoint?url=https://zkalves.github.io/rmap/coverage/badge_function.json)](https://zkalves.github.io/rmap/coverage/)
[![Branch Coverage](https://img.shields.io/endpoint?url=https://zkalves.github.io/rmap/coverage/badge_branch.json)](https://zkalves.github.io/rmap/coverage/)
[![Condition Coverage](https://img.shields.io/endpoint?url=https://zkalves.github.io/rmap/coverage/badge_condition.json)](https://zkalves.github.io/rmap/coverage/)
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
- **Dedicated Reference Examples & Autonomous Simulation Environments**: 15 complete, self-contained reference environments in `examples/environments/` with autonomous Makefiles, C firmware verification harnesses (`test_harness.c`), Rust PAC crates (`test_harness.rs`), and Python test harnesses (`test_harness.py`). Includes focused single-feature models and an extensive SoC-scale multi-block subsystem (`soc_large_scale.rmt`). Run all environments autonomously with `make test-examples`.
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

Automated test coverage analysis across all 6 compiler-supported metrics, published continuously by the [GitHub Actions CI Pipeline](https://github.com/zkalves/rmap/actions/workflows/ci.yml):

> [!TIP]
> **Live CI/CD Coverage Pipeline**: Interactive coverage reports, line-by-line profiling, and call graphs are updated continuously by the pipeline and hosted live on [GitHub Pages: rmap Coverage Dashboard](https://zkalves.github.io/rmap/coverage/).

| Metric | Covered | Total | Coverage Rate | Status |
| :--- | :---: | :---: | :---: | :---: |
| **Lines** | 8,014 | 8,014 | **100.00%** | ✅ |
| **Functions** | 573 | 573 | **100.00%** | ✅ |
| **Branches (Decision)** | 12,286 | 13,104 | **93.76%** | ✅ |
| **Conditions (MC/DC)** | 5,156 | 5,726 | **90.05%** | ✅ |
| **Calls** | 17,014 | 21,378 | **79.59%** | ✅ |
| **Basic Blocks** | 25,587 | 39,557 | **64.68%** | ✅ |
| *Branches (Raw w/ Unwind)* | 13,380 | 22,728 | *58.87%* | ℹ️ |

> [!NOTE]
> **Branch & Condition Coverage Measurement**: In accordance with DO-178C, ISO 26262, and `gcovr` standards, decision branch coverage tracks actual logical control branches (`if`, `switch`, `while`, ternary). Compiler-synthesized exception unwinding landing pads (`throw: true`), allocation checks (`new`/`delete`), and destructor cleanups are excluded from decision branches and shown transparently in raw metrics. Standard exclusion pragmas (`// GCOV_EXCL_LINE`, `// LCOV_EXCL_START`/`STOP`, `// GCOV_EXCL_BR_LINE`) are honored.

### Architectural Subsystems Breakdown

| Subsystem | Lines (%) | Functions (%) | Branches (%) | Conditions (%) | Calls (%) | Blocks (%) |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| **Code Generation Engine** | 100.0% (477/477) | 100.0% (20/20) | 96.4% (963/999) | 92.8% (401/432) | 80.4% | 64.3% |
| **Core Architecture & Model** | 100.0% (838/838) | 100.0% (79/79) | 88.6% (1597/1803) | 90.4% (714/790) | 77.5% | 59.8% |
| **Dialogs & Configuration** | 100.0% (811/811) | 100.0% (96/96) | 98.8% (828/838) | 97.0% (287/296) | 82.3% | 68.8% |
| **Format Parsers & Serializers** | 100.0% (1615/1615) | 100.0% (66/66) | 91.1% (3564/3911) | 87.2% (1757/2014) | 78.9% | 64.9% |
| **GUI Widgets & Main Window** | 100.0% (2546/2546) | 100.0% (181/181) | 96.3% (3162/3282) | 90.7% (1105/1218) | 79.8% | 64.9% |
| **System Services & Utilities** | 100.0% (1727/1727) | 100.0% (131/131) | 95.6% (2172/2271) | 91.4% (892/976) | 80.2% | 66.0% |

<details>
<summary><b>Detailed Source Files Coverage (Click to expand)</b></summary>

| Source File | Subsystem | Lines (%) | Functions (%) | Branches (%) | Conditions (%) | Calls (%) | Blocks (%) |
| :--- | :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| `src/AboutWindow.cpp` | Dialogs & Configuration | 100.0% | 100.0% | 100.0% | 100.0% | 81.7% | 69.5% |
| `src/AboutWindow.hpp` | Dialogs & Configuration | 100.0% | 100.0% | 0.0% | 0.0% | 100.0% | 100.0% |
| `src/AppSettings.cpp` | System Services & Utilities | 100.0% | 100.0% | 96.4% | 92.0% | 81.0% | 69.0% |
| `src/BlockMemoryMapWidget.cpp` | GUI Widgets & Main Window | 100.0% | 100.0% | 96.2% | 91.2% | 80.8% | 66.2% |
| `src/BlockMemoryMapWidget.hpp` | GUI Widgets & Main Window | 100.0% | 100.0% | 0.0% | 0.0% | 100.0% | 100.0% |
| `src/CodeGenerator.cpp` | Code Generation Engine | 100.0% | 100.0% | 96.4% | 92.8% | 80.4% | 64.3% |
| `src/CodeGenerator.hpp` | Code Generation Engine | 100.0% | 100.0% | 0.0% | 0.0% | 0.0% | 100.0% |
| `src/LanguageManager.cpp` | System Services & Utilities | 100.0% | 100.0% | 91.2% | 91.4% | 82.8% | 70.5% |
| `src/ObjectFactory.hpp` | Core Architecture & Model | 100.0% | 100.0% | 0.0% | 0.0% | 50.0% | 70.0% |
| `src/PathUtils.cpp` | System Services & Utilities | 100.0% | 100.0% | 97.1% | 94.6% | 76.5% | 64.5% |
| `src/PathUtils.hpp` | System Services & Utilities | 100.0% | 100.0% | 100.0% | 0.0% | 100.0% | 100.0% |
| `src/PreferencesWindow.cpp` | Dialogs & Configuration | 100.0% | 100.0% | 95.8% | 87.5% | 85.5% | 72.5% |
| `src/PreferencesWindow.hpp` | Dialogs & Configuration | 100.0% | 100.0% | 0.0% | 0.0% | 100.0% | 100.0% |
| `src/ProtobufLogCollector.cpp` | System Services & Utilities | 100.0% | 100.0% | 100.0% | 0.0% | 68.0% | 45.2% |
| `src/ProtobufLogCollector.hpp` | System Services & Utilities | 100.0% | 100.0% | 0.0% | 0.0% | 45.5% | 47.1% |
| `src/RegBitfieldBarWidget.cpp` | GUI Widgets & Main Window | 100.0% | 100.0% | 95.7% | 94.0% | 81.3% | 69.0% |
| `src/RegBitfieldBarWidget.hpp` | GUI Widgets & Main Window | 100.0% | 100.0% | 0.0% | 0.0% | 100.0% | 100.0% |
| `src/RegConfigWindow.cpp` | Dialogs & Configuration | 100.0% | 100.0% | 99.5% | 99.1% | 81.4% | 67.7% |
| `src/RegConfigWindow.hpp` | Dialogs & Configuration | 100.0% | 100.0% | 0.0% | 0.0% | 100.0% | 100.0% |
| `src/RegMapDelegate.cpp` | GUI Widgets & Main Window | 100.0% | 100.0% | 97.2% | 93.9% | 79.6% | 67.1% |
| `src/RegMapTreeItem.cpp` | Core Architecture & Model | 100.0% | 100.0% | 95.1% | 93.8% | 79.9% | 72.5% |
| `src/RegMapTreeModel.cpp` | Core Architecture & Model | 100.0% | 100.0% | 86.7% | 91.0% | 77.5% | 57.5% |
| `src/RegMapTreeModel.hpp` | Core Architecture & Model | 100.0% | 100.0% | 0.0% | 0.0% | 100.0% | 25.0% |
| `src/RegMapTreeView.cpp` | Core Architecture & Model | 100.0% | 100.0% | 100.0% | 0.0% | 100.0% | 100.0% |
| `src/RegMapWindow.cpp` | GUI Widgets & Main Window | 100.0% | 100.0% | 96.4% | 89.4% | 79.5% | 64.0% |
| `src/RegMapWindow.hpp` | GUI Widgets & Main Window | 100.0% | 100.0% | 100.0% | 0.0% | 75.0% | 100.0% |
| `src/Serializable.hpp` | Core Architecture & Model | 100.0% | 100.0% | 0.0% | 0.0% | 0.0% | 25.0% |
| `src/SerializationContext.hpp` | Core Architecture & Model | 100.0% | 100.0% | 93.3% | 66.7% | 61.5% | 64.0% |
| `src/ThemeManager.cpp` | System Services & Utilities | 100.0% | 100.0% | 95.6% | 89.2% | 82.4% | 67.8% |
| `src/UndoCommands.cpp` | Core Architecture & Model | 100.0% | 100.0% | 98.0% | 91.7% | 79.1% | 64.6% |
| `src/format/CmsisSvdHandler.cpp` | Format Parsers & Serializers | 100.0% | 100.0% | 86.3% | 82.5% | 76.6% | 62.4% |
| `src/format/CmsisSvdHandler.hpp` | Format Parsers & Serializers | 100.0% | 100.0% | 0.0% | 0.0% | 100.0% | 100.0% |
| `src/format/CsvHandler.cpp` | Format Parsers & Serializers | 100.0% | 100.0% | 92.8% | 88.5% | 76.5% | 61.5% |
| `src/format/FormatManager.cpp` | Format Parsers & Serializers | 100.0% | 100.0% | 98.7% | 91.2% | 76.5% | 66.0% |
| `src/format/IpxactHandler.cpp` | Format Parsers & Serializers | 100.0% | 100.0% | 90.4% | 84.8% | 76.7% | 61.9% |
| `src/format/JsonHandler.cpp` | Format Parsers & Serializers | 100.0% | 100.0% | 95.0% | 89.0% | 80.3% | 64.2% |
| `src/format/ProtobufHandler.cpp` | Format Parsers & Serializers | 100.0% | 100.0% | 94.1% | 88.1% | 79.4% | 64.9% |
| `src/format/SystemRdlHandler.cpp` | Format Parsers & Serializers | 100.0% | 100.0% | 92.2% | 90.9% | 83.5% | 72.8% |
| `src/main.cpp` | System Services & Utilities | 100.0% | 100.0% | 98.1% | 94.9% | 72.3% | 51.5% |

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
- **Qt 6** (`qt6-base-dev`, `libqt6test6`) *(optional: required for interactive GUI and Qt test runner; rmap falls back gracefully to a lightweight CLI-only executable if absent)*
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

# Standard build with Qt GUI (default when Qt6 is detected)
make

# Or build headless CLI-only without Qt
cmake -B build -S . -DCMAKE_DISABLE_FIND_PACKAGE_Qt6=TRUE
cmake --build build

# Run automated unit test suites (100% pass rate)
make test

# Run comprehensive template verification tests across all 15 output templates
make test-templates

# Run autonomous simulation and compilation across all 19 example environments
make test-examples

# Run complete verification suite
make test-all
```

### Installation

Install the `rmap` executable, code generation templates (`<prefix>/share/rmap/templates`), sample models (`<prefix>/share/rmap/examples`), and pre-rendered documentation in interactive HTML (`<prefix>/share/doc/rmap/html`) and split PDF formats (User Manual: `<prefix>/share/doc/rmap/rmap_user_manual.pdf` and Developer Guide: `<prefix>/share/doc/rmap/rmap_developer_guide.pdf`) to your preferred directory using either Make or CMake. The application automatically discovers installed templates without requiring manual configuration:

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
 
Full documentation is available online on [GitHub Pages](https://zkalves.github.io/rmap/) and installed locally in **interactive HTML** (`share/doc/rmap/html`) and dedicated **split PDF manuals** (`share/doc/rmap/`):
 
 - [Getting Started Guide](https://zkalves.github.io/rmap/user/getting-started.html)
 - [GUI & Register Design](https://zkalves.github.io/rmap/user/gui-guide.html)
 - [CLI Reference & Automation](https://zkalves.github.io/rmap/user/cli-reference.html)
 - [Templates & Code Generation](https://zkalves.github.io/rmap/user/templates-and-codegen.html)
 - [Architecture & Internal Data Flow](https://zkalves.github.io/rmap/user/architecture.html)
 - [Download User Manual (PDF)](https://zkalves.github.io/rmap/pdf/rmap_user_manual.pdf) (50-page user guide)
 - [Download Developer Guide (PDF)](https://zkalves.github.io/rmap/pdf/rmap_developer_guide.pdf) (77-page C++ architecture manual)

---

## License

This project is licensed under the Mozilla Public License 2.0 (MPL 2.0) — see the [LICENSE](LICENSE) file for details.
