/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include <QtTest>
#include "RegMapTreeModel.hpp"
#include "RegMapTreeItem.hpp"

class TestableRegMapTreeModel : public RegMapTreeModel
{
public:
    using RegMapTreeModel::createIndex;
};

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
    void testCrc32AndMemoryGapPadding();
    void testAsicLinterDrc();
    void testMemoryOverlapValidation();
    void testIndexAndDataRolesEdgeCases();
    void testLinterExtendedRules();
    void testModelCoverageEdgeCases();
    void testUvmCookbookRalProperties();
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

void TestRegMapTreeModel::testCrc32AndMemoryGapPadding()
{
    RegMapTreeModel model;
    // Insert Block
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
    QModelIndex blkIndex = model.index(0, 0, QModelIndex());
    model.setData(model.index(0, 3, QModelIndex()), "SPI_CORE", Qt::EditRole);
    model.setData(model.index(0, 1, QModelIndex()), "0", Qt::EditRole);

    // Insert Register 0 @ offset 0x0
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, blkIndex);
    QModelIndex reg0Index = model.index(0, 0, blkIndex);
    model.setData(model.index(0, 3, blkIndex), "CTRL", Qt::EditRole);
    model.setData(model.index(0, 1, blkIndex), "0", Qt::EditRole);

    // Insert Register 1 @ offset 0x10 (16) -> 12 byte gap between reg0 (size 4) and reg1 (offset 16)
    model.insertRows(1, 1, RegMapTreeItem::e_rmmKind::reg, blkIndex);
    model.setData(model.index(1, 3, blkIndex), "STATUS", Qt::EditRole);
    model.setData(model.index(1, 1, blkIndex), "16", Qt::EditRole);

    json extracted = model.extractJsonData(32);

    // Verify CRC32 presence
    QVERIFY(extracted.contains("regmap_crc32"));
    QVERIFY(extracted.contains("regmap_crc32_hex"));
    uint32_t crc1 = extracted["regmap_crc32"].get<uint32_t>();
    QVERIFY(crc1 != 0);
    QString crcHex = QString::fromStdString(extracted["regmap_crc32_hex"].get<std::string>());
    QVERIFY(crcHex.startsWith("0x"));

    // Verify Block-level CRC
    QVERIFY(extracted["blocks"][0].contains("crc32"));
    QVERIFY(extracted["blocks"][0].contains("crc32_hex"));

    // Verify memory gap calculation:
    // reg0: offset 0 -> pad_bytes_before = 0, pad_words_before = 0
    // reg1: offset 16 -> pad_bytes_before = 12, pad_words_before = 3 (12 / 4)
    auto &regs = extracted["blocks"][0]["registers"];
    QCOMPARE(regs.size(), (size_t)2);
    QCOMPARE(regs[0]["pad_bytes_before"].get<uint64_t>(), 0ULL);
    QCOMPARE(regs[0]["pad_words_before"].get<uint64_t>(), 0ULL);
    QCOMPARE(regs[1]["pad_bytes_before"].get<uint64_t>(), 12ULL);
    QCOMPARE(regs[1]["pad_words_before"].get<uint64_t>(), 3ULL);

    // Verify CRC determinism when offset changes
    model.setData(model.index(1, 1, blkIndex), "32", Qt::EditRole);
    json extractedModified = model.extractJsonData(32);
    uint32_t crc2 = extractedModified["regmap_crc32"].get<uint32_t>();
    QVERIFY(crc1 != crc2);

    // Restore offset -> CRC matches crc1
    model.setData(model.index(1, 1, blkIndex), "16", Qt::EditRole);
    json extractedRestored = model.extractJsonData(32);
    uint32_t crc3 = extractedRestored["regmap_crc32"].get<uint32_t>();
    QCOMPARE(crc1, crc3);

    // Insert a second block to exercise multi-block sorting lambda
    model.insertRows(1, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
    model.setData(model.index(1, 3, QModelIndex()), "BLK_SECOND", Qt::EditRole);
    model.setData(model.index(1, 1, QModelIndex()), "0x1000", Qt::EditRole);
    json multiBlkJson = model.extractJsonData(32);
    QCOMPARE(multiBlkJson["blocks"].size(), (size_t)2);
}

