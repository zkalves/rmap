# GUI User Guide

The **rmap** graphical interface is designed specifically for hardware designers, verification engineers, and embedded firmware developers to design, inspect, edit, and validate register maps with real-time feedback, visual packing bars, and multi-format export capabilities.

---

## 1. Dual-Pane Layout & Visualizer

```
┌────────────────────────────────────────────────────────────────────────┐
│ Toolbar: [New] [Open] [Save] [Undo] [Redo] [Add Blk] [Add Reg] [Export]│
├──────────────────────────────┬─────────────────────────────────────────┤
│ Left Panel: Search & Tree    │ Right Panel: Visualizer & Fields Table  │
│                              │                                         │
│ [ 🔍 Search (CTRL, 0x0)... ] │ Register: CTRL  Offset: 0x0  Description: [ SPI Control ]
│                              │ [31:8 Reserved] [7:2 MODE] [1:0 EN]     │
│ ── SPI_Top (blk)             │ ─────────────────────────────────────── │
│    ├── CTRL (reg @ 0x0)      │ Offset | Size | Name     | SW Access    │
│    ├── STATUS (reg @ 0x4)    │ ───────┼──────┼──────────┼───────────── │
│    └── DATA (reg @ 0x8)      │ 0      │ 1    │ EN       | RW           │
│                              │ 1      │ 2    │ MODE     | RW           │
│                              │ 8      │ 8    │ PRESCALE | RW           │
└──────────────────────────────┴─────────────────────────────────────────┘
```

### Key Components

- **Left Panel (Hierarchy Tree & Search)**:
  - Displays high-level structural containers: **Blocks** (`blk`), **Registers** (`reg`), and **Memories** (`mem`).
  - **Tree View Columns & Automatic Sorting**: Displays `Type`, `Offset`, `Name`, and `Description` for all nodes. Registers and blocks are automatically sorted numerically by byte `Offset` (`0x00`, `0x04`, `0x08`, `0x10`, `0x20`...).
  - **Navigation & Selection**: Left panel tree is dedicated to navigation and node selection; cell modifications are performed directly on the contextual right-side panel.
  - **Live Search Filter**: Filter the tree in real time by block name, register name, hex offset (`0x0`), access policy, or description.
- **Right Panel (Register Header, Bitfield Visualizer & Table)**:
  - **Contextual Register Header**: When a register is selected, provides inline editable fields for **Name**, **Offset** (hex/dec/bin), and **Description** with full Undo/Redo support.
  - **Interactive Bitfield Slice Visualizer**: Displays a continuous 32-bit or 64-bit segmented horizontal bar representing the physical bit layout of the selected register.
  - **Bitfields Table**: Detailed table listing all fields in the currently selected register (`Offset`, `Size`, `Name`, `Access Policy`, `HW Access`, `Reset Value`, `Is Rand`, `Volatile`, `Has Reset`, `Description`).

---

## 2. Interactive Bitfield Bar Visualizer

Positioned at the top of the right panel, the bitfield bar provides instant graphical insight into register packing and bit allocation:

1. **Access Policy Colour Synchronization**:
   - Bit slices are colour-coded to match the exact badge colours of the fields table:
     - **`RW`**: Green (`#A5D6A7` / `#1B5E20`)
     - **`RO` / `RC` / `RS`**: Blue (`#BBDEFB` / `#0D47A1`)
     - **`WO`**: Orange (`#FFCC80` / `#E65100`)
     - **`W1C` / `W1S` / `W0C`**: Amber/Yellow (`#FFF59D` / `#F57F17`)
2. **Dark Neutral Reserved Slots (40% Gray)**:
   - Unmapped bit ranges are rendered in a distinctive **40% dark gray** (`#666666`) background with subtle 45° diagonal micro-stripes and a bold `RSVD` / `RESERVED` label.
3. **Colour-Blind Accessible Mode (`Ctrl+Alt+C`)**:
   - Toggles an Okabe-Ito / Wong barrier-free CVD palette (Cyan for `RW`, Deep Royal Blue for `RO`, Vivid Magenta for `WO`, Flame Vermilion for `W1C`) with explicit textual bracket tags (`[RW]`, `[RO]`, `[WO]`, `[W1C]`). Accessible directly via `View -> Colour-Blind Mode` or `Ctrl+Alt+C`.
4. **Rich Hover Tooltips**:
   - Hovering over any slice displays full inspection details: Field Name, Bit Range `[MSB:LSB]`, Bit Width, Software Access Policy, Hardware Access Policy, Reset Value in hex, and Description.
