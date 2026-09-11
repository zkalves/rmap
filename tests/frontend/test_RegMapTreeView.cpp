/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include <QtTest>
#include <QPainter>
#include <QSignalSpy>
#include <QDir>
#include <QContextMenuEvent>
#include <QKeyEvent>
#include "RegMapTreeView.hpp"
#include "RegMapTreeModel.hpp"
#include "RegMapTreeItem.hpp"

class TestRegMapTreeView : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testConstructorAndProperties();
    void testMousePressEmptySpaceClearsSelection();
    void testMousePressOnItemRow();
    void testRightClickAndContextMenu();
    void testKeyboardEvents();
    void testViewportRendering();
};

void TestRegMapTreeView::initTestCase()
{
    QDir("work").removeRecursively();
    QDir().mkpath("work");
}

void TestRegMapTreeView::cleanupTestCase()
{
    QDir("work").removeRecursively();
}

void TestRegMapTreeView::testConstructorAndProperties()
{
    // Construct without parent
    RegMapTreeView treeView;
    QVERIFY(treeView.parent() == nullptr);

    // Construct with parent
    QWidget parentWidget;
    RegMapTreeView childTreeView(&parentWidget);
    QCOMPARE(childTreeView.parent(), &parentWidget);
}

void TestRegMapTreeView::testMousePressEmptySpaceClearsSelection()
{
    RegMapTreeModel model;
    RegMapTreeItem *root = model.getRootItem();

    QVariantMap blkData;
    blkData["Type"] = "blk";
    blkData["Name"] = "BLK0";
    blkData["Offset/LSB"] = "0x0";
    RegMapTreeItem *blk = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::blk, blkData, root);
    root->appendChild(blk);

    QVariantMap regData;
    regData["Type"] = "reg";
    regData["Name"] = "REG0";
    regData["Offset/LSB"] = "0x0";
    RegMapTreeItem *reg = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, regData, blk);
    blk->appendChild(reg);

    RegMapTreeView treeView;
    treeView.setModel(&model);
    treeView.resize(400, 300);
    treeView.expandAll();

    // Select the first block item
    QModelIndex blkIndex = model.index(0, 0, QModelIndex());
    QVERIFY(blkIndex.isValid());
    treeView.setCurrentIndex(blkIndex);
    treeView.selectionModel()->select(blkIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    QVERIFY(treeView.selectionModel()->hasSelection());

    // Send mouse press on empty space (e.g. far below the single item row)
    QPoint emptyPos(200, 250);
    QMouseEvent mousePress(QEvent::MouseButtonPress, emptyPos, emptyPos, emptyPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(treeView.viewport(), &mousePress);

    // mousePressEvent should clear selection and reset current index
    QVERIFY(!treeView.selectionModel()->hasSelection());
    QCOMPARE(treeView.currentIndex(), treeView.rootIndex());
}

void TestRegMapTreeView::testMousePressOnItemRow()
{
    RegMapTreeModel model;
    RegMapTreeItem *root = model.getRootItem();

    QVariantMap blkData;
    blkData["Type"] = "blk";
    blkData["Name"] = "PERIPH";
    blkData["Offset/LSB"] = "0x1000";
    RegMapTreeItem *blk = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::blk, blkData, root);
    root->appendChild(blk);

    RegMapTreeView treeView;
    treeView.setModel(&model);
    treeView.resize(400, 300);
    treeView.show();

    QModelIndex blkIndex = model.index(0, 0, QModelIndex());
    QRect rect = treeView.visualRect(blkIndex);
    QVERIFY(rect.isValid());

    // Click in the center of the item row
    QPoint itemCenter = rect.center();
    QMouseEvent clickEvent(QEvent::MouseButtonPress, itemCenter, itemCenter, itemCenter, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(treeView.viewport(), &clickEvent);

    // Verify item was selected
    QVERIFY(treeView.selectionModel()->hasSelection());
    QCOMPARE(treeView.currentIndex().row(), 0);
}

void TestRegMapTreeView::testRightClickAndContextMenu()
{
    RegMapTreeModel model;
    RegMapTreeItem *root = model.getRootItem();

    QVariantMap blkData;
    blkData["Type"] = "blk";
    blkData["Name"] = "PERIPH_RC";
    RegMapTreeItem *blk = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::blk, blkData, root);
    root->appendChild(blk);

    RegMapTreeView treeView;
    treeView.setModel(&model);
    treeView.resize(400, 300);
    treeView.show();

    QModelIndex blkIndex = model.index(0, 0, QModelIndex());
    QRect rect = treeView.visualRect(blkIndex);

    // Right click mouse event
    QPoint itemCenter = rect.center();
    QMouseEvent rightClick(QEvent::MouseButtonPress, itemCenter, itemCenter, itemCenter, Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    QApplication::sendEvent(treeView.viewport(), &rightClick);

    // Context menu event
    QContextMenuEvent contextEvent(QContextMenuEvent::Mouse, itemCenter, itemCenter, Qt::NoModifier);
    QApplication::sendEvent(&treeView, &contextEvent);

    // Context menu from keyboard
    QContextMenuEvent kbContext(QContextMenuEvent::Keyboard, QPoint(0, 0), QPoint(0, 0));
    QApplication::sendEvent(&treeView, &kbContext);
}

void TestRegMapTreeView::testKeyboardEvents()
{
    RegMapTreeModel model;
    RegMapTreeItem *root = model.getRootItem();

    QVariantMap blk1Data;
    blk1Data["Type"] = "blk";
    blk1Data["Name"] = "BLK1";
    RegMapTreeItem *b1 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::blk, blk1Data, root);
    root->appendChild(b1);

    QVariantMap blk2Data;
    blk2Data["Type"] = "blk";
    blk2Data["Name"] = "BLK2";
    RegMapTreeItem *b2 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::blk, blk2Data, root);
    root->appendChild(b2);

    RegMapTreeView treeView;
    treeView.setModel(&model);
    treeView.resize(400, 300);
    treeView.show();

    // Key Down
    QKeyEvent keyDown(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    QApplication::sendEvent(&treeView, &keyDown);

    // Key Up
    QKeyEvent keyUp(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    QApplication::sendEvent(&treeView, &keyUp);

    // Key Delete
    QKeyEvent keyDel(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
    QApplication::sendEvent(&treeView, &keyDel);

    // Key Insert
    QKeyEvent keyIns(QEvent::KeyPress, Qt::Key_Insert, Qt::NoModifier);
    QApplication::sendEvent(&treeView, &keyIns);

    // Key F2
    QKeyEvent keyF2(QEvent::KeyPress, Qt::Key_F2, Qt::NoModifier);
    QApplication::sendEvent(&treeView, &keyF2);
}

void TestRegMapTreeView::testViewportRendering()
{
    RegMapTreeModel model;
    RegMapTreeItem *root = model.getRootItem();

    QVariantMap blkData;
    blkData["Type"] = "blk";
    blkData["Name"] = "RENDER_BLK";
    RegMapTreeItem *blk = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::blk, blkData, root);
    root->appendChild(blk);

    RegMapTreeView treeView;
    treeView.setModel(&model);
    treeView.resize(500, 300);
    treeView.expandAll();

    QImage img(500, 300, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::white);
    QPainter p(&img);
    treeView.render(&p);
    QVERIFY(!img.isNull());
}

QTEST_MAIN(TestRegMapTreeView)
#include "test_RegMapTreeView.moc"
