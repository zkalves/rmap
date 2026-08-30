# RegMapTreeView

### 1. Class Overview

`RegMapTreeView` is a specialized `QTreeView` subclass that manages the hierarchical navigation tree in the left pane of `RegMapWindow`. It displays register blocks (`blk`), registers (`reg`), address maps (`map`), and memories (`mem`), suppressing field items (`fld`) in favor of the right-pane register table. It customizes mouse click events to ensure robust row selection and clear focus management.

### 2. Project Structure and Dependencies

`RegMapTreeView` is declared in `src/RegMapTreeView.hpp` and implemented in `src/RegMapTreeView.cpp`.
- Instantiated directly in `ui/rmap.ui` as the primary navigation tree view.
- Interfaced with `TreeFilterProxyModel` and `RegMapTreeModel`.

Build Requirements:
- Qt 6 modules: `QtWidgets`, `QtGui`, `QtCore`

### 3. Class Hierarchy and Role

`RegMapTreeView` inherits directly from:
- `QTreeView` (Qt Widgets) — Provides tree view rendering, item selection models, header controls, and branch expansion.

### 4. Public Methods

#### explicit RegMapTreeView(QWidget* parent = nullptr)
Constructs the custom tree view.

#### ~RegMapTreeView() override = default
Destructor.

### 5. Protected Virtual Methods / Event Handlers

#### void mousePressEvent(QMouseEvent *event) [override]
`QTreeView` override. Handles left-click and right-click mouse events, ensuring valid item selection and clearing focus when clicking on empty viewport areas.

### 6. Ownership and Lifecycle

`RegMapTreeView` is owned by `RegMapWindow` through the Qt widget layout hierarchy defined in `ui/rmap.ui`.

### 7. Thread Safety

`RegMapTreeView` is **GUI-thread only**.

### 8. Inter-Class Interactions

- Displays data provided by `TreeFilterProxyModel` wrapping `RegMapTreeModel`.
- Selection changes trigger `RegMapWindow::updateFieldsTable` or `RegMapWindow::updateBlockView`.

### 9. Usage Example

```cpp
#include <QWidget>
#include <QVBoxLayout>
#include "RegMapTreeView.hpp"
#include "RegMapTreeModel.hpp"

void setupTreeView(QWidget *parent, RegMapTreeModel *model)
{
    auto *layout = new QVBoxLayout(parent);
    auto *treeView = new RegMapTreeView(parent);
    treeView->setModel(model);
    layout->addWidget(treeView);
}
```
