#include <QtTest>
#include <QDir>
#include "RegMapTreeModel.hpp"
#include "RegConfigWindow.hpp"
#include "format/FormatManager.hpp"

class TestFormats : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void test_SystemRdlRead();
    void test_SystemRdlWriteAndRoundtrip();
    void test_IpxactWriteAndRoundtrip();
    void test_JsonWriteAndRoundtrip();
    void test_CsvWriteAndRoundtrip();
    void test_CmsisSvdWriteAndRoundtrip();
    void test_CrossFormatConversion();
    void test_CmsisSvdRealWorldFile();
    void test_IpxactRealWorldFile();
    void test_JsonRealWorldFile();
    void test_CsvRealWorldFile();
    void test_ProtobufBinaryRoundtrip();
    void test_ComprehensiveMapRoundtrip();
    void test_WideBus64BitMapRoundtrip();
    void test_InvalidFileHandling();
};

void TestFormats::initTestCase()
{
    QDir().mkpath("work/test_formats");
}

void TestFormats::cleanupTestCase()
{
    QDir("work/test_formats").removeRecursively();
}

void TestFormats::test_SystemRdlRead()
{
    RegMapTreeModel model;
    RegConfigWindow config;

    // Test reading atxmega_spi.rdl
    FormatResult res = FormatManager::instance().loadFile("example/systemRdl/atxmega_spi.rdl", &model, &config);
    QVERIFY2(res.success, qPrintable(res.errorMessage));

    RegMapTreeItem *root = model.getRootItem();
    QVERIFY(root != nullptr);
    QCOMPARE(root->childCount(), 1); // 1 top block

    RegMapTreeItem *blk = root->child(0);
    QVERIFY(blk != nullptr);
    QCOMPARE(blk->childCount(), 4); // CTRL, INTCTRL, STATUS, DATA

    // Check CTRL register
    RegMapTreeItem *ctrl = blk->child(0);
    QCOMPARE(ctrl->data("Name").toString(), QString("CTRL"));
    QCOMPARE(ctrl->data("Offset/LSB").toString(), QString("0x0"));
    QCOMPARE(ctrl->childCount(), 6); // PRESCALER, MODE, MASTER, DORD, ENABLE, CLK2X

    // Check PRESCALER field
    RegMapTreeItem *prescaler = ctrl->child(0);
    QCOMPARE(prescaler->data("Name").toString(), QString("PRESCALER"));
    QCOMPARE(prescaler->data("Offset/LSB").toString(), QString("0"));
    QCOMPARE(prescaler->data("Size/Width").toString(), QString("2"));

    // Check DATA register
    RegMapTreeItem *dataReg = blk->child(3);
    QCOMPARE(dataReg->data("Name").toString(), QString("DATA"));
    QCOMPARE(dataReg->data("Offset/LSB").toString(), QString("0x3"));
    QCOMPARE(dataReg->childCount(), 2); // WDATA, RDATA
}

void TestFormats::test_SystemRdlWriteAndRoundtrip()
{
    RegMapTreeModel model1;
    RegConfigWindow config1;

    FormatResult res1 = FormatManager::instance().loadFile("example/systemRdl/atxmega_spi.rdl", &model1, &config1);
    QVERIFY(res1.success);

    // Save as new RDL
    QString outRdl = "work/test_formats/roundtrip.rdl";
    FormatResult resSave = FormatManager::instance().saveFile(outRdl, &model1, &config1);
    QVERIFY2(resSave.success, qPrintable(resSave.errorMessage));

    // Re-load
    RegMapTreeModel model2;
    RegConfigWindow config2;
    FormatResult res2 = FormatManager::instance().loadFile(outRdl, &model2, &config2);
    QVERIFY2(res2.success, qPrintable(res2.errorMessage));

    RegMapTreeItem *root2 = model2.getRootItem();
    QVERIFY(root2 != nullptr);
    QCOMPARE(root2->childCount(), 1);
    RegMapTreeItem *blk2 = root2->child(0);
    QCOMPARE(blk2->childCount(), 4);
    QCOMPARE(blk2->child(0)->data("Name").toString(), QString("CTRL"));
}

