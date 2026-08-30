# RegMapTreeItem

### 1. Class Overview

`RegMapTreeItem` is a hierarchical tree node class used by `RegMapTreeModel` to represent entities in a hardware register map. Each node has a specific kind (`root`, `blk`, `reg`, `fld`, `mem`, `map`), maintains an internal key-value data dictionary for the 11 table columns, tracks parent and child node pointers, and implements the `Serializable` interface for Protocol Buffer serialization.

### 2. Project Structure and Dependencies

`RegMapTreeItem` is declared in `src/RegMapTreeItem.hpp` and implemented in `src/RegMapTreeItem.cpp`.
- Managed within `RegMapTreeModel`.
- References `SerializationContext` and `Serializable` for round-trip serialization.

Build Requirements:
- Qt 6 modules: `QtCore`

### 3. Class Hierarchy and Role

`RegMapTreeItem` inherits from:
- `QObject` (QtCore) — Provides meta-object system capabilities and `Q_ENUM` registration.
- `Serializable` (Project) — Abstract interface providing `serialize` and `deserialize` methods.

### 4. Enumerations

#### e_rmmKind

| Value | Integer | Description |
|---|---|---|
| `root` | 0 | Root top-level container node. |
| `mem` | 1 | Memory region (e.g. SRAM/ROM block with base address and size in bytes). |
| `map` | 2 | Address map container. |
| `blk` | 3 | Register block (peripheral subsystem containing registers). |
| `reg` | 4 | Hardware register. |
| `fld` | 5 | Bitfield slice within a register. |

### 5. Public Methods

#### explicit RegMapTreeItem(e_rmmKind kind, QVariantMap &data, RegMapTreeItem *parentItem = nullptr)
Constructs a tree item of type `kind` with initial column data dictionary `data` and parent node `parentItem`.

#### explicit RegMapTreeItem()
Constructs an empty root item.

#### ~RegMapTreeItem() override
Destructor. Recursively deletes all child items using `qDeleteAll(m_childItems)`.

#### QVector<RegMapTreeItem*> getChildItems() const
#### QVector<RegMapTreeItem*> childItems() const
#### const QVector<RegMapTreeItem*>& childItemsRef() const
Returns the vector of child node pointers.

#### RegMapTreeItem *child(int row) const
Returns the child item at index `row` (or `nullptr` if out of range).

#### int childCount() const
Returns the total count of direct children.

#### int columnCount() const
Returns the number of display columns (`11`).

#### QVariant data(const QString &column) const
Returns the stored value for `column` (e.g. `"Name"`, `"Offset/LSB"`, `"Access Policy"`).

#### int row() const
Returns the zero-based index of this item within its parent's child list.

#### RegMapTreeItem *parentItem() const
Returns a pointer to the parent node (`nullptr` for root).

#### void appendChild(RegMapTreeItem *child)
Appends `child` to the list of children and sets its parent pointer to `this`.

#### bool insertChildren(e_rmmKind kind, int position, int count, const QVector<QString> &displayColumns)
Instantiates and inserts `count` children of type `kind` at `position`.

#### bool removeChildren(int position, int count)
Removes and deletes `count` children starting at `position`.

#### bool setData(const QString &column, const QVariant &value)
Updates the value for `column`. Synchronizes alias keys (`Offset`, `LSB`, `Offset/LSB`, `Size`, `Width`, `Size/Width`).

#### QVector<e_rmmKind> possibleChildren() const
#### QVector<e_rmmKind> get_possible_children() const
Returns the valid child types allowed under this node:
- `root` allows `blk`, `mem`, `map`.
- `blk` allows `reg`, `blk`, `mem`, `map`.
- `reg` allows `fld`.
- `fld`, `mem`, `map` allow no children.

#### QString icon() const
#### QString get_icon() const
Returns the Qt resource path for this item's icon (e.g. `":/icons/uvm_reg_small.png"`).

#### QString kindString() const
#### QString kindString() const
Returns the lowercase string representation of the node type (`"root"`, `"blk"`, `"reg"`, `"fld"`, `"mem"`, `"map"`).

#### e_rmmKind kind() const
#### e_rmmKind kind() const
Returns the `e_rmmKind` enum value.

#### void serialize(QVariantMap& data, SerializationContext* context) const [override]
`Serializable` override. Serializes item kind, ID, parent ID, child IDs, and column dictionary into `data`.

#### void deserialize(const QVariantMap& data, SerializationContext* context) [override]
`Serializable` override. Restores item kind, parent pointer, children list, and column dictionary from `data`.

### 6. Ownership and Lifecycle

Each `RegMapTreeItem` owns its child items. Deleting a parent node recursively destroys its entire subtree. The root item is owned by `RegMapTreeModel`.

### 7. Thread Safety

`RegMapTreeItem` is **single-threaded** and should only be accessed on the thread managing the tree model.

### 8. Inter-Class Interactions

- Nodes are read and mutated by `RegMapTreeModel`.
- Visualized by `RegBitfieldBarWidget` and `BlockMemoryMapWidget`.
- Serialized to Protocol Buffers via `SerializationContext`.

### 9. Usage Example

```cpp
#include "RegMapTreeItem.hpp"

void createHierarchy()
{
    QVariantMap rootData;
    auto *root = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::root, rootData);

    QVariantMap blkData;
    blkData["Name"] = "SPI_CTRL";
    blkData["Offset/LSB"] = "0x0000";
    auto *blk = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::blk, blkData, root);
    root->appendChild(blk);

    QVariantMap regData;
    regData["Name"] = "STATUS";
    regData["Offset/LSB"] = "0x0004";
    auto *reg = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, regData, blk);
    blk->appendChild(reg);

    delete root; // Automatically deletes blk and reg
}
```
