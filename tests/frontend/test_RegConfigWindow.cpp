#include <QtTest>
#include <QSpinBox>
#include <QLineEdit>
#include <QTableWidget>
#include "RegConfigWindow.hpp"
#include "AppSettings.hpp"

class TestRegConfigWindow : public QObject
{
    Q_OBJECT

private slots:
    void testConfigDialogDefaults();
    void testStateModificationAndRows();
    void testSerializationAndDeserialization();
    void testAcceptAndReject();
    void testProjectMetadataAndParameters();
    void testWindowSizePersistence();
    void testBaseDirRelativization();
};

void TestRegConfigWindow::testConfigDialogDefaults()
{
    RegConfigWindow cfgWin;
    auto *regWidthBox = cfgWin.findChild<QSpinBox*>("regWidthSpinBox");
    QVERIFY(regWidthBox != nullptr);
    QCOMPARE(regWidthBox->value(), 32);

    auto *table = cfgWin.findChild<QTableWidget*>("templateTable");
    QVERIFY(table != nullptr);
    QCOMPARE(table->rowCount(), 0);

    QVERIFY(!cfgWin.isModal());
    QVERIFY(!cfgWin.isSizeGripEnabled());
    QVERIFY(cfgWin.windowFlags().testFlag(Qt::Window));
    QVERIFY(cfgWin.windowFlags().testFlag(Qt::WindowMinMaxButtonsHint));
    QVERIFY(cfgWin.windowFlags().testFlag(Qt::WindowCloseButtonHint));
    QCOMPARE(cfgWin.minimumSize(), QSize(600, 450));
}

void TestRegConfigWindow::testStateModificationAndRows()
{
    RegConfigWindow cfgWin;

    auto *regWidthBox = cfgWin.findChild<QSpinBox*>("regWidthSpinBox");
    auto *tmplEdit = cfgWin.findChild<QLineEdit*>("templateFolder");
    auto *outEdit = cfgWin.findChild<QLineEdit*>("outputFolder");
    auto *pyEdit = cfgWin.findChild<QLineEdit*>("pythonScript");
    auto *table = cfgWin.findChild<QTableWidget*>("templateTable");

    regWidthBox->setValue(64);
    tmplEdit->setText("/custom/templates");
    outEdit->setText("/custom/work");
    pyEdit->setText("/custom/script.py");

    cfgWin.addTemplateRow("/custom/templates/t1.inja", "/custom/work/t1.sv");
    cfgWin.addTemplateRow("/custom/templates/t2.inja", "/custom/work/t2.h");
    QCOMPARE(table->rowCount(), 2);
    QCOMPARE(table->item(0, 0)->text(), QString("/custom/templates/t1.inja"));
    QCOMPARE(table->item(0, 1)->text(), QString("/custom/work/t1.sv"));

    // Select row 0 and click remove button
    auto *btnRemove = cfgWin.findChild<QPushButton*>("btnRemoveRow");
    QVERIFY(btnRemove != nullptr);
    table->selectRow(0);
    btnRemove->click();
    QCOMPARE(table->rowCount(), 1);
    QCOMPARE(table->item(0, 0)->text(), QString("/custom/templates/t2.inja"));
}

void TestRegConfigWindow::testSerializationAndDeserialization()
{
    RegConfigWindow cfgWin;
    auto *regWidthBox = cfgWin.findChild<QSpinBox*>("regWidthSpinBox");
    auto *tmplEdit = cfgWin.findChild<QLineEdit*>("templateFolder");
    auto *outEdit = cfgWin.findChild<QLineEdit*>("outputFolder");

    regWidthBox->setValue(64);
    tmplEdit->setText("./my_templates");
    outEdit->setText("./my_outputs");
    cfgWin.addTemplateRow("my_template.inja", "my_output.sv");

    protormap::Config* config = cfgWin.serialize();
    QVERIFY(config != nullptr);
    QCOMPARE(config->reg_width(), (uint32_t)64);
    QCOMPARE(config->templatefolder(), std::string("./my_templates"));
    QCOMPARE(config->outputfolder(), std::string("./my_outputs"));
    QCOMPARE(config->template_outputs_size(), 1);
    QCOMPARE(config->template_outputs(0).template_filename(), std::string("my_template.inja"));
    QCOMPARE(config->template_outputs(0).output_filepath(), std::string("my_output.sv"));

    // Deserialize into a fresh window
    RegConfigWindow cfgWin2;
    cfgWin2.deserialize(*config);

    auto *regWidthBox2 = cfgWin2.findChild<QSpinBox*>("regWidthSpinBox");
    auto *tmplEdit2 = cfgWin2.findChild<QLineEdit*>("templateFolder");
    auto *outEdit2 = cfgWin2.findChild<QLineEdit*>("outputFolder");
    auto *table2 = cfgWin2.findChild<QTableWidget*>("templateTable");

    QCOMPARE(regWidthBox2->value(), 64);
    QCOMPARE(tmplEdit2->text(), QString("./my_templates"));
    QCOMPARE(outEdit2->text(), QString("./my_outputs"));
    QCOMPARE(table2->rowCount(), 1);
    QCOMPARE(table2->item(0, 0)->text(), QString("my_template.inja"));
    QCOMPARE(table2->item(0, 1)->text(), QString("my_output.sv"));

    delete config;
}

