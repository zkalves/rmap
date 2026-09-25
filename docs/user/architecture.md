# Register Map Architecture & Data Model {#architecture}

**rmap** is designed around a unified register map data model that captures hardware register hierarchies, memory spaces, multi-domain bus mappings, and comprehensive hardware/software access semantics across the entire silicon and firmware lifecycle.

---

## 1. Register Map Hierarchy & Data Model

**rmap** organizes hardware address spaces into a strict 4-level parent-child tree hierarchy:

```text
Root (Project / Address Map)
 ├── Register Block (blk)       ── Base address, address range, register instances
 │    ├── Register (reg)        ── Byte offset, data width, reset value, fields
 │    │    └── Field (fld)      ── Bit position (LSB), bit width, SW/HW access policies
 │    └── Memory Window (mem)   ── SRAM/FIFO window, size, access permissions
 └── Address Map (map)          ── Alternative bus domain mapping (e.g. AXI vs APB)
```

### The 11-Column Register Model

Every node in the register map is defined through an 11-column attribute specification:

| # | Column | Description | Supported Values / Format |
| :-: | :--- | :--- | :--- |
| **0** | **Type** | Node element kind in the address map. | `blk` (Block), `reg` (Register), `fld` (Field), `mem` (Memory), `map` (Address Map) |
| **1** | **Offset** | Register byte offset or Field bit position (LSB). | Hex zero-padded (`0x0000`), Decimal (`0`), Binary (`0b0`) |
| **2** | **Size** | Bit width for Fields or byte size for Memories. | Integer (1 to 1024 bits for fields; bytes for memory) |
| **3** | **Name** | Semantic identifier for RTL signals, macros, and structs. | Valid identifier (`[a-zA-Z_][a-zA-Z0-9_]*`) |
| **4** | **Access Policy (SW)** | Software / Bus register access policy (24 IEEE 1800.2 policies). | `RW`, `RO`, `WO`, `W1`, `WO1`, `W1C`, `W1S`, `W1T`, `W0C`, `W0S`, `W0T`, `RC`, `RS`, `WRC`, `WRS`, `WC`, `WS`, `W1SRC`, `W1CRS`, `W0SRC`, `W0CRS`, `WOC`, `WOS`, `NOACCESS` |
| **5** | **HW Access** | Hardware internal core logic access mode. | `RO`, `RW`, `WO`, `NA`, `W1C`, `W1S`, `W0C`, `RS`, `RC` |
| **6** | **Reset Value** | Hardware reset value for the register or field. | Hexadecimal (`0x0`), Decimal (`0`), Binary (`0b0`) |
| **7** | **Is Rand** | UVM verification randomization flag (`rand`). | `true` / `false` |
| **8** | **Volatile** | Hardware volatile qualifier for C/C++ and Rust headers. | `true` / `false` |
| **9** | **Has Reset** | Whether the bitfield has an explicit reset state. | `true` / `false` |
| **10**| **Description** | Human-readable documentation string for registers, fields, and blocks. | String |

### Architectural Validation Rules

During interactive editing and headless CLI verification (`--lint`), the validation engine enforces strict architectural invariants:
- **Bit Range Boundaries**: Field bit spans (`[LSB + Size - 1 : LSB]`) cannot exceed the register's defined data width.
- **Field Overlap Detection**: Multiple bitfields within the same register cannot share or overlap bit positions.
- **Register Address Alignment**: Register offsets must align to the native word size of the bus interface.
- **Block Boundary Enclosure**: Register offsets and memory window ranges must fit completely within their parent block's declared address space.
- **Unmapped Gap Detection**: Address gaps between registers and unmapped bit positions within registers are explicitly detected and reported.

---

## 2. Dynamic & Parameterizable Data Widths

**rmap** supports flexible, parameterizable register widths:

- **Arbitrary Data Width Support**: Registers and buses are not constrained to fixed 32-bit or 64-bit boundaries. Register widths are dynamically configurable per project, block, or register (supporting **8, 16, 32, 64, 128, 256, 512, and 1024-bit** widths).
- **HDL Parameterization**: RTL code generators emit configurable generic parameters (e.g. `DATA_WIDTH`) with automatically computed byte-strobe widths (`[DATA_WIDTH/8-1:0]`) and address decode logic scaled to native word alignments.
- **Continuous Bitfield Slicing**: Bitfields can span arbitrary bit positions up to the register width, with unmapped bits automatically allocated as reserved slices (`RSVD`).

---

## 3. Access Policy Matrix & Behavioral Semantics

### Comprehensive Software Access Policies (IEEE 1800.2 UVM Standard)

**rmap** supports the full set of 24 UVM access modes defined in IEEE 1800.2 (`uvm_reg_field`):

