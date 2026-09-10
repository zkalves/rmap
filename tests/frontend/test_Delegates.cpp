/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include <QtTest>
#include <QLineEdit>
#include <QComboBox>
#include <QSortFilterProxyModel>
#include "RegMapDelegate.hpp"
#include "RegMapTreeModel.hpp"
#include "ThemeManager.hpp"

class TestDelegates : public QObject
{
    Q_OBJECT

private slots:
    void testHexDecBinDelegateValidation();
    void testHexDecBinDelegateEditorData();
    void testAccessPolicyDelegate();
    void testAllAccessPoliciesInDelegate();
    void testSingleClickAccessPolicyCycling();
    void testSingleClickHwAccessCycling();
    void testSingleClickBoolToggle();
    void testDelegateBadgePainting();
    void testHwAccessDelegate();
    void testBoolDelegate();
    void testStrDelegate();
    void testRegIntDelegate();
    void testRegMapDelegatePaintValidationAndProxy();
    void testDelegateColorBlindModeAndBadgeEdges();
    void testDelegateEventsEdgeCases();
};

void TestDelegates::testHexDecBinDelegateValidation()
{
    QWidget parent;
    RegHexDecBinDelegate delegate(&parent);
    QStyleOptionViewItem option;
    QModelIndex index;

    QWidget *editor = delegate.createEditor(&parent, option, index);
    QVERIFY(editor != nullptr);
    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor);
    QVERIFY(lineEdit != nullptr);

    const QValidator *validator = lineEdit->validator();
    QVERIFY(validator != nullptr);

    int pos = 0;
    QString hexVal = "0x1A2F";
    QCOMPARE(validator->validate(hexVal, pos), QValidator::Acceptable);

    QString decVal = "12345";
    pos = 0;
    QCOMPARE(validator->validate(decVal, pos), QValidator::Acceptable);

    QString binVal = "0b10101";
    pos = 0;
    QCOMPARE(validator->validate(binVal, pos), QValidator::Acceptable);

    QString invalidVal = "0xZZ!!";
    pos = 0;
    QCOMPARE(validator->validate(invalidVal, pos), QValidator::Invalid);

    delete editor;
}

void TestDelegates::testAccessPolicyDelegate()
{
    QWidget parent;
    RegAccessPolicyDelegate delegate(&parent);
    QStyleOptionViewItem option;

    RegMapTreeModel model;
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
    QModelIndex blkIndex = model.index(0, 0, QModelIndex());
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, blkIndex);
    QModelIndex regIndex = model.index(0, 0, blkIndex);
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::fld, regIndex);
    QModelIndex fldAccessIndex = model.index(0, 4, regIndex); // Column 4: Access Policy

    model.setData(fldAccessIndex, "RW", Qt::EditRole);

    QWidget *editor = delegate.createEditor(&parent, option, fldAccessIndex);
    QVERIFY(editor != nullptr);
    QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
    QVERIFY(comboBox != nullptr);

    delegate.setEditorData(editor, fldAccessIndex);
    QCOMPARE(comboBox->currentText(), QString("RW"));

    // Change value in combo box and commit to model
    int idxW1C = comboBox->findText("W1C");
    QVERIFY(idxW1C >= 0);
    comboBox->setCurrentIndex(idxW1C);

    delegate.setModelData(editor, &model, fldAccessIndex);
    QCOMPARE(model.data(fldAccessIndex, Qt::DisplayRole).toString(), QString("W1C"));

    delete editor;
}

