# rmap Register Map Examples

This directory contains curated reference register maps and multi-format files demonstrating **rmap**'s capabilities, data modeling, format support, and verification features.

---

## Directory Overview

```
examples/
├── README.md                 # This documentation guide
│
├── rmt/                      # Native Protocol Buffer register maps (.rmt text & .rmb binary)
│   ├── peripherals/          # Standard peripheral controller models
│   │   ├── spi.rmt / spi.rmb # Standard SPI peripheral controller (Protobuf text & binary)
│   │   ├── uart.rmt          # UART peripheral controller register map
│   │   ├── dma.rmt           # Direct Memory Access (DMA) channel controller
│   │   └── sensor_hub.rmt    # Multi-sensor telemetry hub register map
│   ├── features/             # Advanced feature coverage and architectural patterns
│   │   ├── comprehensive.rmt # Comprehensive feature coverage (all 9 access policies, memory, booleans)
│   │   ├── wide_bus_64bit.rmt# 64-bit data bus architecture example
│   │   └── address_gap_example.rmt# Register map with unmapped address gaps & alignment padding
│   └── validation/           # Negative test cases & linter rule validation
│       └── invalid_overlap.rmt# Validation test cases (address collisions, field overlaps, width overflow)
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

- **`comprehensive.rmt`**: Covers every feature supported by rmap:
  - All 9 Software Access Policies (`RW`, `RO`, `WO`, `W1C`, `W1S`, `W0C`, `RC`, `RS`, `NA`).
  - Core Hardware Access Modes (`RO`, `RW`, `WO`, `W1S`, `W1C`, `NA`).
  - Dedicated memory blocks (`mem` kind) with byte/word sizes.
  - Number radix representations (Hexadecimal `0x`, Decimal, Binary `0b`).
  - Flags: `is_rand` (UVM randomization), `volatile` (firmware volatile qualifier), `has_reset`.
- **`wide_bus_64bit.rmt`**: High-performance system-level register map configured with global `reg_width: 64`. Demonstrates 64-bit register alignment, wide bitfield slicing, and `uint64_t` firmware generation.
- **`address_gap_example.rmt`**: Non-contiguous register addresses with deliberate address gaps (e.g. offset `0x0000` followed by `0x0010`). Demonstrates automatic reserved word generation (`uint32_t _reserved_[...]`) and GUI gap visualization.

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

## Usage Examples

```bash
# Open SPI register map in GUI
./build/bin/rmap -f examples/rmt/peripherals/spi.rmt

# Headless export of all templates to output directory
./build/bin/rmap -f examples/rmt/peripherals/spi.rmt --export --out ./work

# Convert SystemRDL to CMSIS-SVD
./build/bin/rmap -f examples/systemrdl/atxmega_spi.rdl --convert spi.svd

# Convert CSV to Protobuf text
./build/bin/rmap -f examples/csv/dma_controller.csv --convert dma.rmt

# Lint register map with strict checks
./build/bin/rmap -f examples/rmt/validation/invalid_overlap.rmt --lint --strict
```
