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

Defines the abstract behavioral contract between internal core hardware logic and register storage:
- **`RO`**: Hardware logic continuously monitors/samples the current register field state.
- **`RW`**: Hardware logic monitors the field state and synchronously updates field storage when a hardware write enable is asserted with valid write data.
- **`WO`**: Hardware logic synchronously updates field storage when a hardware write enable is asserted; hardware logic does not observe read state.
- **`WIRE`**: Passthrough / live signal bypass. Zero flip-flops or storage elements are synthesized. Software bus reads sample the live external hardware input directly via the read multiplexer.
- **`W1T`**: Hardware toggle on pulse. Hardware drives an active-high toggle pulse mask, flipping the bit state of marked bits in storage.
- **`INCR`**: Hardware counter increment. When an active-high increment strobe is asserted, field storage increments by 1.
- **`DECR`**: Hardware counter decrement. When an active-high decrement strobe is asserted, field storage decrements by 1.
- **`W1C` / `W1S` / `W0C` / `RC` / `RS`**: Hardware logic drives pulse triggers (set, clear, or toggle).
- **`NA`**: Hardware core logic has no interface to the field (software-only register).

> [!NOTE]
> Concrete HDL port signatures, naming conventions, and bus protocol adapters (such as hardware write enables, write data, live sampling, pulse toggles, and counter increment/decrement strobes) are defined by and customized within individual code generation templates. See [templates-and-codegen.md](templates-and-codegen.md).

### Decode-Only Registers & External Storage Policy

Certain peripheral subsystems require address decoding, transaction gating, and bus protocol arbitration without internal flip-flop register storage. Common examples include:
- Offloading register storage and state management to external ASIC/FPGA logic or external IP cores.
- Transparently forwarding register access requests across clock domains, asynchronous boundaries, or multi-cycle external datapaths.
- Mixing fixed internal storage with externally-managed control bitfields within the same register address.

To support these architectures, **rmap** provides an integrated **Decode-Only** policy configurable at both register and field granularity:
- **Full Decode-Only Register**: When a register is configured as decode-only, the register map generator emits address decoding, write data/byte-strobe forwarding, and external read multiplexing logic, but completely omits internal flip-flop storage. Software read and write accesses generate external validation signals and interface directly with external hardware logic. External logic drives read data and controls completion via ready and error handshakes, permitting multi-cycle external transactions and wait-states.
- **Decode-Only Bitfields in Mixed Registers**: When individual bitfields within a standard register are configured as decode-only, only the marked bitfields bypass internal storage and interface externally, while remaining bitfields retain synthesizable flip-flop storage and standard internal hardware update semantics.
- **Hierarchical Signal Scoping Invariant**: If a register is marked decode-only, access signals are generated strictly at the register boundary, avoiding redundant per-field strobe signals. Conversely, if a register contains a mixture of normal and decode-only bitfields, external interface signals are generated exclusively for the decode-only bitfields, while internal storage bitfields maintain their standard hardware ports and flip-flops.

### Hardware vs. Software Arbitration Precedence

When software bus transactions and internal hardware logic attempt concurrent writes to the same bitfield on the same cycle, the data model supports configurable arbitration precedence:
- **Hardware Precedence (Default)**: Internal hardware updates take priority over concurrent software writes, preserving real-time safety guarantees and preventing dropped hardware status events.
- **Software Precedence**: Software writes take priority over internal hardware updates during concurrent access.

### Dynamic Software Locks (`SW_LOCK`)

In security-critical and safety-critical SoC designs, specific registers or fields must be protected from unauthorized software access unless hardware-controlled locking signals or runtime register conditions are satisfied.

**rmap** provides an integrated **Dynamic Software Lock** mechanism supporting fully independent write locks and read locks:

#### Independent Lock Dimensions (Write Lock & Read Lock)
Locks are independently configured for software writes and software reads:
- **Write Lock (`lock_wr`)**: When the write lock condition evaluates true, software bus writes to the register or field are blocked. Storage flip-flop state is preserved unchanged, and software write event strobes are suppressed. Software reads remain unaffected unless a read lock is also specified.
- **Read Lock (`lock_rd`)**: When the read lock condition evaluates true, software bus reads from the register or field are blocked. Read data returns all zeros (preventing unauthorized observation of sensitive cryptographic keys, hardware seeds, or secure configuration), and software read event strobes are suppressed. Software writes remain unaffected unless a write lock is also specified.
- **Simultaneous / Dual Protection**: Both write and read locks may be active simultaneously, governed either by identical conditions or by distinct, independently configured logic expressions.