void TestFormats::test_IpxactWriteAndRoundtrip()
{
    RegMapTreeModel model1;
    RegConfigWindow config1;

    // Load standard SPI protobuf map
    FormatResult res1 = FormatManager::instance().loadFile("example/spi.rmt", &model1, &config1);
    QVERIFY(res1.success);

    // Save as IP-XACT XML
    QString outXml = "work/test_formats/spi.xml";
    FormatResult resSave = FormatManager::instance().saveFile(outXml, &model1, &config1);
    QVERIFY2(resSave.success, qPrintable(resSave.errorMessage));

    // Re-load IP-XACT XML
    RegMapTreeModel model2;
    RegConfigWindow config2;
    FormatResult res2 = FormatManager::instance().loadFile(outXml, &model2, &config2);
    QVERIFY2(res2.success, qPrintable(res2.errorMessage));

    RegMapTreeItem *root2 = model2.getRootItem();
    QVERIFY(root2 != nullptr);
    QVERIFY(root2->childCount() >= 1);
    RegMapTreeItem *blk2 = root2->child(0);
    QCOMPARE(blk2->childCount(), 2); // CTRL, STATUS
}

void TestFormats::test_JsonWriteAndRoundtrip()
{
    RegMapTreeModel model1;
    RegConfigWindow config1;

    FormatResult res1 = FormatManager::instance().loadFile("example/spi.rmt", &model1, &config1);
    QVERIFY(res1.success);

    // Save as JSON
    QString outJson = "work/test_formats/spi.json";
    FormatResult resSave = FormatManager::instance().saveFile(outJson, &model1, &config1);
    QVERIFY2(resSave.success, qPrintable(resSave.errorMessage));

    // Re-load JSON
    RegMapTreeModel model2;
    RegConfigWindow config2;
    FormatResult res2 = FormatManager::instance().loadFile(outJson, &model2, &config2);
    QVERIFY2(res2.success, qPrintable(res2.errorMessage));

    RegMapTreeItem *root2 = model2.getRootItem();
    QVERIFY(root2 != nullptr);
    QCOMPARE(root2->childCount(), 1);
    RegMapTreeItem *blk2 = root2->child(0);
    QCOMPARE(blk2->childCount(), 2); // CTRL, STATUS
    QCOMPARE(blk2->child(0)->data("Name").toString(), QString("CTRL"));
}

void TestFormats::test_CsvWriteAndRoundtrip()
{
    RegMapTreeModel model1;
    RegConfigWindow config1;

    FormatResult res1 = FormatManager::instance().loadFile("example/spi.rmt", &model1, &config1);
    QVERIFY(res1.success);

    // Save as CSV
    QString outCsv = "work/test_formats/spi.csv";
    FormatResult resSave = FormatManager::instance().saveFile(outCsv, &model1, &config1);
    QVERIFY2(resSave.success, qPrintable(resSave.errorMessage));

    // Re-load CSV
    RegMapTreeModel model2;
    RegConfigWindow config2;
    FormatResult res2 = FormatManager::instance().loadFile(outCsv, &model2, &config2);
    QVERIFY2(res2.success, qPrintable(res2.errorMessage));

    RegMapTreeItem *root2 = model2.getRootItem();
    QVERIFY(root2 != nullptr);
    QCOMPARE(root2->childCount(), 1);
    RegMapTreeItem *blk2 = root2->child(0);
    QCOMPARE(blk2->childCount(), 2); // CTRL, STATUS
    QCOMPARE(blk2->child(0)->data("Name").toString(), QString("CTRL"));
    QCOMPARE(blk2->child(0)->childCount(), 2); // Fields inside CTRL (EN, MODE)
}

