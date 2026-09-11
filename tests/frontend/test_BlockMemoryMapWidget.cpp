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
#include <QToolTip>
#include "BlockMemoryMapWidget.hpp"
#include "RegMapTreeItem.hpp"
#include "ThemeManager.hpp"
#include "AppSettings.hpp"

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

class TestBlockMemoryMapWidget : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testEmptyWidgetAndSizing();
    void testColorBlindModeTransitions();
    void testBlockComputationAndGaps();
    void testAccessPolicyInferenceFromFields();
    void testPaintVariants();
    void testMouseInteractionAndTooltips();
    void testSelectionAndRefresh();
    void testZeroRegWidthAndEdgeCases();
};

void TestBlockMemoryMapWidget::initTestCase()
{
    s_originalHandler = qInstallMessageHandler(testOffscreenMessageHandler);
    QDir("work").removeRecursively();
    QDir().mkpath("work");
    qputenv("RMAP_CONFIG_FILE", "work/rmap_test_bmw.conf");
    AppSettings::instance().setConfigFilePath("work/rmap_test_bmw.conf");
    AppSettings::instance().setColorBlindMode(false);
    AppSettings::instance().setColorBlindType(ColorBlindMode::None);
    AppSettings::instance().setColorScheme("solarized8");
    ThemeManager::instance().setTheme("solarized8");
    ThemeManager::instance().setColorBlindMode(ColorBlindMode::None);
}

void TestBlockMemoryMapWidget::cleanupTestCase()
{
    AppSettings::instance().setColorBlindMode(false);
    AppSettings::instance().setColorBlindType(ColorBlindMode::None);
    ThemeManager::instance().setTheme("solarized8");
    ThemeManager::instance().setColorBlindMode(ColorBlindMode::None);
    QDir("work").removeRecursively();
    qInstallMessageHandler(s_originalHandler);
}

