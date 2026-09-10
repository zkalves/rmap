/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include <QtTest>
#include <QSpinBox>
#include <QLineEdit>
#include <QListWidget>
#include <QTableWidget>
#include <QTimer>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include "RegConfigWindow.hpp"
#include "AppSettings.hpp"
#include "PathUtils.hpp"

static QtMessageHandler s_originalHandler = nullptr;
static void testOffscreenMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (type == QtWarningMsg && msg.contains("This plugin does not support")) {
        return;
    }
    if (s_originalHandler) {
        s_originalHandler(type, context, msg);
    }
}

class TestRegConfigWindow : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        s_originalHandler = qInstallMessageHandler(testOffscreenMessageHandler);
        QDir("work").removeRecursively();
        QDir().mkpath("work");
    }
    void cleanupTestCase() {
        QDir("work").removeRecursively();
        qInstallMessageHandler(s_originalHandler);
    }

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
    void testPythonScriptField();
    void testTemplateFolderRemovalVariations();
    void testComputeDefaultOutputPathAllBranches();
    void testOutputFolderEditedExtendedBranches();
    void testTableToolbarButtons();
    void testParameterTableButtons();
    void testSerializationEdgeCases();
    void testLifecycleAndEvents();
    void testBrowseDialogsAutoDismiss();
    void testBrowseDialogsWithSelection();
    void testFullBranchCoverageRegConfig();
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

void TestRegConfigWindow::testPythonScriptField()
{
    RegConfigWindow cfgWin;

    auto *pyEdit = cfgWin.findChild<QLineEdit*>("pythonScript");
    auto *browseBtn = cfgWin.findChild<QPushButton*>("btnBrowsePythonScript");

    QVERIFY(pyEdit != nullptr);
    QVERIFY(browseBtn != nullptr);

    // Initial default: empty field, disabled / inactive
    QCOMPARE(pyEdit->text(), QString(""));
    QCOMPARE(pyEdit->isEnabled(), true);
    QCOMPARE(browseBtn->isEnabled(), true);
    QCOMPARE(cfgWin.isPythonScriptEnabled(), false);

    // Entering a script path enables the python script execution upon serialization
    pyEdit->setText("./scripts/post_generate.py");
    protormap::Config *cfg = cfgWin.serialize();
    QCOMPARE(cfgWin.pythonScript(), QString("./scripts/post_generate.py"));
    QCOMPARE(cfgWin.isPythonScriptEnabled(), true);
    QCOMPARE(cfg->pythonscript(), std::string("./scripts/post_generate.py"));
    QCOMPARE(cfg->python_script_enabled(), true);
    delete cfg;

    // Clearing the field disables python script execution
    pyEdit->setText("   ");
    protormap::Config *cfgDisabled = cfgWin.serialize();
    QCOMPARE(cfgWin.pythonScript(), QString(""));
    QCOMPARE(cfgWin.isPythonScriptEnabled(), false);
    QCOMPARE(cfgDisabled->pythonscript(), std::string(""));
    QCOMPARE(cfgDisabled->python_script_enabled(), false);
    delete cfgDisabled;

    // Deserialize a config with a python script into a clean dialog
    protormap::Config cfgWithPy;
    cfgWithPy.set_pythonscript("./scripts/post_generate.py");
    cfgWithPy.set_python_script_enabled(true);

    RegConfigWindow cfgWin2;
    cfgWin2.deserialize(cfgWithPy);

    auto *pyEdit2 = cfgWin2.findChild<QLineEdit*>("pythonScript");
    QVERIFY(pyEdit2 != nullptr);
    QCOMPARE(pyEdit2->text(), QString("./scripts/post_generate.py"));
    QCOMPARE(cfgWin2.pythonScript(), QString("./scripts/post_generate.py"));
    QCOMPARE(cfgWin2.isPythonScriptEnabled(), true);

    // Test helper methods
    cfgWin2.setPythonScript("./scripts/new_script.py");
    QCOMPARE(cfgWin2.pythonScript(), QString("./scripts/new_script.py"));
    QCOMPARE(pyEdit2->text(), QString("./scripts/new_script.py"));
    QCOMPARE(cfgWin2.isPythonScriptEnabled(), true);

    cfgWin2.setPythonScript("");
    QCOMPARE(cfgWin2.pythonScript(), QString(""));
    QCOMPARE(pyEdit2->text(), QString(""));
    QCOMPARE(cfgWin2.isPythonScriptEnabled(), false);
}