void TestFormats::test_CmsisSvdWriteAndRoundtrip()
{
    RegMapTreeModel model1;
    RegConfigWindow config1;

    FormatResult res1 = FormatManager::instance().loadFile("example/spi.rmt", &model1, &config1);
    QVERIFY(res1.success);

    // Save as CMSIS-SVD (.svd)
    QString outSvd = "work/test_formats/spi.svd";
    FormatResult resSave = FormatManager::instance().saveFile(outSvd, &model1, &config1);
    QVERIFY2(resSave.success, qPrintable(resSave.errorMessage));

    // Re-load CMSIS-SVD (.svd)
    RegMapTreeModel model2;
    RegConfigWindow config2;
    FormatResult res2 = FormatManager::instance().loadFile(outSvd, &model2, &config2);
    QVERIFY2(res2.success, qPrintable(res2.errorMessage));

    RegMapTreeItem *root2 = model2.getRootItem();
    QVERIFY(root2 != nullptr);
    QCOMPARE(root2->childCount(), 1);
    RegMapTreeItem *blk2 = root2->child(0);
    QCOMPARE(blk2->childCount(), 2); // CTRL, STATUS
    QCOMPARE(blk2->child(0)->data("Name").toString(), QString("CTRL"));
    QCOMPARE(blk2->child(0)->childCount(), 2); // Fields inside CTRL
}

void TestFormats::test_CrossFormatConversion()
{
    // Hop 1: .rmt (Protobuf) -> .rdl (SystemRDL)
    RegMapTreeModel m1;
    RegConfigWindow c1;
    QVERIFY(FormatManager::instance().loadFile("example/spi.rmt", &m1, &c1).success);
    QVERIFY(FormatManager::instance().saveFile("work/test_formats/hop1.rdl", &m1, &c1).success);

    // Hop 2: .rdl (SystemRDL) -> .xml (IP-XACT)
    RegMapTreeModel m2;
    RegConfigWindow c2;
    QVERIFY(FormatManager::instance().loadFile("work/test_formats/hop1.rdl", &m2, &c2).success);
    QVERIFY(FormatManager::instance().saveFile("work/test_formats/hop2.xml", &m2, &c2).success);

    // Hop 3: .xml (IP-XACT) -> .svd (ARM CMSIS-SVD)
    RegMapTreeModel m3;
    RegConfigWindow c3;
    QVERIFY(FormatManager::instance().loadFile("work/test_formats/hop2.xml", &m3, &c3).success);
    QVERIFY(FormatManager::instance().saveFile("work/test_formats/hop3.svd", &m3, &c3).success);

    // Hop 4: .svd (ARM CMSIS-SVD) -> .json (JSON)
    RegMapTreeModel m4;
    RegConfigWindow c4;
    QVERIFY(FormatManager::instance().loadFile("work/test_formats/hop3.svd", &m4, &c4).success);
    QVERIFY(FormatManager::instance().saveFile("work/test_formats/hop4.json", &m4, &c4).success);

    // Hop 5: .json (JSON) -> .csv (CSV)
    RegMapTreeModel m5;
    RegConfigWindow c5;
    QVERIFY(FormatManager::instance().loadFile("work/test_formats/hop4.json", &m5, &c5).success);
    QVERIFY(FormatManager::instance().saveFile("work/test_formats/hop5.csv", &m5, &c5).success);

    // Verify final loaded model from CSV
    RegMapTreeModel mFinal;
    RegConfigWindow cFinal;
    QVERIFY(FormatManager::instance().loadFile("work/test_formats/hop5.csv", &mFinal, &cFinal).success);
    RegMapTreeItem *rootFinal = mFinal.getRootItem();
    QVERIFY(rootFinal != nullptr);
    QCOMPARE(rootFinal->childCount(), 1);
    QCOMPARE(rootFinal->child(0)->childCount(), 2); // CTRL, STATUS preserved through all 5 hops!
}

