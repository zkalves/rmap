# UndoCommands

### 1. Overview

`UndoCommands.hpp` declares modular `QUndoCommand` subclasses that encapsulate state-altering operations in **rmap**. Integrated into `QUndoStack`, these commands enable non-destructive `Ctrl+Z` / `Ctrl+Y` undo and redo support across cell editing, node additions, and deep recursive tree node deletions.

### 2. Project Structure and Dependencies

Declared directly in header `src/UndoCommands.hpp`.
- Pushed onto `QUndoStack` by `RegMapWindow`.
- Modifies `RegMapTreeModel` and `RegMapTreeItem`.

Build Requirements:
- Qt 6 modules: `QtGui` (`QUndoCommand`), `QtCore`

### 3. Command Classes

| Class Name | Action Represented | Undo Behavior | Redo Behavior |
|---|---|---|---|
| `EditCellCommand` | Single-cell value modification | Restores `oldVal` to model cell. | Re-applies `newVal` to model cell. |
| `InsertItemCommand` | Inserting a new block, register, field, or memory | Calls `removeRows()` to delete the item. | Calls `insertRows()` to re-create the item. |
| `DeleteItemCommand` | Deleting a node and its entire subtree | Calls `insertRows()` and recursively restores all captured child nodes and column attributes. | Calls `removeRows()` to delete the item again. |

### 4. Auxiliary Data Structures

#### DeleteItemCommand::StoredNode
A recursive snapshot representation of a deleted item and its full child hierarchy:

| Member | Type | Description |
|---|---|---|
| `kind` | `RegMapTreeItem::e_rmmKind` | Node type (`blk`, `reg`, `fld`, `mem`, `map`). |
| `colData` | `QMap<QString, QVariant>` | Snapshot of all 11 column key-value pairs. |
| `children` | `QVector<StoredNode>` | Recursive list of child snapshots. |

### 5. Public Methods by Class

#### EditCellCommand::EditCellCommand(RegMapTreeModel *model, const QModelIndex &index, const QVariant &oldVal, const QVariant &newVal, QUndoCommand *parent = nullptr)
Constructs the cell edit command, setting the descriptive action text (e.g. `"Edit Name"`, `"Edit Offset/LSB"`).

#### void EditCellCommand::undo() [override]
Sets the cell value back to `m_oldVal`.

#### void EditCellCommand::redo() [override]
Sets the cell value to `m_newVal`.

#### InsertItemCommand::InsertItemCommand(RegMapTreeModel *model, RegMapTreeItem::e_rmmKind kind, int row, const QModelIndex &parentIndex, QUndoCommand *parent = nullptr)
Constructs the item insertion command.

#### void InsertItemCommand::undo() [override]
Removes the inserted row from the model.

#### void InsertItemCommand::redo() [override]
Re-inserts the item row into the model.

#### DeleteItemCommand::DeleteItemCommand(RegMapTreeModel *model, int row, const QModelIndex &parentIndex, QUndoCommand *parent = nullptr)
Constructs the item deletion command, recursively capturing the target node's complete data and child hierarchy into `m_storedData`.

#### static void DeleteItemCommand::captureItem(RegMapTreeItem *item, StoredNode &node)
Static recursive helper that copies all 11 column values and all child items into `node`.

#### static void DeleteItemCommand::restoreItem(RegMapTreeModel *model, const QModelIndex &index, const StoredNode &node)
Static recursive helper that restores column values and recreates all descendant rows under `index`.

#### void DeleteItemCommand::undo() [override]
Recreates the deleted root node and recursively restores all captured children.

#### void DeleteItemCommand::redo() [override]
Deletes the row from the model.

### 6. Ownership and Lifecycle

Command instances are allocated on the heap and pushed to `QUndoStack::push()`, which takes ownership and deletes them when the history is pruned or cleared.

### 7. Thread Safety

All undo commands are **GUI-thread only**.

### 8. Usage Example

```cpp
#include <QUndoStack>
#include "UndoCommands.hpp"

void editCellWithUndo(QUndoStack *undoStack, RegMapTreeModel *model, const QModelIndex &index, const QVariant &newVal)
{
    QVariant oldVal = model->data(index, Qt::EditRole);
    if (oldVal != newVal) {
        undoStack->push(new EditCellCommand(model, index, oldVal, newVal));
    }
}
```
