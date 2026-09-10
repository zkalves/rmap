/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include <QtTest>
#include <fcntl.h>
#include <sys/stat.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "rmap.pb.h"
#include "SerializationContext.hpp"
#include "RegMapTreeItem.hpp"
#include "RegMapTreeModel.hpp"
#include "ProtobufLogCollector.hpp"

// Serialization helper operators from RegMapWindow
protormap::RegModel& operator <<( protormap::RegModel& reg_model, const SerializationContext& context );
protormap::RegModel& operator >>( protormap::RegModel& reg_model, SerializationContext& context );

class TestSerialization : public QObject
{
    Q_OBJECT

private slots:
    void testParseSpiRmt();
    void testParseComprehensiveRmt();
    void testParseWideBus64Rmt();
    void testParseUartRmt();
    void testParseDmaRmt();
    void testParseSensorHubRmt();
    void testRoundTripTextAndBinary();
    void testConfigTemplateFoldersAndEnabled();
    void testConfigPythonScriptEnabled();
    void testParseInvalidRmt();
    void testSerializationContextEdgeCases();
};

void TestSerialization::testParseSpiRmt()
{
    int fd = open("examples/rmt/peripherals/spi.rmt", O_RDONLY, S_IRUSR | S_IWUSR);
    QVERIFY2(fd >= 0, "Failed to open examples/rmt/peripherals/spi.rmt");

    protormap::RegModel reg_model;
    google::protobuf::io::FileInputStream fileInput(fd);
    fileInput.SetCloseOnDelete(true);

    google::protobuf::TextFormat::Parser parser;
    ProtobufLogCollector errorCollector;
    parser.RecordErrorsTo(&errorCollector);

    bool success = parser.Parse(&fileInput, &reg_model);
    QVERIFY2(success, errorCollector.get_string().c_str());

    QVERIFY(reg_model.has_config());
    QCOMPARE(reg_model.config().reg_width(), (uint32_t)32);
    QCOMPARE(reg_model.config().template_outputs_size(), 19);
    QVERIFY(reg_model.item_size() > 0);

    SerializationContext context;
    reg_model >> context;
    RegMapTreeItem* root = context.deserialize<RegMapTreeItem>(QVariant::fromValue<int>(0));
    QVERIFY(root != nullptr);
    QVERIFY(root->childCount() > 0);

    delete root;
}

void TestSerialization::testParseComprehensiveRmt()
{
    int fd = open("examples/rmt/features/comprehensive.rmt", O_RDONLY, S_IRUSR | S_IWUSR);
    QVERIFY2(fd >= 0, "Failed to open examples/rmt/features/comprehensive.rmt");

    protormap::RegModel reg_model;
    google::protobuf::io::FileInputStream fileInput(fd);
    fileInput.SetCloseOnDelete(true);

    google::protobuf::TextFormat::Parser parser;
    bool success = parser.Parse(&fileInput, &reg_model);
    QVERIFY(success);

    QCOMPARE(reg_model.config().reg_width(), (uint32_t)32);
    SerializationContext context;
    reg_model >> context;
    RegMapTreeItem* root = context.deserialize<RegMapTreeItem>(QVariant::fromValue<int>(0));
    QVERIFY(root != nullptr);

    // Verify block and memory exist
    QCOMPARE(root->childCount(), 2);
    delete root;
}

void TestSerialization::testParseWideBus64Rmt()
{
    int fd = open("examples/rmt/features/wide_bus_64bit.rmt", O_RDONLY, S_IRUSR | S_IWUSR);
    QVERIFY2(fd >= 0, "Failed to open examples/rmt/features/wide_bus_64bit.rmt");

    protormap::RegModel reg_model;
    google::protobuf::io::FileInputStream fileInput(fd);
    fileInput.SetCloseOnDelete(true);

    google::protobuf::TextFormat::Parser parser;
    bool success = parser.Parse(&fileInput, &reg_model);
    QVERIFY(success);

    QCOMPARE(reg_model.config().reg_width(), (uint32_t)64);

    SerializationContext context;
    reg_model >> context;
    RegMapTreeItem* root = context.deserialize<RegMapTreeItem>(QVariant::fromValue<int>(0));
    QVERIFY(root != nullptr);
    QCOMPARE(root->childCount(), 1);
    delete root;
}

void TestSerialization::testParseUartRmt()
{
    int fd = open("examples/rmt/peripherals/uart.rmt", O_RDONLY, S_IRUSR | S_IWUSR);
    QVERIFY2(fd >= 0, "Failed to open examples/rmt/peripherals/uart.rmt");

    protormap::RegModel reg_model;
    google::protobuf::io::FileInputStream fileInput(fd);
    fileInput.SetCloseOnDelete(true);

    google::protobuf::TextFormat::Parser parser;
    bool success = parser.Parse(&fileInput, &reg_model);
    QVERIFY(success);

    SerializationContext context;
    reg_model >> context;
    RegMapTreeItem* root = context.deserialize<RegMapTreeItem>(QVariant::fromValue<int>(0));
    QVERIFY(root != nullptr);
    QCOMPARE(root->childCount(), 1);
    QCOMPARE(root->child(0)->childCount(), 3); // CTRL, STATUS, DATA
    delete root;
}

