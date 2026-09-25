# rmap Documentation Portal

Welcome to the **rmap** documentation portal. **rmap** is a high-performance GUI & CLI application built with Modern C++ (C++17) and Qt 6 for designing hardware register maps, conducting real-time architectural validation, and generating register models across the full hardware and software lifecycle.

---

## 📚 Documentation Navigation

```text
docs/
├── user/                       # User Guide & Operational Manuals (Primary Source of Truth)
│   ├── index.md                # User Guide top-level hub & role-based roadmaps
│   ├── getting-started.md      # Installation, compilation, and quickstart
│   ├── supported-os.md         # Supported Linux distributions, glibc baselines, pre-built packages
│   ├── gui-guide.md            # Interactive GUI walkthrough, bitfield visualizer, themes
│   ├── cli-reference.md        # Headless generation, CI/CD linting (SARIF/JUnit), semantic diff
│   ├── templates-and-codegen.md# Inja template catalog (RTL SV/Verilog/VHDL, UVM, C, Rust), helpers
│   └── architecture.md         # System architecture, access policies, format matrix, dynamic widths
│
└── dev/                        # Developer & C++ Architecture Guide (Doxygen Flow)
    ├── index.md                # Developer Guide top-level hub & build targets
    ├── architecture.md         # C++ subsystem architecture, data model, and visualizers
    ├── standards.md            # Coding standards & Bidirectional Lockstep Invariants
    └── coverage.md             # Compiler test coverage metrics & quality gates
```

---

## 🧭 Documentation Portals

- @subpage user_guide "📘 rmap User Guide"
  Comprehensive operational manual for hardware engineers (RTL & verification) and automation teams. Contains role-based learning roadmaps and complete guides for interactive GUI editing, continuous bitfield visualizers, headless CLI automation, Inja templates, RAL code generation, and register map access policies.

- @subpage dev_guide "🛠️ rmap Developer Guide"
  Comprehensive technical documentation for developers contributing to or extending **rmap**. Contains the Modern C++17 / Qt 6 subsystem architecture, model-view contracts, format engine, development environment, coding standards, test coverage infrastructure, and automated Doxygen C++ API reference.

---

## 📖 Offline Reference Manuals (PDF)

- [**rmap User Manual (PDF)**](pdf/rmap_user_manual.pdf): Complete 50-page user guide covering installation, interactive GUI operation, bitfield visualizer, CLI headless automation, Inja templating, and verification environments.
- [**rmap Developer Guide (PDF)**](pdf/rmap_developer_guide.pdf): In-depth 77-page C++ architecture and API reference detailing Qt 6 models, custom delegates, visualizer widgets, multi-format registry, undo/redo stack, and serialization.

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