void TestDelegates::testHwAccessDelegate()
{
    QWidget parent;
    RegHwAccessDelegate delegate(&parent);
    QStyleOptionViewItem option;

    RegMapTreeModel model;
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
    QModelIndex blkIndex = model.index(0, 0, QModelIndex());
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, blkIndex);
    QModelIndex regIndex = model.index(0, 0, blkIndex);
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::fld, regIndex);
    QModelIndex fldHwIndex = model.index(0, 5, regIndex); // Column 5: HW Access

    model.setData(fldHwIndex, "RO", Qt::EditRole);

    QWidget *editor = delegate.createEditor(&parent, option, fldHwIndex);
    QVERIFY(editor != nullptr);
    QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
    QVERIFY(comboBox != nullptr);

    delegate.setEditorData(editor, fldHwIndex);
    QCOMPARE(comboBox->currentText(), QString("RO"));

    int idxRW = comboBox->findText("RW");
    QVERIFY(idxRW >= 0);
    comboBox->setCurrentIndex(idxRW);

    delegate.setModelData(editor, &model, fldHwIndex);
    QCOMPARE(model.data(fldHwIndex, Qt::DisplayRole).toString(), QString("RW"));

    delete editor;
}

void TestDelegates::testBoolDelegate()
{
    QWidget parent;
    RegBoolDelegate delegate(&parent);
    QStyleOptionViewItem option;

    RegMapTreeModel model;
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
    QModelIndex blkIndex = model.index(0, 0, QModelIndex());
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, blkIndex);
    QModelIndex regIndex = model.index(0, 0, blkIndex);
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::fld, regIndex);
    QModelIndex fldRandIndex = model.index(0, 7, regIndex); // Column 7: Is Rand

    model.setData(fldRandIndex, "true", Qt::EditRole);

    QWidget *editor = delegate.createEditor(&parent, option, fldRandIndex);
    QVERIFY(editor != nullptr);
    QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
    QVERIFY(comboBox != nullptr);

    delegate.setEditorData(editor, fldRandIndex);
    QCOMPARE(comboBox->currentText(), QString("true"));

    comboBox->setCurrentIndex(comboBox->findText("false"));
    delegate.setModelData(editor, &model, fldRandIndex);
    QCOMPARE(model.data(fldRandIndex, Qt::DisplayRole).toString(), QString("false"));

    delete editor;
}

void TestDelegates::testStrDelegate()
{
    QWidget parent;
    RegStrDelegate delegate(&parent);
    QStyleOptionViewItem option;
    QModelIndex index;

    QWidget *editor = delegate.createEditor(&parent, option, index);
    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor);
    QVERIFY(lineEdit != nullptr);

    const QValidator *validator = lineEdit->validator();
    int pos = 0;
    QString validIdent = "CTRL_REG_1";
    QCOMPARE(validator->validate(validIdent, pos), QValidator::Acceptable);

    QString invalidIdent = "123_INVALID"; // Cannot start with number
    pos = 0;
    QCOMPARE(validator->validate(invalidIdent, pos), QValidator::Invalid);

    delete editor;
}

void TestDelegates::testHexDecBinDelegateEditorData()
{
    QWidget parent;
    RegHexDecBinDelegate delegate(&parent);
    QStyleOptionViewItem option;

    RegMapTreeModel model;
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
    QModelIndex blkIndex = model.index(0, 0, QModelIndex());
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, blkIndex);
    QModelIndex regIndex = model.index(0, 0, blkIndex);
    QModelIndex offsetIndex = model.index(0, 1, blkIndex); // Column 1: Offset/LSB

    model.setData(offsetIndex, "0x10", Qt::EditRole);

    QWidget *editor = delegate.createEditor(&parent, option, offsetIndex);
    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor);
    QVERIFY(lineEdit != nullptr);

    delegate.setEditorData(editor, offsetIndex);
    QCOMPARE(lineEdit->text(), QString("0x0010"));

    lineEdit->setText("0x20");
    delegate.setModelData(editor, &model, offsetIndex);
    QCOMPARE(model.data(offsetIndex, Qt::DisplayRole).toString(), QString("0x0020"));

    delete editor;
}

void TestDelegates::testAllAccessPoliciesInDelegate()
{
    QWidget parent;
    RegAccessPolicyDelegate delegate(&parent);
    QStyleOptionViewItem option;
    QModelIndex dummyIndex;

    QWidget *editor = delegate.createEditor(&parent, option, dummyIndex);
    QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
    QVERIFY(comboBox != nullptr);

    QStringList expectedPolicies = {"RW", "RO", "WO", "W1C", "W0C", "RC", "RS", "W1S", "W0S"};
    for (const QString &policy : expectedPolicies) {
        QVERIFY2(comboBox->findText(policy) >= 0, qPrintable(QString("Policy %1 missing in combo box").arg(policy)));
    }

    delete editor;
}