void TestRegConfigWindow::testTemplateFolderRemovalVariations()
{
    RegConfigWindow cfgWin;
    auto *list = cfgWin.findChild<QListWidget*>("templateFoldersList");
    auto *btnRemove = cfgWin.findChild<QPushButton*>("btnRemoveTemplateFolder");
    QVERIFY(list != nullptr && btnRemove != nullptr);

    // Clear and add 3 folders
    list->clear();
    cfgWin.addTemplateFolder("folder1");
    cfgWin.addTemplateFolder("folder2");
    cfgWin.addTemplateFolder("folder3");
    QCOMPARE(list->count(), 3);

    // Re-adding duplicate or empty string
    cfgWin.addTemplateFolder("");
    cfgWin.addTemplateFolder("folder1");
    QCOMPARE(list->count(), 3);

    // 1. Remove with selectedItems empty, but currentRow >= 0
    list->clearSelection();
    list->setCurrentRow(1); // folder2
    btnRemove->click();
    QCOMPARE(list->count(), 2);
    QCOMPARE(list->item(0)->text(), QString("folder1"));
    QCOMPARE(list->item(1)->text(), QString("folder3"));

    // 2. Remove with selectedItems non-empty (select both)
    list->item(0)->setSelected(true);
    list->item(1)->setSelected(true);
    btnRemove->click();

    // List became empty, so fallback defaultTemplatesDir was added!
    QCOMPARE(list->count(), 1);
    QCOMPARE(list->item(0)->text(), PathUtils::defaultTemplatesDir());

    // Remove the only item with currentRow
    list->setCurrentRow(0);
    list->clearSelection();
    btnRemove->click();
    QCOMPARE(list->count(), 1); // Remains fallback
}

void TestRegConfigWindow::testComputeDefaultOutputPathAllBranches()
{
    RegConfigWindow cfgWin;
    auto *outFolder = cfgWin.findChild<QLineEdit*>("outputFolder");
    outFolder->setText("work");

    cfgWin.setTemplateFolders(QStringList() << "custom_templates" << "other_templates");

    // Add custom template rows and check computeDefaultOutputPath via sync
    cfgWin.addTemplateRow(true, "templates/c/reg_map.h.inja", "");
    cfgWin.addTemplateRow(true, "./templates/rtl/reg_map.sv.inja", "");
    cfgWin.addTemplateRow(true, "custom_templates/pkg/header.tmpl", "");
    cfgWin.addTemplateRow(true, "./custom_templates/pkg/defs.tmpl", "");
    cfgWin.addTemplateRow(true, "other_dir/unmatched.txt", "");

    cfgWin.onSyncDefaultOutputs();

    auto *table = cfgWin.findChild<QTableWidget*>("templateTable");
    QCOMPARE(table->rowCount(), 5);
    QCOMPARE(table->item(0, 2)->text(), QString("work/c/reg_map.h"));
    QCOMPARE(table->item(1, 2)->text(), QString("work/rtl/reg_map.sv"));
    QCOMPARE(table->item(2, 2)->text(), QString("work/pkg/header"));
    QCOMPARE(table->item(3, 2)->text(), QString("work/pkg/defs"));
    QCOMPARE(table->item(4, 2)->text(), QString("work/unmatched.txt"));
}

