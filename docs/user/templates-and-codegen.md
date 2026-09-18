# Templates & Code Generation

**rmap** uses the **Pantor Inja** template engine (a modern C++ template engine inspired by Jinja2) and **nlohmann/json** to render hardware RTL, verification models, embedded firmware, driver classes, and documentation artifacts from the register hierarchy.

---

## 1. Available JSON Context

When templates are executed, the full register map model is exposed as a JSON structure:

```json
{
  "name": "SPI_Project",
  "project_name": "My_SoC",
  "project_version": "1.0.0",
  "reg_width": 32,
  "reg_width_bytes": 4,
  "hw_precedence": true,
  "param_hw_precedence": 1,
  "regmap_crc32": 2743849182,
  "regmap_crc32_hex": "0xA38EB0DE",
  "blocks": [
    {
      "name": "SPI_Top",
      "crc32": 305419896,
      "crc32_hex": "0x12345678",
      "hdl_path": "DUT",
      "maps": [
        {
          "name": "default_map",
          "is_default": true,
          "base_addr": 0,
          "base_hex": "0x0",
          "n_bytes": 4,
          "endianness": "UVM_LITTLE_ENDIAN",
          "byte_addressing": 1
        }
      ],
      "registers": [
        {
          "name": "CTRL",
          "offset_lsb": 0,
          "offset_hex": "0x0",
          "size_width": 32,
          "access": "RW",
          "reset_val": 0,
          "reset_hex": "0x0",
          "description": "Control Register",
          "pad_bytes_before": 0,
          "pad_words_before": 0,
          "hdl_path": "reg_ctrl_q",
          "is_fifo": false,
          "fifo_depth": 8,
          "is_indirect": false,
          "index_reg": "",
          "has_callbacks": false,
          "no_reg_test": false,
          "no_reset_test": false,
          "no_bit_bash_test": false,
          "no_access_test": false,
          "fields": [
            {
              "name": "EN",
              "offset_lsb": 0,
              "size_width": 1,
              "access": "RW",
              "reset_val": 0,
              "reset_hex": "0x0",
              "is_rand": true,
              "volatile": false,
              "has_reset": true,
              "individually_accessible": 1,
              "description": "Enable SPI"
            }
          ]
        }
      ],
      "memories": [
        {
          "name": "BUFFER",
          "offset_lsb": 256,
          "offset_hex": "0x100",
          "size_width": 1024,
          "word_width": 32,
          "depth": 256,
          "access": "RW",
          "hdl_path": "mem_buffer_ram",
          "no_mem_test": false,
          "no_walk_test": false,
          "no_access_test": false,
          "description": "Packet Buffer SRAM"
        }
      ]
    }
  ]
}
```

### Context Schema & Extended RAL Properties Reference

The context passed to Inja templates provides rich hardware architecture and verification attributes:

- **Root Attributes**:
  - `name`: Root register map identifier (defaults to `"regmap"` if unassigned).
  - `project_name` / `project_version`: User-configured project identification metadata.
  - `reg_width` / `reg_width_bytes`: Global register data width in bits (e.g. 32, 64) and bytes.
  - `hw_precedence`: Boolean flag indicating whether hardware updates take priority over concurrent software writes.
  - `param_hw_precedence`: Integer parameter value (`1` = hardware over software, `0` = software over hardware) for synthesizable HDL generics.
  - `regmap_crc32` / `regmap_crc32_hex`: Global 32-bit architectural CRC checksum.
  - `blocks`: Array of peripheral block structures.

- **Address Maps (`maps[]`)**:
  - `name`: Address map identifier (`uvm_reg_map` instance name, e.g. `"default_map"`, `"apb_map"`).
  - `is_default`: Boolean flag indicating if this is the default map.
  - `base_addr` / `base_hex`: Base address offset in integer and hex string representation.
  - `n_bytes`: Native bus byte width for the address map.
  - `endianness`: Endianness specification string (e.g. `"UVM_LITTLE_ENDIAN"`, `"UVM_BIG_ENDIAN"`).
  - `byte_addressing`: Integer flag (`1` for byte-level address increments, `0` for word addressing).

