# rmap Documentation Portal

Welcome to the **rmap** documentation portal. **rmap** is a high-performance GUI & CLI application built with Modern C++ (C++17) and Qt 6 for designing hardware register maps, conducting real-time architectural validation, and generating register models across the full hardware and software lifecycle.

---

## 📚 Documentation Navigation

```text
docs/
├── user/                       # User Guide & Operational Manuals (Primary Source of Truth)
│   ├── architecture.md         # System architecture, access policies, format matrix, dynamic widths
│   ├── templates-and-codegen.md# Inja template catalog (RTL SV/Verilog/VHDL, UVM, C, Rust), helpers
│   ├── cli-reference.md        # Headless generation, CI/CD linting (SARIF/JUnit), semantic diff
│   ├── gui-guide.md            # Interactive GUI walkthrough, bitfield visualizer, themes
│   └── getting-started.md      # Installation, compilation, and quickstart
│
└── dev/                        # Developer & C++ Architecture Guide (Doxygen Flow)
    └── index.md                # Subsystem architecture overview & Doxygen API portal
```

---

## 🚀 User Guides

- [**Getting Started**](user/getting-started.md): Installation prerequisites, building from source with CMake / Make, and basic workflow.
- [**GUI User Guide**](user/gui-guide.md): Dual-pane tree navigation, bitfield visualizer widget, 1-click fast editing, and keyboard shortcuts (`F1`).
- [**CLI Reference & Automation**](user/cli-reference.md): Headless batch generation (`--export`), format conversions (`--convert`), automated CI linter (`--lint`), and semantic diffing (`--diff`).
- [**Templates & Code Generation**](user/templates-and-codegen.md): Inja templating engine, custom callbacks (`upper`, `lower`, `camel_case`, `pascal_case`, `snake_case`, `c_type`, `bitmask`, `to_hex`), and exported JSON data schema.
- [**System Architecture**](user/architecture.md): Full data model, 11-column tree structure, access policy definitions, and format specifications.

---

## 🛠️ Developer & C++ API Reference

- [**C++ Subsystem Architecture Guide**](dev/index.md): Architectural walkthrough of rmap's five core C++ subsystems (Controllers, Models, Visualizers, Formats, and Utilities).
- [**C++ Class Reference (API Documentation)**](annotated.html): Complete annotated index of all C++ classes, structs, signals, slots, and methods.
- [**C++ Class Inheritance Hierarchy**](hierarchy.html): Graphical tree of inheritance across all Qt models, widgets, and handlers.
- [**Source Code File Directory**](files.html): Complete browsable directory of all C++ source files, headers, and syntax-highlighted code.
- [**Global Functions & Constants**](globals.html): Global functions, enums, type definitions, and preprocessor definitions.



---

## 📖 Offline Reference Manuals (PDF)

- [**rmap User Manual (PDF)**](https://zkalves.github.io/rmap/pdf/rmap_user_manual.pdf): Complete 50-page user guide covering installation, interactive GUI operation, bitfield visualizer, CLI headless automation, Inja templating, and verification environments.
- [**rmap Developer Guide (PDF)**](https://zkalves.github.io/rmap/pdf/rmap_developer_guide.pdf): In-depth 77-page C++ architecture and API reference detailing Qt 6 models, custom delegates, visualizer widgets, multi-format registry, undo/redo stack, and serialization.

---

## ⚡ Quick Start

```bash
# Build the application
make

# Run all automated test suites
make test

# Launch GUI with sample SPI peripheral
./build/bin/rmap -f examples/rmt/peripherals/spi.rmt

# Headless batch code generation
./build/bin/rmap -f examples/rmt/peripherals/spi.rmt --export --out ./work
```

---

## 👤 Author & Project Repository

- **Author**: Ezequiel Alves ([@@zkalves](https://github.com/zkalves))
- **Email**: [alvesel@gmail.com](mailto:alvesel@gmail.com)
- **GitHub Page**: [https://github.com/zkalves/rmap](https://github.com/zkalves/rmap)
- **Documentation Portal**: [https://zkalves.github.io/rmap/](https://zkalves.github.io/rmap/)
- **License**: Mozilla Public License 2.0 (MPL-2.0)
