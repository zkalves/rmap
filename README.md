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
