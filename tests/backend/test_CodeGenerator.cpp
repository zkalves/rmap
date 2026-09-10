/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include <QtTest>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QThread>
#include <csignal>
#include "CodeGenerator.hpp"

class TestCodeGenerator : public QObject
{
    Q_OBJECT


private slots:
    void initTestCase() {
        QDir("work").removeRecursively();
        QDir().mkpath("work");
    }
    void cleanupTestCase() {
        QDir("work").removeRecursively();
    }
    void testHelperUpperAndLower();
    void testHelperToHexAndToDec();
    void testHelperBitmask();
    void testHelperPadZero();
    void testHelperSvHex();
    void testFullGenerationCHeader();
    void testFullGenerationUvmModel();
    void testMultiSourceTemplateMappings();
    void testNewNamingAndTypeHelpers();
    void testFullGenerationGenericRtl();
    void testFullGenerationRtlWithMemories();
    void testFullGenerationRustPac();
    void testFullGenerationPythonDriver();
    void testFullGenerationHtmlDoc();
    void testFullGenerationSystemRdl();
    void testFullGenerationIpxact();
    void testFullGenerationCmsisSvd();
    void testFullGenerationMarkdownDoc();
    void testFullGenerationJsonMap();
    void testRelativePathAndBaseDirResolution();
    void testEnvVarExpansionInTemplateAndOutput();
    void testRecursiveDirectoryGeneration();
    void testDynamicPathVariableSubstitution();
    void testCHeaderPaddingGeneration();
    void testRtlStrobeAndCrcGeneration();
    void testDynamicSimulationPathResolution();
    void testComprehensiveTemplateVerification();
    void testPythonScriptExecutionOnGeneration();
    void testErrorRecoveryAndInvalidTemplates();
    void testHelpersExtendedEdgeCases();
    void testLegacyAndDirectoryMethods();
    void testPythonRunnerEdgeCases();
    void testCommandLineInterface();
};

void TestCodeGenerator::testHelperUpperAndLower()
{
    CodeGenerator cg;
    Environment env;
    // Test helper registration via private helper callback mechanism indirectly or directly
    json data;
    data["name"] = "test_block";

    Template tempUpper = env.parse("{{ upper(name) }}");
    Template tempLower = env.parse("{{ lower(name) }}");

    // CodeGenerator registers helpers
    // We test template generation through CodeGenerator::generate
    TemplateMapping mapping;
    // Create temporary template files
    QDir().mkpath("work/test_tmpl");
    QFile fUpper("work/test_tmpl/upper.inja");
    if (fUpper.open(QIODevice::WriteOnly | QIODevice::Text)) {
        fUpper.write("RESULT: {{ upper(name) }} - {{ lower(name) }}");
        fUpper.close();
    }

    std::vector<TemplateMapping> mappings;
    mappings.push_back({"work/test_tmpl/upper.inja", "work/test_tmpl/upper.txt"});

    GenerationReport report = cg.generate(data, "work/test_tmpl", "work/test_tmpl", mappings);
    QVERIFY(!report.has_errors());
    QCOMPARE(report.success_files.size(), (size_t)1);

    QFile out("work/test_tmpl/upper.txt");
    const bool outOpened = out.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(outOpened);
    QString content = out.readAll();
    out.close();

    QCOMPARE(content.trimmed(), QString("RESULT: TEST_BLOCK - test_block"));
}

void TestCodeGenerator::testHelperToHexAndToDec()
{
    CodeGenerator cg;
    json data;
    data["val"] = 255;

    QDir().mkpath("work/test_tmpl");
    QFile f("work/test_tmpl/hex_dec.inja");
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        f.write("HEX: {{ to_hex(val, 4) }} DEC: {{ to_dec(val) }}");
        f.close();
    }

    std::vector<TemplateMapping> mappings;
    mappings.push_back({"work/test_tmpl/hex_dec.inja", "work/test_tmpl/hex_dec.txt"});

    GenerationReport report = cg.generate(data, "work/test_tmpl", "work/test_tmpl", mappings);
    QVERIFY(!report.has_errors());

    QFile out("work/test_tmpl/hex_dec.txt");
    const bool outOpened = out.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(outOpened);
    QString content = out.readAll();
    out.close();

    QCOMPARE(content.trimmed(), QString("HEX: 0x00FF DEC: 255"));
}

void TestCodeGenerator::testHelperBitmask()
{
    CodeGenerator cg;
    json data;
    data["w1"] = 4;
    data["lsb1"] = 0;
    data["w2"] = 3;
    data["lsb2"] = 2;

    QDir().mkpath("work/test_tmpl");
    QFile f("work/test_tmpl/mask.inja");
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        f.write("MASK1: {{ bitmask(w1, lsb1) }} MASK2: {{ bitmask(w2, lsb2) }}");
        f.close();
    }

    std::vector<TemplateMapping> mappings;
    mappings.push_back({"work/test_tmpl/mask.inja", "work/test_tmpl/mask.txt"});

    GenerationReport report = cg.generate(data, "work/test_tmpl", "work/test_tmpl", mappings);
    QVERIFY(!report.has_errors());

    QFile out("work/test_tmpl/mask.txt");
    const bool outOpened = out.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(outOpened);
    QString content = out.readAll();
    out.close();

    // 4 bits at 0 -> 0xF; 3 bits at 2 -> ((1<<3)-1)<<2 = 7<<2 = 28 = 0x1C
    QCOMPARE(content.trimmed(), QString("MASK1: 0xF MASK2: 0x1C"));
}

void TestCodeGenerator::testHelperPadZero()
{
    CodeGenerator cg;
    json data;
    data["num"] = 7;

    QDir().mkpath("work/test_tmpl");
    QFile f("work/test_tmpl/pad.inja");
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        f.write("PAD: {{ pad_zero(num, 4) }}");
        f.close();
    }

    std::vector<TemplateMapping> mappings;
    mappings.push_back({"work/test_tmpl/pad.inja", "work/test_tmpl/pad.txt"});

    GenerationReport report = cg.generate(data, "work/test_tmpl", "work/test_tmpl", mappings);
    QVERIFY(!report.has_errors());

    QFile out("work/test_tmpl/pad.txt");
    const bool outOpened = out.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(outOpened);
    QString content = out.readAll();
    out.close();

    QCOMPARE(content.trimmed(), QString("PAD: 0007"));
}

void TestCodeGenerator::testHelperSvHex()
{
    CodeGenerator cg;
    json data;
    data["offset_hex_zero"] = "0x0000";
    data["offset_hex_four"] = "0x0004";
    data["crc32_hex"] = "0x4D87A9DC";
    data["num_zero"] = 0;
    data["num_four"] = 4;
    data["fld_reset_0"] = 0;
    data["fld_reset_1"] = 1;
    data["fld_reset_5"] = 5;

    QDir().mkpath("work/test_tmpl");
    QFile f("work/test_tmpl/sv_hex.inja");
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        f.write("A: {{ sv_hex(offset_hex_zero, 32) }}\n"
                "B: {{ sv_hex(offset_hex_four, 32) }}\n"
                "C: {{ sv_hex(crc32_hex, 32) }}\n"
                "D: {{ sv_hex(num_zero, 32) }}\n"
                "E: {{ sv_hex(num_four, 32) }}\n"
                "F: {{ sv_hex(fld_reset_0, 1) }}\n"
                "G: {{ sv_hex(fld_reset_1, 1) }}\n"
                "H: {{ sv_hex(fld_reset_5, 4) }}\n"
                "I: {{ sv_hex(offset_hex_four) }}\n"
                "J: {{ sv_hex(num_four, 0) }}\n");
        f.close();
    }

    std::vector<TemplateMapping> mappings;
    mappings.push_back({"work/test_tmpl/sv_hex.inja", "work/test_tmpl/sv_hex.txt"});

    GenerationReport report = cg.generate(data, "work/test_tmpl", "work/test_tmpl", mappings);
    QVERIFY(!report.has_errors());

    QFile out("work/test_tmpl/sv_hex.txt");
    const bool outOpened = out.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(outOpened);
    QString content = out.readAll();
    out.close();

    QVERIFY(content.contains("A: 32'h0000"));
    QVERIFY(content.contains("B: 32'h0004"));
    QVERIFY(content.contains("C: 32'h4D87A9DC"));
    QVERIFY(content.contains("D: 32'h0000"));
    QVERIFY(content.contains("E: 32'h0004"));
    QVERIFY(content.contains("F: 1'h0"));
    QVERIFY(content.contains("G: 1'h1"));
    QVERIFY(content.contains("H: 4'h5"));
    QVERIFY(content.contains("I: 32'h0004"));
    QVERIFY(content.contains("J: 'h4"));
}

void TestCodeGenerator::testFullGenerationCHeader()
{
    CodeGenerator cg;

    json root;
    root["name"] = "SPI";
    root["reg_width"] = 32;
    root["reg_width_bytes"] = 4;

    json blk;
    blk["name"] = "TOP";

    json reg;
    reg["name"] = "CTRL";
    reg["offset_hex"] = "0x0";
    reg["description"] = "Control";

    json fld;
    fld["name"] = "ENABLE";
    fld["offset_lsb"] = 0;
    fld["size_width"] = 1;

    reg["fields"] = json::array({fld});
    blk["registers"] = json::array({reg});
    root["blocks"] = json::array({blk});

    std::vector<TemplateMapping> mappings;
    mappings.push_back({"templates/c/reg_map.h.inja", "work/c/test_c_header.h"});

    GenerationReport report = cg.generate(root, "./templates", "./work", mappings);
    QVERIFY(!report.has_errors());
    QCOMPARE(report.success_files.size(), (size_t)1);

    QFile out("work/c/test_c_header.h");
    const bool outOpened = out.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(outOpened);
    QString content = out.readAll();
    out.close();

    QVERIFY(content.contains("#ifndef SPI_REG_MAP_H"));
    QVERIFY(content.contains("#define TOP_CTRL_OFFSET (0x0)"));
    QVERIFY(content.contains("#define TOP_CTRL_ENABLE_MASK (0x1)"));
}

void TestCodeGenerator::testFullGenerationUvmModel()
{
    CodeGenerator cg;

    json root;
    root["name"] = "SPI";
    root["reg_width"] = 32;
    root["reg_width_bytes"] = 4;

    json blk;
    blk["name"] = "SPI_Block";

    json reg;
    reg["name"] = "STATUS";
    reg["offset_hex"] = "0x4";
    reg["size_width"] = 32;
    reg["access"] = "RO";

    json fld;
    fld["name"] = "TX_READY";
    fld["offset_lsb"] = 0;
    fld["size_width"] = 1;
    fld["access"] = "RO";
    fld["volatile"] = true;
    fld["reset_hex"] = "0x1";
    fld["has_reset"] = true;
    fld["is_rand"] = false;

    reg["fields"] = json::array({fld});
    blk["registers"] = json::array({reg});
    root["blocks"] = json::array({blk});

    std::vector<TemplateMapping> mappings;
    mappings.push_back({"templates/uvm/reg_model.sv.inja", "work/uvm/test_uvm_model.sv"});

    GenerationReport report = cg.generate(root, "./templates", "./work", mappings);
    QVERIFY(!report.has_errors());

    QFile out("work/uvm/test_uvm_model.sv");
    const bool outOpened = out.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(outOpened);
    QString content = out.readAll();
    out.close();

    QVERIFY(content.contains("class spi_block_reg_block extends uvm_reg_block;"));
    QVERIFY(content.contains("class status_reg extends uvm_reg;"));
    QVERIFY(content.contains("this.tx_ready.configure("));
}

