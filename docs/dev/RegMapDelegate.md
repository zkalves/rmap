# RegMapDelegate

### 1. Overview

`RegMapDelegate.hpp` declares a collection of custom Qt item delegates (`QStyledItemDelegate` subclasses) used by `RegMapTreeView` and `QTableView` to format, validate, and interactively edit the 11 columns of the register map model. These delegates provide regex-validated cell editing, color-coded access policy badges, 1-click cycling between common hardware/software access modes, and single-click boolean toggling.

### 2. Project Structure and Dependencies

Defined in `src/RegMapDelegate.hpp` and implemented in `src/RegMapDelegate.cpp`.
- Applied to `RegMapTreeView` (left pane) and `m_fieldsTableView` (right pane) in `RegMapWindow`.
- Uses `ThemeManager` for access policy color styling and barrier-free color-blind palettes.

Build Requirements:
- Qt 6 modules: `QtWidgets`, `QtGui`, `QtCore`

### 3. Delegate Classes and Hierarchy

All delegate classes inherit directly or indirectly from `QStyledItemDelegate` (Qt Widgets):

| Class Name | Base Class | Purpose / Column Mapping | Key Features |
|---|---|---|---|
| `RegMapDelegate` | `QStyledItemDelegate` | General string / description delegate | Base regex line edit editor creation. |
| `RegHexDecBinDelegate` | `RegMapDelegate` | `Offset/LSB`, `Size/Width`, `Reset Value` | Validates hex (`0x...`), decimal (`1234`), and binary (`0b...`) literals. Auto-pads hex offsets. |
| `RegIntDelegate` | `RegMapDelegate` | Numeric integer fields | Validates decimal integer strings. |
| `RegStrDelegate` | `RegMapDelegate` | `Name` column | Enforces valid C/C++/Verilog identifier strings (`^[a-zA-Z_][a-zA-Z0-9_]*$`). |
| `RegAccessPolicyDelegate` | `QStyledItemDelegate` | `Access Policy` (Column 4) | Paints rounded color badges (`RW`, `RO`, `WO`, `W1C`, etc.). Single-click cycles `RW` → `RO` → `WO` → `W1C` → `RW`. |
| `RegHwAccessDelegate` | `QStyledItemDelegate` | `HW Access` (Column 5) | Paints dashed border badges (`RO`, `RW`, `WO`, `NA`, etc.). Single-click cycles `RO` → `RW` → `WO` → `NA` → `RO`. |
| `RegBoolDelegate` | `QStyledItemDelegate` | `Is Rand`, `Volatile`, `Has Reset` (Cols 7, 8, 9) | Combines dropdown editor with 1-click toggling (`true` ↔ `false`). |

### 4. Free Functions

#### AccessColors getAccessPolicyColors(const QString &access, bool colorBlind)
Returns the background, border, and text `QColor` structures for a given access policy (`RW`, `RO`, `WO`, `W1C`, `W0C`, `RC`, `RS`, `W1S`, `W1`, `WO1`, `WRC`, `WRS`, `NA`). If `colorBlind` is `true`, applies the barrier-free Okabe-Ito / Wong CVD palette.

### 5. Public Methods by Class

#### RegMapDelegate::RegMapDelegate(QObject *parent = nullptr)
#### RegMapDelegate::RegMapDelegate(const QRegularExpression &regex, QObject *parent = nullptr)
Constructs the base delegate with an optional regex validator.

#### QWidget* RegMapDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const [override]
Instantiates a `QLineEdit` configured with `QRegularExpressionValidator`.

#### void RegAccessPolicyDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const [override]
Renders a rounded pill badge with policy-specific background and border colors. In color-blind mode, appends bracketed text tags (e.g. `[RW]`, `[RO]`).

#### bool RegAccessPolicyDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) [override]
Intercepts mouse release events. A left-click cycles the access policy: `RW` → `RO` → `WO` → `W1C` → `RW`.

#### void RegHwAccessDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const [override]
Renders a dashed-border badge representing core hardware accessibility.

#### bool RegHwAccessDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) [override]
Intercepts mouse release events. A left-click cycles the HW access mode: `RO` → `RW` → `WO` → `NA` → `RO`.

#### bool RegBoolDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) [override]
Intercepts mouse release events. A single left-click toggles boolean cell values between `true` and `false`.

### 6. Ownership and Lifecycle

Delegates are parented to the host view (`QTreeView` or `QTableView`) or `RegMapWindow`.

### 7. Thread Safety

All delegate classes are **GUI-thread only**.

### 8. Usage Example

```cpp
#include <QTableView>
#include "RegMapDelegate.hpp"

void applyDelegatesToTable(QTableView *table)
{
    // Column 1: Offset / LSB (Hex/Dec/Bin)
    table->setItemDelegateForColumn(1, new RegHexDecBinDelegate(table));

    // Column 3: Name (Valid identifier)
    table->setItemDelegateForColumn(3, new RegStrDelegate(table));

    // Column 4: SW Access Policy (1-click cycle + badge)
    table->setItemDelegateForColumn(4, new RegAccessPolicyDelegate(table));

    // Column 5: HW Access Mode (1-click cycle + dashed badge)
    table->setItemDelegateForColumn(5, new RegHwAccessDelegate(table));

    // Columns 7, 8, 9: Booleans (1-click toggle)
    table->setItemDelegateForColumn(7, new RegBoolDelegate(table));
    table->setItemDelegateForColumn(8, new RegBoolDelegate(table));
    table->setItemDelegateForColumn(9, new RegBoolDelegate(table));
}
```