void TestBlockMemoryMapWidget::testEmptyWidgetAndSizing()
{
    BlockMemoryMapWidget widget;

    // Default sizing
    QCOMPARE(widget.sizeHint(), QSize(360, 300));
    QCOMPARE(widget.minimumSizeHint(), QSize(220, 150));

    // Initial state
    QVERIFY(widget.getBlocks().isEmpty());
    QVERIFY(widget.blocks().isEmpty());
    QVERIFY(!widget.isColorBlindMode());
    QCOMPARE(widget.colorBlindMode(), ColorBlindMode::None);
    QCOMPARE(widget.selectedIndex(), -1);
    QCOMPARE(widget.hoveredIndex(), -1);
    QVERIFY(widget.blockRects().isEmpty());

    // Refresh and setSelectedRegister on empty widget
    widget.refresh();
    widget.setSelectedRegister(0);
    QCOMPARE(widget.selectedIndex(), -1);

    // Clear empty widget
    widget.clear();
    QCOMPARE(widget.sizeHint(), QSize(360, 100));

    // Paint empty widget
    QImage image(360, 100, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    widget.render(&painter);
    QVERIFY(!image.isNull());
}

void TestBlockMemoryMapWidget::testColorBlindModeTransitions()
{
    BlockMemoryMapWidget widget;

    // Test with AppSettings default (None) -> enabled sets Universal
    AppSettings::instance().setColorBlindType(ColorBlindMode::None);
    widget.setColorBlindMode(true);
    QVERIFY(widget.isColorBlindMode());
    QCOMPARE(widget.colorBlindMode(), ColorBlindMode::Universal);

    widget.setColorBlindMode(false);
    QVERIFY(!widget.isColorBlindMode());
    QCOMPARE(widget.colorBlindMode(), ColorBlindMode::None);

    // Test with AppSettings having Deuteranopia
    AppSettings::instance().setColorBlindType(ColorBlindMode::Deuteranopia);
    widget.setColorBlindMode(true);
    QCOMPARE(widget.colorBlindMode(), ColorBlindMode::Deuteranopia);

    // Direct mode changes
    widget.setColorBlindMode(ColorBlindMode::Protanopia);
    QCOMPARE(widget.colorBlindMode(), ColorBlindMode::Protanopia);

    widget.setColorBlindMode(ColorBlindMode::Tritanopia);
    QCOMPARE(widget.colorBlindMode(), ColorBlindMode::Tritanopia);

    // No-op mode change (same mode branch)
    widget.setColorBlindMode(ColorBlindMode::Tritanopia);
    QCOMPARE(widget.colorBlindMode(), ColorBlindMode::Tritanopia);

    widget.setColorBlindMode(ColorBlindMode::None);
    QCOMPARE(widget.colorBlindMode(), ColorBlindMode::None);

    // Construct widget when AppSettings colorBlindMode is active
    AppSettings::instance().setColorBlindMode(true);
    AppSettings::instance().setColorBlindType(ColorBlindMode::Deuteranopia);
    BlockMemoryMapWidget widgetCvd;
    QCOMPARE(widgetCvd.colorBlindMode(), ColorBlindMode::Deuteranopia);

    AppSettings::instance().setColorBlindMode(false);
    AppSettings::instance().setColorBlindType(ColorBlindMode::None);
}

void TestBlockMemoryMapWidget::testBlockComputationAndGaps()
{
    QVariantMap blkData;
    blkData["Type"] = "blk";
    blkData["Name"] = "PERIPH0";
    blkData["Offset/LSB"] = "0x40000000";
    blkData["Size/Width"] = "4096";
    RegMapTreeItem blkItem(RegMapTreeItem::e_rmmKind::blk, blkData);

    // 1. Non-reg items should be ignored in computeBlocks
    QVariantMap memData;
    memData["Type"] = "mem";
    memData["Name"] = "RAM";
    RegMapTreeItem *memItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::mem, memData, &blkItem);
    blkItem.appendChild(memItem);

    QVariantMap dummyFld;
    dummyFld["Type"] = "fld";
    dummyFld["Name"] = "STRAY_FIELD";
    RegMapTreeItem *fldItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::fld, dummyFld, &blkItem);
    blkItem.appendChild(fldItem);

    // 2. Register 0: Offset 0x00, width 32 (4 bytes), RW, has description
    QVariantMap r0Data;
    r0Data["Type"] = "reg";
    r0Data["Name"] = "CTRL";
    r0Data["Offset/LSB"] = "0x00";
    r0Data["Size/Width"] = "32";
    r0Data["Access Policy"] = "RW";
    r0Data["Description"] = "Main peripheral control register";
    RegMapTreeItem *r0 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, r0Data, &blkItem);
    blkItem.appendChild(r0);

    // 3. Register 1: Offset 0x10 (creates 12-byte gap: 0x04..0x0F), default width (32 bits -> 4 bytes), RO, empty desc
    QVariantMap r1Data;
    r1Data["Type"] = "reg";
    r1Data["Name"] = "STATUS";
    r1Data["Offset/LSB"] = "0x10";
    r1Data["Size/Width"] = ""; // empty width -> defaultByteWidth
    r1Data["Access Policy"] = "RO";
    r1Data["Description"] = ""; // empty desc
    RegMapTreeItem *r1 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, r1Data, &blkItem);
    blkItem.appendChild(r1);

    // 4. Register 2: Offset 0b00100000 (0x20 = 32), creates 12-byte gap (0x14..0x1F), width 64 (8 bytes), WO
    QVariantMap r2Data;
    r2Data["Type"] = "reg";
    r2Data["Name"] = "TRIGGER";
    r2Data["Offset/LSB"] = "0b00100000";
    r2Data["Size/Width"] = "64";
    r2Data["Access Policy"] = "WO";
    r2Data["Description"] = "One-shot trigger command";
    RegMapTreeItem *r2 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, r2Data, &blkItem);
    blkItem.appendChild(r2);

    // 5. Register 3: Offset 40 (0x28, immediately adjacent to r2 which ends at 0x27), custom width 4 (< 8)
    QVariantMap r3Data;
    r3Data["Type"] = "reg";
    r3Data["Name"] = "CONFIG";
    r3Data["Offset/LSB"] = "40"; // decimal 40 = 0x28
    r3Data["Size/Width"] = "4";  // < 8 -> byteWidth = 4
    r3Data["Access Policy"] = "W1C";
    r3Data["Description"] = "Interrupt flags";
    RegMapTreeItem *r3 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, r3Data, &blkItem);
    blkItem.appendChild(r3);

    // 6. Register 4: Same offset 40 (0x28) to test sorting branch a.offset == b.offset
    QVariantMap r4Data;
    r4Data["Type"] = "reg";
    r4Data["Name"] = "CONFIG_ALIAS";
    r4Data["Offset/LSB"] = "40";
    r4Data["Size/Width"] = "4";
    r4Data["Access Policy"] = "RO";
    RegMapTreeItem *r4 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, r4Data, &blkItem);
    blkItem.appendChild(r4);

    // 7. Register 5: Offset 0x50, empty name -> tests fallback REG_%1 name
    QVariantMap r5Data;
    r5Data["Type"] = "reg";
    r5Data["Name"] = "";
    r5Data["Offset/LSB"] = "0x50";
    r5Data["Size/Width"] = "32";
    r5Data["Access Policy"] = "RW";
    RegMapTreeItem *r5 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, r5Data, &blkItem);
    blkItem.appendChild(r5);

    BlockMemoryMapWidget widget;
    widget.setBlock(&blkItem, 32);

    const auto &blocks = widget.blocks();
    QCOMPARE(blocks.size(), 9);

    // Block 0: CTRL
    QVERIFY(!blocks[0].isReserved);
    QCOMPARE(blocks[0].name, QString("CTRL"));
    QCOMPARE(blocks[0].offset, 0x00ULL);
    QCOMPARE(blocks[0].sizeBytes, 4ULL);
    QCOMPARE(blocks[0].endOffset, 0x03ULL);
    QCOMPARE(blocks[0].access, QString("RW"));
    QCOMPARE(blocks[0].childRow, 2);

    // Block 1: Gap
    QVERIFY(blocks[1].isReserved);
    QCOMPARE(blocks[1].name, QString("RESERVED"));
    QCOMPARE(blocks[1].offset, 0x04ULL);
    QCOMPARE(blocks[1].sizeBytes, 12ULL);
    QCOMPARE(blocks[1].endOffset, 0x0FULL);
    QCOMPARE(blocks[1].access, QString("NA"));
    QCOMPARE(blocks[1].childRow, -1);

    // Block 2: STATUS
    QVERIFY(!blocks[2].isReserved);
    QCOMPARE(blocks[2].name, QString("STATUS"));
    QCOMPARE(blocks[2].offset, 0x10ULL);
    QCOMPARE(blocks[2].sizeBytes, 4ULL);
    QCOMPARE(blocks[2].endOffset, 0x13ULL);

    // Block 3: Gap
    QVERIFY(blocks[3].isReserved);
    QCOMPARE(blocks[3].offset, 0x14ULL);
    QCOMPARE(blocks[3].sizeBytes, 12ULL);

    // Block 4: TRIGGER
    QVERIFY(!blocks[4].isReserved);
    QCOMPARE(blocks[4].name, QString("TRIGGER"));
    QCOMPARE(blocks[4].offset, 0x20ULL);
    QCOMPARE(blocks[4].sizeBytes, 8ULL);

    // Block 8: Fallback name REG_50
    QVERIFY(!blocks[8].isReserved);
    QCOMPARE(blocks[8].name, QString("REG_50"));
    QCOMPARE(blocks[8].offset, 0x50ULL);

    QVERIFY(widget.sizeHint().height() >= 200);
}