void TestCodeGenerator::testMultiSourceTemplateMappings()
{
    CodeGenerator cg;
    json data;
    data["name"] = "MultiTest";
    data["reg_width"] = 32;
    data["reg_width_bytes"] = 4;
    data["blocks"] = json::array();

    std::vector<TemplateMapping> mappings;
    mappings.push_back({"templates/c/reg_map.h.inja", "work/c/multi_out1.h"});
    mappings.push_back({"templates/uvm/reg_model.sv.inja", "work/uvm/multi_out2.sv"});

    GenerationReport report = cg.generate(data, "./templates", "./work", mappings);
    QVERIFY(!report.has_errors());
    QCOMPARE(report.success_files.size(), (size_t)2);
}

void TestCodeGenerator::testNewNamingAndTypeHelpers()
{
    CodeGenerator cg;
    json data;
    data["name"] = "My_Device_Ctrl";
    data["name_empty"] = "";
    data["name_single"] = "a";
    data["name_single_quote"] = "\"";
    data["name_quoted"] = "\"quoted_id\"";
    data["name_unclosed_l"] = "\"unclosed";
    data["name_unclosed_r"] = "unclosed\"";
    data["name_num"] = 42;
    data["name_spaces"] = "hello world test";
    data["name_dashes"] = "hello-world test";
    data["name_caps"] = "SPI_SYS_ENABLE";
    data["name_pascal"] = "MyDeviceCtrl";
    data["width_8"] = 8;
    data["width_16"] = 16;
    data["width_32"] = 32;
    data["width_64"] = 64;
    data["lsb"] = 4;
    data["width"] = 5;

    QDir().mkpath("work/test_tmpl");
    QFile f("work/test_tmpl/helpers.inja");
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        f.write("CAMEL: {{ camel_case(name) }}\n"
                "CAMEL_EMPTY: {{ camel_case(name_empty) }}\n"
                "CAMEL_SINGLE: {{ camel_case(name_single) }}\n"
                "CAMEL_SQUOTE: {{ camel_case(name_single_quote) }}\n"
                "CAMEL_QUOTED: {{ camel_case(name_quoted) }}\n"
                "CAMEL_UNL: {{ camel_case(name_unclosed_l) }}\n"
                "CAMEL_UNR: {{ camel_case(name_unclosed_r) }}\n"
                "CAMEL_NUM: {{ camel_case(name_num) }}\n"
                "CAMEL_SPACES: {{ camel_case(name_spaces) }}\n"
                "CAMEL_DASHES: {{ camel_case(name_dashes) }}\n"
                "PASCAL: {{ pascal_case(name) }}\n"
                "PASCAL_EMPTY: {{ pascal_case(name_empty) }}\n"
                "PASCAL_SINGLE: {{ pascal_case(name_single) }}\n"
                "PASCAL_SQUOTE: {{ pascal_case(name_single_quote) }}\n"
                "PASCAL_QUOTED: {{ pascal_case(name_quoted) }}\n"
                "PASCAL_UNL: {{ pascal_case(name_unclosed_l) }}\n"
                "PASCAL_UNR: {{ pascal_case(name_unclosed_r) }}\n"
                "PASCAL_NUM: {{ pascal_case(name_num) }}\n"
                "PASCAL_SPACES: {{ pascal_case(name_spaces) }}\n"
                "PASCAL_DASHES: {{ pascal_case(name_dashes) }}\n"
                "SNAKE: {{ snake_case(name) }}\n"
                "SNAKE_EMPTY: {{ snake_case(name_empty) }}\n"
                "SNAKE_SINGLE: {{ snake_case(name_single) }}\n"
                "SNAKE_SQUOTE: {{ snake_case(name_single_quote) }}\n"
                "SNAKE_QUOTED: {{ snake_case(name_quoted) }}\n"
                "SNAKE_UNL: {{ snake_case(name_unclosed_l) }}\n"
                "SNAKE_UNR: {{ snake_case(name_unclosed_r) }}\n"
                "SNAKE_NUM: {{ snake_case(name_num) }}\n"
                "SNAKE_SPACES: {{ snake_case(name_spaces) }}\n"
                "SNAKE_DASHES: {{ snake_case(name_dashes) }}\n"
                "SNAKE_CAPS: {{ snake_case(name_caps) }}\n"
                "SNAKE_PASCAL: {{ snake_case(name_pascal) }}\n"
                "T8: {{ c_type(width_8) }}\n"
                "T16: {{ c_type(width_16) }}\n"
                "T32: {{ c_type(width_32) }}\n"
                "T64: {{ c_type(width_64) }}\n"
                "MSB: {{ msb(width, lsb) }}\n");
        f.close();
    }

    std::vector<TemplateMapping> mappings;
    mappings.push_back({"work/test_tmpl/helpers.inja", "work/test_tmpl/helpers.txt"});

    GenerationReport report = cg.generate(data, "work/test_tmpl", "work/test_tmpl", mappings);
    QVERIFY(!report.has_errors());

    QFile out("work/test_tmpl/helpers.txt");
    const bool outOpened = out.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(outOpened);
    QString content = out.readAll();
    out.close();

    QVERIFY(content.contains("CAMEL: myDeviceCtrl"));
    QVERIFY(content.contains("PASCAL: MyDeviceCtrl"));
    QVERIFY(content.contains("SNAKE: my_device_ctrl"));
    QVERIFY(content.contains("T8: uint8_t"));
    QVERIFY(content.contains("T16: uint16_t"));
    QVERIFY(content.contains("T32: uint32_t"));
    QVERIFY(content.contains("T64: uint64_t"));
    QVERIFY(content.contains("MSB: 8")); // 4 + 5 - 1 = 8
}

void TestCodeGenerator::testFullGenerationGenericRtl()
{
    CodeGenerator cg;
    json root;
    root["name"] = "SPI_GENERIC";
    root["reg_width"] = 32;
    root["reg_width_bytes"] = 4;

    json blk;
    blk["name"] = "SPI_CORE";

    json reg;
    reg["name"] = "CTRL";
    reg["offset_lsb"] = 0;
    reg["offset_hex"] = "0x0";
    reg["size_width"] = 32;
    reg["access"] = "RW";
    reg["reset_val"] = 0;
    reg["reset_hex"] = "0x0";
    reg["description"] = "Control Register";

    json fld;
    fld["name"] = "ENABLE";
    fld["offset_lsb"] = 0;
    fld["size_width"] = 1;
    fld["access"] = "RW";
    fld["reset_val"] = 0;
    fld["reset_hex"] = "0x0";
    fld["description"] = "Enable Core";

    reg["fields"] = json::array({fld});
    blk["registers"] = json::array({reg});
    root["blocks"] = json::array({blk});

    std::vector<TemplateMapping> mappings;
    mappings.push_back({"templates/rtl/reg_map.sv.inja", "work/rtl/spi_core_reg_file.sv"});

    GenerationReport report = cg.generate(root, "./templates", "./work", mappings);
    QVERIFY(!report.has_errors());

    QFile out("work/rtl/spi_core_reg_file.sv");
    const bool outOpened = out.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(outOpened);
    QString content = out.readAll();
    out.close();

    QVERIFY(content.contains("module spi_core_reg_file"));
    QVERIFY(content.contains("input  logic                      clk_i"));
    QVERIFY(content.contains("input  logic                      rst_ni"));
    QVERIFY(content.contains("input  logic                      bus_wr_en_i"));
    QVERIFY(content.contains("input  logic                      bus_rd_en_i"));
    QVERIFY(content.contains("input  logic [ADDR_WIDTH-1:0]     bus_addr_i"));
    QVERIFY(content.contains("output logic [DATA_WIDTH-1:0]     bus_rdata_o"));
    QVERIFY(content.contains("output logic                      bus_ready_o"));
    QVERIFY(content.contains("output logic                      bus_error_o"));
    QVERIFY(content.contains("ADDR_CTRL = 32'h0000;"));
    QVERIFY(content.contains("sw_ctrl_wr_strobe_o"));
    QVERIFY(content.contains("hw_ctrl_enable_o"));
    QVERIFY(content.contains("always_ff @(posedge clk_i or negedge rst_ni)"));
    QVERIFY(content.contains("always_comb begin : proc_ctrl_next"));
    QVERIFY(content.contains("always_comb begin : proc_read_decode"));
}