void TestRegMapTreeModel::testAsicLinterDrc()
{
    RegMapTreeModel model;

    // 1. Reserved Keyword Check
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
    QModelIndex blkIndex = model.index(0, 0, QModelIndex());
    model.setData(model.index(0, 3, QModelIndex()), "BLK_TEST", Qt::EditRole);

    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, blkIndex);
    QModelIndex regIndex = model.index(0, 0, blkIndex);
    model.setData(model.index(0, 1, blkIndex), "0x00", Qt::EditRole);
    model.setData(model.index(0, 3, blkIndex), "logic", Qt::EditRole); // SystemVerilog keyword

    QStringList errKw = model.checkData(32);
    QVERIFY(!errKw.isEmpty());
    QVERIFY(errKw.join("\n").contains("uses reserved keyword 'logic'"));
    QVERIFY(model.isIndexInvalid(model.index(0, 3, blkIndex)));

    // Fix register name, test field keyword
    model.setData(model.index(0, 3, blkIndex), "REG_CFG", Qt::EditRole);
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::fld, regIndex);
    QModelIndex fldIndex = model.index(0, 0, regIndex);
    model.setData(model.index(0, 1, regIndex), "0", Qt::EditRole);
    model.setData(model.index(0, 2, regIndex), "4", Qt::EditRole);
    model.setData(model.index(0, 3, regIndex), "wire", Qt::EditRole); // Verilog keyword

    QStringList errFldKw = model.checkData(32);
    QVERIFY(!errFldKw.isEmpty());
    QVERIFY(errFldKw.join("\n").contains("uses reserved keyword 'wire'"));
    QVERIFY(model.isIndexInvalid(model.index(0, 3, regIndex)));

    // Fix field name
    model.setData(model.index(0, 3, regIndex), "FIELD_A", Qt::EditRole);

    // 2. Field Reset Value Overflow Check (width 4 -> max 15 / 0x0F, set to 0x20)
    model.setData(model.index(0, 6, regIndex), "0x20", Qt::EditRole);
    QStringList errReset = model.checkData(32);
    QVERIFY(!errReset.isEmpty());
    QVERIFY(errReset.join("\n").contains("overflows bit width 4"));
    QVERIFY(model.isIndexInvalid(model.index(0, 6, regIndex))); // Reset Value cell marked invalid

    // Fix reset value
    model.setData(model.index(0, 6, regIndex), "0x0A", Qt::EditRole);
    QVERIFY(model.checkData(32).isEmpty());

    // 3. Contradictory Policies (SW=WO && HW=WO)
    model.setData(model.index(0, 4, regIndex), "WO", Qt::EditRole);
    model.setData(model.index(0, 5, regIndex), "WO", Qt::EditRole);
    QStringList errContradict = model.checkData(32);
    QVERIFY(!errContradict.isEmpty());
    QVERIFY(errContradict.join("\n").contains("both SW and HW are Write-Only"));
    QVERIFY(model.isIndexInvalid(model.index(0, 4, regIndex)));
    QVERIFY(model.isIndexInvalid(model.index(0, 5, regIndex)));

    // Fix HW access to RO
    model.setData(model.index(0, 5, regIndex), "RO", Qt::EditRole);
    QVERIFY(model.checkData(32).isEmpty());

    // 4. Reset Consistency (Has Reset = false but Reset Value != 0)
    model.setData(model.index(0, 9, regIndex), "false", Qt::EditRole);
    QStringList errResetConsist = model.checkData(32);
    QVERIFY(!errResetConsist.isEmpty());
    QVERIFY(errResetConsist.join("\n").contains("specifies non-zero reset value"));
    QVERIFY(model.isIndexInvalid(model.index(0, 6, regIndex)));
    QVERIFY(model.isIndexInvalid(model.index(0, 9, regIndex)));

    // Fix: reset value = 0 or Has Reset = true
    model.setData(model.index(0, 9, regIndex), "true", Qt::EditRole);
    QVERIFY(model.checkData(32).isEmpty());
}

void TestRegMapTreeModel::testMemoryOverlapValidation()
{
    RegMapTreeModel model;

    // Add block
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
    QModelIndex blkIndex = model.index(0, 0, QModelIndex());
    model.setData(model.index(0, 3, QModelIndex()), "SRAM_BLOCK", Qt::EditRole);

    // Add register at 0x1000 (size 4 bytes -> 0x1000 to 0x1003)
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, blkIndex);
    QModelIndex regIndex = model.index(0, 0, blkIndex);
    model.setData(model.index(0, 1, blkIndex), "0x1000", Qt::EditRole);
    model.setData(model.index(0, 3, blkIndex), "REG_CFG", Qt::EditRole);

    // Add memory at 0x1000 (size 1024 bytes -> overlaps with register!)
    model.insertRows(1, 1, RegMapTreeItem::e_rmmKind::mem, blkIndex);
    QModelIndex memIndex = model.index(1, 0, blkIndex);
    model.setData(model.index(1, 1, blkIndex), "0x1000", Qt::EditRole);
    model.setData(model.index(1, 2, blkIndex), "1024", Qt::EditRole);
    model.setData(model.index(1, 3, blkIndex), "MEM_BUF", Qt::EditRole);

    QStringList errs = model.checkData(32);
    QVERIFY(!errs.isEmpty());
    QVERIFY(errs.join("\n").contains("overlaps"));
    QVERIFY(model.isIndexInvalid(model.index(0, 1, blkIndex)));
    QVERIFY(model.isIndexInvalid(model.index(1, 1, blkIndex)));

    // Move memory to non-overlapping address 0x2000
    model.setData(model.index(1, 1, blkIndex), "0x2000", Qt::EditRole);
    QVERIFY(model.checkData(32).isEmpty());
    QVERIFY(!model.isIndexInvalid(model.index(0, 1, blkIndex)));
    QVERIFY(!model.isIndexInvalid(model.index(1, 1, blkIndex)));
}

