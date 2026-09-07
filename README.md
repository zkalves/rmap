# rmap — Hardware Register Map Designer & Model Generator

[![CI](https://github.com/zkalves/rmap/actions/workflows/ci.yml/badge.svg)](https://github.com/zkalves/rmap/actions/workflows/ci.yml)
[![License: MPL 2.0](https://img.shields.io/badge/License-MPL_2.0-blue.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Qt 6](https://img.shields.io/badge/Qt-6-green.svg)](https://www.qt.io/)

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
- **Automated CI/CD Linter**: Headless validation engine (`--lint`, `--strict`) with machine-readable reports in **SARIF** (GitHub PR code scanning), **JUnit XML** (CI test dashboards), **JSON**, or human-readable **Text**.
- **Semantic Register Map Diff Engine**: Headless structural diffing (`--diff`) comparing registers, addresses, bitfields, and access policies across versions with Text and Markdown reports.
- **Curated Multi-Theme Engine & User Preferences**: Built-in dark and light colour schemes (**Solarized 8 (Dark)** default, **Solarized 8 (Light)**, **Nord**, **Dracula**, **Monokai**, **Classic Light**). User GUI settings persist in the user's home configuration file (`~/.config/rmap/rmap.conf`), keeping register map model files decoupled from client-side appearance preferences.
- **Relative Paths & Environment Variable Expansion**: Full support for relative paths, absolute paths, and environment variable expansion (`$VAR`, `${VAR}`, Windows `%VAR%`, and `~`) across all CLI options, configuration settings, and template mappings. All saved repository files store clean, portable relative paths.
- **Conflict-Free Keyboard Shortcuts**: Standardized OS-friendly shortcuts (`Ctrl+Shift+M` for Mem, `Ctrl+Shift+S` for Save As, `Ctrl+Shift+F` for Field, `Ctrl+Shift+B` for Block, `Ctrl+Shift+R` for Register, `Ctrl+Z` for Undo, `Ctrl+Y` for Redo, `Ctrl+P` for Configuration, `Ctrl+,` for Preferences, `Ctrl+Alt+C` for Colour-Blind Mode).

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

Install the `rmap` executable to your preferred directory using either Make or CMake:

#### With Make
```bash
# Default system-wide installation (/usr/local/bin/rmap)
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
