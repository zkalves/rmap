/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include <QtTest>
#include <QDir>
#include "RegMapTreeModel.hpp"
#include "RegConfigWindow.hpp"
#include "format/FormatManager.hpp"
#include "format/SystemRdlHandler.hpp"
#include "format/IpxactHandler.hpp"
#include "format/CmsisSvdHandler.hpp"
#include "format/JsonHandler.hpp"
#include "format/CsvHandler.hpp"
#include "format/ProtobufHandler.hpp"

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
    void test_SystemRdlExtendedSyntaxAndErrors();
    void test_IpxactExtendedSyntaxAndErrors();
    void test_CmsisSvdExtendedSyntaxAndErrors();
    void test_CsvExtendedSyntaxAndErrors();
    void test_JsonExtendedSyntaxAndErrors();
    void test_FormatManagerEdgeCases();
    void test_ProtobufExtendedSyntaxAndErrors();
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
    FormatResult res = FormatManager::instance().loadFile("examples/systemrdl/atxmega_spi.rdl", &model, &config);
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

    FormatResult res1 = FormatManager::instance().loadFile("examples/systemrdl/atxmega_spi.rdl", &model1, &config1);
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
    FormatResult res1 = FormatManager::instance().loadFile("examples/rmt/peripherals/spi.rmt", &model1, &config1);
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

    FormatResult res1 = FormatManager::instance().loadFile("examples/rmt/peripherals/spi.rmt", &model1, &config1);
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

    FormatResult res1 = FormatManager::instance().loadFile("examples/rmt/peripherals/spi.rmt", &model1, &config1);
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

    FormatResult res1 = FormatManager::instance().loadFile("examples/rmt/peripherals/spi.rmt", &model1, &config1);
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
    QVERIFY(FormatManager::instance().loadFile("examples/rmt/peripherals/spi.rmt", &m1, &c1).success);
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
    FormatResult res = FormatManager::instance().loadFile("examples/svd/stm32_uart.svd", &model, &config);
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
    FormatResult res = FormatManager::instance().loadFile("examples/ipxact/spi_ipxact.xml", &model, &config);
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
    FormatResult res = FormatManager::instance().loadFile("examples/json/sensor_hub.json", &model, &config);
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
    FormatResult res = FormatManager::instance().loadFile("examples/csv/dma_controller.csv", &model, &config);
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
    QVERIFY(FormatManager::instance().loadFile("examples/rmt/peripherals/spi.rmt", &model1, &config1).success);

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
    FormatResult loadRes = FormatManager::instance().loadFile("examples/rmt/features/comprehensive.rmt", &model1, &config1);
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
    FormatResult loadRes = FormatManager::instance().loadFile("examples/rmt/features/wide_bus_64bit.rmt", &model1, &config1);
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

