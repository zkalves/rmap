# RegMapTreeModel

### 1. Class Overview

`RegMapTreeModel` is a custom hierarchical 11-column tree model implementing the Qt model/view contract (`QAbstractItemModel`). It manages the register map structure containing blocks (`blk`), registers (`reg`), bitfields (`fld`), memories (`mem`), and address maps (`map`). It provides in-memory node manipulation, 11-column table mapping, real-time architectural validation (overlap detection, address collision, bit width overflow, zero width, missing names), invalid cell tracking for UI highlighting, and JSON tree data extraction for Inja code generation templates.

### 2. Project Structure and Dependencies

`RegMapTreeModel` is declared in `src/RegMapTreeModel.hpp` and implemented in `src/RegMapTreeModel.cpp`.
- Composed of hierarchical `RegMapTreeItem` nodes.
- Consumed by `RegMapTreeView`, `QTableView`, `TreeFilterProxyModel`, and `FieldSortProxyModel` in `RegMapWindow`.
- Interfaced with `FormatManager` serializers and `CodeGenerator`.

Build Requirements:
- Qt 6 modules: `QtCore`, `QtGui`
- Embedded `nlohmann::json` (JSON serialization)

### 3. Class Hierarchy and Role

`RegMapTreeModel` inherits directly from:
- `QAbstractItemModel` (QtCore) — Core Qt abstract base class for hierarchical tree and table data models.

### 4. 11-Column Architecture Reference

The model maps tree data across 11 standard columns:

| Column Index | Header Name | Data Role / Interpretation | Delegate Editor |
|---|---|---|---|
| 0 | `Type` | Node kind (`blk`, `reg`, `fld`, `mem`, `map`) | Fixed Item / Decoration icon |
| 1 | `Offset/LSB` | Register byte offset or Field bit position (LSB) | `RegHexDecBinDelegate` |
| 2 | `Size/Width` | Bit width for fields (or register width) | `RegHexDecBinDelegate` |
| 3 | `Name` | Identifier string for C/RTL code generation | `RegStrDelegate` |
| 4 | `Access Policy` | Software / Bus-Side access policy (`RW`, `RO`, `WO`, etc.) | `RegAccessPolicyDelegate` |
| 5 | `HW Access` | Hardware Core-Side access mode (`RO`, `RW`, `WO`, etc.) | `RegHwAccessDelegate` |
| 6 | `Reset Value` | Reset value in hex, decimal, or binary | `RegHexDecBinDelegate` |
| 7 | `Is Rand` | UVM randomization flag (`rand` property) | `RegBoolDelegate` |
| 8 | `Volatile` | Hardware volatile property for C/C++ & Rust | `RegBoolDelegate` |
| 9 | `Has Reset` | Whether the bitfield has an explicit reset state | `RegBoolDelegate` |
| 10 | `Description` | Human-readable documentation string | `RegMapDelegate` |

### 5. Public Methods

#### explicit RegMapTreeModel(QObject *parent = nullptr)
Constructs the tree model, initializing default column headers and creating an empty root `RegMapTreeItem`.

#### ~RegMapTreeModel() override
Destructor. Deletes the root item and all child nodes.

#### RegMapTreeItem* getRootItem() const
#### RegMapTreeItem* rootItem() const
Returns a pointer to the root `RegMapTreeItem`.

#### void setRootItem(RegMapTreeItem* item)
Replaces the active root item with `item` inside a `beginResetModel()` / `endResetModel()` block, refreshing all attached views.

#### RegMapTreeItem* getItem(const QModelIndex &index) const
#### RegMapTreeItem* item(const QModelIndex &index) const
Returns the `RegMapTreeItem` corresponding to `index` (or `m_rootItem` if `index` is invalid).

#### bool insertRows(int position, int rows, RegMapTreeItem::e_rmmKind kind, QModelIndex parent = QModelIndex())
Inserts `rows` children of type `kind` at row `position` under `parent`.

#### void initRow(int row, QModelIndex index)
Initializes newly inserted rows with default column values based on node type.

#### bool clear()
Clears the entire tree model, replacing it with a fresh root item.

#### void recursiveCheckData(RegMapTreeItem *node, uint32_t regWidth, QStringList &errors)
Recursively validates `node` and all descendants against architectural design rules and ASIC linting constraints:
- **Address & Field Overlap**: Register boundary collisions within blocks, bitfield overlap within registers, and exceeding `regWidth` (32 or 64-bit).
- **Reserved Identifier Keywords**: Flags block, register, and field names matching reserved SystemVerilog/Verilog or C/C++ keywords (e.g. `logic`, `wire`, `reg`, `module`, `assign`, `auto`, `int`, etc.), highlighting column 3 (`Name`).
- **Reset Value Overflow**: Flags bitfield reset values exceeding the maximum capacity `(1ULL << width) - 1`, highlighting column 6 (`Reset Value`).
- **Contradictory Access Policies**: Flags un-readable / un-drivable black hole fields where software is write-only (`SW=WO`) and hardware is write-only (`HW=WO`), highlighting columns 4 & 5.
- **Reset Consistency**: Flags non-zero reset values when `Has Reset` is disabled (`false`), highlighting columns 6 (`Reset Value`) and 9 (`Has Reset`).
Appends error descriptions to `errors` and registers invalid cells in `m_invalidCells`.

