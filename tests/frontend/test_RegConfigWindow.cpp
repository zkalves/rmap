#include <QtTest>
#include <QSpinBox>
#include <QLineEdit>
#include <QListWidget>
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
    void testAutoAdjustSizeOnFirstShow();
    void testPathStorageVarieties();
    void testTemplateFoldersListAndScanning();
    void testEnableDisableToggles();
    void testDynamicOutputFolderSync();
};

void TestRegConfigWindow::testConfigDialogDefaults()
{
    RegConfigWindow cfgWin;
    cfgWin.show();
    QCoreApplication::processEvents();

    // Verify window auto-adjusts to fit all controls comfortably
    QVERIFY(cfgWin.width() >= 750);
    QVERIFY(cfgWin.height() >= 650);

    // Verify all template toolbar buttons are fully sized and visible
    const QStringList toolbarButtons = {
        "btnAddTemplateFolder", "btnRemoveTemplateFolder",
        "btnScanTemplates", "btnSyncOutputs", "btnEnableAll", "btnDisableAll",
        "btnAddTemplateFiles", "btnAddRow", "btnBrowseTemplate",
        "btnBrowseOutputFile", "btnBrowseOutputFolderItem",
        "btnRemoveRow", "btnClearAll"
    };
    for (const QString &btnName : toolbarButtons) {
        auto *btn = cfgWin.findChild<QPushButton*>(btnName);
        QVERIFY2(btn != nullptr, qPrintable(QString("Button %1 must exist").arg(btnName)));
        QVERIFY2(btn->isVisible(), qPrintable(QString("Button %1 must be visible").arg(btnName)));
        QVERIFY2(btn->height() >= 24, qPrintable(QString("Button %1 height (%2) must be >= 24").arg(btnName).arg(btn->height())));
    }

    // Verify parameter buttons
    const QStringList paramButtons = {"btnAddParameter", "btnRemoveParameter"};
    for (const QString &btnName : paramButtons) {
        auto *btn = cfgWin.findChild<QPushButton*>(btnName);
        QVERIFY2(btn != nullptr, qPrintable(QString("Button %1 must exist").arg(btnName)));
        QVERIFY2(btn->isVisible(), qPrintable(QString("Button %1 must be visible").arg(btnName)));
        QVERIFY2(btn->height() >= 24, qPrintable(QString("Button %1 height (%2) must be >= 24").arg(btnName).arg(btn->height())));
    }

    // Verify dialog button box
    auto *buttonBox = cfgWin.findChild<QDialogButtonBox*>("buttonBox");
    QVERIFY(buttonBox != nullptr);
    QVERIFY(buttonBox->isVisible());
    QVERIFY(buttonBox->height() >= 20);

    auto *regWidthBox = cfgWin.findChild<QSpinBox*>("regWidthSpinBox");
    QVERIFY(regWidthBox != nullptr);
    QCOMPARE(regWidthBox->value(), 32);

    auto *foldersList = cfgWin.findChild<QListWidget*>("templateFoldersList");
    QVERIFY(foldersList != nullptr);
    QVERIFY(foldersList->count() >= 1);

    auto *table = cfgWin.findChild<QTableWidget*>("templateTable");
    QVERIFY(table != nullptr);
    QCOMPARE(table->columnCount(), 3);
    QCOMPARE(table->rowCount(), 0);

    QVERIFY(!cfgWin.isModal());
    QVERIFY(!cfgWin.isSizeGripEnabled());
    QVERIFY(cfgWin.windowFlags().testFlag(Qt::Window));
    QVERIFY(cfgWin.windowFlags().testFlag(Qt::WindowMinMaxButtonsHint));
    QVERIFY(cfgWin.windowFlags().testFlag(Qt::WindowCloseButtonHint));
    QCOMPARE(cfgWin.minimumSize(), QSize(600, 480));
}