void TestFormats::test_SystemRdlExtendedSyntaxAndErrors()
{
    QString rdlContent = R"(
        /* Multi-line block comment
           with line breaks */
        <% Embedded perl/asp style comment
           with line breaks %>
        // Line comment
        addrmap ComplexMap {
            name = "ComplexMapName";
            default regwidth = 16;
            default sw = w1c;

            regfile BlockA {
                reg REG_A {
                    name = "Register A";
                    desc = "Desc with \"quotes\", \t tabs, \n newlines and \\ backslash \x fallback";
                    field {
                        desc = "Field 1";
                        sw = rw;
                        hw = ro;
                    } F1[3:0] = 4'hA;

                    field {
                        sw = ro;
                        hw = wo;
                    } F2[0:3] = 4'd2;

                    field {
                        sw = wo;
                        hw = rw;
                    } F3[4] = 1'b0;

                    field {
                        sw = w0c;
                        hw = na;
                    } F4;

                    field {
                        rc;
                        hw = other_hw;
                    } F5;

                    field {
                        w1c;
                    } F6;

                    field {
                        w1s;
                    } F7;

                    F1->reset = 4'h5;
                    F2.reset = 4'h7;
                } INST_A @ 0x20;

                reg REG_B {
                    field F_DEFAULT;
                };
            };
        };
    )";
    QString rdlPath = "work/test_formats/complex.rdl";
    QFile fRdl(rdlPath);
    QVERIFY(fRdl.open(QIODevice::WriteOnly | QIODevice::Text));
    fRdl.write(rdlContent.toUtf8());
    fRdl.close();

    RegMapTreeModel model;
    RegConfigWindow config;
    SystemRdlHandler rdlHandler;
    FormatResult readRes = rdlHandler.read(rdlPath, &model, &config);
    QVERIFY2(readRes.success, qPrintable(readRes.errorMessage));

    // Test unterminated string literal before EOF
    QString badStrRdl = "addrmap Bad { name = \"unterminated string";
    QString badStrPath = "work/test_formats/bad_string.rdl";
    QFile fBadStr(badStrPath);
    QVERIFY(fBadStr.open(QIODevice::WriteOnly | QIODevice::Text));
    fBadStr.write(badStrRdl.toUtf8());
    fBadStr.close();
    RegMapTreeModel badModel;
    rdlHandler.read(badStrPath, &badModel, nullptr);

    // Non-existent file read
    FormatResult badRead = rdlHandler.read("work/test_formats/non_existent.rdl", &model, &config);
    QVERIFY(!badRead.success);

    // Write with null model
    FormatResult badWriteNull = rdlHandler.write("work/test_formats/dummy.rdl", nullptr, nullptr);
    QVERIFY(!badWriteNull.success);

    // Write with uncreatable file path
    FormatResult badWritePath = rdlHandler.write("/non_existent_directory_xyz/file.rdl", &model, &config);
    QVERIFY(!badWritePath.success);

    // Valid write with and without config
    QString outRdl = "work/test_formats/complex_out.rdl";
    FormatResult writeRes = rdlHandler.write(outRdl, &model, &config);
    QVERIFY2(writeRes.success, qPrintable(writeRes.errorMessage));
    QVERIFY(rdlHandler.write(outRdl, &model, nullptr).success);
}