void TestRegMapTreeModel::testIndexAndDataRolesEdgeCases()
{
    RegMapTreeModel model;

    // 1. Column count with valid and invalid parent
    QCOMPARE(model.columnCount(QModelIndex()), 11);

    // 2. Index navigation boundaries
    QVERIFY(!model.index(-1, 0, QModelIndex()).isValid());
    QVERIFY(!model.index(0, -1, QModelIndex()).isValid());
    QVERIFY(!model.index(100, 0, QModelIndex()).isValid());
    QVERIFY(!model.index(0, 100, QModelIndex()).isValid());

    // 3. Parent of invalid index
    QVERIFY(!model.parent(QModelIndex()).isValid());

    // 4. Flags of invalid index
    QCOMPARE(model.flags(QModelIndex()), Qt::NoItemFlags);

    // 5. Data of invalid index
    QVERIFY(!model.data(QModelIndex(), Qt::DisplayRole).isValid());

    // 6. Header data edge cases
    QCOMPARE(model.headerData(1, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Offset"));
    QCOMPARE(model.headerData(2, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Size"));
    QCOMPARE(model.headerData(3, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Name"));
    QVERIFY(model.headerData(99, Qt::Horizontal, Qt::DisplayRole).toString().isEmpty());
    QVERIFY(!model.headerData(0, Qt::Vertical, Qt::DisplayRole).isValid());
    QVERIFY(!model.headerData(0, Qt::Horizontal, Qt::EditRole).isValid());

    // 7. Refresh header data
    model.refreshHeaderData();

    // 8. RowCount when parent.column() > 0
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
    QModelIndex blkCol1 = model.index(0, 1, QModelIndex());
    QCOMPARE(model.rowCount(blkCol1), 0);

    // 9. Invalid insertion kinds
    // Cannot insert field or reg at root
    QVERIFY(!model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::fld, QModelIndex()));
    QVERIFY(!model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, QModelIndex()));
    QModelIndex blkIndex = model.index(0, 0, QModelIndex());
    // Cannot insert field under block
    QVERIFY(!model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::fld, blkIndex));

    // Insert reg under blk
    QVERIFY(model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, blkIndex));
    QModelIndex regIndex = model.index(0, 0, blkIndex);
    // Cannot insert block under reg
    QVERIFY(!model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, regIndex));

    // Insert mem under blk
    QVERIFY(model.insertRows(1, 1, RegMapTreeItem::e_rmmKind::mem, blkIndex));
    QModelIndex memIndex = model.index(1, 0, blkIndex);
    QCOMPARE(model.data(model.index(1, 2, blkIndex), Qt::DisplayRole).toString(), QString("1024"));

    // 10. Parent pointers
    QCOMPARE(model.parent(blkIndex), QModelIndex());
    QCOMPARE(model.parent(regIndex), blkIndex);
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::fld, regIndex);
    QModelIndex fldIndex = model.index(0, 0, regIndex);
    QCOMPARE(model.parent(fldIndex), regIndex);

    // 11. Flags on root vs block vs field
    QVERIFY(!(model.flags(model.index(0, 0, QModelIndex())) & Qt::ItemIsEditable));
    QVERIFY(model.flags(model.index(0, 3, QModelIndex())) & Qt::ItemIsEditable);

    // 12. Data roles: DecorationRole, BackgroundRole, ToolTipRole, FontRole
    QVariant iconVar = model.data(model.index(0, 0, QModelIndex()), Qt::DecorationRole);
    QVERIFY(!model.data(model.index(0, 1, QModelIndex()), Qt::DecorationRole).isValid());
    QVERIFY(!model.data(model.index(0, 3, QModelIndex()), Qt::FontRole).isValid());
    QVERIFY(!model.data(model.index(0, 3, QModelIndex()), Qt::BackgroundRole).isValid());
    QVERIFY(!model.data(model.index(0, 3, QModelIndex()), Qt::ToolTipRole).isValid());

    // Set invalid index via checkData error (empty name)
    model.setData(model.index(0, 3, QModelIndex()), "", Qt::EditRole);
    model.checkData(32);
    QVERIFY(model.data(model.index(0, 3, QModelIndex()), Qt::BackgroundRole).isValid());
    QCOMPARE(model.data(model.index(0, 3, QModelIndex()), Qt::ToolTipRole).toString(),
             tr("Invalid value, address collision, or register width violation"));

    // 13. SetData with role != EditRole or invalid index
    QVERIFY(!model.setData(model.index(0, 3, QModelIndex()), "TEST", Qt::DisplayRole));
    QVERIFY(!model.setData(QModelIndex(), "TEST", Qt::EditRole));

    // 14. Padding hex offsets on reg vs fld
    model.setData(model.index(0, 1, blkIndex), "0x10", Qt::EditRole);
    QCOMPARE(model.data(model.index(0, 1, blkIndex), Qt::DisplayRole).toString(), QString("0x0010"));

    QModelIndex fldOffset = model.index(0, 1, regIndex);
    model.setData(fldOffset, "4", Qt::EditRole);
    QCOMPARE(model.data(fldOffset, Qt::DisplayRole).toString(), QString("4"));

    // 15. SetRootItem and clear
    QVERIFY(model.clear());
    QCOMPARE(model.rowCount(QModelIndex()), 0);

    QVariantMap rootData;
    rootData["Type"] = "root";
    RegMapTreeItem *newRoot = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::root, rootData);
    model.setRootItem(newRoot);
    QCOMPARE(model.getRootItem(), newRoot);
}