void TestFormats::test_CmsisSvdRealWorldFile()
{
    RegMapTreeModel model;
    RegConfigWindow config;
    FormatResult res = FormatManager::instance().loadFile("example/svd/stm32_uart.svd", &model, &config);
    QVERIFY2(res.success, qPrintable(res.errorMessage));

    RegMapTreeItem *root = model.getRootItem();
    QVERIFY(root != nullptr);
    QCOMPARE(root->childCount(), 1);

    RegMapTreeItem *uart = root->child(0);
    QCOMPARE(uart->data("Name").toString(), QString("UART0"));
    QCOMPARE(uart->childCount(), 3); // CTRL, STATUS, DATA

    RegMapTreeItem *ctrl = uart->child(0);
    QCOMPARE(ctrl->data("Name").toString(), QString("CTRL"));
    QCOMPARE(ctrl->childCount(), 5); // TX_EN, RX_EN, PARITY, STOP_BITS, BAUD_DIV
}

void TestFormats::test_IpxactRealWorldFile()
{
    RegMapTreeModel model;
    RegConfigWindow config;
    FormatResult res = FormatManager::instance().loadFile("example/ipxact/spi_ipxact.xml", &model, &config);
    QVERIFY2(res.success, qPrintable(res.errorMessage));

    RegMapTreeItem *root = model.getRootItem();
    QVERIFY(root != nullptr);
    QVERIFY(root->childCount() >= 1);

    RegMapTreeItem *blk = root->child(0);
    QCOMPARE(blk->childCount(), 2); // CTRL, STATUS
}

void TestFormats::test_JsonRealWorldFile()
{
    RegMapTreeModel model;
    RegConfigWindow config;
    FormatResult res = FormatManager::instance().loadFile("example/json/sensor_hub.json", &model, &config);
    QVERIFY2(res.success, qPrintable(res.errorMessage));

    RegMapTreeItem *root = model.getRootItem();
    QVERIFY(root != nullptr);
    QCOMPARE(root->childCount(), 1);

    RegMapTreeItem *blk = root->child(0);
    QCOMPARE(blk->data("Name").toString(), QString("SENSOR_CORE"));
    QCOMPARE(blk->childCount(), 3); // CONFIG, TEMPERATURE, PRESSURE
}

void TestFormats::test_CsvRealWorldFile()
{
    RegMapTreeModel model;
    RegConfigWindow config;
    FormatResult res = FormatManager::instance().loadFile("example/csv/dma_controller.csv", &model, &config);
    QVERIFY2(res.success, qPrintable(res.errorMessage));

    RegMapTreeItem *root = model.getRootItem();
    QVERIFY(root != nullptr);
    QCOMPARE(root->childCount(), 1);

    RegMapTreeItem *blk = root->child(0);
    QCOMPARE(blk->data("Name").toString(), QString("DMA_CONTROLLER"));
    QCOMPARE(blk->childCount(), 5); // DMA_CTRL, DMA_SRC_ADDR, DMA_DST_ADDR, DMA_LEN, DMA_STATUS
}

void TestFormats::test_ProtobufBinaryRoundtrip()
{
    RegMapTreeModel model1;
    RegConfigWindow config1;
    QVERIFY(FormatManager::instance().loadFile("example/spi.rmt", &model1, &config1).success);

    // Save as .rmb binary
    QString outRmb = "work/test_formats/roundtrip.rmb";
    QVERIFY(FormatManager::instance().saveFile(outRmb, &model1, &config1).success);

    // Reload .rmb binary
    RegMapTreeModel model2;
    RegConfigWindow config2;
    FormatResult res = FormatManager::instance().loadFile(outRmb, &model2, &config2);
    QVERIFY2(res.success, qPrintable(res.errorMessage));

    RegMapTreeItem *root2 = model2.getRootItem();
    QVERIFY(root2 != nullptr);
    QCOMPARE(root2->childCount(), 1);
    QCOMPARE(root2->child(0)->childCount(), 2);
}

