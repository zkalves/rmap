<!--
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at https://mozilla.org/MPL/2.0/.

  Copyright (c) 2026 Ezequiel Alves. All rights reserved.
-->

# Handoff & Continuation Guide: 100.00% Coverage Completion

This document provides a concise, complete status report and exact step-by-step instructions to resume and conclude the 100.00% compiler code coverage effort in a fresh session.

---

## 1. Executive Summary & Current Metrics

- **Overall Line Coverage**: **99.84% (7,621 / 7,633 lines)**.
- **Functions Coverage**: **100.00% (554 / 554 functions)**.
- **Decision Branch Coverage**: **84.12% (12,202 / 14,506 branches)**.
- **Conditions (MC/DC)**: **75.73% (4,815 / 6,358 conditions)**.
- **Calls**: **78.47% (16,204 / 20,650 calls)**.
- **Basic Blocks**: **63.22% (23,768 / 37,597 basic blocks)**.
- **CTest Verification**: **20 / 20 test suites passing (100% pass rate)**.
- **Inja Template Verification**: **15 / 15 templates passing (100% pass rate)**.

### Subsystem Breakdown

| Subsystem | Lines | Funcs | Branch | Cond | Status |
| :--- | :---: | :---: | :---: | :---: | :---: |
| **Code Generation Engine** | **100.0%** | **100.0%** | 86.0% | 76.9% | **PASSED** |
| **Core Architecture & Model** | **100.0%** | **100.0%** | 91.2% | 84.2% | **PASSED** |
| **Dialogs & Configuration** | **100.0%** | **100.0%** | 85.6% | 80.0% | **PASSED** |
| **Format Parsers & Serializers** | **100.0%** | **100.0%** | 84.7% | 77.7% | **PASSED** |
| **GUI Widgets & Main Window** | **99.5%** | **100.0%** | 81.4% | 70.6% | **In Progress** |
| **System Services & Utilities** | **100.0%** | **100.0%** | 83.5% | 75.6% | **PASSED** |

**46 out of 47 source files are at 100.00% line coverage.**
Only **1 file** (`src/RegMapWindow.cpp`) has remaining uncovered lines: **1,696 / 1,708 lines (99.30%)**, leaving exactly **12 lines** to 100.00%.

---

## 2. Exactly 12 Remaining Uncovered Lines in `src/RegMapWindow.cpp`

The exact remaining 12 lines identified via GCC 14.2 `gcov` are:

### Group A: Line 136 (`}`)
```cpp
134: class TreeFilterProxyModel : public QSortFilterProxyModel {
135:     ...
136: }
```
- **Reason**: Compiler-synthesized inline destructor closing brace of `TreeFilterProxyModel` or default virtual destructor.
- **Fix**: Add explicit virtual destructor `virtual ~TreeFilterProxyModel() override = default;` or invoke `delete proxy;` in test.

### Group B: Lines 1524–1525 (`QMessageBox::information` in `btnExport`)
```cpp
1523: } else if (report.success_files.empty()) {
1524:     QMessageBox::information(this, tr("Export Notice"),
1525:         tr("No template files were found or specified for generation.\n"
1526:            "Please check template paths in the Configuration dialog."), QMessageBox::Ok);
```
- **Reason**: In `testUncoveredEdgeCases()` step 19, `win.btnExport()` checks `m_config_window->serialize()`. To trigger `report.success_files.empty()` with `report.errors.empty()`, set `cfgWin->setTemplateFolders({"work/empty_tmpl"})` (where `work/empty_tmpl` is an empty directory) and clear `template_outputs` on `cfgWin`.

### Group C: Lines 1891–1892 (`curr = p; currSource = pSource;` in `insertChild`)
```cpp
1890:     if (p->possibleChildren().contains(kind)) { ... break; }
1891:     curr = p;
1892:     currSource = pSource;
```
- **Reason**: In `insertChild(RegMapTreeItem::e_rmmKind::blk)`, the tree view has a selection. Because `treeView->selectionModel()->selectedIndexes()` takes precedence over `currentIndex()`, select a field or register within a nested block, or call `treeView->selectionModel()->select(fieldIdx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows)` before calling `win.insertChild(RegMapTreeItem::e_rmmKind::blk)`. This causes the while loop to ascend `field -> reg -> blk -> root`, hitting lines 1891–1892.

### Group D: Lines 2119–2125 (`duplicateSelectedRegister()` fallback)
```cpp
2118: if (!proxyIndex.isValid() && m_currentRegItem) {
2119:     RegMapTreeItem *blk = m_currentRegItem->parentItem();
2120:     if (blk) {
2121:         QModelIndex blkIdx = (blk->parentItem() == m_model->getRootItem())
2122:             ? m_model->index(blk->row(), 0, QModelIndex())
2123:             : QModelIndex();
2124:         QModelIndex regSrcIdx = m_model->index(m_currentRegItem->row(), 0, blkIdx);
2125:         proxyIndex = m_treeProxy->mapFromSource(regSrcIdx);
2126:     }
2127: }
```
- **Reason**: `duplicateSelectedRegister()` checks `proxyIndex = this->treeView->currentIndex()`. Even after `clearSelection()`, `treeView->currentIndex()` in Qt may return the previously focused index unless `treeView->selectionModel()->setCurrentIndex(QModelIndex(), QItemSelectionModel::NoUpdate)` is called, or an empty selection proxy is used.

---

## 3. Quick-Start Commands for the Next Session

```bash
# 1. Clean gcov profile artifacts before running tests
find build -name "*.gcda" -delete
rm -rf work && mkdir -p work

# 2. Build target binaries
cmake --build build -j$(nproc)

# 3. Run all unit tests headlessly
QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure

# 4. Generate coverage summary and inspect uncovered lines
python3 script/generate_coverage.py --summary

# 5. Extract exact remaining lines in RegMapWindow.cpp
mkdir -p /tmp/gcov_check && cd /tmp/gcov_check
gcov -i -m -b -t -o /home/eze/projects/rmap/build/CMakeFiles/rmap_core.dir/src/RegMapWindow.cpp.o /home/eze/projects/rmap/src/RegMapWindow.cpp > /tmp/gcov_check/out.json
python3 -c '
import json
with open("/tmp/gcov_check/out.json") as f:
    d = json.load(f)
for f_entry in d.get("files", []):
    if f_entry["file"].endswith("RegMapWindow.cpp"):
        uncovered = [l["line_number"] for l in f_entry["lines"] if l["count"] == 0]
        print("Uncovered lines count:", len(uncovered))
        print("Lines:", uncovered)
'

# 6. Run comprehensive verification across all 15 output templates
make test-templates

# 7. Update Code Coverage report in README.md
make coverage-report
```