void TestFormats::test_IpxactExtendedSyntaxAndErrors()
{
    QString xmlContent = R"(<?xml version="1.0" encoding="UTF-8"?>
    <ipxact:component xmlns:ipxact="https://www.accellera.org/XMLSchema/IPXACT/1685-2014">
        <ipxact:name>IpxactChip</ipxact:name>
        <ipxact:addressBlock>
            <ipxact:name>CTRL_BLOCK</ipxact:name>
            <ipxact:baseAddress>'h1000</ipxact:baseAddress>
            <ipxact:register>
                <ipxact:name>REG_TEST</ipxact:name>
                <ipxact:description>Test register description</ipxact:description>
                <ipxact:addressOffset>0b100</ipxact:addressOffset>
                <ipxact:size>32</ipxact:size>
                <ipxact:field>
                    <ipxact:name>F_W0C</ipxact:name>
                    <ipxact:description>Field desc</ipxact:description>
                    <ipxact:bitOffset>0</ipxact:bitOffset>
                    <ipxact:bitWidth>1</ipxact:bitWidth>
                    <ipxact:access>w0c</ipxact:access>
                    <ipxact:value>0b1</ipxact:value>
                </ipxact:field>
                <ipxact:field>
                    <ipxact:name>F_RS</ipxact:name>
                    <ipxact:bitOffset>1</ipxact:bitOffset>
                    <ipxact:bitWidth>1</ipxact:bitWidth>
                    <ipxact:access>rs</ipxact:access>
                </ipxact:field>
                <ipxact:field>
                    <ipxact:name>F_W1S</ipxact:name>
                    <ipxact:bitOffset>2</ipxact:bitOffset>
                    <ipxact:bitWidth>1</ipxact:bitWidth>
                    <ipxact:access>w1s</ipxact:access>
                </ipxact:field>
                <ipxact:field>
                    <ipxact:name>F_W0S</ipxact:name>
                    <ipxact:bitOffset>3</ipxact:bitOffset>
                    <ipxact:bitWidth>1</ipxact:bitWidth>
                    <ipxact:access>w0s</ipxact:access>
                </ipxact:field>
                <ipxact:field>
                    <ipxact:name>F_RWONCE</ipxact:name>
                    <ipxact:bitOffset>4</ipxact:bitOffset>
                    <ipxact:bitWidth>1</ipxact:bitWidth>
                    <ipxact:access>read-writeonce</ipxact:access>
                </ipxact:field>
                <ipxact:field>
                    <ipxact:name>F_W1C</ipxact:name>
                    <ipxact:bitOffset>5</ipxact:bitOffset>
                    <ipxact:bitWidth>1</ipxact:bitWidth>
                    <ipxact:access>w1c</ipxact:access>
                </ipxact:field>
                <ipxact:field>
                    <ipxact:name>F_RC</ipxact:name>
                    <ipxact:bitOffset>6</ipxact:bitOffset>
                    <ipxact:bitWidth>1</ipxact:bitWidth>
                    <ipxact:access>rc</ipxact:access>
                </ipxact:field>
                <ipxact:field>
                    <ipxact:name>F_RO</ipxact:name>
                    <ipxact:bitOffset>7</ipxact:bitOffset>
                    <ipxact:bitWidth>1</ipxact:bitWidth>
                    <ipxact:access>read-only</ipxact:access>
                </ipxact:field>
                <ipxact:field>
                    <ipxact:name>F_WO</ipxact:name>
                    <ipxact:bitOffset>8</ipxact:bitOffset>
                    <ipxact:bitWidth>1</ipxact:bitWidth>
                    <ipxact:access>write-only</ipxact:access>
                </ipxact:field>
            </ipxact:register>
        </ipxact:addressBlock>
    </ipxact:component>
    )";
    QString ipxactPath = "work/test_formats/extended.xml";
    QFile f(ipxactPath);
    QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
    f.write(xmlContent.toUtf8());
    f.close();

    RegMapTreeModel model;
    RegConfigWindow config;
    IpxactHandler handler;
    FormatResult res = handler.read(ipxactPath, &model, &config);
    QVERIFY2(res.success, qPrintable(res.errorMessage));

    // Empty component fallback
    QString emptyComponent = "<ipxact:component xmlns:ipxact=\"https://www.accellera.org/XMLSchema/IPXACT/1685-2014\"><ipxact:name>EmptyComp</ipxact:name></ipxact:component>";
    QString emptyPath = "work/test_formats/empty_comp.xml";
    QFile fEmpty(emptyPath);
    QVERIFY(fEmpty.open(QIODevice::WriteOnly | QIODevice::Text));
    fEmpty.write(emptyComponent.toUtf8());
    fEmpty.close();
    RegMapTreeModel emptyModel;
    FormatResult emptyRes = handler.read(emptyPath, &emptyModel, &config);
    QVERIFY2(emptyRes.success, qPrintable(emptyRes.errorMessage));
    QVERIFY(emptyModel.getRootItem() != nullptr && emptyModel.getRootItem()->childCount() >= 1);

    // Error cases
    QVERIFY(!handler.read("work/test_formats/non_existent.xml", &model, &config).success);
    QVERIFY(!handler.write("work/test_formats/dummy.xml", nullptr, nullptr).success);
    QVERIFY(!handler.write("/non_existent_directory_xyz/file.xml", &model, &config).success);

    // Write with and without config
    QString outXml = "work/test_formats/out_ipxact.xml";
    QVERIFY(handler.write(outXml, &model, &config).success);
    QVERIFY(handler.write(outXml, &model, nullptr).success);
}

