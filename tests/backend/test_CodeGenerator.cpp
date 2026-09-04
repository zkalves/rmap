#include <QtTest>
#include <QDir>
#include <QFile>
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
    void testFullGenerationCHeader();
    void testFullGenerationUvmModel();
    void testMultiSourceTemplateMappings();
    void testNewNamingAndTypeHelpers();
    void testFullGenerationGenericRtl();
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
                "PASCAL: {{ pascal_case(name) }}\n"
                "SNAKE: {{ snake_case(name) }}\n"
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
    QVERIFY(content.contains("ADDR_CTRL = 0x0;"));
    QVERIFY(content.contains("sw_ctrl_wr_strobe_o"));
    QVERIFY(content.contains("hw_ctrl_enable_o"));
    QVERIFY(content.contains("always_ff @(posedge clk_i or negedge rst_ni)"));
    QVERIFY(content.contains("always_comb begin : proc_ctrl_next"));
    QVERIFY(content.contains("always_comb begin : proc_read_decode"));
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
    QVERIFY(content.contains("localparam logic [31:0] REGMAP_CRC32 = 0xCAFE1234;"));
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

QTEST_MAIN(TestCodeGenerator)
#include "test_CodeGenerator.moc"