void TestRegConfigWindow::testOutputFolderEditedExtendedBranches()
{
    RegConfigWindow cfgWin;
    auto *outEdit = cfgWin.findChild<QLineEdit*>("outputFolder");
    outEdit->setText("work");
    cfgWin.onOutputFolderEdited("work");

    auto *table = cfgWin.findChild<QTableWidget*>("templateTable");

    // Row 0: empty outItem text
    cfgWin.addTemplateRow(true, "templates/c/reg_map.h.inja", "");
    // Row 1: outItem text matches expected old
    cfgWin.addTemplateRow(true, "templates/rtl/reg_map.sv.inja", "work/rtl/reg_map.sv");
    // Row 2: outItem text starts with prevDefault + "/"
    cfgWin.addTemplateRow(true, "templates/uvm/reg_model.sv.inja", "work/custom_sub/reg_model.sv");
    // Row 3: outItem text starts with "./" + prevDefault + "/"
    cfgWin.addTemplateRow(true, "templates/rust/reg_map.rs.inja", "./work/rust/reg_map.rs");
    // Row 4: completely unrelated custom output path (should NOT change!)
    cfgWin.addTemplateRow(true, "templates/html/reg_doc.html.inja", "/etc/custom/doc.html");

    cfgWin.onOutputFolderEdited("build/generated");

    QCOMPARE(table->item(0, 2)->text(), QString("build/generated/c/reg_map.h"));
    QCOMPARE(table->item(1, 2)->text(), QString("build/generated/rtl/reg_map.sv"));
    QCOMPARE(table->item(2, 2)->text(), QString("build/generated/uvm/reg_model.sv"));
    QCOMPARE(table->item(3, 2)->text(), QString("build/generated/rust/reg_map.rs"));
    QCOMPARE(table->item(4, 2)->text(), QString("/etc/custom/doc.html"));
}

void TestRegConfigWindow::testTableToolbarButtons()
{
    RegConfigWindow cfgWin;
    auto *table = cfgWin.findChild<QTableWidget*>("templateTable");
    auto *btnAdd = cfgWin.findChild<QPushButton*>("btnAddRow");
    auto *btnRemove = cfgWin.findChild<QPushButton*>("btnRemoveRow");
    auto *btnClear = cfgWin.findChild<QPushButton*>("btnClearAll");
    auto *btnEnable = cfgWin.findChild<QPushButton*>("btnEnableAll");
    auto *btnDisable = cfgWin.findChild<QPushButton*>("btnDisableAll");

    // Add custom row
    btnAdd->click();
    QCOMPARE(table->rowCount(), 1);
    btnAdd->click();
    QCOMPARE(table->rowCount(), 2);

    // Enable and Disable all
    btnDisable->click();
    QCOMPARE(table->item(0, 0)->checkState(), Qt::Unchecked);
    QCOMPARE(table->item(1, 0)->checkState(), Qt::Unchecked);
    btnEnable->click();
    QCOMPARE(table->item(0, 0)->checkState(), Qt::Checked);
    QCOMPARE(table->item(1, 0)->checkState(), Qt::Checked);

    // Select only row 0 and remove
    table->clearSelection();
    table->item(0, 0)->setSelected(true);
    btnRemove->click();
    QCOMPARE(table->rowCount(), 1);

    // Clear all with non-empty table auto-dismissed with No
    QTimer::singleShot(50, []() {
        QWidget *modal = QApplication::activeModalWidget();
        if (modal) {
            modal->close();
        }
    });
    btnClear->click();

    // Empty table clear all returns early
    table->setRowCount(0);
    btnClear->click();
    QCOMPARE(table->rowCount(), 0);
}