void TestDelegates::testSingleClickAccessPolicyCycling()
{
    QWidget parent;
    RegAccessPolicyDelegate delegate(&parent);
    QStyleOptionViewItem option;

    RegMapTreeModel model;
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
    QModelIndex blkIndex = model.index(0, 0, QModelIndex());
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, blkIndex);
    QModelIndex regIndex = model.index(0, 0, blkIndex);
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::fld, regIndex);
    QModelIndex fldAccessIndex = model.index(0, 4, regIndex);

    // Initial state: RW
    model.setData(fldAccessIndex, "RW", Qt::EditRole);
    QCOMPARE(model.data(fldAccessIndex, Qt::DisplayRole).toString(), QString("RW"));

    // Simulate Left Mouse Button Release: RW -> RO
    QMouseEvent click1(QEvent::MouseButtonRelease, QPointF(10, 10), QPointF(10, 10), QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QVERIFY(delegate.editorEvent(&click1, &model, option, fldAccessIndex));
    QCOMPARE(model.data(fldAccessIndex, Qt::DisplayRole).toString(), QString("RO"));

    // Simulate Click 2: RO -> WO
    QMouseEvent click2(QEvent::MouseButtonRelease, QPointF(10, 10), QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QVERIFY(delegate.editorEvent(&click2, &model, option, fldAccessIndex));
    QCOMPARE(model.data(fldAccessIndex, Qt::DisplayRole).toString(), QString("WO"));

    // Simulate Click 3: WO -> W1C
    QMouseEvent click3(QEvent::MouseButtonRelease, QPointF(10, 10), QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QVERIFY(delegate.editorEvent(&click3, &model, option, fldAccessIndex));
    QCOMPARE(model.data(fldAccessIndex, Qt::DisplayRole).toString(), QString("W1C"));

    // Simulate Click 4: W1C -> RW (Full cycle completed)
    QMouseEvent click4(QEvent::MouseButtonRelease, QPointF(10, 10), QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QVERIFY(delegate.editorEvent(&click4, &model, option, fldAccessIndex));
    QCOMPARE(model.data(fldAccessIndex, Qt::DisplayRole).toString(), QString("RW"));
}

void TestDelegates::testSingleClickHwAccessCycling()
{
    QWidget parent;
    RegHwAccessDelegate delegate(&parent);
    QStyleOptionViewItem option;

    RegMapTreeModel model;
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
    QModelIndex blkIndex = model.index(0, 0, QModelIndex());
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, blkIndex);
    QModelIndex regIndex = model.index(0, 0, blkIndex);
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::fld, regIndex);
    QModelIndex fldHwIndex = model.index(0, 5, regIndex);

    // Initial state: RO
    model.setData(fldHwIndex, "RO", Qt::EditRole);
    QCOMPARE(model.data(fldHwIndex, Qt::DisplayRole).toString(), QString("RO"));

    // RO -> RW
    QMouseEvent click1(QEvent::MouseButtonRelease, QPointF(10, 10), QPointF(10, 10), QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QVERIFY(delegate.editorEvent(&click1, &model, option, fldHwIndex));
    QCOMPARE(model.data(fldHwIndex, Qt::DisplayRole).toString(), QString("RW"));

    // RW -> WO
    QMouseEvent click2(QEvent::MouseButtonRelease, QPointF(10, 10), QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QVERIFY(delegate.editorEvent(&click2, &model, option, fldHwIndex));
    QCOMPARE(model.data(fldHwIndex, Qt::DisplayRole).toString(), QString("WO"));

    // WO -> NA
    QMouseEvent click3(QEvent::MouseButtonRelease, QPointF(10, 10), QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QVERIFY(delegate.editorEvent(&click3, &model, option, fldHwIndex));
    QCOMPARE(model.data(fldHwIndex, Qt::DisplayRole).toString(), QString("NA"));

    // NA -> RO
    QMouseEvent click4(QEvent::MouseButtonRelease, QPointF(10, 10), QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QVERIFY(delegate.editorEvent(&click4, &model, option, fldHwIndex));
    QCOMPARE(model.data(fldHwIndex, Qt::DisplayRole).toString(), QString("RO"));
}

void TestDelegates::testSingleClickBoolToggle()
{
    QWidget parent;
    RegBoolDelegate delegate(&parent);
    QStyleOptionViewItem option;

    RegMapTreeModel model;
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
    QModelIndex blkIndex = model.index(0, 0, QModelIndex());
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, blkIndex);
    QModelIndex regIndex = model.index(0, 0, blkIndex);
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::fld, regIndex);
    QModelIndex fldVolatileIndex = model.index(0, 8, regIndex); // Column 8: Volatile

    // Initial state: false
    model.setData(fldVolatileIndex, "false", Qt::EditRole);
    QCOMPARE(model.data(fldVolatileIndex, Qt::DisplayRole).toString(), QString("false"));

    // Toggle to true
    QMouseEvent click1(QEvent::MouseButtonRelease, QPointF(10, 10), QPointF(10, 10), QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QVERIFY(delegate.editorEvent(&click1, &model, option, fldVolatileIndex));
    QCOMPARE(model.data(fldVolatileIndex, Qt::DisplayRole).toString(), QString("true"));

    // Toggle back to false
    QMouseEvent click2(QEvent::MouseButtonRelease, QPointF(10, 10), QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QVERIFY(delegate.editorEvent(&click2, &model, option, fldVolatileIndex));
    QCOMPARE(model.data(fldVolatileIndex, Qt::DisplayRole).toString(), QString("false"));
}

void TestDelegates::testDelegateBadgePainting()
{
    QImage image(120, 30, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);
    QPainter painter(&image);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 120, 30);
    option.state = QStyle::State_Enabled | QStyle::State_Selected;

    RegMapTreeModel model;
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
    QModelIndex blkIndex = model.index(0, 0, QModelIndex());
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, blkIndex);
    QModelIndex regIndex = model.index(0, 0, blkIndex);
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::fld, regIndex);
    QModelIndex fldAccessIndex = model.index(0, 4, regIndex);
    model.setData(fldAccessIndex, "RW", Qt::EditRole);

    RegAccessPolicyDelegate swDelegate;
    swDelegate.paint(&painter, option, fldAccessIndex);

    QModelIndex fldHwIndex = model.index(0, 5, regIndex);
    model.setData(fldHwIndex, "RO", Qt::EditRole);
    RegHwAccessDelegate hwDelegate;
    hwDelegate.paint(&painter, option, fldHwIndex);

    QVERIFY(!image.isNull());
}

void TestDelegates::testRegIntDelegate()
{
    QWidget parent;
    RegIntDelegate delegate(&parent);
    QStyleOptionViewItem option;
    QModelIndex index;

    QWidget *editor = delegate.createEditor(&parent, option, index);
    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor);
    QVERIFY(lineEdit != nullptr);

    const QValidator *validator = lineEdit->validator();
    int pos = 0;
    QString validNum = "987654";
    QCOMPARE(validator->validate(validNum, pos), QValidator::Acceptable);

    QString invalidNum = "12abc";
    pos = 0;
    QCOMPARE(validator->validate(invalidNum, pos), QValidator::Invalid);

    delete editor;
}

void TestDelegates::testRegMapDelegatePaintValidationAndProxy()
{
    QImage image(200, 30, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);
    QPainter painter(&image);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);

    RegMapTreeModel model;
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
    QModelIndex blkIndex = model.index(0, 0, QModelIndex());
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, blkIndex);
    QModelIndex regIndex = model.index(0, 0, blkIndex);
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::fld, regIndex);

    QModelIndex fldOffset = model.index(0, 1, regIndex);
    model.setData(fldOffset, "0x10", Qt::EditRole);

    RegHexDecBinDelegate hexDelegate;
    RegMapDelegate baseDelegate;
    baseDelegate.paint(&painter, option, fldOffset);

    // 1. Paint valid regex value
    hexDelegate.paint(&painter, option, fldOffset);

    // 2. Paint value "NA" (regex check bypassed)
    model.setData(fldOffset, "NA", Qt::EditRole);
    hexDelegate.paint(&painter, option, fldOffset);

    // 3. Paint regex-invalid value
    model.setData(fldOffset, "INVALID_HEX_XYZ", Qt::EditRole);
    hexDelegate.paint(&painter, option, fldOffset);

    // 4. Paint via QSortFilterProxyModel unwrapping
    QSortFilterProxyModel proxy;
    proxy.setSourceModel(&model);
    QModelIndex proxyBlk = proxy.index(0, 0, QModelIndex());
    QModelIndex proxyReg = proxy.index(0, 0, proxyBlk);
    QModelIndex proxyFldOffset = proxy.index(0, 1, proxyReg);

    hexDelegate.paint(&painter, option, proxyFldOffset);

    // 5. Model-level invalid check highlighted via proxy
    model.setData(model.index(0, 3, QModelIndex()), "", Qt::EditRole); // Empty name -> invalid
    model.checkData(32);
    QModelIndex proxyBlkName = proxy.index(0, 3, QModelIndex());
    RegStrDelegate strDelegate;
    strDelegate.paint(&painter, option, proxyBlkName);

    QVERIFY(!image.isNull());
}

