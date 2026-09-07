/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include <QtTest>
#include <QAction>
#include <QTableView>
#include <QTreeView>
#include <QStackedWidget>
#include <QTableWidget>
#include "RegMapWindow.hpp"
#include "BlockMemoryMapWidget.hpp"
#include "AppSettings.hpp"
#include "format/FormatManager.hpp"

static QtMessageHandler s_originalHandler = nullptr;
static void testOffscreenMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (type == QtWarningMsg && msg.contains("This plugin does not support")) {
        return;
    }
    if (s_originalHandler) {
        s_originalHandler(type, context, msg);
    }
}

class TestRegMapWindow : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        s_originalHandler = qInstallMessageHandler(testOffscreenMessageHandler);
        QDir("work").removeRecursively();
        QDir("examples/work").removeRecursively();
        QDir().mkpath("work");
    }
    void cleanupTestCase() {
        QDir("work").removeRecursively();
        QDir("examples/work").removeRecursively();
        qInstallMessageHandler(s_originalHandler);
    }
    void testWindowInitAndFileOpen();
    void testFileNewReset();
    void testFileClose();
    void testLeftPaneEmptyStateWhenNoModel();
    void testProxyFilteringAndSelectionSync();
    void testRegisterSortingByOffset();
    void testBlockMemoryMapView();
    void testItemCreationActions();
    void testItemDeletionAction();
    void testRegisterDuplicationAction();
    void testExportAction();
    void testExportSkipsDisabledTemplates();
    void testBitfieldBarWidgetSync();
    void testSearchBarFiltering();
    void testUndoRedoStack();
    void testHeadlessCliMethods();
    void testSarifAndJunitLintReports();
    void testSemanticDiffWithModifications();
    void testColorBlindModeToggle();
    void testKeyBindingsDialog();
    void testConfigWindowAction();
    void testPreferencesWindowAction();
    void testAboutWindowAction();
    void testMenuStructure();
    void testColorSchemeSwitching();
    void testMainWindowSizePersistence();
};

void TestRegMapWindow::testWindowInitAndFileOpen()
{
    QString file = "examples/rmt/peripherals/spi.rmt";
    RegMapWindow window(file);
    window.show();

    auto *treeView = window.findChild<QTreeView*>("treeView");
    QVERIFY(treeView != nullptr);
    QVERIFY(treeView->model() != nullptr);
    QVERIFY(treeView->model()->rowCount() > 0);
}

void TestRegMapWindow::testFileNewReset()
{
    // 1. Fresh window without a file (New Project)
    RegMapWindow freshWindow;
    auto *freshRightStacked = freshWindow.findChild<QStackedWidget*>("rightStackedWidget");
    auto *freshEmpty = freshWindow.findChild<QWidget*>("emptyViewWidget");
    auto *freshRegView = freshWindow.findChild<QWidget*>("regViewWidget");
    auto *freshLeftStacked = freshWindow.findChild<QStackedWidget*>("leftStackedWidget");
    auto *freshLeftEmpty = freshWindow.findChild<QWidget*>("leftEmptyWidget");
    auto *freshLeftView = freshWindow.findChild<QWidget*>("leftViewWidget");
    auto *freshTreeView = freshWindow.findChild<QTreeView*>("treeView");
    auto *freshSearchBox = freshWindow.findChild<QLineEdit*>("searchEdit");
    QVERIFY(freshRightStacked != nullptr);
    QVERIFY(freshEmpty != nullptr);
    QVERIFY(freshRegView != nullptr);
    QVERIFY(freshLeftStacked != nullptr);
    QVERIFY(freshLeftEmpty != nullptr);
    QVERIFY(freshLeftView != nullptr);
    QVERIFY(freshTreeView != nullptr);
    QVERIFY(freshSearchBox != nullptr);
    QCOMPARE(freshRightStacked->currentWidget(), freshEmpty);
    QVERIFY(freshRightStacked->currentWidget() != freshRegView);
    QCOMPARE(freshLeftStacked->currentWidget(), freshLeftEmpty);
    QVERIFY(freshLeftStacked->currentWidget() != freshLeftView);
    QCOMPARE(freshTreeView->model()->rowCount(), 0);
    QVERIFY(freshSearchBox->text().isEmpty());
    QVERIFY(!freshWindow.isModelLoaded());

    // 2. Open file and then trigger File -> New
    QString file = "examples/rmt/peripherals/spi.rmt";
    RegMapWindow window(file);

    auto *treeView = window.findChild<QTreeView*>("treeView");
    auto *searchBox = window.findChild<QLineEdit*>("searchEdit");
    auto *stacked = window.findChild<QStackedWidget*>("rightStackedWidget");
    auto *emptyWidget = window.findChild<QWidget*>("emptyViewWidget");
    auto *regView = window.findChild<QWidget*>("regViewWidget");
    auto *leftStacked = window.findChild<QStackedWidget*>("leftStackedWidget");
    auto *leftEmptyWidget = window.findChild<QWidget*>("leftEmptyWidget");
    auto *leftViewWidget = window.findChild<QWidget*>("leftViewWidget");
    QVERIFY(treeView != nullptr);
    QVERIFY(searchBox != nullptr);
    QVERIFY(stacked != nullptr);
    QVERIFY(emptyWidget != nullptr);
    QVERIFY(regView != nullptr);
    QVERIFY(leftStacked != nullptr);
    QVERIFY(leftEmptyWidget != nullptr);
    QVERIFY(leftViewWidget != nullptr);
    QCOMPARE(leftStacked->currentWidget(), leftViewWidget);
    QVERIFY(window.isModelLoaded());

    // Set search filter before creating new
    searchBox->setText("CTRL");
    QVERIFY(!searchBox->text().isEmpty());

    auto *actNew = window.findChild<QAction*>("actionFileNew");
    QVERIFY(actNew != nullptr);
    actNew->trigger();

    // After reset, tree should have 0 root rows, search bar clear, and both panes should be empty
    QCOMPARE(treeView->model()->rowCount(), 0);
    QVERIFY(treeView->selectionModel()->selectedIndexes().isEmpty());
    QVERIFY(!treeView->currentIndex().isValid());
    QVERIFY(searchBox->text().isEmpty());
    QCOMPARE(stacked->currentWidget(), emptyWidget);
    QVERIFY(stacked->currentWidget() != regView);
    QCOMPARE(leftStacked->currentWidget(), leftEmptyWidget);
    QVERIFY(leftStacked->currentWidget() != leftViewWidget);
    QVERIFY(!window.isModelLoaded());
}