void TestRegConfigWindow::testParameterTableButtons()
{
    RegConfigWindow cfgWin;
    auto *paramTable = cfgWin.findChild<QTableWidget*>("customParametersTable");
    auto *btnAdd = cfgWin.findChild<QPushButton*>("btnAddParameter");
    auto *btnRemove = cfgWin.findChild<QPushButton*>("btnRemoveParameter");

    // Add parameter rows
    btnAdd->click();
    QCOMPARE(paramTable->rowCount(), 1);
    btnAdd->click();
    QCOMPARE(paramTable->rowCount(), 2);

    // Populate items
    paramTable->item(0, 0)->setText("BUS_WIDTH");
    paramTable->item(0, 1)->setText("64");
    paramTable->item(1, 0)->setText("ADDR_WIDTH");
    paramTable->item(1, 1)->setText("32");

    // Select row 1 and remove
    paramTable->item(1, 0)->setSelected(true);
    btnRemove->click();
    QCOMPARE(paramTable->rowCount(), 1);
    QCOMPARE(paramTable->item(0, 0)->text(), QString("BUS_WIDTH"));
}

void TestRegConfigWindow::testSerializationEdgeCases()
{
    RegConfigWindow cfgWin;
    cfgWin.setRegisterWidth(0); // fallback to 32
    QCOMPARE(cfgWin.registerWidth(), 32U);

    cfgWin.setRegisterWidth(64);
    QCOMPARE(cfgWin.registerWidth(), 64U);

    cfgWin.setProjectName("TEST_PROJ");
    QCOMPARE(cfgWin.projectName(), QString("TEST_PROJ"));

    cfgWin.setProjectVersion("1.0.0");
    QCOMPARE(cfgWin.projectVersion(), QString("1.0.0"));

    cfgWin.setBaseDir("");
    QCOMPARE(cfgWin.baseDir(), QDir::currentPath());

    cfgWin.setTemplateFolders(QStringList());
    QCOMPARE(cfgWin.templateFolders().size(), 1);
    QCOMPARE(cfgWin.templateFolders().first(), PathUtils::defaultTemplatesDir());

    protormap::Config *cfg = cfgWin.serialize();
    QCOMPARE(cfg->reg_width(), 64U);
    QCOMPARE(cfg->project_name(), std::string("TEST_PROJ"));
    QCOMPARE(cfg->project_version(), std::string("1.0.0"));
    delete cfg;

    // Deserialize config with empty template_folders but non-empty templatefolder
    protormap::Config legacyCfg;
    legacyCfg.set_templatefolder("legacy/templates");
    cfgWin.deserialize(legacyCfg);
    QCOMPARE(cfgWin.templateFolders().first(), QString("legacy/templates"));

    // Deserialize config with disabled python script
    protormap::Config pyDisabledCfg;
    pyDisabledCfg.set_python_script_enabled(false);
    cfgWin.deserialize(pyDisabledCfg);
    QVERIFY(!cfgWin.isPythonScriptEnabled());
}

void TestRegConfigWindow::testLifecycleAndEvents()
{
    RegConfigWindow cfgWin;

    // LanguageChange event
    QEvent langChange(QEvent::LanguageChange);
    QApplication::sendEvent(&cfgWin, &langChange);

    // Resize, Move, Close events
    QResizeEvent resizeEv(QSize(750, 600), QSize(600, 480));
    QApplication::sendEvent(&cfgWin, &resizeEv);

    QMoveEvent moveEv(QPoint(100, 100), QPoint(0, 0));
    QApplication::sendEvent(&cfgWin, &moveEv);

    QCloseEvent closeEv;
    QApplication::sendEvent(&cfgWin, &closeEv);

    // showEvent with m_firstShown == false
    cfgWin.show();
    QShowEvent showEv;
    QApplication::sendEvent(&cfgWin, &showEv);
}