void TestRegConfigWindow::testStateModificationAndRows()
{
    RegConfigWindow cfgWin;

    auto *regWidthBox = cfgWin.findChild<QSpinBox*>("regWidthSpinBox");
    auto *outEdit = cfgWin.findChild<QLineEdit*>("outputFolder");
    auto *pyEdit = cfgWin.findChild<QLineEdit*>("pythonScript");
    auto *table = cfgWin.findChild<QTableWidget*>("templateTable");

    regWidthBox->setValue(64);
    cfgWin.setTemplateFolders(QStringList() << "/custom/templates");
    outEdit->setText("/custom/work");
    pyEdit->setText("/custom/script.py");

    cfgWin.addTemplateRow(true, "/custom/templates/t1.inja", "/custom/work/t1.sv");
    cfgWin.addTemplateRow(false, "/custom/templates/t2.inja", "/custom/work/t2.h");
    QCOMPARE(table->rowCount(), 2);
    QCOMPARE(table->item(0, 0)->checkState(), Qt::Checked);
    QCOMPARE(table->item(0, 1)->text(), QString("/custom/templates/t1.inja"));
    QCOMPARE(table->item(0, 2)->text(), QString("/custom/work/t1.sv"));
    QCOMPARE(table->item(1, 0)->checkState(), Qt::Unchecked);
    QCOMPARE(table->item(1, 1)->text(), QString("/custom/templates/t2.inja"));
    QCOMPARE(table->item(1, 2)->text(), QString("/custom/work/t2.h"));

    // Select row 0 and click remove button
    auto *btnRemove = cfgWin.findChild<QPushButton*>("btnRemoveRow");
    QVERIFY(btnRemove != nullptr);
    table->selectRow(0);
    btnRemove->click();
    QCOMPARE(table->rowCount(), 1);
    QCOMPARE(table->item(0, 1)->text(), QString("/custom/templates/t2.inja"));
}

void TestRegConfigWindow::testSerializationAndDeserialization()
{
    RegConfigWindow cfgWin;
    auto *regWidthBox = cfgWin.findChild<QSpinBox*>("regWidthSpinBox");
    auto *outEdit = cfgWin.findChild<QLineEdit*>("outputFolder");

    regWidthBox->setValue(64);
    cfgWin.setTemplateFolders(QStringList() << "./my_templates" << "./extra_templates");
    outEdit->setText("./my_outputs");
    cfgWin.addTemplateRow(true, "my_template.inja", "my_output.sv");
    cfgWin.addTemplateRow(false, "disabled_template.inja", "disabled_output.sv");

    protormap::Config* config = cfgWin.serialize();
    QVERIFY(config != nullptr);
    QCOMPARE(config->reg_width(), (uint32_t)64);
    QCOMPARE(config->template_folders_size(), 2);
    QCOMPARE(config->template_folders(0), std::string("./my_templates"));
    QCOMPARE(config->template_folders(1), std::string("./extra_templates"));
    QCOMPARE(config->outputfolder(), std::string("./my_outputs"));
    QCOMPARE(config->template_outputs_size(), 2);
    QCOMPARE(config->template_outputs(0).template_filename(), std::string("my_template.inja"));
    QCOMPARE(config->template_outputs(0).output_filepath(), std::string("my_output.sv"));
    QCOMPARE(config->template_outputs(0).enabled(), true);
    QCOMPARE(config->template_outputs(1).template_filename(), std::string("disabled_template.inja"));
    QCOMPARE(config->template_outputs(1).enabled(), false);

    // Deserialize into a fresh window
    RegConfigWindow cfgWin2;
    cfgWin2.deserialize(*config);

    auto *regWidthBox2 = cfgWin2.findChild<QSpinBox*>("regWidthSpinBox");
    auto *outEdit2 = cfgWin2.findChild<QLineEdit*>("outputFolder");
    auto *table2 = cfgWin2.findChild<QTableWidget*>("templateTable");

    QCOMPARE(regWidthBox2->value(), 64);
    QCOMPARE(cfgWin2.templateFolders().size(), 2);
    QCOMPARE(cfgWin2.templateFolders()[0], QString("./my_templates"));
    QCOMPARE(cfgWin2.templateFolders()[1], QString("./extra_templates"));
    QCOMPARE(outEdit2->text(), QString("./my_outputs"));
    QCOMPARE(table2->rowCount(), 2);
    QCOMPARE(table2->item(0, 0)->checkState(), Qt::Checked);
    QCOMPARE(table2->item(0, 1)->text(), QString("my_template.inja"));
    QCOMPARE(table2->item(0, 2)->text(), QString("my_output.sv"));
    QCOMPARE(table2->item(1, 0)->checkState(), Qt::Unchecked);
    QCOMPARE(table2->item(1, 1)->text(), QString("disabled_template.inja"));

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

void TestRegConfigWindow::testAutoAdjustSizeOnFirstShow()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString origPath = AppSettings::instance().configFilePath();
    AppSettings::instance().setConfigFilePath(tempDir.path() + "/test_fresh_rmap.conf");

    // Fresh configuration dialog brought up for the first time
    RegConfigWindow cfgWin;
    cfgWin.show();
    QCoreApplication::processEvents();

    // Verify window dimensions accommodate contents without clipping
    QVERIFY(cfgWin.width() >= cfgWin.minimumWidth());
    QVERIFY(cfgWin.height() >= cfgWin.minimumHeight());
    QVERIFY(cfgWin.height() >= 650);

    // Verify all toolbar and parameter buttons have adequate height so text and icons are not crushed
    const QStringList toolbarAndParamButtons = {
        "btnAddParameter", "btnRemoveParameter",
        "btnAddTemplateFolder", "btnRemoveTemplateFolder",
        "btnScanTemplates", "btnEnableAll", "btnDisableAll",
        "btnAddTemplateFiles", "btnAddRow", "btnBrowseTemplate",
        "btnBrowseOutputFile", "btnBrowseOutputFolderItem",
        "btnRemoveRow", "btnClearAll"
    };

    for (const QString &btnName : toolbarAndParamButtons) {
        auto *btn = cfgWin.findChild<QPushButton*>(btnName);
        QVERIFY2(btn != nullptr, qPrintable(QString("Button %1 must exist").arg(btnName)));
        QVERIFY2(btn->isVisible(), qPrintable(QString("Button %1 must be visible").arg(btnName)));
        QVERIFY2(btn->height() >= 24, qPrintable(QString("Button %1 height (%2) must be >= 24").arg(btnName).arg(btn->height())));
        QVERIFY2(btn->width() >= 30, qPrintable(QString("Button %1 width (%2) must be >= 30").arg(btnName).arg(btn->width())));
    }

    const QStringList browseButtons = {
        "btnBrowseOutputFolder", "btnBrowsePythonScript"
    };
    for (const QString &btnName : browseButtons) {
        auto *btn = cfgWin.findChild<QPushButton*>(btnName);
        QVERIFY2(btn != nullptr, qPrintable(QString("Button %1 must exist").arg(btnName)));
        QVERIFY2(btn->isVisible(), qPrintable(QString("Button %1 must be visible").arg(btnName)));
        QVERIFY2(btn->height() >= 20, qPrintable(QString("Button %1 height (%2) must be >= 20").arg(btnName).arg(btn->height())));
        QVERIFY2(btn->width() >= 60, qPrintable(QString("Button %1 width (%2) must be >= 60").arg(btnName).arg(btn->width())));
    }

    AppSettings::instance().setConfigFilePath(origPath);
}

