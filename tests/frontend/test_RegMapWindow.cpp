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
#include <QMessageBox>
#include <QMenu>
#include "RegMapWindow.hpp"
#include "BlockMemoryMapWidget.hpp"
#include "LanguageManager.hpp"
#include "AppSettings.hpp"
#include "format/FormatManager.hpp"
#include "RegConfigWindow.hpp"
#include "SerializationContext.hpp"
#include "proto/rmap.pb.h"
#include <QFileDialog>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

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
        qputenv("RMAP_CONFIG_FILE", "work/rmap_test.conf");
        AppSettings::instance().setConfigFilePath("work/rmap_test.conf");
        AppSettings::instance().setColorBlindMode(false);
        AppSettings::instance().setColorScheme("solarized8");
    }
    void cleanupTestCase() {
        AppSettings::instance().setColorBlindMode(false);
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
    void testExportLaunchesPythonScriptWhenEnabled();
    void testExportSkipsPythonScriptWhenDisabled();
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
    void testLanguageMenuStructure();
    void testLanguageSwitching();
    void testFileSaveAndSaveAsVariations();
    void testFileOpenAndReloadVariations();
    void testWindowLifecycleAndEvents();
    void testValidationAndExportDialogs();
    void testContextMenuAndDuplicationVariations();
    void testSortingAndProxyEdgeCases();
    void testHeadlessCliExtended();
    void testUncoveredEdgeCases();
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

void TestRegMapWindow::testExportLaunchesPythonScriptWhenEnabled()
{
    QString file = "examples/rmt/peripherals/spi.rmt";
    RegMapWindow window(file);

    QDir().mkpath("work/test_fe_py");
    QString scriptPath = "work/test_fe_py/post_script.py";
    QString markerPath = "work/test_fe_py/script_ran.marker";
    QFile::remove(markerPath);

    QFile pyFile(scriptPath);
    QVERIFY(pyFile.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream out(&pyFile);
    out << "import os\n";
    out << "with open('work/test_fe_py/script_ran.marker', 'w') as f:\n";
    out << "    f.write(f'LAUNCHED:{name}:{reg_width}')\n";
    pyFile.close();

    window.configWindow()->setBaseDir(QDir::currentPath());
    protormap::Config cfg;
    cfg.set_templatefolder("./templates");
    cfg.set_outputfolder("./work/test_fe_py");
    cfg.set_pythonscript(scriptPath.toStdString());
    cfg.set_python_script_enabled(true);

    auto *entry1 = cfg.add_template_outputs();
    entry1->set_template_filename("templates/c/reg_map.h.inja");
    entry1->set_output_filepath("work/test_fe_py/reg_map.h");
    entry1->set_enabled(true);

    window.configWindow()->deserialize(cfg);

    QTimer *dismissTimer = new QTimer(&window);
    QObject::connect(dismissTimer, &QTimer::timeout, []() {
        QWidget* modal = QApplication::activeModalWidget();
        if (modal) {
            modal->close();
        }
    });
    dismissTimer->start(50);

    auto *actExport = window.findChild<QAction*>("actionExport");
    QVERIFY(actExport != nullptr);
    actExport->trigger();

    QVERIFY(QFile::exists("work/test_fe_py/reg_map.h"));
    QVERIFY(QFile::exists(markerPath));

    QFile marker(markerPath);
    QVERIFY(marker.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = QString::fromUtf8(marker.readAll());
    marker.close();
    QCOMPARE(content, QString("LAUNCHED:spi:32"));

    dismissTimer->stop();

    // Also test headless export with python script enabled
    QFile::remove(markerPath);
    bool headlessSuccess = window.headlessExport("work/test_fe_py");
    QVERIFY(headlessSuccess);
    QVERIFY(QFile::exists(markerPath));
}

void TestRegMapWindow::testExportSkipsPythonScriptWhenDisabled()
{
    QString file = "examples/rmt/peripherals/spi.rmt";
    RegMapWindow window(file);

    QDir().mkpath("work/test_fe_py_dis");
    QString scriptPath = "work/test_fe_py_dis/disabled_script.py";
    QString markerPath = "work/test_fe_py_dis/should_not_exist.marker";
    QFile::remove(markerPath);

    QFile pyFile(scriptPath);
    QVERIFY(pyFile.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream out(&pyFile);
    out << "import os\n";
    out << "with open('work/test_fe_py_dis/should_not_exist.marker', 'w') as f:\n";
    out << "    f.write('ERROR')\n";
    pyFile.close();

    window.configWindow()->setBaseDir(QDir::currentPath());
    protormap::Config cfg;
    cfg.set_templatefolder("./templates");
    cfg.set_outputfolder("./work/test_fe_py_dis");
    cfg.set_pythonscript(scriptPath.toStdString());
    cfg.set_python_script_enabled(false); // Explicitly DISABLED in configuration

    auto *entry1 = cfg.add_template_outputs();
    entry1->set_template_filename("templates/c/reg_map.h.inja");
    entry1->set_output_filepath("work/test_fe_py_dis/reg_map.h");
    entry1->set_enabled(true);

    window.configWindow()->deserialize(cfg);
    QCOMPARE(window.configWindow()->isPythonScriptEnabled(), false);
    QCOMPARE(window.configWindow()->pythonScript(), QString(""));

    QTimer *dismissTimer = new QTimer(&window);
    QObject::connect(dismissTimer, &QTimer::timeout, []() {
        QWidget* modal = QApplication::activeModalWidget();
        if (modal) {
            modal->close();
        }
    });
    dismissTimer->start(50);

    auto *actExport = window.findChild<QAction*>("actionExport");
    QVERIFY(actExport != nullptr);
    actExport->trigger();

    QVERIFY(QFile::exists("work/test_fe_py_dis/reg_map.h"));
    // Script should NOT have executed
    QVERIFY(!QFile::exists(markerPath));

    dismissTimer->stop();

    // Also headless export must not execute disabled script
    bool headlessSuccess = window.headlessExport("work/test_fe_py_dis");
    QVERIFY(headlessSuccess);
    QVERIFY(!QFile::exists(markerPath));
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

    // InsertItemCommand with default/root kind for complete switch branch coverage
    InsertItemCommand dummyCmd(model, RegMapTreeItem::e_rmmKind::root, 0, QModelIndex());
    QCOMPARE(dummyCmd.text(), QString("Add Item"));

    // Delete item command push, undo, and redo
    auto *actDel = window.findChild<QAction*>("actionDeleteItem");
    QVERIFY(actDel != nullptr);
    auto *treeView = window.findChild<QTreeView*>("treeView");
    QVERIFY(treeView != nullptr);
    auto *treeProxy = qobject_cast<QAbstractProxyModel*>(treeView->model());
    QVERIFY(treeProxy != nullptr);
    QModelIndex blkProxy = treeProxy->mapFromSource(model->index(0, 0, QModelIndex()));
    QModelIndex regProxy = treeProxy->mapFromSource(model->index(0, 0, model->index(0, 0, QModelIndex())));
    treeView->setCurrentIndex(regProxy);
    int prevRegCount = model->rowCount(model->index(0, 0, QModelIndex()));
    actDel->trigger();
    QCOMPARE(model->rowCount(model->index(0, 0, QModelIndex())), prevRegCount - 1);
    undoStack->undo();
    QCOMPARE(model->rowCount(model->index(0, 0, QModelIndex())), prevRegCount);
    undoStack->redo();
    QCOMPARE(model->rowCount(model->index(0, 0, QModelIndex())), prevRegCount - 1);

    // Duplicate item command undo and redo
    treeView->setCurrentIndex(blkProxy);
    int blkCount = model->rowCount(QModelIndex());
    auto *actDup = window.findChild<QAction*>("actionDuplicate");
    QVERIFY(actDup != nullptr);
    actDup->trigger();
    QCOMPARE(model->rowCount(QModelIndex()), blkCount + 1);
    undoStack->undo();
    QCOMPARE(model->rowCount(QModelIndex()), blkCount);
    undoStack->redo();
    QCOMPARE(model->rowCount(QModelIndex()), blkCount + 1);

    // Edit cell command via UI panel
    auto *blkNameEdit = window.findChild<QLineEdit*>("blkNameEdit");
    if (blkNameEdit) {
        blkNameEdit->setText("UI_MOD_NAME");
        emit blkNameEdit->editingFinished();
        undoStack->undo();
        undoStack->redo();
    }
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

    // Exercise menuColorBlindProfile aboutToShow and profile action
    auto *cbMenu = window.findChild<QMenu*>("menuColorBlindProfile");
    if (cbMenu) {
        emit cbMenu->aboutToShow();
        for (auto *act : cbMenu->actions()) {
            if (act->data().toString() == "deuteranopia") {
                act->trigger();
                break;
            }
        }
        emit cbMenu->aboutToShow();
    }
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
    emit menuScheme->aboutToShow();
    for (auto *act : menuScheme->actions()) {
        if (act->data().toString() == "dracula") {
            act->trigger();
            break;
        }
    }
    emit ThemeManager::instance().themesUpdated();
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

void TestRegMapWindow::testLanguageMenuStructure()
{
    QString file = "examples/rmt/peripherals/spi.rmt";
    RegMapWindow window(file);
    window.show();

    // Verify Language menu is inside View menu only
    auto *menuView = window.findChild<QMenu*>("menuView");
    QVERIFY(menuView != nullptr);

    auto *menuLang = window.findChild<QMenu*>("menuLanguage");
    QVERIFY(menuLang != nullptr);
    QVERIFY(menuView->actions().contains(menuLang->menuAction()));

    // Verify exact build-time configured languages (7 languages)
    QCOMPARE(menuLang->actions().size(), 7);

    // Verify runtime addition/removal actions do NOT exist
    auto *actAdd = window.findChild<QAction*>("actionAddTranslationFile");
    auto *actReload = window.findChild<QAction*>("actionReloadTranslations");
    QVERIFY(actAdd == nullptr);
    QVERIFY(actReload == nullptr);
}

void TestRegMapWindow::testLanguageSwitching()
{
    QString file = "examples/rmt/peripherals/spi.rmt";
    RegMapWindow window(file);
    window.show();

    // Default is English
    // Verify menu checkmarks
    auto *menuLang = window.findChild<QMenu*>("menuLanguage");
    QVERIFY(menuLang != nullptr);

    auto verifyOnlyThisLanguageChecked = [menuLang](const QString &code) -> bool {
        int checkedCount = 0;
        QString checkedCode;
        for (auto *act : menuLang->actions()) {
            if (act->isChecked()) {
                checkedCount++;
                checkedCode = act->data().toString();
            }
        }
        return (checkedCount == 1 && checkedCode.compare(code, Qt::CaseInsensitive) == 0);
    };

    QVERIFY(verifyOnlyThisLanguageChecked("en"));

    // Switch to Spanish via action trigger
    QAction *actEs = nullptr;
    for (auto *act : menuLang->actions()) {
        if (act->data().toString() == "es") {
            actEs = act;
            break;
        }
    }
    QVERIFY(actEs != nullptr);
    actEs->trigger();

    QCOMPARE(window.language(), QString("es"));
    QCOMPARE(LanguageManager::instance().currentLanguage(), QString("es"));
    QVERIFY(verifyOnlyThisLanguageChecked("es"));

    // Verify model header translations
    auto *model = window.getModel();
    QVERIFY(model != nullptr);
    QCOMPARE(model->headerData(1, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Desplazamiento"));
    QCOMPARE(model->headerData(2, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Tamaño"));

    // Switch to German via setLanguage
    window.setLanguage("de");
    QCOMPARE(window.language(), QString("de"));
    QVERIFY(verifyOnlyThisLanguageChecked("de"));
    QCOMPARE(model->headerData(1, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Offset"));
    QCOMPARE(model->headerData(2, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Größe"));

    // Switch back to English
    window.setLanguage("en");
    QCOMPARE(window.language(), QString("en"));
    QVERIFY(verifyOnlyThisLanguageChecked("en"));
    QCOMPARE(model->headerData(1, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Offset"));
    QCOMPARE(model->headerData(2, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Size"));

    // Switch via LanguageManager directly (as Preferences dialog does)
    LanguageManager::instance().setLanguage("fr");
    QCOMPARE(window.language(), QString("fr"));
    QVERIFY(verifyOnlyThisLanguageChecked("fr"));
    emit menuLang->aboutToShow();

    // Reset back to English
    window.setLanguage("en");
    QCOMPARE(window.language(), QString("en"));
    QVERIFY(verifyOnlyThisLanguageChecked("en"));
}

void TestRegMapWindow::testFileSaveAndSaveAsVariations()
{
    // 1. Save with fresh empty window -> triggers SaveAs
    {
        RegMapWindow window;
        window.show();
        auto *actSave = window.findChild<QAction*>("actionFileSave");
        QVERIFY(actSave != nullptr);

        QTimer::singleShot(50, []() {
            if (auto *modal = QApplication::activeModalWidget()) modal->close();
        });
        actSave->trigger();
    }

    // 2. Save with loaded window -> saves directly
    {
        QFile::remove("work/test_save_act.rmt");
        QVERIFY(QFile::copy("examples/rmt/peripherals/spi.rmt", "work/test_save_act.rmt"));
        RegMapWindow window("work/test_save_act.rmt");
        window.show();
        auto *actSave = window.findChild<QAction*>("actionFileSave");
        QVERIFY(actSave != nullptr);
        actSave->trigger();
    }

    // 3. fileSave(fname) with custom path
    {
        RegMapWindow window("examples/rmt/peripherals/spi.rmt");
        QVERIFY(window.fileSave("work/test_save_direct.rmt"));
        QVERIFY(QFile::exists("work/test_save_direct.rmt"));

        // fileSave with write error (visible window covers QMessageBox, hidden covers std::cerr)
        window.show();
        QTimer::singleShot(50, []() {
            if (auto *modal = QApplication::activeModalWidget()) modal->close();
        });
        QVERIFY(!window.fileSave("/proc/invalid_path/cannot_save.rmt"));
        window.hide();
        QVERIFY(!window.fileSave("/proc/invalid_path/cannot_save_hidden.rmt"));
    }

    // 4. SaveAs action with auto-dismiss
    {
        RegMapWindow window("examples/rmt/peripherals/spi.rmt");
        auto *actSaveAs = window.findChild<QAction*>("actionFileSaveAs");
        QVERIFY(actSaveAs != nullptr);
        QTimer::singleShot(50, []() {
            if (auto *modal = QApplication::activeModalWidget()) modal->close();
        });
        actSaveAs->trigger();
    }

    // 5. regmap_modified and notModified via cell edit
    {
        RegMapWindow window("examples/rmt/peripherals/spi.rmt");
        auto *model = window.getModel();
        QVERIFY(model != nullptr);
        QModelIndex blkIdx = model->index(0, 0, QModelIndex());
        QModelIndex regIdx = model->index(0, 0, blkIdx);
        QModelIndex nameIdx = model->index(0, 3, regIdx);
        model->setData(nameIdx, "MODIFIED_REG", Qt::EditRole);
        QVERIFY(window.windowTitle().endsWith('*'));

        window.fileSave("work/test_save_clearmod.rmt");
        QVERIFY(!window.windowTitle().endsWith('*'));
    }
}

void TestRegMapWindow::testFileOpenAndReloadVariations()
{
    // 1. File reload on modified model with Cancel vs Ok
    {
        RegMapWindow window("examples/rmt/peripherals/spi.rmt");
        auto *model = window.getModel();
        QModelIndex blkIdx = model->index(0, 0, QModelIndex());
        QModelIndex regIdx = model->index(0, 0, blkIdx);
        model->setData(model->index(0, 3, regIdx), "NEW_NAME", Qt::EditRole);

        auto *actReload = window.findChild<QAction*>("actionFileReload");
        QVERIFY(actReload != nullptr);

        // Cancel reload
        QTimer::singleShot(50, []() {
            if (auto *box = qobject_cast<QMessageBox*>(QApplication::activeModalWidget())) {
                if (auto *btn = box->button(QMessageBox::Cancel)) btn->click();
                else box->close();
            } else if (auto *m = QApplication::activeModalWidget()) m->close();
        });
        actReload->trigger();

        // Ok reload
        QTimer::singleShot(50, []() {
            if (auto *box = qobject_cast<QMessageBox*>(QApplication::activeModalWidget())) {
                if (auto *btn = box->button(QMessageBox::Ok)) btn->click();
                else box->close();
            } else if (auto *m = QApplication::activeModalWidget()) m->close();
        });
        actReload->trigger();
    }

    // 2. File Open on modified model with Cancel vs Ok
    {
        RegMapWindow window("examples/rmt/peripherals/spi.rmt");
        auto *model = window.getModel();
        QModelIndex blkIdx = model->index(0, 0, QModelIndex());
        model->setData(model->index(0, 3, blkIdx), "MOD_BLK", Qt::EditRole);

        auto *actOpen = window.findChild<QAction*>("actionFileOpen");
        QVERIFY(actOpen != nullptr);

        // Cancel open
        QTimer::singleShot(50, []() {
            if (auto *box = qobject_cast<QMessageBox*>(QApplication::activeModalWidget())) {
                if (auto *btn = box->button(QMessageBox::Cancel)) btn->click();
                else box->close();
            } else if (auto *m = QApplication::activeModalWidget()) m->close();
        });
        actOpen->trigger();

        // Ok open with auto-closing open dialog
        QTimer::singleShot(50, []() {
            if (auto *box = qobject_cast<QMessageBox*>(QApplication::activeModalWidget())) {
                if (auto *btn = box->button(QMessageBox::Ok)) btn->click();
            }
            QTimer::singleShot(50, []() {
                if (auto *m = QApplication::activeModalWidget()) m->close();
            });
        });
        actOpen->trigger();
    }

    // 3. fileOpen with nonexistent file
    {
        RegMapWindow window;
        QTimer::singleShot(50, []() {
            if (auto *modal = QApplication::activeModalWidget()) modal->close();
        });
        window.fileOpen("nonexistent_file_path.rmt");
    }

    // 4. fileOpen with corrupt file
    {
        RegMapWindow window;
        QTimer::singleShot(50, []() {
            if (auto *modal = QApplication::activeModalWidget()) modal->close();
        });
        window.fileOpen("CMakeLists.txt");
    }

    // 5. File New and Close with Save option when modified
    {
        QFile::remove("work/temp_save_new.rmt");
        QVERIFY(QFile::copy("examples/rmt/peripherals/spi.rmt", "work/temp_save_new.rmt"));
        RegMapWindow window("work/temp_save_new.rmt");
        auto *model = window.getModel();
        model->setData(model->index(0, 3, model->index(0, 0, QModelIndex())), "MOD_NAME", Qt::EditRole);

        auto *actNew = window.findChild<QAction*>("actionFileNew");
        QVERIFY(actNew != nullptr);

        QTimer::singleShot(50, []() {
            if (auto *box = qobject_cast<QMessageBox*>(QApplication::activeModalWidget())) {
                if (auto *btn = box->button(QMessageBox::Save)) btn->click();
                else box->close();
            } else if (auto *m = QApplication::activeModalWidget()) m->close();
        });
        actNew->trigger();

        // After fileNew, window has a fresh model
        model = window.getModel();
        QVERIFY(model != nullptr);
        auto *actAddBlk = window.findChild<QAction*>("actionAddRegBlock");
        if (actAddBlk) actAddBlk->trigger();

        auto *actClose = window.findChild<QAction*>("actionFileClose");
        QVERIFY(actClose != nullptr);

        QTimer::singleShot(50, []() {
            if (auto *box = qobject_cast<QMessageBox*>(QApplication::activeModalWidget())) {
                if (auto *btn = box->button(QMessageBox::Ok)) btn->click();
                else box->close();
            } else if (auto *m = QApplication::activeModalWidget()) m->close();
        });
        actClose->trigger();
    }
}

void TestRegMapWindow::testWindowLifecycleAndEvents()
{
    RegMapWindow window("examples/rmt/peripherals/spi.rmt");
    window.show();
    QCoreApplication::processEvents();

    // Exercise deleting destructor
    delete new RegMapWindow();

    // Close event
    QCloseEvent closeEv;
    QApplication::sendEvent(&window, &closeEv);

    // Quit action
    auto *actQuit = window.findChild<QAction*>("actionQuit");
    if (actQuit) {
        actQuit->trigger();
    }

    // Language change event
    QEvent langEv(QEvent::LanguageChange);
    QApplication::sendEvent(&window, &langEv);

    // Color blind mode extended
    window.setColourBlindType(ColorBlindMode::Protanopia);
    QCOMPARE(window.colourBlindType(), ColorBlindMode::Protanopia);
    QCOMPARE(window.colorBlindType(), ColorBlindMode::Protanopia);
    QVERIFY(window.isColourBlindMode());

    window.setColourBlindType(ColorBlindMode::Tritanopia);
    QCOMPARE(window.colourBlindType(), ColorBlindMode::Tritanopia);

    window.setColourBlindType(ColorBlindMode::None);
    QCOMPARE(window.colourBlindType(), ColorBlindMode::None);
    QVERIFY(!window.isColourBlindMode());
}

void TestRegMapWindow::testValidationAndExportDialogs()
{
    // 1. Check on valid model (spi.rmt)
    {
        RegMapWindow window("examples/rmt/peripherals/spi.rmt");
        auto *actCheck = window.findChild<QAction*>("actionCheck");
        QVERIFY(actCheck != nullptr);

        QTimer::singleShot(50, []() {
            if (auto *modal = QApplication::activeModalWidget()) modal->close();
        });
        actCheck->trigger();
    }

    // 2. Check on invalid model (invalid_overlap.rmt)
    {
        RegMapWindow window("examples/rmt/validation/invalid_overlap.rmt");
        auto *actCheck = window.findChild<QAction*>("actionCheck");
        QVERIFY(actCheck != nullptr);

        QTimer::singleShot(50, []() {
            if (auto *modal = QApplication::activeModalWidget()) modal->close();
        });
        actCheck->trigger();
    }

    // 3. Export on invalid model - reply No
    {
        RegMapWindow window("examples/rmt/validation/invalid_overlap.rmt");
        auto *actExport = window.findChild<QAction*>("actionExport");
        QVERIFY(actExport != nullptr);

        QTimer::singleShot(50, []() {
            if (auto *box = qobject_cast<QMessageBox*>(QApplication::activeModalWidget())) {
                if (auto *btn = box->button(QMessageBox::No)) btn->click();
                else box->close();
            } else if (auto *m = QApplication::activeModalWidget()) m->close();
        });
        actExport->trigger();
    }

    // 4. Export on invalid model - reply Yes
    {
        RegMapWindow window("examples/rmt/validation/invalid_overlap.rmt");
        auto *actExport = window.findChild<QAction*>("actionExport");
        QVERIFY(actExport != nullptr);

        QTimer::singleShot(50, []() {
            if (auto *box = qobject_cast<QMessageBox*>(QApplication::activeModalWidget())) {
                if (auto *btn = box->button(QMessageBox::Yes)) btn->click();
                else box->close();
            }
            QTimer::singleShot(100, []() {
                if (auto *m = QApplication::activeModalWidget()) m->close();
            });
        });
        actExport->trigger();
    }

    // 5. Export with empty template table
    {
        RegMapWindow window("examples/rmt/peripherals/spi.rmt");
        auto *cfgWin = window.configWindow();
        QVERIFY(cfgWin != nullptr);
        auto *tbl = cfgWin->findChild<QTableWidget*>("templateTable");
        if (tbl) tbl->setRowCount(0);

        auto *actExport = window.findChild<QAction*>("actionExport");
        QVERIFY(actExport != nullptr);

        QTimer::singleShot(50, []() {
            if (auto *modal = QApplication::activeModalWidget()) modal->close();
        });
        actExport->trigger();
    }
}

void TestRegMapWindow::testContextMenuAndDuplicationVariations()
{
    RegMapWindow window("examples/rmt/peripherals/spi.rmt");
    window.show();
    QCoreApplication::processEvents();

    auto *treeView = window.findChild<QTreeView*>("treeView");
    QVERIFY(treeView != nullptr);
    auto *model = window.getModel();
    QVERIFY(model != nullptr);

    // 1. Duplicate a field item (fld) via fieldsTableView focus
    auto *fieldsTable = window.findChild<QTableView*>("fieldsTableView");
    QVERIFY(fieldsTable != nullptr);
    if (fieldsTable->model() && fieldsTable->model()->rowCount() > 0) {
        treeView->setCurrentIndex(QModelIndex());
        fieldsTable->setFocus();
        fieldsTable->setCurrentIndex(fieldsTable->model()->index(0, 0));
        auto *actDup = window.findChild<QAction*>("actionDuplicate");
        QVERIFY(actDup != nullptr);
        int prevCount = fieldsTable->model()->rowCount();
        actDup->trigger();
        QCOMPARE(fieldsTable->model()->rowCount(), prevCount + 1);
    }

    auto dismissContextMenu = [&window](bool triggerDup) {
        auto *menu = window.findChild<QMenu*>("treeContextMenu");
        if (menu) {
            if (triggerDup) {
                for (auto *act : menu->actions()) {
                    if (act->text().contains("Duplicate")) {
                        act->trigger();
                        break;
                    }
                }
            }
            menu->hide();
        }
    };

    // 3. Tree context menu at valid position - trigger duplicate
    QTimer::singleShot(50, [&dismissContextMenu]() {
        dismissContextMenu(true);
    });
    QModelIndex proxyFirst = treeView->model()->index(0, 0);
    QRect r = treeView->visualRect(proxyFirst);
    emit treeView->customContextMenuRequested(r.center());

    // 4. Tree context menu at invalid position
    QTimer::singleShot(50, [&dismissContextMenu]() {
        dismissContextMenu(false);
    });
    emit treeView->customContextMenuRequested(QPoint(-15, -15));

    // 5. Fields table context menu at valid and invalid positions
    if (fieldsTable && fieldsTable->model() && fieldsTable->model()->rowCount() > 0) {
        QTimer::singleShot(50, [&dismissContextMenu]() {
            dismissContextMenu(true);
        });
        QRect fr = fieldsTable->visualRect(fieldsTable->model()->index(0, 0));
        emit fieldsTable->customContextMenuRequested(fr.center());

        QTimer::singleShot(50, [&dismissContextMenu]() {
            dismissContextMenu(false);
        });
        emit fieldsTable->customContextMenuRequested(QPoint(-15, -15));
    }

    // 6. Add Address Map action
    auto *actAddMap = window.findChild<QAction*>("actionAddRegMap");
    if (actAddMap) {
        actAddMap->trigger();
    }

    // 7. Test Undo / Redo with EditCellCommand and InsertItemCommand via GUI actions
    auto *stack = window.undoStack();
    QVERIFY(stack != nullptr);

    QModelIndex blkProxy = treeView->model()->index(0, 0);
    treeView->expand(blkProxy);
    QModelIndex regProxy = treeView->model()->index(0, 0, blkProxy);
    QVERIFY(regProxy.isValid());

    // Switch selection to register to populate m_currentRegItem
    treeView->setCurrentIndex(regProxy);
    QCoreApplication::processEvents();

    auto *regNameEdit = window.findChild<QLineEdit*>("regNameEdit");
    QVERIFY(regNameEdit != nullptr);
    regNameEdit->setText("EDITED_NAME");
    emit regNameEdit->editingFinished();

    stack->undo();
    stack->redo();
    stack->undo();

    auto *regOffsetEdit = window.findChild<QLineEdit*>("regOffsetEdit");
    if (regOffsetEdit) {
        regOffsetEdit->setText("0x80");
        emit regOffsetEdit->editingFinished();
        stack->undo();
        stack->redo();
        stack->undo();
    }

    auto *regDescEdit = window.findChild<QLineEdit*>("regDescEdit");
    if (regDescEdit) {
        regDescEdit->setText("New Description");
        emit regDescEdit->editingFinished();
        stack->undo();
        stack->redo();
        stack->undo();
    }

    // Insert new register child into block via insertChild
    treeView->setCurrentIndex(blkProxy);
    QCoreApplication::processEvents();

    auto *blkNameEdit = window.findChild<QLineEdit*>("blkNameEdit");
    if (blkNameEdit) {
        blkNameEdit->setText("EDITED_BLK");
        emit blkNameEdit->editingFinished();
        stack->undo();
        stack->redo();
        stack->undo();
    }

    int countBefore = treeView->model()->rowCount(blkProxy);
    window.insertChild(RegMapTreeItem::e_rmmKind::reg);
    QCOMPARE(treeView->model()->rowCount(blkProxy), countBefore + 1);
    stack->undo();
    QCOMPARE(treeView->model()->rowCount(blkProxy), countBefore);
    stack->redo();
    QCOMPARE(treeView->model()->rowCount(blkProxy), countBefore + 1);
    stack->undo();
}

void TestRegMapWindow::testSortingAndProxyEdgeCases()
{
    RegMapWindow window("examples/rmt/peripherals/spi.rmt");
    auto *model = window.getModel();
    QVERIFY(model != nullptr);

    QModelIndex blkIdx = model->index(0, 0, QModelIndex());
    QVERIFY(blkIdx.isValid());

    // Insert data with 0b, decimal, and 0x formats
    QModelIndex reg1 = model->index(0, 0, blkIdx);
    model->setData(model->index(0, 1, reg1), "0b100", Qt::EditRole);
    QModelIndex reg2 = model->index(1, 0, blkIdx);
    model->setData(model->index(1, 1, reg2), "10", Qt::EditRole);
    QModelIndex reg3 = model->index(2, 0, blkIdx);
    model->setData(model->index(2, 1, reg3), "0x20", Qt::EditRole);

    auto *treeView = window.findChild<QTreeView*>("treeView");
    QVERIFY(treeView != nullptr);
    treeView->sortByColumn(1, Qt::AscendingOrder);
    treeView->sortByColumn(1, Qt::DescendingOrder);
    treeView->sortByColumn(2, Qt::AscendingOrder);
    treeView->sortByColumn(3, Qt::AscendingOrder);

    auto *fieldsTable = window.findChild<QTableView*>("fieldsTableView");
    if (fieldsTable) {
        fieldsTable->sortByColumn(1, Qt::AscendingOrder);
        fieldsTable->sortByColumn(2, Qt::AscendingOrder);
        fieldsTable->sortByColumn(3, Qt::AscendingOrder);
    }
}

void TestRegMapWindow::testHeadlessCliExtended()
{
    // 1. headlessExport with empty filename
    {
        RegMapWindow emptyWin;
        QVERIFY(!emptyWin.headlessExport("work"));
    }

    // 2. headlessExport with output directory override
    {
        RegMapWindow spiWin("examples/rmt/peripherals/spi.rmt");
        QVERIFY(spiWin.headlessExport("work/spi_custom_out"));
        QVERIFY(QDir("work/spi_custom_out").exists());
    }

    // 3. headlessLint with strict mode on wide bus (checks alignment and warns on missing descriptions)
    {
        RegMapWindow wideWin("examples/rmt/features/wide_bus_64bit.rmt");
        QVERIFY(!wideWin.headlessLint(true, "text", ""));
        QVERIFY(wideWin.headlessLint(false, "text", ""));
    }

    // 4. headlessLint with invalid overlap and JUnit format
    {
        RegMapWindow invWin("examples/rmt/validation/invalid_overlap.rmt");
        QVERIFY(!invWin.headlessLint(false, "junit", "work/lint_fail.xml"));
        QVERIFY(QFile::exists("work/lint_fail.xml"));
    }

    // 5. headlessLint with invalid overlap and text format
    {
        RegMapWindow invWin("examples/rmt/validation/invalid_overlap.rmt");
        QVERIFY(!invWin.headlessLint(false, "text", "work/lint_fail.txt"));
        QVERIFY(QFile::exists("work/lint_fail.txt"));
    }

    // 6. headlessLint with unwritable destination
    {
        RegMapWindow spiWin("examples/rmt/peripherals/spi.rmt");
        QVERIFY(!spiWin.headlessLint(false, "text", "/proc/cannot_write/report.txt"));
    }

    // 7. semanticDiff with missing file1
    QVERIFY(!RegMapWindow::semanticDiff("nonexistent_1.rmt", "examples/rmt/peripherals/spi.rmt", "text", ""));

    // 8. semanticDiff with missing file2
    QVERIFY(!RegMapWindow::semanticDiff("examples/rmt/peripherals/spi.rmt", "nonexistent_2.rmt", "text", ""));

    // 9. semanticDiff with markdown format (same files)
    QVERIFY(RegMapWindow::semanticDiff("examples/rmt/peripherals/spi.rmt", "examples/rmt/peripherals/spi.rmt", "markdown", "work/diff_same.md"));
    QVERIFY(QFile::exists("work/diff_same.md"));

    // 10. semanticDiff with markdown format (different files)
    QVERIFY(RegMapWindow::semanticDiff("examples/rmt/peripherals/spi.rmt", "examples/rmt/features/wide_bus_64bit.rmt", "markdown", "work/diff_diff.md"));
    QVERIFY(QFile::exists("work/diff_diff.md"));
}

// Forward-declare Protobuf serialization operators defined in RegMapWindow.cpp
protormap::RegModel& operator <<( protormap::RegModel& reg_model, const SerializationContext& context );
protormap::RegModel& operator >>( protormap::RegModel& reg_model, SerializationContext& context );

void TestRegMapWindow::testUncoveredEdgeCases()
{
    // 1. isColorBlindMode() and colorScheme() inline getters in RegMapWindow.hpp
    {
        RegMapWindow win;
        QCOMPARE(win.isColorBlindMode(), win.isColourBlindMode());
        QCOMPARE(win.colorScheme(), win.colourScheme());
    }

    // 2. parseNumericValue binary ("0b...") and decimal ("...") in duplicateItem (lines 44-45, 47, 2115-2117)
    {
        RegMapWindow win("examples/rmt/peripherals/spi.rmt");
        auto *model = win.getModel();
        QVERIFY(model != nullptr);
        QModelIndex blkIdx = model->index(0, 0, QModelIndex());
        QModelIndex regIdx = model->index(0, 0, blkIdx);

        // Set LSB of field 0 to binary "0b100" and field 1 to decimal "16"
        model->setData(model->index(0, 1, regIdx), "0b100", Qt::EditRole); // field 0 offset
        model->setData(model->index(1, 1, regIdx), "16", Qt::EditRole);    // field 1 offset

        // Duplicate field 0 (exercises lines 44-45 in parseNumericValue and lines 2115-2117)
        win.duplicateItem(model->index(0, 0, regIdx));

        // Duplicate field 1 (exercises line 47 in parseNumericValue)
        win.duplicateItem(model->index(1, 0, regIdx));

        // Duplicate item with direct source model index (exercises line 2091)
        win.duplicateItem(regIdx);
    }

    // 3. Window title when filename is empty (line 1091)
    {
        RegMapWindow win;
        win.setLanguage("es");
        QVERIFY(!win.windowTitle().isEmpty());
        win.setLanguage("en");
    }

    // 4. Window geometry fallback branches (lines 1117 & 1120)
    {
        AppSettings::instance().setMainWindowGeometry(QByteArray());
        AppSettings::instance().setMainWindowSize(QSize(-1, -1));
        AppSettings::instance().setMainWindowPos(QPoint(50, 50));
        RegMapWindow win;
        win.restoreWindowStateFromSettings();
    }

    auto dismissModal = [](const QString &filePath = QString(), int button = -1) {
        QApplication::processEvents();
        auto *timer = new QTimer();
        auto start = std::chrono::steady_clock::now();
        QObject::connect(timer, &QTimer::timeout, [timer, start, filePath, button]() {
            QWidget *modal = QApplication::activeModalWidget();
            if (!modal) {
                for (auto *w : QApplication::topLevelWidgets()) {
                    if (w->isVisible() && (w->isModal() || qobject_cast<QMessageBox*>(w) || qobject_cast<QFileDialog*>(w))) {
                        modal = w;
                        break;
                    }
                }
            }
            if (modal) {
                timer->stop();
                timer->deleteLater();
                if (auto *fileDlg = qobject_cast<QFileDialog*>(modal)) {
                    if (!filePath.isEmpty()) {
                        auto *edit = fileDlg->findChild<QLineEdit*>("fileNameEdit");
                        if (edit) {
                            edit->setText(QFileInfo(filePath).fileName());
                        }
                        for (auto *btn : fileDlg->findChildren<QPushButton*>()) {
                            QString txt = btn->text();
                            if (txt.contains("Save") || txt.contains("Guardar") || txt.contains("Speichern") ||
                                txt.contains("Open") || txt.contains("Abrir") || txt.contains("Öffnen") ||
                                txt.contains("Choose") || (!txt.contains("Cancel") && !txt.contains("Cancelar") && btn->isDefault())) {
                                btn->click();
                                return;
                            }
                        }
                    }
                    modal->close();
                    return;
                } else if (auto *msgBox = qobject_cast<QMessageBox*>(modal)) {
                    if (button >= 0) {
                        if (auto *btn = msgBox->button(static_cast<QMessageBox::StandardButton>(button))) {
                            btn->click();
                        } else {
                            msgBox->accept();
                        }
                    } else {
                        msgBox->accept();
                    }
                    return;
                } else if (auto *dlg = qobject_cast<QDialog*>(modal)) {
                    dlg->accept();
                    return;
                } else {
                    modal->close();
                    return;
                }
            }
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
            if (elapsed > 3000) {
                timer->stop();
                timer->deleteLater();
            }
        });
        timer->start(5);
    };

    // 5. btnFileNew Save branch (line 1256)
    {
        RegMapWindow win("examples/rmt/peripherals/spi.rmt");
        win.m_rmap_filename = "work/spi_test_save.rmt";
        auto *model = win.getModel();
        model->setData(model->index(0, 3, model->index(0, 0, QModelIndex())), "MODIFIED_NAME", Qt::EditRole);
        win.m_is_regmap_modified = true;

        dismissModal(QString(), QMessageBox::Save);
        win.btnFileNew();
    }

    // 6. btnFileSaveAs and btnFileOpen modal dialog file selection (lines 1291-1296, 1315-1318, 1751)
    {
        RegMapWindow win("examples/rmt/peripherals/spi.rmt");
        win.m_rmap_filename = "work/spi_test_save.rmt";
        win.m_active_folder = QDir::current().absoluteFilePath("work");

        // btnFileSaveAs with accepted chosen file
        dismissModal("work/saved_as_test.rmt");
        win.btnFileSaveAs();
        QVERIFY(QFile::exists("work/saved_as_test.rmt"));

        // btnFileOpen with accepted chosen file (exists in work/ from previous save)
        dismissModal("work/saved_as_test.rmt");
        win.btnFileOpen();

        // fileSave("") when filename is empty delegates to btnFileSaveAs (line 1751)
        RegMapWindow emptyWin;
        dismissModal(); // reject
        emptyWin.fileSave("");
    }

    // 7. btnExport branches (lines 1411, 1418, 1429, 1448-1463)
    {
        RegMapWindow win("examples/rmt/peripherals/spi.rmt");
        auto *cfgWin = win.configWindow();
        QVERIFY(cfgWin != nullptr);

        // Line 1411 (empty templatefolder), line 1418 (project_name), line 1429 (custom_parameters)
        auto *cfg = cfgWin->serialize();
        cfg->set_templatefolder("");
        cfg->set_project_name("CustomExportProject");
        (*cfg->mutable_custom_parameters())["author"] = "Tester";
        cfgWin->deserialize(*cfg);
        delete cfg;

        // Line 1460 (no templates notice): clear template outputs
        auto *emptyCfg = cfgWin->serialize();
        emptyCfg->clear_template_outputs();
        cfgWin->deserialize(*emptyCfg);
        delete emptyCfg;

        dismissModal();
        win.btnExport();

        // Lines 1448-1458 (export completed with errors): add non-existent template
        auto *errCfg = cfgWin->serialize();
        auto *out = errCfg->add_template_outputs();
        out->set_template_filename("nonexistent/bad_template.inja");
        out->set_output_filepath("work/bad_out.txt");
        out->set_enabled(true);
        cfgWin->deserialize(*errCfg);
        delete errCfg;

        dismissModal();
        win.btnExport();
    }

    // 8. updateBitfieldBar selection edge cases (lines 1570-1571, 1578-1579)
    {
        RegMapWindow win("examples/rmt/peripherals/spi.rmt");
        auto *treeView = win.findChild<QTreeView*>("treeView");
        auto *fieldsTable = win.findChild<QTableView*>("fieldsTableView");
        auto *model = win.getModel();

        // Select a register so fields table is populated
        QModelIndex blkIdx = model->index(0, 0, QModelIndex());
        QModelIndex regIdx = model->index(0, 0, blkIdx);
        treeView->setCurrentIndex(treeView->model()->index(0, 0, treeView->model()->index(0, 0, QModelIndex())));

        if (fieldsTable && fieldsTable->selectionModel()) {
            fieldsTable->setCurrentIndex(fieldsTable->model()->index(0, 0, QModelIndex()));
            fieldsTable->selectionModel()->clearSelection();
            fieldsTable->selectionModel()->setCurrentIndex(QModelIndex(), QItemSelectionModel::NoUpdate);
        }
    }

    // 9. Protobuf operator << and >> for mem and map (lines 1689-1690, 1722)
    {
        protormap::RegModel regModel;
        SerializationContext ctxOut;

        QVariantMap memData;
        memData["kind"] = QVariant::fromValue(RegMapTreeItem::e_rmmKind::mem);
        memData["id"] = 10;
        memData["parent"] = 0;
        QVariantMap memColData;
        memColData["Name"] = "SRAM_BLOCK";
        memData["itemData"] = memColData;
        ctxOut.append_record<RegMapTreeItem>(nullptr, memData);

        QVariantMap mapData;
        mapData["kind"] = QVariant::fromValue(RegMapTreeItem::e_rmmKind::map);
        mapData["id"] = 20;
        mapData["parent"] = 0;
        QVariantMap mapColData;
        mapColData["Name"] = "MAIN_MAP";
        mapData["itemData"] = mapColData;
        ctxOut.append_record<RegMapTreeItem>(nullptr, mapData);

        regModel << ctxOut; // covers lines 1689 and 1690

        SerializationContext ctxIn;
        regModel >> ctxIn;  // covers line 1722
    }

    // 10. insertChild tree traversal and fallback branches (lines 1831-1832, 1845-1853, 1857)
    {
        RegMapWindow win("examples/rmt/peripherals/spi.rmt");
        auto *treeView = win.findChild<QTreeView*>("treeView");
        auto *proxy = qobject_cast<QAbstractProxyModel*>(treeView->model());
        auto *model = win.getModel();

        // Select a field, then insert reg (while loop climbs up from fld -> reg -> blk, lines 1831-1832)
        QModelIndex blkIdx = model->index(0, 0, QModelIndex());
        QModelIndex regIdx = model->index(0, 0, blkIdx);
        QModelIndex fldIdx = model->index(0, 0, regIdx);
        QModelIndex proxyFld = proxy ? proxy->mapFromSource(fldIdx) : QModelIndex();
        treeView->setCurrentIndex(proxyFld);
        win.insertChild(RegMapTreeItem::e_rmmKind::reg);

        // Deselect in treeView, insert reg -> falls back to firstChild (lines 1845-1853)
        treeView->selectionModel()->clearSelection();
        treeView->setCurrentIndex(QModelIndex());
        win.insertChild(RegMapTreeItem::e_rmmKind::reg);

        // Empty window with no blocks -> insertChild returns early (line 1857)
        RegMapWindow emptyWin;
        emptyWin.fileNew();
        auto *emptyModel = emptyWin.getModel();
        emptyModel->removeRows(0, emptyModel->rowCount(QModelIndex()), QModelIndex());
        emptyWin.insertChild(RegMapTreeItem::e_rmmKind::reg);
    }

    // 11. onTreeSelectionChanged reconnection of fields table model (lines 1943-1944)
    {
        RegMapWindow win("examples/rmt/peripherals/spi.rmt");
        auto *fieldsTable = win.findChild<QTableView*>("fieldsTableView");
        auto *treeView = win.findChild<QTreeView*>("treeView");
        if (fieldsTable) {
            fieldsTable->setModel(nullptr);
        }
        auto *proxy = qobject_cast<QAbstractProxyModel*>(treeView->model());
        auto *model = win.getModel();
        QModelIndex reg0 = model->index(0, 0, model->index(0, 0, QModelIndex()));
        treeView->setCurrentIndex(proxy ? proxy->mapFromSource(reg0) : QModelIndex());
    }

    // 12. duplicateCurrentItem when treeView index is invalid but m_currentRegItem is set (lines 2070-2076)
    // and duplicateItem when new_proxy is not valid in treeProxy (lines 2133-2140)
    {
        RegMapWindow win("examples/rmt/peripherals/spi.rmt");
        auto *treeView = win.findChild<QTreeView*>("treeView");
        auto *model = win.getModel();
        QModelIndex blk0 = model->index(0, 0, QModelIndex());
        QModelIndex reg0 = model->index(0, 0, blk0);
        win.m_currentRegItem = model->getItem(reg0);
        treeView->selectionModel()->clearSelection();
        treeView->setCurrentIndex(QModelIndex());
        win.duplicateSelectedRegister(); // executes lines 2070-2076

        // Set search filter that will not match duplicated field name so new_proxy is invalid (lines 2133-2140)
        auto *searchEdit = win.findChild<QLineEdit*>("searchLineEdit");
        if (searchEdit) {
            searchEdit->setText("ZZZZ_NEVER_MATCH_DUPLICATE");
        }
        QModelIndex fld0 = model->index(0, 0, reg0);
        win.duplicateItem(fld0);
        if (searchEdit) searchEdit->clear();
    }

    // 13. headlessExport output path prefix branches (lines 2169, 2176, 2187, 2203, 2206-2209, 2217, 2231-2236)
    {
        QFile::remove("work/headless_spi.rmt");
        QFile::copy("examples/rmt/peripherals/spi.rmt", "work/headless_spi.rmt");
        RegMapWindow win("work/headless_spi.rmt");
        auto *cfgWin = win.configWindow();
        auto *cfg = cfgWin->serialize();
        cfg->set_templatefolder("");
        cfg->set_project_name("HeadlessProject");
        (*cfg->mutable_custom_parameters())["version"] = "1.0";
        cfg->set_outputfolder("custom_cfg_out");

        cfg->clear_template_outputs();
        auto *out1 = cfg->add_template_outputs();
        out1->set_template_filename("templates/c/reg_map.h.inja");
        out1->set_output_filepath("./custom_cfg_out/header.h");
        out1->set_enabled(true);

        auto *out2 = cfg->add_template_outputs();
        out2->set_template_filename("templates/sim/Makefile.inja");
        out2->set_output_filepath("./work/Makefile");
        out2->set_enabled(true);

        auto *out3 = cfg->add_template_outputs();
        out3->set_template_filename("templates/markdown/reg_doc.md.inja");
        out3->set_output_filepath("raw_filename.md");
        out3->set_enabled(true);

        cfgWin->deserialize(*cfg);
        delete cfg;

        QVERIFY(win.headlessExport("work/headless_prefix_out"));
        QVERIFY(win.headlessExport(""));

        auto *errCfg = cfgWin->serialize();
        errCfg->clear_template_outputs();
        auto *errOut = errCfg->add_template_outputs();
        errOut->set_template_filename("nonexistent_path/bad.inja");
        errOut->set_output_filepath("work/bad.txt");
        errOut->set_enabled(true);
        cfgWin->deserialize(*errCfg);
        delete errCfg;
        QVERIFY(!win.headlessExport("work"));
    }

    // 14. headlessLint strict mode unaligned offset warning and JUnit format (lines 2275, 2332, 2345-2349)
    {
        RegMapWindow win("examples/rmt/peripherals/spi.rmt");
        auto *model = win.getModel();
        QModelIndex blk0 = model->index(0, 0, QModelIndex());
        model->setData(model->index(1, 1, blk0), "0x21", Qt::EditRole);

        QVERIFY(!win.headlessLint(true, "junit", "work/junit_unaligned_strict.xml"));
        QVERIFY(QFile::exists("work/junit_unaligned_strict.xml"));

        QVERIFY(win.headlessLint(false, "junit", "work/junit_unaligned_nonstrict.xml"));
        QVERIFY(QFile::exists("work/junit_unaligned_nonstrict.xml"));
    }

    // 15. semanticDiff added, modified, removed fields and markdown output (lines 2473, 2475, 2481, 2486, 2515-2517, 2536-2537)
    {
        json j1, j2;
        j1["blocks"] = json::array({
            {
                {"name", "BLK"},
                {"offset", 0},
                {"registers", json::array({
                    {
                        {"name", "REG1"},
                        {"offset", 0},
                        {"fields", json::array({
                            {{"name", "FLD_MOD"}, {"offset_lsb", 0}, {"size_width", 8}},
                            {{"name", "FLD_REM"}, {"offset_lsb", 8}, {"size_width", 8}}
                        })}
                    }
                })}
            }
        });

        j2["blocks"] = json::array({
            {
                {"name", "BLK"},
                {"offset", 0},
                {"registers", json::array({
                    {
                        {"name", "REG1"},
                        {"offset", 0},
                        {"fields", json::array({
                            {{"name", "FLD_MOD"}, {"offset_lsb", 0}, {"size_width", 16}},
                            {{"name", "FLD_ADD"}, {"offset_lsb", 16}, {"size_width", 8}}
                        })}
                    }
                })}
            }
        });

        QFile f1("work/diff_f1.json");
        QVERIFY(f1.open(QIODevice::WriteOnly | QIODevice::Text));
        f1.write(j1.dump(2).c_str());
        f1.close();

        QFile f2("work/diff_f2.json");
        QVERIFY(f2.open(QIODevice::WriteOnly | QIODevice::Text));
        f2.write(j2.dump(2).c_str());
        f2.close();

        QVERIFY(RegMapWindow::semanticDiff("work/diff_f1.json", "work/diff_f2.json", "markdown", "work/diff_fields.md"));
        QVERIFY(QFile::exists("work/diff_fields.md"));
        QVERIFY(!RegMapWindow::semanticDiff("work/diff_f1.json", "work/diff_f2.json", "markdown", "/proc/invalid/out.md"));
    }

    // 16. Empty filename window title update via LanguageChange event (line 1091)
    {
        RegMapWindow win;
        QEvent ev(QEvent::LanguageChange);
        QCoreApplication::sendEvent(&win, &ev);
        QVERIFY(!win.windowTitle().isEmpty());
    }

    // 17. btnFileClose with unsaved changes and Save reply (line 1256)
    {
        QFile::remove("work/temp_close.rmt");
        QFile::copy("examples/rmt/peripherals/spi.rmt", "work/temp_close.rmt");
        RegMapWindow win("work/temp_close.rmt");
        auto *model = win.getModel();
        QModelIndex blk0 = model->index(0, 0, QModelIndex());
        model->setData(model->index(0, 1, blk0), "0x44", Qt::EditRole);
        dismissModal(QString(), QMessageBox::Save);
        win.btnFileClose();
    }

    // 18. btnExport with partial success and errors (lines 1447-1449)
    {
        RegMapWindow win("examples/rmt/peripherals/spi.rmt");
        auto *cfgWin = win.configWindow();
        auto *cfg = cfgWin->serialize();
        cfg->clear_template_outputs();
        auto *goodOut = cfg->add_template_outputs();
        goodOut->set_template_filename("templates/c/reg_map.h.inja");
        goodOut->set_output_filepath("work/good_export.h");
        goodOut->set_enabled(true);
        auto *badOut = cfg->add_template_outputs();
        badOut->set_template_filename("nonexistent_bad.inja");
        badOut->set_output_filepath("work/bad_export.h");
        badOut->set_enabled(true);
        cfgWin->deserialize(*cfg);
        delete cfg;

        dismissModal(QString(), QMessageBox::Ok);
        win.btnExport();
    }

    // 19. btnExport with zero templates found (lines 1524-1525)
    {
        QDir().mkpath("work/empty_tmpl");
        RegMapWindow win("examples/rmt/peripherals/spi.rmt");
        auto *cfgWin = win.configWindow();
        cfgWin->setTemplateFolders({"work/empty_tmpl"});
        auto *cfg = cfgWin->serialize();
        cfg->clear_template_outputs();
        cfgWin->deserialize(*cfg);
        delete cfg;

        dismissModal(QString(), QMessageBox::Ok);
        win.btnExport();
    }

    // 20. insertChild climbs up hierarchy from blk to root (lines 1889-1890)
    {
        RegMapWindow win("examples/rmt/peripherals/spi.rmt");
        auto *treeView = win.findChild<QTreeView*>("treeView");
        QVERIFY(treeView);
        auto *proxy = qobject_cast<QSortFilterProxyModel*>(treeView->model());
        QVERIFY(proxy);
        QModelIndex blk0 = proxy->index(0, 0);
        treeView->selectionModel()->select(blk0, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        treeView->setCurrentIndex(blk0);
        win.insertChild(RegMapTreeItem::e_rmmKind::fld);
    }

    // 21. duplicateSelectedRegister with cleared current index but m_currentRegItem cached (lines 2119-2125)
    {
        RegMapWindow win("examples/rmt/peripherals/spi.rmt");
        auto *treeView = win.findChild<QTreeView*>("treeView");
        auto *proxy = qobject_cast<QSortFilterProxyModel*>(treeView->model());
        QModelIndex blk0 = proxy->index(0, 0);
        QModelIndex reg0 = proxy->index(0, 0, blk0);
        treeView->setCurrentIndex(reg0);

        auto *fieldsTable = win.findChild<QTableView*>("fieldsTableView");
        if (fieldsTable) {
            fieldsTable->selectionModel()->clearSelection();
            fieldsTable->setCurrentIndex(QModelIndex());
            fieldsTable->clearFocus();
        }
        treeView->selectionModel()->blockSignals(true);
        treeView->selectionModel()->clearSelection();
        treeView->setCurrentIndex(QModelIndex());
        treeView->selectionModel()->blockSignals(false);

        win.duplicateSelectedRegister();

        // Also test branch where blk->parentItem() != m_model->getRootItem() (line 2121)
        auto *model = win.model();
        QModelIndex blk0Source = proxy->mapToSource(blk0);
        model->insertRows(model->rowCount(blk0Source), 1, RegMapTreeItem::e_rmmKind::blk, blk0Source);
        QModelIndex subBlkSource = model->index(model->rowCount(blk0Source) - 1, 0, blk0Source);
        model->insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, subBlkSource);
        QModelIndex subRegSource = model->index(0, 0, subBlkSource);
        QModelIndex subRegProxy = proxy->mapFromSource(subRegSource);
        treeView->setCurrentIndex(subRegProxy);

        if (fieldsTable) {
            fieldsTable->selectionModel()->clearSelection();
            fieldsTable->setCurrentIndex(QModelIndex());
            fieldsTable->clearFocus();
        }
        treeView->selectionModel()->blockSignals(true);
        treeView->selectionModel()->clearSelection();
        treeView->setCurrentIndex(QModelIndex());
        treeView->selectionModel()->blockSignals(false);

        win.duplicateSelectedRegister();
    }

    // 22. headlessExport with model validation warnings (lines 2149-2152)
    {
        QFile::remove("work/temp_invalid.rmt");
        QFile::copy("examples/rmt/validation/invalid_overlap.rmt", "work/temp_invalid.rmt");
        RegMapWindow win("work/temp_invalid.rmt");
        win.headlessExport("work/invalid_export");
    }

    // 23. parseNumericValue binary and decimal in field duplicate (lines 44, 45, 47)
    {
        RegMapWindow win("examples/rmt/peripherals/spi.rmt");
        auto *treeView = win.findChild<QTreeView*>("treeView");
        auto *proxy = qobject_cast<QSortFilterProxyModel*>(treeView->model());
        QModelIndex blk0 = proxy->index(0, 0);
        QModelIndex reg0 = proxy->index(0, 0, blk0);
        treeView->setCurrentIndex(reg0);

        auto *fieldsTable = win.findChild<QTableView*>("fieldsTableView");
        QVERIFY(fieldsTable);
        auto *fieldProxy = qobject_cast<QSortFilterProxyModel*>(fieldsTable->model());
        QVERIFY(fieldProxy);
        QVERIFY(fieldProxy->rowCount() > 0);

        QModelIndex fld0Offset = fieldProxy->index(0, 1);
        QModelIndex fld0Width = fieldProxy->index(0, 2);
        fieldProxy->setData(fld0Offset, "0b01", Qt::EditRole);
        fieldProxy->setData(fld0Width, "4", Qt::EditRole);

        fieldsTable->setCurrentIndex(fieldProxy->index(0, 0));
        fieldsTable->setFocus();
        win.duplicateSelectedRegister();
    }

    // 24. UndoCommands: EditCellCommand invalid index, InsertItemCommand kinds, and null DeleteItemCommand
    {
        RegMapWindow win("examples/rmt/peripherals/spi.rmt");
        auto *model = win.model();

        EditCellCommand invalidCell(model, QModelIndex(), "old", "new");
        invalidCell.undo();
        invalidCell.redo();

        InsertItemCommand cmdBlk(model, RegMapTreeItem::e_rmmKind::blk, 0, QModelIndex());
        QCOMPARE(cmdBlk.text(), QString("Add Block"));
        InsertItemCommand cmdReg(model, RegMapTreeItem::e_rmmKind::reg, 0, QModelIndex());
        QCOMPARE(cmdReg.text(), QString("Add Register"));
        InsertItemCommand cmdFld(model, RegMapTreeItem::e_rmmKind::fld, 0, QModelIndex());
        QCOMPARE(cmdFld.text(), QString("Add Field"));
        InsertItemCommand cmdMem(model, RegMapTreeItem::e_rmmKind::mem, 0, QModelIndex());
        QCOMPARE(cmdMem.text(), QString("Add Memory"));
        InsertItemCommand cmdMap(model, RegMapTreeItem::e_rmmKind::map, 0, QModelIndex());
        QCOMPARE(cmdMap.text(), QString("Add Map"));

        DeleteItemCommand cmdDelNull(model, 9999, QModelIndex());
        DeleteItemCommand::StoredNode node;
        DeleteItemCommand::captureItem(nullptr, node);
        DeleteItemCommand::restoreItem(model, QModelIndex(), node);

        DeleteItemCommand::StoredNode emptyNode;
        emptyNode.kind = RegMapTreeItem::e_rmmKind::reg;
        emptyNode.colData["Name"] = "";
        DuplicateItemCommand dupEmpty(model, 0, QModelIndex(), emptyNode);
        QCOMPARE(dupEmpty.text(), QString("Duplicate Item"));
    }

    // 25. btnKeyBindings dialog dismissal
    {
        RegMapWindow win("examples/rmt/peripherals/spi.rmt");
        QTimer::singleShot(50, []() {
            for (QWidget *w : QApplication::topLevelWidgets()) {
                if (auto *dlg = qobject_cast<QDialog*>(w)) {
                    dlg->accept();
                }
            }
        });
        win.btnKeyBindings();
    }

    // 26. btnPreferences, btnAbout, btnConfig, and btnQuitButton
    {
        RegMapWindow win("examples/rmt/peripherals/spi.rmt");
        win.btnPreferences();
        win.btnAbout();
        win.btnConfig();
        win.btnQuitButton();
    }

    // 27. btnFileReload, btnFileNew, btnFileClose with unsaved changes permutations
    {
        RegMapWindow win("examples/rmt/peripherals/spi.rmt");
        // Reload when not modified
        win.btnFileReload();

        // Reload when modified with Cancel
        win.regmap_modified();
        dismissModal(QString(), QMessageBox::Cancel);
        win.btnFileReload();

        // Reload when modified with Ok
        dismissModal(QString(), QMessageBox::Ok);
        win.btnFileReload();

        // btnFileNew when not modified
        win.btnFileNew();

        // btnFileNew when modified with Cancel
        win.regmap_modified();
        dismissModal(QString(), QMessageBox::Cancel);
        win.btnFileNew();

        // btnFileNew when modified with Ok
        dismissModal(QString(), QMessageBox::Ok);
        win.btnFileNew();

        // btnFileClose when not modified
        win.btnFileClose();

        // btnFileClose when modified with Cancel
        win.regmap_modified();
        dismissModal(QString(), QMessageBox::Cancel);
        win.btnFileClose();

        // btnFileClose when modified with Ok
        dismissModal(QString(), QMessageBox::Ok);
        win.btnFileClose();
    }

    // 28. btnExport with validation issues: user selects No, user selects Yes
    {
        QFile::remove("work/temp_invalid.rmt");
        QFile::copy("examples/rmt/validation/invalid_overlap.rmt", "work/temp_invalid.rmt");
        RegMapWindow win("work/temp_invalid.rmt");

        // User clicks No -> export aborted early (line 1468)
        dismissModal(QString(), QMessageBox::No);
        win.btnExport();

        // User clicks Yes -> export proceeds anyway (lines 1466, 1509-1530)
        dismissModal(QString(), QMessageBox::Yes);
        dismissModal(QString(), QMessageBox::Ok);
        win.btnExport();
    }

    // 29. showTreeContextMenu on treeView and fieldsTableView
    {
        RegMapWindow win("examples/rmt/peripherals/spi.rmt");
        auto *treeView = win.findChild<QTreeView*>("treeView");
        auto *fieldsTable = win.findChild<QTableView*>("fieldsTableView");

        if (treeView) {
            QTimer::singleShot(30, [&win]() {
                if (auto *menu = win.findChild<QMenu*>("treeContextMenu")) {
                    if (!menu->actions().isEmpty()) {
                        menu->actions().first()->trigger();
                    }
                    menu->close();
                }
            });
            emit treeView->customContextMenuRequested(QPoint(5, 5));

            QTimer::singleShot(30, [&win]() {
                if (auto *menu = win.findChild<QMenu*>("treeContextMenu")) {
                    menu->close();
                }
            });
            emit treeView->customContextMenuRequested(QPoint(9999, 9999));
        }

        if (fieldsTable) {
            QTimer::singleShot(30, [&win]() {
                if (auto *menu = win.findChild<QMenu*>("treeContextMenu")) {
                    if (menu->actions().size() > 1) {
                        menu->actions().at(1)->trigger();
                    }
                    menu->close();
                }
            });
            emit fieldsTable->customContextMenuRequested(QPoint(5, 5));

            QTimer::singleShot(30, [&win]() {
                if (auto *menu = win.findChild<QMenu*>("treeContextMenu")) {
                    menu->close();
                }
            });
            emit fieldsTable->customContextMenuRequested(QPoint(9999, 9999));
        }
    }

    // 30. duplicateItem variations: source model index, invalid index, blk item, regBytes == 0
    {
        RegMapWindow win("examples/rmt/peripherals/spi.rmt");
        auto *model = win.model();

        // Passing source model index directly (line 2141-2142)
        QModelIndex srcBlk = model->index(0, 0, QModelIndex());
        QModelIndex srcReg = model->index(0, 0, srcBlk);
        win.duplicateItem(srcReg);

        // Passing invalid index or root item (line 2147: returns early)
        win.duplicateItem(QModelIndex());

        // Duplicate a block item (lines 2161 and 2165 false)
        win.duplicateItem(srcBlk);

        // Duplicate when reg_width = 7 (regBytes == 0 -> falls back to 4, line 2156)
        auto *cfgWin = win.configWindow();
        cfgWin->setRegisterWidth(7);
        win.duplicateItem(srcReg);
        cfgWin->setRegisterWidth(32);
    }

    // 31. semanticDiff comprehensive comparisons with added, removed, modified registers and fields
    {
        // Diff between spi.rmt and uart.rmt (added and removed registers)
        QVERIFY(RegMapWindow::semanticDiff(
            "examples/rmt/peripherals/spi.rmt",
            "examples/rmt/peripherals/uart.rmt",
            "text",
            "work/diff_spi_uart.txt"
        ));
        QVERIFY(QFile::exists("work/diff_spi_uart.txt"));

        // Markdown format diff between spi.rmt and uart.rmt
        QVERIFY(RegMapWindow::semanticDiff(
            "examples/rmt/peripherals/spi.rmt",
            "examples/rmt/peripherals/uart.rmt",
            "markdown",
            "work/diff_spi_uart.md"
        ));
        QVERIFY(QFile::exists("work/diff_spi_uart.md"));

        // Diff where file1 is valid, but file2 does not exist (line 2416-2419 returns false)
        QVERIFY(!RegMapWindow::semanticDiff(
            "examples/rmt/peripherals/spi.rmt",
            "nonexistent_file_diff_123.rmt",
            "text",
            ""
        ));

        // Diff write failure on invalid path (line 2505-2506)
        QVERIFY(!RegMapWindow::semanticDiff(
            "examples/rmt/peripherals/spi.rmt",
            "examples/rmt/peripherals/uart.rmt",
            "text",
            "/non_existent_directory_xyz/diff.txt"
        ));

        // Create modified SPI map with offset, access, reset, and field modifications
        QFile::remove("work/spi_modified.rmt");
        QFile::copy("examples/rmt/peripherals/spi.rmt", "work/spi_modified.rmt");
        {
            RegMapWindow modWin("work/spi_modified.rmt");
            auto *modModel = modWin.model();
            QModelIndex b0 = modModel->index(0, 0, QModelIndex());
            QModelIndex r0 = modModel->index(0, 0, b0);
            modModel->setData(modModel->index(0, 1, b0), "0x100", Qt::EditRole); // Offset changed
            modModel->setData(modModel->index(0, 4, b0), "RO", Qt::EditRole);    // Access changed
            modModel->setData(modModel->index(0, 6, b0), "0xFF", Qt::EditRole);  // Reset changed
            // Modify field 0
            modModel->setData(modModel->index(0, 4, r0), "RO", Qt::EditRole); // Field access changed
            // Add a field
            modModel->insertRows(modModel->rowCount(r0), 1, RegMapTreeItem::e_rmmKind::fld, r0);
            modModel->setData(modModel->index(modModel->rowCount(r0)-1, 3, r0), "NEW_FLD", Qt::EditRole);
            // Remove a field
            if (modModel->rowCount(r0) > 2) {
                modModel->removeRows(1, 1, r0);
            }
            modWin.fileSave("work/spi_modified.rmt");
        }

        // Diff spi.rmt vs spi_modified.rmt in both text and markdown formats
        QVERIFY(RegMapWindow::semanticDiff(
            "examples/rmt/peripherals/spi.rmt",
            "work/spi_modified.rmt",
            "text",
            "work/diff_mod.txt"
        ));
        QVERIFY(RegMapWindow::semanticDiff(
            "examples/rmt/peripherals/spi.rmt",
            "work/spi_modified.rmt",
            "markdown",
            "work/diff_mod.md"
        ));
    }

    // 32. setColourBlindType permutations and window state restoration
    {
        RegMapWindow win("examples/rmt/peripherals/spi.rmt");

        // Set colour blind type to None
        win.setColourBlindType(ColorBlindMode::None);
        QCOMPARE(win.colourBlindType(), ColorBlindMode::None);

        // When not in colour blind mode, setting a type turns it on
        win.setColourBlindType(ColorBlindMode::Protanopia);
        QCOMPARE(win.colourBlindType(), ColorBlindMode::Protanopia);

        // When already in colour blind mode, setting another type changes it directly (lines 850-878)
        win.setColourBlindType(ColorBlindMode::Deuteranopia);
        QCOMPARE(win.colourBlindType(), ColorBlindMode::Deuteranopia);
        win.setColourBlindType(ColorBlindMode::Tritanopia);
        QCOMPARE(win.colourBlindType(), ColorBlindMode::Tritanopia);
        win.setColourBlindType(ColorBlindMode::Achromatopsia);
        QCOMPARE(win.colourBlindType(), ColorBlindMode::Achromatopsia);

        // Restore window state with empty geometry, valid size and valid pos
        AppSettings::instance().setMainWindowGeometry(QByteArray());
        AppSettings::instance().setMainWindowSize(QSize(1024, 768));
        AppSettings::instance().setMainWindowPos(QPoint(50, 50));
        AppSettings::instance().setMainWindowState(QByteArray());
        win.restoreWindowStateFromSettings();

        // Restore window state with valid geometry and state
        win.saveWindowStateToSettings();
        win.restoreWindowStateFromSettings();

        // Fallback size <= 0
        AppSettings::instance().setMainWindowGeometry(QByteArray());
        AppSettings::instance().setMainWindowSize(QSize(0, 0));
        AppSettings::instance().setMainWindowPos(QPoint());
        win.restoreWindowStateFromSettings();
    }

    // 33. insertChild fallback to first child when root cannot accept (lines 1907-1915)
    {
        RegMapWindow win("examples/rmt/peripherals/spi.rmt");
        auto *treeView = win.findChild<QTreeView*>("treeView");
        // Clear selection so nothing is selected
        if (treeView && treeView->selectionModel()) {
            treeView->selectionModel()->clearSelection();
            treeView->setCurrentIndex(QModelIndex());
        }
        // Inserting a reg with nothing selected: root cannot accept reg, but firstChild (blk) can!
        win.insertChild(RegMapTreeItem::e_rmmKind::reg);

        // Empty window: root cannot accept fld, childCount == 0, targetParentItem is null -> returns (lines 1918-1920)
        RegMapWindow emptyWin;
        emptyWin.insertChild(RegMapTreeItem::e_rmmKind::fld);
    }

    // 34. Protobuf operator >> and << with parent_id == id == 0 and unknown item kind
    {
        protormap::RegModel regModel;
        protormap::RegItem *item = regModel.add_item();
        item->set_kind(protormap::RegItem_Kind_BLK);
        item->set_id(0);
        item->set_parent_id(0); // triggers (item.parent_id() == item.id() && item.id() == 0)

        protormap::RegItem *unknownItem = regModel.add_item();
        unknownItem->set_kind(static_cast<protormap::RegItem_Kind>(999)); // triggers default: ; in operator >>

        SerializationContext ctxIn;
        regModel >> ctxIn;
    }

    // 35. Export helper methods permutations
    {
        RegMapWindow win("examples/rmt/peripherals/spi.rmt");
        protormap::Config cfg;

        // resolveExportOutputFolder
        QCOMPARE(RegMapWindow::resolveExportOutputFolder("custom/out", &cfg), std::string("custom/out"));
        cfg.set_outputfolder("cfg/out");
        QCOMPARE(RegMapWindow::resolveExportOutputFolder("", &cfg), std::string("cfg/out"));
        cfg.clear_outputfolder();
        QCOMPARE(RegMapWindow::resolveExportOutputFolder("", &cfg), std::string(PathUtils::DEFAULT_OUTPUT_DIR));
        QCOMPARE(RegMapWindow::resolveExportOutputFolder("", nullptr), std::string(PathUtils::DEFAULT_OUTPUT_DIR));

        // resolveExportProjectName
        nlohmann::json j1;
        cfg.set_project_name("MyProject");
        RegMapWindow::resolveExportProjectName(&cfg, "my_file.rmt", j1);
        QCOMPARE(j1["name"].get<std::string>(), std::string("MyProject"));

        nlohmann::json j2;
        cfg.clear_project_name();
        RegMapWindow::resolveExportProjectName(&cfg, "my_file.rmt", j2);
        QCOMPARE(j2["name"].get<std::string>(), std::string("my_file"));

        nlohmann::json j3;
        j3["name"] = "regmap";
        RegMapWindow::resolveExportProjectName(&cfg, "another_file.rmt", j3);
        QCOMPARE(j3["name"].get<std::string>(), std::string("another_file"));

        nlohmann::json j4;
        j4["name"] = "custom_chip";
        RegMapWindow::resolveExportProjectName(&cfg, "another_file.rmt", j4);
        QCOMPARE(j4["name"].get<std::string>(), std::string("custom_chip"));

        nlohmann::json j5;
        RegMapWindow::resolveExportProjectName(&cfg, "", j5);
        QVERIFY(!j5.contains("name"));

        nlohmann::json j6;
        RegMapWindow::resolveExportProjectName(nullptr, "null_cfg.rmt", j6);
        QCOMPARE(j6["name"].get<std::string>(), std::string("null_cfg"));

        // isExportPythonEnabled
        QCOMPARE(win.isExportPythonEnabled(nullptr), false);

        cfg.set_python_script_enabled(true);
        QCOMPARE(win.isExportPythonEnabled(&cfg), true);
        cfg.set_python_script_enabled(false);
        QCOMPARE(win.isExportPythonEnabled(&cfg), false);

        cfg.clear_python_script_enabled();
        bool expected = win.configWindow() ? win.configWindow()->isPythonScriptEnabled() : false;
        QCOMPARE(win.isExportPythonEnabled(&cfg), expected);

        // Fallback when m_config_window is null
        RegConfigWindow *savedCfgWin = win.m_config_window;
        win.m_config_window = nullptr;
        cfg.set_pythonscript("gen.py");
        QCOMPARE(win.isExportPythonEnabled(&cfg), true);
        cfg.set_pythonscript("");
        QCOMPARE(win.isExportPythonEnabled(&cfg), false);
        win.m_config_window = savedCfgWin;
    }

    // 36. TreeFilterProxyModel and FieldSortProxyModel comprehensive sort, filter, headerData
    {
        RegMapWindow win("examples/rmt/peripherals/spi.rmt");
        auto *treeView = win.findChild<QTreeView*>("treeView");
        auto *searchEdit = win.findChild<QLineEdit*>("searchEdit");
        auto *fieldsTable = win.findChild<QTableView*>("fieldsTableView");
        QVERIFY(treeView && searchEdit && fieldsTable);

        auto *treeProxy = qobject_cast<QSortFilterProxyModel*>(treeView->model());
        auto *fieldProxy = qobject_cast<QSortFilterProxyModel*>(fieldsTable->model());
        QVERIFY(treeProxy && fieldProxy);

        // headerData tests
        QCOMPARE(treeProxy->headerData(1, Qt::Horizontal).toString(), QString("Offset"));
        QCOMPARE(treeProxy->headerData(2, Qt::Horizontal).toString(), QString("Size"));
        QVERIFY(!treeProxy->headerData(0, Qt::Horizontal).toString().isEmpty());
        QVERIFY(!treeProxy->headerData(0, Qt::Vertical).isValid());

        // Filter tests
        searchEdit->setText(""); // empty filter -> accepts all non-fld
        searchEdit->setText("spi"); // matches block name
        searchEdit->setText("cr1"); // matches register name
        searchEdit->setText("spe"); // matches child field name (recursive match)
        searchEdit->setText("0x0"); // matches offset
        searchEdit->setText("rw"); // matches access policy
        searchEdit->setText("control"); // matches description
        searchEdit->setText("nonexistent_search_query_99999"); // matches nothing
        searchEdit->setText(""); // reset

        // TreeView sorting across all columns
        for (int col = 0; col < 11; ++col) {
            treeView->sortByColumn(col, Qt::AscendingOrder);
            treeView->sortByColumn(col, Qt::DescendingOrder);
        }

        // FieldsTable sorting across all columns
        for (int col = 0; col < 11; ++col) {
            fieldsTable->sortByColumn(col, Qt::AscendingOrder);
            fieldsTable->sortByColumn(col, Qt::DescendingOrder);
        }
    }

    // 37. gatherRegs traversal branches and batchExport custom out_dir prefixes
    {
        // Construct models with non-blk directly under root, non-reg under blk, non-fld under reg
        RegMapTreeModel modelA;
        RegMapTreeItem *rootA = modelA.getRootItem();
        QVariantMap emptyData;
        
        // 1. Non-block item directly under root (e.g. mem)
        RegMapTreeItem *memUnderRoot = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::mem, emptyData, rootA);
        rootA->appendChild(memUnderRoot);

        // 2. Valid block under root
        RegMapTreeItem *blkA = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::blk, emptyData, rootA);
        blkA->setData("Name", "BLK_TEST");
        rootA->appendChild(blkA);

        // 3. Non-reg item directly under blk (e.g. mem)
        RegMapTreeItem *memUnderBlk = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::mem, emptyData, blkA);
        blkA->appendChild(memUnderBlk);

        // 4. Valid reg under blk
        RegMapTreeItem *regA = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, emptyData, blkA);
        regA->setData("Name", "REG_TEST");
        regA->setData("Offset/LSB", "0x0");
        blkA->appendChild(regA);

        // 5. Non-fld item directly under reg (e.g. mem)
        RegMapTreeItem *memUnderReg = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::mem, emptyData, regA);
        regA->appendChild(memUnderReg);

        // 6. Valid fld under reg
        RegMapTreeItem *fldA = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::fld, emptyData, regA);
        fldA->setData("Name", "FLD1");
        fldA->setData("Offset/LSB", "0");
        fldA->setData("Size/Width", "1");
        regA->appendChild(fldA);

        FormatManager::instance().saveFile("work/test_gather_a.rmt", &modelA, nullptr);

        RegMapTreeModel modelB;
        RegMapTreeItem *rootB = modelB.getRootItem();
        RegMapTreeItem *blkB = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::blk, emptyData, rootB);
        blkB->setData("Name", "BLK_TEST");
        rootB->appendChild(blkB);

        RegMapTreeItem *regB = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, emptyData, blkB);
        regB->setData("Name", "REG_TEST");
        regB->setData("Offset/LSB", "0x4");
        blkB->appendChild(regB);

        RegMapTreeItem *fldB = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::fld, emptyData, regB);
        fldB->setData("Name", "FLD1");
        fldB->setData("Offset/LSB", "0");
        fldB->setData("Size/Width", "8");
        regB->appendChild(fldB);

        FormatManager::instance().saveFile("work/test_gather_b.rmt", &modelB, nullptr);

        QVERIFY(RegMapWindow::semanticDiff("work/test_gather_a.rmt", "work/test_gather_b.rmt", "text", "work/diff_gather.txt"));
        QVERIFY(RegMapWindow::semanticDiff("work/test_gather_a.rmt", "work/test_gather_b.rmt", "markdown", "work/diff_gather.md"));

        // Batch export with custom out_dir prefixes
        RegMapWindow win("examples/rmt/peripherals/spi.rmt");
        win.headlessExport("work/custom_export_1");
        win.headlessExport("./work/custom_export_2");
    }

    // 38. Comprehensive branch & condition boost: proxy models, editor focus, strict linter, duplication, undo text
    {
        RegMapWindow win("examples/rmt/peripherals/spi.rmt");
        auto *treeView = win.findChild<QTreeView*>("treeView");
        auto *fieldsTable = win.findChild<QTableView*>("fieldsTableView");
        auto *treeProxy = qobject_cast<QSortFilterProxyModel*>(treeView->model());
        auto *fieldProxy = qobject_cast<QSortFilterProxyModel*>(fieldsTable->model());
        auto *undoStack = win.findChild<QUndoStack*>();
        auto *model = win.getModel();
        QVERIFY(treeView && fieldsTable && treeProxy && fieldProxy && undoStack && model);

        win.show();
        QApplication::processEvents();

        // 1. Undo / Redo text updates (empty command text vs non-empty command text)
        undoStack->push(new QUndoCommand(""));
        undoStack->undo();
        undoStack->redo();
        undoStack->push(new QUndoCommand("CustomOperation"));
        undoStack->undo();
        undoStack->redo();


        // Add registers with 0b, 0x, decimal offsets and sizes
        model->insertRows(model->rowCount(), 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
        QModelIndex newBlk = model->index(model->rowCount() - 1, 0, QModelIndex());
        model->setData(model->index(newBlk.row(), 3, QModelIndex()), "BLK_PROXY", Qt::EditRole);

        // Reg 1: binary offset 0b10, size 16, reset 0b101
        model->insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, newBlk);
        QModelIndex r1 = model->index(0, 0, newBlk);
        model->setData(model->index(0, 1, newBlk), "0b10", Qt::EditRole);
        model->setData(model->index(0, 2, newBlk), "16", Qt::EditRole);
        model->setData(model->index(0, 6, newBlk), "0b101", Qt::EditRole);

        // Reg 2: binary offset 0b100, size 0x20, reset 0x10
        model->insertRows(1, 1, RegMapTreeItem::e_rmmKind::reg, newBlk);
        QModelIndex r2 = model->index(1, 0, newBlk);
        model->setData(model->index(1, 1, newBlk), "0b100", Qt::EditRole);
        model->setData(model->index(1, 2, newBlk), "0x20", Qt::EditRole);
        model->setData(model->index(1, 6, newBlk), "0x10", Qt::EditRole);

        // Field 1: offset 0b0, size 0b10, reset 0x5
        model->insertRows(0, 1, RegMapTreeItem::e_rmmKind::fld, r1);
        model->setData(model->index(0, 1, r1), "0b0", Qt::EditRole);
        model->setData(model->index(0, 2, r1), "0b10", Qt::EditRole);
        model->setData(model->index(0, 6, r1), "0x5", Qt::EditRole);

        // Field 2: offset 0b10, size 0x4, reset 0b11
        model->insertRows(1, 1, RegMapTreeItem::e_rmmKind::fld, r1);
        model->setData(model->index(1, 1, r1), "0b10", Qt::EditRole);
        model->setData(model->index(1, 2, r1), "0x4", Qt::EditRole);
        model->setData(model->index(1, 6, r1), "0b11", Qt::EditRole);

        // Trigger sort on tree proxy columns 1, 2, 6
        treeProxy->sort(1, Qt::AscendingOrder);
        treeProxy->sort(1, Qt::DescendingOrder);
        treeProxy->sort(2, Qt::AscendingOrder);
        treeProxy->sort(2, Qt::DescendingOrder);
        treeProxy->sort(6, Qt::AscendingOrder);
        treeProxy->sort(6, Qt::DescendingOrder);

        // Trigger sort on field proxy columns 1, 2, 6
        fieldProxy->sort(1, Qt::AscendingOrder);
        fieldProxy->sort(1, Qt::DescendingOrder);
        fieldProxy->sort(2, Qt::AscendingOrder);
        fieldProxy->sort(2, Qt::DescendingOrder);
        fieldProxy->sort(6, Qt::AscendingOrder);
        fieldProxy->sort(6, Qt::DescendingOrder);

        // 3. Selection change with field having focus and duplicating field
        QModelIndex regProxyIdx = treeProxy->mapFromSource(r1);
        treeView->setCurrentIndex(regProxyIdx);
        QApplication::processEvents();

        QModelIndex fldProxyIdx = fieldProxy->index(0, 0);
        fieldsTable->selectionModel()->select(fldProxyIdx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        fieldsTable->setCurrentIndex(fldProxyIdx);
        fieldsTable->setFocus();
        QApplication::processEvents();
        win.duplicateSelectedRegister();

        // Duplication via fallback when proxyIndex is invalid but m_currentRegItem is set
        treeView->setCurrentIndex(regProxyIdx);
        QApplication::processEvents();
        treeView->selectionModel()->clearSelection();
        treeView->setCurrentIndex(QModelIndex());
        fieldsTable->selectionModel()->clearSelection();
        fieldsTable->setCurrentIndex(QModelIndex());
        fieldsTable->clearFocus();
        treeView->clearFocus();
        win.duplicateSelectedRegister();

        // Duplication with direct source index, invalid index, and root index
        win.duplicateItem(r1);
        win.duplicateItem(QModelIndex());
        QModelIndex rootSrcIdx = model->index(0, 0, QModelIndex()).parent();
        win.duplicateItem(rootSrcIdx);

        // 4. Insertion with nothing selected in tree
        treeView->selectionModel()->clearSelection();
        treeView->setCurrentIndex(QModelIndex());
        win.insertChild(RegMapTreeItem::e_rmmKind::blk);
        treeView->selectionModel()->clearSelection();
        treeView->setCurrentIndex(QModelIndex());
        win.insertChild(RegMapTreeItem::e_rmmKind::reg);

        // 5. Selecting non-reg and non-blk/map item (e.g. mem node)
        model->insertRows(model->rowCount(), 1, RegMapTreeItem::e_rmmKind::mem, QModelIndex());
        QModelIndex memProxyIdx = treeProxy->mapFromSource(model->index(model->rowCount() - 1, 0, QModelIndex()));
        treeView->selectionModel()->select(memProxyIdx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

        // 6. Delete item with no selection / invalid index
        treeView->selectionModel()->clearSelection();
        treeView->setCurrentIndex(QModelIndex());
        win.btnDeleteItem();

        // 7. Focus on header line edits during updates
        auto *regNameEdit = win.findChild<QLineEdit*>("regNameEdit");
        auto *regOffsetEdit = win.findChild<QLineEdit*>("regOffsetEdit");
        auto *regDescEdit = win.findChild<QLineEdit*>("regDescEdit");
        auto *blkNameEdit = win.findChild<QLineEdit*>("blkNameEdit");
        auto *blkOffsetEdit = win.findChild<QLineEdit*>("blkOffsetEdit");
        auto *blkDescEdit = win.findChild<QLineEdit*>("blkDescEdit");

        // Select a register
        regProxyIdx = treeProxy->mapFromSource(r1);
        treeView->setCurrentIndex(regProxyIdx);
        QApplication::processEvents();

        if (regNameEdit) {
            regNameEdit->setFocus();
            emit model->dataChanged(r1, r1);
            regNameEdit->clearFocus();
        }
        if (regOffsetEdit) {
            regOffsetEdit->setFocus();
            emit model->dataChanged(r1, r1);
            regOffsetEdit->clearFocus();
        }
        if (regDescEdit) {
            regDescEdit->setFocus();
            emit model->dataChanged(r1, r1);
            regDescEdit->clearFocus();
        }

        // Select a block
        QModelIndex blkProxyIdx = treeProxy->mapFromSource(newBlk);
        treeView->setCurrentIndex(blkProxyIdx);
        QApplication::processEvents();

        if (blkNameEdit) {
            blkNameEdit->setFocus();
            emit model->dataChanged(newBlk, newBlk);
            blkNameEdit->clearFocus();
        }
        if (blkOffsetEdit) {
            blkOffsetEdit->setFocus();
            emit model->dataChanged(newBlk, newBlk);
            blkOffsetEdit->clearFocus();
        }
        if (blkDescEdit) {
            blkDescEdit->setFocus();
            emit model->dataChanged(newBlk, newBlk);
            blkDescEdit->clearFocus();
        }

        // 8. Strict linting: 64-bit offset (>= 0x100000000ULL), >= 0x10000ULL, decimal offset, unaligned offset, empty desc
        RegMapTreeModel lintModel;
        lintModel.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
        QModelIndex lBlk = lintModel.index(0, 0, QModelIndex());
        lintModel.setData(lintModel.index(0, 1, QModelIndex()), "0x1000000000", Qt::EditRole); // 64-bit offset
        lintModel.setData(lintModel.index(0, 3, QModelIndex()), "BLK_64", Qt::EditRole);
        lintModel.setData(lintModel.index(0, 10, QModelIndex()), "", Qt::EditRole); // empty desc

        // Reg with offset >= 0x10000, unaligned offset 0x3 (off % 4 != 0), empty desc
        lintModel.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, lBlk);
        QModelIndex lReg1 = lintModel.index(0, 0, lBlk);
        lintModel.setData(lintModel.index(0, 1, lBlk), "0x10003", Qt::EditRole); // unaligned + >= 0x10000
        lintModel.setData(lintModel.index(0, 3, lBlk), "REG_UNALIGNED", Qt::EditRole);
        lintModel.setData(lintModel.index(0, 10, lBlk), "", Qt::EditRole); // empty desc

        // Reg with decimal offset string "1024"
        lintModel.insertRows(1, 1, RegMapTreeItem::e_rmmKind::reg, lBlk);
        QModelIndex lReg2 = lintModel.index(1, 0, lBlk);
        lintModel.setData(lintModel.index(1, 1, lBlk), "1024", Qt::EditRole);
        lintModel.setData(lintModel.index(1, 3, lBlk), "REG_DEC", Qt::EditRole);
        lintModel.setData(lintModel.index(1, 10, lBlk), "Has desc", Qt::EditRole);

        // Field with empty description
        lintModel.insertRows(0, 1, RegMapTreeItem::e_rmmKind::fld, lReg1);
        lintModel.setData(lintModel.index(0, 3, lReg1), "FLD_NO_DESC", Qt::EditRole);
        lintModel.setData(lintModel.index(0, 10, lReg1), "", Qt::EditRole);

        FormatManager::instance().saveFile("work/test_lint_fail.rmt", &lintModel, nullptr);

        RegMapWindow lintWin("work/test_lint_fail.rmt");
        lintWin.headlessLint(true, "json", "work/lint_fail.json");
        lintWin.headlessLint(true, "sarif", "work/lint_fail.sarif");
        lintWin.headlessLint(true, "junit", "work/lint_fail.junit");
        lintWin.headlessLint(true, "text", "work/lint_fail.txt");
    }
}

QTEST_MAIN(TestRegMapWindow)
#include "test_RegMapWindow.moc"