void TestRegConfigWindow::testBrowseDialogsAutoDismiss()
{
    RegConfigWindow cfgWin;
    auto *table = cfgWin.findChild<QTableWidget*>("templateTable");
    table->setRowCount(0);
    table->setCurrentCell(-1, -1);

    // Calling onBrowseOutputFile with currentRow -1 triggers QMessageBox, auto-dismiss it
    QTimer::singleShot(50, []() {
        QWidget *modal = QApplication::activeModalWidget();
        if (modal) modal->close();
    });
    auto *btnBrowseOut = cfgWin.findChild<QPushButton*>("btnBrowseOutputFile");
    btnBrowseOut->click();

    // Calling onBrowseOutputFolderItem with currentRow -1 triggers QMessageBox, auto-dismiss it
    QTimer::singleShot(50, []() {
        QWidget *modal = QApplication::activeModalWidget();
        if (modal) modal->close();
    });
    auto *btnBrowseFolder = cfgWin.findChild<QPushButton*>("btnBrowseOutputFolderItem");
    btnBrowseFolder->click();

    // Now populate a row and test when row >= 0
    cfgWin.addTemplateRow(true, "templates/c/reg_map.h.inja", "work/c/reg_map.h");
    table->setCurrentCell(0, 0);

    QTimer::singleShot(50, []() {
        QWidget *modal = QApplication::activeModalWidget();
        if (modal) modal->close();
    });
    btnBrowseOut->click();

    QTimer::singleShot(50, []() {
        QWidget *modal = QApplication::activeModalWidget();
        if (modal) modal->close();
    });
    btnBrowseFolder->click();

    // onBrowseOutputFolder
    QTimer::singleShot(50, []() {
        QWidget *modal = QApplication::activeModalWidget();
        if (modal) modal->close();
    });
    auto *btnBrowseOutFolder = cfgWin.findChild<QPushButton*>("btnBrowseOutputFolder");
    btnBrowseOutFolder->click();

    // onBrowsePythonScript
    QTimer::singleShot(50, []() {
        QWidget *modal = QApplication::activeModalWidget();
        if (modal) modal->close();
    });
    auto *btnBrowsePy = cfgWin.findChild<QPushButton*>("btnBrowsePythonScript");
    btnBrowsePy->click();

    // onAddTemplateFiles
    QTimer::singleShot(50, []() {
        QWidget *modal = QApplication::activeModalWidget();
        if (modal) modal->close();
    });
    auto *btnAddTmplFiles = cfgWin.findChild<QPushButton*>("btnAddTemplateFiles");
    btnAddTmplFiles->click();

    // onBrowseTemplate
    QTimer::singleShot(50, []() {
        QWidget *modal = QApplication::activeModalWidget();
        if (modal) modal->close();
    });
    auto *btnBrowseTmpl = cfgWin.findChild<QPushButton*>("btnBrowseTemplate");
    btnBrowseTmpl->click();

    // onAddTemplateFolder
    QTimer::singleShot(50, []() {
        QWidget *modal = QApplication::activeModalWidget();
        if (modal) modal->close();
    });
    auto *btnAddFolder = cfgWin.findChild<QPushButton*>("btnAddTemplateFolder");
    btnAddFolder->click();

    // onScanTemplates
    auto *btnScan = cfgWin.findChild<QPushButton*>("btnScanTemplates");
    btnScan->click();

    // 2-argument overload of addTemplateRow
    int prevRows = table->rowCount();
    cfgWin.addTemplateRow("my_tmpl", "my_out");
    QCOMPARE(table->rowCount(), prevRows + 1);

    // Test onSyncDefaultOutputs and onOutputFolderEdited with null outItem
    table->setItem(0, 2, nullptr);
    cfgWin.onSyncDefaultOutputs();
    QVERIFY(table->item(0, 2) != nullptr);

    table->setItem(0, 2, nullptr);
    cfgWin.onOutputFolderEdited("work/new_folder");
    QVERIFY(table->item(0, 2) != nullptr);

    // Test templateFolders() fallback when list is empty
    auto *foldersList = cfgWin.findChild<QListWidget*>("templateFoldersList");
    foldersList->clear();
    QCOMPARE(cfgWin.templateFolders().size(), 1);
    QCOMPARE(cfgWin.templateFolders().first(), PathUtils::defaultTemplatesDir());

    // Test computeDefaultOutputPath with ./ prefix
    QString expDefaultTmpl = PathUtils::normalizeSeparators(PathUtils::expandEnvVars(PathUtils::defaultTemplatesDir()));
    cfgWin.addTemplateRow(true, "./" + expDefaultTmpl + "/c/reg_map.h.inja", "");
    cfgWin.onSyncDefaultOutputs();

    // ClearAll confirming with QMessageBox::Yes
    QTimer::singleShot(50, []() {
        QWidget *modal = QApplication::activeModalWidget();
        if (auto *box = qobject_cast<QMessageBox*>(modal)) {
            if (auto *btn = box->button(QMessageBox::Yes)) {
                btn->click();
            } else {
                box->accept();
            }
        } else if (modal) {
            modal->close();
        }
    });
    auto *btnClear = cfgWin.findChild<QPushButton*>("btnClearAll");
    btnClear->click();
    QCOMPARE(table->rowCount(), 0);
}

