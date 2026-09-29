<!--
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at https://mozilla.org/MPL/2.0/.

  Copyright (c) 2026 Ezequiel Alves. All rights reserved.
-->

# rmap Register Map Examples & Autonomous Environments

This directory contains curated reference register maps, multi-format exchange models, and **autonomous simulation and compilation environments** demonstrating **rmap**'s capabilities, data modeling, format support, code generation, and verification workflows.

Every example in this directory is **fully self-contained**: each example directory contains its own register map file(s), test harnesses (C firmware, Python driver, Rust PAC), and an autonomous `Makefile` requiring no external project files other than the `rmap` tool and template catalog.

---

## Directory Overview

```text
examples/
├── README.md                 # This comprehensive documentation guide
├── Makefile                  # Automated runner executing all 20 examples (make -C examples all)
│
├── peripherals/              # Self-contained standard peripheral controller examples
│   ├── spi/                  # SPI controller (spi.rmt, spi.rmb, Makefile, test harnesses)
│   ├── uart/                 # UART controller (uart.rmt, Makefile, test harnesses)
│   ├── dma/                  # DMA channel controller (dma.rmt, Makefile, test harnesses)
│   └── sensor_hub/           # Telemetry sensor hub (sensor_hub.rmt, Makefile, test harnesses)
│
└── features/                 # Self-contained architectural feature examples
    ├── address_gap_example/  # Address gap padding & alignment (address_gap_example.rmt, Makefile, harnesses)
    ├── advanced_ral/         # Advanced UVM RAL constructs (advanced_ral.rmt, advanced_ral.rmb, Makefile, harnesses)
    ├── comprehensive/        # All 24 IEEE 1800.2 SW access policies & HW modes (comprehensive.rmt, Makefile, harnesses)
    ├── custom_parameters/    # Key-value peripheral customization parameters (custom_parameters.rmt, Makefile, harnesses)
    ├── diff/                 # Semantic register map diff (diff_v1.rmt, diff_v2.rmt, Makefile)
    ├── format_conversion/    # Headless cross-conversion (SVD, SystemRDL, IP-XACT, JSON, CSV, Makefile)
    ├── hw_precedence/        # Configurable SW/HW arbitration precedence (hw_precedence.rmt, Makefile, harnesses)
    ├── map_node/             # Hierarchical memory map & bus domain (map_node.rmt, Makefile, harnesses)
    ├── narrow_bus_8bit/      # 8-bit narrow register bus architecture (narrow_bus_8bit.rmt, Makefile, harnesses)
    ├── project_metadata/     # Project metadata propagation (project_metadata.rmt, Makefile, harnesses)
    ├── python_post_generate/ # Post-generation hook automation (python_post_generate.rmt, post_generate.py, Makefile, harnesses)
    ├── soc_large_scale/      # Full SoC-scale multi-block subsystem (soc_large_scale.rmt, soc_large_scale.rmb, post_generate.py, Makefile, harnesses)
    ├── software_locks/       # 3-level hierarchical software locks (software_locks.rmt, Makefile, harnesses)
    ├── strict_validation/    # Strict architectural linting & negative test cases (strict_validation.rmt, invalid_*.rmt, Makefile, harnesses)
    ├── template_folders/     # Multi-folder template search (template_folders.rmt, Makefile, harnesses)
    └── wide_bus_64bit/       # 64-bit wide register bus (wide_bus_64bit.rmt, Makefile, harnesses)
```

---

## Example Descriptions

### 1. Peripheral Controllers (`peripherals/`)

- **`spi/` (`spi.rmt`, `spi.rmb`)**: Standard 32-bit peripheral (SPI Controller) with `CTRL`, `STATUS`, `DATA`, and `CLK_DIV` registers. Demonstrates standard peripheral register design with `RW`, `RO`, and `W1C` access policies, hardware sideband indicators, and reset values. Available in both human-readable text Protobuf (`.rmt`) and compact binary Protobuf (`.rmb`).
- **`uart/` (`uart.rmt`)**: UART peripheral controller register map with baud rate, control, and interrupt flags.
- **`dma/` (`dma.rmt`)**: Direct Memory Access (DMA) channel controller with source/destination addresses and transfer lengths.
- **`sensor_hub/` (`sensor_hub.rmt`)**: Multi-sensor telemetry hub register map with calibrated sensor readings and thresholds.

### 2. Architectural Features (`features/`)

