<!--
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at https://mozilla.org/MPL/2.0/.

  Copyright (c) 2026 Ezequiel Alves. All rights reserved.
-->

# rmap Register Map Examples & Autonomous Environments

This directory contains curated reference register maps, multi-format exchange models, and **autonomous simulation and compilation environments** demonstrating **rmap**'s capabilities, data modeling, format support, code generation, and verification workflows.

---

## Directory Overview

```
examples/
├── README.md                 # This comprehensive documentation guide
├── Makefile                  # Automated runner executing all example environments (make test-examples)
│
├── environments/             # Self-contained simulation and compilation environments
│   ├── spi/                  # SPI controller build & simulation environment
│   ├── uart/                 # UART controller build & simulation environment
│   ├── dma/                  # DMA controller build & simulation environment
│   ├── sensor_hub/           # Sensor hub build & simulation environment
│   ├── comprehensive/        # All 9 SW access policies & HW modes environment
│   ├── wide_bus_64bit/       # 64-bit wide register bus environment
│   ├── address_gap_example/  # Address gap padding & alignment environment
│   ├── map_node/             # Hierarchical memory map & bus domain environment
│   ├── custom_parameters/    # Key-value peripheral customization parameters environment
│   ├── project_metadata/     # Project metadata propagation environment
│   ├── strict_validation/    # Strict architectural linting environment
│   ├── python_post_generate/ # Post-generation hook automation environment
│   ├── template_folders/     # Multi-folder template search environment
│   ├── diff/                 # Semantic register map diff environment
│   ├── advanced_ral/         # Advanced UVM RAL constructs (indirect regs, FIFOs, callbacks, test exclusions)
│   ├── format_conversion/    # Headless cross-conversion across SVD, SystemRDL, IP-XACT, JSON, CSV, and Protobuf
│   └── soc_large_scale/      # Full SoC-scale multi-block subsystem environment
│
├── rmt/                      # Native Protocol Buffer register maps (.rmt text & .rmb binary)
│   ├── peripherals/          # Standard peripheral controller models
│   │   ├── spi.rmt / spi.rmb # Standard SPI peripheral controller (Protobuf text & binary)
│   │   ├── uart.rmt          # UART peripheral controller register map
│   │   ├── dma.rmt           # Direct Memory Access (DMA) channel controller
│   │   └── sensor_hub.rmt    # Multi-sensor telemetry hub register map
│   ├── features/             # Dedicated feature coverage & architectural patterns
│   │   ├── soc_large_scale.rmt / .rmb # Large-scale SoC with multiple blocks, maps, memories & registers
│   │   ├── comprehensive.rmt # Comprehensive feature coverage (all 9 access policies, memory, booleans)
│   │   ├── wide_bus_64bit.rmt# 64-bit data bus architecture example
│   │   ├── address_gap_example.rmt# Non-contiguous address gaps & alignment padding
│   │   ├── map_node.rmt      # Hierarchical memory mapping with uvm_reg_map domains
│   │   ├── custom_parameters.rmt# Peripheral configuration key-value parameters
│   │   ├── project_metadata.rmt# Project name, version, author, and license fields
│   │   ├── strict_validation.rmt# Strict linting compliant register map
│   │   ├── python_post_generate.rmt# Automated post-generation execution hook
│   │   ├── template_folders.rmt# Multi-directory template discovery configuration
│   │   ├── advanced_ral.rmt / .rmb # Advanced UVM RAL constructs (indirect, FIFO, callbacks, test directives)
│   │   ├── diff_v1.rmt       # Semantic diff base version
│   │   └── diff_v2.rmt       # Semantic diff modified version (additions, deletions, mutations)
│   └── validation/           # Negative test cases & linter rule validation
│       └── invalid_overlap.rmt# Validation test cases (address collisions, field overlaps, width overflow)
│
├── scripts/                  # Helper automation scripts
│   └── post_generate.py      # Sample post-generation Python processing hook
│
└── Multi-Format Reference Files
    ├── csv/
    │   └── dma_controller.csv# RFC 4180 tabular register map representation
    ├── ipxact/
    │   └── spi_ipxact.xml    # IEEE 1685-2014 IP-XACT XML peripheral schema
    ├── json/
    │   └── sensor_hub.json   # Structured JSON register map schema export
    ├── svd/
    │   └── stm32_uart.svd    # ARM CMSIS-SVD Cortex-M peripheral description XML
    └── systemrdl/
        └── atxmega_spi.rdl   # Accellera SystemRDL 2.0 register model specification
```

---

## Example Descriptions

### 1. Peripheral Controllers (`rmt/peripherals/`)

