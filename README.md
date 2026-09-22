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

**rmap** is a high-performance GUI & CLI tool for designing hardware register maps, validating architectures, and generating synthesizable RTL, UVM verification environments, firmware headers, and documentation.

---

## Key Features

- **Interactive Bitfield Visualizer**: Segmented 32/64-bit slice bar with color-coded access badges (`RW`, `RO`, `W1C`, etc.), live search, real-time filtering, and multi-level undo/redo (`Ctrl+Z` / `Ctrl+Y`).
- **Universal Multi-Format Interop**: Lossless import, export, and cross-conversion across **SystemRDL 1.0/2.0**, **ARM CMSIS-SVD**, **IP-XACT** (2009/2014/2022), **JSON**, **CSV/TSV**, and **Protobuf**.
- **Turnkey Code Generation**: One-click generation of synthesizable **SystemVerilog RTL**, complete **UVM** register models (IEEE 1800.2 & pyuvm), self-checking testbenches, **C/C++** headers, **Rust PAC** crates, **Python** drivers, and **HTML/Markdown** specs.
- **Headless CLI & Validation**: Command-line validation (`--lint`, `--strict`) with SARIF, JUnit XML, JSON, and text reports; semantic structural diffing (`--diff`) across revisions.
- **Python Scripting Hooks**: Automatically invoke custom Python scripts post-generation with the complete register model injected into the script context.
- **Themes & Accessibility**: Curated dark/light themes (Solarized, Nord, Dracula), WCAG AAA color-blind modes (Okabe-Ito palettes), and 7-language UI localization.

---

<!-- COVERAGE_SECTION_START -->
## Code Coverage Metrics

