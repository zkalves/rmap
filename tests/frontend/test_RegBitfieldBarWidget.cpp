/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include <QtTest>
#include <QPainter>
#include <QSignalSpy>
#include "RegBitfieldBarWidget.hpp"
#include "RegMapTreeItem.hpp"
#include "ThemeManager.hpp"

class TestRegBitfieldBarWidget : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testEmptyWidgetAndSizing();
    void testSliceComputation32Bit();
    void testSliceComputation64BitAndTrailingGap();
    void testColorBlindModeTransitions();
    void testPaintSlicesVariants();
    void testMouseInteractionAndTooltips();
    void testSelectionAndRefresh();
};

void TestRegBitfieldBarWidget::initTestCase()
{
}

void TestRegBitfieldBarWidget::cleanupTestCase()
{
    ThemeManager::instance().setTheme("solarized8");
    ThemeManager::instance().setColorBlindMode(ColorBlindMode::None);
}

void TestRegBitfieldBarWidget::testEmptyWidgetAndSizing()
{
    RegBitfieldBarWidget widget;

    // Default sizing
    QCOMPARE(widget.sizeHint(), QSize(600, 80));
    QCOMPARE(widget.minimumSizeHint(), QSize(300, 68));

    // Initial state
    QVERIFY(widget.getSlices().isEmpty());
    QVERIFY(!widget.isColorBlindMode());
    QCOMPARE(widget.colorBlindMode(), ColorBlindMode::None);

    // Refresh, clear, setSelectedField when empty should not crash
    widget.refresh();
    widget.setSelectedField(0);
    widget.clear();
    QVERIFY(widget.slices().isEmpty());

    // Paint empty widget
    QImage image(600, 80, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    widget.render(&painter);
    QVERIFY(!image.isNull());
}

void TestRegBitfieldBarWidget::testSliceComputation32Bit()
{
    QVariantMap regData;
    regData["Type"] = "reg";
    regData["Name"] = "CTRL";
    regData["Offset/LSB"] = "0x0";
    regData["Size/Width"] = "32";
    RegMapTreeItem regItem(RegMapTreeItem::e_rmmKind::reg, regData);

    // Field 1: LSB 0, width 4, name "MODE", SW "RW", HW "RO", reset 0x5
    QVariantMap f1Data;
    f1Data["Type"] = "fld";
    f1Data["Name"] = "MODE";
    f1Data["Offset/LSB"] = "0";
    f1Data["Size/Width"] = "4";
    f1Data["Access Policy"] = "RW";
    f1Data["HW Access"] = "RO";
    f1Data["Reset Value"] = "0x5";
    f1Data["Description"] = "Mode field description";
    RegMapTreeItem *f1 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::fld, f1Data, &regItem);
    regItem.appendChild(f1);

    // Field 2: LSB 8, width 2, name "STATUS", SW "RO", HW "WO", reset 0x1
    QVariantMap f2Data;
    f2Data["Type"] = "fld";
    f2Data["Name"] = "STATUS";
    f2Data["Offset/LSB"] = "8";
    f2Data["Size/Width"] = "2";
    f2Data["Access Policy"] = "RO";
    f2Data["HW Access"] = "WO";
    f2Data["Reset Value"] = "0x1";
    f2Data["Description"] = "";
    RegMapTreeItem *f2 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::fld, f2Data, &regItem);
    regItem.appendChild(f2);

    RegBitfieldBarWidget widget;
    widget.setRegister(&regItem, 32);

    const auto &slices = widget.slices();
    QCOMPARE(slices.size(), 4);

    // Slice 0: Reserved bits [31:10] (width 22)
    QVERIFY(slices[0].isReserved);
    QCOMPARE(slices[0].msb, 31ULL);
    QCOMPARE(slices[0].lsb, 10ULL);
    QCOMPARE(slices[0].width, 22ULL);

    // Slice 1: STATUS bits [9:8] (width 2)
    QVERIFY(!slices[1].isReserved);
    QCOMPARE(slices[1].name, QString("STATUS"));
    QCOMPARE(slices[1].msb, 9ULL);
    QCOMPARE(slices[1].lsb, 8ULL);
    QCOMPARE(slices[1].width, 2ULL);
    QCOMPARE(slices[1].access, QString("RO"));
    QCOMPARE(slices[1].hwAccess, QString("WO"));

    // Slice 2: Reserved bits [7:4] (width 4)
    QVERIFY(slices[2].isReserved);
    QCOMPARE(slices[2].msb, 7ULL);
    QCOMPARE(slices[2].lsb, 4ULL);
    QCOMPARE(slices[2].width, 4ULL);

    // Slice 3: MODE bits [3:0] (width 4)
    QVERIFY(!slices[3].isReserved);
    QCOMPARE(slices[3].name, QString("MODE"));
    QCOMPARE(slices[3].msb, 3ULL);
    QCOMPARE(slices[3].lsb, 0ULL);
    QCOMPARE(slices[3].width, 4ULL);
    QCOMPARE(slices[3].access, QString("RW"));
    QCOMPARE(slices[3].hwAccess, QString("RO"));
    QCOMPARE(slices[3].resetVal, 5ULL);
}