- **`spi.rmt` & `spi.rmb`**: Standard 32-bit peripheral (SPI Controller) with `CTRL`, `STATUS`, `DATA`, and `CLK_DIV` registers. Demonstrates standard peripheral register design with `RW`, `RO`, and `W1C` access policies, hardware sideband indicators, and reset values. Available in both human-readable text Protobuf (`.rmt`) and compact binary Protobuf (`.rmb`).
- **`uart.rmt`**: UART peripheral controller register map with baud rate, control, and interrupt flags.
- **`dma.rmt`**: Direct Memory Access (DMA) channel controller with source/destination addresses and transfer lengths.
- **`sensor_hub.rmt`**: Multi-sensor telemetry hub register map with calibrated sensor readings and thresholds.

### 2. Architectural Features (`rmt/features/`)

- **`soc_large_scale.rmt` & `soc_large_scale.rmb`**: A complete System-on-Chip (SoC) architecture demonstrating large-scale hardware modeling:
  - **4 Functional Subsystem Blocks**: `CPU_SUBSYSTEM` (0x00000), `DMA_CONTROLLER` (0x10000), `SECURITY_ENGINE` (0x20000), `PERIPHERAL_BRIDGE` (0x30000).
  - **3 Memory Map Domains**: `AXI_SYSTEM_MAP` (64-bit base 0x8000_0000), `APB_PERIPHERAL_MAP` (32-bit base 0x4000_0000), and `DEBUG_JTAG_MAP` (32-bit base 0x0000_0000).
  - **3 Dedicated Embedded Memories**: `INSTRUCTION_SRAM` (64 KB, 32-bit word, RO), `DATA_TCM` (128 KB, 64-bit word, RW), and `PACKET_BUFFER` (16 KB, 8-bit word, RW).
  - **40 Individual Registers**: Covering all 9 Software Access Policies (`RW`, `RO`, `WO`, `W1C`, `W1S`, `W0C`, `RC`, `RS`, `NA`), Hardware Access Modes (`RO`, `RW`, `WO`, `W1S`, `W1C`, `NA`), mixed radices (Hexadecimal `0x`, Binary `0b`, Decimal), volatile and randomization flags, address gaps, and custom parameters.
- **`comprehensive.rmt`**: Exhaustive feature coverage:
  - All 9 Software Access Policies (`RW`, `RO`, `WO`, `W1C`, `W1S`, `W0C`, `RC`, `RS`, `NA`).
  - Core Hardware Access Modes (`RO`, `RW`, `WO`, `W1S`, `W1C`, `NA`).
  - Dedicated memory blocks (`mem` kind) with custom byte/word sizes.
  - Number radix representations (Hexadecimal `0x`, Decimal, Binary `0b`).
  - Flags: `is_rand` (UVM randomization), `volatile` (firmware volatile qualifier), `has_reset`.
- **`wide_bus_64bit.rmt`**: High-performance 64-bit data bus architecture configured with global `reg_width: 64`. Demonstrates 64-bit register alignment, wide bitfield slicing, and `uint64_t` firmware header generation.
- **`address_gap_example.rmt`**: Non-contiguous register offsets with deliberate address gaps (e.g. offset `0x0000` followed by `0x0010` and `0x0040`). Demonstrates automatic reserved word generation (`uint32_t _reserved_[...]`) and GUI gap detection visualization.
- **`map_node.rmt`**: Multi-bus hierarchical memory mapping architecture showcasing `map` kind nodes with base offsets, addressing spaces, and sub-blocks.
- **`custom_parameters.rmt`**: Demonstrates peripheral customization via key-value parameters (`VERSION_ID`, `TIMEOUT_CYCLES`, `BURST_LEN`, `FIFO_DEPTH`, `ENABLE_ECC`) propagated to code generation templates.
- **`project_metadata.rmt`**: Project-level metadata configuration (`project_name`, `project_version`, `project_author`, `project_license`) in header guards, doc banners, and package files.
- **`strict_validation.rmt`**: Strict lint-compliant register map adhering to full address alignment, exhaustive descriptions on blocks, registers, and fields, and valid non-overlapping bitfields.
- **`python_post_generate.rmt`**: Integration with automated post-generation processing hooks (`examples/scripts/post_generate.py`).
- **`template_folders.rmt`**: Multi-directory template discovery and custom template search paths (`custom_templates/`).
- **`advanced_ral.rmt` & `advanced_ral.rmb`**: Advanced UVM RAL constructs derived from the UVM Cookbook:
  - Indirect addressing registers (`uvm_reg_indirect_data` via index register pointer `CFG_INDEX`).
  - Hardware streaming FIFO registers (`uvm_reg_fifo` with custom FIFO depth).
  - Register callback hooks (`STATUS_CBS_cbs` extending `uvm_reg_cbs` with pre/post read/write hooks).
  - Test sequence exclusion directives (`NO_REG_TEST`, `NO_REG_HW_RESET_TEST`, `NO_REG_BIT_BASH_TEST`, `NO_MEM_TEST`, `NO_MEM_WALK_TEST`, `NO_MEM_ACCESS_TEST`).
  - Embedded functional coverage models (`val_cg` on register values, access covergroup on address map).