5. **Bidirectional Interactive Selection Sync**:
   - Clicking a slice in the visualizer selects and scrolls to the corresponding row in the fields table.
   - Selecting a row in the fields table immediately highlights the slice in the visualizer bar.

---

## 3. Register Block View & Address Space Diagram

When selecting a **Register Block** (`blk` or `map`) in the navigation tree, the right pane automatically adapts to display the block's header and vertical address space memory map diagram:

```
┌───────────────────────────────────────────────────────────────────────────┐
│ Block: Sparse_Device   Offset: 0x0000   Description: [ Sparse Device Map ]│
├───────────────────────────────────────────────────────────────────────────┤
│ 🗺 Address Space Memory Map                                               │
│                                                                           │
│ 0x0000 ┌────────────────────────────────────────────────────────────────┐ │
│        │ CTRL                     [RW] [4B] Global Control Register     │ │
│ 0x0004 ├────────────────────────────────────────────────────────────────┤ │
│        │ STATUS                   [RO] [4B] Hardware Status Register    │ │
│ 0x0008 ├────────────────────────────────────────────────────────────────┤ │
│        │ //////// RESERVED [24 Bytes Gap] ///////////////////////////// │ │
│ 0x0020 ├────────────────────────────────────────────────────────────────┤ │
│        │ TX_FIFO                  [WO] [4B] Transmit FIFO Buffer        │ │
│ 0x0024 ├────────────────────────────────────────────────────────────────┤ │
│        │ RX_FIFO                  [RO] [4B] Receive FIFO Buffer         │ │
│ 0x0028 ├────────────────────────────────────────────────────────────────┤ │
│        │ //////// RESERVED [24 Bytes Gap] ///////////////////////////// │ │
│ 0x0040 ├────────────────────────────────────────────────────────────────┤ │
│        │ IRQ_ENABLE               [RW] [4B] Interrupt Enable Mask       │ │
└───────────────────────────────────────────────────────────────────────────┘
```

1. **Consistent Editable Block Header**:
   - Matches the exact dimensions, spacing, typography, and inline **Name**, **Offset**, and **Description** editors as the register view, with full Undo/Redo integration.
2. **Visual Address Space Diagram**:
   - **Proportional Scaling**: Renders a vertical, stacked memory map where individual register blocks scale proportionally to their byte width with left-hand address ruler annotations.
   - **Explicit RESERVED Gap Blocks**: Automatically detects and renders unmapped address gaps with dark-gray diagonal hatching and byte gap annotations (`[Gap: N Bytes]`), enabling immediate visual verification of address alignment and padding.
   - **Access Policy Colour-Coding**: Matches active theme and colour-blind mode (`RW` Green, `RO` Blue, `WO` Orange, `W1C` Yellow).
   - **Click-to-Navigate**: Click any register block in the diagram to navigate directly to its bitfield visualizer and field editor.
   - **Example File**: See [`examples/rmt/features/address_gap_example.rmt`](../../examples/rmt/features/address_gap_example.rmt) for a sample register map demonstrating unmapped address gaps.

---

## 4. Data Model & Column Reference

**rmap** uses an 11-column data model designed for hardware, verification, and firmware lifecycles:

| # | Column | Description | Supported Formats / Values | Single-Click Interaction |
| :-: | :--- | :--- | :--- | :--- |
| **0** | **Type** | Node element kind (`blk`, `reg`, `fld`, `mem`, `map`). | Fixed selection | Select node |
| **1** | **Offset** | Register byte offset or Field bit position (LSB). | Hex zero-padded (`0x0000`), Dec (`0`), Bin (`0b0`) | Edit text |
| **2** | **Size** | Bit width for Fields (Register width is global in Config). | Integer (1 to 64) | Edit text |
| **3** | **Name** | Identifier name for structs, macros, and RTL signals. | String | Edit text |
| **4** | **Access Policy (SW)** | Software / Bus register access policy. | `RW`, `RO`, `WO`, `W1C`, `W1S`, `W0C`, `RC`, `RS`, `NA` | Cycles `RW` &rarr; `RO` &rarr; `WO` &rarr; `W1C` |
| **5** | **HW Access** | Hardware internal core logic access mode. | `RO`, `RW`, `WO`, `NA`, `W1C`, `W1S`, `W0C`, `RS`, `RC` | Cycles `RO` &rarr; `RW` &rarr; `WO` &rarr; `NA` |
| **6** | **Reset Value** | Reset value for register bitfield. | Hex (`0x0`), Dec (`0`), Bin (`0b0`) | Edit text |
| **7** | **Is Rand** | UVM verification randomization flag (`rand`). | `true` / `false` | Toggles `true` &harr; `false` |
| **8** | **Volatile** | Hardware volatile property for C/C++ & Rust. | `true` / `false` | Toggles `true` &harr; `false` |
| **9** | **Has Reset** | Whether the bitfield has an explicit reset state. | `true` / `false` | Toggles `true` &harr; `false` |
| **10**| **Description** | Human-readable documentation string for registers, fields, and blocks. | String | Edit text |