#### 3-Level Hierarchical Software Locking (Block, Register, Field)
Software read and write locks can be defined hierarchically across all three tiers of the peripheral model:
1. **Block Level**: A block-level lock gates all registers and bitfields contained within the peripheral block.
2. **Register Level**: A register-level lock gates all bitfields within the target register.
3. **Field Level**: A field-level lock selectively gates individual bit-slice updates and read data zeroing for protected fields, while keeping sibling fields within the same register fully accessible.

**Hierarchical Composition Rule**:
The effective lock condition for any bitfield is the logical disjunction (OR) of its field lock, parent register lock, and ancestor block lock:
- `eff_wr_locked(field) = blk_wr_locked || reg_wr_locked || fld_wr_locked`
- `eff_rd_locked(field) = blk_rd_locked || reg_rd_locked || fld_rd_locked`

Under this hierarchy:
- If a block is locked, every register and field in the block is locked.
- If a register is locked, every field in that register is locked.
- If an individual field is locked, only that field is protected; neighboring fields remain writable or readable according to their own permissions.
- Software event strobes are generated per-register and per-field, and are suppressed whenever the corresponding hierarchical lock evaluates true.

#### Language-Agnostic Expression Syntax & Operators
To maintain complete architectural independence between the core data model and HDL target deliverables, lock expressions in the data model and interchange formats are strictly language-agnostic logic expressions comprising:
- **External Hardware Input Signals**: Signals from core logic, external pins, or security controllers (e.g. `sec_lock`, `debug_unlocked`, `pin_strapping_lock`).
- **Internal Register Bitfields**: References to fields in the same or other registers using dot notation (e.g. `SYS_CTRL.LOCK`, `SECURITY.DEBUG_KEY`).
- **Literals**: Standard binary and integer constants (`1'b0`, `1'b1`, `0`, `1`).
- **Supported Operators**:
  - Logical AND: `&&`, `and`, `AND`
  - Logical OR: `||`, `or`, `OR`
  - Bitwise XOR: `^`, `xor`, `XOR`
  - Logical NOT / Inversion: `!`, `not`, `NOT`, `~`
  - Equality / Inequality: `==`, `!=`, `/=`
  - Grouping Parentheses: `( ... )`

#### Target HDL Conversion via Template Callbacks
The core data model and serialization handlers never embed target HDL signal naming or syntax. Instead, code generation templates dynamically convert agnostic lock expressions into target HDL constructs using built-in Inja helper callbacks:
- `sv_lock_expr(expr)`: Emits synthesizable SystemVerilog logic expressions with indexed vector slices.
- `v_lock_expr(expr)`: Emits synthesizable Verilog-2001 logic expressions.
- `vhd_lock_expr(expr)`: Emits synthesizable VHDL boolean expressions with `std_logic` conversions (`'1'`).

#### Backwards Compatibility & Legacy Scope Tags
For backward compatibility with earlier project files, combined expressions with scope tags (`[w]`, `[r]`, `[rw]`, `[w] <expr1>; [r] <expr2>`) are automatically parsed upon load and decomposed into independent `lock_wr` and `lock_rd` model attributes.

#### External Signal Deduplication Invariant
When multiple registers or fields reference the same external hardware lock input signal (for example, multiple control registers all governed by an external lock input), **rmap** automatically recognizes the re-use and declares **exactly one** top-level input port declaration on block boundaries in code generation templates. This guarantees zero redundant port declarations across SystemVerilog, Verilog 2001, and VHDL.

#### Hardware Strobe & Access Gating Semantics
When a block, register, or field evaluates as locked:
1. **Software Write Protection**: When write-locked, software write pulse strobes are suppressed low, and software write data is blocked from mutating internal storage.
2. **Software Read Protection**: When read-locked, software read pulse strobes are suppressed low, and bus read multiplexers return all zeros for the locked bit positions.
3. **Hardware Logic Independence**: Core hardware updates continue to function according to the configured hardware access policy (`RW`, `WO`, `INCR`, `DECR`, `W1T`), unaffected by software lock state.
4. **Formal Verification (SVA)**: Invariant assertions mathematically verify in simulation and formal model checking that software writes cannot alter write-locked registers or fields, and software reads return zero when read-locked.

### Access Violation & Bus Error Response Policies

In bus interface design, host software may attempt illegal access transactions, including:
1. **Write to Read-Only (`RO`)**: Attempting a software write to registers or bitfields with non-writable access policies (`RO`, `RC`, `RS`, `NOACCESS`).
2. **Read from Write-Only (`WO`)**: Attempting a software read from registers or bitfields with non-readable access policies (`WO`, `WO1`, `WOC`, `WOS`, `NOACCESS`).
3. **Write to Write-Locked**: Attempting a software write to a block, register, or field while its write lock condition evaluates true.
4. **Read from Read-Locked**: Attempting a software read from a block, register, or field while its read lock condition evaluates true.