void TestRegConfigWindow::testAcceptAndReject()
{
    RegConfigWindow cfgWin;
    auto *regWidthBox = cfgWin.findChild<QSpinBox*>("regWidthSpinBox");
    regWidthBox->setValue(16);

    cfgWin.accept();
    protormap::Config* cfg = cfgWin.serialize();
    QVERIFY(cfg != nullptr);
    QCOMPARE(cfg->reg_width(), (uint32_t)16);
    delete cfg;

    regWidthBox->setValue(32);
    cfgWin.reject();
    // Rejected resets UI back to previous state
    QCOMPARE(regWidthBox->value(), 16);
}

void TestRegConfigWindow::testProjectMetadataAndParameters()
{
    RegConfigWindow cfgWin;
    cfgWin.setProjectName("Custom_SoC");
    cfgWin.setProjectVersion("2.1.0");
    cfgWin.setRegisterWidth(64);

    QCOMPARE(cfgWin.projectName(), QString("Custom_SoC"));
    QCOMPARE(cfgWin.projectVersion(), QString("2.1.0"));
    QCOMPARE(cfgWin.registerWidth(), (uint32_t)64);

    cfgWin.addParameterRow("BUS_TYPE", "AXI4_LITE");
    cfgWin.addParameterRow("CLOCK_FREQ_MHZ", "250");
}

void TestRegConfigWindow::testWindowSizePersistence()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString origPath = AppSettings::instance().configFilePath();
    AppSettings::instance().setConfigFilePath(tempDir.path() + "/test_rmap.conf");

    {
        RegConfigWindow cfgWin;
        cfgWin.resize(700, 500);
        cfgWin.saveWindowStateToSettings();
        QCOMPARE(AppSettings::instance().configWindowSize(), QSize(700, 500));
    }

    {
        RegConfigWindow cfgWin2;
        QCOMPARE(AppSettings::instance().configWindowSize(), QSize(700, 500));
        QCOMPARE(cfgWin2.size(), QSize(700, 500));
    }

    AppSettings::instance().setConfigFilePath(origPath);
}

void TestRegConfigWindow::testBaseDirRelativization()
{
    RegConfigWindow cfgWin;
    cfgWin.setBaseDir("/home/user/project");

    auto *tmplEdit = cfgWin.findChild<QLineEdit*>("templateFolder");
    auto *outEdit = cfgWin.findChild<QLineEdit*>("outputFolder");
    auto *pyEdit = cfgWin.findChild<QLineEdit*>("pythonScript");

    tmplEdit->setText("/home/user/project/templates");
    outEdit->setText("/home/user/project/build/work");
    pyEdit->setText("/home/user/project/scripts/gen.py");

    cfgWin.addTemplateRow("/home/user/project/templates/t1.inja", "/home/user/project/build/work/t1.sv");

    protormap::Config* config = cfgWin.serialize();
    QVERIFY(config != nullptr);

    QCOMPARE(config->templatefolder(), std::string("./templates"));
    QCOMPARE(config->outputfolder(), std::string("./build/work"));
    QCOMPARE(config->pythonscript(), std::string("./scripts/gen.py"));
    QCOMPARE(config->template_outputs_size(), 1);
    QCOMPARE(config->template_outputs(0).template_filename(), std::string("./templates/t1.inja"));
    QCOMPARE(config->template_outputs(0).output_filepath(), std::string("./build/work/t1.sv"));

    delete config;
}

QTEST_MAIN(TestRegConfigWindow)
#include "test_RegConfigWindow.moc"