void TestRegMapWindow::testFileClose()
{
    QString file = "examples/rmt/peripherals/spi.rmt";
    RegMapWindow window(file);

    auto *treeView = window.findChild<QTreeView*>("treeView");
    auto *searchBox = window.findChild<QLineEdit*>("searchEdit");
    auto *stacked = window.findChild<QStackedWidget*>("rightStackedWidget");
    auto *emptyWidget = window.findChild<QWidget*>("emptyViewWidget");
    auto *regView = window.findChild<QWidget*>("regViewWidget");
    auto *leftStacked = window.findChild<QStackedWidget*>("leftStackedWidget");
    auto *leftEmptyWidget = window.findChild<QWidget*>("leftEmptyWidget");
    auto *leftViewWidget = window.findChild<QWidget*>("leftViewWidget");
    QVERIFY(treeView != nullptr);
    QVERIFY(searchBox != nullptr);
    QVERIFY(stacked != nullptr);
    QVERIFY(emptyWidget != nullptr);
    QVERIFY(regView != nullptr);
    QVERIFY(leftStacked != nullptr);
    QVERIFY(leftEmptyWidget != nullptr);
    QVERIFY(leftViewWidget != nullptr);
    QVERIFY(treeView->model()->rowCount() > 0);
    QCOMPARE(leftStacked->currentWidget(), leftViewWidget);
    QVERIFY(window.isModelLoaded());

    // Set search filter before closing
    searchBox->setText("STATUS");
    QVERIFY(!searchBox->text().isEmpty());

    auto *actClose = window.findChild<QAction*>("actionFileClose");
    QVERIFY(actClose != nullptr);
    QCOMPARE(actClose->text(), QString("Close model"));
    QCOMPARE(actClose->shortcut(), QKeySequence("Ctrl+W"));
    actClose->trigger();

    // After closing model, tree should have 0 root rows, left pane search box clear, and both panes should show empty view
    QCOMPARE(treeView->model()->rowCount(), 0);
    QVERIFY(treeView->selectionModel()->selectedIndexes().isEmpty());
    QVERIFY(!treeView->currentIndex().isValid());
    QVERIFY(searchBox->text().isEmpty());
    QCOMPARE(stacked->currentWidget(), emptyWidget);
    QVERIFY(stacked->currentWidget() != regView);
    QCOMPARE(leftStacked->currentWidget(), leftEmptyWidget);
    QVERIFY(leftStacked->currentWidget() != leftViewWidget);
    QVERIFY(!window.isModelLoaded());
}

void TestRegMapWindow::testLeftPaneEmptyStateWhenNoModel()
{
    // Verify that when there is no model loaded, the left pane shows nothing at all
    RegMapWindow window;
    auto *leftStacked = window.leftStackedWidget();
    auto *leftEmpty = window.leftEmptyWidget();
    auto *leftView = window.leftViewWidget();
    auto *rightStacked = window.rightStackedWidget();
    auto *rightEmpty = window.emptyViewWidget();

    QVERIFY(leftStacked != nullptr);
    QVERIFY(leftEmpty != nullptr);
    QVERIFY(leftView != nullptr);
    QVERIFY(rightStacked != nullptr);
    QVERIFY(rightEmpty != nullptr);

    // Initial state: no model loaded -> both left and right panes show empty views
    QVERIFY(!window.isModelLoaded());
    QCOMPARE(leftStacked->currentWidget(), leftEmpty);
    QCOMPARE(rightStacked->currentWidget(), rightEmpty);

    // Add a block -> model is now loaded -> left pane switches to tree view
    auto *actAddBlk = window.findChild<QAction*>("actionAddRegBlock");
    QVERIFY(actAddBlk != nullptr);
    actAddBlk->trigger();

    QVERIFY(window.isModelLoaded());
    QCOMPARE(leftStacked->currentWidget(), leftView);
    QVERIFY(rightStacked->currentWidget() != rightEmpty);

    // Undo adding the block -> row count becomes 0 -> both panes switch back to empty views
    window.undoStack()->undo();
    QVERIFY(!window.isModelLoaded());
    QCOMPARE(leftStacked->currentWidget(), leftEmpty);
    QCOMPARE(rightStacked->currentWidget(), rightEmpty);

    // Redo adding the block -> model restored -> left pane switches back to tree view
    window.undoStack()->redo();
    QVERIFY(window.isModelLoaded());
    QCOMPARE(leftStacked->currentWidget(), leftView);

    // Select the restored block and delete it -> row count becomes 0 -> both panes switch back to empty views
    auto *treeView = window.findChild<QTreeView*>("treeView");
    QVERIFY(treeView != nullptr);
    QModelIndex blkIndex = treeView->model()->index(0, 0);
    treeView->setCurrentIndex(blkIndex);

    auto *actDelete = window.findChild<QAction*>("actionDeleteItem");
    QVERIFY(actDelete != nullptr);
    actDelete->trigger();
    QVERIFY(!window.isModelLoaded());
    QCOMPARE(leftStacked->currentWidget(), leftEmpty);
    QCOMPARE(rightStacked->currentWidget(), rightEmpty);

    // Open an existing file -> left pane shows tree view
    window.fileOpen("examples/rmt/peripherals/spi.rmt");
    QVERIFY(window.isModelLoaded());
    QCOMPARE(leftStacked->currentWidget(), leftView);

    // Close the file -> both panes switch to empty views
    auto *actClose = window.findChild<QAction*>("actionFileClose");
    QVERIFY(actClose != nullptr);
    actClose->trigger();
    QVERIFY(!window.isModelLoaded());
    QCOMPARE(leftStacked->currentWidget(), leftEmpty);
    QCOMPARE(rightStacked->currentWidget(), rightEmpty);
}

