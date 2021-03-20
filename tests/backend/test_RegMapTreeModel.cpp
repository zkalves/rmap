#include <QtTest>
#include "RegMapTreeModel.hpp"
#include "RegMapTreeItem.hpp"

class TestRegMapTreeModel : public QObject
{
    Q_OBJECT

private slots:
    void testModelStructureAndHeaders();
    void testItemInsertionAndRemoval();
    void testDataGetAndSet();
    void testJsonExtraction();
    void testValidationRules();
    void test64BitWideRegisters();
    void testInvalidCellsTracking();
};

void TestRegMapTreeModel::testModelStructureAndHeaders()
{
    RegMapTreeModel model;
    QCOMPARE(model.columnCount(), 11);
    QCOMPARE(model.headerData(0, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Type"));
    QCOMPARE(model.headerData(1, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Offset"));
    QCOMPARE(model.headerData(2, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Size"));
    QCOMPARE(model.headerData(3, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Name"));
    QCOMPARE(model.headerData(4, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Access Policy"));
    QCOMPARE(model.headerData(5, Qt::Horizontal, Qt::DisplayRole).toString(), QString("HW Access"));
    QCOMPARE(model.headerData(6, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Reset Value"));
    QCOMPARE(model.headerData(7, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Is Rand"));
    QCOMPARE(model.headerData(8, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Volatile"));
    QCOMPARE(model.headerData(9, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Has Reset"));
    QCOMPARE(model.headerData(10, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Description"));
}

void TestRegMapTreeModel::testItemInsertionAndRemoval()
{
    RegMapTreeModel model;

    // Insert Block at root
    QVERIFY(model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex()));
    QCOMPARE(model.rowCount(QModelIndex()), 1);

    QModelIndex blkIndex = model.index(0, 0, QModelIndex());
    QVERIFY(blkIndex.isValid());

    // Insert Register under Block
    QVERIFY(model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, blkIndex));
    QCOMPARE(model.rowCount(blkIndex), 1);

    QModelIndex regIndex = model.index(0, 0, blkIndex);
    QVERIFY(regIndex.isValid());

    // Insert Field under Register
    QVERIFY(model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::fld, regIndex));
    QCOMPARE(model.rowCount(regIndex), 1);

    // Remove Field
    QVERIFY(model.removeRows(0, 1, regIndex));
    QCOMPARE(model.rowCount(regIndex), 0);

    // Remove Register
    QVERIFY(model.removeRows(0, 1, blkIndex));
    QCOMPARE(model.rowCount(blkIndex), 0);

    // Remove Block
    QVERIFY(model.removeRows(0, 1, QModelIndex()));
    QCOMPARE(model.rowCount(QModelIndex()), 0);
}

void TestRegMapTreeModel::testDataGetAndSet()
{
    RegMapTreeModel model;
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
    QModelIndex blkIndex = model.index(0, 3, QModelIndex()); // Column 3: Name

    QVERIFY(model.setData(blkIndex, "MY_BLOCK", Qt::EditRole));
    QCOMPARE(model.data(blkIndex, Qt::DisplayRole).toString(), QString("MY_BLOCK"));

    // Insert register and test Description (Column 10)
    QModelIndex blkCol0 = model.index(0, 0, QModelIndex());
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, blkCol0);
    QModelIndex regDescIndex = model.index(0, 10, blkCol0); // Column 10: Description
    QVERIFY(model.setData(regDescIndex, "Main Control Register for SPI interface", Qt::EditRole));
    QCOMPARE(model.data(regDescIndex, Qt::DisplayRole).toString(), QString("Main Control Register for SPI interface"));
}

void TestRegMapTreeModel::testJsonExtraction()
{
    RegMapTreeModel model;
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
    QModelIndex blkIndex = model.index(0, 0, QModelIndex());
    model.setData(model.index(0, 3, QModelIndex()), "SPI_TOP", Qt::EditRole);

    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, blkIndex);
    QModelIndex regIndex = model.index(0, 0, blkIndex);
    model.setData(model.index(0, 1, blkIndex), "0x00", Qt::EditRole);
    model.setData(model.index(0, 3, blkIndex), "CTRL", Qt::EditRole);

    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::fld, regIndex);
    model.setData(model.index(0, 1, regIndex), "0", Qt::EditRole);     // LSB: 0
    model.setData(model.index(0, 2, regIndex), "1", Qt::EditRole);     // Width: 1
    model.setData(model.index(0, 3, regIndex), "ENABLE", Qt::EditRole);// Name: ENABLE
    model.setData(model.index(0, 4, regIndex), "RW", Qt::EditRole);    // Access: RW

    json extracted = model.extractJsonData(32);
    QCOMPARE(extracted["reg_width"].get<int>(), 32);
    QCOMPARE(extracted["reg_width_bytes"].get<int>(), 4);
    QVERIFY(extracted.contains("blocks"));
    QCOMPARE(extracted["blocks"].size(), (size_t)1);
    QCOMPARE(extracted["blocks"][0]["name"].get<std::string>(), std::string("SPI_TOP"));
    QVERIFY(extracted["blocks"][0].contains("registers"));
    QCOMPARE(extracted["blocks"][0]["registers"].size(), (size_t)1);
    QCOMPARE(extracted["blocks"][0]["registers"][0]["name"].get<std::string>(), std::string("CTRL"));
    QVERIFY(extracted["blocks"][0]["registers"][0].contains("fields"));
    QCOMPARE(extracted["blocks"][0]["registers"][0]["fields"].size(), (size_t)1);
    QCOMPARE(extracted["blocks"][0]["registers"][0]["fields"][0]["name"].get<std::string>(), std::string("ENABLE"));
}

void TestRegMapTreeModel::testValidationRules()
{
    RegMapTreeModel model;

    // 1. Clean hierarchy should produce 0 errors
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
    QModelIndex blkIndex = model.index(0, 0, QModelIndex());
    model.setData(model.index(0, 3, QModelIndex()), "BLK0", Qt::EditRole);

    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, blkIndex);
    QModelIndex reg1Index = model.index(0, 0, blkIndex);
    model.setData(model.index(0, 1, blkIndex), "0x00", Qt::EditRole);
    model.setData(model.index(0, 3, blkIndex), "REG0", Qt::EditRole);

    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::fld, reg1Index);
    model.setData(model.index(0, 1, reg1Index), "0", Qt::EditRole); // LSB
    model.setData(model.index(0, 2, reg1Index), "8", Qt::EditRole); // Width
    model.setData(model.index(0, 3, reg1Index), "FLD0", Qt::EditRole);

    QStringList errorsClean = model.checkData(32);
    QVERIFY2(errorsClean.isEmpty(), "Valid model should have zero errors");

    // 2. Field width overflow (LSB 28 + Width 8 = MSB 35 >= 32)
    model.insertRows(1, 1, RegMapTreeItem::e_rmmKind::fld, reg1Index);
    model.setData(model.index(1, 1, reg1Index), "28", Qt::EditRole);
    model.setData(model.index(1, 2, reg1Index), "8", Qt::EditRole);
    model.setData(model.index(1, 3, reg1Index), "FLD_OVERFLOW", Qt::EditRole);

    QStringList errorsOverflow = model.checkData(32);
    QVERIFY(!errorsOverflow.isEmpty());
    QVERIFY(errorsOverflow.join("\n").contains("exceeds register width"));

    // Remove the overflowing field
    model.removeRows(1, 1, reg1Index);

    // 3. Bitfield collision (FLD0: 0..7, FLD_COLLIDE: 4..11)
    model.insertRows(1, 1, RegMapTreeItem::e_rmmKind::fld, reg1Index);
    model.setData(model.index(1, 1, reg1Index), "4", Qt::EditRole);
    model.setData(model.index(1, 2, reg1Index), "8", Qt::EditRole);
    model.setData(model.index(1, 3, reg1Index), "FLD_COLLIDE", Qt::EditRole);

    QStringList errorsFldCollide = model.checkData(32);
    QVERIFY(!errorsFldCollide.isEmpty());
    QVERIFY(errorsFldCollide.join("\n").contains("overlaps with"));

    // Remove the colliding field
    model.removeRows(1, 1, reg1Index);

    // 4. Register address collision (REG0 @ 0x00, REG1 @ 0x00)
    model.insertRows(1, 1, RegMapTreeItem::e_rmmKind::reg, blkIndex);
    model.setData(model.index(1, 1, blkIndex), "0x00", Qt::EditRole); // Same offset 0x00
    model.setData(model.index(1, 3, blkIndex), "REG1", Qt::EditRole);

    QStringList errorsRegCollide = model.checkData(32);
    QVERIFY(!errorsRegCollide.isEmpty());
    QVERIFY(errorsRegCollide.join("\n").contains("overlaps with"));
}

void TestRegMapTreeModel::test64BitWideRegisters()
{
    RegMapTreeModel model;
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
    QModelIndex blkIndex = model.index(0, 0, QModelIndex());
    model.setData(model.index(0, 3, QModelIndex()), "BLK64", Qt::EditRole);

    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, blkIndex);
    QModelIndex regIndex = model.index(0, 0, blkIndex);
    model.setData(model.index(0, 1, blkIndex), "0x00", Qt::EditRole);
    model.setData(model.index(0, 3, blkIndex), "WIDE_REG", Qt::EditRole);

    // Add field spanning bit 48..63 (LSB 48, Width 16)
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::fld, regIndex);
    model.setData(model.index(0, 1, regIndex), "48", Qt::EditRole);
    model.setData(model.index(0, 2, regIndex), "16", Qt::EditRole);
    model.setData(model.index(0, 3, regIndex), "HIGH_FIELD", Qt::EditRole);

    // In 32-bit mode, this should fail
    QStringList errors32 = model.checkData(32);
    QVERIFY(!errors32.isEmpty());

    // In 64-bit mode, this should pass cleanly
    QStringList errors64 = model.checkData(64);
    QVERIFY2(errors64.isEmpty(), "Valid 64-bit field should pass with regWidth=64");

    // JSON export in 64-bit mode
    json extracted = model.extractJsonData(64);
    QCOMPARE(extracted["reg_width"].get<int>(), 64);
    QCOMPARE(extracted["reg_width_bytes"].get<int>(), 8);
}

void TestRegMapTreeModel::testInvalidCellsTracking()
{
    RegMapTreeModel model;
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
    QModelIndex blkIndex = model.index(0, 0, QModelIndex());

    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, blkIndex);
    QModelIndex regIndex = model.index(0, 0, blkIndex);

    // Add overflowing field
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::fld, regIndex);
    model.setData(model.index(0, 1, regIndex), "30", Qt::EditRole);
    model.setData(model.index(0, 2, regIndex), "8", Qt::EditRole); // 30 + 8 = 38 > 32

    model.checkData(32);

    // Verify cell is marked invalid
    QModelIndex lsbIndex = model.index(0, 1, regIndex);
    QVERIFY(model.isIndexInvalid(lsbIndex));
}

QTEST_MAIN(TestRegMapTreeModel)
#include "test_RegMapTreeModel.moc"