void TestRegConfigWindow::testPathStorageVarieties()
{
    RegConfigWindow cfgWin;
    cfgWin.setBaseDir("/home/user/project");

    auto *outEdit = cfgWin.findChild<QLineEdit*>("outputFolder");
    auto *pyEdit = cfgWin.findChild<QLineEdit*>("pythonScript");

    // Relative, absolute, and environment variable paths
    cfgWin.setTemplateFolders(QStringList() << "$MY_TEMPLATES_DIR");
    outEdit->setText("/opt/shared/build/work");
    pyEdit->setText("./scripts/gen.py");

    cfgWin.addTemplateRow(true, "/opt/templates/t1.inja", "/opt/shared/build/work/t1.sv");
    cfgWin.addTemplateRow(true, "$CUSTOM_TEMPLATES/t2.inja", "./relative_out/t2.h");

    protormap::Config* config = cfgWin.serialize();
    QVERIFY(config != nullptr);

    // Verify paths are preserved without forced conversion
    QCOMPARE(config->templatefolder(), std::string("$MY_TEMPLATES_DIR"));
    QCOMPARE(config->outputfolder(), std::string("/opt/shared/build/work"));
    QCOMPARE(config->pythonscript(), std::string("./scripts/gen.py"));
    QCOMPARE(config->template_outputs_size(), 2);
    QCOMPARE(config->template_outputs(0).template_filename(), std::string("/opt/templates/t1.inja"));
    QCOMPARE(config->template_outputs(0).output_filepath(), std::string("/opt/shared/build/work/t1.sv"));
    QCOMPARE(config->template_outputs(1).template_filename(), std::string("$CUSTOM_TEMPLATES/t2.inja"));
    QCOMPARE(config->template_outputs(1).output_filepath(), std::string("./relative_out/t2.h"));

    delete config;
}