Automated test coverage analysis across all 6 compiler-supported metrics, published continuously by the [GitHub Actions CI Pipeline](https://github.com/zkalves/rmap/actions/workflows/ci.yml):

> [!TIP]
> **Live CI/CD Coverage Pipeline**: Interactive coverage reports, line-by-line profiling, and call graphs are updated continuously by the pipeline and hosted live on [GitHub Pages: rmap Coverage Dashboard](https://zkalves.github.io/rmap/coverage/).

| Metric | Covered | Total | Coverage Rate | Status |
| :--- | :---: | :---: | :---: | :---: |
| **Lines** | 8,783 | 8,783 | **100.00%** | ✅ |
| **Functions** | 584 | 584 | **100.00%** | ✅ |
| **Branches (Decision)** | 13,403 | 14,365 | **93.30%** | ✅ |
| **Conditions (MC/DC)** | 5,827 | 6,548 | **88.99%** | ✅ |
| **Calls** | 17,791 | 22,361 | **79.56%** | ✅ |
| **Basic Blocks** | 26,984 | 41,883 | **64.43%** | ✅ |
| *Branches (Raw w/ Unwind)* | 14,345 | 24,182 | *59.32%* | ℹ️ |

> [!NOTE]
> **Branch & Condition Coverage Measurement**: In accordance with DO-178C, ISO 26262, and `gcovr` standards, decision branch coverage tracks actual logical control branches (`if`, `switch`, `while`, ternary). Compiler-synthesized exception unwinding landing pads (`throw: true`), allocation checks (`new`/`delete`), and destructor cleanups are excluded from decision branches and shown transparently in raw metrics. Standard exclusion pragmas (`// GCOV_EXCL_LINE`, `// LCOV_EXCL_START`/`STOP`, `// GCOV_EXCL_BR_LINE`) are honored.

### Architectural Subsystems Breakdown

| Subsystem | Lines (%) | Functions (%) | Branches (%) | Conditions (%) | Calls (%) | Blocks (%) |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| **Code Generation Engine** | 100.0% (672/672) | 100.0% (21/21) | 94.7% (1240/1309) | 91.4% (552/604) | 80.9% | 64.7% |
| **Core Architecture & Model** | 100.0% (839/839) | 100.0% (79/79) | 88.6% (1597/1803) | 89.4% (706/790) | 77.8% | 59.3% |
| **Dialogs & Configuration** | 100.0% (820/820) | 100.0% (96/96) | 98.1% (844/860) | 95.9% (301/314) | 82.3% | 68.5% |
| **Format Parsers & Serializers** | 100.0% (1850/1850) | 100.0% (69/69) | 92.1% (3958/4299) | 88.3% (2039/2308) | 79.6% | 66.1% |
| **GUI Widgets & Main Window** | 100.0% (2790/2790) | 100.0% (182/182) | 94.4% (3356/3555) | 87.0% (1241/1426) | 79.7% | 64.4% |
| **System Services & Utilities** | 100.0% (1812/1812) | 100.0% (137/137) | 94.8% (2408/2539) | 89.3% (988/1106) | 78.8% | 63.9% |

<details>
<summary><b>Detailed Source Files Coverage (Click to expand)</b></summary>

| Source File | Subsystem | Lines (%) | Functions (%) | Branches (%) | Conditions (%) | Calls (%) | Blocks (%) |
| :--- | :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| `src/AboutWindow.cpp` | Dialogs & Configuration | 100.0% | 100.0% | 98.0% | 91.7% | 81.1% | 69.5% |
| `src/AboutWindow.hpp` | Dialogs & Configuration | 100.0% | 100.0% | 0.0% | 0.0% | 100.0% | 100.0% |
| `src/AppSettings.cpp` | System Services & Utilities | 100.0% | 100.0% | 96.2% | 91.9% | 81.1% | 69.2% |
| `src/BlockMemoryMapWidget.cpp` | GUI Widgets & Main Window | 100.0% | 100.0% | 96.2% | 91.2% | 80.8% | 66.2% |
| `src/BlockMemoryMapWidget.hpp` | GUI Widgets & Main Window | 100.0% | 100.0% | 0.0% | 0.0% | 100.0% | 100.0% |
| `src/CodeGenerator.cpp` | Code Generation Engine | 100.0% | 100.0% | 94.7% | 91.4% | 80.9% | 64.7% |
| `src/CodeGenerator.hpp` | Code Generation Engine | 100.0% | 100.0% | 0.0% | 0.0% | 0.0% | 100.0% |
| `src/LanguageManager.cpp` | System Services & Utilities | 100.0% | 100.0% | 90.0% | 87.3% | 81.3% | 67.5% |
| `src/ObjectFactory.hpp` | Core Architecture & Model | 100.0% | 100.0% | 0.0% | 0.0% | 25.0% | 35.0% |
| `src/PathUtils.cpp` | System Services & Utilities | 100.0% | 100.0% | 96.6% | 91.4% | 76.4% | 64.3% |
| `src/PathUtils.hpp` | System Services & Utilities | 100.0% | 100.0% | 100.0% | 0.0% | 100.0% | 100.0% |
| `src/PreferencesWindow.cpp` | Dialogs & Configuration | 100.0% | 100.0% | 93.5% | 86.1% | 85.3% | 70.8% |
| `src/PreferencesWindow.hpp` | Dialogs & Configuration | 100.0% | 100.0% | 0.0% | 0.0% | 100.0% | 100.0% |
| `src/ProtobufLogCollector.cpp` | System Services & Utilities | 100.0% | 100.0% | 100.0% | 0.0% | 68.0% | 45.2% |
| `src/ProtobufLogCollector.hpp` | System Services & Utilities | 100.0% | 100.0% | 0.0% | 0.0% | 100.0% | 47.1% |
| `src/RegBitfieldBarWidget.cpp` | GUI Widgets & Main Window | 100.0% | 100.0% | 95.7% | 94.0% | 81.3% | 69.0% |
| `src/RegBitfieldBarWidget.hpp` | GUI Widgets & Main Window | 100.0% | 100.0% | 0.0% | 0.0% | 100.0% | 100.0% |
| `src/RegConfigWindow.cpp` | Dialogs & Configuration | 100.0% | 100.0% | 99.5% | 99.1% | 81.4% | 67.7% |
| `src/RegConfigWindow.hpp` | Dialogs & Configuration | 100.0% | 100.0% | 0.0% | 0.0% | 100.0% | 100.0% |
| `src/RegMapDelegate.cpp` | GUI Widgets & Main Window | 100.0% | 100.0% | 97.2% | 93.9% | 79.6% | 67.1% |
| `src/RegMapTreeItem.cpp` | Core Architecture & Model | 100.0% | 100.0% | 95.1% | 93.8% | 79.9% | 72.5% |
| `src/RegMapTreeModel.cpp` | Core Architecture & Model | 100.0% | 100.0% | 86.7% | 91.0% | 77.5% | 57.5% |
| `src/RegMapTreeModel.hpp` | Core Architecture & Model | 100.0% | 100.0% | 0.0% | 0.0% | 100.0% | 25.0% |
| `src/RegMapTreeView.cpp` | Core Architecture & Model | 100.0% | 100.0% | 100.0% | 0.0% | 100.0% | 100.0% |
| `src/RegMapWindow.cpp` | GUI Widgets & Main Window | 100.0% | 100.0% | 93.7% | 84.2% | 79.4% | 63.4% |
| `src/RegMapWindow.hpp` | GUI Widgets & Main Window | 100.0% | 100.0% | 100.0% | 0.0% | 75.0% | 100.0% |
| `src/Serializable.hpp` | Core Architecture & Model | 100.0% | 100.0% | 0.0% | 0.0% | 0.0% | 25.0% |
| `src/SerializationContext.hpp` | Core Architecture & Model | 100.0% | 100.0% | 93.3% | 44.4% | 78.1% | 45.0% |
| `src/ThemeManager.cpp` | System Services & Utilities | 100.0% | 100.0% | 95.1% | 88.1% | 79.9% | 63.9% |
| `src/UndoCommands.cpp` | Core Architecture & Model | 100.0% | 100.0% | 98.0% | 91.7% | 79.1% | 64.6% |
| `src/format/CmsisSvdHandler.cpp` | Format Parsers & Serializers | 100.0% | 100.0% | 90.9% | 88.0% | 78.5% | 64.9% |
| `src/format/CmsisSvdHandler.hpp` | Format Parsers & Serializers | 100.0% | 100.0% | 0.0% | 0.0% | 100.0% | 100.0% |
| `src/format/CsvHandler.cpp` | Format Parsers & Serializers | 100.0% | 100.0% | 93.1% | 88.9% | 76.5% | 61.5% |
| `src/format/FormatManager.cpp` | Format Parsers & Serializers | 100.0% | 100.0% | 87.1% | 81.0% | 82.0% | 74.3% |
| `src/format/IpxactHandler.cpp` | Format Parsers & Serializers | 100.0% | 100.0% | 91.4% | 86.6% | 76.8% | 62.0% |
| `src/format/JsonHandler.cpp` | Format Parsers & Serializers | 100.0% | 100.0% | 95.0% | 89.0% | 80.3% | 64.2% |
| `src/format/ProtobufHandler.cpp` | Format Parsers & Serializers | 100.0% | 100.0% | 94.1% | 88.1% | 79.4% | 64.9% |
| `src/format/SystemRdlHandler.cpp` | Format Parsers & Serializers | 100.0% | 100.0% | 92.4% | 91.1% | 83.6% | 72.9% |
| `src/main.cpp` | System Services & Utilities | 100.0% | 100.0% | 96.3% | 90.6% | 72.4% | 52.2% |

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
- **Protocol Buffers** (`libprotobuf-dev`, `protobuf-compiler`)
- **Qt 6** (`qt6-base-dev`, `libqt6test6`) *(optional: builds lightweight headless CLI if absent)*

```bash
# Ubuntu / Debian
sudo apt-get install -y build-essential cmake git qt6-base-dev libqt6test6 libprotobuf-dev protobuf-compiler

# Fedora / RHEL / Rocky / AlmaLinux / CentOS Stream
sudo dnf install -y gcc-c++ cmake git qt6-qtbase-devel protobuf-devel protobuf-compiler
```

### Build & Test

```bash
git clone https://github.com/zkalves/rmap.git
cd rmap

make            # Build with Qt GUI (or headless CLI if Qt6 is absent)
make test       # Run unit test suite
make test-all   # Run complete verification suite (templates, examples, simulation)
```

### Installation

```bash
sudo make install                  # Default prefix: /usr/local
make install PREFIX=$HOME/.local  # Custom user prefix
```

---

## Usage

### Interactive GUI

```bash
./build/bin/rmap                                    # Launch GUI
./build/bin/rmap -f examples/rmt/peripherals/spi.rmt  # Open existing register map
```

### Headless CLI

```bash
# Code generation & format conversion
./build/bin/rmap -f spi.rmt --export --out ./work
./build/bin/rmap -f spi.rmt --convert spi.svd

# Architectural linting (SARIF, JUnit XML, JSON, or text)
./build/bin/rmap -f spi.rmt --lint --strict --report-format sarif --out lint.sarif

# Semantic structural diffing between revisions
./build/bin/rmap -f rev_a.rmt --diff rev_b.rmt --report-format markdown --out diff.md
```

---

## Releases & Versioning

`rmap` adheres to [Semantic Versioning 2.0.0](https://semver.org/) and [Conventional Commits](https://www.conventionalcommits.org/). Releases, changelogs, and Git tags are managed automatically via CI.

---

## Documentation

Full documentation is available on [GitHub Pages](https://zkalves.github.io/rmap/):

- [Getting Started Guide](https://zkalves.github.io/rmap/user/getting-started.html)
- [GUI & Register Design](https://zkalves.github.io/rmap/user/gui-guide.html)
- [CLI Reference & Automation](https://zkalves.github.io/rmap/user/cli-reference.html)
- [Templates & Code Generation](https://zkalves.github.io/rmap/user/templates-and-codegen.html)
- [Architecture & Internal Data Flow](https://zkalves.github.io/rmap/user/architecture.html)
- [User Manual (PDF)](https://zkalves.github.io/rmap/pdf/rmap_user_manual.pdf) | [Developer Guide (PDF)](https://zkalves.github.io/rmap/pdf/rmap_developer_guide.pdf)

---

## License

This project is licensed under the Mozilla Public License 2.0 (MPL 2.0) — see the [LICENSE](LICENSE) file for details.
