# rmap User Guide {#user_guide}

Welcome to the **rmap** User Guide. This guide provides comprehensive documentation for designing, validating, simulating, and generating hardware register maps across interactive GUI and headless CLI workflows.

---

## Recommended Learning Roadmaps

Depending on your role and operational workflow, we recommend following these reading paths:

- **🖥️ Hardware Engineers (RTL & Verification)**:
  1. [Getting Started](@ref getting_started) &mdash; Installation, building, and basic workflow.
  2. [Supported Operating Systems](@ref supported_os) &mdash; OS compatibility matrix and packages.
  3. [GUI User Guide](@ref gui_guide) &mdash; Interactive visual design, 32/64-bit continuous bitfield packing visualizer, and shortcuts.
  4. [System Architecture](@ref architecture) &mdash; Core register map data model, 24 UVM access policies, HW arbitration, dynamic bus widths, and multi-bus address maps.
  5. [Templates & Code Generation](@ref templates_codegen) &mdash; Inja templating engine, synthesizable RTL (Verilog, SystemVerilog, VHDL), UVM RAL models, firmware headers (C, Rust, Python), and verification testbenches.

- **⚙️ EDA & CI/CD Automation Engineers**:
  1. [Getting Started](@ref getting_started) &mdash; Offscreen execution setup.
  2. [CLI Reference & Automation](@ref cli_reference) &mdash; Headless batch code generation (`--export`), multi-format conversions (`--convert`), automated CI linter (`--lint`), and semantic diffing (`--diff`).
  3. [System Architecture](@ref architecture) &mdash; Multi-format interoperability matrix (SVD, SystemRDL, IP-XACT, JSON, CSV, Protobuf).
  4. [Templates & Code Generation](@ref templates_codegen) &mdash; Template catalog, custom variables, and headless export integration.

---

## User Guide Chapters

- @subpage getting_started "Getting Started"
  Prerequisites, toolchain dependencies, building from source with CMake/Make, and initial verification.

- @subpage supported_os "Supported Operating Systems & Compatibility Matrix"
  Linux distribution compatibility matrix, glibc baselines, and pre-built packages (AppImage, DEB, RPM).

- @subpage gui_guide "GUI User Guide"
  Dual-pane layout, 32/64-bit continuous bitfield visualizer, stacked memory map, inline validation, and shortcuts (`F1`).

- @subpage cli_reference "CLI Reference & Automation"
  Headless batch generation, automated CI linter (SARIF, JUnit XML), semantic register map diffing, and format conversion.

- @subpage templates_codegen "Templates & Code Generation"
  Inja template engine, built-in deliverables (SystemVerilog, Verilog 2001, VHDL, UVM, C, Rust, Python), custom helpers, and JSON context.

- @subpage architecture "Architecture & Internals"
  Core data model, 11-column hierarchy, 24 UVM access policies, HW arbitration, dynamic bus widths, and multi-format capabilities.
