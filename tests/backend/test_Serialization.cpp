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
};

void TestSerialization::testParseSpiRmt()
{
    int fd = open("examples/spi.rmt", O_RDONLY, S_IRUSR | S_IWUSR);
    QVERIFY2(fd >= 0, "Failed to open examples/spi.rmt");

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
    QCOMPARE(reg_model.config().template_outputs_size(), 2);
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
    int fd = open("examples/comprehensive.rmt", O_RDONLY, S_IRUSR | S_IWUSR);
    QVERIFY2(fd >= 0, "Failed to open examples/comprehensive.rmt");

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
    int fd = open("examples/wide_bus_64bit.rmt", O_RDONLY, S_IRUSR | S_IWUSR);
    QVERIFY2(fd >= 0, "Failed to open examples/wide_bus_64bit.rmt");

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
    int fd = open("examples/uart.rmt", O_RDONLY, S_IRUSR | S_IWUSR);
    QVERIFY2(fd >= 0, "Failed to open examples/uart.rmt");

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
    int fd = open("examples/dma.rmt", O_RDONLY, S_IRUSR | S_IWUSR);
    QVERIFY2(fd >= 0, "Failed to open examples/dma.rmt");

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
    int fd = open("examples/sensor_hub.rmt", O_RDONLY, S_IRUSR | S_IWUSR);
    QVERIFY2(fd >= 0, "Failed to open examples/sensor_hub.rmt");

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

QTEST_MAIN(TestSerialization)
#include "test_Serialization.moc"