- **Registers (`registers[]`)**:
  - `name`, `offset_lsb`, `offset_hex`, `size_width`, `access`, `reset_val`, `reset_hex`, `description`: Core register properties.
  - `pad_bytes_before` / `pad_words_before`: Address gap bytes and words before this register (used for C struct padding `_reserved_`).
  - `hdl_path`: Backdoor HDL signal path relative to DUT (defaults to `"reg_<regname>_q"`).
  - `is_fifo`: Boolean indicating if register represents a FIFO port.
  - `fifo_depth`: FIFO buffer depth in words (default `8`).
  - `is_indirect`: Boolean indicating indirect register addressing via an index register.
  - `index_reg`: Name of the index/address register used to address this indirect register.
  - `has_callbacks`: Boolean indicating whether UVM pre/post-read/write callbacks are registered.
  - `no_reg_test`: Disables all standard UVM built-in register test sequences (`NO_REG_TEST`).
  - `no_reset_test`: Disables UVM reset sequence (`NO_REG_HW_RESET_TEST`).
  - `no_bit_bash_test`: Disables UVM bit bash sequence (`NO_REG_BIT_BASH_TEST`).
  - `no_access_test`: Disables UVM register access sequence (`NO_REG_ACCESS_TEST`).
  - `fields`: Array of bitfield definitions within this register.

- **Bitfields (`fields[]`)**:
  - `name`, `offset_lsb`, `size_width`, `access`, `reset_val`, `reset_hex`, `is_rand`, `volatile`, `has_reset`, `description`: Core field properties.
  - `individually_accessible`: Integer flag (`1` or `0`) indicating whether the bitfield can be individually accessed or byte-enabled without altering neighboring bits.

- **Memories (`memories[]`)**:
  - `name`, `offset_lsb`, `offset_hex`, `size_width`, `description`: Memory window identifier, address offset, and total size.
  - `depth`: Number of memory words (depth).
  - `word_width`: Width of each memory word in bits.
  - `hdl_path`: Backdoor HDL RAM array path for simulation backdoor access.
  - `no_mem_test`: Disables all UVM built-in memory test sequences.
  - `no_walk_test`: Disables UVM memory walking test sequence (`NO_MEM_WALK_TEST`).
  - `no_access_test`: Disables UVM memory access test sequence (`NO_MEM_ACCESS_TEST`).

---

## 2. Inja Template Helpers

**rmap** provides built-in custom helper callbacks for hardware numbering, casing, type deduction, and bit manipulation:

| Helper | Syntax Example | Output Example |
| :--- | :--- | :--- |
| `upper` | `{{ upper(reg.name) }}` | `"CTRL"` |
| `lower` | `{{ lower(reg.name) }}` | `"ctrl"` |
| `camel_case` | `{{ camel_case("TX_FIFO_CTRL") }}` | `"txFifoCtrl"` |
| `pascal_case` | `{{ pascal_case("tx_fifo_ctrl") }}` | `"TxFifoCtrl"` |
| `snake_case` | `{{ snake_case("TxFifoCtrl") }}` | `"tx_fifo_ctrl"` |
| `c_type` | `{{ c_type(reg.size_width) }}` | `"uint32_t"` |
| `msb` | `{{ msb(fld.size_width, fld.offset_lsb) }}` | `3` (for width 4, LSB 0 &rarr; `[3:0]`) |
| `to_hex` | `{{ to_hex(fld.reset_val, 8) }}` | `"0x00000000"` |
| `to_dec` | `{{ to_dec(reg.size_width) }}` | `"32"` |
| `bitmask` | `{{ bitmask(fld.size_width, fld.offset_lsb) }}` | `0x00000001` |
| `pad_zero`| `{{ pad_zero(fld.offset_lsb, 2) }}` | `"00"` |
| `sv_hex`  | `{{ sv_hex(fld.reset_val, 32) }}` | `32'h0000` |

---

## 3. Built-in Generation Templates

**rmap** includes a comprehensive suite of production-grade generation templates organized by output type in `templates/`:

1. **Synthesizable SystemVerilog Register File** (`rtl/reg_map.sv.inja`):
   - IEEE 1800-2017 SystemVerilog synthesizable register file conforming to ASIC/FPGA HDL coding guidelines.
   - Standardized signal suffixes (`clk_i`, `rst_ni`, `_i` inputs, `_o` outputs, `_q`/`_d` register state).
   - Bus-agnostic generic slave register file with address decode logic and 2-space indentation.
   - Dynamic parameterizable data width via `DATA_WIDTH` parameter (default 32, configurable to 8, 16, 32, 64, 128, 256, 512+ bits).
   - Byte-level write enable strobing (`wstrb_i` with width `DATA_WIDTH / 8`).
   - Hardware sideband interface signals (`hw_*_i`, `hw_*_we_i`, `hw_*_o`).
   - Software read/write access strobes (`sw_*_wr_strobe_o`, `sw_*_rd_strobe_o`) pulsing for 1 cycle upon transaction completion to trigger peripheral operations.
   - Configurable hardware vs. software write precedence via parameter (`PARAM_HW_PRECEDENCE` default 1 = hardware over software; 0 = software over hardware).
   - External SRAM / sub-bus passthrough ports (`mem_<name>_req_o`, `we_o`, `addr_o`, `wdata_o`, `wstrb_o`, `rdata_i`, `ready_i`) for defined memory (`mem`) regions.

2. **Synthesizable Verilog-2001 Register File** (`rtl/reg_map.v.inja`):
   - IEEE 1364-2001 synthesizable Verilog implementation for legacy ASIC synthesis and FPGA toolchains.
   - Matches bus-agnostic interface, byte write strobes, configurable `DATA_WIDTH`, and `HW_PRECEDENCE` parameterization.

3. **Synthesizable VHDL Register File** (`rtl/reg_map.vhd.inja`):
   - IEEE 1076-1993/2008 compliant synthesizable VHDL register file with `std_logic_vector` ports, generic parameters (`DATA_WIDTH`, `HW_PRECEDENCE`), and synchronous process blocks.

4. **Synthesizable APB4 Register Slave Wrapper** (`rtl/apb_reg_file.sv.inja`):
   - AMBA 4 APB (APB4) compliant synthesizable bridge wrapping the generic register file with `paddr`, `psel`, `penable`, `pwrite`, `pwdata`, `pstrb`, `pready`, `prdata`, and `pslverr` signals.

5. **Synthesizable AXI4-Lite Register Slave Wrapper** (`rtl/axil_reg_file.sv.inja`):
   - AMBA 4 AXI4-Lite compliant synthesizable bridge wrapping the generic register file with standard 5-channel handshakes (AW, W, B, AR, R channels) and parameterized data widths.

6. **Formal & Dynamic SystemVerilog Assertions** (`rtl/reg_map_sva.sv.inja`):
   - IEEE 1800-2017 SystemVerilog Assertions (SVA) checker bindable directly to `reg_map`.
   - Formally verifies reset states, bus write/read protocol properties, write-1-to-clear invariants, and hardware precedence arbitration.

7. **UVM SystemVerilog Register Model** (`uvm/reg_model.sv.inja`):
   - Complete `uvm_reg_block`, `uvm_reg`, and `uvm_reg_field` hierarchy.
   - Explicit multi-map support: models multiple distinct `uvm_reg_map` instances per block (e.g. `apb_map`, `axi_map`, `sec_map`), assigning registers to distinct bus domains, offsets, and privilege levels.
   - `uvm_mem` instance and address mapping integration for hardware memory windows.
   - Backdoor HDL access paths (`add_hdl_path`).
   - Functional coverage sampling hooks (`build_coverage`).
   - Universal multi-version UVM compatibility across Accellera UVM 1.1d, UVM 1.2, IEEE 1800.2-2017, and IEEE 1800.2-2020 via version macros, conditional `post_predict` parameter typing, and `uvm_door_e` / `uvm_path_e` compatibility aliases.

8. **C/C++ Firmware Header** (`c/reg_map.h.inja`):
   - Clean register address offsets and bitfield definitions.
   - Bitfield extraction and update macros (`RMAP_REG_GET`, `RMAP_REG_SET`).
   - Packed volatile C struct representation.

9. **Rust Peripheral Access Crate** (`rust/reg_map.rs.inja`):
   - `#[repr(C)]` memory-mapped register block structures.
   - Type-safe `read()`, `write()`, and `modify()` accessors using volatile pointer operations.

10. **Python Bring-Up Driver** (`python/reg_map.py.inja`):
    - Standalone Python object-oriented register driver class.
    - Bitfield getter/setter helpers.
    - Pluggable bus transport adapters for Cocotb, PyUVM, PyFTDI, PySerial, or JTAG.

11. **Interactive HTML Specification** (`html/reg_doc.html.inja`):
    - Responsive, styled single-page HTML documentation.
    - Live interactive search bar.
    - Color-coded graphical bitfield slice bars with access policy tags.