void TestRegMapWindow::testProxyFilteringAndSelectionSync()
{
    QString file = "examples/rmt/peripherals/spi.rmt";
    RegMapWindow window(file);
    window.show();

    auto *treeView = window.findChild<QTreeView*>("treeView");
    auto *fieldsTable = window.findChild<QTableView*>("fieldsTableView");

    QVERIFY(treeView != nullptr);
    QVERIFY(fieldsTable != nullptr);

    // Left tree should contain the block at row 0
    QModelIndex blkProxyIndex = treeView->model()->index(0, 0);
    QVERIFY(blkProxyIndex.isValid());

    // Select the block -> fields table should be blank (model detached / rootIndex cleared)
    treeView->setCurrentIndex(blkProxyIndex);
    QVERIFY(fieldsTable->model() == nullptr || fieldsTable->rootIndex() == QModelIndex());

    // Expand block and select register CTRL (row 0 of block)
    QModelIndex regProxyIndex = treeView->model()->index(0, 0, blkProxyIndex);
    QVERIFY(regProxyIndex.isValid());
    treeView->setCurrentIndex(regProxyIndex);

    // Right fields table should now be populated with fields for CTRL
    QVERIFY(fieldsTable->model() != nullptr);
    QVERIFY(fieldsTable->model()->rowCount(fieldsTable->rootIndex()) > 0);

    // Verify treeView header is "Offset" and fields table header is "LSB" to distinguish bit-level indices from byte offsets
    QCOMPARE(treeView->model()->headerData(1, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Offset"));
    QCOMPARE(fieldsTable->model()->headerData(1, Qt::Horizontal, Qt::DisplayRole).toString(), QString("LSB"));
    QCOMPARE(fieldsTable->model()->headerData(2, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Size"));

    // Verify treeView is non-editable (navigation & selection only)
    QCOMPARE(treeView->editTriggers(), QAbstractItemView::NoEditTriggers);

    // Verify register header widget and editable fields are visible
    auto *regHeader = window.findChild<QWidget*>("regHeaderWidget");
    auto *regNameEdit = window.findChild<QLineEdit*>("regNameEdit");
    auto *regOffsetEdit = window.findChild<QLineEdit*>("regOffsetEdit");
    auto *regDescEdit = window.findChild<QLineEdit*>("regDescEdit");
    QVERIFY(regHeader != nullptr);
    QVERIFY(regNameEdit != nullptr);
    QVERIFY(regOffsetEdit != nullptr);
    QVERIFY(regDescEdit != nullptr);
    QVERIFY(regHeader->isVisible());
    QCOMPARE(regNameEdit->text(), QString("CTRL"));
    QCOMPARE(regOffsetEdit->text(), QString("0x0000"));

    // Edit register name, offset (written in hex without padding), description via right panel UI
    regNameEdit->setText("CTRL_NEW");
    emit regNameEdit->editingFinished();

    regOffsetEdit->setText("0x2");
    emit regOffsetEdit->editingFinished();
    QCOMPARE(regOffsetEdit->text(), QString("0x0002"));

    regDescEdit->setText("Updated SPI Control Register description");
    emit regDescEdit->editingFinished();

    // Verify fields table stretches to window width
    QVERIFY(fieldsTable->horizontalHeader()->stretchLastSection());
    QCOMPARE(fieldsTable->horizontalHeader()->sectionResizeMode(10), QHeaderView::Stretch);

    // Verify model reflected the changes with zero-padded hex offset
    QModelIndex nameIndex = treeView->model()->index(0, 3, blkProxyIndex);
    QCOMPARE(treeView->model()->data(nameIndex, Qt::DisplayRole).toString(), QString("CTRL_NEW"));
    QModelIndex offsetIndex = treeView->model()->index(0, 1, blkProxyIndex);
    QCOMPARE(treeView->model()->data(offsetIndex, Qt::DisplayRole).toString(), QString("0x0002"));
    QModelIndex descIndex = treeView->model()->index(0, 10, blkProxyIndex);
    QCOMPARE(treeView->model()->data(descIndex, Qt::DisplayRole).toString(), QString("Updated SPI Control Register description"));
}

void TestRegMapWindow::testRegisterSortingByOffset()
{
    QString file = "examples/rmt/features/address_gap_example.rmt";
    RegMapWindow window(file);
    window.show();

    auto *treeView = window.findChild<QTreeView*>("treeView");
    QVERIFY(treeView != nullptr);

    QModelIndex blkProxy = treeView->model()->index(0, 0);
    QVERIFY(blkProxy.isValid());

    int regCount = treeView->model()->rowCount(blkProxy);
    QVERIFY(regCount >= 7);

    // Verify registers are sorted numerically ascending by Offset
    uint64_t prevOffset = 0;
    for (int r = 0; r < regCount; ++r) {
        QModelIndex offsetIdx = treeView->model()->index(r, 1, blkProxy);
        QString offsetStr = treeView->model()->data(offsetIdx, Qt::DisplayRole).toString();
        bool ok = false;
        uint64_t currentOffset = offsetStr.toULongLong(&ok, 16);
        if (!ok) currentOffset = offsetStr.toULongLong(&ok, 10);
        QVERIFY(ok);
        if (r > 0) {
            QVERIFY(currentOffset >= prevOffset);
        }
        prevOffset = currentOffset;
    }
}

void TestRegMapWindow::testBlockMemoryMapView()
{
    QString file = "examples/rmt/features/address_gap_example.rmt";
    RegMapWindow window(file);
    window.show();

    auto *treeView = window.findChild<QTreeView*>("treeView");
    auto *stackedWidget = window.findChild<QStackedWidget*>("rightStackedWidget");
    auto *blockViewWidget = window.findChild<QWidget*>("blockViewWidget");
    auto *regViewWidget = window.findChild<QWidget*>("regViewWidget");
    auto *memMapWidget = window.findChild<BlockMemoryMapWidget*>("blockMemoryMapWidget");
    auto *blkNameEdit = window.findChild<QLineEdit*>("blkNameEdit");
    auto *blkOffsetEdit = window.findChild<QLineEdit*>("blkOffsetEdit");
    auto *blkDescEdit = window.findChild<QLineEdit*>("blkDescEdit");

    QVERIFY(treeView != nullptr);
    QVERIFY(stackedWidget != nullptr);
    QVERIFY(blockViewWidget != nullptr);
    QVERIFY(regViewWidget != nullptr);
    QVERIFY(memMapWidget != nullptr);
    QVERIFY(blkNameEdit != nullptr);
    QVERIFY(blkOffsetEdit != nullptr);
    QVERIFY(blkDescEdit != nullptr);

    // 1. Select block in tree -> right pane switches to blockViewWidget
    QModelIndex blkIndex = treeView->model()->index(0, 0);
    treeView->setCurrentIndex(blkIndex);

    QCOMPARE(stackedWidget->currentWidget(), blockViewWidget);
    QCOMPARE(blkNameEdit->text(), QString("Sparse_Device"));
    QCOMPARE(blkOffsetEdit->text(), QString("0x0000"));

    // 2. Verify Memory Map Diagram has rendered blocks and detected address gaps
    const auto &blocks = memMapWidget->getBlocks();
    QVERIFY(!blocks.isEmpty());

    // Check CTRL block at 0x0000
    QCOMPARE(blocks[0].name, QString("CTRL"));
    QCOMPARE(blocks[0].offset, (uint64_t)0x0000);
    QCOMPARE(blocks[0].sizeBytes, (uint64_t)4);
    QCOMPARE(blocks[0].isReserved, false);

    // Check STATUS block at 0x0004
    QCOMPARE(blocks[1].name, QString("STATUS"));
    QCOMPARE(blocks[1].offset, (uint64_t)0x0004);
    QCOMPARE(blocks[1].sizeBytes, (uint64_t)4);
    QCOMPARE(blocks[1].isReserved, false);

    // Check first unmapped gap at 0x0008 (24 bytes gap up to 0x0020)
    QCOMPARE(blocks[2].name, QString("RESERVED"));
    QCOMPARE(blocks[2].offset, (uint64_t)0x0008);
    QCOMPARE(blocks[2].sizeBytes, (uint64_t)24);
    QCOMPARE(blocks[2].isReserved, true);

    // Check TX_FIFO at 0x0020
    QCOMPARE(blocks[3].name, QString("TX_FIFO"));
    QCOMPARE(blocks[3].offset, (uint64_t)0x0020);
    QCOMPARE(blocks[3].isReserved, false);

    // 3. Test Click-to-Navigate on Diagram: Click second register (STATUS)
    emit memMapWidget->registerClicked(1, nullptr);
    // Should navigate to register view for STATUS
    QCOMPARE(stackedWidget->currentWidget(), regViewWidget);
    auto *regNameEdit = window.findChild<QLineEdit*>("regNameEdit");
    QVERIFY(regNameEdit != nullptr);
    QCOMPARE(regNameEdit->text(), QString("STATUS"));

    // 4. Edit block name, offset, description via UI
    treeView->setCurrentIndex(blkIndex);
    QCOMPARE(stackedWidget->currentWidget(), blockViewWidget);

    blkNameEdit->setText("Sparse_Device_Updated");
    emit blkNameEdit->editingFinished();

    blkOffsetEdit->setText("0x1000");
    emit blkOffsetEdit->editingFinished();

    blkDescEdit->setText("Updated Sparse Device description");
    emit blkDescEdit->editingFinished();

    QModelIndex blkNameIdx = treeView->model()->index(0, 3);
    QCOMPARE(treeView->model()->data(blkNameIdx, Qt::DisplayRole).toString(), QString("Sparse_Device_Updated"));
    QModelIndex blkOffsetIdx = treeView->model()->index(0, 1);
    QCOMPARE(treeView->model()->data(blkOffsetIdx, Qt::DisplayRole).toString(), QString("0x1000"));
    QModelIndex descIndex = treeView->model()->index(0, 10);
    QCOMPARE(treeView->model()->data(descIndex, Qt::DisplayRole).toString(), QString("Updated Sparse Device description"));
}

void TestRegMapWindow::testItemCreationActions()
{
    qDebug() << "--- START testItemCreationActions ---";
    QString emptyFile = "";
    RegMapWindow window(emptyFile);
    window.show();

    auto *treeView = window.findChild<QTreeView*>("treeView");
    auto *leftStacked = window.leftStackedWidget();
    auto *leftEmpty = window.leftEmptyWidget();
    auto *leftView = window.leftViewWidget();

    // Before adding any item, left pane must be empty
    QCOMPARE(leftStacked->currentWidget(), leftEmpty);
    QVERIFY(!window.isModelLoaded());

    // Add Block
    auto *actAddBlk = window.findChild<QAction*>("actionAddRegBlock");
    QVERIFY(actAddBlk != nullptr);
    actAddBlk->trigger();

    // After adding block, left pane must show tree view
    QCOMPARE(leftStacked->currentWidget(), leftView);
    QVERIFY(window.isModelLoaded());

    qDebug() << "Block added, rowCount:" << treeView->model()->rowCount();
    QCOMPARE(treeView->model()->rowCount(), 1);
    QModelIndex blkIndex = treeView->model()->index(0, 0);
    treeView->setCurrentIndex(blkIndex);

    // Add Register under selected Block
    auto *actAddReg = window.findChild<QAction*>("actionAddReg");
    QVERIFY(actAddReg != nullptr);
    actAddReg->trigger();

    // Verify automatically moved/selected the newly created register
    qDebug() << "Reg added, rowCount under blk:" << treeView->model()->rowCount(blkIndex);
    QCOMPARE(treeView->model()->rowCount(blkIndex), 1);
    QCOMPARE(treeView->currentIndex().parent(), blkIndex);

    // Add another Register while the first Register is selected -> should also add it under the block and move to it
    actAddReg->trigger();
    QCOMPARE(treeView->model()->rowCount(blkIndex), 2);
    QCOMPARE(treeView->currentIndex().parent(), blkIndex);

    // Add Field under currently selected Register
    auto *actAddFld = window.findChild<QAction*>("actionAddRegField");
    QVERIFY(actAddFld != nullptr);
    actAddFld->trigger();

    auto *fieldsTable = window.findChild<QTableView*>("fieldsTableView");
    QVERIFY(fieldsTable != nullptr);
    QVERIFY(fieldsTable->model() != nullptr);
    qDebug() << "Fld added, fieldsTable rowCount:" << fieldsTable->model()->rowCount(fieldsTable->rootIndex());
    QCOMPARE(fieldsTable->model()->rowCount(fieldsTable->rootIndex()), 1);

    // Add Memory at root level
    treeView->selectionModel()->clearSelection();
    treeView->setCurrentIndex(QModelIndex());
    auto *actAddMem = window.findChild<QAction*>("actionAddMem");
    QVERIFY(actAddMem != nullptr);
    actAddMem->trigger();

    qDebug() << "Mem added, root rowCount:" << treeView->model()->rowCount();
    QCOMPARE(treeView->model()->rowCount(), 2);
    qDebug() << "--- END testItemCreationActions ---";
}

void TestRegMapWindow::testItemDeletionAction()
{
    qDebug() << "--- START testItemDeletionAction ---";
    QString emptyFile = "";
    RegMapWindow window(emptyFile);

    auto *treeView = window.findChild<QTreeView*>("treeView");
    auto *leftStacked = window.leftStackedWidget();
    auto *leftEmpty = window.leftEmptyWidget();
    auto *leftView = window.leftViewWidget();
    auto *rightStacked = window.rightStackedWidget();
    auto *rightEmpty = window.emptyViewWidget();

    // Initially empty
    QCOMPARE(leftStacked->currentWidget(), leftEmpty);
    QCOMPARE(rightStacked->currentWidget(), rightEmpty);
    QVERIFY(!window.isModelLoaded());

    auto *actAddBlk = window.findChild<QAction*>("actionAddRegBlock");
    actAddBlk->trigger();

    QCOMPARE(treeView->model()->rowCount(), 1);
    QCOMPARE(leftStacked->currentWidget(), leftView);
    QVERIFY(window.isModelLoaded());

    QModelIndex blkIndex = treeView->model()->index(0, 0);
    treeView->setCurrentIndex(blkIndex);

    auto *actDelete = window.findChild<QAction*>("actionDeleteItem");
    QVERIFY(actDelete != nullptr);
    actDelete->trigger();

    qDebug() << "Item deleted, rowCount:" << treeView->model()->rowCount();
    QCOMPARE(treeView->model()->rowCount(), 0);
    // After deleting all items, left and right panes must show empty views
    QCOMPARE(leftStacked->currentWidget(), leftEmpty);
    QCOMPARE(rightStacked->currentWidget(), rightEmpty);
    QVERIFY(!window.isModelLoaded());
    qDebug() << "--- END testItemDeletionAction ---";
}

void TestRegMapWindow::testRegisterDuplicationAction()
{
    qDebug() << "--- START testRegisterDuplicationAction ---";
    QString file = "examples/rmt/peripherals/spi.rmt";
    RegMapWindow window(file);

    auto *treeView = window.findChild<QTreeView*>("treeView");
    QVERIFY(treeView != nullptr);

    auto *actDup = window.findChild<QAction*>("actionDuplicate");
    QVERIFY(actDup != nullptr);
    QCOMPARE(actDup->shortcut(), QKeySequence("Ctrl+D"));

    // In spi.rmt, root child 0 is BLK (e.g. SPI)
    QModelIndex blkIndex = treeView->model()->index(0, 0);
    QVERIFY(blkIndex.isValid());

    // Expand blk
    treeView->setExpanded(blkIndex, true);
    int initialRegCount = treeView->model()->rowCount(blkIndex);
    QVERIFY(initialRegCount > 0);

    // Select the first register (e.g. CTRL)
    QModelIndex reg0Index = treeView->model()->index(0, 0, blkIndex);
    treeView->setCurrentIndex(reg0Index);
    QString origName = treeView->model()->data(treeView->model()->index(0, 3, blkIndex)).toString();

    // Trigger duplicate
    actDup->trigger();

    // Verify rowCount of block increased by 1
    int newRegCount = treeView->model()->rowCount(blkIndex);
    QCOMPARE(newRegCount, initialRegCount + 1);

    // Verify duplicated register properties
    QModelIndex dupRegIndex = treeView->model()->index(1, 0, blkIndex);
    QString dupName = treeView->model()->data(treeView->model()->index(1, 3, blkIndex)).toString();
    QCOMPARE(dupName, origName + "_COPY");

    // Select duplicate register -> fieldsTable shows its fields
    auto *fieldsTable = window.findChild<QTableView*>("fieldsTableView");
    QVERIFY(fieldsTable != nullptr);
    treeView->setCurrentIndex(dupRegIndex);
    QVERIFY(fieldsTable->model() != nullptr);
    QVERIFY(fieldsTable->model()->rowCount(fieldsTable->rootIndex()) > 0);

    // Verify Undo
    auto *actUndo = window.findChild<QAction*>("actionUndo");
    QVERIFY(actUndo != nullptr);
    actUndo->trigger();
    QCOMPARE(treeView->model()->rowCount(blkIndex), initialRegCount);

    // Verify Redo
    auto *actRedo = window.findChild<QAction*>("actionRedo");
    QVERIFY(actRedo != nullptr);
    actRedo->trigger();
    QCOMPARE(treeView->model()->rowCount(blkIndex), newRegCount);

    qDebug() << "--- END testRegisterDuplicationAction ---";
}

void TestRegMapWindow::testExportAction()
{
    QString file = "examples/rmt/peripherals/spi.rmt";
    RegMapWindow window(file);

    // Re-anchor config output to work/
    window.configWindow()->setBaseDir(QDir::currentPath());
    protormap::Config cfg;
    cfg.set_templatefolder("./templates");
    cfg.set_outputfolder("./work");
    auto *entry1 = cfg.add_template_outputs();
    entry1->set_template_filename("templates/c/reg_map.h.inja");
    entry1->set_output_filepath("work/c/reg_map.h");
    auto *entry2 = cfg.add_template_outputs();
    entry2->set_template_filename("templates/uvm/reg_model.sv.inja");
    entry2->set_output_filepath("work/uvm/reg_model.sv");
    window.configWindow()->deserialize(cfg);

    // Dismiss modal QMessageBox automatically when triggered
    QTimer *dismissTimer = new QTimer(&window);
    QObject::connect(dismissTimer, &QTimer::timeout, []() {
        QWidget* modal = QApplication::activeModalWidget();
        if (modal) {
            modal->close();
        }
    });
    QFile::remove("work/c/reg_map.h");
    QFile::remove("work/uvm/reg_model.sv");
    dismissTimer->start(50);

    auto *actExport = window.findChild<QAction*>("actionExport");
    QVERIFY(actExport != nullptr);
    actExport->trigger();

    // Verify generated files exist in work/
    QVERIFY(QFile::exists("work/c/reg_map.h"));
    QVERIFY(QFile::exists("work/uvm/reg_model.sv"));

    dismissTimer->stop();
}

void TestRegMapWindow::testExportSkipsDisabledTemplates()
{
    QString file = "examples/rmt/peripherals/spi.rmt";
    RegMapWindow window(file);

    window.configWindow()->setBaseDir(QDir::currentPath());
    protormap::Config cfg;
    cfg.set_templatefolder("./templates");
    cfg.set_outputfolder("./work");
    auto *entry1 = cfg.add_template_outputs();
    entry1->set_template_filename("templates/c/reg_map.h.inja");
    entry1->set_output_filepath("work/c/reg_map.h");
    entry1->set_enabled(true);

    auto *entry2 = cfg.add_template_outputs();
    entry2->set_template_filename("templates/uvm/reg_model.sv.inja");
    entry2->set_output_filepath("work/uvm/reg_model.sv");
    entry2->set_enabled(false); // DISABLED
    window.configWindow()->deserialize(cfg);

    QTimer *dismissTimer = new QTimer(&window);
    QObject::connect(dismissTimer, &QTimer::timeout, []() {
        QWidget* modal = QApplication::activeModalWidget();
        if (modal) {
            modal->close();
        }
    });
    QFile::remove("work/c/reg_map.h");
    QFile::remove("work/uvm/reg_model.sv");
    dismissTimer->start(50);

    auto *actExport = window.findChild<QAction*>("actionExport");
    QVERIFY(actExport != nullptr);
    actExport->trigger();

    // Verify enabled file was generated and disabled file was NOT generated
    QVERIFY(QFile::exists("work/c/reg_map.h"));
    QVERIFY(!QFile::exists("work/uvm/reg_model.sv"));

    dismissTimer->stop();
}

void TestRegMapWindow::testBitfieldBarWidgetSync()
{
    QString file = "examples/rmt/peripherals/spi.rmt";
    RegMapWindow window(file);
    window.show();

    auto *treeView = window.findChild<QTreeView*>("treeView");
    auto *bitfieldBar = window.findChild<RegBitfieldBarWidget*>();
    QVERIFY(treeView != nullptr);
    QVERIFY(bitfieldBar != nullptr);

    // Select register CTRL (row 0 of block 0)
    QModelIndex blkProxyIndex = treeView->model()->index(0, 0);
    QModelIndex regProxyIndex = treeView->model()->index(0, 0, blkProxyIndex);
    treeView->setCurrentIndex(regProxyIndex);

    // Bitfield bar should be visible and have slices
    QVERIFY(!bitfieldBar->getSlices().empty());

    // Find the slice corresponding to child row 0 (EN)
    int enSliceIdx = -1;
    for (int i = 0; i < bitfieldBar->getSlices().size(); ++i) {
        if (!bitfieldBar->getSlices()[i].isReserved && bitfieldBar->getSlices()[i].childRow == 0) {
            enSliceIdx = i;
            break;
        }
    }
    QVERIFY(enSliceIdx >= 0);
    QCOMPARE(bitfieldBar->getSlices()[enSliceIdx].access, QString("RW"));

    // Mutate field access policy to "W1C" in the model
    auto *model = window.getModel();
    QModelIndex blkSourceIdx = model->index(0, 0, QModelIndex());
    QModelIndex regSourceIdx = model->index(0, 0, blkSourceIdx);
    QModelIndex fldAccessIdx = model->index(0, 4, regSourceIdx); // Col 4: SW Access Policy
    model->setData(fldAccessIdx, "W1C", Qt::EditRole);

    // Verify bitfield bar immediately updated to W1C
    QCOMPARE(bitfieldBar->getSlices()[enSliceIdx].access, QString("W1C"));

    // Mutate to "RO"
    model->setData(fldAccessIdx, "RO", Qt::EditRole);
    QCOMPARE(bitfieldBar->getSlices()[enSliceIdx].access, QString("RO"));

    // Test Bidirectional Cross-Probing:
    auto *fieldsTable = window.findChild<QTableView*>("fieldsTableView");
    QVERIFY(fieldsTable != nullptr);

    // 1. Click slice in bitfieldBar -> selects row in fieldsTable
    emit bitfieldBar->fieldClicked(0);
    QVERIFY(fieldsTable->selectionModel() != nullptr);
    QVERIFY(fieldsTable->selectionModel()->hasSelection());
    QVERIFY(fieldsTable->selectionModel()->isRowSelected(fieldsTable->currentIndex().row(), fieldsTable->rootIndex()));

    // 2. Select row in fieldsTable -> highlights slice in bitfieldBar
    fieldsTable->selectRow(1);
    QVERIFY(bitfieldBar->getSlices().size() > 0);
}

void TestRegMapWindow::testSearchBarFiltering()
{
    QString file = "examples/rmt/peripherals/spi.rmt";
    RegMapWindow window(file);
    window.show();

    auto *treeView = window.findChild<QTreeView*>("treeView");
    auto *searchBox = window.findChild<QLineEdit*>();
    QVERIFY(treeView != nullptr);
    QVERIFY(searchBox != nullptr);

    // Filter by "STATUS"
    searchBox->setText("STATUS");
    QVERIFY(treeView->model()->rowCount() > 0);

    // Clear filter
    searchBox->clear();
    QCOMPARE(treeView->model()->rowCount(), 1);
}

void TestRegMapWindow::testUndoRedoStack()
{
    QString file = "examples/rmt/peripherals/spi.rmt";
    RegMapWindow window(file);

    QUndoStack *undoStack = window.getUndoStack();
    QVERIFY(undoStack != nullptr);
    int initialCount = undoStack->count();

    // Trigger an edit command on model via UndoStack
    RegMapTreeModel *model = window.getModel();
    QModelIndex blkIndex = model->index(0, 3, QModelIndex()); // Name of block
    QString oldVal = model->data(blkIndex, Qt::DisplayRole).toString();

    undoStack->push(new EditCellCommand(model, blkIndex, oldVal, "SPI_RENAMED"));
    QCOMPARE(model->data(blkIndex, Qt::DisplayRole).toString(), QString("SPI_RENAMED"));
    QCOMPARE(undoStack->count(), initialCount + 1);

    // Undo
    undoStack->undo();
    QCOMPARE(model->data(blkIndex, Qt::DisplayRole).toString(), oldVal);

    // Redo
    undoStack->redo();
    QCOMPARE(model->data(blkIndex, Qt::DisplayRole).toString(), QString("SPI_RENAMED"));
}

void TestRegMapWindow::testHeadlessCliMethods()
{
    QString file = "examples/rmt/peripherals/spi.rmt";
    RegMapWindow window(file);

    // 1. Headless lint
    bool lintPass = window.headlessLint(false, "text", "");
    QVERIFY(lintPass);

    bool lintJsonPass = window.headlessLint(false, "json", "work/test_lint_out.json");
    QVERIFY(lintJsonPass);
    QVERIFY(QFile::exists("work/test_lint_out.json"));

    // 2. Semantic diff against itself (0 changes)
    bool diffPass = RegMapWindow::semanticDiff("examples/rmt/peripherals/spi.rmt", "examples/rmt/peripherals/spi.rmt", "text", "");
    QVERIFY(diffPass);

    // 3. Headless export
    bool exportPass = window.headlessExport("work/test_headless_export");
    QVERIFY(exportPass);
    QVERIFY(QFile::exists("work/test_headless_export/c/reg_map.h"));
    QVERIFY(QFile::exists("work/test_headless_export/uvm/reg_model.sv"));

    // 4. Headless methods with environment variables
    qputenv("RMAP_TEST_SPI", "examples/rmt/peripherals/spi.rmt");
    qputenv("RMAP_TEST_OUT_DIR", "work/test_env_export");

    RegMapWindow envWindow("$RMAP_TEST_SPI");
    bool envExportPass = envWindow.headlessExport("$RMAP_TEST_OUT_DIR");
    QVERIFY(envExportPass);
    QVERIFY(QFile::exists("work/test_env_export/c/reg_map.h"));
    QVERIFY(QFile::exists("work/test_env_export/uvm/reg_model.sv"));

    bool envDiffPass = RegMapWindow::semanticDiff("$RMAP_TEST_SPI", "$RMAP_TEST_SPI", "text", "");
    QVERIFY(envDiffPass);

    qunsetenv("RMAP_TEST_SPI");
    qunsetenv("RMAP_TEST_OUT_DIR");
}

void TestRegMapWindow::testSarifAndJunitLintReports()
{
    // 1. Test clean file in SARIF format
    RegMapWindow cleanWindow("examples/rmt/peripherals/spi.rmt");
    QString sarifOut = "work/test_spi.sarif";
    bool cleanSarifPass = cleanWindow.headlessLint(false, "sarif", sarifOut);
    QVERIFY(cleanSarifPass);
    QVERIFY(QFile::exists(sarifOut));

    QFile fSarif(sarifOut);
    const bool sarifOpened = fSarif.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(sarifOpened);
    QJsonDocument sarifDoc = QJsonDocument::fromJson(fSarif.readAll());
    fSarif.close();
    QVERIFY(!sarifDoc.isNull());
    QJsonObject sarifObj = sarifDoc.object();
    QCOMPARE(sarifObj["version"].toString(), QString("2.1.0"));
    QVERIFY(sarifObj.contains("runs"));
    QJsonArray runs = sarifObj["runs"].toArray();
    QCOMPARE(runs.size(), 1);
    QJsonObject run0 = runs[0].toObject();
    QCOMPARE(run0["tool"].toObject()["driver"].toObject()["name"].toString(), QString("rmap-lint"));

    // 2. Test clean file in JUnit XML format
    QString junitOut = "work/test_spi.xml";
    bool cleanJunitPass = cleanWindow.headlessLint(false, "junit", junitOut);
    QVERIFY(cleanJunitPass);
    QVERIFY(QFile::exists(junitOut));

    QFile fJunit(junitOut);
    const bool junitOpened = fJunit.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(junitOpened);
    QString junitContent = fJunit.readAll();
    fJunit.close();
    QVERIFY(junitContent.contains("<testsuites name=\"rmap-lint\""));
    QVERIFY(junitContent.contains("<testsuite name=\"RegisterMapValidation\""));
    QVERIFY(junitContent.contains("<testcase name=\"AddressAndOverlapCheck\"/>"));
    QVERIFY(!junitContent.contains("<failure"));

    // 3. Test invalid file with strict rules
    RegMapWindow invalidWindow("examples/rmt/validation/invalid_overlap.rmt");
    QString strictSarifOut = "work/test_invalid_strict.sarif";
    bool invalidStrictPass = invalidWindow.headlessLint(true, "sarif", strictSarifOut);
    QVERIFY(!invalidStrictPass); // Expected to fail validation
    QVERIFY(QFile::exists(strictSarifOut));

    QFile fStrictSarif(strictSarifOut);
    const bool strictSarifOpened = fStrictSarif.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(strictSarifOpened);
    QJsonDocument strictDoc = QJsonDocument::fromJson(fStrictSarif.readAll());
    fStrictSarif.close();
    QVERIFY(!strictDoc.isNull());
    QJsonArray strictResults = strictDoc.object()["runs"].toArray()[0].toObject()["results"].toArray();
    QVERIFY(strictResults.size() > 0);
}

void TestRegMapWindow::testSemanticDiffWithModifications()
{
    // Create a modified copy of spi.rmt
    RegMapTreeModel model;
    RegConfigWindow config;
    QVERIFY(FormatManager::instance().loadFile("examples/rmt/peripherals/spi.rmt", &model, &config).success);

    // Add a new register TX_BUFFER to Block 0
    QModelIndex blkIndex = model.index(0, 0, QModelIndex());
    QVERIFY(model.insertRows(2, 1, RegMapTreeItem::e_rmmKind::reg, blkIndex));
    model.setData(model.index(2, 1, blkIndex), "0x0008", Qt::EditRole);
    model.setData(model.index(2, 3, blkIndex), "TX_BUFFER", Qt::EditRole);
    model.setData(model.index(2, 4, blkIndex), "WO", Qt::EditRole);

    // Save modified model
    QString modFile = "work/test_diff_modified.rmt";
    QVERIFY(FormatManager::instance().saveFile(modFile, &model, &config).success);

    // 1. Run semantic diff in Markdown format
    QString mdOut = "work/test_diff_report.md";
    bool mdDiffPass = RegMapWindow::semanticDiff("examples/rmt/peripherals/spi.rmt", modFile, "markdown", mdOut);
    QVERIFY(mdDiffPass);
    QVERIFY(QFile::exists(mdOut));

    QFile fMd(mdOut);
    const bool mdOpened = fMd.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(mdOpened);
    QString mdContent = fMd.readAll();
    fMd.close();
    QVERIFY(mdContent.contains("# Register Map Diff"));
    QVERIFY(mdContent.contains("➕ Added Registers"));
    QVERIFY(mdContent.contains("TX_BUFFER"));

    // 2. Run semantic diff in Text format
    QString txtOut = "work/test_diff_report.txt";
    bool txtDiffPass = RegMapWindow::semanticDiff("examples/rmt/peripherals/spi.rmt", modFile, "text", txtOut);
    QVERIFY(txtDiffPass);
    QVERIFY(QFile::exists(txtOut));

    QFile fTxt(txtOut);
    const bool txtOpened = fTxt.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(txtOpened);
    QString txtContent = fTxt.readAll();
    fTxt.close();
    QVERIFY(txtContent.contains("=== Register Map Diff"));
    QVERIFY(txtContent.contains("+ ADDED:"));
    QVERIFY(txtContent.contains("TX_BUFFER"));
}

void TestRegMapWindow::testColorBlindModeToggle()
{
    QString file = "examples/rmt/peripherals/spi.rmt";
    RegMapWindow window(file);
    window.show();

    auto *bitfieldBar = window.findChild<RegBitfieldBarWidget*>();
    QVERIFY(bitfieldBar != nullptr);
    QCOMPARE(window.isColorBlindMode(), false);
    QCOMPARE(bitfieldBar->isColorBlindMode(), false);

    // Toggle color blind mode ON
    auto *actColorBlind = window.findChild<QAction*>("actionColorBlindMode");
    QVERIFY(actColorBlind != nullptr);
    actColorBlind->trigger();

    QCOMPARE(window.isColorBlindMode(), true);
    QCOMPARE(bitfieldBar->isColorBlindMode(), true);

    // Toggle back OFF
    actColorBlind->trigger();
    QCOMPARE(window.isColorBlindMode(), false);
    QCOMPARE(bitfieldBar->isColorBlindMode(), false);
}

void TestRegMapWindow::testKeyBindingsDialog()
{
    QString file = "examples/rmt/peripherals/spi.rmt";
    RegMapWindow window(file);
    window.show();

    auto *actKeyBindings = window.findChild<QAction*>("actionKeyboardShortcuts");
    QVERIFY(actKeyBindings != nullptr);

    // Auto-dismiss the key bindings modal dialog
    QTimer *dismissTimer = new QTimer(&window);
    QObject::connect(dismissTimer, &QTimer::timeout, []() {
        QWidget* modal = QApplication::activeModalWidget();
        if (modal) {
            modal->close();
        }
    });
    dismissTimer->start(50);

    actKeyBindings->trigger();
    dismissTimer->stop();
}

void TestRegMapWindow::testConfigWindowAction()
{
    QString file = "examples/rmt/peripherals/spi.rmt";
    RegMapWindow window(file);
    window.show();

    auto *actConfig = window.findChild<QAction*>("actionConfig");
    QVERIFY(actConfig != nullptr);
    actConfig->trigger();

    auto *cfgWin = window.findChild<RegConfigWindow*>();
    QVERIFY(cfgWin != nullptr);
    QVERIFY(cfgWin->isVisible());
    QVERIFY(!cfgWin->isModal());
    QVERIFY(!cfgWin->isSizeGripEnabled());

    cfgWin->accept();
    QVERIFY(!cfgWin->isVisible());
}

void TestRegMapWindow::testPreferencesWindowAction()
{
    QString file = "examples/rmt/peripherals/spi.rmt";
    RegMapWindow window(file);
    window.show();

    auto *actPref = window.findChild<QAction*>("actionPreferences");
    QVERIFY(actPref != nullptr);
    actPref->trigger();

    auto *prefWin = window.findChild<PreferencesWindow*>();
    QVERIFY(prefWin != nullptr);
    QVERIFY(prefWin->isVisible());
    QVERIFY(!prefWin->isModal());

    prefWin->accept();
    QVERIFY(!prefWin->isVisible());
}

void TestRegMapWindow::testAboutWindowAction()
{
    QString file = "examples/rmt/peripherals/spi.rmt";
    RegMapWindow window(file);
    window.show();

    auto *actAbout = window.findChild<QAction*>("actionAbout");
    QVERIFY(actAbout != nullptr);
    actAbout->trigger();

    auto *aboutWin = window.findChild<AboutWindow*>();
    QVERIFY(aboutWin != nullptr);
    QVERIFY(aboutWin->isVisible());
    QVERIFY(!aboutWin->isModal());
    QCOMPARE(window.aboutWindow(), aboutWin);

    aboutWin->accept();
    QVERIFY(!aboutWin->isVisible());
}

void TestRegMapWindow::testMenuStructure()
{
    QString file = "examples/rmt/peripherals/spi.rmt";
    RegMapWindow window(file);
    window.show();

    auto *menuEdit = window.findChild<QMenu*>("menuEdit");
    auto *menuView = window.findChild<QMenu*>("menuView");
    auto *menuHelp = window.findChild<QMenu*>("menuHelp");
    auto *actColorBlind = window.findChild<QAction*>("actionColorBlindMode");
    auto *actConfig = window.findChild<QAction*>("actionConfig");
    auto *actPref = window.findChild<QAction*>("actionPreferences");
    auto *actAbout = window.findChild<QAction*>("actionAbout");

    QVERIFY(menuEdit != nullptr);
    QVERIFY(menuView != nullptr);
    QVERIFY(menuHelp != nullptr);
    QVERIFY(actColorBlind != nullptr);
    QVERIFY(actConfig != nullptr);
    QVERIFY(actPref != nullptr);
    QVERIFY(actAbout != nullptr);

    // Verify colour blind mode only appears in View menu, not Edit menu
    QVERIFY(!menuEdit->actions().contains(actColorBlind));
    QVERIFY(menuView->actions().contains(actColorBlind));

    // Verify configuration and preferences appear in Edit menu
    QVERIFY(menuEdit->actions().contains(actConfig));
    QVERIFY(menuEdit->actions().contains(actPref));

    // Verify about action appears in Help menu
    QVERIFY(menuHelp->actions().contains(actAbout));
}

void TestRegMapWindow::testColorSchemeSwitching()
{
    QString file = "examples/rmt/peripherals/spi.rmt";
    RegMapWindow window(file);
    window.show();

    // Default theme is Solarized 8
    QCOMPARE(window.colourScheme(), QString("solarized8"));
    QCOMPARE(ThemeManager::instance().currentThemeId(), QString("solarized8"));

    // Switch to Dracula
    window.setColorScheme("dracula");
    QCOMPARE(window.colourScheme(), QString("dracula"));
    QCOMPARE(ThemeManager::instance().currentThemeId(), QString("dracula"));

    // Switch to Solarized 8 Light
    window.setColorScheme("solarized8_light");
    QCOMPARE(window.colourScheme(), QString("solarized8_light"));

    // Switch to Nord
    window.setColorScheme("nord");
    QCOMPARE(window.colourScheme(), QString("nord"));

    // Switch back to Solarized 8
    window.setColorScheme("solarized8");
    QCOMPARE(window.colourScheme(), QString("solarized8"));

    // Check menu actions exist
    auto *menuScheme = window.findChild<QMenu*>("menuColourScheme");
    if (!menuScheme) {
        menuScheme = window.findChild<QMenu*>("menuColorScheme");
    }
    QVERIFY(menuScheme != nullptr);
    QVERIFY(menuScheme->actions().size() >= 6);
}

void TestRegMapWindow::testMainWindowSizePersistence()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString origPath = AppSettings::instance().configFilePath();
    AppSettings::instance().setConfigFilePath(tempDir.path() + "/test_rmap.conf");

    QString file = "examples/rmt/peripherals/spi.rmt";
    {
        RegMapWindow window(file);
        window.resize(720, 520);
        window.saveWindowStateToSettings();
        QCOMPARE(AppSettings::instance().mainWindowSize(), QSize(720, 520));
    }

    {
        RegMapWindow window2(file);
        QCOMPARE(AppSettings::instance().mainWindowSize(), QSize(720, 520));
        QCOMPARE(window2.size(), QSize(720, 520));
    }

    AppSettings::instance().setConfigFilePath(origPath);
}

QTEST_MAIN(TestRegMapWindow)
#include "test_RegMapWindow.moc"