static void chooseInModalFileDialog(const QString &path)
{
    QTimer::singleShot(30, [path]() {
        QWidget *modal = QApplication::activeModalWidget();
        if (modal) {
            auto *dlg = qobject_cast<QFileDialog*>(modal);
            if (dlg) {
                auto *edit = dlg->findChild<QLineEdit*>("fileNameEdit");
                if (edit) {
                    edit->setText(path);
                }
                for (auto *btn : dlg->findChildren<QPushButton*>()) {
                    if (btn->text().contains("Choose") || btn->text().contains("Open") || btn->text().contains("Save")) {
                        btn->click();
                        return;
                    }
                }
            }
            modal->close();
        }
    });
}

void TestRegConfigWindow::testBrowseDialogsWithSelection()
{
    QDir().mkpath("work/test_config_browse/custom_templates");
    QDir().mkpath("work/test_config_browse/out");

    QFile f1("work/test_config_browse/custom_templates/sample.inja");
    if (f1.open(QIODevice::WriteOnly)) {
        f1.write("// sample template");
        f1.close();
    }

    QFile fpy("work/test_config_browse/test_script.py");
    if (fpy.open(QIODevice::WriteOnly)) {
        fpy.write("# python script");
        fpy.close();
    }

    // 1. onAddTemplateFolder
    {
        RegConfigWindow cfgWin;
        QString dir = QDir("work/test_config_browse/custom_templates").absolutePath();
        chooseInModalFileDialog(dir);
        auto *btnAdd = cfgWin.findChild<QPushButton*>("btnAddTemplateFolder");
        btnAdd->click();
        QVERIFY(cfgWin.templateFolders().contains("./work/test_config_browse/custom_templates") ||
                cfgWin.templateFolders().contains("work/test_config_browse/custom_templates"));
    }

    // 2. onBrowseOutputFolder
    {
        RegConfigWindow cfgWin;
        QString dir = QDir("work/test_config_browse/out").absolutePath();
        chooseInModalFileDialog(dir);
        auto *btn = cfgWin.findChild<QPushButton*>("btnBrowseOutputFolder");
        btn->click();
        auto *outEdit = cfgWin.findChild<QLineEdit*>("outputFolder");
        QVERIFY(outEdit->text().contains("work/test_config_browse/out"));
    }

    // 3. onBrowsePythonScript
    {
        RegConfigWindow cfgWin;
        QString pyFile = QDir("work/test_config_browse/test_script.py").absolutePath();
        chooseInModalFileDialog(pyFile);
        auto *btn = cfgWin.findChild<QPushButton*>("btnBrowsePythonScript");
        btn->click();
        auto *pyEdit = cfgWin.findChild<QLineEdit*>("pythonScript");
        QVERIFY(pyEdit->text().contains("work/test_config_browse/test_script.py"));
    }

    // 4. onAddTemplateFiles
    {
        RegConfigWindow cfgWin;
        QString injaFile = QDir("work/test_config_browse/custom_templates/sample.inja").absolutePath();
        chooseInModalFileDialog(injaFile);
        auto *btn = cfgWin.findChild<QPushButton*>("btnAddTemplateFiles");
        btn->click();
        auto *table = cfgWin.findChild<QTableWidget*>("templateTable");
        QVERIFY(table->rowCount() > 0);
    }

    // 5. onBrowseTemplate
    {
        RegConfigWindow cfgWin;
        cfgWin.addTemplateRow(true, "", "");
        auto *table = cfgWin.findChild<QTableWidget*>("templateTable");
        table->setCurrentCell(0, 1);
        table->setItem(0, 1, nullptr); // ensure line 456-457 executes (null item(row, 1))
        table->setItem(0, 2, nullptr); // ensure line 465-466 executes (null item(row, 2))
        QString injaFile = QDir("work/test_config_browse/custom_templates/sample.inja").absolutePath();
        chooseInModalFileDialog(injaFile);
        auto *btn = cfgWin.findChild<QPushButton*>("btnBrowseTemplate");
        btn->click();
        QVERIFY(table->item(0, 1) != nullptr);
        QVERIFY(table->item(0, 2) != nullptr);

        // Row 1 with non-empty output path to hit else branch of line 463
        cfgWin.addTemplateRow(true, "", "existing/output.sv");
        table->setCurrentCell(1, 1);
        chooseInModalFileDialog(injaFile);
        btn->click();
        QCOMPARE(table->item(1, 2)->text(), QString("existing/output.sv"));
    }

    // 6. onBrowseOutputFile
    {
        RegConfigWindow cfgWin;
        cfgWin.addTemplateRow(true, "tmpl.inja", "");
        auto *table = cfgWin.findChild<QTableWidget*>("templateTable");
        table->setCurrentCell(0, 2);
        table->setItem(0, 2, nullptr); // ensure line 496 executes (null item(row, 2))
        QString outFile = QDir("work/test_config_browse/custom_out.h").absolutePath();
        chooseInModalFileDialog(outFile);
        auto *btn = cfgWin.findChild<QPushButton*>("btnBrowseOutputFile");
        btn->click();
        QVERIFY(table->item(0, 2) != nullptr);
        QVERIFY(table->item(0, 2)->text().contains("work/test_config_browse/custom_out.h"));
    }

    // 7. onBrowseOutputFolderItem
    {
        RegConfigWindow cfgWin;
        cfgWin.addTemplateRow(true, "tmpl.inja", "");
        auto *table = cfgWin.findChild<QTableWidget*>("templateTable");
        table->setCurrentCell(0, 2);
        table->setItem(0, 2, nullptr); // ensure line 524 executes (null item(row, 2))
        QString dir = QDir("work/test_config_browse/out").absolutePath();
        chooseInModalFileDialog(dir);
        auto *btn = cfgWin.findChild<QPushButton*>("btnBrowseOutputFolderItem");
        btn->click();
        QVERIFY(table->item(0, 2) != nullptr);
        QVERIFY(table->item(0, 2)->text().contains("work/test_config_browse/out"));
    }
}