void TestBlockMemoryMapWidget::testAccessPolicyInferenceFromFields()
{
    QVariantMap blkData;
    blkData["Type"] = "blk";
    blkData["Name"] = "INFER_BLK";
    RegMapTreeItem blkItem(RegMapTreeItem::e_rmmKind::blk, blkData);

    // Register with empty access policy, containing RW field -> infers RW
    QVariantMap r1Data;
    r1Data["Type"] = "reg";
    r1Data["Name"] = "REG_RW";
    r1Data["Offset/LSB"] = "0x0";
    r1Data["Access Policy"] = "";
    RegMapTreeItem *r1 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, r1Data, &blkItem);
    blkItem.appendChild(r1);

    QVariantMap f1Data;
    f1Data["Type"] = "fld";
    f1Data["Name"] = "F_RW";
    f1Data["Access Policy"] = "RW";
    RegMapTreeItem *f1 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::fld, f1Data, r1);
    r1->appendChild(f1);

    // Register with empty access policy, containing only RO fields -> infers RO
    QVariantMap r2Data;
    r2Data["Type"] = "reg";
    r2Data["Name"] = "REG_RO";
    r2Data["Offset/LSB"] = "0x4";
    r2Data["Access Policy"] = "";
    RegMapTreeItem *r2 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, r2Data, &blkItem);
    blkItem.appendChild(r2);

    QVariantMap f2Data;
    f2Data["Type"] = "fld";
    f2Data["Name"] = "F_RO";
    f2Data["Access Policy"] = "RO";
    RegMapTreeItem *f2 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::fld, f2Data, r2);
    r2->appendChild(f2);

    // Register with empty access policy, containing only WO fields -> infers WO
    QVariantMap r3Data;
    r3Data["Type"] = "reg";
    r3Data["Name"] = "REG_WO";
    r3Data["Offset/LSB"] = "0x8";
    r3Data["Access Policy"] = "";
    RegMapTreeItem *r3 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, r3Data, &blkItem);
    blkItem.appendChild(r3);

    QVariantMap f3Data;
    f3Data["Type"] = "fld";
    f3Data["Name"] = "F_WO";
    f3Data["Access Policy"] = "WO";
    RegMapTreeItem *f3 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::fld, f3Data, r3);
    r3->appendChild(f3);

    // Register with empty access policy and empty field policy -> fallback RW
    QVariantMap r4Data;
    r4Data["Type"] = "reg";
    r4Data["Name"] = "REG_DEF";
    r4Data["Offset/LSB"] = "0xC";
    r4Data["Access Policy"] = "";
    RegMapTreeItem *r4 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, r4Data, &blkItem);
    blkItem.appendChild(r4);

    QVariantMap f4Data;
    f4Data["Type"] = "fld";
    f4Data["Name"] = "F_NONE";
    f4Data["Access Policy"] = "";
    RegMapTreeItem *f4 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::fld, f4Data, r4);
    r4->appendChild(f4);

    // Non-fld child inside register -> skipped during field scan
    QVariantMap strayData;
    strayData["Type"] = "mem";
    RegMapTreeItem *stray = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::mem, strayData, r4);
    r4->appendChild(stray);

    BlockMemoryMapWidget widget;
    widget.setBlock(&blkItem, 32);

    const auto &blocks = widget.blocks();
    QCOMPARE(blocks.size(), 4);
    QCOMPARE(blocks[0].access, QString("RW"));
    QCOMPARE(blocks[1].access, QString("RO"));
    QCOMPARE(blocks[2].access, QString("WO"));
    QCOMPARE(blocks[3].access, QString("RW"));
}

