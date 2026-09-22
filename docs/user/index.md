# rmap Documentation

Welcome to the documentation for **rmap**, an open-source tool for hardware register map authoring, RTL register file synthesis (SystemVerilog, Verilog-2001, Synthesizable VHDL), verification model synthesis (UVM SystemVerilog, PyUVM/Cocotb, SVA), and firmware header generation (C/C++, Rust, Python).

---

## Documentation Sections

- [🚀 Getting Started](getting-started.md) — Installation, building, and basic workflow.
- [🖥️ GUI User Guide](gui-guide.md) — Visual design, bitfield packing, validation, and context menus.
- [⚙️ CLI Reference & Automation](cli-reference.md) — Headless batch generation for CI/CD and Makefiles.
- [📝 Templates & Code Generation](templates-and-codegen.md) — Writing Inja templates, using helper functions, and custom variables.
- [🏛️ Architecture & Internals](architecture.md) — Data models, Protobuf serialization, and validation engine.

---

## Core Capabilities

- **Dual-Pane Interface**: Hierarchy navigation for Blocks and Registers on the left, with bitfield editing on the right.
- **Real-Time Validation**: Instant detection of register address collisions, bitfield overlaps, and bit-width boundary overflows.
- **Headless CLI Generation**: Seamlessly generate C headers, RTL, and UVM models directly in Makefiles and CI/CD pipelines without launching a GUI.
- **Flexible Code Generation**: Built with [Pantor Inja](https://github.com/pantor/inja) and [nlohmann/json](https://github.com/nlohmann/json). Easily customize or write templates.
- **Multi-Format Storage**: Robust Google Protocol Buffers storage supporting human-readable text (`.rmt`) and high-performance binary (`.rmb`) formats, plus lossless conversion across ARM CMSIS-SVD, Accellera SystemRDL, and IP-XACT IEEE 1685.
- **24 IEEE 1800.2 UVM Access Policies**: Native support for all 24 standard UVM register access policies (`RW`, `RO`, `WO`, `W1`, `WO1`, `W1C`, `W1S`, `W1T`, `W0C`, `W0S`, `W0T`, `RC`, `RS`, `WRC`, `WRS`, `WC`, `WS`, `W1SRC`, `W1CRS`, `W0SRC`, `W0CRS`, `WOC`, `WOS`, `NOACCESS` / `NA`).
- **Dynamic Parameterizable Data Widths**: Supports arbitrary word widths (8, 16, 32, 64, 128, 256, 512+ bits) with automatic byte-strobe generation.
- **Configurable HW/SW Arbitration Precedence**: Parameterizable hardware vs. software precedence (`PARAM_HW_PRECEDENCE`) for concurrent writes.
- **Synthesizable Multi-Target RTL**: Generates standard IEEE 1800-2017 SystemVerilog, IEEE 1364-2001 Verilog, and IEEE 1076 VHDL with APB4/AXI4-Lite wrappers and formal SVA assertions.
- **Flexible Number Formatting**: Input offsets, sizes, and reset values seamlessly using Hex (`0x`), Decimal, or Binary (`0b`).

---

## Quick Example

```bash
# Build the application
make

# Run all automated test suites
make test

# Generate C header, RTL, and UVM models headlessly from an existing map
./build/bin/rmap -f examples/rmt/peripherals/spi.rmt --export --out ./work
```

---

## 👤 Author & GitHub Repository

- **Author**: Ezequiel Alves ([@@zkalves](https://github.com/zkalves))
- **GitHub Page**: [https://github.com/zkalves/rmap](https://github.com/zkalves/rmap)
- **License**: Mozilla Public License 2.0 (MPL-2.0)
