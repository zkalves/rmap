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
  "regmap_crc32": 2743849182,
  "regmap_crc32_hex": "0xA38EB0DE",
  "blocks": [
    {
      "name": "SPI_Top",
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
          "description": "Control Register",
          "pad_bytes_before": 0,
          "pad_words_before": 0,
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
              "description": "Enable SPI"
            }
          ]
        }
      ]
    }
  ]
}
```

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

1. **Synthesizable SystemVerilog Register File (`rtl/reg_map.sv.inja`)**:
   - IEEE 1800-2017 SystemVerilog synthesizable register file conforming to ASIC/FPGA HDL coding guidelines.
   - Standardized signal suffixes (`clk_i`, `rst_ni`, `_i` inputs, `_o` outputs, `_q`/`_d` register state).
   - Bus-agnostic generic slave register file with address decode logic and 2-space indentation.
   - Byte-level write enable strobing (`wstrb_i`).
   - Hardware sideband interface signals (`hw_*_i`, `hw_*_o`, `hw_*_set_i`).
   - Software read/write access strobes (`sw_*_wr_strobe_o`, `sw_*_rd_strobe_o`).
   - Hardware write priority over software writes.
   - External SRAM / sub-bus passthrough ports (`mem_<name>_req_o`, `we_o`, `addr_o`, `wdata_o`, `wstrb_o`, `rdata_i`, `ready_i`) for defined memory (`mem`) regions.

2. **UVM SystemVerilog Register Model (`uvm/reg_model.sv.inja`)**:
   - Complete `uvm_reg_block`, `uvm_reg`, and `uvm_reg_field` hierarchy.
   - `uvm_mem` instance and address mapping integration for hardware memory windows.
   - Backdoor HDL access paths (`add_hdl_path`).
   - Functional coverage sampling hooks.

3. **C/C++ Firmware Header (`c/reg_map.h.inja`)**:
   - Clean register address offsets and bitfield definitions.
   - Bitfield extraction and update macros (`RMAP_REG_GET`, `RMAP_REG_SET`).
   - Packed volatile C struct representation.

4. **Rust Peripheral Access Crate (`rust/reg_map.rs.inja`)**:
   - `#[repr(C)]` memory-mapped register block structures.
   - Type-safe `read()`, `write()`, and `modify()` accessors using volatile pointer operations.

5. **Python Bring-Up Driver (`python/reg_map.py.inja`)**:
   - Standalone Python object-oriented register driver class.
   - Bitfield getter/setter helpers.
   - Pluggable bus transport adapters for Cocotb, PyUVM, PyFTDI, PySerial, or JTAG.

6. **Interactive HTML Specification (`html/reg_doc.html.inja`)**:
   - Responsive, styled single-page HTML documentation.
   - Live interactive search bar.
   - Color-coded graphical bitfield slice bars with access policy tags.

7. **SystemRDL 2.0 Specification (`systemrdl/reg_map.rdl.inja`)**:
   - Standard Accellera SystemRDL 2.0 register file and addrmap specification.
   - Complete `field`, `reg`, and `regfile` component hierarchy with SW and HW access policies.

8. **IP-XACT IEEE 1685-2014/2022 (`ipxact/reg_map.xml.inja`)**:
   - Complete IP-XACT XML register model component definition (`ipxact:component`, `ipxact:memoryMaps`, `ipxact:addressBlock`, `ipxact:register`, `ipxact:field`).

9. **ARM CMSIS-SVD Peripheral XML (`svd/reg_map.xml.inja`)**:
   - Cortex-M CMSIS-SVD device specification (`<device>`, `<peripheral>`, `<register>`, `<field>`) for IDE debuggers (Keil, IAR, VS Code Cortex-Debug, SVDconv).

10. **Markdown Documentation Specification (`markdown/reg_doc.md.inja`)**:
    - Clean GitHub-flavored Markdown register map table specification with block anchors and bitfield tables.

11. **JSON Schema Specification (`json/reg_map.json.inja`)**:
    - Formatted JSON register map schema export for custom tooling, CI scripts, and automation pipelines.

---

## 4. Multi-Source Template Mappings & Folder Scanning

In the **Configuration Dialog** (`Ctrl+P`), you can configure multiple template search folders, individual template files, per-template enable toggles, and target output destinations:

- **Multiple Template Search Folders**: Configure one or more search directories (e.g. `./templates`, `../shared_templates`, `$MY_TEMPLATES`).
- **Automatic Template Scanning**: Click **Scan / Refresh** to recursively discover all `*.inja` and `*.tmpl` files across all configured search folders. Newly discovered templates are added to the list unchecked (disabled by default) so you can selectively enable only what you need, while preserving your existing configuration.
- **Granular Enable / Disable Controls**: Toggle individual templates via checkboxes in the **Enable** column, or use **All** / **None** buttons to bulk-toggle generation. Only enabled templates are processed during GUI export (`Ctrl+E`) and headless export (`--export`).
- **Relative Path Resolution**: Template and output paths specified as relative (e.g. `templates/c/reg_map.h.inja`, `work/c/reg_map.h`) resolve relative to the register map file's directory first, with fallback to the Current Working Directory (CWD).
- **Environment Variable Expansion**: Paths can include `$VAR`, `${VAR}`, Windows `%VAR%`, and `~` (home directory), expanded dynamically at generation time.
- **Mirrored Output Directory Architecture**: By default, generated files automatically mirror the category subdirectory of their source template (e.g. `templates/rtl/reg_map.sv.inja` &rarr; `<out_dir>/rtl/reg_map.sv`, `templates/c/reg_map.h.inja` &rarr; `<out_dir>/c/reg_map.h`).
- **Dynamic GUI Synchronization**: Changing the **Default Output Folder** in the Configuration Dialog automatically updates all rows using default mirrored paths, while leaving any custom per-template overrides untouched.
- **Sync Outputs**: Click **Sync Outputs** in the toolbar to synchronize or reset all template outputs to the active default output folder.
- **Dynamic Path Variables**: Output destination paths support meaningful dynamic variables for flexible SoC repository layouts:
  - `{output_folder}` / `{output_dir}`: Configured global default output folder.
  - `{category}` / `{cat}`: Template category subfolder (e.g. `rtl`, `c`, `uvm`, `html`).
  - `{block_name}` / `{block}`: Lowercase register block or peripheral name (e.g. `spi_core`).
  - `{project_name}` / `{project}`: Project name defined in configuration.
  - `{file_extension}` / `{ext}`: Output file extension (stripped of `.inja` / `.tmpl`).
  - `{template_name}` / `{filename}`: Base template name (e.g. `reg_map`, `reg_doc`).
  *(e.g., `{output_folder}/{category}/{block_name}_regs.{file_extension}`).*
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

2. **`rmap` Module & Aliases**:
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

- **Automated Verification Harness (`tests/test_template.py`)**: Runs comprehensive functional, structural, and syntax tests on each template:
  - **`c`**: Verifies include guards, `extern "C"`, bit manipulation macros (`_GET`, `_SET`, `_MASK`, `_SHIFT`), alignment padding (`_reserved_`), CRC32 macros, and compiles a C99/C++17 runtime test harness with `gcc`/`g++`.
  - **`rtl`**: Verifies synthesizable SystemVerilog module declaration, bus slave interface, hardware sideband signals, address decoding, byte-strobe updates, W1C/W1S/W0C logic, and runs `verilator` / `iverilog` syntax & lint checks.
  - **`uvm`**: Verifies `uvm_reg_block`, `uvm_reg`, and `uvm_reg_field` hierarchy, factory registration, field access configuration, backdoor HDL paths, and address map registration.
  - **`rust`**: Verifies `#![no_std]` PAC layout, volatile pointers, transparent struct wrappers, bit extraction functions (`get_*`, `set_*`), and compiles library and functional tests with `rustc`.
  - **`python`**: Validates syntax with `py_compile`, dynamically imports the driver module, attaches mock bus read/write callbacks, and tests field read-modify-write operations.
  - **`html`**: Validates HTML5 syntax with `HTMLParser`, DOM table structures, interactive search inputs, and verifies no unrendered Inja tags remain.
  - **`markdown`**: Validates table column alignments, bit range formatting `[msb:lsb]`, block headings, and renders tables with Python markdown.
  - **`systemrdl`**: Validates Accellera SystemRDL 2.0 `addrmap`, `regfile`, `reg`, `field` hierarchy, balanced braces, and software/hardware access properties.
  - **`ipxact`**: Validates IEEE 1685-2014 XML schema hierarchy, namespaces, memory maps, address blocks, registers, and fields with `xml.etree` and `xmllint`.
  - **`svd`**: Validates ARM CMSIS-SVD 1.3 XML schema, peripherals, registers, and bit ranges.
  - **`json`**: Validates JSON schema correctness, block/register/field nesting, and verifies strict boolean/integer data types.

- **Local Execution**:
  ```bash
  # Test all templates
  make test-templates

  # Test an individual template
  python3 tests/test_template.py rtl
  python3 tests/test_template.py c
  ```

- **CI/CD Integration**: In GitHub Actions (`.github/workflows/ci.yml`), the `test-templates` matrix job runs 15 parallel test jobs in CI with template-specific toolchains (`verilator 5.050`, `rustc`, `libxml2-utils`, `peakrdl`, etc.).

[Next: Architecture & Internal Data Flow &rarr;](architecture.md)

