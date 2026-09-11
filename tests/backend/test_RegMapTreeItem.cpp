/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include <QtTest>
#include "RegMapTreeItem.hpp"
#include "SerializationContext.hpp"

class TestRegMapTreeItem : public QObject
{
    Q_OBJECT

private slots:
    void testNodeCreationAndKinds();
    void testHierarchyAndRelationships();
    void testDataRetrieval();
    void testSerializationRoundtrip();
    void testPossibleChildren();
    void testItemMutationAndChildRemoval();
    void testIconsAndKindString();
};

void TestRegMapTreeItem::testNodeCreationAndKinds()
{
    // Default constructor
    RegMapTreeItem defaultItem;
    QCOMPARE(defaultItem.kindString(), QString("root"));
    QCOMPARE(defaultItem.parentItem(), nullptr);
    QCOMPARE(defaultItem.row(), 0);
    QCOMPARE(defaultItem.columnCount(), 0);

    QVariantMap emptyData;
    RegMapTreeItem rootItem(RegMapTreeItem::e_rmmKind::root, emptyData);
    QCOMPARE(rootItem.kindString(), QString("root"));
    QCOMPARE(rootItem.columnCount(), 0);

    RegMapTreeItem blkItem(RegMapTreeItem::e_rmmKind::blk, emptyData);
    QCOMPARE(blkItem.kindString(), QString("blk"));

    RegMapTreeItem regItem(RegMapTreeItem::e_rmmKind::reg, emptyData);
    QCOMPARE(regItem.kindString(), QString("reg"));

    RegMapTreeItem fldItem(RegMapTreeItem::e_rmmKind::fld, emptyData);
    QCOMPARE(fldItem.kindString(), QString("fld"));

    RegMapTreeItem memItem(RegMapTreeItem::e_rmmKind::mem, emptyData);
    QCOMPARE(memItem.kindString(), QString("mem"));

    RegMapTreeItem mapItem(RegMapTreeItem::e_rmmKind::map, emptyData);
    QCOMPARE(mapItem.kindString(), QString("map"));

    // Invalid/custom kind for switch default branch
    RegMapTreeItem customItem(static_cast<RegMapTreeItem::e_rmmKind>(99), emptyData);
    QCOMPARE(customItem.kindString(), QString(""));
    QCOMPARE(customItem.icon(), QString(""));
    QVERIFY(customItem.possibleChildren().isEmpty());
}

void TestRegMapTreeItem::testHierarchyAndRelationships()
{
    QVariantMap dataRoot;
    dataRoot["Name"] = "RootNode";
    auto *root = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::root, dataRoot);

    QVariantMap dataBlk;
    dataBlk["Name"] = "Block0";
    auto *blk = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::blk, dataBlk, root);
    root->appendChild(blk);

    QVariantMap dataReg1;
    dataReg1["Name"] = "Reg0";
    auto *reg1 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, dataReg1, blk);
    blk->appendChild(reg1);

    QVariantMap dataReg2;
    dataReg2["Name"] = "Reg1";
    auto *reg2 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, dataReg2, blk);
    blk->appendChild(reg2);

    QCOMPARE(root->childCount(), 1);
    QCOMPARE(root->child(0), blk);
    QCOMPARE(root->child(99), nullptr);

    QCOMPARE(blk->childCount(), 2);
    QCOMPARE(blk->child(0), reg1);
    QCOMPARE(blk->child(1), reg2);
    QCOMPARE(blk->parentItem(), root);

    QCOMPARE(reg1->row(), 0);
    QCOMPARE(reg2->row(), 1);
    QCOMPARE(reg1->parentItem(), blk);
    QCOMPARE(reg2->parentItem(), blk);

    QVector<RegMapTreeItem*> blkChildren = blk->getChildItems();
    QCOMPARE(blkChildren.size(), 2);
    QCOMPARE(blkChildren[0], reg1);
    QCOMPARE(blkChildren[1], reg2);
    QCOMPARE(blk->childItems().size(), 2);
    QCOMPARE(blk->childItemsRef().size(), 2);
    QCOMPARE(root->row(), 0);
    QCOMPARE(root->child(-1), nullptr);

    delete root; // Automatically deletes children recursively
}