void TestRegBitfieldBarWidget::testSliceComputation64BitAndTrailingGap()
{
    QVariantMap regData;
    regData["Type"] = "reg";
    regData["Name"] = "WIDE_REG";
    regData["Offset/LSB"] = "0x0";
    regData["Size/Width"] = "64";
    RegMapTreeItem regItem(RegMapTreeItem::e_rmmKind::reg, regData);

    // Field with empty name and defaults, LSB 16, width 16 (bits 31:16)
    QVariantMap fData;
    fData["Type"] = "fld";
    fData["Name"] = ""; // empty name -> fallback UNNAMED
    fData["Offset/LSB"] = "16";
    fData["Size/Width"] = "16";
    fData["Access Policy"] = ""; // empty -> fallback RW
    fData["HW Access"] = "";     // empty -> fallback RO
    RegMapTreeItem *fld = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::fld, fData, &regItem);
    regItem.appendChild(fld);

    RegBitfieldBarWidget widget;
    widget.setRegister(&regItem, 64);

    const auto &slices = widget.slices();
    QCOMPARE(slices.size(), 3);

    // Leading gap: [63:32] (width 32)
    QVERIFY(slices[0].isReserved);
    QCOMPARE(slices[0].msb, 63ULL);
    QCOMPARE(slices[0].lsb, 32ULL);

    // Field: [31:16] (width 16)
    QVERIFY(!slices[1].isReserved);
    QCOMPARE(slices[1].name, QString("UNNAMED"));
    QCOMPARE(slices[1].access, QString("RW"));
    QCOMPARE(slices[1].hwAccess, QString("RO"));

    // Trailing gap: [15:0] (width 16)
    QVERIFY(slices[2].isReserved);
    QCOMPARE(slices[2].msb, 15ULL);
    QCOMPARE(slices[2].lsb, 0ULL);
}

void TestRegBitfieldBarWidget::testColorBlindModeTransitions()
{
    RegBitfieldBarWidget widget;

    widget.setColorBlindMode(true);
    QVERIFY(widget.isColorBlindMode());

    widget.setColorBlindMode(false);
    QVERIFY(!widget.isColorBlindMode());

    widget.setColorBlindMode(ColorBlindMode::Universal);
    QCOMPARE(widget.colorBlindMode(), ColorBlindMode::Universal);

    widget.setColorBlindMode(ColorBlindMode::Deuteranopia);
    QCOMPARE(widget.colorBlindMode(), ColorBlindMode::Deuteranopia);

    widget.setColorBlindMode(ColorBlindMode::Protanopia);
    QCOMPARE(widget.colorBlindMode(), ColorBlindMode::Protanopia);

    widget.setColorBlindMode(ColorBlindMode::Tritanopia);
    QCOMPARE(widget.colorBlindMode(), ColorBlindMode::Tritanopia);

    widget.setColorBlindMode(ColorBlindMode::None);
    QCOMPARE(widget.colorBlindMode(), ColorBlindMode::None);
}

