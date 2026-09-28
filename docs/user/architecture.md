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
| **3** | **Name** | Semantic identifier for registers, fields, memories, and blocks. | Valid identifier (`[a-zA-Z_][a-zA-Z0-9_]*`) |
| **4** | **Access Policy (SW)** | Software / Bus register access policy (24 IEEE 1800.2 policies). | `RW`, `RO`, `WO`, `W1`, `WO1`, `W1C`, `W1S`, `W1T`, `W0C`, `W0S`, `W0T`, `RC`, `RS`, `WRC`, `WRS`, `WC`, `WS`, `W1SRC`, `W1CRS`, `W0SRC`, `W0CRS`, `WOC`, `WOS`, `NOACCESS` |
| **5** | **HW Access** | Hardware internal core logic access mode. | `RO`, `RW`, `WO`, `NA`, `W1C`, `W1S`, `W0C`, `RS`, `RC` |
| **6** | **Reset Value** | Hardware reset value for the register or field. | Hexadecimal (`0x0`), Decimal (`0`), Binary (`0b0`) |
| **7** | **Is Rand** | Constrained-random verification stimulus flag. | `true` / `false` |
| **8** | **Volatile** | Indicates hardware state can change asynchronously outside software control. | `true` / `false` |
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
- **Word Alignment & Address Scaling**: Register offsets naturally align to native word sizes, with automatic address gap detection and memory layout verification.
- **Continuous Bitfield Slicing**: Bitfields can span arbitrary bit positions up to the register width, with unmapped bits automatically allocated as reserved slices (`RSVD`).

---

## 3. Access Policy Matrix & Behavioral Semantics

### Comprehensive Software Access Policies (IEEE 1800.2 Standard)

**rmap** supports the full set of 24 standard register field access modes defined in IEEE 1800.2:

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

Defines the abstract behavioral contract between internal core hardware logic and the register storage:
- **`RO`**: Hardware logic continuously monitors/samples the current register field state.
- **`RW`**: Hardware logic monitors the field and applies synchronous updates with a write-enable condition.
- **`WO`**: Hardware logic drives updates directly into internal register storage.
- **`W1C` / `W1S` / `W0C` / `RC` / `RS`**: Hardware logic drives event pulse triggers (set, clear, or toggle).
- **`NA`**: Hardware core logic has no interface to the field (software-only register).

### Hardware vs. Software Arbitration Precedence

When software bus transactions and internal hardware logic attempt concurrent writes to the same bitfield on the same cycle, the data model supports configurable arbitration precedence:
- **Hardware Precedence (Default)**: Internal hardware updates take priority over concurrent software writes, preserving real-time safety guarantees and preventing dropped hardware status events.
- **Software Precedence**: Software writes take priority over internal hardware updates during concurrent access.

### Core Data Model vs. Template-Specific Implementations

**rmap** strictly decouples its core architectural data model from target-specific code generation artifacts:
- **Tool Architecture & Data Model**: Defines address offsets, bitfield layouts, standard access policies (IEEE 1800.2), hardware access semantics, reset values, and arbitration rules in a target-agnostic manner.
- **Template-Specific Implementations**: Concrete HDL signal naming conventions (e.g. `clk_i`, `rst_ni`, `hw_*_i`, `hw_*_o`), byte write strobes (`wstrb_i`), software read/write access pulse strobes (`sw_*_wr_strobe_o`, `sw_*_rd_strobe_o`), bus protocol slave wrappers (APB4, AXI4-Lite), SVA assertions, C struct layouts, and UVM adapter classes are defined by and customized within individual code generation templates.
- For detailed signal specifications, protocol handshakes, and timing waveforms of individual deliverables, see the [Template Deliverable Catalog & Specifications](@ref templates_codegen).

---

## 4. Multiple Address Maps & Bus Domains

Modern SoCs frequently access the same peripheral through multiple bus interfaces or security privilege regimes:
- **Multi-Bus Domains**: Dual interfaces such as a high-speed datapath and a low-power configuration/debug interface.
- **Security & Privilege Domains**: Secure World vs. Non-Secure World address mappings with differing offsets and access permissions.
- **Multiple Address Maps in rmap**: The data model allows assigning registers to multiple distinct address map (`map`) contexts, configuring independent base addresses, offsets, and access privileges per map. Target-specific template deliverables map these nodes to their native structures (such as `uvm_reg_map` in UVM, `ipxact:memoryMap` / `ipxact:memoryRemap` in IP-XACT, and `addrmap` in SystemRDL).

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