void TestDelegates::testDelegateColorBlindModeAndBadgeEdges()
{
    QImage image(300, 40, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);
    QPainter painter(&image);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 300, 40); // Wide badge triggers badgeRect width adjustment

    RegMapTreeModel model;
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
    QModelIndex blkIndex = model.index(0, 0, QModelIndex());
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, blkIndex);
    QModelIndex regIndex = model.index(0, 0, blkIndex);
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::fld, regIndex);

    QModelIndex fldAccessIndex = model.index(0, 4, regIndex);
    QModelIndex fldHwIndex = model.index(0, 5, regIndex);

    // 1. Paint access "NA" or empty string
    model.setData(fldAccessIndex, "NA", Qt::EditRole);
    RegAccessPolicyDelegate swDelegate;
    swDelegate.paint(&painter, option, fldAccessIndex);

    model.setData(fldAccessIndex, "", Qt::EditRole);
    swDelegate.paint(&painter, option, fldAccessIndex);

    // 2. Paint with parent property "colorBlindMode" set to true
    QWidget parentWidget;
    parentWidget.setProperty("colorBlindMode", true);
    RegAccessPolicyDelegate swDelegateParent(&parentWidget);
    model.setData(fldAccessIndex, "RW", Qt::EditRole);
    swDelegateParent.paint(&painter, option, fldAccessIndex);

    RegHwAccessDelegate hwDelegateParent(&parentWidget);
    model.setData(fldHwIndex, "RO", Qt::EditRole);
    hwDelegateParent.paint(&painter, option, fldHwIndex);

    // 3. Paint with global Universal ColorBlindMode
    ThemeManager::instance().setColorBlindMode(ColorBlindMode::Universal);
    swDelegate.paint(&painter, option, fldAccessIndex);
    hwDelegateParent.paint(&painter, option, fldHwIndex);

    // Empty HW access defaults to RO
    model.setData(fldHwIndex, "", Qt::EditRole);
    hwDelegateParent.paint(&painter, option, fldHwIndex);

    // Reset back to None
    ThemeManager::instance().setColorBlindMode(ColorBlindMode::None);

    QVERIFY(!image.isNull());
}