- **`diff_v1.rmt` & `diff_v2.rmt`**: Semantic register map comparison reference pair demonstrating block, register, and field additions, deletions, offset mutations, and field property changes.

### 3. Validation & Linting (`rmt/validation/`)

- **`invalid_overlap.rmt`**: Intentionally contains design flaws to verify rmap's real-time validation engine:
  - Register address overlap collisions.
  - Field bit range collisions within a register.
  - Field offset exceeding register bit width.

---

### 4. External Format Reference Models

| Directory | File | Format Standard | Description |
| :--- | :--- | :--- | :--- |
| **`csv/`** | `dma_controller.csv` | RFC 4180 CSV | Spreadsheet-friendly tabular register map format. |
| **`ipxact/`** | `spi_ipxact.xml` | IEEE 1685-2014 / 2022 | Standardized IP-XACT XML peripheral component model. |
| **`json/`** | `sensor_hub.json` | JSON Schema | Standardized JSON register map exchange format. |
| **`svd/`** | `stm32_uart.svd` | ARM CMSIS-SVD | ARM Cortex-M peripheral description XML. |
| **`systemrdl/`** | `atxmega_spi.rdl` | Accellera SystemRDL 2.0 | Accellera register description language specification. |

---

## Autonomous Simulation & Compilation Environments (`environments/`)

Every feature and peripheral model includes a dedicated, self-contained execution environment in `examples/environments/<name>/`. Each environment provides an autonomous `Makefile` alongside real test harnesses for C firmware, Rust PAC crates, and Python bring-up scripts.

### Standard Makefile Targets Across All Environments

| Target | Description | Tool Requirements |
| :--- | :--- | :--- |
| `make all` | Runs `export`, `validate`, and all available compilation and simulation targets | GCC or Clang, Python 3 |
| `make export` | Headlessly generates all configured code generation templates using `rmap -e` | Built `rmap` binary |
| `make validate` | Runs strict linter validation checks using `rmap -l --strict` | Built `rmap` binary |
| `make compile-c` | Compiles and executes the C firmware harness (`test_harness.c`) | GCC or Clang |
| `make compile-rust`| Compiles and runs the Rust PAC module harness (`test_harness.rs`) | `rustc` (gracefully skipped if absent) |
| `make run-python` | Executes the Python driver verification harness (`test_harness.py`) | Python 3 |
| `make sim-rtl` | Runs synthesizable SystemVerilog register slice simulation | Icarus Verilog (`iverilog`) |
| `make sim-verilator`| Compiles and executes cycle-accurate C++ simulation model | `verilator` |
| `make sim-pyuvm` | Runs Python PyUVM functional verification testbench | `cocotb-config` & `iverilog` |
| `make sim-uvm` | Executes IEEE 1800.2 standard SystemVerilog UVM verification suite | Cadence Xcelium, Synopsys VCS, or Siemens Questa |
| `make clean` | Cleans up all generated outputs and build artifacts | None |

### Running Environments

```bash
# Run autonomous verification across all 17 environments simultaneously
make test-examples

# Or run from the examples directory:
make -C examples all

# Run an individual environment directly (e.g. SPI):
cd examples/environments/spi
make all

# Run only C firmware compilation and testing:
cd examples/environments/comprehensive
make compile-c

# Run strict architectural validation:
cd examples/environments/strict_validation
make validate

# Clean an environment:
cd examples/environments/spi
make clean
```

---

## CLI Usage Recipes

```bash
# Open SPI register map in GUI
./build/bin/rmap -f examples/rmt/peripherals/spi.rmt

# Headless export of all templates to an output directory
./build/bin/rmap -f examples/rmt/peripherals/spi.rmt --export --out ./work

# Convert SystemRDL to ARM CMSIS-SVD
./build/bin/rmap -f examples/systemrdl/atxmega_spi.rdl --convert spi.svd

# Convert CSV table to Protobuf text
./build/bin/rmap -f examples/csv/dma_controller.csv --convert dma.rmt

# Run strict architectural linter validation
./build/bin/rmap -f examples/rmt/features/strict_validation.rmt --lint --strict

# Perform semantic diff between two register map revisions
./build/bin/rmap -f examples/rmt/features/diff_v1.rmt --diff examples/rmt/features/diff_v2.rmt
```
