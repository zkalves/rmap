<!--
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at https://mozilla.org/MPL/2.0/.

  Copyright (c) 2026 Ezequiel Alves. All rights reserved.
-->

# rmap Code Generation Templates

This directory contains the built-in code generation templates used by **rmap** to transform hardware register map specifications into synthesizable SystemVerilog RTL, multi-version UVM verification models, embedded C/C++ firmware headers, Rust PAC crates, Python bring-up drivers, and interactive documentation.

Templates are rendered using the [Pantor Inja](https://github.com/pantor/inja) modern C++ template engine (v3.3.0) and [nlohmann/json](https://github.com/nlohmann/json).

---

## 1. Directory Structure

```text
templates/
├── c/
│   └── reg_map.h.inja          # C/C++ firmware headers and register struct layouts
├── html/
│   └── reg_doc.html.inja       # Interactive searchable HTML register map specification
├── ipxact/
│   └── reg_map.xml.inja        # IP-XACT IEEE 1685-2014/2022 XML component model
├── json/
│   └── reg_map.json.inja       # Formatted JSON register map schema export
├── markdown/
│   └── reg_doc.md.inja         # Markdown documentation tables and register summaries
├── python/
│   └── reg_map.py.inja         # Python object-oriented register driver & bus abstraction
├── pyuvm_tb/
│   └── tb_pyuvm.py.inja        # Open-source Cocotb & pyuvm testbench
├── rtl/
│   ├── apb_reg_file.sv.inja    # Synthesizable AMBA 4 APB (APB4) slave bridge wrapper
│   ├── axil_reg_file.sv.inja   # Synthesizable AMBA 4 AXI4-Lite slave bridge wrapper
│   ├── reg_map.sv.inja         # Synthesizable IEEE 1800-2017 SystemVerilog register file
│   ├── reg_map.v.inja          # Synthesizable IEEE 1364-2001 Verilog register file
│   ├── reg_map.vhd.inja        # Synthesizable IEEE 1076 VHDL register file
│   └── reg_map_sva.sv.inja     # Formal & dynamic SystemVerilog Assertions (SVA) checker
├── rtl_tb/
│   └── tb_reg_map.sv.inja      # Self-checking SystemVerilog RTL testbench
├── rust/
│   └── reg_map.rs.inja         # Rust Peripheral Access Crate (PAC) with type-safe accessors
├── sim/
│   └── Makefile.inja           # Multi-simulator runner Makefile (Icarus, Verilator, Cocotb, UVM)
├── svd/
│   └── reg_map.xml.inja        # ARM CMSIS-SVD peripheral XML for IDEs and debuggers
├── systemrdl/
│   └── reg_map.rdl.inja        # Accellera SystemRDL 2.0 register file specification
├── uvm/
│   └── reg_model.sv.inja       # Universal multi-version UVM register model (1.1d, 1.2, 1800.2-2017/2020)
└── uvm_tb/                     # Universal multi-version UVM verification testbench suite
    ├── reg_bus_if.sv.inja      # Generic synchronous register bus interface
    ├── reg_bus_pkg.sv.inja     # UVM bus transaction, driver, monitor, and agent package
    ├── reg_env.sv.inja         # UVM register verification environment and predictor
    ├── reg_tests.sv.inja       # Built-in UVM test sequences (reset, RW access, bit-bash)
    └── tb_top.sv.inja          # Top-level SystemVerilog testbench harness module
```

---

## 2. Built-in Template Catalog

### 1. Synthesizable SystemVerilog RTL (`rtl/reg_map.sv.inja`)
- **Language**: Synthesizable IEEE 1800-2017 SystemVerilog.
- **Output**: `<out_dir>/rtl/reg_map.sv`
- **Features**:
  - Clean bus-agnostic register slave interface conforming to ASIC/FPGA synchronous design guidelines.
  - Byte-level write strobing (`wstrb_i`).
  - Standardized naming suffixes: `_i` for inputs, `_o` for outputs, `_q` for register state, `_d` for combinatorial next-state.
  - Granular hardware sideband control ports: `hw_<reg>_<fld>_i` (data input), `hw_<reg>_<fld>_we_i` (hardware write enable), and `hw_<reg>_<fld>_o` (current field value output).
  - Software read/write access strobes: `sw_<reg>_<fld>_wr_strobe_o` and `sw_<reg>_<fld>_rd_strobe_o` for peripheral hardware triggering.
  - Configurable access behaviors: `RW`, `RO`, `WO`, `W1C`, `W1S`, `W1T`, `W0C`, `RC`, `RS`. Hardware writes take strict precedence over software writes.
  - External SRAM / sub-bus passthrough memory ports for defined memory (`mem`) regions: `mem_<name>_req_o`, `we_o`, `addr_o`, `wdata_o`, `wstrb_o`, `rdata_i`, `ready_i`.

### 2. Synthesizable Verilog-2001 Register File (`rtl/reg_map.v.inja`)
- **Language**: Synthesizable IEEE 1364-2001 Verilog.
- **Output**: `<out_dir>/rtl/reg_map.v`
- **Features**:
  - IEEE 1364-2001 synthesizable Verilog implementation for legacy ASIC synthesis and FPGA toolchains.
  - Matches bus-agnostic interface, byte write strobes, configurable `DATA_WIDTH`, and `HW_PRECEDENCE` parameterization.

### 3. Synthesizable VHDL Register File (`rtl/reg_map.vhd.inja`)
- **Language**: Synthesizable IEEE 1076-1993/2008 VHDL.
- **Output**: `<out_dir>/rtl/reg_map.vhd`
- **Features**:
  - IEEE 1076-1993/2008 compliant synthesizable VHDL register file with `std_logic_vector` ports, generic parameters (`DATA_WIDTH`, `HW_PRECEDENCE`), and synchronous process blocks.

### 4. Synthesizable APB4 Register Slave Wrapper (`rtl/apb_reg_file.sv.inja`)
- **Language**: Synthesizable SystemVerilog (AMBA 4 APB).
- **Output**: `<out_dir>/rtl/apb_reg_file.sv`
- **Features**:
  - AMBA 4 APB (APB4) compliant synthesizable bridge wrapping the generic register file with `paddr`, `psel`, `penable`, `pwrite`, `pwdata`, `pstrb`, `pready`, `prdata`, and `pslverr` signals.

### 5. Synthesizable AXI4-Lite Register Slave Wrapper (`rtl/axil_reg_file.sv.inja`)
- **Language**: Synthesizable SystemVerilog (AMBA 4 AXI4-Lite).
- **Output**: `<out_dir>/rtl/axil_reg_file.sv`
- **Features**:
  - AMBA 4 AXI4-Lite compliant synthesizable bridge wrapping the generic register file with standard 5-channel handshakes (AW, W, B, AR, R channels) and parameterized data widths.

### 6. Formal & Dynamic SystemVerilog Assertions (`rtl/reg_map_sva.sv.inja`)
- **Language**: IEEE 1800-2017 SystemVerilog Assertions (SVA).
- **Output**: `<out_dir>/rtl/reg_map_sva.sv`
- **Features**:
  - IEEE 1800-2017 SystemVerilog Assertions (SVA) checker bindable directly to `reg_map`.
  - Formally verifies reset states, bus write/read protocol properties, write-1-to-clear invariants, and hardware precedence arbitration.

### 7. Universal UVM Register Model (`uvm/reg_model.sv.inja`)
- **Language**: SystemVerilog UVM (Universal Verification Methodology).
- **Output**: `<out_dir>/uvm/reg_model.sv`
- **Multi-Version Compatibility**:
  - Accellera UVM 1.1d
  - Accellera UVM 1.2
  - IEEE 1800.2-2017
  - IEEE 1800.2-2020
- **Features**:
  - Full hierarchical model generation: `uvm_reg_field`, `uvm_reg`, `uvm_mem`, and `uvm_reg_block`.
  - Conditional version macros (`UVM_VERSION_1_1` through `UVM_VERSION_2020`) to bridge API differences transparently.
  - Backward-compatible access typing aliases (`uvm_door_e` / `uvm_path_e`).
  - Strict parameter compatibility for `post_predict` across UVM 1.1/1.2 (`uvm_reg_item` vs. `uvm_reg_data_t`).
  - Automatic `uvm_reg_map` address mapping with native byte address offsets and block base addresses.
  - Backdoor HDL path registration via `add_hdl_path()` and register coverage hooks (`build_coverage()`).

### 8. C/C++ Firmware Header (`c/reg_map.h.inja`)
- **Language**: ANSI C (C99) and C++.
- **Output**: `<out_dir>/c/reg_map.h`
- **Features**:
  - Standard integer types (`stdint.h`).
  - Register offset constants (`#define <BLOCK>_<REG>_OFFSET 0x...`).
  - Bitfield bitmask and bit-shift macros (`#define <BLOCK>_<REG>_<FLD>_SHIFT ...`, `_MASK ...`).
  - Ergonomic field extraction and assignment macros (`RMAP_REG_GET`, `RMAP_REG_SET`).
  - Packed volatile memory-mapped C `struct` representations matching hardware memory layouts.

### 9. Rust Peripheral Access Crate (`rust/reg_map.rs.inja`)
- **Language**: Safe Rust with standard `core::ptr::read_volatile` and `write_volatile`.
- **Output**: `<out_dir>/rust/reg_map.rs`
- **Features**:
  - `#[repr(C)]` memory-mapped register block structures.
  - Zero-cost abstractions for bitfield reading, writing, and modifying (`read()`, `write()`, `modify()`).
  - Strong compile-time typing preventing out-of-range field assignments.

### 10. Python Hardware Bring-Up Driver (`python/reg_map.py.inja`)
- **Language**: Python 3.7+.
- **Output**: `<out_dir>/python/reg_map.py`
- **Features**:
  - Standalone object-oriented register block classes.
  - Field accessors with automatic bitwise masking and shifting.
  - Pluggable bus transport adapters for Cocotb, PyUVM, PyFTDI, PySerial, or custom JTAG/SPI bridges.

### 11. Interactive HTML Specification (`html/reg_doc.html.inja`)
- **Language**: Standalone HTML5 / CSS3 / JavaScript (no external CDN dependencies).
- **Output**: `<out_dir>/html/reg_doc.html`
- **Features**:
  - Responsive, modern layout with real-time fuzzy search bar.
  - Color-coded graphical register bitfield visualizer bars displaying bit slices, reset states, and access permissions.
  - Tabular register and bitfield property summaries.

### 12. Accellera SystemRDL 2.0 (`systemrdl/reg_map.rdl.inja`)
- **Language**: SystemRDL 2.0 (Accellera standard).
- **Output**: `<out_dir>/systemrdl/reg_map.rdl`
- **Features**:
  - Formal `addrmap`, `regfile`, `reg`, and `field` component hierarchy.
  - Complete software and hardware access policy annotations (`sw = rw;`, `hw = r;`, etc.).
  - Compatible with commercial SystemRDL compilers and open-source PeakRDL.

### 13. IP-XACT IEEE 1685 (`ipxact/reg_map.xml.inja`)
- **Language**: IP-XACT XML (IEEE 1685-2014 and IEEE 1685-2022).
- **Output**: `<out_dir>/ipxact/reg_map.xml`
- **Features**:
  - Standard schema elements: `ipxact:component`, `ipxact:memoryMaps`, `ipxact:addressBlock`, `ipxact:register`, and `ipxact:field`.
  - Full vendor, library, name, version (VLNV) metadata integration.

### 14. ARM CMSIS-SVD Peripheral XML (`svd/reg_map.xml.inja`)
- **Language**: ARM CMSIS-SVD 1.3 XML schema.
- **Output**: `<out_dir>/svd/reg_map.xml`
- **Features**:
  - Direct import into embedded IDEs (Keil MDK, IAR Embedded Workbench, VS Code Cortex-Debug, Eclipse).
  - Accurate base addresses, register offsets, bit ranges (`[msb:lsb]`), and reset values.

### 15. Markdown Register Specification (`markdown/reg_doc.md.inja`)
- **Language**: GitHub Flavored Markdown (GFM).
- **Output**: `<out_dir>/markdown/reg_doc.md`
- **Features**:
  - Ready for inclusion in project Git documentation portals, wikis, and PR reviews.
  - Clean summary tables showing register addresses, widths, reset states, and bitfield breakdowns.

### 16. JSON Schema Export (`json/reg_map.json.inja`)
- **Language**: JSON.
- **Output**: `<out_dir>/json/reg_map.json`
- **Features**:
  - Standard machine-readable serialization of the complete register hierarchy.
  - Perfect for CI/CD linting, automated validation scripts, and custom internal tool pipelines.

### 17. Self-Checking RTL Testbench (`rtl_tb/tb_reg_map.sv.inja`)
- **Language**: SystemVerilog.
- **Output**: `<out_dir>/rtl_tb/tb_reg_map.sv`
- **Features**:
  - Complete testbench harness instantiating the generated register file.
  - Automated tasks verifying reset values, read/write accesses, bitfield masks, and clear-on-write behaviors.
  - Ready for execution in open-source simulators (Icarus Verilog, Verilator) and commercial tools.

### 18. Open-Source Python UVM Testbench (`pyuvm_tb/tb_pyuvm.py.inja`)
- **Language**: Python 3 with Cocotb & pyuvm.
- **Output**: `<out_dir>/pyuvm_tb/tb_pyuvm.py`
- **Features**:
  - Headless verification using open-source simulators (Icarus Verilog, Verilator).
  - Implements complete UVM testbench structure in Python without requiring proprietary commercial licenses.

### 19. Universal UVM Verification Environment (`uvm_tb/`)
- **Files**:
  - `reg_bus_if.sv.inja`: Generic synchronous bus interface with clock, reset, request, write-enable, address, data, and acknowledge signals.
  - `reg_bus_pkg.sv.inja`: Bus transaction (`reg_bus_trans`), driver, monitor, sequencer, and agent package with register adapter (`reg2bus_adapter`).
  - `reg_env.sv.inja`: UVM verification environment instantiating the register model, bus agent, predictor, and coverage collector.
  - `reg_tests.sv.inja`: Comprehensive test sequence suite including reset check, register access, and random bit-bashing.
  - `tb_top.sv.inja`: Top-level SystemVerilog module instantiating the DUT register file, interface, clock/reset generator, and starting the test.
- **Multi-Version Compatibility**: Compatible with Accellera UVM 1.1d, 1.2, IEEE 1800.2-2017, and IEEE 1800.2-2020.

### 20. Multi-Tool Simulation Runner Makefile (`sim/Makefile.inja`)
- **Language**: GNU Makefile.
- **Output**: `<out_dir>/sim/Makefile`
- **Features**:
  - Automated simulation targets:
    - `make sim-rtl`: Run RTL testbench with Icarus Verilog.
    - `make sim-verilator`: Compile and run with Verilator.
    - `make sim-pyuvm`: Run Cocotb/pyuvm testbench.
    - `make sim-uvm SIM=<vcs|xrun|mti> UVM_VER=<version>`: Run full UVM verification across commercial simulators with version selection.

---

## 3. Inja Template Helpers

**rmap** extends the Inja template engine with custom C++ helper functions tailored for hardware modeling and code generation:

| Helper Function | Syntax Example | Output Example | Description |
| :--- | :--- | :--- | :--- |
| `upper` | `{{ upper(reg.name) }}` | `"CTRL"` | Converts string to uppercase. |
| `lower` | `{{ lower(reg.name) }}` | `"ctrl"` | Converts string to lowercase. |
| `camel_case` | `{{ camel_case("TX_FIFO_CTRL") }}` | `"txFifoCtrl"` | Converts identifier to camelCase. |
| `pascal_case`| `{{ pascal_case("tx_fifo_ctrl") }}` | `"TxFifoCtrl"` | Converts identifier to PascalCase. |
| `snake_case` | `{{ snake_case("TxFifoCtrl") }}` | `"tx_fifo_ctrl"` | Converts identifier to snake_case. |
| `c_type` | `{{ c_type(reg.size_width) }}` | `"uint32_t"` | Deduce C standard integer type from bit width (8 &rarr; `uint8_t`, 16 &rarr; `uint16_t`, 32 &rarr; `uint32_t`, 64 &rarr; `uint64_t`). |
| `msb` | `{{ msb(fld.size_width, fld.offset_lsb) }}` | `3` | Calculate MSB index: `offset_lsb + size_width - 1`. |
| `to_hex` | `{{ to_hex(fld.reset_val, 8) }}` | `"0x00000000"` | Formats integer value as padded hexadecimal string. |
| `to_dec` | `{{ to_dec(reg.size_width) }}` | `"32"` | Formats integer value as decimal string. |
| `bitmask` | `{{ bitmask(fld.size_width, fld.offset_lsb) }}` | `0x0000000F` | Computes 64-bit mask for bitfield slice. |
| `pad_zero` | `{{ pad_zero(fld.offset_lsb, 2) }}` | `"04"` | Pads decimal number with leading zeroes. |
| `sv_hex` | `{{ sv_hex(fld.reset_val, 32) }}` | `32'h0000_0000` | Formats SystemVerilog sized hexadecimal literal. |

---

## 4. Context Data Model

When Inja executes a template, the register map is serialized to a hierarchical JSON context object:

```json
{
  "name": "spi",
  "project_name": "My_SoC",
  "project_version": "1.0.0",
  "reg_width": 32,
  "reg_width_bytes": 4,
  "regmap_crc32": 2743849182,
  "regmap_crc32_hex": "0xA38EB0DE",
  "blocks": [
    {
      "name": "spi_core",
      "base_addr": 0,
      "base_addr_hex": "0x0",
      "total_size": 256,
      "total_size_hex": "0x100",
      "crc32": 305419896,
      "crc32_hex": "0x12345678",
      "registers": [
        {
          "name": "CTRL",
          "offset_lsb": 0,
          "offset_hex": "0x0",
          "size_width": 32,
          "access": "RW",
          "reset_val": 0,
          "reset_hex": "0x0",
          "description": "Main Control Register",
          "pad_bytes_before": 0,
          "pad_words_before": 0,
          "fields": [
            {
              "name": "ENABLE",
              "offset_lsb": 0,
              "size_width": 1,
              "access": "RW",
              "reset_val": 0,
              "reset_hex": "0x0",
              "is_rand": true,
              "volatile": false,
              "has_reset": true,
              "description": "Peripheral enable bit"
            }
          ]
        }
      ],
      "memories": [
        {
          "name": "BUFFER_RAM",
          "offset_lsb": 1024,
          "offset_hex": "0x400",
          "size_width": 1024,
          "size_hex": "0x400",
          "access": "RW",
          "description": "Packet buffer RAM window"
        }
      ]
    }
  ],
  "custom_parameters": {
    "FIFO_DEPTH": "16",
    "BUS_PROTOCOL": "APB4"
  }
}
```

---

## 5. Adding Custom Templates

To add a new code generation template:
1. Create a new `.inja` or `.tmpl` file in this directory (or in a project-specific template search folder).
2. Use standard Inja syntax (`{{ variable }}`, `{% for item in list %}`, `{% if condition %}`).
3. Leverage custom helpers (`upper()`, `snake_case()`, `msb()`, etc.) for clean code generation.
4. Add the template to your register map configuration via the **Configuration Dialog** (`Ctrl+P`) or by specifying `--export` paths.
5. Add an automated test case in `tests/test_template.py` to ensure long-term regression testing.

---

## 6. Testing Templates

Run comprehensive template verification across all 20 template deliverables:

```bash
# Run template verification suite
make test-templates

# Or invoke test runner directly
python3 tests/test_template.py all
```