void TestSerialization::testParseDmaRmt()
{
    int fd = open("examples/rmt/peripherals/dma.rmt", O_RDONLY, S_IRUSR | S_IWUSR);
    QVERIFY2(fd >= 0, "Failed to open examples/rmt/peripherals/dma.rmt");

    protormap::RegModel reg_model;
    google::protobuf::io::FileInputStream fileInput(fd);
    fileInput.SetCloseOnDelete(true);

    google::protobuf::TextFormat::Parser parser;
    bool success = parser.Parse(&fileInput, &reg_model);
    QVERIFY(success);

    SerializationContext context;
    reg_model >> context;
    RegMapTreeItem* root = context.deserialize<RegMapTreeItem>(QVariant::fromValue<int>(0));
    QVERIFY(root != nullptr);
    QCOMPARE(root->childCount(), 1);
    delete root;
}

void TestSerialization::testParseSensorHubRmt()
{
    int fd = open("examples/rmt/peripherals/sensor_hub.rmt", O_RDONLY, S_IRUSR | S_IWUSR);
    QVERIFY2(fd >= 0, "Failed to open examples/rmt/peripherals/sensor_hub.rmt");

    protormap::RegModel reg_model;
    google::protobuf::io::FileInputStream fileInput(fd);
    fileInput.SetCloseOnDelete(true);

    google::protobuf::TextFormat::Parser parser;
    bool success = parser.Parse(&fileInput, &reg_model);
    QVERIFY(success);

    SerializationContext context;
    reg_model >> context;
    RegMapTreeItem* root = context.deserialize<RegMapTreeItem>(QVariant::fromValue<int>(0));
    QVERIFY(root != nullptr);
    QCOMPARE(root->childCount(), 1);
    delete root;
}

void TestSerialization::testRoundTripTextAndBinary()
{
    // Build a model in memory
    RegMapTreeModel model;
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
    QModelIndex blkIndex = model.index(0, 0, QModelIndex());
    model.setData(model.index(0, 3, QModelIndex()), "ROUNDTRIP_BLK", Qt::EditRole);

    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, blkIndex);
    QModelIndex regIndex = model.index(0, 0, blkIndex);
    model.setData(model.index(0, 1, blkIndex), "0x20", Qt::EditRole);
    model.setData(model.index(0, 3, blkIndex), "REG_RT", Qt::EditRole);

    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::fld, regIndex);
    model.setData(model.index(0, 1, regIndex), "0", Qt::EditRole);
    model.setData(model.index(0, 2, regIndex), "4", Qt::EditRole);
    model.setData(model.index(0, 3, regIndex), "FLD_RT", Qt::EditRole);
    model.setData(model.index(0, 4, regIndex), "W1C", Qt::EditRole);

    // Serialize to SerializationContext
    SerializationContext saveContext;
    RegMapTreeItem* root = model.getItem(QModelIndex());
    QVariantMap rootSerial;
    root->serialize(rootSerial, &saveContext);

    protormap::RegModel protoModel;
    protoModel << saveContext;

    // Convert to text format string
    std::string textOutput;
    google::protobuf::TextFormat::PrintToString(protoModel, &textOutput);
    QVERIFY(!textOutput.empty());

    // Parse back from text format
    protormap::RegModel restoredTextModel;
    QVERIFY(google::protobuf::TextFormat::ParseFromString(textOutput, &restoredTextModel));

    SerializationContext restoreContext;
    restoredTextModel >> restoreContext;
    RegMapTreeItem* restoredRoot = restoreContext.deserialize<RegMapTreeItem>(QVariant::fromValue<int>(0));
    QVERIFY(restoredRoot != nullptr);
    QCOMPARE(restoredRoot->childCount(), 1);

    RegMapTreeItem* restoredBlk = restoredRoot->child(0);
    QCOMPARE(restoredBlk->data("Name").toString(), QString("ROUNDTRIP_BLK"));
    QCOMPARE(restoredBlk->childCount(), 1);

    RegMapTreeItem* restoredReg = restoredBlk->child(0);
    QCOMPARE(restoredReg->data("Name").toString(), QString("REG_RT"));
    QCOMPARE(restoredReg->data("Offset/LSB").toString(), QString("0x0020"));
    QCOMPARE(restoredReg->childCount(), 1);

    RegMapTreeItem* restoredFld = restoredReg->child(0);
    QCOMPARE(restoredFld->data("Name").toString(), QString("FLD_RT"));
    QCOMPARE(restoredFld->data("Access Policy").toString(), QString("W1C"));

    delete restoredRoot;

    // Binary roundtrip
    std::string binaryOutput;
    protoModel.SerializeToString(&binaryOutput);
    QVERIFY(!binaryOutput.empty());

    protormap::RegModel restoredBinModel;
    QVERIFY(restoredBinModel.ParseFromString(binaryOutput));
    QCOMPARE(restoredBinModel.item_size(), protoModel.item_size());
}