| Policy | Read Effect | Write Effect | Description & Common Use-Case |
| :--- | :--- | :--- | :--- |
| **`RW`** | Read current value | Write updates stored value | Standard Read/Write control registers and configuration parameters. |
| **`RO`** | Read current value | Writes ignored | Read-Only status register, revision IDs, live hardware telemetry. |
| **`WO`** | Returns 0 / undefined | Write updates stored value | Write-Only command registers and software reset pulses. |
| **`RC`** | Read current value; clears to 0 | Writes ignored | Read-to-Clear event registers. |
| **`RS`** | Read current value; sets to 1 | Writes ignored | Read-to-Set status registers. |
| **`WC`** | Read current value | All bits cleared to 0 | Write-Clear (clears on any write). |
| **`WS`** | Read current value | All bits set to 1 | Write-Set (sets on any write). |
| **`WRC`** | Read current value; clears to 0 | Write updates stored value | Write Read-Clear. |
| **`WRS`** | Read current value; sets to 1 | Write updates stored value | Write Read-Set. |
| **`W1C`** | Read current value | Write '1' clears bit; '0' no effect | Write-1-to-Clear interrupt status flags. |
| **`W1S`** | Read current value | Write '1' sets bit; '0' no effect | Write-1-to-Set software override flags. |
| **`W1T`** | Read current value | Write '1' toggles bit; '0' no effect | Write-1-to-Toggle diagnostic registers. |
| **`W0C`** | Read current value | Write '0' clears bit; '1' no effect | Write-0-to-Clear active-low interrupt status. |
| **`W0S`** | Read current value | Write '0' sets bit; '1' no effect | Write-0-to-Set active-low flags. |
| **`W0T`** | Read current value | Write '0' toggles bit; '1' no effect | Write-0-to-Toggle diagnostic registers. |
| **`W1SRC`** | Read clears to 0 | Write '1' sets bit; '0' no effect | Write-1-Set, Read-Clear. |
| **`W1CRS`** | Read sets to 1 | Write '1' clears bit; '0' no effect | Write-1-Clear, Read-Set. |
| **`W0SRC`** | Read clears to 0 | Write '0' sets bit; '1' no effect | Write-0-Set, Read-Clear. |
| **`W0CRS`** | Read sets to 1 | Write '0' clears bit; '1' no effect | Write-0-Clear, Read-Set. |
| **`W1`** | Read current value | First write updates; subsequent ignored | Write-Once security configuration keys. |
| **`WO1`** | Returns 0 / undefined | First write updates; subsequent ignored | Write-Only Once tamper lockout keys. |
| **`WOC`** | Returns 0 / undefined | Clears all bits to 0 | Write-Only Clear trigger. |
| **`WOS`** | Returns 0 / undefined | Sets all bits to 1 | Write-Only Set trigger. |
| **`NOACCESS`** | Read prohibited | Writes prohibited | Unmapped / Reserved register slice (`NA`). |

### Hardware Access Policies (HW)

Defines how internal peripheral logic interfaces with the register storage:
- **`RO`**: Hardware only observes the field output (`hw_<reg>_<fld>_o`).
- **`RW`**: Hardware observes and writes updates via `hw_<reg>_<fld>_i` when `hw_<reg>_<fld>_we_i` is asserted.
- **`WO`**: Hardware drives updates directly into the register flip-flops.
- **`W1C` / `W1S` / `W0C` / `RC` / `RS`**: Hardware drives event set/clear/toggle strobes.
- **`NA`**: No hardware connection.

### Hardware vs. Software Arbitration

When software and hardware attempt concurrent writes on the same clock cycle, synthesis and simulation models support configurable precedence rules:
- **`HW_PRECEDENCE = 1` (Default)**: Hardware updates take priority over software writes to preserve safety timing and avoid missing critical hardware interrupts.
- **`HW_PRECEDENCE = 0`**: Software writes take priority over hardware updates.

### Software Access Strobes

Synthesizable RTL templates emit dedicated 1-cycle active-high pulse strobes for software operations:
- **Register-Level Strobes**: `sw_<reg>_wr_strobe_o` (write pulse) and `sw_<reg>_rd_strobe_o` (read pulse).
- **Field-Level Strobes**: `sw_<reg>_<fld>_wr_strobe_o` and `sw_<reg>_<fld>_rd_strobe_o`.
- **Purpose**: Enables peripheral logic to react immediately to software transactions without polling (e.g. triggering an SPI transfer, acknowledging an interrupt, resetting a hardware timer, or popping a FIFO).

---

## 4. Multiple Address Maps (`uvm_reg_map`)

Modern SoCs frequently access the same peripheral through multiple bus interfaces or security privilege regimes:
- **Multi-Bus Domains**: Dual interfaces such as an AXI4-Lite high-speed datapath and an APB4 low-power configuration/debug interface.
- **Security & Privilege Domains**: Secure World vs. Non-Secure World address mappings with differing offsets and access permissions.
- **Multi-Map Support in rmap**: Allows assigning registers to multiple distinct `uvm_reg_map` instances within a `uvm_reg_block` (e.g. `apb_map`, `axi_map`), configuring independent base addresses, offsets, and access privileges per map.

