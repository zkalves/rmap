# BlockMemoryMapWidget

### 1. Class Overview

`BlockMemoryMapWidget` is a custom Qt Widget providing a vertical stacked-block memory map diagram for register blocks and memory subsystems. It computes the absolute byte address map of all registers within a block, identifies sparse address gaps, renders unmapped address ranges as reserved regions with diagonal 45° hatching and `RESERVED` labels, and provides interactive click-to-navigate cross-probing to select registers directly in the main tree view.

### 2. Project Structure and Dependencies

`BlockMemoryMapWidget` is declared in `src/BlockMemoryMapWidget.hpp` and implemented in `src/BlockMemoryMapWidget.cpp`.
- Reads `RegMapTreeItem` instances of type `reg` and `mem` to compute address boundaries and sizes.
- Embedded inside a `QScrollArea` in `RegMapWindow` when a block item is selected.

Build Requirements:
- Qt 6 modules: `QtWidgets`, `QtGui`, `QtCore`

### 3. Class Hierarchy and Role

`BlockMemoryMapWidget` inherits directly from:
- `QWidget` (Qt Widgets) — Provides custom 2D diagram painting (`QPainter`), scroll area integration, mouse tracking, and tooltip support.

### 4. Auxiliary Data Structures

#### MemoryMapBlock
An aggregate data structure describing an address segment within the vertical memory map:

| Member | Type | Description |
|---|---|---|
| `item` | `RegMapTreeItem*` | Pointer to the register or memory tree item (`nullptr` for reserved address gaps). |
| `offset` | `uint64_t` | Base byte address offset. |
| `sizeBytes` | `uint64_t` | Length of the segment in bytes. |
| `endOffset` | `uint64_t` | Ending byte address offset (`offset + sizeBytes - 1`). |
| `name` | `QString` | Register/memory identifier or `RESERVED`. |
| `access` | `QString` | Software access policy string. |
| `desc` | `QString` | Documentation description string. |
| `isReserved` | `bool` | `true` if this address block represents an unmapped gap. |
| `childRow` | `int` | Row index of the register under its parent block (-1 for reserved gaps). |

### 5. Signals

#### void registerClicked(int childRow, RegMapTreeItem *regItem)
Emitted when the user clicks on a register block within the memory map diagram. `childRow` is the child row index in the parent block, and `regItem` is the target register node. Connected by `RegMapWindow` to select the register in the main navigation tree.

### 6. Public Methods

#### explicit BlockMemoryMapWidget(QWidget *parent = nullptr)
Constructs the memory map widget, enabling mouse tracking for interactive hover states.

#### void setBlock(RegMapTreeItem *blkItem, uint32_t regWidthBits = 32)
Loads the registers of `blkItem`, computes address ranges based on `regWidthBits` (typically 32 or 64 bits = 4 or 8 bytes per register), calculates reserved address gaps, resizes the widget to fit the scroll area, and triggers a repaint.

#### void clear()
Clears the active block reference and reset diagram blocks.

#### void refresh()
Re-queries the current block item, recalculates address segments, and updates the widget height.

#### void setSelectedRegister(int childRow)
Highlights the register block corresponding to `childRow` with an active accent border.

#### const QVector<MemoryMapBlock>& getBlocks() const
#### const QVector<MemoryMapBlock>& blocks() const
Returns the vector of computed memory map blocks and reserved gaps.

#### void setColorBlindMode(bool enabled)
Enables or disables Okabe-Ito / Wong CVD barrier-free color palette.

#### bool isColorBlindMode() const
Returns `true` if color-blind mode is enabled.

### 7. Protected Virtual Methods / Event Handlers

#### void paintEvent(QPaintEvent *event) [override]
`QWidget` override. Renders the vertical stacked address blocks, drawing address markers on the left, rectangular fill boxes with access-based colors (or diagonal hatching for reserved gaps), register names, and active selection borders.

#### void mouseMoveEvent(QMouseEvent *event) [override]
`QWidget` override. Tracks cursor position and displays HTML tooltips showing register name, address range, size in bytes, and description.

#### void mousePressEvent(QMouseEvent *event) [override]
`QWidget` override. Handles mouse clicks and emits `registerClicked(childRow, regItem)` when a valid register block is clicked.

#### void leaveEvent(QEvent *event) [override]
`QWidget` override. Clears hover highlighting when the mouse leaves the widget bounds.

#### QSize sizeHint() const [override]
Returns the calculated size hint based on the total number of blocks and gaps.

#### QSize minimumSizeHint() const [override]
Returns `(200, 150)` minimum size.

### 8. Ownership and Lifecycle

`BlockMemoryMapWidget` is hosted inside a `QScrollArea` owned by `RegMapWindow`. It holds non-owning pointers to items managed by `RegMapTreeModel`.

### 9. Thread Safety

`BlockMemoryMapWidget` is **GUI-thread only**.

### 10. Inter-Class Interactions

- Updated by `RegMapWindow::updateBlockView()` whenever a block item is selected in the main tree.
- Emits `registerClicked` connected to `RegMapWindow::navigateToRegister()` to cross-probe into the register view.

### 11. Usage Example

```cpp
#include <QScrollArea>
#include "BlockMemoryMapWidget.hpp"
#include "RegMapTreeItem.hpp"

void setupBlockMemoryView(QScrollArea *scrollArea, RegMapTreeItem *blkItem)
{
    auto *memMapWidget = new BlockMemoryMapWidget(scrollArea);
    scrollArea->setWidget(memMapWidget);
    scrollArea->setWidgetResizable(true);

    // Populate diagram with 32-bit register width
    memMapWidget->setBlock(blkItem, 32);

    // Handle cross-navigation clicks
    QObject::connect(memMapWidget, &BlockMemoryMapWidget::registerClicked,
                     [](int childRow, RegMapTreeItem *regItem) {
        if (regItem) {
            // Navigate tree view to clicked register
        }
    });
}
```