void TestRegMapTreeModel::testLinterExtendedRules()
{
    RegMapTreeModel model;

    // 1. Block with empty Name
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
    QModelIndex blkIndex = model.index(0, 0, QModelIndex());
    model.setData(model.index(0, 3, QModelIndex()), "", Qt::EditRole);

    QStringList errs = model.checkData(32);
    QVERIFY(errs.join("\n").contains("empty Name"));

    model.setData(model.index(0, 3, QModelIndex()), "VALID_BLOCK", Qt::EditRole);

    // 2. Field with bit width 0
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, blkIndex);
    QModelIndex regIndex = model.index(0, 0, blkIndex);
    model.setData(model.index(0, 1, blkIndex), "0x0", Qt::EditRole);
    model.setData(model.index(0, 3, blkIndex), "REG1", Qt::EditRole);

    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::fld, regIndex);
    QModelIndex fldIndex = model.index(0, 0, regIndex);
    model.setData(model.index(0, 1, regIndex), "0", Qt::EditRole);
    model.setData(model.index(0, 2, regIndex), "0", Qt::EditRole);
    model.setData(model.index(0, 3, regIndex), "ZERO_WIDTH_FLD", Qt::EditRole);

    QStringList errWidth0 = model.checkData(32);
    QVERIFY(errWidth0.join("\n").contains("invalid bit width 0"));

    // 3. Field exceeding register width
    model.setData(model.index(0, 1, regIndex), "30", Qt::EditRole);
    model.setData(model.index(0, 2, regIndex), "8", Qt::EditRole);
    QStringList errExceed = model.checkData(32);
    QVERIFY(errExceed.join("\n").contains("exceeds register width (32 bits)"));

    // Fix field
    model.setData(model.index(0, 1, regIndex), "0", Qt::EditRole);
    model.setData(model.index(0, 2, regIndex), "8", Qt::EditRole);
    QVERIFY(model.checkData(32).isEmpty());

    // 4. Register without fields, reset value overflow
    model.removeRows(0, 1, regIndex);
    model.setData(model.index(0, 6, blkIndex), "0x100000000", Qt::EditRole);
    QStringList errRegReset = model.checkData(32);
    QVERIFY(errRegReset.join("\n").contains("overflows register width (32 bits)"));
}