void TestFormats::test_CmsisSvdExtendedSyntaxAndErrors()
{
    QString svdContent = R"(<?xml version="1.0" encoding="utf-8"?>
    <device schemaVersion="1.3" xmlns:xs="http://www.w3.org/2001/XMLSchema-instance">
        <name>TestMCU</name>
        <size>32</size>
        <peripheral>
            <name>TIMER0</name>
            <description>Timer peripheral</description>
            <baseAddress>#40001000</baseAddress>
            <register>
                <name>CFG</name>
                <description>Config register</description>
                <addressOffset>0b1000</addressOffset>
                <size>32</size>
                <access>read-writeOnce</access>
                <resetValue>0x1234</resetValue>
                <field>
                    <name>MODE</name>
                    <description>Mode field</description>
                    <bitRange>[3:0]</bitRange>
                    <access>writeonce</access>
                </field>
                <field>
                    <name>STATUS</name>
                    <bitOffset>4</bitOffset>
                    <bitWidth>2</bitWidth>
                    <access>w0c</access>
                </field>
                <field>
                    <name>FLAG_RC</name>
                    <bitOffset>6</bitOffset>
                    <bitWidth>1</bitWidth>
                    <access>rc</access>
                </field>
                <field>
                    <name>FLAG_RS</name>
                    <bitOffset>7</bitOffset>
                    <bitWidth>1</bitWidth>
                    <access>rs</access>
                </field>
                <field>
                    <name>FLAG_W1S</name>
                    <bitOffset>8</bitOffset>
                    <bitWidth>1</bitWidth>
                    <access>w1s</access>
                </field>
                <field>
                    <name>FLAG_W0S</name>
                    <bitOffset>9</bitOffset>
                    <bitWidth>1</bitWidth>
                    <access>w0s</access>
                </field>
            </register>
        </peripheral>
    </device>
    )";
    QString svdPath = "work/test_formats/extended.svd";
    QFile f(svdPath);
    QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
    f.write(svdContent.toUtf8());
    f.close();

    RegMapTreeModel model;
    RegConfigWindow config;
    CmsisSvdHandler handler;
    FormatResult res = handler.read(svdPath, &model, &config);
    QVERIFY2(res.success, qPrintable(res.errorMessage));

    // Error cases
    QVERIFY(!handler.read("work/test_formats/non_existent.svd", &model, &config).success);
    QVERIFY(!handler.write("work/test_formats/dummy.svd", nullptr, nullptr).success);
    QVERIFY(!handler.write("/non_existent_directory_xyz/file.svd", &model, &config).success);

    // Write with and without config
    QString outSvd = "work/test_formats/out_svd.svd";
    QVERIFY(handler.write(outSvd, &model, &config).success);
    QVERIFY(handler.write(outSvd, &model, nullptr).success);
}