void TestRegConfigWindow::testTemplateFoldersListAndScanning()
{
    RegConfigWindow cfgWin;
    cfgWin.setBaseDir(QDir::currentPath());
    cfgWin.setTemplateFolders(QStringList() << "templates/c" << "templates/rtl");

    auto *table = cfgWin.findChild<QTableWidget*>("templateTable");
    QCOMPARE(table->rowCount(), 0);

    cfgWin.scanTemplateFolders();
    QVERIFY(table->rowCount() >= 2);

    // Verify C and RTL templates were discovered, and that scanning does NOT automatically enable them
    bool foundC = false;
    bool foundRtl = false;
    for (int r = 0; r < table->rowCount(); ++r) {
        QCOMPARE(table->item(r, 0)->checkState(), Qt::Unchecked);
        QString tmpl = table->item(r, 1)->text();
        if (tmpl.contains("reg_map.h.inja")) foundC = true;
        if (tmpl.contains("reg_map.sv.inja")) foundRtl = true;
    }
    QVERIFY(foundC);
    QVERIFY(foundRtl);
}

void TestRegConfigWindow::testEnableDisableToggles()
{
    RegConfigWindow cfgWin;
    cfgWin.addTemplateRow(true, "templates/c/reg_map.h.inja", "work/c/reg_map.h");
    cfgWin.addTemplateRow(true, "templates/rtl/reg_map.sv.inja", "work/rtl/reg_map.sv");

    auto *table = cfgWin.findChild<QTableWidget*>("templateTable");
    auto *btnDisableAll = cfgWin.findChild<QPushButton*>("btnDisableAll");
    auto *btnEnableAll = cfgWin.findChild<QPushButton*>("btnEnableAll");

    QVERIFY(btnDisableAll != nullptr);
    QVERIFY(btnEnableAll != nullptr);

    btnDisableAll->click();
    QCOMPARE(table->item(0, 0)->checkState(), Qt::Unchecked);
    QCOMPARE(table->item(1, 0)->checkState(), Qt::Unchecked);

    btnEnableAll->click();
    QCOMPARE(table->item(0, 0)->checkState(), Qt::Checked);
    QCOMPARE(table->item(1, 0)->checkState(), Qt::Checked);
}

void TestRegConfigWindow::testDynamicOutputFolderSync()
{
    RegConfigWindow cfgWin;
    auto *outEdit = cfgWin.findChild<QLineEdit*>("outputFolder");
    auto *table = cfgWin.findChild<QTableWidget*>("templateTable");
    auto *btnSyncOutputs = cfgWin.findChild<QPushButton*>("btnSyncOutputs");

    QVERIFY(outEdit != nullptr);
    QVERIFY(table != nullptr);
    QVERIFY(btnSyncOutputs != nullptr);

    // Initial default output folder is empty / "./work"
    outEdit->setText("work");
    cfgWin.onOutputFolderEdited("work");

    // Row 0: uses default mirrored path
    cfgWin.addTemplateRow(true, "templates/rtl/reg_map.sv.inja", "work/rtl/reg_map.sv");
    // Row 1: uses an explicit custom override
    cfgWin.addTemplateRow(true, "templates/c/reg_map.h.inja", "../custom/path/my_regs.h");

    QCOMPARE(table->item(0, 2)->text(), QString("work/rtl/reg_map.sv"));
    QCOMPARE(table->item(1, 2)->text(), QString("../custom/path/my_regs.h"));

    // User updates default output folder to "build/generated"
    outEdit->setText("build/generated");
    cfgWin.onOutputFolderEdited("build/generated");

    // Row 0 (default) updates dynamically
    QCOMPARE(table->item(0, 2)->text(), QString("build/generated/rtl/reg_map.sv"));
    // Row 1 (custom override) remains preserved
    QCOMPARE(table->item(1, 2)->text(), QString("../custom/path/my_regs.h"));

    // Clicking "Sync Outputs" re-syncs all rows to the active output folder
    btnSyncOutputs->click();
    QCOMPARE(table->item(0, 2)->text(), QString("build/generated/rtl/reg_map.sv"));
    QCOMPARE(table->item(1, 2)->text(), QString("build/generated/c/reg_map.h"));
}

QTEST_MAIN(TestRegConfigWindow)
#include "test_RegConfigWindow.moc"