void TestFormats::test_ComprehensiveMapRoundtrip()
{
    RegMapTreeModel model1;
    RegConfigWindow config1;
    FormatResult loadRes = FormatManager::instance().loadFile("example/comprehensive.rmt", &model1, &config1);
    QVERIFY2(loadRes.success, qPrintable(loadRes.errorMessage));

    RegMapTreeItem *root = model1.getRootItem();
    QVERIFY(root != nullptr);
    QVERIFY(root->childCount() >= 1);

    // Save as JSON, XML, SVD, RDL, CSV
    QString outJson = "work/test_formats/comp.json";
    QString outXml  = "work/test_formats/comp.xml";
    QString outSvd  = "work/test_formats/comp.svd";
    QString outRdl  = "work/test_formats/comp.rdl";
    QString outCsv  = "work/test_formats/comp.csv";

    QVERIFY(FormatManager::instance().saveFile(outJson, &model1, &config1).success);
    QVERIFY(FormatManager::instance().saveFile(outXml,  &model1, &config1).success);
    QVERIFY(FormatManager::instance().saveFile(outSvd,  &model1, &config1).success);
    QVERIFY(FormatManager::instance().saveFile(outRdl,  &model1, &config1).success);
    QVERIFY(FormatManager::instance().saveFile(outCsv,  &model1, &config1).success);

    // Reload from JSON and verify items
    RegMapTreeModel modelJson;
    RegConfigWindow configJson;
    FormatResult loadJsonRes = FormatManager::instance().loadFile(outJson, &modelJson, &configJson);
    QVERIFY2(loadJsonRes.success, qPrintable(loadJsonRes.errorMessage));
    QVERIFY(modelJson.getRootItem() != nullptr);
    QCOMPARE(modelJson.getRootItem()->childCount(), root->childCount());
}

void TestFormats::test_WideBus64BitMapRoundtrip()
{
    RegMapTreeModel model1;
    RegConfigWindow config1;
    FormatResult loadRes = FormatManager::instance().loadFile("example/wide_bus_64bit.rmt", &model1, &config1);
    QVERIFY2(loadRes.success, qPrintable(loadRes.errorMessage));

    RegMapTreeItem *root = model1.getRootItem();
    QVERIFY(root != nullptr);
    QVERIFY(root->childCount() >= 1);

    // Save as JSON and reload
    QString outJson = "work/test_formats/wide64.json";
    QVERIFY(FormatManager::instance().saveFile(outJson, &model1, &config1).success);

    RegMapTreeModel model2;
    RegConfigWindow config2;
    FormatResult loadRes2 = FormatManager::instance().loadFile(outJson, &model2, &config2);
    QVERIFY2(loadRes2.success, qPrintable(loadRes2.errorMessage));

    // Validate 64-bit width passes checkData(64)
    QStringList errors = model2.checkData(64);
    QVERIFY2(errors.isEmpty(), "Wide 64-bit map should have zero errors under 64-bit mode");
}

void TestFormats::test_InvalidFileHandling()
{
    RegMapTreeModel model;
    RegConfigWindow config;

    // 1. Non-existent file
    FormatResult resNonExistent = FormatManager::instance().loadFile("non_existent_file.xyz", &model, &config);
    QVERIFY(!resNonExistent.success);

    // 2. Corrupt JSON file
    QFile fBadJson("work/test_formats/bad.json");
    if (fBadJson.open(QIODevice::WriteOnly | QIODevice::Text)) {
        fBadJson.write("{ corrupted json content [}}");
        fBadJson.close();
    }
    FormatResult resBadJson = FormatManager::instance().loadFile("work/test_formats/bad.json", &model, &config);
    QVERIFY(!resBadJson.success);

    // 3. Corrupt XML file
    QFile fBadXml("work/test_formats/bad.xml");
    if (fBadXml.open(QIODevice::WriteOnly | QIODevice::Text)) {
        fBadXml.write("<unclosed_tag><another>");
        fBadXml.close();
    }
    FormatResult resBadXml = FormatManager::instance().loadFile("work/test_formats/bad.xml", &model, &config);
    QVERIFY(!resBadXml.success);
}

QTEST_MAIN(TestFormats)
#include "test_Formats.moc"