void TestFormats::test_CsvExtendedSyntaxAndErrors()
{
    QString csvContent = "\"Type\",\"Block\",\"Register\",\"Field\",\"Offset/LSB\",\"Width\",\"Access\",\"Reset\",\"IsRand\",\"Volatile\",\"HasReset\",\"Description\"\r\n"
                         "blk,UART_BLK,,,0x0,,RW,0x0,false,false,false,\"UART block with \"\"quoted\"\" desc\"\r\n"
                         "reg,UART_BLK,BAUD,,0x10,32,RW,0x0,false,false,false,\"Baud rate register\"\r\n"
                         "fld,UART_BLK,BAUD,DIV,0,16,RW,0x100,true,false,true,\"Divider field\"\r\n";
    QString csvPath = "work/test_formats/extended.csv";
    QFile fCsv(csvPath);
    QVERIFY(fCsv.open(QIODevice::WriteOnly | QIODevice::Text));
    fCsv.write(csvContent.toUtf8());
    fCsv.close();

    RegMapTreeModel model;
    RegConfigWindow config;
    CsvHandler handler;
    FormatResult res = handler.read(csvPath, &model, &config);
    QVERIFY2(res.success, qPrintable(res.errorMessage));

    // TSV write and read
    QString tsvPath = "work/test_formats/test.tsv";
    FormatResult writeTsv = handler.write(tsvPath, &model, &config);
    QVERIFY2(writeTsv.success, qPrintable(writeTsv.errorMessage));

    RegMapTreeModel tsvModel;
    FormatResult readTsv = handler.read(tsvPath, &tsvModel, &config);
    QVERIFY2(readTsv.success, qPrintable(readTsv.errorMessage));

    // Empty CSV file error
    QString emptyPath = "work/test_formats/empty.csv";
    QFile fEmpty(emptyPath);
    QVERIFY(fEmpty.open(QIODevice::WriteOnly | QIODevice::Text));
    fEmpty.close();
    FormatResult emptyRes = handler.read(emptyPath, &model, &config);
    QVERIFY(!emptyRes.success);
    QCOMPARE(emptyRes.errorMessage, QString("CSV file is empty."));

    // Error cases
    QVERIFY(!handler.read("work/test_formats/non_existent.csv", &model, &config).success);
    QVERIFY(!handler.write("work/test_formats/dummy.csv", nullptr, nullptr).success);
    QVERIFY(!handler.write("/non_existent_directory_xyz/file.csv", &model, &config).success);
}

void TestFormats::test_JsonExtendedSyntaxAndErrors()
{
    QString jsonContent = R"({
        "name": "CustomChip",
        "project_version": "2.5",
        "reg_width": 64,
        "blocks": [
            {
                "name": "DMA_BLK",
                "offset_hex": "0x4000",
                "description": "DMA block",
                "registers": [
                    {
                        "name": "SRC_ADDR",
                        "offset_lsb": 0,
                        "description": "Source address",
                        "access": "RW",
                        "hw_access": "RO",
                        "reset_hex": "0x0",
                        "fields": [
                            {
                                "name": "ADDR",
                                "offset_lsb": 0,
                                "size_width": 64,
                                "access": "RW",
                                "hw_access": "RO",
                                "reset_hex": "0x0",
                                "is_rand": true,
                                "volatile": true,
                                "has_reset": true,
                                "description": "64-bit source address pointer"
                            }
                        ]
                    }
                ]
            }
        ],
        "memories": [
            {
                "name": "SRAM_BUF",
                "offset_hex": "0x8000",
                "size_width": 8192,
                "access": "RW",
                "hw_access": "RW",
                "description": "Shared buffer memory"
            },
            {
                "name": "ROM_BOOT",
                "offset_lsb": 131072,
                "size_width": 4096,
                "access": "RO",
                "hw_access": "RO",
                "description": "Boot ROM memory"
            }
        ]
    })";

    QString jsonPath = "work/test_formats/extended.json";
    QFile f(jsonPath);
    QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
    f.write(jsonContent.toUtf8());
    f.close();

    RegMapTreeModel model;
    RegConfigWindow config;
    JsonHandler handler;
    FormatResult res = handler.read(jsonPath, &model, &config);
    QVERIFY2(res.success, qPrintable(res.errorMessage));

    RegMapTreeItem *root = model.getRootItem();
    QVERIFY(root != nullptr);
    bool foundMem = false;
    for (RegMapTreeItem *child : root->getChildItems()) {
        if (child->kind() == RegMapTreeItem::e_rmmKind::mem) {
            foundMem = true;
            break;
        }
    }
    QVERIFY(foundMem);

    // Error cases
    QVERIFY(!handler.read("work/test_formats/non_existent.json", &model, &config).success);
    QVERIFY(!handler.write("work/test_formats/dummy.json", nullptr, nullptr).success);
    QVERIFY(!handler.write("/non_existent_directory_xyz/file.json", &model, &config).success);

    // Write with and without config
    QString outJson = "work/test_formats/extended_out.json";
    QVERIFY(handler.write(outJson, &model, &config).success);
    QVERIFY(handler.write(outJson, &model, nullptr).success);
}