void TestCodeGenerator::testFullGenerationRtlWithMemories()
{
    CodeGenerator cg;
    json root;
    root["name"] = "SOC_BLOCK";
    root["reg_width"] = 32;
    root["reg_width_bytes"] = 4;

    json blk;
    blk["name"] = "PERIPH";

    json reg;
    reg["name"] = "STATUS";
    reg["offset_lsb"] = 0;
    reg["offset_hex"] = "0x0";
    reg["size_width"] = 32;
    reg["access"] = "RW";
    reg["reset_val"] = 0;
    reg["reset_hex"] = "0x0";
    reg["description"] = "Status Register";

    json fld;
    fld["name"] = "READY";
    fld["offset_lsb"] = 0;
    fld["size_width"] = 1;
    fld["access"] = "RO";
    fld["hw_access"] = "RO";
    fld["reset_val"] = 1;
    fld["reset_hex"] = "0x1";
    fld["description"] = "Ready bit";
    reg["fields"] = json::array({fld});
    blk["registers"] = json::array({reg});

    json mem;
    mem["name"] = "BUFFER_RAM";
    mem["offset_lsb"] = 4096;
    mem["offset_hex"] = "0x1000";
    mem["size_width"] = 1024;
    mem["access"] = "RW";
    mem["description"] = "Packet Buffer SRAM";
    blk["memories"] = json::array({mem});

    root["blocks"] = json::array({blk});

    std::vector<TemplateMapping> mappings;
    mappings.push_back({"templates/rtl/reg_map.sv.inja", "work/rtl/periph_reg_file.sv"});
    mappings.push_back({"templates/rtl_tb/tb_reg_map.sv.inja", "work/rtl_tb/tb_periph.sv"});
    mappings.push_back({"templates/uvm/reg_model.sv.inja", "work/uvm/periph_reg_model.sv"});
    mappings.push_back({"templates/c/reg_map.h.inja", "work/c/periph_reg_map.h"});

    GenerationReport report = cg.generate(root, "./templates", "./work", mappings);
    QVERIFY(!report.has_errors());

    // 1. Check RTL
    QFile rtlFile("work/rtl/periph_reg_file.sv");
    QVERIFY(rtlFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString rtlContent = rtlFile.readAll();
    rtlFile.close();

    QVERIFY(rtlContent.contains("output logic                      mem_buffer_ram_req_o"));
    QVERIFY(rtlContent.contains("output logic                      mem_buffer_ram_we_o"));
    QVERIFY(rtlContent.contains("output logic [ADDR_WIDTH-1:0]     mem_buffer_ram_addr_o"));
    QVERIFY(rtlContent.contains("output logic [DATA_WIDTH-1:0]     mem_buffer_ram_wdata_o"));
    QVERIFY(rtlContent.contains("output logic [STRB_WIDTH-1:0]     mem_buffer_ram_wstrb_o"));
    QVERIFY(rtlContent.contains("input  logic [DATA_WIDTH-1:0]     mem_buffer_ram_rdata_i"));
    QVERIFY(rtlContent.contains("input  logic                      mem_buffer_ram_ready_i"));
    QVERIFY(rtlContent.contains("localparam logic [ADDR_WIDTH-1:0] MEM_BUFFER_RAM_START = 32'h1000;"));
    QVERIFY(rtlContent.contains("localparam logic [ADDR_WIDTH-1:0] MEM_BUFFER_RAM_SIZE  = 1024;"));
    QVERIFY(rtlContent.contains("logic mem_buffer_ram_hit;"));
    QVERIFY(rtlContent.contains("mem_buffer_ram_hit: begin"));
    QVERIFY(rtlContent.contains("bus_rdata_o = mem_buffer_ram_rdata_i;"));
    QVERIFY(rtlContent.contains("bus_ready_o = mem_buffer_ram_ready_i;"));

    // 2. Check RTL Testbench
    QFile tbFile("work/rtl_tb/tb_periph.sv");
    QVERIFY(tbFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString tbContent = tbFile.readAll();
    tbFile.close();
    QVERIFY(tbContent.contains("tb_sram_buffer_ram"));
    QVERIFY(tbContent.contains(".mem_buffer_ram_req_o   (mem_buffer_ram_req_o)"));
    QVERIFY(tbContent.contains("Phase 6: Memory Subsystem Passthrough Verification"));

    // 3. Check UVM Model
    QFile uvmFile("work/uvm/periph_reg_model.sv");
    QVERIFY(uvmFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString uvmContent = uvmFile.readAll();
    uvmFile.close();
    QVERIFY(uvmContent.contains("uvm_mem buffer_ram;"));
    QVERIFY(uvmContent.contains("this.default_map.add_mem(this.buffer_ram, 0x1000);"));

    // 4. Check C Header
    QFile cFile("work/c/periph_reg_map.h");
    QVERIFY(cFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString cContent = cFile.readAll();
    cFile.close();
    QVERIFY(cContent.contains("#define PERIPH_BUFFER_RAM_OFFSET (0x1000)"));
    QVERIFY(cContent.contains("#define PERIPH_BUFFER_RAM_SIZE   (1024U)"));
}

void TestCodeGenerator::testFullGenerationRustPac()
{
    CodeGenerator cg;
    json root;
    root["name"] = "SPI_PAC";
    root["reg_width"] = 32;
    root["reg_width_bytes"] = 4;

    json blk;
    blk["name"] = "SPI";

    json reg;
    reg["name"] = "CTRL";
    reg["offset_lsb"] = 0;
    reg["offset_hex"] = "0x0";
    reg["size_width"] = 32;
    reg["access"] = "RW";
    reg["reset_val"] = 0;
    reg["reset_hex"] = "0x0";
    reg["description"] = "SPI Control";

    json fld;
    fld["name"] = "EN";
    fld["offset_lsb"] = 0;
    fld["size_width"] = 1;
    fld["access"] = "RW";
    fld["reset_val"] = 0;
    fld["reset_hex"] = "0x0";
    fld["description"] = "Enable";

    reg["fields"] = json::array({fld});
    blk["registers"] = json::array({reg});
    root["blocks"] = json::array({blk});

    std::vector<TemplateMapping> mappings;
    mappings.push_back({"templates/rust/reg_map.rs.inja", "work/rust/spi_pac.rs"});

    GenerationReport report = cg.generate(root, "./templates", "./work", mappings);
    QVERIFY(!report.has_errors());

    QFile out("work/rust/spi_pac.rs");
    const bool outOpened = out.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(outOpened);
    QString content = out.readAll();
    out.close();

    QVERIFY(content.contains("pub struct SPI"));
    QVERIFY(content.contains("pub struct CTRL_REG"));
}

void TestCodeGenerator::testFullGenerationPythonDriver()
{
    CodeGenerator cg;
    json root;
    root["name"] = "SPI_PY";
    root["reg_width"] = 32;
    root["reg_width_bytes"] = 4;

    json blk;
    blk["name"] = "SPI";

    json reg;
    reg["name"] = "CTRL";
    reg["offset_lsb"] = 0;
    reg["offset_hex"] = "0x0";
    reg["size_width"] = 32;
    reg["access"] = "RW";
    reg["reset_val"] = 0;
    reg["reset_hex"] = "0x0";
    reg["description"] = "SPI Control";

    json fld;
    fld["name"] = "EN";
    fld["offset_lsb"] = 0;
    fld["size_width"] = 1;
    fld["access"] = "RW";
    fld["reset_val"] = 0;
    fld["reset_hex"] = "0x0";
    fld["description"] = "Enable";

    reg["fields"] = json::array({fld});
    blk["registers"] = json::array({reg});
    root["blocks"] = json::array({blk});

    std::vector<TemplateMapping> mappings;
    mappings.push_back({"templates/python/reg_map.py.inja", "work/python/spi_driver.py"});

    GenerationReport report = cg.generate(root, "./templates", "./work", mappings);
    QVERIFY(!report.has_errors());

    QFile out("work/python/spi_driver.py");
    const bool outOpened = out.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(outOpened);
    QString content = out.readAll();
    out.close();

    QVERIFY(content.contains("class SPIBlock:"));
    QVERIFY(content.contains("self.ctrl = Register("));
}

void TestCodeGenerator::testFullGenerationHtmlDoc()
{
    CodeGenerator cg;
    json root;
    root["name"] = "SPI_DOC";
    root["reg_width"] = 32;
    root["reg_width_bytes"] = 4;

    json blk;
    blk["name"] = "SPI";

    json reg;
    reg["name"] = "CTRL";
    reg["offset_lsb"] = 0;
    reg["offset_hex"] = "0x0";
    reg["size_width"] = 32;
    reg["access"] = "RW";
    reg["reset_val"] = 0;
    reg["reset_hex"] = "0x0";
    reg["description"] = "SPI Control";

    json fld;
    fld["name"] = "EN";
    fld["offset_lsb"] = 0;
    fld["size_width"] = 1;
    fld["access"] = "RW";
    fld["reset_val"] = 0;
    fld["reset_hex"] = "0x0";
    fld["description"] = "Enable";

    reg["fields"] = json::array({fld});
    blk["registers"] = json::array({reg});
    root["blocks"] = json::array({blk});

    std::vector<TemplateMapping> mappings;
    mappings.push_back({"templates/html/reg_doc.html.inja", "work/html/spi_doc.html"});

    GenerationReport report = cg.generate(root, "./templates", "./work", mappings);
    QVERIFY(!report.has_errors());

    QFile out("work/html/spi_doc.html");
    const bool outOpened = out.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(outOpened);
    QString content = out.readAll();
    out.close();

    QVERIFY(content.contains("<!DOCTYPE html>"));
    QVERIFY(content.contains("SPI_DOC"));
    QVERIFY(content.contains("Hardware Register Map Specification"));
}

void TestCodeGenerator::testFullGenerationSystemRdl()
{
    CodeGenerator cg;
    json root;
    root["name"] = "SPI_SYS";
    root["description"] = "SPI System Map";
    root["reg_width"] = 32;

    json blk;
    blk["name"] = "SPI_BLK";
    blk["offset_hex"] = "0x4000";
    blk["description"] = "SPI Controller Block";

    json reg;
    reg["name"] = "CTRL";
    reg["offset_hex"] = "0x0";
    reg["access"] = "RW";
    reg["description"] = "Control Register";

    json fld;
    fld["name"] = "ENABLE";
    fld["offset_lsb"] = 0;
    fld["size_width"] = 1;
    fld["access"] = "RW";
    fld["hw_access"] = "RO";
    fld["reset_val"] = 1;
    fld["has_reset"] = true;
    fld["volatile"] = false;
    fld["description"] = "Module Enable";

    reg["fields"] = json::array({fld});
    blk["registers"] = json::array({reg});
    root["blocks"] = json::array({blk});

    std::vector<TemplateMapping> mappings;
    mappings.push_back({"templates/systemrdl/reg_map.rdl.inja", "work/systemrdl/spi_map.rdl"});

    GenerationReport report = cg.generate(root, "./templates", "./work", mappings);
    QVERIFY(!report.has_errors());

    QFile out("work/systemrdl/spi_map.rdl");
    const bool outOpened = out.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(outOpened);
    QString content = out.readAll();
    out.close();

    QVERIFY(content.contains("addrmap spi_sys {"));
    QVERIFY(content.contains("regfile spi_blk_rf {"));
    QVERIFY(content.contains("reg ctrl_reg_t {"));
    QVERIFY(content.contains("field {"));
    QVERIFY(content.contains("enable[0:0]"));
}

void TestCodeGenerator::testFullGenerationIpxact()
{
    CodeGenerator cg;
    json root;
    root["name"] = "SPI_IPXACT";
    root["description"] = "SPI Peripheral Model";
    root["reg_width"] = 32;

    json blk;
    blk["name"] = "SPI_BLOCK";
    blk["offset_hex"] = "0x4000";
    blk["description"] = "SPI Core";

    json reg;
    reg["name"] = "STATUS";
    reg["offset_hex"] = "0x4";
    reg["access"] = "RO";
    reg["volatile"] = true;
    reg["reset_hex"] = "0x00000000";
    reg["description"] = "Status Register";

    json fld;
    fld["name"] = "TX_READY";
    fld["offset_lsb"] = 0;
    fld["size_width"] = 1;
    fld["access"] = "RO";
    fld["hw_access"] = "WO";
    fld["reset_hex"] = "0x0";
    fld["has_reset"] = true;
    fld["volatile"] = true;
    fld["description"] = "TX FIFO Ready";

    reg["fields"] = json::array({fld});
    blk["registers"] = json::array({reg});
    root["blocks"] = json::array({blk});

    std::vector<TemplateMapping> mappings;
    mappings.push_back({"templates/ipxact/reg_map.xml.inja", "work/ipxact/spi_ipxact.xml"});

    GenerationReport report = cg.generate(root, "./templates", "./work", mappings);
    QVERIFY(!report.has_errors());

    QFile out("work/ipxact/spi_ipxact.xml");
    const bool outOpened = out.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(outOpened);
    QString content = out.readAll();
    out.close();

    QVERIFY(content.contains("<ipxact:component"));
    QVERIFY(content.contains("<ipxact:name>SPI_IPXACT</ipxact:name>"));
    QVERIFY(content.contains("<ipxact:name>STATUS</ipxact:name>"));
    QVERIFY(content.contains("<ipxact:name>TX_READY</ipxact:name>"));
}

void TestCodeGenerator::testFullGenerationCmsisSvd()
{
    CodeGenerator cg;
    json root;
    root["name"] = "SPI_SVD";
    root["description"] = "SPI SVD Description";
    root["reg_width"] = 32;

    json blk;
    blk["name"] = "SPI0";
    blk["offset_hex"] = "0x40001000";
    blk["description"] = "SPI Controller 0";

    json reg;
    reg["name"] = "DATA";
    reg["offset_hex"] = "0x8";
    reg["access"] = "RW";
    reg["reset_hex"] = "0x0";
    reg["description"] = "Data Register";

    json fld;
    fld["name"] = "BYTE";
    fld["offset_lsb"] = 0;
    fld["size_width"] = 8;
    fld["access"] = "RW";
    fld["description"] = "Data Byte";

    reg["fields"] = json::array({fld});
    blk["registers"] = json::array({reg});
    root["blocks"] = json::array({blk});

    std::vector<TemplateMapping> mappings;
    mappings.push_back({"templates/svd/reg_map.xml.inja", "work/svd/spi.svd"});

    GenerationReport report = cg.generate(root, "./templates", "./work", mappings);
    QVERIFY(!report.has_errors());

    QFile out("work/svd/spi.svd");
    const bool outOpened = out.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(outOpened);
    QString content = out.readAll();
    out.close();

    QVERIFY(content.contains("<device"));
    QVERIFY(content.contains("<name>SPI_SVD</name>"));
    QVERIFY(content.contains("<name>SPI0</name>"));
    QVERIFY(content.contains("<name>DATA</name>"));
    QVERIFY(content.contains("<name>BYTE</name>"));
}

void TestCodeGenerator::testFullGenerationMarkdownDoc()
{
    CodeGenerator cg;
    json root;
    root["name"] = "SPI_MD";
    root["description"] = "SPI Documentation";
    root["reg_width"] = 32;

    json blk;
    blk["name"] = "SPI_CORE";
    blk["offset_hex"] = "0x0";
    blk["description"] = "Core Registers";

    json reg;
    reg["name"] = "CFG";
    reg["offset_hex"] = "0x0";
    reg["access"] = "RW";
    reg["reset_hex"] = "0x0000";
    reg["description"] = "Configuration";

    json fld;
    fld["name"] = "BAUD";
    fld["offset_lsb"] = 0;
    fld["size_width"] = 16;
    fld["access"] = "RW";
    fld["hw_access"] = "RO";
    fld["reset_hex"] = "0x0000";
    fld["description"] = "Baud Rate Divisor";

    reg["fields"] = json::array({fld});
    blk["registers"] = json::array({reg});
    root["blocks"] = json::array({blk});

    std::vector<TemplateMapping> mappings;
    mappings.push_back({"templates/markdown/reg_doc.md.inja", "work/markdown/spi_doc.md"});

    GenerationReport report = cg.generate(root, "./templates", "./work", mappings);
    QVERIFY(!report.has_errors());

    QFile out("work/markdown/spi_doc.md");
    const bool outOpened = out.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(outOpened);
    QString content = out.readAll();
    out.close();

    QVERIFY(content.contains("# SPI_MD — Register Map Specification"));
    QVERIFY(content.contains("## Block: SPI_CORE"));
    QVERIFY(content.contains("`[15:0]`"));
    QVERIFY(content.contains("**BAUD**"));
}

void TestCodeGenerator::testFullGenerationJsonMap()
{
    CodeGenerator cg;
    json root;
    root["name"] = "SPI_JSON";
    root["description"] = "SPI JSON Spec";
    root["reg_width"] = 32;

    json blk;
    blk["name"] = "SPI_BLOCK";
    blk["offset_hex"] = "0x0";
    blk["description"] = "SPI Peripheral";

    json reg;
    reg["name"] = "TX_DATA";
    reg["offset_hex"] = "0x0";
    reg["offset_lsb"] = 0;
    reg["size_width"] = 32;
    reg["access"] = "WO";
    reg["sw_access"] = "WO";
    reg["hw_access"] = "RO";
    reg["reset_hex"] = "0x0";
    reg["description"] = "Transmit Data";

    json fld;
    fld["name"] = "DATA";
    fld["offset_lsb"] = 0;
    fld["size_width"] = 8;
    fld["access"] = "WO";
    fld["sw_access"] = "WO";
    fld["hw_access"] = "RO";
    fld["reset_hex"] = "0x0";
    fld["is_rand"] = true;
    fld["volatile"] = false;
    fld["has_reset"] = true;
    fld["description"] = "Payload";

    reg["fields"] = json::array({fld});
    blk["registers"] = json::array({reg});
    root["blocks"] = json::array({blk});

    std::vector<TemplateMapping> mappings;
    mappings.push_back({"templates/json/reg_map.json.inja", "work/json/spi_map.json"});

    GenerationReport report = cg.generate(root, "./templates", "./work", mappings);
    QVERIFY(!report.has_errors());

    QFile out("work/json/spi_map.json");
    const bool outOpened = out.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(outOpened);
    QString content = out.readAll();
    out.close();

    QVERIFY(content.contains("\"name\": \"SPI_JSON\""));
    QVERIFY(content.contains("\"name\": \"TX_DATA\""));
    QVERIFY(content.contains("\"name\": \"DATA\""));
}

void TestCodeGenerator::testRelativePathAndBaseDirResolution()
{
    CodeGenerator cg;
    json data;
    data["greeting"] = "Hello, Relative Paths!";

    // Create a mock subproject directory with relative template and output
    QDir().mkpath("work/test_codegen_rel/subproject/templates");
    QDir().mkpath("work/test_codegen_rel/subproject/build");

    QFile tmplFile("work/test_codegen_rel/subproject/templates/rel_test.inja");
    const bool tmplOpened = tmplFile.open(QIODevice::WriteOnly | QIODevice::Text);
    QVERIFY(tmplOpened);
    tmplFile.write("MSG: {{ greeting }}");
    tmplFile.close();

    std::string baseDir = QDir("work/test_codegen_rel/subproject").absolutePath().toStdString();

    std::vector<TemplateMapping> mappings;
    mappings.push_back({"./templates/rel_test.inja", "./build/rel_test.txt"});

    GenerationReport report = cg.generate(data, "./templates", "./build", mappings, baseDir);
    QVERIFY(!report.has_errors());
    QCOMPARE(report.success_files.size(), (size_t)1);

    QFile outFile("work/test_codegen_rel/subproject/build/rel_test.txt");
    const bool outOpened = outFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(outOpened);
    QString content = outFile.readAll();
    outFile.close();

    QCOMPARE(content.trimmed(), QString("MSG: Hello, Relative Paths!"));

    // Fallback to global/installed default templates directory (lines 226-230)
    QDir().mkpath("work/test_codegen_rel/global_tmpls");
    QFile globTmpl("work/test_codegen_rel/global_tmpls/global_rel.inja");
    QVERIFY(globTmpl.open(QIODevice::WriteOnly | QIODevice::Text));
    globTmpl.write("GLOBAL: {{ greeting }}");
    globTmpl.close();

    qputenv("RMAP_TEMPLATES_DIR", "work/test_codegen_rel/global_tmpls");
    std::vector<TemplateMapping> globMappings;
    globMappings.push_back({"global_rel.inja", "work/test_codegen_rel/global_out.txt"});
    GenerationReport globReport = cg.generate(data, "work/test_codegen_rel/nonexistent_folder", "work/test_codegen_rel", globMappings, "");
    qunsetenv("RMAP_TEMPLATES_DIR");
    QVERIFY(!globReport.has_errors());
    QVERIFY(QFile::exists("work/test_codegen_rel/global_out.txt"));

    // resolveOutputPath prefix branches: line 253 (./ + expDefaultTmpl + /)
    {
        qputenv("RMAP_TEMPLATES_DIR", "templates");
        data["name"] = "pfx_test";
        data["blocks"] = json::array({
            {
                {"name", "ctrl"},
                {"offset", 0},
                {"registers", json::array()}
            }
        });
        std::vector<TemplateMapping> pfxMappings;
        pfxMappings.push_back({"./templates/c/reg_map.h.inja", ""});
        GenerationReport pfxReport = cg.generate(data, "templates", "work/test_codegen_rel/pfx1", pfxMappings);
        qunsetenv("RMAP_TEMPLATES_DIR");
        QVERIFY(!pfxReport.has_errors());
        QVERIFY(QFile::exists("work/test_codegen_rel/pfx1/c/reg_map.h"));
    }

    // resolveOutputPath prefix branches: line 257 (./templates/ when expDefaultTmpl != templates)
    {
        qputenv("RMAP_TEMPLATES_DIR", "/custom/nonexistent/templates_dir_123");
        std::vector<TemplateMapping> pfx2Mappings;
        pfx2Mappings.push_back({"./templates/c/reg_map.h.inja", ""});
        GenerationReport pfx2Report = cg.generate(data, "templates", "work/test_codegen_rel/pfx2", pfx2Mappings);
        qunsetenv("RMAP_TEMPLATES_DIR");
        QVERIFY(!pfx2Report.has_errors());
        QVERIFY(QFile::exists("work/test_codegen_rel/pfx2/c/reg_map.h"));
    }
}

void TestCodeGenerator::testEnvVarExpansionInTemplateAndOutput()
{
    CodeGenerator cg;
    json data;
    data["var_name"] = "ENV_VAL";

    QString tmplDirAbs = QDir("work/test_codegen_env/tmpls").absolutePath();
    QString outDirAbs  = QDir("work/test_codegen_env/outs").absolutePath();
    QDir().mkpath(tmplDirAbs);
    QDir().mkpath(outDirAbs);

    qputenv("RMAP_CODEGEN_TMPL", tmplDirAbs.toLocal8Bit());
    qputenv("RMAP_CODEGEN_OUT", outDirAbs.toLocal8Bit());

    QFile tmplFile(tmplDirAbs + "/env_template.inja");
    const bool tmplOpened = tmplFile.open(QIODevice::WriteOnly | QIODevice::Text);
    QVERIFY(tmplOpened);
    tmplFile.write("VALUE: {{ var_name }}");
    tmplFile.close();

    std::vector<TemplateMapping> mappings;
    mappings.push_back({"$RMAP_CODEGEN_TMPL/env_template.inja", "${RMAP_CODEGEN_OUT}/env_out.txt"});

    GenerationReport report = cg.generate(data, "./templates", "./work", mappings);
    QVERIFY(!report.has_errors());
    QCOMPARE(report.success_files.size(), (size_t)1);

    QFile outFile(outDirAbs + "/env_out.txt");
    const bool outOpened = outFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(outOpened);
    QString content = outFile.readAll();
    outFile.close();

    QCOMPARE(content.trimmed(), QString("VALUE: ENV_VAL"));

    qunsetenv("RMAP_CODEGEN_TMPL");
    qunsetenv("RMAP_CODEGEN_OUT");
}

void TestCodeGenerator::testRecursiveDirectoryGeneration()
{
    CodeGenerator cg;

    json root;
    root["name"] = "SPI_ALL";
    root["description"] = "Comprehensive Generation Test";
    root["reg_width"] = 32;
    root["reg_width_bytes"] = 4;
    root["project_name"] = "SPI_ALL";
    root["project_version"] = "1.0";

    json blk;
    blk["name"] = "SPI_CORE";
    blk["offset_hex"] = "0x0";
    blk["description"] = "Core Control Block";

    json reg;
    reg["name"] = "CTRL";
    reg["offset_hex"] = "0x0";
    reg["offset_lsb"] = 0;
    reg["size_width"] = 32;
    reg["access"] = "RW";
    reg["sw_access"] = "RW";
    reg["hw_access"] = "RO";
    reg["reset_hex"] = "0x00000000";
    reg["reset_val"] = 0;
    reg["volatile"] = false;
    reg["description"] = "Control Register";

    json fld;
    fld["name"] = "ENABLE";
    fld["offset_lsb"] = 0;
    fld["size_width"] = 1;
    fld["access"] = "RW";
    fld["sw_access"] = "RW";
    fld["hw_access"] = "RO";
    fld["reset_hex"] = "0x0";
    fld["reset_val"] = 0;
    fld["has_reset"] = true;
    fld["volatile"] = false;
    fld["is_rand"] = false;
    fld["description"] = "Enable Field";

    reg["fields"] = json::array({fld});
    blk["registers"] = json::array({reg});
    root["blocks"] = json::array({blk});

    GenerationReport report = cg.parseDirectory(root, "templates", "work");
    QVERIFY(!report.has_errors());
    QCOMPARE(report.success_files.size(), (size_t)19);

    // Verify each expected output subfolder contains its rendered file
    QVERIFY(QFile::exists("work/c/reg_map.h"));
    QVERIFY(QFile::exists("work/rtl/reg_map.sv"));
    QVERIFY(QFile::exists("work/uvm/reg_model.sv"));
    QVERIFY(QFile::exists("work/rust/reg_map.rs"));
    QVERIFY(QFile::exists("work/python/reg_map.py"));
    QVERIFY(QFile::exists("work/html/reg_doc.html"));
    QVERIFY(QFile::exists("work/markdown/reg_doc.md"));
    QVERIFY(QFile::exists("work/systemrdl/reg_map.rdl"));
    QVERIFY(QFile::exists("work/ipxact/reg_map.xml"));
    QVERIFY(QFile::exists("work/svd/reg_map.xml"));
    QVERIFY(QFile::exists("work/json/reg_map.json"));
    QVERIFY(QFile::exists("work/rtl_tb/tb_reg_map.sv"));
    QVERIFY(QFile::exists("work/pyuvm_tb/tb_pyuvm.py"));
    QVERIFY(QFile::exists("work/uvm_tb/tb_top.sv"));
    QVERIFY(QFile::exists("work/sim/Makefile"));
}

void TestCodeGenerator::testDynamicPathVariableSubstitution()
{
    CodeGenerator cg;
    json data;
    data["name"] = "spi_master";
    data["project_name"] = "spi_project";
    data["reg_width"] = 32;
    data["reg_width_bytes"] = 4;
    json blk;
    blk["name"] = "spi_core";
    blk["registers"] = json::array();
    data["blocks"] = json::array({blk});

    // Test meaningful variables: {output_folder}, {category}, {block_name}, {file_extension}
    std::vector<TemplateMapping> mappings;
    mappings.push_back({
        "templates/c/reg_map.h.inja",
        "{output_folder}/{category}/{block_name}_custom.{file_extension}"
    });

    GenerationReport report = cg.generate(data, "templates", "work/custom_out", mappings);
    QVERIFY(!report.has_errors());
    QCOMPARE(report.success_files.size(), (size_t)1);
    QVERIFY(QFile::exists("work/custom_out/c/spi_core_custom.h"));

    // Test category directory override deduplication (hw/rtl/ does not produce hw/rtl/rtl)
    std::vector<TemplateMapping> dirMappings;
    dirMappings.push_back({
        "templates/rtl/reg_map.sv.inja",
        "work/dedup_test/rtl/"
    });
    GenerationReport report2 = cg.generate(data, "templates", "work/dedup_test", dirMappings);
    QVERIFY(!report2.has_errors());
    QVERIFY(QFile::exists("work/dedup_test/rtl/reg_map.sv"));
    QVERIFY(!QFile::exists("work/dedup_test/rtl/rtl/reg_map.sv"));
}

void TestCodeGenerator::testCHeaderPaddingGeneration()
{
    CodeGenerator cg;
    json root;
    root["name"] = "SPI_PAD";
    root["reg_width"] = 32;
    root["reg_width_bytes"] = 4;
    root["regmap_crc32_hex"] = "0xDEADBEEF";

    json blk;
    blk["name"] = "CORE";
    blk["crc32_hex"] = "0x12345678";

    // Register 0 @ offset 0x0
    json reg0;
    reg0["name"] = "CTRL";
    reg0["offset_hex"] = "0x0";
    reg0["offset_lsb"] = 0;
    reg0["size_width"] = 32;
    reg0["access"] = "RW";
    reg0["description"] = "Control";
    reg0["pad_words_before"] = 0;
    reg0["pad_bytes_before"] = 0;
    reg0["fields"] = json::array();

    // Register 1 @ offset 0x10 -> 12 byte gap = 3 words
    json reg1;
    reg1["name"] = "STATUS";
    reg1["offset_hex"] = "0x10";
    reg1["offset_lsb"] = 16;
    reg1["size_width"] = 32;
    reg1["access"] = "RO";
    reg1["description"] = "Status";
    reg1["pad_words_before"] = 3;
    reg1["pad_bytes_before"] = 12;
    reg1["fields"] = json::array();

    blk["registers"] = json::array({reg0, reg1});
    root["blocks"] = json::array({blk});

    std::vector<TemplateMapping> mappings;
    mappings.push_back({"templates/c/reg_map.h.inja", "work/c/test_pad.h"});

    GenerationReport report = cg.generate(root, "templates", "work", mappings);
    QVERIFY(!report.has_errors());

    QFile outFile("work/c/test_pad.h");
    QVERIFY(outFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = outFile.readAll();
    outFile.close();

    // Verify reserved word padding array was generated
    QVERIFY(content.contains("uint32_t _reserved_1[3];"));
    // Verify CRC32 macros were generated
    QVERIFY(content.contains("#define SPI_PAD_REGMAP_CRC32 (0xDEADBEEF)"));
    QVERIFY(content.contains("#define CORE_BLOCK_CRC32 (0x12345678)"));
}

void TestCodeGenerator::testRtlStrobeAndCrcGeneration()
{
    CodeGenerator cg;
    json root;
    root["name"] = "SPI_RTL";
    root["reg_width"] = 32;
    root["reg_width_bytes"] = 4;

    json blk;
    blk["name"] = "CORE";
    blk["crc32_hex"] = "0xCAFE1234";

    json reg;
    reg["name"] = "INTR_STATUS";
    reg["offset_hex"] = "0x0";
    reg["offset_lsb"] = 0;
    reg["size_width"] = 32;

    json fld;
    fld["name"] = "TX_DONE";
    fld["offset_lsb"] = 0;
    fld["size_width"] = 1;
    fld["access"] = "W1C";
    fld["sw_access"] = "W1C";
    fld["hw_access"] = "W1S";

    reg["fields"] = json::array({fld});
    blk["registers"] = json::array({reg});
    root["blocks"] = json::array({blk});

    std::vector<TemplateMapping> mappings;
    mappings.push_back({"templates/rtl/reg_map.sv.inja", "work/rtl/test_strobe.sv"});

    GenerationReport report = cg.generate(root, "templates", "work", mappings);
    QVERIFY(!report.has_errors());

    QFile outFile("work/rtl/test_strobe.sv");
    QVERIFY(outFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = outFile.readAll();
    outFile.close();

    // Verify REGMAP_CRC32 localparam
    QVERIFY(content.contains("localparam logic [31:0] REGMAP_CRC32 = 32'hCAFE1234;"));
    // Verify byte-strobe qualified W1C update
    QVERIFY(content.contains("wstrb_i[b]"));
}

void TestCodeGenerator::testDynamicSimulationPathResolution()
{
    CodeGenerator cg;
    json root;
    json blk;
    blk["name"] = "MY_PERIPH";
    json reg;
    reg["name"] = "CTRL";
    reg["offset_lsb"] = 0;
    reg["offset_hex"] = "0x0000";
    reg["size_width"] = 32;
    reg["access"] = "RW";
    reg["reset_val"] = 0;
    reg["reset_hex"] = "0x00000000";
    json fld;
    fld["name"] = "EN";
    fld["offset_lsb"] = 0;
    fld["size_width"] = 1;
    fld["access"] = "RW";
    fld["sw_access"] = "RW";
    fld["hw_access"] = "RO";
    fld["reset_val"] = 0;
    fld["reset_hex"] = "0x0";
    fld["has_reset"] = true;
    fld["volatile"] = false;
    fld["is_rand"] = false;
    fld["description"] = "Enable Field";
    reg["fields"] = json::array({fld});
    blk["registers"] = json::array({reg});
    blk["crc32_hex"] = "0xCAFE1234";
    root["name"] = "my_periph";
    root["reg_width"] = 32;
    root["blocks"] = json::array({blk});

    std::vector<TemplateMapping> mappings;
    mappings.push_back({"templates/rtl/reg_map.sv.inja", "work/hardware/custom_rtl/spi_core.sv"});
    mappings.push_back({"templates/uvm/reg_model.sv.inja", "work/verif/custom_uvm/spi_model.sv"});
    mappings.push_back({"templates/sim/Makefile.inja", "work/build/sim/Makefile"});

    GenerationReport report = cg.generate(root, "templates", "work", mappings);
    QVERIFY(!report.has_errors());
    QCOMPARE(report.success_files.size(), (size_t)3);

    QFile mkFile("work/build/sim/Makefile");
    QVERIFY(mkFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString mkContent = QString::fromUtf8(mkFile.readAll());
    mkFile.close();

    QVERIFY2(mkContent.contains("RTL_SRC     ?= ../../hardware/custom_rtl/spi_core.sv"),
             qPrintable(QString("Expected RTL_SRC relative path in Makefile, got:\n%1").arg(mkContent)));
    QVERIFY2(mkContent.contains("UVM_SRC     ?= ../../verif/custom_uvm/spi_model.sv"),
             qPrintable(QString("Expected UVM_SRC relative path in Makefile, got:\n%1").arg(mkContent)));
    QVERIFY2(mkContent.contains("RTL_INC_DIR ?= ../../hardware/custom_rtl"),
             qPrintable(QString("Expected RTL_INC_DIR in Makefile, got:\n%1").arg(mkContent)));
    QVERIFY2(mkContent.contains("UVM_INC_DIR ?= ../../verif/custom_uvm"),
             qPrintable(QString("Expected UVM_INC_DIR in Makefile, got:\n%1").arg(mkContent)));
}

void TestCodeGenerator::testComprehensiveTemplateVerification()
{
    QString pythonBin = QStandardPaths::findExecutable("python3");
    if (pythonBin.isEmpty()) {
        QSKIP("python3 not available in PATH");
    }

    QString rmapBin = QDir("build/bin/rmap").absolutePath();
    if (!QFile::exists(rmapBin)) {
        rmapBin = QDir("bin/rmap").absolutePath();
    }
    if (!QFile::exists(rmapBin)) {
        rmapBin = QDir("../bin/rmap").absolutePath();
    }
    if (!QFile::exists(rmapBin)) {
        QSKIP("rmap binary not available for comprehensive template test");
    }

    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("RMAP_BIN", rmapBin);
    env.insert("QT_QPA_PLATFORM", "offscreen");
    proc.setProcessEnvironment(env);
    proc.start(pythonBin, QStringList() << "tests/test_template.py" << "all");
    bool finished = proc.waitForFinished(60000);
    QVERIFY2(finished, "Template verification process timed out");
    QByteArray output = proc.readAll();
    if (proc.exitCode() != 0) {
        qWarning() << "Template verification failed output:\n" << output.constData();
    }
    QCOMPARE(proc.exitCode(), 0);
}

void TestCodeGenerator::testPythonScriptExecutionOnGeneration()
{
    QDir("work/test_python_exec").removeRecursively();
    QDir().mkpath("work/test_python_exec");

    // 1. Create a Python script that asserts all Inja template variables are available
    QString scriptPath = "work/test_python_exec/verify_vars.py";
    QFile scriptFile(scriptPath);
    QVERIFY(scriptFile.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream out(&scriptFile);
    out << "import sys, os, json\n";
    out << "import rmap\n";
    out << "\n";
    out << "# 1. Verify global variables match Inja template context\n";
    out << "assert name == 'spi', f'Expected name spi, got {name}'\n";
    out << "assert project_name == 'SPI_Controller', f'Expected project_name SPI_Controller, got {project_name}'\n";
    out << "assert project_version == '2.1.0', f'Expected project_version 2.1.0, got {project_version}'\n";
    out << "assert reg_width == 32, f'Expected reg_width 32, got {reg_width}'\n";
    out << "assert reg_width_bytes == 4, f'Expected reg_width_bytes 4, got {reg_width_bytes}'\n";
    out << "assert custom_key == 'custom_val', f'Expected custom_val, got {custom_key}'\n";
    out << "assert len(blocks) == 1, f'Expected 1 block, got {len(blocks)}'\n";
    out << "assert blocks[0]['name'] == 'SPI0', f'Expected SPI0, got {blocks[0][\"name\"]}'\n";
    out << "assert len(blocks[0]['registers']) == 1, 'Expected 1 register'\n";
    out << "assert blocks[0]['registers'][0]['name'] == 'CR', 'Expected CR'\n";
    out << "\n";
    out << "# 2. Verify dictionary containers (data, context, rmap, regmap)\n";
    out << "assert data['name'] == 'spi'\n";
    out << "assert context['project_name'] == 'SPI_Controller'\n";
    out << "assert rmap.name == 'spi'\n";
    out << "assert regmap['reg_width'] == 32\n";
    out << "\n";
    out << "# 3. Verify helper functions\n";
    out << "assert to_hex(16, 4) == '0x0010'\n";
    out << "assert to_dec('0x10') == '16'\n";
    out << "assert bitmask(8, 0) == '0xFF'\n";
    out << "\n";
    out << "# 4. Verify sys.argv[1] has JSON context file\n";
    out << "assert len(sys.argv) >= 2\n";
    out << "assert os.path.exists(sys.argv[1])\n";
    out << "with open(sys.argv[1], 'r', encoding='utf-8') as f:\n";
    out << "    loaded = json.load(f)\n";
    out << "    assert loaded['name'] == 'spi'\n";
    out << "\n";
    out << "# 5. Verify environment variables\n";
    out << "assert os.environ.get('RMAP_NAME') == 'spi'\n";
    out << "assert os.environ.get('RMAP_PROJECT_NAME') == 'SPI_Controller'\n";
    out << "assert os.environ.get('RMAP_JSON_FILE') is not None\n";
    out << "\n";
    out << "# 6. Write success marker\n";
    out << "with open('work/test_python_exec/success.marker', 'w') as f:\n";
    out << "    f.write(f'PASS: {name}_{project_name}')\n";
    out << "print('Python test output for CodeGenerator')\n";
    scriptFile.close();

    // Prepare JSON data identical to Inja template input
    json testJson;
    testJson["name"] = "spi";
    testJson["project_name"] = "SPI_Controller";
    testJson["project_version"] = "2.1.0";
    testJson["reg_width"] = 32;
    testJson["custom_key"] = "custom_val";

    json reg;
    reg["name"] = "CR";
    reg["offset_lsb"] = 0;
    reg["offset_hex"] = "0x00";
    reg["description"] = "Control Register";

    json blk;
    blk["name"] = "SPI0";
    blk["offset"] = 0;
    blk["offset_hex"] = "0x0000";
    blk["registers"] = json::array({reg});
    testJson["blocks"] = json::array({blk});

    std::vector<TemplateMapping> mappings;
    mappings.push_back({"templates/c/reg_map.h.inja", "work/test_python_exec/reg_map.h"});

    CodeGenerator cg;
    GenerationReport report = cg.generate(
        testJson,
        "templates",
        "work/test_python_exec",
        mappings,
        QDir::currentPath().toStdString(),
        scriptPath.toStdString()
    );

    QVERIFY2(!report.has_errors(), report.errors.empty() ? "" : report.errors[0].second.c_str());
    QVERIFY(QFile::exists("work/test_python_exec/reg_map.h"));
    QVERIFY(QFile::exists("work/test_python_exec/success.marker"));

    QFile marker("work/test_python_exec/success.marker");
    QVERIFY(marker.open(QIODevice::ReadOnly | QIODevice::Text));
    QString markerContent = QString::fromUtf8(marker.readAll());
    marker.close();
    QCOMPARE(markerContent, QString("PASS: spi_SPI_Controller"));

    // Also verify report includes Python script in success_files
    bool foundPySuccess = false;
    for (const auto &f : report.success_files) {
        if (f.find("verify_vars.py") != std::string::npos) {
            foundPySuccess = true;
            break;
        }
    }
    QVERIFY(foundPySuccess);

    // 2. Test failure handling when script exits with error
    QString failScriptPath = "work/test_python_exec/fail_script.py";
    QFile failFile(failScriptPath);
    QVERIFY(failFile.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream failOut(&failFile);
    failOut << "import sys\n";
    failOut << "sys.stderr.write('Intentional script failure test\\n')\n";
    failOut << "sys.exit(42)\n";
    failFile.close();

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*Python script execution failed: Intentional script failure test.*"));
    GenerationReport failReport = cg.generate(
        testJson,
        "templates",
        "work/test_python_exec",
        mappings,
        QDir::currentPath().toStdString(),
        failScriptPath.toStdString()
    );

    QVERIFY(failReport.has_errors());
    QCOMPARE(failReport.errors.size(), (size_t)1);
    QVERIFY(QString::fromStdString(failReport.errors[0].second).contains("Intentional script failure test"));

    // 3. Test error handling when script file does not exist
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*Python script execution failed: Python script file does not exist.*"));
    GenerationReport missingReport = cg.generate(
        testJson,
        "templates",
        "work/test_python_exec",
        mappings,
        QDir::currentPath().toStdString(),
        "work/test_python_exec/does_not_exist.py"
    );
    QVERIFY(missingReport.has_errors());
    QVERIFY(QString::fromStdString(missingReport.errors[0].second).contains("does not exist"));
}

void TestCodeGenerator::testErrorRecoveryAndInvalidTemplates()
{
    CodeGenerator cg;
    json data = json::object();
    data["name"] = "test_block";

    // 1. Template file does not exist in mappings
    std::vector<TemplateMapping> mappings = {
        {"templates/non_existent_tmpl.inja", "work/cg_err/out.txt"}
    };
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*Template file does not exist: templates/non_existent_tmpl\\.inja.*"));
    GenerationReport rep = cg.generate(data, "templates", "work/cg_err", mappings);
    QVERIFY(rep.has_errors());
    QVERIFY(rep.errors[0].second.find("Template file does not exist") != std::string::npos);

    // 2. Inja syntax error during template parsing / rendering
    QDir().mkpath("work/cg_err");
    QFile badTmpl("work/cg_err/bad_syntax.inja");
    QVERIFY(badTmpl.open(QIODevice::WriteOnly | QIODevice::Text));
    badTmpl.write("{% if %}\n{{ undefined_fn(1, 2) }}\n");
    badTmpl.close();

    std::vector<TemplateMapping> badMappings = {
        {"work/cg_err/bad_syntax.inja", "work/cg_err/bad_out.txt"}
    };
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*Template rendering error:.*unknown function undefined_fn.*"));
    GenerationReport badRep = cg.generate(data, "templates", "work/cg_err", badMappings, QDir::currentPath().toStdString());
    QVERIFY(badRep.has_errors());
    QVERIFY(badRep.errors[0].second.find("Template rendering error") != std::string::npos);

    // 3. Empty mappings triggers parseDirectory fallback
    data["blocks"] = json::array({
        {
            {"name", "ctrl_block"},
            {"offset", 0},
            {"registers", json::array()}
        }
    });
    GenerationReport emptyMapRep = cg.generate(data, "templates/c", "work/cg_empty_map", {});
    QVERIFY(!emptyMapRep.has_errors());
    QVERIFY(!emptyMapRep.success_files.empty());
}

void TestCodeGenerator::testHelpersExtendedEdgeCases()
{
    CodeGenerator cg;
    json data = json::object();
    data["name"] = "test_block";

    QDir().mkpath("work/cg_helpers");
    QFile tmpl("work/cg_helpers/edge.inja");
    QVERIFY(tmpl.open(QIODevice::WriteOnly | QIODevice::Text));
    tmpl.write(
        "upper_q={{ upper(\"quoted\") }}\n"
        "lower_q={{ lower(\"QUOTED\") }}\n"
        "hex_str={{ to_hex(\"0x30\", 4) }}\n"
        "hex_bad={{ to_hex(\"not_a_num\", 4) }}\n"
        "dec_str={{ to_dec(\"123\") }}\n"
        "dec_bad={{ to_dec(\"not_a_num\") }}\n"
        "mask_64={{ bitmask(64, 0) }}\n"
        "mask_big_lsb={{ bitmask(32, 70) }}\n"
        "pad_str={{ pad_zero(\"42\", 4) }}\n"
        "pad_bad={{ pad_zero(\"not_a_num\", 4) }}\n"
        "snake_dash={{ snake_case(\"foo-bar baz\") }}\n"
        "snake_camel={{ snake_case(\"camelCaseWord\") }}\n"
        "sv_hex_str={{ sv_hex(\"0x123\", \"16\") }}\n"
        "sv_hex_quoted={{ sv_hex(\"\\\"0x1234\\\"\", \"16\") }}\n"
        "sv_hex_bad={{ sv_hex(\"not_num\", \"invalid\") }}\n"
        "sv_hex_no_width={{ sv_hex(255) }}\n"
        "sv_hex_zero_width={{ sv_hex(255, 0) }}\n"
        "hex_bad_width={{ to_hex(48, \"bad_width\") }}\n"
        "mask_bad={{ bitmask(\"bad_w\", \"bad_l\") }}\n"
        "mask_0={{ bitmask(0, 0) }}\n"
        "pad_bad_width={{ pad_zero(42, \"bad_width\") }}\n"
        "ctype_bad={{ c_type(\"bad\") }}\n"
        "ctype_8={{ c_type(8) }}\n"
        "ctype_16={{ c_type(16) }}\n"
        "ctype_32={{ c_type(32) }}\n"
        "ctype_64={{ c_type(64) }}\n"
        "msb_bad={{ msb(\"bad0\", \"bad1\") }}\n"
        "msb_0={{ msb(0, 0) }}\n"
        "sv_hex_64={{ sv_hex(1, 64) }}\n"
        "sv_hex_big={{ sv_hex(131072, 64) }}\n"
        "snake_aB={{ snake_case(\"aB\") }}\n"
        "snake_abcDef={{ snake_case(\"ABCDef\") }}\n"
        "snake_consec={{ snake_case(\"foo--bar  baz\") }}\n"
    );
    tmpl.close();

    std::vector<TemplateMapping> mappings = {
        {"work/cg_helpers/edge.inja", "work/cg_helpers/edge.txt"}
    };
    GenerationReport rep = cg.generate(data, "templates", "work/cg_helpers", mappings, QDir::currentPath().toStdString());
    QVERIFY(!rep.has_errors());

    QFile outFile("work/cg_helpers/edge.txt");
    QVERIFY(outFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = QString::fromUtf8(outFile.readAll());
    outFile.close();
    QVERIFY(content.contains("upper_q=QUOTED"));
    QVERIFY(content.contains("lower_q=quoted"));
    QVERIFY(content.contains("hex_str=0x0030"));
    QVERIFY(content.contains("hex_bad=0x0000"));
    QVERIFY(content.contains("dec_str=123"));
    QVERIFY(content.contains("dec_bad=0"));
    QVERIFY(content.contains("mask_64=0xFFFFFFFFFFFFFFFF"));
    QVERIFY(content.contains("mask_big_lsb=0x0"));
    QVERIFY(content.contains("pad_str=0042"));
    QVERIFY(content.contains("snake_dash=foo_bar_baz"));
    QVERIFY(content.contains("snake_camel=camel_case_word"));
    QVERIFY(content.contains("sv_hex_str=16'h0123"));
    QVERIFY(content.contains("sv_hex_quoted=16'h1234"));
    QVERIFY(content.contains("sv_hex_bad=32'h0000"));
    QVERIFY(content.contains("hex_bad_width=0x30"));
    QVERIFY(content.contains("mask_bad=0x0"));
    QVERIFY(content.contains("mask_0=0x0"));
    QVERIFY(content.contains("pad_bad_width=42"));
    QVERIFY(content.contains("ctype_bad=uint32_t"));
    QVERIFY(content.contains("ctype_8=uint8_t"));
    QVERIFY(content.contains("ctype_16=uint16_t"));
    QVERIFY(content.contains("ctype_32=uint32_t"));
    QVERIFY(content.contains("ctype_64=uint64_t"));
    QVERIFY(content.contains("msb_bad=0"));
    QVERIFY(content.contains("msb_0=0"));
    QVERIFY(content.contains("sv_hex_64=64'h0001"));
    QVERIFY(content.contains("sv_hex_big=64'h0000000000020000"));
    QVERIFY(content.contains("snake_aB=a_b"));
    QVERIFY(content.contains("snake_abcDef=abc_def"));
    QVERIFY(content.contains("snake_consec=foo_bar_baz"));
}

void TestCodeGenerator::testLegacyAndDirectoryMethods()
{
    CodeGenerator cg;
    json data = json::object();
    data["name"] = "legacy_test";
    data["blocks"] = json::array({
        {
            {"name", "ctrl"},
            {"offset", 0},
            {"registers", json::array()}
        }
    });

    // 1. parse() legacy method
    cg.parse(data, "templates/c", "work/cg_legacy_parse");

    // 2. parseCustom() legacy method
    std::vector<TemplateMapping> mappings = {
        {"templates/c/reg_map.h.inja", "work/cg_legacy_custom/reg_map.h"}
    };
    cg.parseCustom(data, "templates", mappings);

    // 3. parseDirectory with non-existent directory
    GenerationReport repNonEx = cg.parseDirectory(data, "non_existent_folder_xyz_123", "work/cg_out");
    QVERIFY(repNonEx.has_errors());
    QVERIFY(repNonEx.errors[0].second.find("Template directory does not exist") != std::string::npos);

    // 4. parseDirectory with empty directory
    QDir().mkpath("work/cg_empty_tmpl_dir");
    GenerationReport repEmpty = cg.parseDirectory(data, "work/cg_empty_tmpl_dir", "work/cg_out");
    QVERIFY(!repEmpty.has_errors());
    QVERIFY(repEmpty.success_files.empty());

    // 4b. parseDirectory with empty directory and python_script (covers line 553)
    QDir().mkpath("work/cg_other_dir");
    QFile dummyScript("work/cg_other_dir/run.py");
    QVERIFY(dummyScript.open(QIODevice::WriteOnly | QIODevice::Text));
    dummyScript.write("print('empty dir script')\n");
    dummyScript.close();
    GenerationReport repEmptyPy = cg.parseDirectory(data, "work/cg_empty_tmpl_dir", "work/cg_out", "", dummyScript.fileName().toStdString());
    QVERIFY(!repEmptyPy.has_errors());
    QVERIFY(!repEmptyPy.success_files.empty());

    // 5. Template ending in .tmpl
    QFile tmplFile("work/cg_empty_tmpl_dir/sample.tmpl");
    QVERIFY(tmplFile.open(QIODevice::WriteOnly | QIODevice::Text));
    tmplFile.write("Hello {{ name }}!\n");
    tmplFile.close();

    GenerationReport repTmpl = cg.parseDirectory(data, "work/cg_empty_tmpl_dir", "work/cg_tmpl_out");
    QVERIFY(!repTmpl.has_errors());
    QVERIFY(QFile::exists("work/cg_tmpl_out/sample"));

    // 6. Empty template path returns empty string
    std::vector<TemplateMapping> emptyTmplMapping = {{"", "work/cg_empty_tmpl_out.txt"}};
    cg.generate(data, "templates", "work/cg_empty_map", emptyTmplMapping);

    // 7. Empty template folder & output folder in parseDirectory
    cg.parseDirectory(data, "", "");

    // 8. Register padding when regOffset <= currentOffset, and reg_width == 0
    json padData = json::object();
    padData["name"] = "pad_test";
    padData["reg_width"] = 0;
    padData["blocks"] = json::array({
        {
            {"name", "b1"},
            {"offset", 0},
            {"registers", json::array({
                {{"name", "r1"}, {"offset", 0}},
                {{"name", "r2"}, {"offset", 0}}
            })}
        }
    });
    cg.generate(padData, "templates/c", "work/cg_pad_test", {{"templates/c/reg_map.h.inja", "work/cg_pad_test/reg_map.h"}});
}

void TestCodeGenerator::testPythonRunnerEdgeCases()
{
    CodeGenerator cg;
    json data = json::object();
    std::string err;

    // 1. Empty python script path
    bool okEmpty = cg.runPythonScript("", data, "", nullptr, &err);
    QVERIFY(!okEmpty);
    QCOMPARE(err, std::string("No Python script specified."));

    // 2. Non-existent python script file
    bool okNonEx = cg.runPythonScript("non_existent_script_xyz.py", data, "", nullptr, &err);
    QVERIFY(!okNonEx);
    QVERIFY(err.find("Python script file does not exist") != std::string::npos);

    // 3. Invalid python executable
    qputenv("RMAP_PYTHON", "/non/existent/bin/python_xyz_12345");
    QDir().mkpath("work/cg_py_test");
    QFile scriptFile("work/cg_py_test/test.py");
    QVERIFY(scriptFile.open(QIODevice::WriteOnly | QIODevice::Text));
    scriptFile.write("print('ok')\n");
    scriptFile.close();

    bool okBadExe = cg.runPythonScript(scriptFile.fileName().toStdString(), data, "", nullptr, &err);
    QVERIFY(!okBadExe);
    qunsetenv("RMAP_PYTHON");

    // 4. Missing python interpreter in PATH (lines 596, 599-600)
    qunsetenv("RMAP_PYTHON");
    qunsetenv("PYTHON");
    QByteArray savedPath = qgetenv("PATH");
    qputenv("PATH", "/nonexistent_bin_dir_12345");
    bool okNoPy = cg.runPythonScript(scriptFile.fileName().toStdString(), data, "", nullptr, &err);
    qputenv("PATH", savedPath);
    QVERIFY(!okNoPy);
    QVERIFY(err.find("Python interpreter ('python3' or 'python') not found in system PATH.") != std::string::npos);

    // 5. Temporary JSON file failure via RMAP_TMPDIR (lines 607-608)
    qputenv("RMAP_TMPDIR", "/dev/null/not_a_dir_12345");
    bool okBadTmp = cg.runPythonScript(scriptFile.fileName().toStdString(), data, "", nullptr, &err);
    qunsetenv("RMAP_TMPDIR");
    QVERIFY(!okBadTmp);
    QVERIFY(err.find("Failed to create temporary file for register map JSON context.") != std::string::npos);

    // 6. Python script timeout (lines 718-721)
    QFile sleepScript("work/cg_py_test/sleep.py");
    QVERIFY(sleepScript.open(QIODevice::WriteOnly | QIODevice::Text));
    sleepScript.write("import time\ntime.sleep(2)\n");
    sleepScript.close();

    qputenv("RMAP_PYTHON_TIMEOUT", "100");
    bool okTimeout = cg.runPythonScript(sleepScript.fileName().toStdString(), data, "", nullptr, &err);
    qunsetenv("RMAP_PYTHON_TIMEOUT");
    QVERIFY(!okTimeout);
    QVERIFY(err.find("Python script execution timed out") != std::string::npos);

    // 7. Successful execution with empty base_dir (workDir fallback)
    std::string outStr;
    bool okSuccessEmpty = cg.runPythonScript(scriptFile.fileName().toStdString(), data, "", &outStr, &err);
    QVERIFY(okSuccessEmpty);
    QVERIFY(outStr.find("ok") != std::string::npos);

    // 8. Successful execution with non-existent base_dir (workDir fallback to script dir)
    bool okSuccessNonExDir = cg.runPythonScript(scriptFile.fileName().toStdString(), data, "/non_existent_folder_xyz_12345", &outStr, &err);
    QVERIFY(okSuccessNonExDir);

    // 9. Successful execution with existing base_dir
    bool okSuccessWorkDir = cg.runPythonScript(scriptFile.fileName().toStdString(), data, "work/cg_py_test", &outStr, &err);
    QVERIFY(okSuccessWorkDir);

    // 10. Python script non-zero exit code
    QFile failScript("work/cg_py_test/fail.py");
    QVERIFY(failScript.open(QIODevice::WriteOnly | QIODevice::Text));
    failScript.write("import sys\nsys.stderr.write('fatal script error\\n')\nsys.exit(42)\n");
    failScript.close();

    bool okFail = cg.runPythonScript(failScript.fileName().toStdString(), data, "", &outStr, &err);
    QVERIFY(!okFail);
    QVERIFY(err.find("fatal script error") != std::string::npos);

    // 10b. Python script failure with empty stderr (pyErr empty, error on stdout instead)
    QFile failStdoutScript("work/cg_py_test/fail_stdout.py");
    QVERIFY(failStdoutScript.open(QIODevice::WriteOnly | QIODevice::Text));
    failStdoutScript.write("import sys\nsys.stdout.write('error on stdout\\n')\nsys.exit(1)\n");
    failStdoutScript.close();

    bool okFailStdout = cg.runPythonScript(failStdoutScript.fileName().toStdString(), data, "", &outStr, &err);
    QVERIFY(!okFailStdout);

    // 10c. runPythonScript with nullptr stdout and stderr pointers
    cg.runPythonScript("", data, "", nullptr, nullptr);
    cg.runPythonScript("non_existent_xyz.py", data, "", nullptr, nullptr);
    cg.runPythonScript(scriptFile.fileName().toStdString(), data, "", nullptr, nullptr);

    // 10d. RMAP_PYTHON_TIMEOUT_MS env var
    qputenv("RMAP_PYTHON_TIMEOUT_MS", "5000");
    bool okTimeoutMs = cg.runPythonScript(scriptFile.fileName().toStdString(), data, "", &outStr, &err);
    qunsetenv("RMAP_PYTHON_TIMEOUT_MS");
    QVERIFY(okTimeoutMs);

    // 11. Template categorization pre-scan mappings: rtl, rtl_tb, uvm, uvm_tb, sim dir, sim file
    {
        json root;
        root["name"] = "SCAN_TEST";
        root["reg_width"] = 32;

        QDir().mkpath("work/scan_test/rtl");
        QDir().mkpath("work/scan_test/rtl_tb");
        QDir().mkpath("work/scan_test/uvm");
        QDir().mkpath("work/scan_test/uvm_tb");
        QDir().mkpath("work/scan_test/sim");

        QFile fRtlTmpl("work/scan_test/rtl/reg_map.sv.inja");
        fRtlTmpl.open(QIODevice::WriteOnly); fRtlTmpl.write("// rtl\n"); fRtlTmpl.close();

        QFile fRtlTbTmpl("work/scan_test/rtl_tb/tb_reg_map.sv.inja");
        fRtlTbTmpl.open(QIODevice::WriteOnly); fRtlTbTmpl.write("// rtl_tb\n"); fRtlTbTmpl.close();

        QFile fUvmTmpl("work/scan_test/uvm/reg_model.sv.inja");
        fUvmTmpl.open(QIODevice::WriteOnly); fUvmTmpl.write("// uvm\n"); fUvmTmpl.close();

        QFile fUvmTbTmpl("work/scan_test/uvm_tb/tb_reg_model.sv.inja");
        fUvmTbTmpl.open(QIODevice::WriteOnly); fUvmTbTmpl.write("// uvm_tb\n"); fUvmTbTmpl.close();

        QFile fSimTmpl("work/scan_test/sim/Makefile.inja");
        fSimTmpl.open(QIODevice::WriteOnly); fSimTmpl.write("RTL_REL = {{ sim_rel_rtl_path }}\nUVM_REL = {{ sim_rel_uvm_path }}\n"); fSimTmpl.close();

        std::vector<TemplateMapping> scanMappings = {
            {"work/scan_test/rtl/reg_map.sv.inja", "work/scan_test/out_rtl/reg_map.sv"},
            {"work/scan_test/rtl_tb/tb_reg_map.sv.inja", "work/scan_test/out_rtl_tb/tb_reg_map.sv"},
            {"work/scan_test/uvm/reg_model.sv.inja", "work/scan_test/out_uvm/reg_model.sv"},
            {"work/scan_test/uvm_tb/tb_reg_model.sv.inja", "work/scan_test/out_uvm_tb/tb_reg_model.sv"},
            {"work/scan_test/sim/Makefile.inja", "work/scan_test/sim/"},
            {"work/scan_test/sim/Makefile.inja", "work/scan_test/sim/Makefile"}
        };

        GenerationReport scanReport = cg.generate(root, "work/scan_test", "work/scan_test/out", scanMappings);
        QVERIFY(!scanReport.has_errors());
    }
}

void TestCodeGenerator::testCommandLineInterface()
{
    QString rmapBin = QDir("build/bin/rmap").absolutePath();
    if (!QFile::exists(rmapBin)) {
        rmapBin = QDir("bin/rmap").absolutePath();
    }
    if (!QFile::exists(rmapBin)) {
        rmapBin = QDir("../bin/rmap").absolutePath();
    }
    if (!QFile::exists(rmapBin)) {
        QSKIP("rmap binary not available for CLI tests");
    }

    auto runRmap = [&](const QStringList &args, const QProcessEnvironment *customEnv = nullptr) -> QPair<int, QString> {
        QProcess proc;
        proc.setProcessChannelMode(QProcess::MergedChannels);
        QProcessEnvironment env = customEnv ? *customEnv : QProcessEnvironment::systemEnvironment();
        env.insert("QT_QPA_PLATFORM", "offscreen");
        proc.setProcessEnvironment(env);
        proc.start(rmapBin, args);
        proc.waitForFinished(10000);
        return {proc.exitCode(), QString::fromUtf8(proc.readAll())};
    };

    // 1. --help
    auto [codeHelp, outHelp] = runRmap({"--help"});
    QCOMPARE(codeHelp, 0);
    QVERIFY(outHelp.contains("Usage:"));

    // 2. --version
    auto [codeVer, outVer] = runRmap({"--version"});
    QCOMPARE(codeVer, 0);
    QVERIFY(outVer.contains("rmap"));

    // 3. Headless mode auto-detection when neither DISPLAY nor WAYLAND_DISPLAY is set
    {
        QProcessEnvironment noDispEnv = QProcessEnvironment::systemEnvironment();
        noDispEnv.remove("DISPLAY");
        noDispEnv.remove("WAYLAND_DISPLAY");
        auto [codeNoDisp, outNoDisp] = runRmap({"-f", "examples/rmt/peripherals/spi.rmt", "-l"}, &noDispEnv);
        QCOMPARE(codeNoDisp, 0);
    }

    // 4. Semantic diff CLI mode (-d)
    {
        auto [codeDiff, outDiff] = runRmap({"-f", "examples/rmt/peripherals/spi.rmt", "-d", "examples/rmt/peripherals/spi.rmt",
                                            "--report-format", "markdown", "-o", "work/cli_diff.md"});
        QCOMPARE(codeDiff, 0);
        QVERIFY(QFile::exists("work/cli_diff.md"));
    }

    // 5. Theme and Language options with lint (-t, --lang, -l)
    {
        auto [codeThemeLang, outThemeLang] = runRmap({"-f", "examples/rmt/peripherals/spi.rmt", "-t", "dracula", "--lang", "es", "-l"});
        QCOMPARE(codeThemeLang, 0);
    }

    // 6. Strict linting mode (--strict)
    {
        auto [codeStrictFail, outStrictFail] = runRmap({"-f", "examples/rmt/features/wide_bus_64bit.rmt", "-l", "--strict"});
        QCOMPARE(codeStrictFail, 1);
    }

    // 7. Conversion mode (-c)
    {
        auto [codeConv, outConv] = runRmap({"-f", "examples/rmt/peripherals/spi.rmt", "-c", "work/cli_conv.svd"});
        QCOMPARE(codeConv, 0);
        QVERIFY(QFile::exists("work/cli_conv.svd"));
    }

    // 8. Headless export mode (-e, -o)
    {
        auto [codeExp, outExp] = runRmap({"-f", "examples/rmt/peripherals/spi.rmt", "-e", "-o", "work/cli_exp_dir"});
        QCOMPARE(codeExp, 0);
        QVERIFY(QDir("work/cli_exp_dir").exists());
    }

    // 9. Short and long option variants (-h, -v, --export, --convert, --lint, --diff)
    {
        auto [codeH, outH] = runRmap({"-h"});
        QCOMPARE(codeH, 0);
        QVERIFY(outH.contains("Usage:"));

        auto [codeV, outV] = runRmap({"-v"});
        QCOMPARE(codeV, 0);
        QVERIFY(outV.contains("rmap"));

        auto [codeExpLong, outExpLong] = runRmap({"-f", "examples/rmt/peripherals/spi.rmt", "--export", "--out", "work/cli_exp_long"});
        QCOMPARE(codeExpLong, 0);

        auto [codeConvLong, outConvLong] = runRmap({"-f", "examples/rmt/peripherals/spi.rmt", "--convert", "work/cli_conv_long.svd"});
        QCOMPARE(codeConvLong, 0);

        auto [codeLintLong, outLintLong] = runRmap({"-f", "examples/rmt/peripherals/spi.rmt", "--lint"});
        QCOMPARE(codeLintLong, 0);

        auto [codeDiffLong, outDiffLong] = runRmap({"-f", "examples/rmt/peripherals/spi.rmt", "--diff", "examples/rmt/peripherals/spi.rmt", "--report-format", "text"});
        QCOMPARE(codeDiffLong, 0);
    }

    // 10. Alternative theme and language option flags (--theme, --colour-scheme, --color-scheme, --language)
    {
        auto [c1, o1] = runRmap({"-f", "examples/rmt/peripherals/spi.rmt", "--theme", "nord", "--lint"});
        QCOMPARE(c1, 0);

        auto [c2, o2] = runRmap({"-f", "examples/rmt/peripherals/spi.rmt", "--colour-scheme", "solarized8_light", "--lint"});
        QCOMPARE(c2, 0);

        auto [c3, o3] = runRmap({"-f", "examples/rmt/peripherals/spi.rmt", "--color-scheme", "monokai", "--language", "pt_BR", "--lint"});
        QCOMPARE(c3, 0);
    }

    // 11. CLI error branches (diff failure, export failure without file)
    {
        auto [codeDiffFail, outDiffFail] = runRmap({"-f", "examples/rmt/peripherals/spi.rmt", "--diff", "nonexistent_diff_file.rmt"});
        QCOMPARE(codeDiffFail, 1);

        auto [codeExpFail, outExpFail] = runRmap({"--export"});
        QCOMPARE(codeExpFail, 1);
    }

    // 12. Headless mode when DISPLAY is unset but WAYLAND_DISPLAY is set
    {
        QProcessEnvironment waylandEnv = QProcessEnvironment::systemEnvironment();
        waylandEnv.remove("DISPLAY");
        waylandEnv.insert("WAYLAND_DISPLAY", "wayland-0");
        auto [codeWayland, outWayland] = runRmap({"-f", "examples/rmt/peripherals/spi.rmt", "-l"}, &waylandEnv);
        QCOMPARE(codeWayland, 0);
    }

    // 13. Additional CLI option permutations (--help, --version, -c, -d, --strict, report formats)
    {
        auto [codeHelpLong, outHelpLong] = runRmap({"--help"});
        QCOMPARE(codeHelpLong, 0);

        auto [codeVerLong, outVerLong] = runRmap({"--version"});
        QCOMPARE(codeVerLong, 0);

        auto [codeConvShort, outConvShort] = runRmap({"-f", "examples/rmt/peripherals/spi.rmt", "-c", "work/cli_conv_short.svd"});
        QCOMPARE(codeConvShort, 0);

        auto [codeDiffShort, outDiffShort] = runRmap({"-f", "examples/rmt/peripherals/spi.rmt", "-d", "examples/rmt/peripherals/spi.rmt"});
        QCOMPARE(codeDiffShort, 0);

        auto [codeLintStrict, outLintStrict] = runRmap({"-f", "examples/rmt/peripherals/spi.rmt", "--lint", "--strict"});
        QCOMPARE(codeLintStrict, 0);

        auto [codeLintJson, outLintJson] = runRmap({"-f", "examples/rmt/peripherals/spi.rmt", "--lint", "--report-format", "json", "-o", "work/cli_lint.json"});
        QCOMPARE(codeLintJson, 0);
        QVERIFY(QFile::exists("work/cli_lint.json"));

        auto [codeLintSarif, outLintSarif] = runRmap({"-f", "examples/rmt/peripherals/spi.rmt", "--lint", "--report-format", "sarif", "-o", "work/cli_lint.sarif"});
        QCOMPARE(codeLintSarif, 0);
        QVERIFY(QFile::exists("work/cli_lint.sarif"));

        auto [codeLintJunit, outLintJunit] = runRmap({"-f", "examples/rmt/peripherals/spi.rmt", "--lint", "--report-format", "junit", "-o", "work/cli_lint.junit"});
        QCOMPARE(codeLintJunit, 0);
        QVERIFY(QFile::exists("work/cli_lint.junit"));

        auto [codeDiffMd, outDiffMd] = runRmap({"-f", "examples/rmt/peripherals/spi.rmt", "-d", "examples/rmt/peripherals/spi.rmt", "--report-format", "markdown", "-o", "work/cli_diff.md"});
        QCOMPARE(codeDiffMd, 0);
        QVERIFY(QFile::exists("work/cli_diff.md"));

        auto [codeExpDefault, outExpDefault] = runRmap({"-f", "examples/rmt/peripherals/spi.rmt", "--export"});
        QCOMPARE(codeExpDefault, 0);

        auto [codeLintFail, outLintFail] = runRmap({"-f", "examples/rmt/validation/invalid_overlap.rmt", "--lint"});
        QCOMPARE(codeLintFail, 1);

        // Language-only CLI option without theme (line 109-111 in main.cpp)
        auto [codeLangOnly, outLangOnly] = runRmap({"-f", "examples/rmt/peripherals/spi.rmt", "--lang", "es", "-l"});
        QCOMPARE(codeLangOnly, 0);

        // DISPLAY set and WAYLAND_DISPLAY set
        QProcessEnvironment dispWaylandEnv = QProcessEnvironment::systemEnvironment();
        dispWaylandEnv.insert("DISPLAY", ":99");
        dispWaylandEnv.insert("WAYLAND_DISPLAY", "wayland-0");
        auto [cDW, oDW] = runRmap({"-f", "examples/rmt/peripherals/spi.rmt", "-l"}, &dispWaylandEnv);
        QCOMPARE(cDW, 0);

        // DISPLAY set and WAYLAND_DISPLAY unset
        QProcessEnvironment dispOnlyEnv = QProcessEnvironment::systemEnvironment();
        dispOnlyEnv.insert("DISPLAY", ":99");
        dispOnlyEnv.remove("WAYLAND_DISPLAY");
        auto [cDO, oDO] = runRmap({"-f", "examples/rmt/peripherals/spi.rmt", "-l"}, &dispOnlyEnv);
        QCOMPARE(cDO, 0);

        // Diff with different files
        auto [codeDiffDiff, outDiffDiff] = runRmap({"-f", "examples/rmt/peripherals/spi.rmt", "-d", "examples/rmt/peripherals/uart.rmt"});
        QCOMPARE(codeDiffDiff, 0);
    }

    // 13. Interactive GUI startup and clean SIGTERM shutdown (exercises mainWin->show() and app.exec())
    {
        QProcess proc;
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("QT_QPA_PLATFORM", "offscreen");
        proc.setProcessEnvironment(env);
        proc.start(rmapBin, QStringList());
        QVERIFY(proc.waitForStarted(5000));
        QThread::msleep(300);
        proc.terminate();
        if (!proc.waitForFinished(5000)) {
            proc.kill();
            proc.waitForFinished(2000);
        }
        QVERIFY(proc.state() == QProcess::NotRunning);
    }

    // 14. Interactive GUI startup and clean SIGINT shutdown
    {
        QProcess proc;
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("QT_QPA_PLATFORM", "offscreen");
        proc.setProcessEnvironment(env);
        proc.start(rmapBin, QStringList());
        QVERIFY(proc.waitForStarted(5000));
        QThread::msleep(300);
        ::kill(static_cast<pid_t>(proc.processId()), SIGINT);
        if (!proc.waitForFinished(5000)) {
            proc.kill();
            proc.waitForFinished(2000);
        }
        QVERIFY(proc.state() == QProcess::NotRunning);
    }

    // 15. CLI convert failure and export failure
    {
        auto [codeConvFail, outConvFail] = runRmap({"-f", "examples/rmt/peripherals/spi.rmt", "-c", "/dev/null/cannot_write/out.svd"});
        QCOMPARE(codeConvFail, 1);

        auto [codeExpShortFail, outExpShortFail] = runRmap({"-e"});
        QCOMPARE(codeExpShortFail, 1);
    }
}

QTEST_MAIN(TestCodeGenerator)
#include "test_CodeGenerator.moc"