#### QStringList checkData(uint32_t regWidth = 32)
Runs a full architectural validation check over all nodes using register bit width `regWidth` (typically 32 or 64). Returns the complete list of validation error strings.

#### bool isIndexInvalid(const QModelIndex &index) const
Returns `true` if the cell at `index` has failed validation checks (used by `data()` to paint light red backgrounds `#FFC8C8`).

#### json recursiveExtractJsonData(RegMapTreeItem *node, uint32_t regWidth = 32)
Recursively converts `node` and its descendants into structured `nlohmann::json` objects containing sorted `blocks`, `registers`, `fields`, and `memories` arrays. Computes `pad_bytes_before` and `pad_words_before` for non-contiguous register offsets to enable accurate C struct memory-mapped padding.

#### json extractJsonData(uint32_t regWidth = 32)
Exports the entire tree model into a root `nlohmann::json` object formatted for Inja code generation templates. Computes deterministic IEEE 802.3 CRC32 checksums:
- `regmap_crc32`: 32-bit unsigned integer checksum of the entire register model.
- `regmap_crc32_hex`: Hexadecimal string representation (`0x...`).
- `blk["crc32"]` / `blk["crc32_hex"]`: Block-level checksums.

#### static uint32_t calculateCrc32(const uint8_t *data, size_t length, uint32_t previousCrc32 = 0)
Computes IEEE 802.3 CRC32 over the supplied byte buffer using a precomputed 256-entry lookup table.

#### static uint32_t computeBlockCrc32(const json &blkJson)
Computes deterministic CRC32 for a single register block and its registers/fields.

#### static uint32_t computeTreeCrc32(const json &rootJson)
Computes deterministic CRC32 across all blocks and registers in the tree.

### 6. Protected Virtual Methods / Model Contract Overrides

#### QVariant data(const QModelIndex &index, int role) const [override]
`QAbstractItemModel` override. Returns node data for `Qt::DisplayRole`, `Qt::EditRole`, `Qt::DecorationRole` (node kind icons), `Qt::BackgroundRole` (red error highlighting), and `Qt::ToolTipRole`.

#### Qt::ItemFlags flags(const QModelIndex &index) const [override]
`QAbstractItemModel` override. Returns `Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable` for valid cells.

#### QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const [override]
`QAbstractItemModel` override. Returns the 11-column header title strings.

#### QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const [override]
`QAbstractItemModel` override. Creates model indices pointing to internal `RegMapTreeItem` nodes.

#### QModelIndex parent(const QModelIndex &index) const [override]
`QAbstractItemModel` override. Resolves parent model indices in the tree hierarchy.

#### bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) [override]
`QAbstractItemModel` override. Updates cell data, formats hex strings, triggers architectural re-validation, and emits `dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole})`.

#### int rowCount(const QModelIndex &parent = QModelIndex()) const [override]
`QAbstractItemModel` override. Returns the number of child items under `parent`.

#### int columnCount(const QModelIndex &parent = QModelIndex()) const [override]
`QAbstractItemModel` override. Returns `11` columns.

#### bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex()) [override]
`QAbstractItemModel` override. Removes `rows` child items starting at `position` under `parent`.

### 7. Ownership and Lifecycle

`RegMapTreeModel` is parented to `RegMapWindow`. It owns `m_rootItem` and recursively deletes all allocated tree items upon destruction or model resets.

### 8. Thread Safety

`RegMapTreeModel` is **GUI-thread only** when attached to active Qt views.

### 9. Inter-Class Interactions

- Attached to `RegMapTreeView` in the left pane and `QTableView` in the right pane.
- Notifies views via standard `dataChanged`, `beginInsertRows`, `endInsertRows`, `beginRemoveRows`, and `endRemoveRows` signals.
- Serialized to and from disk via `FormatManager`.
- Extracted to JSON via `extractJsonData()` for consumption by `CodeGenerator`.

### 10. Usage Example

```cpp
#include "RegMapTreeModel.hpp"
#include "RegMapTreeItem.hpp"

void populateRegisterModel()
{
    RegMapTreeModel model;

    // Insert a block under root
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
    QModelIndex blkIdx = model.index(0, 0, QModelIndex());
    model.setData(model.index(0, 3, QModelIndex()), "UART_BLOCK");

    // Insert a register under the block
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, blkIdx);
    QModelIndex regIdx = model.index(0, 0, blkIdx);
    model.setData(model.index(0, 1, blkIdx), "0x0000");
    model.setData(model.index(0, 3, blkIdx), "CTRL");

    // Validate model
    QStringList errors = model.checkData(32);
}
```