void TestBlockMemoryMapWidget::testPaintVariants()
{
    QVariantMap blkData;
    blkData["Type"] = "blk";
    blkData["Name"] = "PAINT_BLK";
    RegMapTreeItem blkItem(RegMapTreeItem::e_rmmKind::blk, blkData);

    // Reg 0: 1 byte width (< 46px block height so sub-row is skipped)
    QVariantMap r0Data;
    r0Data["Type"] = "reg";
    r0Data["Name"] = "TINY";
    r0Data["Offset/LSB"] = "0x0";
    r0Data["Size/Width"] = "1"; // 1 byte
    r0Data["Access Policy"] = "RO";
    r0Data["Description"] = "Tiny byte";
    RegMapTreeItem *r0 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, r0Data, &blkItem);
    blkItem.appendChild(r0);

    // Address gap
    // Reg 1: 4 bytes, with description
    QVariantMap r1Data;
    r1Data["Type"] = "reg";
    r1Data["Name"] = "NORMAL";
    r1Data["Offset/LSB"] = "0x10";
    r1Data["Size/Width"] = "32";
    r1Data["Access Policy"] = "RW";
    r1Data["Description"] = "Normal 32-bit register";
    RegMapTreeItem *r1 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, r1Data, &blkItem);
    blkItem.appendChild(r1);

    // Reg 2: 4 bytes, WITHOUT description
    QVariantMap r2Data;
    r2Data["Type"] = "reg";
    r2Data["Name"] = "NODESC";
    r2Data["Offset/LSB"] = "0x14";
    r2Data["Size/Width"] = "32";
    r2Data["Access Policy"] = "WO";
    r2Data["Description"] = "";
    RegMapTreeItem *r2 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, r2Data, &blkItem);
    blkItem.appendChild(r2);

    BlockMemoryMapWidget widget;
    widget.setBlock(&blkItem, 32);
    widget.resize(400, 500);

    // 1. Normal render
    QImage img1(400, 500, QImage::Format_ARGB32_Premultiplied);
    img1.fill(Qt::white);
    QPainter p1(&img1);
    widget.render(&p1);
    QVERIFY(!img1.isNull());

    // 2. Render with selection
    widget.setSelectedRegister(1); // Normal register (childRow 1)
    QImage img2(400, 500, QImage::Format_ARGB32_Premultiplied);
    img2.fill(Qt::white);
    QPainter p2(&img2);
    widget.render(&p2);
    QVERIFY(!img2.isNull());

    // 3. Render with ColorBlindMode::Universal
    widget.setColorBlindMode(ColorBlindMode::Universal);
    QImage img3(400, 500, QImage::Format_ARGB32_Premultiplied);
    img3.fill(Qt::white);
    QPainter p3(&img3);
    widget.render(&p3);
    QVERIFY(!img3.isNull());
    widget.setColorBlindMode(ColorBlindMode::None);

    // 4. Narrow widget (width < 120 triggers blockWidth clamp to 120)
    widget.resize(100, 500);
    QImage img4(100, 500, QImage::Format_ARGB32_Premultiplied);
    img4.fill(Qt::white);
    QPainter p4(&img4);
    widget.render(&p4);
    QVERIFY(!img4.isNull());
    widget.resize(400, 500);

    // 5. Dark themes
    ThemeManager::instance().setTheme("dracula");
    QImage img5(400, 500, QImage::Format_ARGB32_Premultiplied);
    img5.fill(Qt::black);
    QPainter p5(&img5);
    widget.render(&p5);
    QVERIFY(!img5.isNull());

    ThemeManager::instance().setTheme("monokai");
    QImage img6(400, 500, QImage::Format_ARGB32_Premultiplied);
    QPainter p6(&img6);
    widget.render(&p6);
    QVERIFY(!img6.isNull());

    // 6. Light themes
    ThemeManager::instance().setTheme("solarized8_light");
    QImage img7(400, 500, QImage::Format_ARGB32_Premultiplied);
    img7.fill(Qt::white);
    QPainter p7(&img7);
    widget.render(&p7);
    QVERIFY(!img7.isNull());

    ThemeManager::instance().setTheme("classic");
    QImage img8(400, 500, QImage::Format_ARGB32_Premultiplied);
    QPainter p8(&img8);
    widget.render(&p8);
    QVERIFY(!img8.isNull());

    // 7. High contrast themes
    ThemeManager::instance().setTheme("high_contrast_dark");
    QImage img9(400, 500, QImage::Format_ARGB32_Premultiplied);
    img9.fill(Qt::black);
    QPainter p9(&img9);
    widget.render(&p9);
    QVERIFY(!img9.isNull());

    ThemeManager::instance().setTheme("high_contrast_light");
    QImage img10(400, 500, QImage::Format_ARGB32_Premultiplied);
    img10.fill(Qt::white);
    QPainter p10(&img10);
    widget.render(&p10);
    QVERIFY(!img10.isNull());

    ThemeManager::instance().setTheme("solarized8");
}