void TestRegConfigWindow::testFullBranchCoverageRegConfig()
{
    // 1. Two-argument addTemplateRow with ./templates prefix and RMAP_TEMPLATES_DIR override
    {
        qputenv("RMAP_TEMPLATES_DIR", "/custom/templates/dir");
        RegConfigWindow cfgWin;
        cfgWin.addTemplateRow(QString("./templates/c/reg_map.h.inja"), QString(""));
        cfgWin.addTemplateRow(QString("templates/c/reg_map.h.inja"), QString(""));
        cfgWin.onSyncDefaultOutputs();
        auto *table = cfgWin.findChild<QTableWidget*>("templateTable");
        QVERIFY(table->rowCount() >= 2);
        qunsetenv("RMAP_TEMPLATES_DIR");
    }

    // 2. Custom parameters serialization and deserialization
    {
        RegConfigWindow cfgWin;
        cfgWin.addParameterRow("CLK_FREQ", "100MHz");
        cfgWin.addParameterRow("", ""); // empty key branch ignored
        protormap::Config* cfg = cfgWin.serialize();
        QVERIFY(cfg != nullptr);
        QCOMPARE(cfg->custom_parameters().count("CLK_FREQ"), 1);
        QCOMPARE(QString::fromStdString(cfg->custom_parameters().at("CLK_FREQ")), QString("100MHz"));
        delete cfg;

        protormap::Config protoIn;
        (*protoIn.mutable_custom_parameters())["BAUD"] = "115200";
        cfgWin.deserialize(protoIn);
        auto *paramTable = cfgWin.findChild<QTableWidget*>("customParametersTable");
        QVERIFY(paramTable != nullptr);
        QVERIFY(paramTable->rowCount() > 0);
        QCOMPARE(paramTable->item(0, 0)->text(), QString("BAUD"));
        QCOMPARE(paramTable->item(0, 1)->text(), QString("115200"));

        // UI update with empty folders list via reject
        cfgWin.setTemplateFolders({ " ", "" });
        cfgWin.reject();
        QVERIFY(cfgWin.templateFolders().contains(PathUtils::defaultTemplatesDir()));
    }

    // 3. Window size & position branches (empty geometry, invalid size, valid pos)
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        QString origConf = AppSettings::instance().configFilePath();
        AppSettings::instance().setConfigFilePath(tempDir.path() + "/cfg.conf");
        AppSettings::instance().setConfigWindowGeometry(QByteArray());
        AppSettings::instance().setConfigWindowSize(QSize(0, 0));
        AppSettings::instance().setConfigWindowPos(QPoint(50, 60));

        {
            RegConfigWindow cfgWin2;
            cfgWin2.show();
            QCoreApplication::processEvents();
            QVERIFY(cfgWin2.isVisible());
        }
        AppSettings::instance().setConfigFilePath(origConf);
    }

    // 4. File browse helpers with non-empty initial values and auto-dismiss
    {
        RegConfigWindow cfgWin;
        auto *outEdit = cfgWin.findChild<QLineEdit*>("outputFolder");
        QVERIFY(outEdit != nullptr);
        outEdit->setText("work");
        QTimer::singleShot(50, []() {
            if (auto *modal = QApplication::activeModalWidget()) modal->close();
        });
        auto *btnOut = cfgWin.findChild<QPushButton*>("btnBrowseOutputFolder");
        QVERIFY(btnOut != nullptr);
        btnOut->click();

        cfgWin.setPythonScript("script/test.py");
        QTimer::singleShot(50, []() {
            if (auto *modal = QApplication::activeModalWidget()) modal->close();
        });
        auto *btnPy = cfgWin.findChild<QPushButton*>("btnBrowsePythonScript");
        QVERIFY(btnPy != nullptr);
        btnPy->click();
    }

    // 5. Browse template with row < 0 and rowCount > 0
    {
        RegConfigWindow cfgWin;
        cfgWin.addTemplateRow(true, "", "");
        auto *table = cfgWin.findChild<QTableWidget*>("templateTable");
        table->setCurrentCell(-1, -1);
        QTimer::singleShot(50, []() {
            if (auto *modal = QApplication::activeModalWidget()) modal->close();
        });
        auto *btnTmpl = cfgWin.findChild<QPushButton*>("btnBrowseTemplate");
        QVERIFY(btnTmpl != nullptr);
        btnTmpl->click();
    }

    // 6. Browse template with row < 0 and rowCount == 0 (with template folders present)
    {
        RegConfigWindow cfgWin;
        auto *table = cfgWin.findChild<QTableWidget*>("templateTable");
        table->setRowCount(0);
        table->setCurrentCell(-1, -1);
        cfgWin.setTemplateFolders({ "templates" });
        QTimer::singleShot(50, []() {
            if (auto *modal = QApplication::activeModalWidget()) modal->close();
        });
        auto *btnTmpl = cfgWin.findChild<QPushButton*>("btnBrowseTemplate");
        QVERIFY(btnTmpl != nullptr);
        btnTmpl->click();
    }
}

QTEST_MAIN(TestRegConfigWindow)
#include "test_RegConfigWindow.moc"