void TestRegMapTreeItem::testDataRetrieval()
{
    QVariantMap data;
    data["Name"] = "CTRL";
    data["Offset/LSB"] = "0x10";
    data["Size/Width"] = "32";
    data["Access Policy"] = "RW";
    data["Reset Value"] = "0x0";
    data["Is Rand"] = "true";
    data["Volatile"] = "false";
    data["Has Reset"] = "true";
    data["Description"] = "Control Register";

    RegMapTreeItem item(RegMapTreeItem::e_rmmKind::reg, data);
    QCOMPARE(item.columnCount(), 9);
    QCOMPARE(item.data("Name").toString(), QString("CTRL"));
    QCOMPARE(item.data("Offset/LSB").toString(), QString("0x10"));
    // Fallback from Offset/LSB -> Offset and LSB
    QCOMPARE(item.data("Offset").toString(), QString("0x10"));
    QCOMPARE(item.data("LSB").toString(), QString("0x10"));

    QCOMPARE(item.data("Size/Width").toString(), QString("32"));
    // Fallback from Size/Width -> Size and Width
    QCOMPARE(item.data("Size").toString(), QString("32"));
    QCOMPARE(item.data("Width").toString(), QString("32"));

    QCOMPARE(item.data("Access Policy").toString(), QString("RW"));
    QCOMPARE(item.data("Reset Value").toString(), QString("0x0"));
    QCOMPARE(item.data("Is Rand").toString(), QString("true"));
    QCOMPARE(item.data("Volatile").toString(), QString("false"));
    QCOMPARE(item.data("Has Reset").toString(), QString("true"));
    QCOMPARE(item.data("Description").toString(), QString("Control Register"));

    QVERIFY(!item.data("NonExistentColumn").isValid());

    // Test reverse fallbacks (Offset -> Offset/LSB, Size -> Size/Width)
    QVariantMap revData;
    revData["Offset"] = "0x20";
    revData["Size"] = "16";
    RegMapTreeItem revItem(RegMapTreeItem::e_rmmKind::reg, revData);
    QCOMPARE(revItem.data("Offset/LSB").toString(), QString("0x20"));
    QCOMPARE(revItem.data("Size/Width").toString(), QString("16"));

    // Test setData synchronization for Offset/LSB and Size/Width
    revItem.setData("LSB", "0x30");
    QCOMPARE(revItem.data("Offset").toString(), QString("0x30"));
    QCOMPARE(revItem.data("Offset/LSB").toString(), QString("0x30"));

    revItem.setData("Width", "8");
    QCOMPARE(revItem.data("Size").toString(), QString("8"));
    QCOMPARE(revItem.data("Size/Width").toString(), QString("8"));
}