void TestRegMapTreeModel::testModelCoverageEdgeCases()
{
    TestableRegMapTreeModel model;

    // 1. Data with invalid internalPointer item -> covers L140
    QModelIndex fakeNullIdx = model.createIndex(0, 0, nullptr);
    QCOMPARE(model.data(fakeNullIdx, Qt::DisplayRole), QVariant());

    // 2. Parent with invalid internalPointer childItem -> covers L248
    QCOMPARE(model.parent(fakeNullIdx), QModelIndex());

    // 3. Qt::DecorationRole on col 0 with empty icon -> covers L153
    QVariantMap emptyData;
    RegMapTreeItem customItem(static_cast<RegMapTreeItem::e_rmmKind>(99), emptyData);
    QModelIndex customIdx = model.createIndex(0, 0, &customItem);
    QCOMPARE(model.data(customIdx, Qt::DecorationRole), QVariant());

    // 4. recursiveExtractJsonData(nullptr, 32) -> covers L581
    nlohmann::json nullJson = model.recursiveExtractJsonData(nullptr, 32);
    QVERIFY(nullJson.is_null() || nullJson.empty());

    // 5. computeTreeCrc32 with direct registers array -> covers L755-758
    nlohmann::json rootWithRegs;
    rootWithRegs["registers"] = nlohmann::json::array({
        { {"name", "DIRECT_REG"}, {"offset_lsb", 0x10}, {"size_width", 32}, {"access", "RW"}, {"reset_val", 0x1234} }
    });
    uint32_t crc = RegMapTreeModel::computeTreeCrc32(rootWithRegs);
    QVERIFY(crc != 0);

    // 6. Binary string parsing ("0b...") -> covers L64
    QVERIFY(model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex()));
    QModelIndex blkIdx = model.index(0, 0, QModelIndex());
    // Column 0 DisplayRole returns kindString -> covers L179
    QCOMPARE(model.data(blkIdx, Qt::DisplayRole).toString(), QString("blk"));

    QVERIFY(model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, blkIdx));
    QModelIndex regIdx = model.index(0, 0, blkIdx);
    model.setData(model.index(0, 1, blkIdx), "0x0", Qt::EditRole);
    model.setData(model.index(0, 6, blkIdx), "0b1010", Qt::EditRole); // binary reset value
    QCOMPARE(model.data(model.index(0, 6, blkIdx), Qt::DisplayRole).toString(), QString("0b1010"));

    // 7. Multiple fields with out-of-order offsets -> covers L658 (std::sort comparator)
    QVERIFY(model.insertRows(0, 2, RegMapTreeItem::e_rmmKind::fld, regIdx));
    QModelIndex fld0 = model.index(0, 0, regIdx);
    QModelIndex fld1 = model.index(1, 0, regIdx);
    model.setData(model.index(0, 1, regIdx), "8", Qt::EditRole);
    model.setData(model.index(0, 2, regIdx), "4", Qt::EditRole);
    model.setData(model.index(0, 3, regIdx), "FLD_HIGH", Qt::EditRole);
    model.setData(model.index(1, 1, regIdx), "0", Qt::EditRole);
    model.setData(model.index(1, 2, regIdx), "4", Qt::EditRole);
    model.setData(model.index(1, 3, regIdx), "FLD_LOW", Qt::EditRole);

    // 8. Memories with out-of-order offsets -> covers L626-627, 663-666
    QVERIFY(model.insertRows(1, 2, RegMapTreeItem::e_rmmKind::mem, QModelIndex()));
    model.setData(model.index(1, 1, QModelIndex()), "0x2000", Qt::EditRole);
    model.setData(model.index(1, 3, QModelIndex()), "MEM2", Qt::EditRole);
    model.setData(model.index(2, 1, QModelIndex()), "0x1000", Qt::EditRole);
    model.setData(model.index(2, 3, QModelIndex()), "MEM1", Qt::EditRole);

    // 9. Root item with empty Name and Description -> covers L762, L765
    model.getRootItem()->setData("Name", "");
    model.getRootItem()->setData("Description", "");
    nlohmann::json extracted = model.extractJsonData(32);
    QCOMPARE(QString::fromStdString(extracted["name"]), QString("regmap"));
    QCOMPARE(QString::fromStdString(extracted["description"]), QString("Hardware Register Map Specification"));
    QVERIFY(extracted.contains("memories"));
    QCOMPARE(extracted["memories"].size(), 2);
    QCOMPARE(QString::fromStdString(extracted["memories"][0]["name"]), QString("MEM1"));

    // 10. padHexOffset with invalid hex, and with > 32-bit address
    model.setData(model.index(0, 1, blkIdx), "0xNOT_HEX", Qt::EditRole);
    QCOMPARE(model.data(model.index(0, 1, blkIdx), Qt::DisplayRole).toString(), QString("0xNOT_HEX"));
    model.setData(model.index(0, 1, blkIdx), "0x10000", Qt::EditRole);
    QCOMPARE(model.data(model.index(0, 1, blkIdx), Qt::DisplayRole).toString(), QString("0x00010000"));
    model.setData(model.index(0, 1, blkIdx), "0x100000000", Qt::EditRole);
    QCOMPARE(model.data(model.index(0, 1, blkIdx), Qt::DisplayRole).toString(), QString("0x0000000100000000"));

    // 11. insertRow with prevOffset >= 0x100000000 (exercises formatHex with >= 0x100000000)
    QVERIFY(model.insertRows(1, 1, RegMapTreeItem::e_rmmKind::reg, blkIdx));
    QCOMPARE(model.data(model.index(1, 1, blkIdx), Qt::DisplayRole).toString(), QString("0x0000000100000004"));

    // 12. insertRow when prevSize == 0 (exercises byteSize = (prevSize == 0) ? 4 : prevSize / 8)
    model.setData(model.index(1, 2, blkIdx), "0", Qt::EditRole);
    QVERIFY(model.insertRows(2, 1, RegMapTreeItem::e_rmmKind::reg, blkIdx));
    QCOMPARE(model.data(model.index(2, 1, blkIdx), Qt::DisplayRole).toString(), QString("0x0000000100000008"));

    // 13. checkData with regWidth == 0 (defaults to 4 bytes for registers)
    QStringList zeroWidthErrors = model.checkData(0);
    Q_UNUSED(zeroWidthErrors);

    // 14. checkData with 64-bit regWidth and register with no fields
    RegMapTreeModel model64;
    model64.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
    model64.setData(model64.index(0, 3, QModelIndex()), "BLK64", Qt::EditRole);
    QModelIndex b64 = model64.index(0, 0, QModelIndex());
    model64.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, b64);
    model64.setData(model64.index(0, 3, b64), "REG64_NO_FLD", Qt::EditRole);
    model64.setData(model64.index(0, 6, b64), "0xFFFFFFFFFFFFFFFF", Qt::EditRole);
    QStringList errs64 = model64.checkData(64);
    QCOMPARE(errs64.size(), 0);

    // 15. Memory with size_bytes == 0 in checkData (defaults to 4 bytes)
    model.setData(model.index(1, 2, QModelIndex()), "0", Qt::EditRole);
    QStringList memZeroSizeErrs = model.checkData(32);
    Q_UNUSED(memZeroSizeErrs);

    // 16. data() with Qt::UserRole (non-display role fallback)
    QCOMPARE(model.data(blkIdx, Qt::UserRole), QVariant());

    // 17. insertRow after a register with offset in [0x10000, 0x100000000) (exercises line 75 of formatHex)
    model.setData(model.index(2, 1, blkIdx), "0x10000", Qt::EditRole);
    model.setData(model.index(2, 2, blkIdx), "32", Qt::EditRole);
    QVERIFY(model.insertRows(3, 1, RegMapTreeItem::e_rmmKind::reg, blkIdx));
    QCOMPARE(model.data(model.index(3, 1, blkIdx), Qt::DisplayRole).toString(), QString("0x00010004"));

    // 18. Memory inside a block in checkData (childKind == mem)
    QVERIFY(model.insertRows(4, 1, RegMapTreeItem::e_rmmKind::mem, blkIdx));
    model.setData(model.index(4, 1, blkIdx), "0x20000", Qt::EditRole);
    model.setData(model.index(4, 2, blkIdx), "1024", Qt::EditRole);
    model.setData(model.index(4, 3, blkIdx), "BLK_MEM", Qt::EditRole);
    QStringList blkMemErrs = model.checkData(32);
    Q_UNUSED(blkMemErrs);

    // 19. Empty access policy and HW access in extractJsonData
    model.setData(model.index(3, 4, blkIdx), "", Qt::EditRole);
    model.setData(model.index(3, 5, blkIdx), "", Qt::EditRole);
    nlohmann::json emptyAccessJson = model.extractJsonData(32);
    Q_UNUSED(emptyAccessJson);

    // 20. Child item of kind 'map' in extractJsonData
    QVariantMap mapData;
    mapData["Name"] = "SYS_MAP";
    RegMapTreeItem *rootItem = model.getRootItem();
    RegMapTreeItem *mapChild = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::map, mapData, rootItem);
    rootItem->appendChild(mapChild);
    nlohmann::json mapJson = model.extractJsonData(32);
    Q_UNUSED(mapJson);

    // 21. setRootItem with the SAME root item (exercises line 280: m_rootItem == item)
    model.setRootItem(model.getRootItem());

    // 22. checkData with 64-bit field width
    QModelIndex reg0 = model.index(0, 0, blkIdx);
    QVERIFY(model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::fld, reg0));
    model.setData(model.index(0, 1, reg0), "0", Qt::EditRole);
    model.setData(model.index(0, 2, reg0), "64", Qt::EditRole); // width == 64
    model.setData(model.index(0, 3, reg0), "WIDE_FLD", Qt::EditRole);
    QStringList wideErrs = model.checkData(64);
    Q_UNUSED(wideErrs);

    // 23. extractJsonData with regWidth < 8
    nlohmann::json zeroWidthJson = model.extractJsonData(0);
    Q_UNUSED(zeroWidthJson);

    // 24. computeBlockCrc32 and computeTreeCrc32 with non-array "registers", "fields", "blocks"
    nlohmann::json badRootJson;
    badRootJson["blocks"] = "not_an_array";
    badRootJson["registers"] = "not_an_array";
    uint32_t c1 = RegMapTreeModel::computeTreeCrc32(badRootJson);
    Q_UNUSED(c1);

    nlohmann::json badBlkJson;
    badBlkJson["name"] = "B1";
    badBlkJson["registers"] = 12345; // not an array
    uint32_t c2 = RegMapTreeModel::computeBlockCrc32(badBlkJson);
    Q_UNUSED(c2);

    nlohmann::json badFldJson;
    badFldJson["name"] = "B2";
    nlohmann::json singleReg;
    singleReg["name"] = "R1";
    singleReg["fields"] = "not_an_array";
    badFldJson["registers"] = nlohmann::json::array({singleReg});
    uint32_t c3 = RegMapTreeModel::computeBlockCrc32(badFldJson);
    Q_UNUSED(c3);

    // 25. checkData with mem child item (with size == 0 and non-zero)
    QVariantMap memDataZero;
    memDataZero["Type"] = "mem";
    memDataZero["Name"] = "MEM_ZERO";
    memDataZero["Offset/LSB"] = "0x1000";
    memDataZero["Size/Width"] = "0"; // size_bytes == 0 -> size_bytes = 4
    RegMapTreeItem *memItemZero = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::mem, memDataZero, rootItem);
    rootItem->appendChild(memItemZero);

    QVariantMap memDataNonZero;
    memDataNonZero["Type"] = "mem";
    memDataNonZero["Name"] = "MEM_NONZERO";
    memDataNonZero["Offset/LSB"] = "0x2000";
    memDataNonZero["Size/Width"] = "1024";
    RegMapTreeItem *memItemNonZero = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::mem, memDataNonZero, rootItem);
    rootItem->appendChild(memItemNonZero);

    QStringList memErrs = model.checkData(32);
    Q_UNUSED(memErrs);

    // 26. extractJsonData with mem nodes (size_width == 0 -> 1024)
    nlohmann::json memExtracted = model.extractJsonData(32);
    Q_UNUSED(memExtracted);

    // 27. computeTreeCrc32 with JSON object missing "blocks" key entirely
    nlohmann::json emptyRootObj = nlohmann::json::object();
    uint32_t cEmpty = RegMapTreeModel::computeTreeCrc32(emptyRootObj);
    Q_UNUSED(cEmpty);

    nlohmann::json emptyBlkObj = nlohmann::json::object();
    uint32_t cEmptyBlk = RegMapTreeModel::computeBlockCrc32(emptyBlkObj);
    Q_UNUSED(cEmptyBlk);
}

