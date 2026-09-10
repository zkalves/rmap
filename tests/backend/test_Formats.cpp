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
#include "RegMapTreeItem.hpp"

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

                    field my_fld_type {
                        sw = rs;
                        w0s;
                    } F8;

                    field { sw = w1s; } F9;
                    field { sw = w0s; } F10;
                    field { sw = custom_unknown; } F11;

                    unknown_reg_property = 123;

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

    // Test lexer single char fallback ($) and parser peek/consume past EOF
    QString truncRdl = "$ addrmap Incomplete { reg REG_INC { field {";
    QString truncPath = "work/test_formats/truncated.rdl";
    QFile fTrunc(truncPath);
    QVERIFY(fTrunc.open(QIODevice::WriteOnly | QIODevice::Text));
    fTrunc.write(truncRdl.toUtf8());
    fTrunc.close();
    rdlHandler.read(truncPath, &badModel, nullptr);

    // Advanced RDL syntax: block comments, embedded comments, arrows, defaults, binary literals
    {
        QString advRdl = R"(
            /* Block comment test */
            <% Embedded template comment %>
            addrmap {
                default regwidth = 64;
                default sw = rw;
                default unknown_default_prop = 123;

                regfile {
                    reg ControlReg {
                        field {
                            name = "EN";
                        } en[0:0] = 0b1;

                        field {
                            name = "STATUS";
                        } status[1:1];

                        status->reset = 0b1010;
                        status.other_prop = 1;
                        status->other_prop = 2;
                    } CTRL @ 0x10;

                    reg {
                        field {
                            desc = "Anonymous field";
                        } f_anon;
                    } ANON_REG;

                    reg MyType InstNoBody @ 0x20;
                };
            };
        )";
        QString advPath = "work/test_formats/advanced.rdl";
        QFile fAdv(advPath);
        QVERIFY(fAdv.open(QIODevice::WriteOnly | QIODevice::Text));
        fAdv.write(advRdl.toUtf8());
        fAdv.close();
        RegMapTreeModel advModel;
        RegConfigWindow advConfig;
        FormatResult advRes = rdlHandler.read(advPath, &advModel, &advConfig);
        QVERIFY2(advRes.success, qPrintable(advRes.errorMessage));
    }

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

    // Special write case: empty block name, width 0, hwAccess WO and RW, reset permutations, non-blk, non-reg
    {
        RegMapTreeModel specialMdl;
        // Non-block item directly under root
        specialMdl.insertRows(0, 1, RegMapTreeItem::e_rmmKind::mem, QModelIndex());
        
        // Block 1 with empty name
        specialMdl.insertRows(1, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
        QModelIndex bIdx = specialMdl.index(1, 0, QModelIndex());
        specialMdl.setData(specialMdl.index(1, 3, QModelIndex()), "", Qt::EditRole); // empty name -> "block"

        // Non-reg item inside block
        specialMdl.insertRows(0, 1, RegMapTreeItem::e_rmmKind::mem, bIdx);

        // Register 1 inside block with non-empty description
        specialMdl.insertRows(1, 1, RegMapTreeItem::e_rmmKind::reg, bIdx);
        QModelIndex rIdx = specialMdl.index(1, 0, bIdx);
        specialMdl.setData(specialMdl.index(1, 3, bIdx), "REG_TEST", Qt::EditRole);
        specialMdl.setData(specialMdl.index(1, 10, bIdx), "Reg Description", Qt::EditRole);

        // Field 1: hasReset=true, resetVal="0x1" (both true), hwAccess="" (empty -> "r"), fldDesc non-empty
        specialMdl.insertRows(0, 1, RegMapTreeItem::e_rmmKind::fld, rIdx);
        QModelIndex f1 = specialMdl.index(0, 0, rIdx);
        specialMdl.setData(specialMdl.index(0, 2, rIdx), "0", Qt::EditRole); // width 0 -> 1
        specialMdl.setData(specialMdl.index(0, 3, rIdx), "F1", Qt::EditRole);
        specialMdl.setData(specialMdl.index(0, 5, rIdx), "", Qt::EditRole); // empty hwAccess -> "r"
        specialMdl.setData(specialMdl.index(0, 6, rIdx), "0x1", Qt::EditRole);
        specialMdl.setData(specialMdl.index(0, 9, rIdx), "true", Qt::EditRole); // Has Reset = true
        specialMdl.setData(specialMdl.index(0, 10, rIdx), "Field 1 desc", Qt::EditRole);

        // Field 2: hasReset=true, resetVal="" (hasReset true, val empty), hwAccess="ro" -> "r"
        specialMdl.insertRows(1, 1, RegMapTreeItem::e_rmmKind::fld, rIdx);
        QModelIndex f2 = specialMdl.index(1, 0, rIdx);
        specialMdl.setData(specialMdl.index(1, 3, rIdx), "F2", Qt::EditRole);
        specialMdl.setData(specialMdl.index(1, 5, rIdx), "ro", Qt::EditRole);
        specialMdl.setData(specialMdl.index(1, 6, rIdx), "", Qt::EditRole);
        specialMdl.setData(specialMdl.index(1, 9, rIdx), "true", Qt::EditRole);

        // Field 3: hasReset=false, resetVal="0x5" (hasReset false, val non-empty), hwAccess="wo" -> "w"
        specialMdl.insertRows(2, 1, RegMapTreeItem::e_rmmKind::fld, rIdx);
        QModelIndex f3 = specialMdl.index(2, 0, rIdx);
        specialMdl.setData(specialMdl.index(2, 3, rIdx), "F3", Qt::EditRole);
        specialMdl.setData(specialMdl.index(2, 5, rIdx), "wo", Qt::EditRole);
        specialMdl.setData(specialMdl.index(2, 6, rIdx), "0x5", Qt::EditRole);
        specialMdl.setData(specialMdl.index(2, 9, rIdx), "false", Qt::EditRole);

        // Field 4: hasReset=false, resetVal="" (both false), hwAccess="rw" -> "rw"
        specialMdl.insertRows(3, 1, RegMapTreeItem::e_rmmKind::fld, rIdx);
        QModelIndex f4 = specialMdl.index(3, 0, rIdx);
        specialMdl.setData(specialMdl.index(3, 3, rIdx), "F4", Qt::EditRole);
        specialMdl.setData(specialMdl.index(3, 5, rIdx), "rw", Qt::EditRole);
        specialMdl.setData(specialMdl.index(3, 6, rIdx), "", Qt::EditRole);
        specialMdl.setData(specialMdl.index(3, 9, rIdx), "false", Qt::EditRole);

        // Append non-fld child directly under reg
        RegMapTreeItem *rItem = specialMdl.getRootItem()->child(1)->child(1);
        QVariantMap emptyData;
        rItem->appendChild(new RegMapTreeItem(RegMapTreeItem::e_rmmKind::mem, emptyData, rItem));

        rdlHandler.write("work/test_formats/special_rdl_out.rdl", &specialMdl, nullptr);

        // Empty root write
        RegMapTreeModel emptyRdlModel;
        emptyRdlModel.setRootItem(nullptr);
        QVERIFY(!rdlHandler.write("work/test_formats/empty_root.rdl", &emptyRdlModel, nullptr).success);

        // Multi-line and perl-style comments, sw=rc, rclr, w1clr, w1set, regwidth, sw, project_name, reset
        QString commentRdl = "/* comment */ <% perl comment %> addrmap CommentRdl { /* block */ regwidth = 64; sw = rw; project_name = \"comment_chip\"; reg REG_C { reset = 0x55; field { sw = rc; rclr; } F_RC; field { w1clr; } F_W1C; field { w1set; } F_W1S; } INST_C @ 0x0; };";
        QString commentPath = "work/test_formats/comments.rdl";
        QFile fComment(commentPath);
        QVERIFY(fComment.open(QIODevice::WriteOnly | QIODevice::Text));
        fComment.write(commentRdl.toUtf8());
        fComment.close();
        RegMapTreeModel commentModel;
        QVERIFY(rdlHandler.read(commentPath, &commentModel, nullptr).success);

        // Unclosed comments at EOF
        QString unclosedRdl1 = "/* unclosed comment at EOF";
        QString unclosedPath1 = "work/test_formats/unclosed1.rdl";
        QFile fUnclosed1(unclosedPath1);
        QVERIFY(fUnclosed1.open(QIODevice::WriteOnly | QIODevice::Text));
        fUnclosed1.write(unclosedRdl1.toUtf8());
        fUnclosed1.close();
        rdlHandler.read(unclosedPath1, &badModel, nullptr);

        QString unclosedRdl2 = "<% unclosed erb at EOF";
        QString unclosedPath2 = "work/test_formats/unclosed2.rdl";
        QFile fUnclosed2(unclosedPath2);
        QVERIFY(fUnclosed2.open(QIODevice::WriteOnly | QIODevice::Text));
        fUnclosed2.write(unclosedRdl2.toUtf8());
        fUnclosed2.close();
        rdlHandler.read(unclosedPath2, &badModel, nullptr);

        // Decimal prefix 'd, binary prefix 0b, and fallback format 'o
        QString numRdl = "addrmap NumRdl { reg REG_N { field { } FN @ 0b1000 = 'd100; field { } FO = 'o77; } INST_N @ 0x0; };";
        QString numPath = "work/test_formats/nums.rdl";
        QFile fNum(numPath);
        QVERIFY(fNum.open(QIODevice::WriteOnly | QIODevice::Text));
        fNum.write(numRdl.toUtf8());
        fNum.close();
        RegMapTreeModel numModel;
        QVERIFY(rdlHandler.read(numPath, &numModel, nullptr).success);
    }
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
                <ipxact:field>
                    <ipxact:name>F_UNKNOWN</ipxact:name>
                    <ipxact:bitOffset>9</ipxact:bitOffset>
                    <ipxact:bitWidth>1</ipxact:bitWidth>
                    <ipxact:access>custom_unknown</ipxact:access>
                </ipxact:field>
            </ipxact:register>
        </ipxact:addressBlock>
        <ipxact:register>
            <ipxact:name>DIRECT_IPXACT_REG</ipxact:name>
            <ipxact:addressOffset>0x500</ipxact:addressOffset>
            <ipxact:size>32</ipxact:size>
            <ipxact:access>custom_unknown</ipxact:access>
        </ipxact:register>
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

    // Empty root write
    RegMapTreeModel emptyIpxactMdl;
    emptyIpxactMdl.setRootItem(nullptr);
    QVERIFY(!handler.write("work/test_formats/empty_root.xml", &emptyIpxactMdl, nullptr).success);

    // Additional IP-XACT syntax: rw, ro, r, wo, w, size 0, decimal and hex address formats
    QString ipxactSyntax = R"(<?xml version="1.0" encoding="UTF-8"?>
    <ipxact:component xmlns:ipxact="https://www.accellera.org/XMLSchema/IPXACT/1685-2014">
        <ipxact:name>SyntaxComp</ipxact:name>
        <ipxact:addressBlock>
            <ipxact:name>BLK_SYNTAX</ipxact:name>
            <ipxact:baseAddress>4096</ipxact:baseAddress>
            <ipxact:register>
                <ipxact:name>REG_SZ0</ipxact:name>
                <ipxact:addressOffset>0x0</ipxact:addressOffset>
                <ipxact:size>0</ipxact:size>
                <ipxact:access>rw</ipxact:access>
                <ipxact:field>
                    <ipxact:name>F_RO</ipxact:name>
                    <ipxact:bitOffset>0</ipxact:bitOffset>
                    <ipxact:bitWidth>1</ipxact:bitWidth>
                    <ipxact:access>ro</ipxact:access>
                </ipxact:field>
                <ipxact:field>
                    <ipxact:name>F_R</ipxact:name>
                    <ipxact:bitOffset>1</ipxact:bitOffset>
                    <ipxact:bitWidth>1</ipxact:bitWidth>
                    <ipxact:access>r</ipxact:access>
                </ipxact:field>
                <ipxact:field>
                    <ipxact:name>F_WO</ipxact:name>
                    <ipxact:bitOffset>2</ipxact:bitOffset>
                    <ipxact:bitWidth>1</ipxact:bitWidth>
                    <ipxact:access>wo</ipxact:access>
                </ipxact:field>
                <ipxact:field>
                    <ipxact:name>F_W</ipxact:name>
                    <ipxact:bitOffset>3</ipxact:bitOffset>
                    <ipxact:bitWidth>1</ipxact:bitWidth>
                    <ipxact:access>w</ipxact:access>
                </ipxact:field>
            </ipxact:register>
        </ipxact:addressBlock>
    </ipxact:component>
    )";
    QString syntaxPath = "work/test_formats/syntax_ipxact.xml";
    QFile fSyn(syntaxPath);
    QVERIFY(fSyn.open(QIODevice::WriteOnly | QIODevice::Text));
    fSyn.write(ipxactSyntax.toUtf8());
    fSyn.close();
    RegMapTreeModel synModel;
    QVERIFY(handler.read(syntaxPath, &synModel, nullptr).success);

    // Export IP-XACT with project version, non-blk under root, non-reg under blk, non-fld under reg, empty desc
    QVariantMap dummyData;
    dummyData["Type"] = "mem";
    synModel.getRootItem()->appendChild(new RegMapTreeItem(RegMapTreeItem::e_rmmKind::mem, dummyData, synModel.getRootItem()));
    RegMapTreeItem *blk0 = synModel.getRootItem()->child(0);
    blk0->appendChild(new RegMapTreeItem(RegMapTreeItem::e_rmmKind::mem, dummyData, blk0));
    RegMapTreeItem *reg0 = blk0->child(0);
    reg0->setData("Description", ""); // empty description
    reg0->appendChild(new RegMapTreeItem(RegMapTreeItem::e_rmmKind::mem, dummyData, reg0));

    RegConfigWindow synCfg;
    synCfg.setProjectVersion("2.1.0");
    synCfg.setRegisterWidth(64);
    QVERIFY(handler.write("work/test_formats/syn_out.xml", &synModel, &synCfg).success);

    // Error cases: read non-existent, write null model, write uncreatable path
    QVERIFY(!handler.read("work/test_formats/non_existent.xml", &synModel, nullptr).success);
    QVERIFY(!handler.write("work/test_formats/dummy.xml", nullptr, nullptr).success);
    QVERIFY(!handler.write("/non_existent_directory_xyz/file.xml", &synModel, &synCfg).success);
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
        <register>
            <name>DIRECT_SVD_REG</name>
            <addressOffset>0x100</addressOffset>
            <size>32</size>
            <access>unknown_policy</access>
        </register>
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

    // Malformed XML SVD read error
    QString badXmlPath = "work/test_formats/malformed.svd";
    QFile fBad(badXmlPath);
    QVERIFY(fBad.open(QIODevice::WriteOnly | QIODevice::Text));
    fBad.write("<device><unclosed>");
    fBad.close();
    FormatResult badXmlRes = handler.read(badXmlPath, &model, &config);
    QVERIFY(!badXmlRes.success);
    QVERIFY(badXmlRes.errorMessage.contains("XML Parse Error"));

    // Write with and without config
    QString outSvd = "work/test_formats/out_svd.svd";
    QVERIFY(handler.write(outSvd, &model, &config).success);
    QVERIFY(handler.write(outSvd, &model, nullptr).success);

    // Empty root write
    RegMapTreeModel emptySvdMdl;
    emptySvdMdl.setRootItem(nullptr);
    QVERIFY(!handler.write("work/test_formats/empty_root.svd", &emptySvdMdl, nullptr).success);

    // SVD with size 0, rw, ro, r, wo, writeonce, w1c
    QString svdSyntax = R"(<?xml version="1.0" encoding="utf-8"?>
    <device schemaVersion="1.3">
        <name>SyntaxDev</name>
        <size>0</size>
        <peripheral>
            <name>PERIPH_SYN</name>
            <baseAddress>0x1000</baseAddress>
            <register>
                <name>REG_RW</name>
                <addressOffset>0x0</addressOffset>
                <size>32</size>
                <access>rw</access>
            </register>
            <register>
                <name>REG_RO</name>
                <addressOffset>0x4</addressOffset>
                <size>32</size>
                <access>ro</access>
            </register>
            <register>
                <name>REG_R</name>
                <addressOffset>0x8</addressOffset>
                <size>32</size>
                <access>r</access>
            </register>
            <register>
                <name>REG_WO</name>
                <addressOffset>0xC</addressOffset>
                <size>32</size>
                <access>wo</access>
            </register>
            <register>
                <name>REG_W1C</name>
                <addressOffset>0x10</addressOffset>
                <size>32</size>
                <access>w1c</access>
            </register>
        </peripheral>
    </device>
    )";
    QString svdSynPath = "work/test_formats/syntax_svd.svd";
    QFile fSvdSyn(svdSynPath);
    QVERIFY(fSvdSyn.open(QIODevice::WriteOnly | QIODevice::Text));
    fSvdSyn.write(svdSyntax.toUtf8());
    fSvdSyn.close();
    RegMapTreeModel svdSynModel;
    QVERIFY(handler.read(svdSynPath, &svdSynModel, nullptr).success);

    // Write SVD with register having 0 fields, non-blk under root, non-reg under blk, non-fld under reg, and RO/WO/W1C/W0C
    QVariantMap dummyData;
    dummyData["Type"] = "mem";
    svdSynModel.getRootItem()->appendChild(new RegMapTreeItem(RegMapTreeItem::e_rmmKind::mem, dummyData, svdSynModel.getRootItem()));
    RegMapTreeItem *p0 = svdSynModel.getRootItem()->child(0);
    p0->appendChild(new RegMapTreeItem(RegMapTreeItem::e_rmmKind::mem, dummyData, p0));
    RegMapTreeItem *r0 = p0->child(0);
    // r0 has 0 children initially, which tests childCount() == 0

    // Add field with WO, W1C, W0C
    QVariantMap fldMap;
    fldMap["Type"] = "fld";
    fldMap["Name"] = "FLD_WO";
    fldMap["Offset/LSB"] = "0";
    fldMap["Size/Width"] = "1";
    fldMap["Access Policy"] = "WO";
    RegMapTreeItem *r1 = p0->child(1);
    r1->appendChild(new RegMapTreeItem(RegMapTreeItem::e_rmmKind::fld, fldMap, r1));
    r1->appendChild(new RegMapTreeItem(RegMapTreeItem::e_rmmKind::mem, dummyData, r1)); // non-fld under reg

    fldMap["Name"] = "FLD_W1C";
    fldMap["Access Policy"] = "W1C";
    RegMapTreeItem *r2 = p0->child(2);
    r2->appendChild(new RegMapTreeItem(RegMapTreeItem::e_rmmKind::fld, fldMap, r2));

    fldMap["Name"] = "FLD_W0C";
    fldMap["Access Policy"] = "W0C";
    RegMapTreeItem *r3 = p0->child(3);
    r3->appendChild(new RegMapTreeItem(RegMapTreeItem::e_rmmKind::fld, fldMap, r3));

    QVERIFY(handler.write("work/test_formats/svd_write_cases.svd", &svdSynModel, nullptr).success);

    // Error cases: read non-existent, write uncreatable path
    QVERIFY(!handler.read("work/test_formats/non_existent.svd", &model, nullptr).success);
    QVERIFY(!handler.write("/non_existent_directory_xyz/file.svd", &svdSynModel, nullptr).success);
}

