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
    mappings.push_back({"templates/reg_map.h.inja", "work/test_c_header.h"});

    GenerationReport report = cg.generate(root, "./templates", "./work", mappings);
    QVERIFY(!report.has_errors());
    QCOMPARE(report.success_files.size(), (size_t)1);

    QFile out("work/test_c_header.h");
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
    mappings.push_back({"templates/uvm_reg_model.sv.inja", "work/test_uvm_model.sv"});

    GenerationReport report = cg.generate(root, "./templates", "./work", mappings);
    QVERIFY(!report.has_errors());

    QFile out("work/test_uvm_model.sv");
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
    mappings.push_back({"templates/reg_map.h.inja", "work/multi_out1.h"});
    mappings.push_back({"templates/uvm_reg_model.sv.inja", "work/multi_out2.sv"});

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
    mappings.push_back({"templates/generic_reg_file.sv.inja", "work/spi_core_reg_file.sv"});

    GenerationReport report = cg.generate(root, "./templates", "./work", mappings);
    QVERIFY(!report.has_errors());

    QFile out("work/spi_core_reg_file.sv");
    const bool outOpened = out.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(outOpened);
    QString content = out.readAll();
    out.close();

    QVERIFY(content.contains("module spi_core_reg_file"));
    QVERIFY(content.contains("input  logic                      clk_i"));
    QVERIFY(content.contains("input  logic                      rst_ni"));
    QVERIFY(content.contains("input  logic                      wr_en_i"));
    QVERIFY(content.contains("input  logic                      rd_en_i"));
    QVERIFY(content.contains("input  logic [ADDR_WIDTH-1:0]     addr_i"));
    QVERIFY(content.contains("output logic [DATA_WIDTH-1:0]     rdata_o"));
    QVERIFY(content.contains("output logic                      ready_o"));
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
    mappings.push_back({"templates/reg_map.rs.inja", "work/spi_pac.rs"});

    GenerationReport report = cg.generate(root, "./templates", "./work", mappings);
    QVERIFY(!report.has_errors());

    QFile out("work/spi_pac.rs");
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
    mappings.push_back({"templates/reg_map.py.inja", "work/spi_driver.py"});

    GenerationReport report = cg.generate(root, "./templates", "./work", mappings);
    QVERIFY(!report.has_errors());

    QFile out("work/spi_driver.py");
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
    mappings.push_back({"templates/reg_doc.html.inja", "work/spi_doc.html"});

    GenerationReport report = cg.generate(root, "./templates", "./work", mappings);
    QVERIFY(!report.has_errors());

    QFile out("work/spi_doc.html");
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
    mappings.push_back({"templates/systemrdl_map.rdl.inja", "work/spi_map.rdl"});

    GenerationReport report = cg.generate(root, "./templates", "./work", mappings);
    QVERIFY(!report.has_errors());

    QFile out("work/spi_map.rdl");
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
    mappings.push_back({"templates/ipxact_map.xml.inja", "work/spi_ipxact.xml"});

    GenerationReport report = cg.generate(root, "./templates", "./work", mappings);
    QVERIFY(!report.has_errors());

    QFile out("work/spi_ipxact.xml");
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
    mappings.push_back({"templates/cmsis_svd.xml.inja", "work/spi.svd"});

    GenerationReport report = cg.generate(root, "./templates", "./work", mappings);
    QVERIFY(!report.has_errors());

    QFile out("work/spi.svd");
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
    mappings.push_back({"templates/reg_doc.md.inja", "work/spi_doc.md"});

    GenerationReport report = cg.generate(root, "./templates", "./work", mappings);
    QVERIFY(!report.has_errors());

    QFile out("work/spi_doc.md");
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
    mappings.push_back({"templates/reg_map.json.inja", "work/spi_map.json"});

    GenerationReport report = cg.generate(root, "./templates", "./work", mappings);
    QVERIFY(!report.has_errors());

    QFile out("work/spi_map.json");
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

QTEST_MAIN(TestCodeGenerator)
#include "test_CodeGenerator.moc"