void TestBlockMemoryMapWidget::testMouseInteractionAndTooltips()
{
    QVariantMap blkData;
    blkData["Type"] = "blk";
    blkData["Name"] = "INTERACT_BLK";
    RegMapTreeItem blkItem(RegMapTreeItem::e_rmmKind::blk, blkData);

    QVariantMap r0Data;
    r0Data["Type"] = "reg";
    r0Data["Name"] = "REG_A";
    r0Data["Offset/LSB"] = "0x0";
    r0Data["Size/Width"] = "32";
    r0Data["Access Policy"] = "RW";
    r0Data["Description"] = "Description for REG_A";
    RegMapTreeItem *r0 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, r0Data, &blkItem);
    blkItem.appendChild(r0);

    // Gap from 0x4 to 0x1F (28 bytes)
    QVariantMap r1Data;
    r1Data["Type"] = "reg";
    r1Data["Name"] = "REG_B";
    r1Data["Offset/LSB"] = "0x20";
    r1Data["Size/Width"] = "32";
    r1Data["Access Policy"] = "RO";
    r1Data["Description"] = ""; // empty description -> tooltip uses "None"
    RegMapTreeItem *r1 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, r1Data, &blkItem);
    blkItem.appendChild(r1);

    BlockMemoryMapWidget widget;
    widget.resize(400, 500);
    widget.setBlock(&blkItem, 32);

    // Render once to populate m_blockRects
    QImage img(400, 500, QImage::Format_ARGB32_Premultiplied);
    QPainter p(&img);
    widget.render(&p);

    const auto &rects = widget.blockRects();
    QCOMPARE(rects.size(), 3);
    // rect 0: REG_A
    // rect 1: RESERVED GAP
    // rect 2: REG_B

    QSignalSpy clickSpy(&widget, &BlockMemoryMapWidget::registerClicked);

    // 1. Move mouse over REG_A (active register with description)
    QPoint posA = rects[0].center().toPoint();
    QMouseEvent moveA(QEvent::MouseMove, posA, posA, posA, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&widget, &moveA);
    QCOMPARE(widget.cursor().shape(), Qt::PointingHandCursor);
    QCOMPARE(widget.hoveredIndex(), 0);

    // Re-move at same point (early return when idx == m_hoveredIndex)
    QApplication::sendEvent(&widget, &moveA);
    QCOMPARE(widget.hoveredIndex(), 0);

    // 2. Render while hovered over non-reserved block to test isHovered border branch
    widget.render(&p);

    // 3. Click on REG_A -> emits registerClicked
    QMouseEvent pressA(QEvent::MouseButtonPress, posA, posA, posA, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&widget, &pressA);
    QCOMPARE(clickSpy.count(), 1);
    QCOMPARE(clickSpy.takeFirst().at(0).toInt(), 0);
    QCOMPARE(widget.selectedIndex(), 0);

    // 4. Move mouse over Gap (reserved block)
    QPoint posGap = rects[1].center().toPoint();
    QMouseEvent moveGap(QEvent::MouseMove, posGap, posGap, posGap, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&widget, &moveGap);
    QCOMPARE(widget.cursor().shape(), Qt::ArrowCursor);
    QCOMPARE(widget.hoveredIndex(), 1);

    // Render while hovered over reserved block to test reserved hovered border branch
    widget.render(&p);

    // 5. Click on Gap -> does NOT emit registerClicked
    QMouseEvent pressGap(QEvent::MouseButtonPress, posGap, posGap, posGap, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&widget, &pressGap);
    QCOMPARE(clickSpy.count(), 0);

    // 6. Move mouse over REG_B (active register WITHOUT description)
    QPoint posB = rects[2].center().toPoint();
    QMouseEvent moveB(QEvent::MouseMove, posB, posB, posB, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&widget, &moveB);
    QCOMPARE(widget.cursor().shape(), Qt::PointingHandCursor);
    QCOMPARE(widget.hoveredIndex(), 2);

    // 7. Move mouse to empty space outside all blocks (e.g. (10, 2))
    QPoint posOutside(10, 2);
    QMouseEvent moveOut(QEvent::MouseMove, posOutside, posOutside, posOutside, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&widget, &moveOut);
    QCOMPARE(widget.cursor().shape(), Qt::ArrowCursor);
    QCOMPARE(widget.hoveredIndex(), -1);

    // Click on empty space outside
    QMouseEvent pressOut(QEvent::MouseButtonPress, posOutside, posOutside, posOutside, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&widget, &pressOut);
    QCOMPARE(clickSpy.count(), 0);

    // 8. Hover back over block then trigger Leave event
    QApplication::sendEvent(&widget, &moveA);
    QCOMPARE(widget.hoveredIndex(), 0);
    QEvent leaveEvent(QEvent::Leave);
    QApplication::sendEvent(&widget, &leaveEvent);
    QCOMPARE(widget.hoveredIndex(), -1);
    QCOMPARE(widget.cursor().shape(), Qt::ArrowCursor);

    // Second leave event when m_hoveredIndex is already -1 (early exit)
    QApplication::sendEvent(&widget, &leaveEvent);
    QCOMPARE(widget.hoveredIndex(), -1);
}