void TestSerialization::testConfigTemplateFoldersAndEnabled()
{
    protormap::Config cfg;
    cfg.set_reg_width(32);
    cfg.add_template_folders("templates/c");
    cfg.add_template_folders("templates/rtl");
    cfg.add_template_folders("/custom/templates");

    auto *t1 = cfg.add_template_outputs();
    t1->set_template_filename("templates/c/reg_map.h.inja");
    t1->set_output_filepath("work/c/reg_map.h");
    t1->set_enabled(true);

    auto *t2 = cfg.add_template_outputs();
    t2->set_template_filename("templates/rtl/reg_map.sv.inja");
    t2->set_output_filepath("work/rtl/reg_map.sv");
    t2->set_enabled(false);

    // Text serialization round-trip
    std::string textOutput;
    google::protobuf::TextFormat::PrintToString(cfg, &textOutput);
    QVERIFY(!textOutput.empty());

    protormap::Config restoredCfg;
    QVERIFY(google::protobuf::TextFormat::ParseFromString(textOutput, &restoredCfg));
    QCOMPARE(restoredCfg.template_folders_size(), 3);
    QCOMPARE(QString::fromStdString(restoredCfg.template_folders(0)), QString("templates/c"));
    QCOMPARE(QString::fromStdString(restoredCfg.template_folders(1)), QString("templates/rtl"));
    QCOMPARE(QString::fromStdString(restoredCfg.template_folders(2)), QString("/custom/templates"));

    QCOMPARE(restoredCfg.template_outputs_size(), 2);
    QVERIFY(restoredCfg.template_outputs(0).has_enabled());
    QCOMPARE(restoredCfg.template_outputs(0).enabled(), true);
    QVERIFY(restoredCfg.template_outputs(1).has_enabled());
    QCOMPARE(restoredCfg.template_outputs(1).enabled(), false);
}

void TestSerialization::testConfigPythonScriptEnabled()
{
    protormap::Config cfg;
    cfg.set_pythonscript("./scripts/post_gen.py");
    cfg.set_python_script_enabled(true);

    std::string textOutput;
    google::protobuf::TextFormat::PrintToString(cfg, &textOutput);
    QVERIFY(!textOutput.empty());

    protormap::Config restoredCfg;
    QVERIFY(google::protobuf::TextFormat::ParseFromString(textOutput, &restoredCfg));
    QCOMPARE(QString::fromStdString(restoredCfg.pythonscript()), QString("./scripts/post_gen.py"));
    QVERIFY(restoredCfg.has_python_script_enabled());
    QCOMPARE(restoredCfg.python_script_enabled(), true);

    // Also test explicitly disabled
    cfg.set_python_script_enabled(false);
    google::protobuf::TextFormat::PrintToString(cfg, &textOutput);
    protormap::Config restoredCfg2;
    QVERIFY(google::protobuf::TextFormat::ParseFromString(textOutput, &restoredCfg2));
    QVERIFY(restoredCfg2.has_python_script_enabled());
    QCOMPARE(restoredCfg2.python_script_enabled(), false);
}

void TestSerialization::testParseInvalidRmt()
{
    std::string invalid_proto = "this is totally invalid protobuf { [[[ invalid";
    google::protobuf::io::ArrayInputStream input(invalid_proto.data(), static_cast<int>(invalid_proto.size()));
    protormap::RegModel reg_model;
    google::protobuf::TextFormat::Parser parser;
    ProtobufLogCollector errorCollector;
    parser.RecordErrorsTo(&errorCollector);
    bool success = parser.Parse(&input, &reg_model);
    QVERIFY(!success);
    QVERIFY(!errorCollector.string().empty());
    QVERIFY(errorCollector.string().find("ERROR") != std::string::npos);
}

void TestSerialization::testSerializationContextEdgeCases()
{
    SerializationContext context;
    // Invalid handle
    QVERIFY(context.deserialize<RegMapTreeItem>(QVariant()) == nullptr);
    // Out-of-bounds indices (triggers line 86 of SerializationContext.hpp)
    QVERIFY(context.deserialize<RegMapTreeItem>(QVariant(-1)) == nullptr);
    QVERIFY(context.deserialize<RegMapTreeItem>(QVariant(99999)) == nullptr);

    // Nullptr serialize
    QVERIFY(!context.serialize<RegMapTreeItem>(nullptr).isValid());

    // Valid object serialize and repeat serialize
    RegMapTreeItem item;
    QVariant h1 = context.serialize(&item);
    QVERIFY(h1.isValid());
    QVariant h2 = context.serialize(&item);
    QCOMPARE(h1, h2);

    // Repeat deserialize
    RegMapTreeItem *d1 = context.deserialize<RegMapTreeItem>(h1);
    QCOMPARE(d1, &item);
    RegMapTreeItem *d2 = context.deserialize<RegMapTreeItem>(h1);
    QCOMPARE(d2, &item);

    // append_record
    QVariantMap recordMap;
    recordMap["Name"] = "Dummy";
    context.append_record(&item, recordMap);

    // clear
    context.clear();
    QVERIFY(context.deserialize<RegMapTreeItem>(h1) == nullptr);
}

QTEST_MAIN(TestSerialization)
#include "test_Serialization.moc"