void TestFormats::test_CsvExtendedSyntaxAndErrors()
{
    QString csvContent = "\"Type\",\"Block\",\"Register\",\"Field\",\"Offset/LSB\",\"Width\",\"Access\",\"Reset\",\"IsRand\",\"Volatile\",\"HasReset\",\"Description\"\r\n"
                         "blk,UART_BLK,,,0x0,,RW,0x0,false,false,false,\"UART block with \"\"quoted\"\" desc\"\r\n"
                         "reg,UART_BLK,BAUD,,0x10,32,RW,0x0,false,false,false,\"Baud rate register\"\r\n"
                         "fld,UART_BLK,BAUD,DIV,0,16,RW,0x100,true,false,true,\"Divider field\"\r\n";
    QString csvPath = "work/test_formats/extended.csv";
    QFile fCsv(csvPath);
    QVERIFY(fCsv.open(QIODevice::WriteOnly));
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

    // CSV write with quotes, commas, and newlines in description -> covers escapeCsv
    model.setData(model.index(0, 10, model.index(0, 0, QModelIndex())), "Desc with \"quotes\", comma, and\r\nnewlines", Qt::EditRole);
    QString quotedCsvPath = "work/test_formats/quoted_out.csv";
    QVERIFY(handler.write(quotedCsvPath, &model, &config).success);

    // CSV read without trailing newline at EOF -> covers lines 70-72
    QString noNewlineCsv = "work/test_formats/no_newline.csv";
    QFile fNoNl(noNewlineCsv);
    QVERIFY(fNoNl.open(QIODevice::WriteOnly | QIODevice::Text));
    fNoNl.write("Type,Block,Register,Field,Offset/LSB,Width,Access,Reset,IsRand,Volatile,HasReset,Description\nblk,BLK1,,,0x0,,RW,0x0,false,false,false,No trailing newline");
    fNoNl.close();
    RegMapTreeModel noNlModel;
    QVERIFY(handler.read(noNewlineCsv, &noNlModel, &config).success);

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

    // CSV with 'block' as header, skipped short rows, duplicate block/register, RO access, anonymous block
    {
        QString edgeCsvPath = "work/test_formats/edge_cases.csv";
        QFile fEdge(edgeCsvPath);
        QVERIFY(fEdge.open(QIODevice::WriteOnly | QIODevice::Text));
        fEdge.write("block,Block,Register,Field,Offset/LSB,Width,Access,Reset,IsRand,Volatile,HasReset,Description\n"
                    "short,row\n"
                    "blk,BLK1,,,0x0,,RW,0x0,false,false,false,\n"
                    "reg,BLK1,REG1,,0x0,32,RO,0x0,false,false,false,Reg RO\n"
                    "fld,BLK1,REG1,FLD1,0,8,RO,0x0,true,false,true,Field RO\n"
                    "fld,BLK1,REG1,FLD2,8,8,RW,0x0,true,false,true,Field RW\n"
                    "reg,,REG_ANON,,0x4,32,RW,0x0,false,false,false,\n"
                    "fld,,REG_ANON,FLD_A,0,16,RW,0x0,true,false,true,\n"
                    "fld,BLK1,REG1,FLD_QUOTE,24,8,RW,0x0,true,false,true,\"nested \"\"quotes\"\" here and \r\n CRLF\"\n"
                    "other,BLK1,REG1,FLD3,16,8,RW,0x0,true,false,true,\n");
        fEdge.close();
        RegMapTreeModel edgeModel;
        RegConfigWindow edgeConfig;
        QVERIFY(handler.read(edgeCsvPath, &edgeModel, &edgeConfig).success);

        // Export with a non-blk item under root, a non-reg child under blk, and non-fld under reg
        auto *root = edgeModel.getRootItem();
        QVariantMap memData;
        memData["Type"] = "mem";
        memData["Name"] = "SRAM";
        auto *memItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::mem, memData, root);
        root->appendChild(memItem);

        auto *blkItem = root->child(0);
        auto *subBlkItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::blk, memData, blkItem);
        blkItem->appendChild(subBlkItem);

        auto *regItem = blkItem->child(0);
        auto *dummyChild = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::blk, memData, regItem);
        regItem->appendChild(dummyChild);

        QString outEdgeCsv = "work/test_formats/edge_out.csv";
        QVERIFY(handler.write(outEdgeCsv, &edgeModel, &edgeConfig).success);

        // Test rows of variable column counts from 4 to 11 to test every column fallback
        QString varColCsvPath = "work/test_formats/var_col.csv";
        QFile fVar(varColCsvPath);
        QVERIFY(fVar.open(QIODevice::WriteOnly | QIODevice::Text));
        fVar.write("Type,Block,Register,Field,Offset/LSB,Width,Access,Reset,IsRand,Volatile,HasReset,Description\n"
                   "\n"
                   "reg,BLK_V,REG4,F4\n"
                   "reg,BLK_V,REG5,F5,0x10\n"
                   "reg,BLK_V,REG6,F6,0x14,16\n"
                   "reg,BLK_V,REG7,F7,0x18,16,WO\n"
                   "reg,BLK_V,REG8,F8,0x1C,16,WO,0x55\n"
                   "reg,BLK_V,REG9,F9,0x20,16,WO,0x55,false\n"
                   "reg,BLK_V,REG10,F10,0x24,16,WO,0x55,false,true\n"
                   "reg,BLK_V,REG11,F11,0x28,16,WO,0x55,false,true,false\n"
                   "fld,BLK_V,REG4,,0x0\n"
                   "fld,BLK_V,,FLD_NO_REG,0x0\n"
        );
        fVar.close();
        RegMapTreeModel varModel;
        QVERIFY(handler.read(varColCsvPath, &varModel, nullptr).success);

        // Standalone CR (\r) newline support
        QString crCsvPath = "work/test_formats/cr_only.csv";
        QFile fCr(crCsvPath);
        QVERIFY(fCr.open(QIODevice::WriteOnly | QIODevice::Text));
        fCr.write("Type,Block,Register,Field\rreg,BLK_CR,REG_CR,FLD_CR\r");
        fCr.close();
        RegMapTreeModel crModel;
        QVERIFY(handler.read(crCsvPath, &crModel, nullptr).success);

        // write with null root item
        RegMapTreeModel emptyModel;
        emptyModel.setRootItem(nullptr);
        QVERIFY(!handler.write("work/test_formats/no_root.csv", &emptyModel, nullptr).success);

        // read with model == nullptr
        QVERIFY(handler.read(varColCsvPath, nullptr, nullptr).success);

        // Headerless CSV (first row is data directly)
        QString noHdrPath = "work/test_formats/no_hdr.csv";
        QFile fNoHdr(noHdrPath);
        QVERIFY(fNoHdr.open(QIODevice::WriteOnly | QIODevice::Text));
        fNoHdr.write("reg,BLK_NH,REG_NH,FLD_NH,0x0,32,RW,0x0,false,false,false,No Header Desc\n");
        fNoHdr.close();
        RegMapTreeModel noHdrModel;
        QVERIFY(handler.read(noHdrPath, &noHdrModel, nullptr).success);

        // TSV read & write with quotes, escaped quotes, and newlines in description
        QString tsvPath = "work/test_formats/roundtrip.tsv";
        QVERIFY(handler.write(tsvPath, &edgeModel, nullptr).success);
        RegMapTreeModel tsvModel;
        QVERIFY(handler.read(tsvPath, &tsvModel, nullptr).success);

        // Quotes, commas, and double quotes inside CSV
        QString quotesCsvPath = "work/test_formats/quotes.csv";
        QFile fQuotes(quotesCsvPath);
        QVERIFY(fQuotes.open(QIODevice::WriteOnly | QIODevice::Text));
        fQuotes.write("Type,Block,Register,Field,Offset/LSB,Width,Access,Reset,IsRand,Volatile,HasReset,Description\n"
                      "reg,BLK_Q,REG_Q,,\"0x0\",32,RW,0x0,false,false,false,\"Description with, comma and \"\"escaped\"\" quotes\"\n"
                      "fld,BLK_Q,REG_Q,FLD_Q,0,8,RW,0x0,true,false,true,\"Field with\nnewline and \rcarriage return\"\n");
        fQuotes.close();
        RegMapTreeModel quotesModel;
        QVERIFY(handler.read(quotesCsvPath, &quotesModel, nullptr).success);

        // Empty CSV file
        QString emptyCsvPath = "work/test_formats/completely_empty.csv";
        QFile fEmpty(emptyCsvPath);
        QVERIFY(fEmpty.open(QIODevice::WriteOnly | QIODevice::Text));
        fEmpty.close();
        RegMapTreeModel emptyCsvModel;
        QVERIFY(!handler.read(emptyCsvPath, &emptyCsvModel, nullptr).success);
    }
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
                        "offset_hex": "0x0",
                        "access": "RW",
                        "hw_access": "RO",
                        "reset_hex": "0x0",
                        "description": "Source address register",
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

    // Minimal JSON fallback branches (no project_name, registers with offset_lsb/neither, memory defaults, export without config)
    {
        QString minimalJson = R"({
            "blocks": [{
                "registers": [
                    { "name": "REG_LSB", "offset_lsb": 16 },
                    { "name": "REG_NO_OFF" }
                ]
            }],
            "memories": [
                { "name": "MEM_DEFAULT" }
            ]
        })";
        QString minPath = "work/test_formats/minimal.json";
        QFile fMin(minPath);
        QVERIFY(fMin.open(QIODevice::WriteOnly | QIODevice::Text));
        fMin.write(minimalJson.toUtf8());
        fMin.close();
        RegMapTreeModel minModel;
        QVERIFY(handler.read(minPath, &minModel, nullptr).success);
        QVERIFY(handler.write("work/test_formats/no_cfg.json", &minModel, nullptr).success);
    }

    // Error cases
    QVERIFY(!handler.read("work/test_formats/non_existent.json", &model, &config).success);
    QVERIFY(!handler.write("work/test_formats/dummy.json", nullptr, nullptr).success);
    QVERIFY(!handler.write("/non_existent_directory_xyz/file.json", &model, &config).success);

    // Negative types in JSON: non-string project_name/version, non-array blocks/memories
    {
        QString corruptJson = R"({
            "project_name": 123,
            "project_version": 456,
            "blocks": "not_an_array",
            "memories": "not_an_array"
        })";
        QString corruptPath = "work/test_formats/corrupt.json";
        QFile fCorrupt(corruptPath);
        QVERIFY(fCorrupt.open(QIODevice::WriteOnly | QIODevice::Text));
        fCorrupt.write(corruptJson.toUtf8());
        fCorrupt.close();
        RegMapTreeModel corruptModel;
        QVERIFY(handler.read(corruptPath, &corruptModel, nullptr).success);
    }

    // Negative types inside block and memory: non-array registers, non-string name
    {
        QString corruptBlkJson = R"({
            "name": 999,
            "blocks": [{
                "registers": "not_an_array"
            }],
            "memories": [{
                "size_width": "not_a_number"
            }]
        })";
        QString corruptBlkPath = "work/test_formats/corrupt_blk.json";
        QFile fCorruptBlk(corruptBlkPath);
        QVERIFY(fCorruptBlk.open(QIODevice::WriteOnly | QIODevice::Text));
        fCorruptBlk.write(corruptBlkJson.toUtf8());
        fCorruptBlk.close();
        RegMapTreeModel corruptBlkModel;
        QVERIFY(handler.read(corruptBlkPath, &corruptBlkModel, nullptr).success);
    }

    // Negative types inside register: non-array fields
    {
        QString corruptRegJson = R"({
            "blocks": [{
                "registers": [{
                    "name": "R_CORRUPT",
                    "fields": "not_an_array"
                }]
            }]
        })";
        QString corruptRegPath = "work/test_formats/corrupt_reg.json";
        QFile fCorruptReg(corruptRegPath);
        QVERIFY(fCorruptReg.open(QIODevice::WriteOnly | QIODevice::Text));
        fCorruptReg.write(corruptRegJson.toUtf8());
        fCorruptReg.close();
        RegMapTreeModel corruptRegModel;
        QVERIFY(handler.read(corruptRegPath, &corruptRegModel, nullptr).success);
    }

    // JSON parse error
    {
        QString badJsonPath = "work/test_formats/bad_parse.json";
        QFile fBad(badJsonPath);
        QVERIFY(fBad.open(QIODevice::WriteOnly | QIODevice::Text));
        fBad.write("{ invalid json syntax [");
        fBad.close();
        RegMapTreeModel badModel;
        FormatResult badRes = handler.read(badJsonPath, &badModel, nullptr);
        QVERIFY(!badRes.success);
        QVERIFY(badRes.errorMessage.contains("JSON parse error"));
    }

    // JSON with reg_width as string (not number), project_name instead of name, and fields with false flags
    {
        QString flagJson = R"({
            "project_name": "FlagProject",
            "reg_width": "invalid_not_a_number",
            "blocks": [{
                "name": "B1",
                "offset_hex": "0x0",
                "description": "B1 desc",
                "registers": [{
                    "name": "R1",
                    "offset_hex": "0x0",
                    "access": "RO",
                    "hw_access": "WO",
                    "reset_hex": "0x0",
                    "description": "R1 desc",
                    "fields": [{
                        "name": "F1",
                        "offset_lsb": 0,
                        "size_width": 1,
                        "access": "RO",
                        "hw_access": "WO",
                        "reset_hex": "0x0",
                        "is_rand": false,
                        "volatile": false,
                        "has_reset": false,
                        "description": "F1 desc"
                    }]
                }]
            }],
            "memories": [{
                "name": "M1",
                "offset_hex": "0x1000",
                "size_width": 1024,
                "access": "RO",
                "hw_access": "RO",
                "description": "M1 desc"
            }]
        })";
        QString flagPath = "work/test_formats/flags.json";
        QFile fFlag(flagPath);
        QVERIFY(fFlag.open(QIODevice::WriteOnly | QIODevice::Text));
        fFlag.write(flagJson.toUtf8());
        fFlag.close();
        RegMapTreeModel flagModel;
        RegConfigWindow flagCfg;
        QVERIFY(handler.read(flagPath, &flagModel, &flagCfg).success);
        QCOMPARE(flagCfg.projectName(), QString("FlagProject"));
    }

    // Write with and without config
    QString outJson = "work/test_formats/extended_out.json";
    QVERIFY(handler.write(outJson, &model, &config).success);
    QVERIFY(handler.write(outJson, &model, nullptr).success);

    // Write with config having reg_width = 0, empty project_name and empty project_version (lines 162-164)
    {
        RegConfigWindow emptyCfg;
        emptyCfg.setRegisterWidth(0);
        emptyCfg.setProjectName("");
        emptyCfg.setProjectVersion("");
        QVERIFY(handler.write("work/test_formats/empty_cfg_out.json", &model, &emptyCfg).success);
    }

    // Read JSON without project_name or name (defaults to chip_map, lines 48-54)
    {
        QString noNameJson = R"({ "reg_width": 32, "blocks": [] })";
        QString noNamePath = "work/test_formats/no_name.json";
        QFile fNoName(noNamePath);
        QVERIFY(fNoName.open(QIODevice::WriteOnly | QIODevice::Text));
        fNoName.write(noNameJson.toUtf8());
        fNoName.close();
        RegMapTreeModel noNameMdl;
        RegConfigWindow noNameCfg;
        QVERIFY(handler.read(noNamePath, &noNameMdl, &noNameCfg).success);
        QCOMPARE(noNameCfg.projectName(), QString("chip_map"));
    }

    // Empty root write
    RegMapTreeModel emptyJsonMdl;
    emptyJsonMdl.setRootItem(nullptr);
    QVERIFY(!handler.write("work/test_formats/empty_root.json", &emptyJsonMdl, nullptr).success);

    // Reading JSON with fields and memories missing every optional property
    {
        QString bareJson = R"({
            "blocks": [{
                "registers": [{
                    "fields": [{}]
                }]
            }],
            "memories": [{
                "offset_lsb": 4096
            }]
        })";
        QString barePath = "work/test_formats/bare.json";
        QFile fBare(barePath);
        QVERIFY(fBare.open(QIODevice::WriteOnly | QIODevice::Text));
        fBare.write(bareJson.toUtf8());
        fBare.close();
        RegMapTreeModel bareModel;
        QVERIFY(handler.read(barePath, &bareModel, nullptr).success);
    }
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

    // Corrupted Protobuf syntax read error -> covers lines 50-53
    QString badRmt = "work/test_formats/bad_syntax.rmt";
    QFile fBadRmt(badRmt);
    QVERIFY(fBadRmt.open(QIODevice::WriteOnly | QIODevice::Text));
    fBadRmt.write("corrupted_syntax {{{ unknown");
    fBadRmt.close();
    FormatResult parseFail = handler.read(badRmt, &model, &config);
    QVERIFY(!parseFail.success);
    QVERIFY(parseFail.errorMessage.contains("Failed to parse Protobuf file"));

    // Corrupted binary .rmb read error
    QString badRmb = "work/test_formats/bad_syntax.rmb";
    QFile fBadRmb(badRmb);
    QVERIFY(fBadRmb.open(QIODevice::WriteOnly));
    fBadRmb.write("\xff\xff\xff\xff\x00\x01\x02");
    fBadRmb.close();
    FormatResult parseFailRmb = handler.read(badRmb, &model, &config);
    QVERIFY(!parseFailRmb.success);
    QVERIFY(parseFailRmb.errorMessage.contains("Failed to parse Protobuf file"));

    // Read with null config, null model, and both null
    QVERIFY(handler.read("examples/rmt/peripherals/spi.rmt", &model, nullptr).success);
    QVERIFY(handler.read("examples/rmt/peripherals/spi.rmt", nullptr, &config).success);
    QVERIFY(handler.read("examples/rmt/peripherals/spi.rmt", nullptr, nullptr).success);

    // Valid text .rmt write -> covers lines 104-107
    model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex());
    QString validRmt = "work/test_formats/valid.rmt";
    FormatResult writeRmt = handler.write(validRmt, &model, &config);
    QVERIFY(writeRmt.success);

    // Write with null config, null model, and empty model
    QVERIFY(handler.write("work/test_formats/no_cfg.rmt", &model, nullptr).success);
    QVERIFY(handler.write("work/test_formats/no_mdl.rmt", nullptr, &config).success);
    RegMapTreeModel emptyMdl;
    emptyMdl.setRootItem(nullptr);
    QVERIFY(handler.write("work/test_formats/empty_mdl.rmt", &emptyMdl, &config).success);

    // Write failure on full device -> covers lines 110-112
    if (QFile::exists("/dev/full")) {
        QString devFullSymlink = "work/test_formats/dev_full.rmb";
        QFile::remove(devFullSymlink);
        QFile::link("/dev/full", devFullSymlink);
        FormatResult fullRes = handler.write(devFullSymlink, &model, &config);
        QVERIFY(!fullRes.success);
        QFile::remove(devFullSymlink);
    }

    // Test deserialization of missing parent and out-of-bounds children via RegMapTreeItem::deserialize
    SerializationContext ctx;
    RegMapTreeItem item;
    QVariantMap dataWithoutParent;
    dataWithoutParent["childItems"] = QList<QVariant>{ QVariant(), QVariant(-1), QVariant(99999) };
    item.deserialize(dataWithoutParent, &ctx);
    QVERIFY(item.getChildItems().size() == 3);
    QCOMPARE(item.getChildItems().at(0), nullptr);
    QCOMPARE(item.getChildItems().at(1), nullptr);
    QCOMPARE(item.getChildItems().at(2), nullptr);
}

QTEST_MAIN(TestFormats)
#include "test_Formats.moc"
