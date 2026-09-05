# rmap Documentation Portal

Welcome to the **rmap** documentation portal. **rmap** is a high-performance GUI & CLI application built with Modern C++ (C++17) and Qt 6 for designing hardware register maps, conducting real-time architectural validation, and generating register models across the full hardware and software lifecycle.

---

## 📚 Documentation Navigation

```
docs/
├── user/                       # User Guide & Operational Manuals
│   ├── getting-started.md      # Installation, compilation, and quickstart
│   ├── gui-guide.md            # Interactive GUI walkthrough, bitfield visualizer, themes
│   ├── cli-reference.md        # Headless generation, CI/CD linting (SARIF/JUnit), semantic diff
│   ├── templates-and-codegen.md# Inja template syntax, custom helpers, JSON schema
│   └── architecture.md         # System architecture and multi-format engine
│
└── dev/                        # Developer & C++ API Architecture Reference
    ├── index.md                # C++ class reference index
    ├── RegMapWindow.md         # Main window controller and UI orchestration
    ├── RegMapTreeModel.md      # Hierarchical QAbstractItemModel data model
    ├── RegMapTreeItem.md       # Tree node hierarchy and serialization
    ├── RegBitfieldBarWidget.md # Interactive 32/64-bit continuous slice visualizer
    ├── BlockMemoryMapWidget.md # Stacked memory map diagram with gap detection
    ├── CodeGenerator.md        # Pantor Inja code generator engine
    ├── FormatManager.md        # Multi-format serializer / deserializer registry
    ├── ThemeManager.md         # Multi-theme engine and CVD accessible palettes
    ├── PathUtils.md            # Path resolution and environment variable expansion
    └── SerializationContext.md # Object graph persistence framework
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

- [**C++ Developer Documentation Index**](dev/index.md): Complete index of classes, widgets, delegates, format handlers, and core utilities.
- [**Application Entry Point (`src/main.cpp`)](dev/rmap.md): Startup sequence, headless auto-detection, and command-line argument parsing.
- [**Main Window Controller (`RegMapWindow`)](dev/RegMapWindow.md): UI coordination, dock widgets, undo/redo stack, and headless handlers.
- [**Tree Model (`RegMapTreeModel`)](dev/RegMapTreeModel.md): 11-column tree model, invalid cell tracking, and real-time validation.
- [**Interactive Bitfield Visualizer (`RegBitfieldBarWidget`)](dev/RegBitfieldBarWidget.md): 32/64-bit continuous slice rendering, reserved slot hatching, and bidirectional selection.
- [**Stacked Memory Map (`BlockMemoryMapWidget`)](dev/BlockMemoryMapWidget.md): Vertical memory map diagram with unmapped gap detection.
- [**Template Engine (`CodeGenerator`)](dev/CodeGenerator.md): Pantor Inja integration, helper callbacks, and multi-source code generation.
- [**Format Registry (`FormatManager`)](dev/FormatManager.md): Handlers for SystemRDL, IP-XACT, CMSIS-SVD, JSON, CSV, and Protobuf.
- [**Theme Engine (`ThemeManager`)](dev/ThemeManager.md): Palettes for Solarized 8, Nord, Dracula, Monokai, Classic, and CVD Barrier-Free mode.

---

## ⚡ Quick Start

```bash
# Build the application
make

# Run all 11 automated test suites
make test

# Launch GUI with sample SPI peripheral
./build/bin/rmap -f examples/rmt/peripherals/spi.rmt

# Headless batch code generation
./build/bin/rmap -f examples/rmt/peripherals/spi.rmt --export --out ./work
```
