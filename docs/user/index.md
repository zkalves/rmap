# rmap Documentation

Welcome to the documentation for **rmap**, an open-source tool for hardware register map authoring, verification model synthesis (UVM SystemVerilog), and firmware header generation (C/C++).

---

## Documentation Sections

- [🚀 Getting Started](getting-started.html) — Installation, building, and basic workflow.
- [🖥️ GUI User Guide](gui-guide.html) — Visual design, bitfield packing, validation, and context menus.
- [⚙️ CLI Reference & Automation](cli-reference.html) — Headless batch generation for CI/CD and Makefiles.
- [📝 Templates & Code Generation](templates-and-codegen.html) — Writing Inja templates, using helper functions, and custom variables.
- [🏛️ Architecture & Internals](architecture.html) — Data models, Protobuf serialization, and validation engine.

---

## Core Capabilities

- **Dual-Pane Interface**: Hierarchy navigation for Blocks and Registers on the left, with bitfield editing on the right.
- **Real-Time Validation**: Instant detection of register address collisions, bitfield overlaps, and bit-width boundary overflows.
- **Headless CLI Generation**: Seamlessly generate C headers and UVM models directly in Makefiles and CI/CD pipelines without launching a GUI.
- **Flexible Code Generation**: Built with [Pantor Inja](https://github.com/pantor/inja) and [nlohmann/json](https://github.com/nlohmann/json). Easily customize or write templates.
- **Multi-Format Storage**: Robust Google Protocol Buffers storage supporting human-readable text (`.rmt`) and high-performance binary (`.rmb`) formats.
- **UVM Access Policies**: Native support for all 9 UVM register access policies (`RW`, `RO`, `WO`, `W1C`, `W0C`, `RC`, `RS`, `W1S`, `W0S`).
- **Flexible Number Formatting**: Input offsets, sizes, and reset values seamlessly using Hex (`0x`), Decimal, or Binary (`0b`).

---

## Quick Example

```bash
# Build the application
make

# Run automated tests
make test

# Generate C header and UVM models headlessly from an existing map
./build/bin/rmap -f examples/spi.rmt --export --out ./work
```