---

## 4. Comprehensive Access Policy Reference

### Software (SW / Bus-Side) Access Policies

Software access policies define how the CPU or bus master (AXI/AHB/APB/Wishbone) interacts with the register:

| SW Policy | Full Name | Read Operation | Write Operation | Common Hardware Use-Case |
| :--- | :--- | :--- | :--- | :--- |
| **`RW`** | Read / Write | Returns current stored value. | Writes new value into storage. | General control registers, configuration parameters, thresholds. |
| **`RO`** | Read Only | Returns current hardware status. | Write is ignored / has no effect. | Hardware status flags, chip revision IDs, live sensor readings. |
| **`WO`** | Write Only | Returns `0` (or undefined). | Modifies storage or triggers action. | Trigger registers, software reset pulses, write-only command words. |
| **`W1C`** | Write 1 to Clear | Returns current flag value. | Writing `1` clears the bit to `0`; writing `0` leaves it unchanged. | Interrupt status registers, event acknowledgement flags. |
| **`W1S`** | Write 1 to Set | Returns current flag value. | Writing `1` sets the bit to `1`; writing `0` leaves it unchanged. | Software interrupt triggers, manual event assertions. |
| **`W0C`** | Write 0 to Clear | Returns current flag value. | Writing `0` clears the bit to `0`; writing `1` leaves it unchanged. | Active-low status acknowledgement registers. |
| **`RC`** | Read Clears | Returns current value and immediately clears bit to `0` (destructive read). | Write is ignored or modifies storage. | FIFO read pointers, self-clearing interrupt status flags. |
| **`RS`** | Read Sets | Returns current value and sets bit to `1`. | Write is ignored. | Status latch registers. |
| **`NA`** | No Access | Bus read produces error or `0`. | Bus write produces error or ignored. | Internal hardware storage unmapped from software address space. |

### Hardware (HW / Core-Side) Access Policies

Hardware access policies define how internal synthesizable RTL core logic interacts with the register:

| HW Policy | Full Name | Core Logic Read | Core Logic Write | Common Hardware Use-Case |
| :--- | :--- | :--- | :--- | :--- |
| **`RO`** | Hardware Read Only | Core logic continuously samples software configuration. | Core logic cannot write to register. | Static control registers, clock dividers, enable masks. |
| **`RW`** | Hardware Read / Write | Core logic reads current value. | Core logic updates value on internal conditions. | Bidirectional handshake counters, shared scratchpad registers. |
| **`WO`** | Hardware Write Only | Core logic does not read from register. | Core logic updates internal hardware status. | Status indicators, error counters, RX buffer state. |
| **`W1C`** | Hardware Write 1 to Clear| Core logic reads current value. | Hardware pulse clears the bit. | Auto-clearing timers, hardware watchdog flags. |
| **`W1S`** | Hardware Write 1 to Set | Core logic reads current value. | Hardware pulse asserts/sets the bit. | Interrupt request assertion, error event capture. |
| **`W0C`** | Hardware Write 0 to Clear| Core logic reads current value. | Active-low hardware pulse clears the bit. | Active-low reset triggers. |
| **`NA`** | No Hardware Access | Core logic does not interface with field. | Core logic does not interface with field. | Software-only scratchpad registers, reserved debug words. |

---

## 5. Real-Time Architectural Validation

**rmap** continuously inspects the register AST in real time to catch hardware design errors before RTL generation:

1. **Address Collisions**: Registers occupying the same byte offset or overlapping address ranges within a Block are highlighted in red.
2. **Bitfield Overlaps**: Fields sharing the same bit indices within a Register are flagged immediately in red.
3. **Width Overflow**: Bitfields whose `LSB + Width > Register Width` (e.g. bit 33 on a 32-bit bus) are highlighted in red.
4. **Non-Blocking Visual Indicators**: Invalid cells are highlighted with clear error badges without interrupting navigation or typing.

---

## 6. Multi-Level Undo / Redo System

The entire GUI is backed by a modular `QUndoStack`:
- **Undo (`Ctrl+Z`)**: Reverts cell edits, insertions, or deletions.
- **Redo (`Ctrl+Y`)**: Re-applies reverted operations.
- **Descriptive Action Names**: The Undo/Redo menu and tooltip reflect the exact operation (e.g. `Undo Edit Name`, `Undo Add Register`, `Undo Delete CTRL`).

