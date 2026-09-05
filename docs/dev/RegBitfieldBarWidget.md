# RegBitfieldBarWidget

### 1. Class Overview

`RegBitfieldBarWidget` is a custom Qt Widget providing an interactive graphical horizontal bitfield slice visualizer for 32-bit and 64-bit hardware registers. It renders continuous bit ranges from MSB down to LSB, automatically detects and styles unmapped bit gaps as 40% dark gray reserved slots with diagonal 45° micro-stripes and `RSVD` labels, color-codes mapped bitfields according to their software access policies (`RW`, `RO`, `WO`, `W1C`, etc.), displays detailed hover tooltips, and provides bidirectional selection synchronization with the fields table.

### 2. Project Structure and Dependencies

`RegBitfieldBarWidget` is declared in `src/RegBitfieldBarWidget.hpp` and implemented in `src/RegBitfieldBarWidget.cpp`.
- Relies on `RegMapTreeItem` to extract field attributes (offset LSB, width, access policy, reset value, description).
- Instantiated within `RegMapWindow` as the register view visualizer in the right pane.

Build Requirements:
- Qt 6 modules: `QtWidgets`, `QtGui`, `QtCore`

### 3. Class Hierarchy and Role

`RegBitfieldBarWidget` inherits directly from:
- `QWidget` (Qt Widgets) — Provides custom 2D painting surface (`QPainter`), mouse tracking, tooltip events, and layout sizing hints.

### 4. Auxiliary Data Structures

#### BitfieldSlice
A lightweight aggregate structure representing a single slice (mapped field or unmapped reserved gap) in the visualizer bar:

| Member | Type | Description |
|---|---|---|
| `item` | `RegMapTreeItem*` | Pointer to the underlying field tree item (`nullptr` for reserved slots). |
| `lsb` | `uint64_t` | Least significant bit index. |
| `msb` | `uint64_t` | Most significant bit index (`lsb + width - 1`). |
| `width` | `uint64_t` | Bit width of the slice. |
| `name` | `QString` | Field identifier string (or `RESERVED` for unmapped bits). |
| `access` | `QString` | Software access policy string (`RW`, `RO`, `WO`, etc.). |
| `hwAccess` | `QString` | Hardware core-side access mode (`RO`, `RW`, `WO`, etc.). |
| `resetVal` | `uint64_t` | Reset value of the field. |
| `desc` | `QString` | Documentation description string. |
| `isReserved` | `bool` | `true` if this slice represents an unmapped gap. |
| `childRow` | `int` | Row index of the field under its parent register item (-1 for reserved). |

### 5. Signals

#### void fieldClicked(int childRow)
Emitted when the user clicks on a bit slice in the bar widget. `childRow` specifies the row index of the clicked field under the parent register item (-1 if a reserved slot was clicked). Connected by `RegMapWindow` to highlight the corresponding row in the fields table.

### 6. Public Methods

#### explicit RegBitfieldBarWidget(QWidget *parent = nullptr)
Constructs the bitfield bar widget, enabling mouse tracking for dynamic hover tooltips.

#### void setRegister(RegMapTreeItem *regItem, uint32_t regWidth = 32)
Configures the widget to display the bitfields of `regItem`. Recomputes mapped slices and reserved gap regions across `regWidth` bits (typically 32 or 64) and schedules a repaint.

#### void clear()
Clears the active register reference and all computed slices, blanking the widget display.

#### void refresh()
Re-queries the current register item, recomputes all bit slices, and updates the display.

#### void setSelectedField(int childRow)
Highlights the slice corresponding to `childRow` with an active selection border.

#### const QVector<BitfieldSlice>& getSlices() const
#### const QVector<BitfieldSlice>& slices() const
Returns the vector of computed mapped and reserved bitfield slices.

#### void setColorBlindMode(bool enabled)
Enables or disables Okabe-Ito / Wong CVD barrier-free color palette with explicit textual bracket badges (`[RW]`, `[RO]`, `[WO]`, `[W1C]`).

#### bool isColorBlindMode() const
Returns `true` if color-blind mode is currently active.

### 7. Protected Virtual Methods / Event Handlers

#### void paintEvent(QPaintEvent *event) [override]
`QWidget` override. Draws the continuous register slice bar, drawing background badges, border outlines, diagonal striping for reserved slots, MSB:LSB boundary markers, field names, access badges, and active selection highlights.

#### void mouseMoveEvent(QMouseEvent *event) [override]
`QWidget` override. Updates hover index, highlights the slice under the cursor, and presents a rich HTML tooltip displaying field name, bit range, access policies, reset value, and description.

#### void mousePressEvent(QMouseEvent *event) [override]
`QWidget` override. Identifies the clicked slice and emits `fieldClicked(childRow)` upon left-click.

#### void leaveEvent(QEvent *event) [override]
`QWidget` override. Clears hover highlighting when the mouse cursor leaves the widget area.

#### QSize sizeHint() const [override]
Returns the recommended default size `(600, 68)`.

#### QSize minimumSizeHint() const [override]
Returns the minimum widget size `(320, 52)`.

### 8. Ownership and Lifecycle

`RegBitfieldBarWidget` is owned and managed by its parent widget or layout within `RegMapWindow`. It maintains raw non-owning pointers (`RegMapTreeItem*`) to tree nodes managed by `RegMapTreeModel`.

### 9. Thread Safety

`RegBitfieldBarWidget` is **GUI-thread only**.

### 10. Inter-Class Interactions

- Connected to `RegMapWindow` to receive active register updates when the user selects a register in the left tree.
- Emits `fieldClicked` connected to `RegMapWindow` to synchronize selection with `QTableView`.

### 11. Usage Example

```cpp
#include <QWidget>
#include <QVBoxLayout>
#include "RegBitfieldBarWidget.hpp"
#include "RegMapTreeItem.hpp"

void setupRegisterView(QWidget *parent, RegMapTreeItem *regItem)
{
    auto *layout = new QVBoxLayout(parent);
    auto *bitfieldBar = new RegBitfieldBarWidget(parent);
    layout->addWidget(bitfieldBar);

    // Display 32-bit register fields
    bitfieldBar->setRegister(regItem, 32);

    // Connect slice click to selection handler
    QObject::connect(bitfieldBar, &RegBitfieldBarWidget::fieldClicked, [](int childRow) {
        if (childRow >= 0) {
            // Select field in table view
        }
    });
}
```