void TestBlockMemoryMapWidget::testSelectionAndRefresh()
{
    QVariantMap blkData;
    blkData["Type"] = "blk";
    blkData["Name"] = "REFRESH_BLK";
    RegMapTreeItem blkItem(RegMapTreeItem::e_rmmKind::blk, blkData);

    QVariantMap r0Data;
    r0Data["Type"] = "reg";
    r0Data["Name"] = "R0";
    r0Data["Offset/LSB"] = "0x0";
    r0Data["Size/Width"] = "32";
    RegMapTreeItem *r0 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, r0Data, &blkItem);
    blkItem.appendChild(r0);

    BlockMemoryMapWidget widget;
    widget.setBlock(&blkItem, 32);

    // Select valid childRow 0
    widget.setSelectedRegister(0);
    QCOMPARE(widget.selectedIndex(), 0);

    // Refresh preserves selection
    widget.refresh();
    QCOMPARE(widget.selectedIndex(), 0);

    // Select non-existent childRow
    widget.setSelectedRegister(999);
    QCOMPARE(widget.selectedIndex(), -1);

    // Refresh when no selection
    widget.refresh();
    QCOMPARE(widget.selectedIndex(), -1);

    // Clear
    widget.clear();
    QVERIFY(widget.blocks().isEmpty());
    QCOMPARE(widget.selectedIndex(), -1);
}

