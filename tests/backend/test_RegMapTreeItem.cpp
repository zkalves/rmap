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
    QVariantMap emptyData;
    RegMapTreeItem rootItem(RegMapTreeItem::e_rmmKind::root, emptyData);
    QCOMPARE(rootItem.kindString(), QString("root"));

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
    QCOMPARE(item.data("Name").toString(), QString("CTRL"));
    QCOMPARE(item.data("Offset/LSB").toString(), QString("0x10"));
    QCOMPARE(item.data("Size/Width").toString(), QString("32"));
    QCOMPARE(item.data("Access Policy").toString(), QString("RW"));
    QCOMPARE(item.data("Reset Value").toString(), QString("0x0"));
    QCOMPARE(item.data("Is Rand").toString(), QString("true"));
    QCOMPARE(item.data("Volatile").toString(), QString("false"));
    QCOMPARE(item.data("Has Reset").toString(), QString("true"));
    QCOMPARE(item.data("Description").toString(), QString("Control Register"));

    QVERIFY(!item.data("NonExistentColumn").isValid());
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

    QCOMPARE(serialData["kind"].value<RegMapTreeItem::e_rmmKind>(), RegMapTreeItem::e_rmmKind::root);
    QVERIFY(serialData.contains("id"));
    QVERIFY(serialData.contains("childItems"));

    delete root;
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
    QCOMPARE(blk->child(1)->data("Name").toString(), QString("Reg2"));

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
    QVERIFY(iconRoot.isEmpty());
    QVERIFY(!iconBlk.isEmpty());
    QVERIFY(!iconReg.isEmpty());
    QVERIFY(!iconFld.isEmpty());
}

QTEST_MAIN(TestRegMapTreeItem)
#include "test_RegMapTreeItem.moc"