void TestRegBitfieldBarWidget::testPaintSlicesVariants()
{
    QVariantMap regData;
    regData["Type"] = "reg";
    regData["Name"] = "STATUS_REG";
    RegMapTreeItem regItem(RegMapTreeItem::e_rmmKind::reg, regData);

    QVariantMap f1Data;
    f1Data["Type"] = "fld";
    f1Data["Name"] = "RX_FULL";
    f1Data["Offset/LSB"] = "0";
    f1Data["Size/Width"] = "1";
    f1Data["Access Policy"] = "RO";
    f1Data["HW Access"] = "WO";
    RegMapTreeItem *f1 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::fld, f1Data, &regItem);
    regItem.appendChild(f1);

    QVariantMap f2Data;
    f2Data["Type"] = "fld";
    f2Data["Name"] = "FIFO_DATA";
    f2Data["Offset/LSB"] = "8";
    f2Data["Size/Width"] = "8";
    f2Data["Access Policy"] = "RW";
    f2Data["HW Access"] = "RO";
    RegMapTreeItem *f2 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::fld, f2Data, &regItem);
    regItem.appendChild(f2);

    RegBitfieldBarWidget widget;
    widget.setRegister(&regItem, 32);
    widget.resize(800, 80);

    // 1. Paint normal
    QImage img1(800, 80, QImage::Format_ARGB32_Premultiplied);
    img1.fill(Qt::white);
    QPainter p1(&img1);
    widget.render(&p1);
    QVERIFY(!img1.isNull());

    // 2. Paint with selection
    widget.setSelectedField(0); // RX_FULL
    QImage img2(800, 80, QImage::Format_ARGB32_Premultiplied);
    img2.fill(Qt::white);
    QPainter p2(&img2);
    widget.render(&p2);
    QVERIFY(!img2.isNull());

    // 3. Paint in ColorBlindMode
    widget.setColorBlindMode(ColorBlindMode::Universal);
    QImage img3(800, 80, QImage::Format_ARGB32_Premultiplied);
    img3.fill(Qt::white);
    QPainter p3(&img3);
    widget.render(&p3);
    QVERIFY(!img3.isNull());
    widget.setColorBlindMode(ColorBlindMode::None);

    // 4. Paint wide widget (wide reserved slice > 60 px)
    widget.resize(1600, 80);
    QImage img4(1600, 80, QImage::Format_ARGB32_Premultiplied);
    img4.fill(Qt::white);
    QPainter p4(&img4);
    widget.render(&p4);
    QVERIFY(!img4.isNull());

    // 5. Paint narrow widget (slice width < 40 px)
    widget.resize(300, 70);
    QImage img5(300, 70, QImage::Format_ARGB32_Premultiplied);
    img5.fill(Qt::white);
    QPainter p5(&img5);
    widget.render(&p5);
    QVERIFY(!img5.isNull());

    // 6. Paint with dark theme
    ThemeManager::instance().setTheme("dracula");
    QImage img6(800, 80, QImage::Format_ARGB32_Premultiplied);
    img6.fill(Qt::black);
    QPainter p6(&img6);
    widget.render(&p6);
    QVERIFY(!img6.isNull());
    ThemeManager::instance().setTheme("solarized8");
}