void TestBlockMemoryMapWidget::testZeroRegWidthAndEdgeCases()
{
    QVariantMap blkData;
    blkData["Type"] = "blk";
    blkData["Name"] = "EDGE_BLK";
    RegMapTreeItem blkItem(RegMapTreeItem::e_rmmKind::blk, blkData);

    // Register with 0 width -> fallback to default
    QVariantMap r0Data;
    r0Data["Type"] = "reg";
    r0Data["Name"] = "REG_ZERO";
    r0Data["Offset/LSB"] = "0x0";
    r0Data["Size/Width"] = "0";
    RegMapTreeItem *r0 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, r0Data, &blkItem);
    blkItem.appendChild(r0);

    // Register with invalid offset string (toULongLong fails -> 0)
    QVariantMap r1Data;
    r1Data["Type"] = "reg";
    r1Data["Name"] = "REG_INV_HEX";
    r1Data["Offset/LSB"] = "0xZZZ";
    r1Data["Size/Width"] = "32";
    RegMapTreeItem *r1 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, r1Data, &blkItem);
    blkItem.appendChild(r1);

    // Register with invalid binary string (toULongLong fails -> 0)
    QVariantMap r2Data;
    r2Data["Type"] = "reg";
    r2Data["Name"] = "REG_INV_BIN";
    r2Data["Offset/LSB"] = "0b999";
    r2Data["Size/Width"] = "32";
    RegMapTreeItem *r2 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, r2Data, &blkItem);
    blkItem.appendChild(r2);

    // Register with invalid decimal string
    QVariantMap r3Data;
    r3Data["Type"] = "reg";
    r3Data["Name"] = "REG_INV_DEC";
    r3Data["Offset/LSB"] = "not_a_number";
    r3Data["Size/Width"] = "32";
    RegMapTreeItem *r3 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, r3Data, &blkItem);
    blkItem.appendChild(r3);

    BlockMemoryMapWidget widget;
    // Pass 0 as regWidthBits -> should default to 32 bits (4 bytes)
    widget.setBlock(&blkItem, 0);
    QCOMPARE(widget.blocks().size(), 4);
    for (const auto &b : widget.blocks()) {
        QCOMPARE(b.sizeBytes, 4ULL);
    }

    // Set null block
    widget.setBlock(nullptr, 32);
    QVERIFY(widget.blocks().isEmpty());
}

QTEST_MAIN(TestBlockMemoryMapWidget)
#include "test_BlockMemoryMapWidget.moc"