void TestFormats::test_FormatManagerEdgeCases()
{
    FormatManager &fm = FormatManager::instance();

    // Filter string
    QString filters = fm.allFilterString();
    QVERIFY(filters.contains("All Supported Formats"));
    QVERIFY(filters.contains("*.rdl"));
    QVERIFY(filters.contains("*.svd"));
    QVERIFY(filters.contains("*.xml"));
    QVERIFY(filters.contains("*.json"));
    QVERIFY(filters.contains("*.csv"));
    QVERIFY(filters.contains("*.rmt"));

    // Handlers list
    QVERIFY(fm.handlers().size() >= 6);

    // handlerByName
    QVERIFY(fm.handlerByName("SystemRDL") != nullptr);
    QVERIFY(fm.handlerByName("IP-XACT") != nullptr);
    QVERIFY(fm.handlerByName("CMSIS-SVD") != nullptr);
    QVERIFY(fm.handlerByName("JSON") != nullptr);
    QVERIFY(fm.handlerByName("CSV") != nullptr);
    QVERIFY(fm.handlerByName("Protobuf") != nullptr);
    QVERIFY(fm.handlerByName("NonExistentFormat123") == nullptr);

    // handlerForFile
    QVERIFY(fm.handlerForFile("test.rdl") != nullptr);
    QVERIFY(fm.handlerForFile("test.xml") != nullptr);
    QVERIFY(fm.handlerForFile("test.svd") != nullptr);
    QVERIFY(fm.handlerForFile("test.json") != nullptr);
    QVERIFY(fm.handlerForFile("test.csv") != nullptr);
    QVERIFY(fm.handlerForFile("test.tsv") != nullptr);
    QVERIFY(fm.handlerForFile("test.rmt") != nullptr);
    QVERIFY(fm.handlerForFile("test.rmb") != nullptr);

    // Unknown extension fallback
    auto fallbackHandler = fm.handlerForFile("test.custom_ext");
    QVERIFY(fallbackHandler != nullptr);
    QCOMPARE(fallbackHandler->formatName(), QString("Protobuf"));

    // Clear handlers and test error reporting
    fm.clearHandlers();
    QCOMPARE(fm.handlers().size(), static_cast<size_t>(0));
    QVERIFY(fm.handlerForFile("test.rdl") == nullptr);

    RegMapTreeModel model;
    RegConfigWindow config;
    FormatResult noHandlerLoad = fm.loadFile("test.rdl", &model, &config);
    QVERIFY(!noHandlerLoad.success);
    QVERIFY(noHandlerLoad.errorMessage.contains("No format handler found"));

    FormatResult noHandlerSave = fm.saveFile("test.rdl", &model, &config);
    QVERIFY(!noHandlerSave.success);
    QVERIFY(noHandlerSave.errorMessage.contains("No format handler found"));

    // Restore default handlers
    fm.registerDefaultHandlers();
    QVERIFY(fm.handlers().size() >= 6);
}

void TestFormats::test_ProtobufExtendedSyntaxAndErrors()
{
    ProtobufHandler handler;
    RegMapTreeModel model;
    RegConfigWindow config;

    // Error opening non-existent file
    FormatResult badRead = handler.read("work/test_formats/non_existent.rmt", &model, &config);
    QVERIFY(!badRead.success);

    // Error opening non-existent binary rmb
    FormatResult badReadRmb = handler.read("work/test_formats/non_existent.rmb", &model, &config);
    QVERIFY(!badReadRmb.success);

    // Error writing to invalid path
    FormatResult badWrite = handler.write("/non_existent_directory_xyz/file.rmt", &model, &config);
    QVERIFY(!badWrite.success);

    FormatResult badWriteRmb = handler.write("/non_existent_directory_xyz/file.rmb", &model, &config);
    QVERIFY(!badWriteRmb.success);
}

QTEST_MAIN(TestFormats)
#include "test_Formats.moc"