void TestRegMapTreeModel::testUvmCookbookRalProperties()
{
    RegMapTreeModel model;
    RegMapTreeItem *root = model.getRootItem();

    // 1. Test Block with custom map
    QVariantMap blkData;
    blkData["Name"] = "SUB_SYSTEM";
    blkData["Offset/LSB"] = "0x0000";
    blkData["HDL Path"] = "tb_top.dut.subsys";
    RegMapTreeItem *blk = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::blk, blkData, root);
    root->appendChild(blk);

    // Add map under block
    QVariantMap mapData;
    mapData["Name"] = "AHB_MAP";
    mapData["Offset/LSB"] = "0x1000";
    mapData["Size/Width"] = "4";
    mapData["Endianness"] = "UVM_LITTLE_ENDIAN";
    mapData["Byte Addressing"] = "true";
    RegMapTreeItem *mapItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::map, mapData, blk);
    blk->appendChild(mapItem);

    // Verify map possible children
    auto mapChildren = mapItem->possibleChildren();
    QVERIFY(mapChildren.contains(RegMapTreeItem::e_rmmKind::reg));
    QVERIFY(mapChildren.contains(RegMapTreeItem::e_rmmKind::mem));

    // Add register with HDL Path, test disables, and FIFO attributes
    QVariantMap regData;
    regData["Name"] = "FIFO_REG";
    regData["Offset/LSB"] = "0x00";
    regData["Size/Width"] = "32";
    regData["HDL Path"] = "dut.subsys.fifo_reg_q";
    regData["NO_REG_BIT_BASH_TEST"] = "true";
    regData["NO_REG_HW_RESET_TEST"] = "true";
    regData["Reg Type"] = "fifo";
    regData["FIFO Depth"] = "16";
    RegMapTreeItem *regItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, regData, blk);
    blk->appendChild(regItem);

    // Add field with Individually Accessible attribute
    QVariantMap fldData;
    fldData["Name"] = "DATA";
    fldData["Offset/LSB"] = "0";
    fldData["Size/Width"] = "8";
    fldData["Access Policy"] = "W1";
    fldData["Individually Accessible"] = "false";
    RegMapTreeItem *fldItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::fld, fldData, regItem);
    regItem->appendChild(fldItem);

    // Add memory with depth, word width, HDL path, and NO_MEM_WALK_TEST
    QVariantMap memData;
    memData["Name"] = "PACKET_RAM";
    memData["Offset/LSB"] = "0x2000";
    memData["Size/Width"] = "4096";
    memData["Depth"] = "1024";
    memData["Word Width"] = "32";
    memData["HDL Path"] = "dut.subsys.packet_ram_mem";
    memData["NO_MEM_WALK_TEST"] = "true";
    memData["NO_MEM_ACCESS_TEST"] = "true";
    RegMapTreeItem *memItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::mem, memData, blk);
    blk->appendChild(memItem);

    // Add indirect register with Index Reg and Callbacks
    QVariantMap indData;
    indData["Name"] = "INDIRECT_DATA";
    indData["Offset/LSB"] = "0x08";
    indData["Size/Width"] = "32";
    indData["Reg Type"] = "indirect";
    indData["Index Reg"] = "FIFO_REG";
    indData["Has Callbacks"] = "true";
    RegMapTreeItem *indItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, indData, blk);
    blk->appendChild(indItem);

    // Extract JSON and verify UVM RAL properties
    nlohmann::json rootJson = model.extractJsonData(32);
    QVERIFY(rootJson.contains("blocks"));
    QCOMPARE(rootJson["blocks"].size(), 1);

    const auto &blkJson = rootJson["blocks"][0];
    QCOMPARE(blkJson["name"].get<std::string>(), std::string("SUB_SYSTEM"));
    QCOMPARE(blkJson["hdl_path"].get<std::string>(), std::string("tb_top.dut.subsys"));

    // Maps verification
    QVERIFY(blkJson.contains("maps"));
    QCOMPARE(blkJson["maps"].size(), 1);
    const auto &extractedMapJson = blkJson["maps"][0];
    QCOMPARE(extractedMapJson["name"].get<std::string>(), std::string("AHB_MAP"));
    QCOMPARE(extractedMapJson["base_addr"].get<uint64_t>(), 0x1000ULL);
    QCOMPARE(extractedMapJson["n_bytes"].get<uint64_t>(), 4ULL);
    QCOMPARE(extractedMapJson["endianness"].get<std::string>(), std::string("UVM_LITTLE_ENDIAN"));
    QCOMPARE(extractedMapJson["byte_addressing"].get<int>(), 1);

    // Register verification
    QVERIFY(blkJson.contains("registers"));
    QCOMPARE(blkJson["registers"].size(), 2);
    const auto &extractedRegJson = blkJson["registers"][0];
    QCOMPARE(extractedRegJson["name"].get<std::string>(), std::string("FIFO_REG"));
    QCOMPARE(extractedRegJson["hdl_path"].get<std::string>(), std::string("dut.subsys.fifo_reg_q"));
    QCOMPARE(extractedRegJson["no_bit_bash_test"].get<bool>(), true);
    QCOMPARE(extractedRegJson["no_reset_test"].get<bool>(), true);
    QCOMPARE(extractedRegJson["is_fifo"].get<bool>(), true);
    QCOMPARE(extractedRegJson["fifo_depth"].get<uint64_t>(), 16ULL);

    const auto &extractedIndJson = blkJson["registers"][1];
    QCOMPARE(extractedIndJson["name"].get<std::string>(), std::string("INDIRECT_DATA"));
    QCOMPARE(extractedIndJson["is_indirect"].get<bool>(), true);
    QCOMPARE(extractedIndJson["index_reg"].get<std::string>(), std::string("FIFO_REG"));
    QCOMPARE(extractedIndJson["has_callbacks"].get<bool>(), true);

    // Field verification
    QVERIFY(extractedRegJson.contains("fields"));
    QCOMPARE(extractedRegJson["fields"].size(), 1);
    const auto &extractedFld = extractedRegJson["fields"][0];
    QCOMPARE(extractedFld["name"].get<std::string>(), std::string("DATA"));
    QCOMPARE(extractedFld["access"].get<std::string>(), std::string("W1"));
    QCOMPARE(extractedFld["individually_accessible"].get<int>(), 0);

    // Memory verification
    QVERIFY(blkJson.contains("memories"));
    QCOMPARE(blkJson["memories"].size(), 1);
    const auto &extractedMem = blkJson["memories"][0];
    QCOMPARE(extractedMem["name"].get<std::string>(), std::string("PACKET_RAM"));
    QCOMPARE(extractedMem["depth"].get<uint64_t>(), 1024ULL);
    QCOMPARE(extractedMem["word_width"].get<uint64_t>(), 32ULL);
    QCOMPARE(extractedMem["hdl_path"].get<std::string>(), std::string("dut.subsys.packet_ram_mem"));
    QCOMPARE(extractedMem["no_walk_test"].get<bool>(), true);
    QCOMPARE(extractedMem["no_access_test"].get<bool>(), true);

    // Also verify NO_REG_TEST and NO_MEM_TEST omnibus flags
    QVariantMap allDisableReg;
    allDisableReg["Name"] = "DIS_REG";
    allDisableReg["Offset/LSB"] = "0x04";
    allDisableReg["Size/Width"] = "32";
    allDisableReg["NO_REG_TEST"] = "true";
    RegMapTreeItem *disRegItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, allDisableReg, blk);
    blk->appendChild(disRegItem);

    QVariantMap allDisableMem;
    allDisableMem["Name"] = "DIS_MEM";
    allDisableMem["Offset/LSB"] = "0x4000";
    allDisableMem["Size/Width"] = "1024";
    allDisableMem["NO_MEM_TEST"] = "true";
    RegMapTreeItem *disMemItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::mem, allDisableMem, blk);
    blk->appendChild(disMemItem);

    nlohmann::json rootDisJson = model.extractJsonData(32);
    const auto &disBlk = rootDisJson["blocks"][0];
    bool foundDisReg = false;
    for (const auto &r : disBlk["registers"]) {
        if (r["name"] == "DIS_REG") {
            foundDisReg = true;
            QCOMPARE(r["no_reg_test"].get<bool>(), true);
            QCOMPARE(r["no_reset_test"].get<bool>(), true);
            QCOMPARE(r["no_bit_bash_test"].get<bool>(), true);
            QCOMPARE(r["no_access_test"].get<bool>(), true);
        }
    }
    QVERIFY(foundDisReg);

    bool foundDisMem = false;
    for (const auto &m : disBlk["memories"]) {
        if (m["name"] == "DIS_MEM") {
            foundDisMem = true;
            QCOMPARE(m["no_mem_test"].get<bool>(), true);
            QCOMPARE(m["no_walk_test"].get<bool>(), true);
            QCOMPARE(m["no_access_test"].get<bool>(), true);
        }
    }
    QVERIFY(foundDisMem);
}

QTEST_MAIN(TestRegMapTreeModel)
#include "test_RegMapTreeModel.moc"