---

## 7. Keyboard Shortcuts Reference

Access all key bindings at any time by pressing **`F1`** or selecting **Help &rarr; Key Bindings**:

| Category | Action | Shortcut |
| :--- | :--- | :--- |
| **File Operations** | New Register Map | `Ctrl+N` |
| | Open File | `Ctrl+O` |
| | Close Model | `Ctrl+W` |
| | Save File | `Ctrl+S` |
| | Save As... | `Ctrl+Shift+S` |
| | Code Generation & Export | `Ctrl+E` |
| | Preferences & Configuration | `Ctrl+P` |
| **Edit Operations** | Undo | `Ctrl+Z` |
| | Redo | `Ctrl+Y` |
| | Duplicate Selected Register/Field | `Ctrl+D` |
| | Delete Selected Item | `Delete` / `Del` / `Backspace` |
| **Item Insertion** | Add Register Block (`blk`) | `Ctrl+Shift+B` |
| | Add Register (`reg`) | `Ctrl+Shift+R` / `Ctrl+Return` |
| | Add Bitfield (`fld`) | `Ctrl+Shift+F` / `Ctrl+Shift+Return` |
| | Add Memory (`mem`) | `Ctrl+Shift+M` |
| | Add Map (`map`) | `Ctrl+M` |
| **Configuration & Preferences**| Open Project Configuration | `Ctrl+P` |
| | Open Application Preferences | `Ctrl+,` |
| **View & Navigation**| Toggle Colour-Blind Mode | `Ctrl+Alt+C` |
| | Focus Search Bar | `Ctrl+F` |
| | Show Key Bindings Help | `F1` |

---

## 8. Multi-Format Loading & Format-Aware Saving

- **Universal Format Import (`Ctrl+O`)**:
  - **ARM CMSIS-SVD** (`.svd`)
  - **SystemRDL 1.0 & 2.0** (`.rdl`)
  - **IP-XACT IEEE 1685-2009/2014/2022** (`.xml`, `.ipxact`)
  - **Standard JSON Schema** (`.json`)
  - **RFC 4180 CSV / TSV** (`.csv`, `.tsv`)
  - **Google Protocol Buffers** (`.rmt`, `.rmb`)
- **Format-Aware Quick Save (`Ctrl+S`)**:
  - Saving an opened file (`.rdl`, `.svd`, `.xml`, `.json`, `.csv`, `.rmt`) automatically saves back to its native format without coercing to `.rmt`.
- **Cross-Format Conversion (`File -> Save As...`)**:
  - Convert any register map to any other supported format simply by choosing the target extension.

---

## 9. Colour Schemes, Preferences & User Home Configuration

**rmap** includes a built-in multi-theme appearance engine with **Solarized 8 (Dark)** active by default:

- **Available Built-In Themes**:
  - **Solarized 8 (Dark)** (Default): Ethan Schoonover's classic cyan/blue/green/yellow/orange palette on a dark teal base (`#002b36`).
  - **Solarized 8 (Light)**: Precision warm light palette (`#fdf6e3` base, `#657b83` content).
  - **Nord (Dark)**: Arctic elegance based on Polar Night and Frost (`#2e3440`).
  - **Dracula (Dark)**: High-contrast purple, cyan, green, and pink palette (`#282a36`).
  - **Monokai (Dark)**: Classic vibrant editor theme (`#272822`).
  - **Classic Light**: Standard light desktop theme.

- **Theme Switching & Preferences**:
  - Change active theme via **View &rarr; Colour Scheme** or through the dedicated **Preferences Window (`Ctrl+,` / Edit &rarr; Preferences...)**.
  - Pass the `--colour-scheme <name>` (or `--theme <name>`) CLI option when starting `rmap` (e.g. `rmap --colour-scheme nord`).

- **User Home Configuration File (`~/.config/rmap/rmap.conf`)**:
  - User-specific GUI preferences (selected colour scheme, colour-blind mode, main window size/geometry, configuration window size, preferences window size, and panel splitter positions) are stored in the user's home area in `~/.config/rmap/rmap.conf` (or `$XDG_CONFIG_HOME/rmap/rmap.conf`).
  - Window dimensions and layout states are automatically saved on resize or close and restored on next launch.
  - Register map project files (`.rmt`, `.rdl`, `.svd`, `.json`) remain strictly focused on hardware architecture and register definitions without polluting project files with client-specific GUI state.