- **`soc_large_scale/` (`soc_large_scale.rmt`, `soc_large_scale.rmb`, `post_generate.py`)**: A complete System-on-Chip (SoC) architecture demonstrating large-scale hardware modeling:
  - **4 Functional Subsystem Blocks**: `CPU_SUBSYSTEM` (0x00000), `DMA_CONTROLLER` (0x10000), `SECURITY_ENGINE` (0x20000), `PERIPHERAL_BRIDGE` (0x30000).
  - **3 Memory Map Domains**: `AXI_SYSTEM_MAP` (64-bit base 0x8000_0000), `APB_PERIPHERAL_MAP` (32-bit base 0x4000_0000), and `DEBUG_JTAG_MAP` (32-bit base 0x0000_0000).
  - **3 Dedicated Embedded Memories**: `INSTRUCTION_SRAM` (64 KB, 32-bit word, RO), `DATA_TCM` (128 KB, 64-bit word, RW), and `PACKET_BUFFER` (16 KB, 8-bit word, RW).
  - **40 Individual Registers**: Covering all 9 Software Access Policies (`RW`, `RO`, `WO`, `W1C`, `W1S`, `W0C`, `RC`, `RS`, `NA`), Hardware Access Modes (`RO`, `RW`, `WO`, `W1S`, `W1C`, `NA`), mixed radices (Hexadecimal `0x`, Binary `0b`, Decimal), volatile and randomization flags, address gaps, and custom parameters.
- **`comprehensive/` (`comprehensive.rmt`)**: Exhaustive feature coverage:
  - All 24 IEEE 1800.2 Software Access Policies (`RW`, `RO`, `WO`, `W1`, `WO1`, `W1C`, `W1S`, `W1T`, `W0C`, `W0S`, `W0T`, `RC`, `RS`, `WRC`, `WRS`, `WC`, `WS`, `W1SRC`, `W1CRS`, `W0SRC`, `W0CRS`, `WOC`, `WOS`, `NOACCESS`).
  - Core Hardware Access Modes (`RO`, `RW`, `WO`, `W1S`, `W1C`, `NA`).
  - All 20 Code Generation Template Deliverables (24 inja template files) generated simultaneously.
  - Dedicated memory blocks (`mem` kind) with custom byte/word sizes.
  - Number radix representations (Hexadecimal `0x`, Decimal, Binary `0b`).
  - Flags: `is_rand` (UVM randomization), `volatile` (firmware volatile qualifier), `has_reset`.
- **`hw_precedence/` (`hw_precedence.rmt`)**: Configurable write arbitration precedence configured with `hw_precedence: false`. Demonstrates software-precedence mode (`PARAM_HW_PRECEDENCE = 0` / `'0'`) across SystemVerilog, Verilog-2001, VHDL, APB4, AXI4-Lite, and formal SVA.
- **`software_locks/` (`software_locks.rmt`)**: Demonstrates the **3-level hierarchical software locking architecture** across blocks, registers, and individual bitfields:
  - Block-level independent software write and read locks (`[w] hw_tamper_lock_i; [r] hw_debug_lock_i`).
  - Register-level composite write locks (`hw_sec_lock_i || (SEC_CTRL.LOCK_BIT == 1)`) and read locks (`[r] hw_read_lock_i`).
  - Dual software read and write locks (`[rw] hw_crypto_lock_i`).
  - Field-level selective locking within a single register: public unlocked status field alongside write-locked, read-locked, and dual-locked bitfields with selective bit-slice masking and strobe suppression.
- **`narrow_bus_8bit/` (`narrow_bus_8bit.rmt`)**: 8-bit narrow data bus architecture configured with global `reg_width: 8`. Demonstrates byte-aligned registers (`uint8_t` in C firmware), 8-bit RTL bus interface (`DATA_WIDTH=8`, `STRB_WIDTH=1`), and byte-slicing bitfield operations.
- **`wide_bus_64bit/` (`wide_bus_64bit.rmt`)**: High-performance 64-bit data bus architecture configured with global `reg_width: 64`. Demonstrates 64-bit register alignment, wide bitfield slicing, and `uint64_t` firmware header generation.
- **`address_gap_example/` (`address_gap_example.rmt`)**: Non-contiguous register offsets with deliberate address gaps (e.g. offset `0x0000` followed by `0x0010` and `0x0040`). Demonstrates automatic reserved word generation (`uint32_t _reserved_[...]`) and GUI gap detection visualization.
- **`map_node/` (`map_node.rmt`)**: Multi-bus hierarchical memory mapping architecture showcasing `map` kind nodes with base offsets, addressing spaces, and sub-blocks.
- **`custom_parameters/` (`custom_parameters.rmt`)**: Demonstrates peripheral customization via key-value parameters (`VERSION_ID`, `TIMEOUT_CYCLES`, `BURST_LEN`, `FIFO_DEPTH`, `ENABLE_ECC`) propagated to code generation templates.
- **`project_metadata/` (`project_metadata.rmt`)**: Project-level metadata configuration (`project_name`, `project_version`, `project_author`, `project_license`) in header guards, doc banners, and package files.
- **`strict_validation/` (`strict_validation.rmt`, `invalid_*.rmt`)**: Strict lint-compliant register map adhering to full address alignment, exhaustive descriptions on blocks, registers, and fields, and valid non-overlapping bitfields. Also self-contains the negative validation test suite:
  - `invalid_overlap.rmt`: Address collisions and bit range overlaps.
  - `invalid_keyword.rmt`: Reserved C/SystemVerilog keyword identifier collisions.
  - `invalid_reset_overflow.rmt`: Field reset values exceeding bit width capacity.
  - `invalid_blackhole.rmt`: Contradictory access policy validation (`SW=WO` and `HW=WO`).