12. **SystemRDL 2.0 Specification** (`systemrdl/reg_map.rdl.inja`):
    - Standard Accellera SystemRDL 2.0 register file and addrmap specification.
    - Complete `field`, `reg`, and `regfile` component hierarchy with SW and HW access policies.

13. **IP-XACT IEEE 1685-2014/2022** (`ipxact/reg_map.xml.inja`):
    - Complete IP-XACT XML register model component definition (`ipxact:component`, `ipxact:memoryMaps`, `ipxact:addressBlock`, `ipxact:register`, `ipxact:field`).

14. **ARM CMSIS-SVD Peripheral XML** (`svd/reg_map.xml.inja`):
    - Cortex-M CMSIS-SVD device specification (`<device>`, `<peripheral>`, `<register>`, `<field>`) for IDE debuggers (Keil, IAR, VS Code Cortex-Debug, SVDconv).

15. **Markdown Documentation Specification** (`markdown/reg_doc.md.inja`):
    - Clean GitHub-flavored Markdown register map table specification with block anchors and bitfield tables.

16. **JSON Schema Specification** (`json/reg_map.json.inja`):
    - Formatted JSON register map schema export for custom tooling, CI scripts, and automation pipelines.

17. **Self-Checking RTL Testbench** (`rtl_tb/tb_reg_map.sv.inja`):
    - Standalone SystemVerilog testbench exercising reset values, read/write accesses, bitfield masks, and read-only/write-1-to-clear behaviors in Icarus Verilog or Verilator.

18. **Open-Source Python UVM Testbench** (`pyuvm_tb/tb_pyuvm.py.inja`):
    - Complete Python verification testbench utilizing Cocotb and pyuvm to run register tests headlessly with open-source simulators.

19. **Universal UVM Verification Environment** (`uvm_tb/*.sv.inja`):
    - Complete modular UVM testbench suite containing generic bus interface (`reg_bus_if.sv`), bus VIP package (`reg_bus_pkg.sv`), register environment (`reg_env.sv`), built-in test sequences (`reg_tests.sv`), and top-level harness (`tb_top.sv`).
    - Compatible across Accellera UVM 1.1d, UVM 1.2, IEEE 1800.2-2017, and IEEE 1800.2-2020.

20. **Multi-Tool Simulation Makefile** (`sim/Makefile.inja`):
    - Automated runner Makefile targeting Icarus Verilog (`sim-rtl`), Verilator (`sim-verilator`), Cocotb/pyuvm (`sim-pyuvm`), and commercial EDA simulators (`sim-uvm SIM=vcs|xrun|mti`).
    - Configurable UVM version selection via `UVM_VER=1800.2-2020|1800.2-2017|1.2|1.1d` or custom path via `UVM_HOME=/path/to/uvm`, with automatic local repository discovery.

---

## 4. Multi-Source Template Mappings & Folder Scanning

In the **Configuration Dialog** (`Ctrl+P`), you can configure multiple template search folders, individual template files, per-template enable toggles, and target output destinations:

- **Multiple Template Search Folders**: Configure one or more search directories (e.g. `./templates`, `../shared_templates`, `$MY_TEMPLATES`).
- **Automatic Template Scanning**: Click **Scan / Refresh** to recursively discover all `*.inja` and `*.tmpl` files across all configured search folders. Newly discovered templates are added to the list unchecked (disabled by default) so you can selectively enable only what you need, while preserving your existing configuration.
- **Granular Enable / Disable Controls**: Toggle individual templates via checkboxes in the **Enable** column, or use **All** / **None** buttons to bulk-toggle generation. Only enabled templates are processed during GUI export (`Ctrl+E`) and headless export (`--export`).
- **Relative Path Resolution**: Template and output paths specified as relative resolve relative to the register map file directory first, with fallback to the Current Working Directory (CWD).
- **Environment Variable Expansion**: Paths can include `$VAR`, `${VAR}`, Windows `%VAR%`, and `~` (home directory), expanded dynamically at generation time.
- **Mirrored Output Directory Architecture**: By default, generated files automatically mirror the category subdirectory of their source template (e.g. `templates/rtl/reg_map.sv.inja` &rarr; `&lt;out_dir&gt;/rtl/reg_map.sv`, `templates/c/reg_map.h.inja` &rarr; `&lt;out_dir&gt;/c/reg_map.h`).
- **Dynamic GUI Synchronization**: Changing the **Default Output Folder** in the Configuration Dialog automatically updates all rows using default mirrored paths, while leaving any custom per-template overrides untouched.
- **Sync Outputs**: Click **Sync Outputs** in the toolbar to synchronize or reset all template outputs to the active default output folder.
- **Dynamic Path Variables**: Output destination paths support meaningful dynamic variables and aliases for flexible SoC repository layouts:
  - `{output_folder}` / `{output_dir}` / `{out_dir}` / `{out}`: Configured global default output folder.
  - `{category}` / `{cat}`: Template category subfolder (e.g. `rtl`, `c`, `uvm`, `html`).
  - `{block_name}` / `{block}` / `{name}`: Lowercase register block or peripheral name (e.g. `spi_core`).
  - `{project_name}` / `{project}`: Project name defined in configuration.
  - `{file_extension}` / `{ext}`: Output file extension (stripped of `.inja` / `.tmpl`).
  - `{template_name}` / `{filename}`: Base template name (e.g. `reg_map`, `reg_doc`).
  *(e.g., `{out}/{category}/{block_name}_regs.{ext}`).*