---

## 5. Multi-Format Interoperability Matrix

**rmap** provides bidirectional roundtrip translation across industry-standard register specification formats:

| Format | Standard / Ecosystem | File Extensions | Primary Use-Case |
| :--- | :--- | :--- | :--- |
| **ARM CMSIS-SVD** | ARM Cortex-M Ecosystem | `.svd`, `.xml` | Microcontroller peripheral descriptions, IDE debuggers, firmware drivers. |
| **SystemRDL** | Accellera Standard (1.0 & 2.0) | `.rdl`, `.systemrdl` | Formal register description language for silicon IP and SoC integration. |
| **IP-XACT** | IEEE 1685 (2009, 2014, 2022) | `.xml`, `.ipxact` | Multi-vendor SoC packaging, EDA toolchain integration, bus interfaces. |
| **Google Protobuf** | rmap Native Architecture | `.rmt` (text), `.rmb` (binary) | High-speed native serialization, lossless attribute preservation, project files. |
| **JSON** | Standard JSON Schema | `.json` | Web dashboards, custom Python scripting, CI/CD automated validation. |
| **CSV / TSV** | RFC 4180 Tabular Data | `.csv`, `.tsv` | Spreadsheet authoring in Excel/LibreOffice, team register reviews. |

### Format Capabilities, Limitations & Implications

| Format | Native Capabilities | Known Limitations | Technical Implications |
| :--- | :--- | :--- | :--- |
| **ARM CMSIS-SVD** | Microcontroller peripherals, interrupt vectors, bit ranges, reset values. | Single flat address map; no hardware sideband ports; no memory window SRAM signals. | When converting to SVD, multi-map definitions and hardware sideband configs are omitted; generated SVD is strictly debugger/firmware focused. |
| **SystemRDL 2.0** | Full address map hierarchy, composite access policies, user properties, hardware access. | Complex syntax requiring conformant lexer/parser; some toolchains support only SystemRDL 1.0 subsets. | Full roundtrip fidelity preserved. Target toolchains must support SystemRDL 2.0 or downgrade to 1.0. |
| **IP-XACT IEEE 1685** | Multi-vendor standard, multiple `memoryRemap` and `addressBlock` nodes, bus interfaces. | XML schema version drift (2009 vs 2014 vs 2022); schema strictness varies between vendors. | Translators must specify schema target version; custom vendor extensions outside standard tags may require normalization. |
| **Google Protobuf** | 100% attribute fidelity, fast binary serialization, project metadata, template configs. | Proprietary binary/text format not directly ingestible by commercial 3rd-party EDA tools without rmap. | Primary project interchange format for rmap; requires export to SVD/SystemRDL/IP-XACT for external tool ingestion. |
| **JSON Schema** | Machine-readable, extensible, easy web and CI script integration. | Non-standardized industry schema; differs between EDA vendor implementations. | Standardized within rmap ecosystem; custom JSON schemas require mapping to rmap's JSON schema. |
| **CSV / TSV** | Universal spreadsheet tabular authoring in Excel/LibreOffice. | Flat table structure; cannot represent multi-level nested addrmaps or multi-map configurations natively. | Exporting deep hierarchies to CSV flattens names (e.g. `block_reg_field`); re-import requires hierarchical reconstruction. |

---

## 6. Configuration & Environment Variables

**rmap** behavior, template paths, and resource discovery can be configured through environment variables:

| Variable | Description | Default Fallback |
| :--- | :--- | :--- |
| `RMAP_CONFIG_FILE` | Path to persistent user configuration file. | `~/.config/rmap/rmap.conf` |
| `RMAP_THEMES_PATH` / `RMAP_THEME_DIR` | Directory containing custom color scheme files. | Bundled application themes |
| `RMAP_TRANSLATIONS_PATH` / `RMAP_TRANSLATION_DIR` | Directory containing custom JSON translations (`rmap_*.json`). | Bundled compiled translations |
| `RMAP_TEMPLATES_DIR` | Search directory for Inja code generation templates. | Bundled `templates/` directory |
| `RMAP_EXAMPLES_DIR` | Directory containing reference register map examples. | Bundled `examples/` directory |
| `RMAP_DOCS_DIR` | Directory containing offline documentation. | Bundled `_site/` directory |
| `RMAP_PYTHON` / `PYTHON` | Custom Python interpreter binary for test and helper scripts. | `python3` from `PATH` |
| `RMAP_PYTHON_TIMEOUT` | Execution timeout in milliseconds for Python scripts and generators. | `60000` (60 seconds) |
| `RMAP_TMPDIR` | Custom temporary directory for intermediate files. | System temporary directory |

---

> [!NOTE]
> **Developer Documentation**:
> For the internal Modern C++17 / Qt 6 subsystem design, model-view contracts, and complete C++ API reference, see the [rmap Developer Guide](@ref dev_guide) and [C++ Subsystem Architecture](@ref dev_architecture).