#### Configurable Response Models: Silent Drop vs. Protocol Error
By default, standard register files silently ignore illegal writes (preserving storage) and return all zeros on illegal reads without halting bus transactions, maintaining broad compatibility with generic bus masters.

To catch software driver bugs or enforce security access policies, hardware register templates provide four independent configuration parameters:
- **`ERROR_ON_WRITE_TO_RO`**: When enabled, writing to any byte lane targeting a Read-Only or write-ignored field asserts a protocol bus error and suppresses software write event strobes.
- **`ERROR_ON_READ_FROM_WO`**: When enabled, reading a register containing Write-Only or read-prohibited fields asserts a protocol bus error, returns all zeros, and suppresses software read event strobes.
- **`ERROR_ON_WRITE_TO_LOCKED`**: When enabled, writing to any register or bitfield while write-locked asserts a protocol bus error and suppresses software write event strobes.
- **`ERROR_ON_READ_FROM_LOCKED`**: When enabled, reading any register or bitfield while read-locked asserts a protocol bus error, returns all zeros, and suppresses software read event strobes.

Protocol adapters (such as AMBA 4 APB and AMBA AXI4-Lite) automatically translate internal bus errors into native bus protocol responses (asserting APB `PSLVERR` or returning AXI `SLVERR` response codes).

#### Lossless Multi-Format Interoperability
Independent write and read lock expressions are preserved with 100% roundtrip fidelity across all supported formats:

| Format | Representation | Specification / Standard |
| :--- | :--- | :--- |
| **JSON** | `"lock_wr": "<expr>"`, `"lock_rd": "<expr>"` (with fallback `"lock"`) on blocks, registers, and fields | Standard rmap JSON schema. |
| **SystemRDL 2.0** | `rmap_lock_wr = "<expr>";`<br>`rmap_lock_rd = "<expr>";` on `addrmap`, `reg`, and `field` | Accellera SystemRDL 2.0 user-defined properties (`rmap_lock_wr`, `rmap_lock_rd`). |
| **IP-XACT** | `<rmap:lock_wr><expr></rmap:lock_wr>`<br>`<rmap:lock_rd><expr></rmap:lock_rd>` under `addressBlock`, `register`, and `field` | IEEE 1685 qualified with `xmlns:rmap="https://github.com/zkalves/rmap"`. |
| **ARM CMSIS-SVD** | `<rmap_lock_wr><expr></rmap_lock_wr>`<br>`<rmap_lock_rd><expr></rmap_lock_rd>` under `peripheral`, `register`, and `field` | ARM CMSIS-SVD vendor extension nodes. |
| **CSV** | `Write Lock` and `Read Lock` columns across `blk`, `reg`, and `fld` rows | RFC 4180 spreadsheet columns. |
| **Google Protobuf** | Item metadata string map (`"Write Lock"`, `"Read Lock"`) on blocks, registers, and fields | Native `.rmt` / `.rmb` schema. |

### Core Data Model vs. Template-Specific Implementations

**rmap** strictly decouples its core architectural data model from target-specific code generation artifacts:
- **Tool Architecture & Data Model**: Defines address offsets, bitfield layouts, standard access policies (IEEE 1800.2), hardware access semantics, reset values, arbitration rules, and agnostic lock expressions in a target-agnostic manner.
- **Template-Specific Implementations**: Concrete HDL signal naming conventions (e.g. clock, reset, hardware ports), byte write strobes, software read/write access pulse strobes, bus protocol slave wrappers (APB4, AXI4-Lite), SVA assertions, C struct layouts, and UVM adapter classes are defined by and customized within individual code generation templates.
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

### Project Identification & Metadata Mapping

rmap provides configurable project metadata attributes in the Project Configuration dialog (`groupMetadata`), serialized into native `.rmt` / `.rmb` Protobuf configurations and translated bidirectionally across external format targets:

| Metadata Field | GUI Label / Proto Field | IP-XACT (IEEE 1685) | ARM CMSIS-SVD | SystemRDL 2.0 | JSON |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Vendor** | `Vendor` / `project_vendor` | `<ipxact:vendor>` | `<vendor>` | N/A | `"project_vendor"` |
| **Library** | `Library` / `project_library` | `<ipxact:library>` | N/A | N/A | `"project_library"` |
| **Name** | `Project Name` / `project_name` | `<ipxact:name>` | `<name>` | `addrmap <name>` / `name = "<name>";` | `"project_name"` |
| **Version** | `Version` / `project_version` | `<ipxact:version>` | `<version>` | N/A | `"project_version"` |
| **Description** | `Description` / `project_description` | `<ipxact:description>` | `<description>` | `desc = "<desc>";` | `"project_description"` |

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