- **Direct File Output & Per-Template Overrides**: Users can override any template to target an explicit file path (e.g. `../../hw/rtl/spi_reg_file.sv` or `../../sw/include/spi_regs.h`). Missing parent directories are created automatically.
- **Directory Output**: If a directory is specified (or ends in `/`), the filename is automatically computed by stripping `.inja` from the template name, preserving the relative subfolder structure (e.g. `work/c/reg_map.h`).
- **Relative Include Resolution**: Inja is initialized with each template's directory as root, ensuring `{% include %}` directives resolve cleanly regardless of template location.

---

## 5. Post-Generation Python Script Execution

In addition to Inja templates, **rmap** can automatically launch a custom Python script upon code generation (both in interactive GUI export `Ctrl+E` and headless batch export `--export`).

### Configuration
In the **Configuration Dialog** (`Ctrl+P`), set the **Python Script (Optional)** field (or click **Browse...** to pick a `.py` file). Like the output folder and project name fields, the Python script execution is **active whenever the field has content**. To disable it, simply leave the field empty.

### Available Variables & Context
The Python script has full access to the exact same data model and variables available to Inja templates through multiple ergonomic interfaces:

1. **Global Variables**:
   All root template variables are directly injected into the script's global namespace:
   - `name`: Register map / peripheral block name (e.g. `"spi"`).
   - `project_name`: Project name string from configuration.
   - `project_version`: Project version string.
   - `reg_width`: Register bit width (e.g. `32` or `64`).
   - `reg_width_bytes`: Register width in bytes (`reg_width / 8`).
   - `blocks`: Full list of register blocks with all registers, fields, memories, and CRC32 checksums.
   - Any user-defined custom key-value parameters.

2. **rmap Module & Aliases**:
   You can also access the data through `import rmap` or the predefined `data`, `context`, and `regmap` objects:
   ```python
   import rmap

   print(f"Generating for block: {rmap.name}")
   for block in rmap.blocks:
       for reg in block["registers"]:
           print(f"Register: {reg['name']} @ {reg['offset_hex']}")
   ```

3. **Built-in Helpers**:
   Bitwise and formatting helpers matching Inja templates are readily available:
   - `to_hex(val, width=8)`: Formats number as zero-padded hex (e.g. `to_hex(15, 4)` &rarr; `"0x000F"`).
   - `to_dec(val)`: Formats number as decimal string.
   - `bitmask(width, lsb)`: Computes bitmask integer.

4. **Command-Line Argument & Standard Input**:
   - `sys.argv[1]`: Absolute path to a temporary JSON file containing the full register map model.
   - `sys.stdin`: Full register model JSON data piped via standard input (loadable via `json.load(sys.stdin)`).

5. **Environment Variables**:
   - `RMAP_NAME`: Name of the register map.
   - `RMAP_PROJECT_NAME`: Configured project name.
   - `RMAP_PROJECT_VERSION`: Configured project version.
   - `RMAP_REG_WIDTH`: Register width.
   - `RMAP_JSON_FILE`: Temporary JSON file path.
   - `RMAP_JSON_DATA`: Complete JSON string of the register model.

---

## 6. Automated Template Testing & Verification

Every template in `templates/` is validated through automated test pipelines in CI/CD and locally:

- **Automated Verification Harness (`tests/test_template.py`)**: Runs comprehensive functional, structural, and syntax tests on each deliverable:
  - **c**: Verifies include guards, `extern "C"`, bit manipulation macros (`_GET`, `_SET`, `_MASK`, `_SHIFT`), alignment padding (`_reserved_`), CRC32 macros, and compiles a C99/C++17 runtime test harness with `gcc`/`g++`.
  - **rtl**: Verifies synthesizable SystemVerilog module declaration, bus slave interface, hardware sideband signals, address decoding, byte-strobe updates, W1C/W1S/W0C logic, and runs `verilator` / `iverilog` syntax & lint checks.
  - **verilog**: Verifies IEEE 1364-2001 synthesizable Verilog module structure, bus slave ports, byte-write enables, hardware sidebands, and checks syntax with `iverilog`.
  - **vhdl**: Verifies IEEE 1076 synthesizable VHDL entity and architecture, `std_logic_vector` ports, address decoding, and syntax verification.
  - **apb**: Verifies AMBA 4 APB (APB4) synthesizable bridge wrapper port bindings, setup and access phase state machine, byte-strobe decoding, and linting.
  - **axil**: Verifies AMBA 4 AXI4-Lite synthesizable bridge wrapper port handshakes, 5-channel transaction logic, and linting.
  - **sva**: Verifies formal and dynamic SystemVerilog Assertions (SVA) checking write-strobe protocol rules, address decode invariants, and reset integrity with `verilator` / `iverilog`.
  - **uvm**: Verifies `uvm_reg_block`, `uvm_reg`, and `uvm_reg_field` hierarchy, factory registration, field access configuration, backdoor HDL paths, and address map registration.
  - **rust**: Verifies `#![no_std]` PAC layout, volatile pointers, transparent struct wrappers, bit extraction functions (`get_*`, `set_*`), and compiles library and functional tests with `rustc`.
  - **python**: Validates syntax with `py_compile`, dynamically imports the driver module, attaches mock bus read/write callbacks, and tests field read-modify-write operations.
  - **html**: Validates HTML5 syntax with `HTMLParser`, DOM table structures, interactive search inputs, and verifies no unrendered Inja tags remain.
  - **markdown**: Validates table column alignments, bit range formatting `[msb:lsb]`, block headings, and renders tables with Python markdown.
  - **systemrdl**: Validates Accellera SystemRDL 2.0 `addrmap`, `regfile`, `reg`, `field` hierarchy, balanced braces, and software/hardware access properties.
  - **ipxact**: Validates IEEE 1685-2014 XML schema hierarchy, namespaces, memory maps, address blocks, registers, and fields with `xml.etree` and `xmllint`.
  - **svd**: Validates ARM CMSIS-SVD 1.3 XML schema, peripherals, registers, and bit ranges.
  - **json**: Validates JSON schema correctness, block/register/field nesting, and verifies strict boolean/integer data types.
  - **rtl_tb**: Validates self-checking testbench syntax, reset verification, and read/write register checks.
  - **pyuvm_tb**: Validates Python syntax via `py_compile`, Cocotb `@cocotb.test()` decorators, and pyuvm sequence inheritance.
  - **uvm_tb**: Validates universal UVM testbench suite, agent, VIP driver/monitor/adapter, reset/access test sequences, and multi-version `uvm_revision_string()` version reporting.
  - **sim_makefile**: Validates multi-tool simulation targets (`sim-rtl`, `sim-verilator`, `sim-pyuvm`, `sim-uvm`) and UVM version selection variables (`UVM_VER`, `1800.2-2020`, `1800.2-2017`, `1.2`, `1.1d`).

- **Local Execution**:
  ```bash
  # Test all 20 deliverables
  make test-templates

  # Test an individual deliverable
  python3 tests/test_template.py rtl
  python3 tests/test_template.py apb
  python3 tests/test_template.py axil
  python3 tests/test_template.py sva
  python3 tests/test_template.py verilog
  python3 tests/test_template.py vhdl
  python3 tests/test_template.py c
  ```

- **CI/CD Integration**: In GitHub Actions (`.github/workflows/ci.yml`), the `test-templates` matrix job runs 20 parallel test jobs in CI with deliverable-specific toolchains (`verilator 5.052`, `rustc`, `libxml2-utils`, `peakrdl`, etc.).

[Next: Architecture & Internal Data Flow &rarr;](architecture.md)