- **`python_post_generate/` (`python_post_generate.rmt`, `post_generate.py`)**: Integration with automated post-generation processing hooks executing `post_generate.py`.
- **`template_folders/` (`template_folders.rmt`)**: Multi-directory template discovery and custom template search paths (`custom_templates/`).
- **`advanced_ral/` (`advanced_ral.rmt`, `advanced_ral.rmb`)**: Advanced UVM RAL constructs derived from the UVM Cookbook:
  - Indirect addressing registers (`uvm_reg_indirect_data` via index register pointer `CFG_INDEX`).
  - Hardware streaming FIFO registers (`uvm_reg_fifo` with custom FIFO depth).
  - Register callback hooks (`STATUS_CBS_cbs` extending `uvm_reg_cbs` with pre/post read/write hooks).
  - Test sequence exclusion directives (`NO_REG_TEST`, `NO_REG_HW_RESET_TEST`, `NO_REG_BIT_BASH_TEST`, `NO_MEM_TEST`, `NO_MEM_WALK_TEST`, `NO_MEM_ACCESS_TEST`).
  - Embedded functional coverage models (`val_cg` on register values, access covergroup on address map).
- **`diff/` (`diff_v1.rmt`, `diff_v2.rmt`)**: Semantic register map comparison reference pair demonstrating block, register, and field additions, deletions, offset mutations, and field property changes.
- **`format_conversion/` (`stm32_uart.svd`, `atxmega_spi.rdl`, `spi_ipxact.xml`, `sensor_hub.json`, `dma_controller.csv`)**: Headless cross-conversion environment validating roundtrip translation across ARM CMSIS-SVD, Accellera SystemRDL, IEEE 1685 IP-XACT, JSON Schema, and RFC 4180 CSV.

---

## Autonomous Simulation & Compilation Environments

Every example includes an autonomous `Makefile` alongside real test harnesses for C firmware (`test_harness.c`), Rust PAC modules (`test_harness.rs`), and Python bring-up scripts (`test_harness.py`).

### Standard Makefile Targets Across Examples

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

### Running Examples

```bash
# Run autonomous verification across all 20 examples simultaneously
make -C examples all

# Run an individual peripheral example directly (e.g. SPI):
cd examples/peripherals/spi
make all

# Run an architectural feature example (e.g. comprehensive):
cd examples/features/comprehensive
make compile-c

# Run strict architectural validation and negative test cases:
cd examples/features/strict_validation
make validate

# Run semantic diff between revisions:
cd examples/features/diff
make diff

# Run format cross-conversion across SVD, SystemRDL, IP-XACT, JSON, and CSV:
cd examples/features/format_conversion
make all

# Clean an individual example:
cd examples/peripherals/spi
make clean
```

### Standalone Portability: Copying Examples Outside the Repository

Each example folder is 100% self-contained and portable. After installing `rmap` to your system (e.g., via `make install` to `/usr/local` or `make install PREFIX=$HOME/.local`), you can copy any individual example folder or the entire `examples/` directory to any folder or external project:

```bash
# 1. Copy the installed examples directory to your preferred workspace
cp -r /usr/local/share/rmap/examples ~/my_rmap_eval
# (Or copy a single self-contained example):
# cp -r /usr/local/share/rmap/examples/peripherals/spi ~/my_spi_project

cd ~/my_rmap_eval

# 2. Run simulation and compilation across all 20 examples simultaneously
make all

# 3. Or enter any individual example and execute its targets
cd peripherals/spi
make all
make compile-c
make run-python
make sim-rtl
make clean
```

**Portability Architecture:**
- **Automatic Binary Discovery**: Each example Makefile dynamically checks for `rmap` in the local build directory (`../../../build/bin/rmap`), seamlessly falling back to `rmap` located in the system `PATH`.
- **Automatic Template Resolution**: When running outside the git repository, `rmap` automatically resolves default templates from the installed asset path (`<prefix>/share/rmap/templates`).
- **Fully Self-Contained**: Register map inputs (`.rmt`), negative test vectors, multi-format files, helper scripts (`post_generate.py`), and test harnesses resolve strictly within each example directory.

---

## CLI Usage Recipes

```bash
# Open SPI register map in GUI
./build/bin/rmap -f examples/peripherals/spi/spi.rmt

# Headless export of all templates to an output directory
./build/bin/rmap -f examples/peripherals/spi/spi.rmt --export --out ./work

# Convert SystemRDL to ARM CMSIS-SVD
./build/bin/rmap -f examples/features/format_conversion/atxmega_spi.rdl --convert spi.svd

# Convert CSV table to Protobuf text
./build/bin/rmap -f examples/features/format_conversion/dma_controller.csv --convert dma.rmt

# Run strict architectural linter validation
./build/bin/rmap -f examples/features/strict_validation/strict_validation.rmt --lint --strict

# Perform semantic diff between two register map revisions
./build/bin/rmap -f examples/features/diff/diff_v1.rmt --diff examples/features/diff/diff_v2.rmt
```