void TestRegMapTreeItem::testSerializationRoundtrip()
{
    SerializationContext context;
    QVariantMap rootData;
    rootData["Name"] = "Top";
    auto *root = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::root, rootData);

    QVariantMap regData;
    regData["Name"] = "STATUS";
    regData["Offset/LSB"] = "0x04";
    auto *reg = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, regData, root);
    root->appendChild(reg);

    QVariantMap serialData;
    root->serialize(serialData, &context);

    QCOMPARE(static_cast<int>(serialData["kind"].value<RegMapTreeItem::e_rmmKind>()), static_cast<int>(RegMapTreeItem::e_rmmKind::root));
    QVERIFY(serialData.contains("id"));
    QVERIFY(serialData.contains("childItems"));

    delete root;

    // Test deserialize on a standalone item with invalid/out-of-bounds children
    QVariantMap dummyData;
    dummyData["kind"] = QVariant::fromValue(RegMapTreeItem::e_rmmKind::blk);
    dummyData["parent"] = QVariant();
    dummyData["childItems"] = QList<QVariant>{ QVariant(), QVariant(-1), QVariant(99999) };
    QVariantMap itemData;
    itemData["Name"] = "DESER_BLK";
    dummyData["itemData"] = itemData;

    RegMapTreeItem deserItem;
    deserItem.deserialize(dummyData, &context);
    QCOMPARE(static_cast<int>(deserItem.kind()), static_cast<int>(RegMapTreeItem::e_rmmKind::blk));
    QCOMPARE(deserItem.data("Name").toString(), QString("DESER_BLK"));

    // Full hierarchy deserialize with parent and children
    SerializationContext deserCtx;
    QVariantMap rootRecord;
    rootRecord["kind"] = QVariant::fromValue(RegMapTreeItem::e_rmmKind::root);
    rootRecord["id"] = 0;
    rootRecord["parent"] = QVariant();
    rootRecord["childItems"] = QList<QVariant>{1};
    QVariantMap rootItemData;
    rootItemData["Name"] = "ROOT";
    rootRecord["itemData"] = rootItemData;

    QVariantMap regRecord;
    regRecord["kind"] = QVariant::fromValue(RegMapTreeItem::e_rmmKind::reg);
    regRecord["id"] = 1;
    regRecord["parent"] = 0;
    regRecord["childItems"] = QList<QVariant>();
    QVariantMap regItemData;
    regItemData["Name"] = "REG0";
    regRecord["itemData"] = regItemData;

    deserCtx.append_record<RegMapTreeItem>(nullptr, rootRecord);
    deserCtx.append_record<RegMapTreeItem>(nullptr, regRecord);

    // Test invalid and out-of-range handles (covers L81, 85, 86)
    QVERIFY(deserCtx.deserialize<RegMapTreeItem>(QVariant()) == nullptr);
    QVERIFY(deserCtx.deserialize<RegMapTreeItem>(QVariant(-1)) == nullptr);
    QVERIFY(deserCtx.deserialize<RegMapTreeItem>(QVariant(9999)) == nullptr);

    // Deserialize root -> automatically deserializes reg child too (covers L88-105)
    RegMapTreeItem* deserRoot = deserCtx.deserialize<RegMapTreeItem>(QVariant(0));
    QVERIFY(deserRoot != nullptr);
    QCOMPARE(deserRoot->childCount(), 1);
    QCOMPARE(deserRoot->child(0)->data("Name").toString(), QString("REG0"));

    // Calling deserialize again on already-deserialized object returns cached pointer (covers L90-91)
    RegMapTreeItem* reRoot = deserCtx.deserialize<RegMapTreeItem>(QVariant(0));
    QCOMPARE(reRoot, deserRoot);

    delete deserRoot;
}

void TestRegMapTreeItem::testPossibleChildren()
{
    QVariantMap empty;
    RegMapTreeItem root(RegMapTreeItem::e_rmmKind::root, empty);
    auto rootChildren = root.possibleChildren();
    QVERIFY(rootChildren.contains(RegMapTreeItem::e_rmmKind::blk));
    QVERIFY(rootChildren.contains(RegMapTreeItem::e_rmmKind::mem));

    RegMapTreeItem blk(RegMapTreeItem::e_rmmKind::blk, empty);
    auto blkChildren = blk.possibleChildren();
    QVERIFY(blkChildren.contains(RegMapTreeItem::e_rmmKind::reg));
    QVERIFY(blkChildren.contains(RegMapTreeItem::e_rmmKind::blk));

    RegMapTreeItem reg(RegMapTreeItem::e_rmmKind::reg, empty);
    auto regChildren = reg.possibleChildren();
    QVERIFY(regChildren.contains(RegMapTreeItem::e_rmmKind::fld));

    RegMapTreeItem fld(RegMapTreeItem::e_rmmKind::fld, empty);
    auto fldChildren = fld.possibleChildren();
    QVERIFY(fldChildren.isEmpty());
}