void TestRegBitfieldBarWidget::testMouseInteractionAndTooltips()
{
    QVariantMap regData;
    regData["Type"] = "reg";
    regData["Name"] = "INTERACT_REG";
    RegMapTreeItem regItem(RegMapTreeItem::e_rmmKind::reg, regData);

    QVariantMap fData;
    fData["Type"] = "fld";
    fData["Name"] = "CMD";
    fData["Offset/LSB"] = "0";
    fData["Size/Width"] = "4";
    fData["Access Policy"] = "WO";
    fData["HW Access"] = "RO";
    fData["Reset Value"] = "0x0";
    fData["Description"] = "Command trigger field";
    RegMapTreeItem *fld = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::fld, fData, &regItem);
    regItem.appendChild(fld);

    RegBitfieldBarWidget widget;
    widget.resize(800, 80);
    widget.setRegister(&regItem, 32);

    // Render once to populate slice bounding rects
    QImage img(800, 80, QImage::Format_ARGB32_Premultiplied);
    QPainter p(&img);
    widget.render(&p);

    QSignalSpy clickSpy(&widget, &RegBitfieldBarWidget::fieldClicked);

    // Mouse move over field slice (CMD is bits [3:0], far right of the bar)
    QPoint fieldPos(750, 40);
    QMouseEvent moveEvent(QEvent::MouseMove, fieldPos, fieldPos, fieldPos, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&widget, &moveEvent);

    // Mouse press on field slice
    QMouseEvent pressEvent(QEvent::MouseButtonPress, fieldPos, fieldPos, fieldPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&widget, &pressEvent);

    QCOMPARE(clickSpy.count(), 1);
    QCOMPARE(clickSpy.takeFirst().at(0).toInt(), 0);

    // Mouse move over reserved slice (far left, bits [31:4])
    QPoint rsvdPos(100, 40);
    QMouseEvent moveRsvd(QEvent::MouseMove, rsvdPos, rsvdPos, rsvdPos, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&widget, &moveRsvd);

    // Mouse press on reserved slice should NOT emit fieldClicked
    QMouseEvent pressRsvd(QEvent::MouseButtonPress, rsvdPos, rsvdPos, rsvdPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&widget, &pressRsvd);
    QCOMPARE(clickSpy.count(), 0);

    // Mouse move outside slices (e.g. y = -10)
    QPoint outsidePos(100, -10);
    QMouseEvent moveOutside(QEvent::MouseMove, outsidePos, outsidePos, outsidePos, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&widget, &moveOutside);

    // Leave event
    QEvent leaveEvent(QEvent::Leave);
    QApplication::sendEvent(&widget, &leaveEvent);
}

void TestRegBitfieldBarWidget::testSelectionAndRefresh()
{
    QVariantMap regData;
    regData["Type"] = "reg";
    regData["Name"] = "REFRESH_REG";
    RegMapTreeItem regItem(RegMapTreeItem::e_rmmKind::reg, regData);

    QVariantMap f1Data;
    f1Data["Type"] = "fld";
    f1Data["Name"] = "FLD_A";
    f1Data["Offset/LSB"] = "0";
    f1Data["Size/Width"] = "4";
    RegMapTreeItem *f1 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::fld, f1Data, &regItem);
    regItem.appendChild(f1);

    RegBitfieldBarWidget widget;
    widget.setRegister(&regItem, 32);

    widget.setSelectedField(0); // Select FLD_A (childRow 0)
    widget.refresh();           // Preserves selection across refresh

    widget.setSelectedField(99); // Non-existent childRow
    widget.clear();              // Clears slices
    QVERIFY(widget.slices().isEmpty());

    // Access color helper
    QVERIFY(widget.getAccessColor("RW", true).isValid());
    QVERIFY(widget.getAccessColor("RW", false).isValid());

    // Trigger themeChanged signal lambda
    ThemeManager::instance().setTheme("solarized8");

    // Test zero regWidth defaults to 32
    widget.setRegister(&regItem, 0);
    QCOMPARE(widget.slices().size(), 2);

    // Test binary and hex parseNum, zero width field fallback to 1, no description
    QVariantMap regData2;
    regData2["Type"] = "reg";
    regData2["Name"] = "NUM_PARSE_REG";
    RegMapTreeItem regItem2(RegMapTreeItem::e_rmmKind::reg, regData2);

    QVariantMap fBin;
    fBin["Type"] = "fld";
    fBin["Name"] = "BIN_FLD";
    fBin["Offset/LSB"] = "0b0010"; // binary 2
    fBin["Size/Width"] = "0";      // 0 width -> defaults to 1
    fBin["Reset Value"] = "0b101";
    // no description
    RegMapTreeItem *fBinItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::fld, fBin, &regItem2);
    regItem2.appendChild(fBinItem);

    widget.setRegister(&regItem2, 32);
    widget.resize(100, 80); // Very narrow widget to exercise sliceW < 14
    QImage tinyImg(100, 80, QImage::Format_ARGB32_Premultiplied);
    QPainter tinyPainter(&tinyImg);
    widget.render(&tinyPainter);

    // Hover over slice without description
    QPoint binPos(90, 40);
    QMouseEvent moveNoDesc(QEvent::MouseMove, binPos, binPos, binPos, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&widget, &moveNoDesc);
}

QTEST_MAIN(TestRegBitfieldBarWidget)
#include "test_RegBitfieldBarWidget.moc"