void TestDelegates::testDelegateEventsEdgeCases()
{
    QWidget parent;
    QStyleOptionViewItem option;

    RegMapTreeModel model;
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
    QModelIndex blkIndex = model.index(0, 0, QModelIndex());
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, blkIndex);
    QModelIndex regIndex = model.index(0, 0, blkIndex);
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::fld, regIndex);

    QModelIndex fldAccessIndex = model.index(0, 4, regIndex);
    QModelIndex fldHwIndex = model.index(0, 5, regIndex);
    QModelIndex fldBoolIndex = model.index(0, 7, regIndex);

    // 1. Right click event is ignored by all single-click cycling delegates
    QMouseEvent rightClick(QEvent::MouseButtonRelease, QPointF(10, 10), QPointF(10, 10), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPointF(10, 10), QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    RegAccessPolicyDelegate swDelegate(&parent);
    QVERIFY(!swDelegate.editorEvent(&rightClick, &model, option, fldAccessIndex));
    QVERIFY(!swDelegate.editorEvent(&pressEvent, &model, option, fldAccessIndex));

    RegHwAccessDelegate hwDelegate(&parent);
    QVERIFY(!hwDelegate.editorEvent(&rightClick, &model, option, fldHwIndex));
    QVERIFY(!hwDelegate.editorEvent(&pressEvent, &model, option, fldHwIndex));

    RegBoolDelegate boolDelegate(&parent);
    QVERIFY(!boolDelegate.editorEvent(&rightClick, &model, option, fldBoolIndex));
    QVERIFY(!boolDelegate.editorEvent(&pressEvent, &model, option, fldBoolIndex));

    // 2. Cycling from unknown / non-standard policy defaults to standard
    model.setData(fldAccessIndex, "CUSTOM_POLICY", Qt::EditRole);
    QMouseEvent leftClick(QEvent::MouseButtonRelease, QPointF(10, 10), QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QVERIFY(swDelegate.editorEvent(&leftClick, &model, option, fldAccessIndex));
    QCOMPARE(model.data(fldAccessIndex, Qt::DisplayRole).toString(), QString("RW"));

    model.setData(fldHwIndex, "CUSTOM_HW", Qt::EditRole);
    QVERIFY(hwDelegate.editorEvent(&leftClick, &model, option, fldHwIndex));
    QCOMPARE(model.data(fldHwIndex, Qt::DisplayRole).toString(), QString("RO"));

    // 3. Bool delegate toggle with "1" and other values
    model.setData(fldBoolIndex, "1", Qt::EditRole);
    QVERIFY(boolDelegate.editorEvent(&leftClick, &model, option, fldBoolIndex));
    QCOMPARE(model.data(fldBoolIndex, Qt::DisplayRole).toString(), QString("false"));

    model.setData(fldBoolIndex, "0", Qt::EditRole);
    QVERIFY(boolDelegate.editorEvent(&leftClick, &model, option, fldBoolIndex));
    QCOMPARE(model.data(fldBoolIndex, Qt::DisplayRole).toString(), QString("true"));

    // 4. setEditorData with value not in combo box
    QWidget *editor = swDelegate.createEditor(&parent, option, fldAccessIndex);
    model.setData(fldAccessIndex, "UNRECOGNIZED", Qt::EditRole);
    swDelegate.setEditorData(editor, fldAccessIndex);
    delete editor;

    QWidget *hwEditor = hwDelegate.createEditor(&parent, option, fldHwIndex);
    model.setData(fldHwIndex, "UNRECOGNIZED", Qt::EditRole);
    hwDelegate.setEditorData(hwEditor, fldHwIndex);
    delete hwEditor;

    QWidget *boolEditor = boolDelegate.createEditor(&parent, option, fldBoolIndex);
    model.setData(fldBoolIndex, "UNRECOGNIZED", Qt::EditRole);
    boolDelegate.setEditorData(boolEditor, fldBoolIndex);
    delete boolEditor;

    // 5. Test getAccessPolicyColors boolean overload
    AccessColors acSwFalse = getAccessPolicyColors("RW", false);
    AccessColors acSwTrue = getAccessPolicyColors("RW", true);
    QVERIFY(acSwFalse.bg.isValid());
    QVERIFY(acSwTrue.bg.isValid());
}

QTEST_MAIN(TestDelegates)
#include "test_Delegates.moc"