void TestRegMapTreeItem::testItemMutationAndChildRemoval()
{
    QVariantMap rootData;
    rootData["Name"] = "Root";
    auto *root = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::root, rootData);

    QVariantMap blkData;
    blkData["Name"] = "BlockA";
    auto *blk = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::blk, blkData, root);
    root->appendChild(blk);

    QVariantMap reg1Data;
    reg1Data["Name"] = "Reg1";
    auto *reg1 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, reg1Data, blk);
    blk->appendChild(reg1);

    QVariantMap reg2Data;
    reg2Data["Name"] = "Reg2";
    auto *reg2 = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, reg2Data, blk);
    blk->appendChild(reg2);

    QCOMPARE(blk->childCount(), 2);

    // Test setData
    reg1->setData("Name", "Reg1_Renamed");
    QCOMPARE(reg1->data("Name").toString(), QString("Reg1_Renamed"));

    // Test removeChildren
    bool removed = blk->removeChildren(0, 1);
    QVERIFY(removed);
    QCOMPARE(blk->childCount(), 1);
    QCOMPARE(blk->child(0)->data("Name").toString(), QString("Reg2"));

    // Test insertChildren
    QVector<QString> cols = {"Type", "Offset/LSB", "Size/Width", "Name", "Access Policy", "Reset Value", "Is Rand", "Volatile", "Has Reset", "Description"};
    bool inserted = blk->insertChildren(RegMapTreeItem::e_rmmKind::reg, 0, 1, cols);
    QVERIFY(inserted);
    QCOMPARE(blk->childCount(), 2);
    blk->child(0)->setData("Name", "Reg0_Inserted");
    QCOMPARE(blk->child(0)->data("Name").toString(), QString("Reg0_Inserted"));
    // Test invalid insertChildren (out-of-bounds)
    QVERIFY(!blk->insertChildren(RegMapTreeItem::e_rmmKind::reg, -1, 1, cols));
    QVERIFY(!blk->insertChildren(RegMapTreeItem::e_rmmKind::reg, 100, 1, cols));

    // Test invalid removeChildren (out-of-bounds)
    QVERIFY(!blk->removeChildren(-1, 1));
    QVERIFY(!blk->removeChildren(0, 100));

    delete root;
}

void TestRegMapTreeItem::testIconsAndKindString()
{
    QVariantMap empty;
    RegMapTreeItem root(RegMapTreeItem::e_rmmKind::root, empty);
    RegMapTreeItem blk(RegMapTreeItem::e_rmmKind::blk, empty);
    RegMapTreeItem reg(RegMapTreeItem::e_rmmKind::reg, empty);
    RegMapTreeItem fld(RegMapTreeItem::e_rmmKind::fld, empty);
    RegMapTreeItem mem(RegMapTreeItem::e_rmmKind::mem, empty);
    RegMapTreeItem map(RegMapTreeItem::e_rmmKind::map, empty);

    QCOMPARE(root.kindString(), QString("root"));
    QCOMPARE(blk.kindString(), QString("blk"));
    QCOMPARE(reg.kindString(), QString("reg"));
    QCOMPARE(fld.kindString(), QString("fld"));
    QCOMPARE(mem.kindString(), QString("mem"));
    QCOMPARE(map.kindString(), QString("map"));

    // Icons
    QString iconRoot = root.icon();
    QString iconBlk = blk.icon();
    QString iconReg = reg.icon();
    QString iconFld = fld.icon();
    QString iconMem = mem.icon();
    QString iconMap = map.icon();
    QVERIFY(iconRoot.isEmpty());
    QVERIFY(!iconBlk.isEmpty());
    QVERIFY(!iconReg.isEmpty());
    QVERIFY(!iconFld.isEmpty());
    QVERIFY(!iconMem.isEmpty());
    QVERIFY(!iconMap.isEmpty());
}

QTEST_MAIN(TestRegMapTreeItem)
#include "test_RegMapTreeItem.moc"
