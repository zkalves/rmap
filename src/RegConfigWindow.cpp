/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "RegConfigWindow.hpp"
#include "ThemeManager.hpp"
#include "AppSettings.hpp"
#include "PathUtils.hpp"
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QSet>
#include <QMap>
#include <algorithm>

namespace {

inline QString tableItemText(const QTableWidget *table, int row, int col)
{
    const QTableWidgetItem *item = table->item(row, col);
    return item ? item->text().trimmed() : QString();
}

inline void setTableItemText(QTableWidget *table, int row, int col, const QString &text)
{
    QTableWidgetItem *item = table->item(row, col);
    if (!item) {
        item = new QTableWidgetItem();
        table->setItem(row, col, item);
    }
    item->setText(text);
}

inline QString initialTemplateDir(const QListWidget *list, const QString &base)
{
    if (list->count() > 0) {
        QString first = list->item(0)->text().trimmed();
        if (!first.isEmpty()) {
            return PathUtils::resolvePath(first, base);
        }
    }
    return base;
}

} // namespace

RegConfigWindow::RegConfigWindow(QWidget *parent) :
    QDialog(parent, Qt::Window),
    m_firstShown(true),
    m_regWidth(32)
{
    setupUi(this);

    setWindowTitle(tr("Configuration"));
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    setMinimumSize(600, 480);

    this->templateTable->setColumnCount(3);
    this->templateTable->setHorizontalHeaderLabels(QStringList()
        << tr("Enable")
        << tr("Template Source (File / Path)")
        << tr("Output Destination (File / Folder)"));
    this->templateTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    this->templateTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    this->templateTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

    // Connect Template Folders buttons
    connect(this->btnAddTemplateFolder,    &QPushButton::clicked, this, &RegConfigWindow::onAddTemplateFolder);
    connect(this->btnRemoveTemplateFolder, &QPushButton::clicked, this, &RegConfigWindow::onRemoveTemplateFolder);

    // Connect General Settings browse buttons
    connect(this->btnBrowseOutputFolder,   &QPushButton::clicked, this, &RegConfigWindow::onBrowseOutputFolder);
    connect(this->btnBrowsePythonScript,   &QPushButton::clicked, this, &RegConfigWindow::onBrowsePythonScript);
    connect(this->outputFolder,            &QLineEdit::textEdited, this, &RegConfigWindow::onOutputFolderEdited);

    // Connect Template & Output Table buttons
    connect(this->btnScanTemplates,          &QPushButton::clicked, this, &RegConfigWindow::onScanTemplates);
    connect(this->btnSyncOutputs,            &QPushButton::clicked, this, &RegConfigWindow::onSyncDefaultOutputs);
    connect(this->btnEnableAll,              &QPushButton::clicked, this, &RegConfigWindow::onEnableAll);
    connect(this->btnDisableAll,             &QPushButton::clicked, this, &RegConfigWindow::onDisableAll);
    connect(this->btnAddTemplateFiles,       &QPushButton::clicked, this, &RegConfigWindow::onAddTemplateFiles);
    connect(this->btnAddRow,                 &QPushButton::clicked, this, &RegConfigWindow::onAddCustomRow);
    connect(this->btnBrowseTemplate,         &QPushButton::clicked, this, &RegConfigWindow::onBrowseTemplate);
    connect(this->btnBrowseOutputFile,       &QPushButton::clicked, this, &RegConfigWindow::onBrowseOutputFile);
    connect(this->btnBrowseOutputFolderItem, &QPushButton::clicked, this, &RegConfigWindow::onBrowseOutputFolderItem);
    connect(this->btnRemoveRow,              &QPushButton::clicked, this, &RegConfigWindow::onRemoveSelected);
    connect(this->btnClearAll,               &QPushButton::clicked, this, &RegConfigWindow::onClearAll);

    // Connect Parameter Table buttons
    connect(this->btnAddParameter,    &QPushButton::clicked, this, &RegConfigWindow::onAddParameterRow);
    connect(this->btnRemoveParameter, &QPushButton::clicked, this, &RegConfigWindow::onRemoveParameterRow);

    this->customParametersTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    this->customParametersTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    // Default template folder
    this->templateFoldersList->addItem(PathUtils::defaultTemplatesDir());

    restoreWindowStateFromSettings();
}

RegConfigWindow::~RegConfigWindow() = default;

void RegConfigWindow::setBaseDir(const QString &baseDir)
{
    m_baseDir = baseDir;
}

QString RegConfigWindow::baseDir() const
{
    if (!m_baseDir.isEmpty()) {
        return m_baseDir;
    }
    return QDir::currentPath();
}

void RegConfigWindow::setTemplateFolders(const QStringList &folders)
{
    m_templateFolders = folders;
    this->templateFoldersList->clear();
    for (const QString &f : m_templateFolders) {
        if (!f.trimmed().isEmpty()) {
            this->templateFoldersList->addItem(f.trimmed());
        }
    }
    if (this->templateFoldersList->count() == 0) {
        this->templateFoldersList->addItem(PathUtils::defaultTemplatesDir());
    }
}

QStringList RegConfigWindow::templateFolders() const noexcept
{
    QStringList list;
    for (int i = 0; i < this->templateFoldersList->count(); ++i) {
        QString f = this->templateFoldersList->item(i)->text().trimmed();
        if (!f.isEmpty()) {
            list.append(f);
        }
    }
    if (list.isEmpty()) {
        list.append(PathUtils::defaultTemplatesDir());
    }
    return list;
}

void RegConfigWindow::addTemplateFolder(const QString &folder)
{
    QString trimmed = folder.trimmed();
    if (trimmed.isEmpty()) return;
    for (int i = 0; i < this->templateFoldersList->count(); ++i) {
        if (this->templateFoldersList->item(i)->text().trimmed() == trimmed) {
            return;
        }
    }
    this->templateFoldersList->addItem(trimmed);
}

void RegConfigWindow::onAddTemplateFolder()
{
    QString startDir = initialTemplateDir(this->templateFoldersList, baseDir());
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Template Search Folder"), startDir);
    if (!dir.isEmpty()) {
        QString relDir = PathUtils::toRelativePath(dir, baseDir());
        addTemplateFolder(relDir);
        scanTemplateFolders();
    }
}

void RegConfigWindow::onRemoveTemplateFolder()
{
    QList<QListWidgetItem*> selected = this->templateFoldersList->selectedItems();
    if (selected.isEmpty()) {
        int row = this->templateFoldersList->currentRow();
        if (row >= 0) {
            delete this->templateFoldersList->takeItem(row);
        }
    } else {
        for (auto *item : selected) {
            delete this->templateFoldersList->takeItem(this->templateFoldersList->row(item));
        }
    }
    if (this->templateFoldersList->count() == 0) {
        this->templateFoldersList->addItem(PathUtils::defaultTemplatesDir());
    }
}

void RegConfigWindow::addParameterRow(const QString &key, const QString &val)
{
    int row = this->customParametersTable->rowCount();
    this->customParametersTable->insertRow(row);
    this->customParametersTable->setItem(row, 0, new QTableWidgetItem(key));
    this->customParametersTable->setItem(row, 1, new QTableWidgetItem(val));
}

void RegConfigWindow::addTemplateRow(bool enabled, const QString &tmpl, const QString &out)
{
    int row = this->templateTable->rowCount();
    this->templateTable->insertRow(row);

    auto *checkItem = new QTableWidgetItem();
    checkItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    checkItem->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
    checkItem->setTextAlignment(Qt::AlignCenter);

    auto *tmplItem = new QTableWidgetItem(tmpl);
    auto *outItem = new QTableWidgetItem(out);

    this->templateTable->setItem(row, 0, checkItem);
    this->templateTable->setItem(row, 1, tmplItem);
    this->templateTable->setItem(row, 2, outItem);
}

void RegConfigWindow::addTemplateRow(const QString &tmpl, const QString &out)
{
    addTemplateRow(true, tmpl, out);
}

QString RegConfigWindow::computeDefaultOutputPath(const QString &tmplRelPath, const QString &outFolderOverride)
{
    QString defaultOutDir = outFolderOverride.trimmed();
    if (defaultOutDir.isEmpty()) {
        defaultOutDir = this->outputFolder->text().trimmed();
    }
    if (defaultOutDir.isEmpty()) defaultOutDir = PathUtils::defaultOutputDir();
    defaultOutDir = PathUtils::normalizeSeparators(defaultOutDir);

    QString relSubPath = tmplRelPath;
    QString expDefaultTmpl = PathUtils::normalizeSeparators(PathUtils::expandEnvVars(PathUtils::defaultTemplatesDir()));
    if (relSubPath.startsWith(expDefaultTmpl + "/", Qt::CaseInsensitive)) {
        relSubPath = relSubPath.mid(expDefaultTmpl.length() + 1);
    } else if (relSubPath.startsWith("./" + expDefaultTmpl + "/", Qt::CaseInsensitive)) {
        relSubPath = relSubPath.mid(expDefaultTmpl.length() + 3);
    } else if (relSubPath.startsWith("templates/", Qt::CaseInsensitive)) {
        relSubPath = relSubPath.mid(10);
    } else if (relSubPath.startsWith("./templates/", Qt::CaseInsensitive)) {
        relSubPath = relSubPath.mid(12);
    } else {
        bool matched = false;
        for (int i = 0; i < this->templateFoldersList->count(); ++i) {
            QString f = PathUtils::normalizeSeparators(this->templateFoldersList->item(i)->text().trimmed());
            if (!f.isEmpty()) {
                if (relSubPath.startsWith(f + "/", Qt::CaseInsensitive)) {
                    relSubPath = relSubPath.mid(f.length() + 1);
                    matched = true;
                    break;
                } else if (relSubPath.startsWith("./" + f + "/", Qt::CaseInsensitive)) {
                    relSubPath = relSubPath.mid(f.length() + 3);
                    matched = true;
                    break;
                }
            }
        }
        if (!matched) {
            QFileInfo fi(tmplRelPath);
            relSubPath = fi.fileName();
        }
    }

    if (relSubPath.endsWith(".inja", Qt::CaseInsensitive)) {
        relSubPath.chop(5);
    } else if (relSubPath.endsWith(".tmpl", Qt::CaseInsensitive)) {
        relSubPath.chop(5);
    }

    return defaultOutDir + "/" + relSubPath;
}

void RegConfigWindow::scanTemplateFolders()
{
    QStringList folders = templateFolders();

    QSet<QString> existingTemplates;
    for (int r = 0; r < this->templateTable->rowCount(); ++r) {
        QString t = tableItemText(this->templateTable, r, 1);
        if (!t.isEmpty()) {
            existingTemplates.insert(PathUtils::normalizeSeparators(t));
        }
    }

    for (const QString &folder : folders) {
        QString resolvedDir = PathUtils::resolvePath(folder, baseDir());
        QDir dir(resolvedDir);
        if (!dir.exists()) continue;

        QDirIterator it(resolvedDir, QStringList() << "*.inja" << "*.tmpl", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString file = it.next();
            QString relTmpl = PathUtils::toRelativePath(file, baseDir());
            QString normRel = PathUtils::normalizeSeparators(relTmpl);

            if (!existingTemplates.contains(normRel)) {
                QString defaultOut = computeDefaultOutputPath(relTmpl);
                addTemplateRow(false, relTmpl, defaultOut);
                existingTemplates.insert(normRel);
            }
        }
    }
}

void RegConfigWindow::onScanTemplates()
{
    scanTemplateFolders();
}

void RegConfigWindow::onSyncDefaultOutputs()
{
    QString currDefault = this->outputFolder->text().trimmed();
    if (currDefault.isEmpty()) currDefault = PathUtils::defaultOutputDir();
    currDefault = PathUtils::normalizeSeparators(currDefault);

    for (int r = 0; r < this->templateTable->rowCount(); ++r) {
        QString tmpl = tableItemText(this->templateTable, r, 1);
        if (tmpl.isEmpty()) continue;

        QString newOut = computeDefaultOutputPath(tmpl, currDefault);
        setTableItemText(this->templateTable, r, 2, newOut);
    }
}

void RegConfigWindow::onOutputFolderEdited(const QString &newFolder)
{
    QString prevDefault = m_outputFolder.trimmed().isEmpty() ? PathUtils::defaultOutputDir() : m_outputFolder.trimmed();
    prevDefault = PathUtils::normalizeSeparators(prevDefault);

    QString newDefault = newFolder.trimmed().isEmpty() ? PathUtils::defaultOutputDir() : newFolder.trimmed();
    newDefault = PathUtils::normalizeSeparators(newDefault);

    QString prevPrefix = prevDefault + "/";

    for (int r = 0; r < this->templateTable->rowCount(); ++r) {
        QString tmpl = tableItemText(this->templateTable, r, 1);
        if (tmpl.isEmpty()) continue;

        QString expectedOld = computeDefaultOutputPath(tmpl, prevDefault);
        QString currentOut = tableItemText(this->templateTable, r, 2);

        QString normOut = PathUtils::normalizeSeparators(currentOut);
        if (normOut.startsWith("./")) {
            normOut = normOut.mid(2);
        }

        // If current output was empty or equal to the previous default output, update it to the new default
        if (normOut.isEmpty() || normOut == expectedOld || normOut.startsWith(prevPrefix, Qt::CaseInsensitive))
        {
            QString newOut = computeDefaultOutputPath(tmpl, newDefault);
            setTableItemText(this->templateTable, r, 2, newOut);
        }
    }
    m_outputFolder = newFolder;
}

void RegConfigWindow::onEnableAll()
{
    for (int r = 0; r < this->templateTable->rowCount(); ++r) {
        if (auto *item = this->templateTable->item(r, 0)) {
            item->setCheckState(Qt::Checked);
        }
    }
}

void RegConfigWindow::onDisableAll()
{
    for (int r = 0; r < this->templateTable->rowCount(); ++r) {
        if (auto *item = this->templateTable->item(r, 0)) {
            item->setCheckState(Qt::Unchecked);
        }
    }
}

void RegConfigWindow::onBrowseOutputFolder()
{
    QString initialDir = this->outputFolder->text().trimmed();
    if (initialDir.isEmpty()) initialDir = baseDir();
    else initialDir = PathUtils::resolvePath(initialDir, baseDir());
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Default Output Folder"), initialDir);
    if (!dir.isEmpty()) {
        QString relDir = PathUtils::toRelativePath(dir, baseDir());
        this->outputFolder->setText(relDir);
        onOutputFolderEdited(relDir);
    }
}

void RegConfigWindow::onBrowsePythonScript()
{
    QString initial = this->Ui_config::pythonScript->text().trimmed();
    if (initial.isEmpty()) initial = baseDir();
    else initial = PathUtils::resolvePath(initial, baseDir());
    QString file = QFileDialog::getOpenFileName(
        this,
        tr("Select Python Script"),
        initial,
        tr("Python Files (*.py);;All Files (*.*)")
    );
    if (!file.isEmpty()) {
        this->Ui_config::pythonScript->setText(PathUtils::toRelativePath(file, baseDir()));
    }
}

void RegConfigWindow::onAddTemplateFiles()
{
    QString startDir = initialTemplateDir(this->templateFoldersList, baseDir());

    QStringList files = QFileDialog::getOpenFileNames(
        this,
        tr("Select Template Files"),
        startDir,
        tr("Inja Templates (*.inja *.tmpl *.sv *.h *.cpp *.txt);;All Files (*.*)")
    );

    if (!files.isEmpty()) {
        for (const QString &file : files) {
            QString relTmpl = PathUtils::toRelativePath(file, baseDir());
            QString defaultOut = computeDefaultOutputPath(relTmpl);
            addTemplateRow(true, relTmpl, defaultOut);
        }
    }
}

void RegConfigWindow::onAddCustomRow()
{
    addTemplateRow(true, "", "");
    int lastRow = this->templateTable->rowCount() - 1;
    this->templateTable->setCurrentCell(lastRow, 1);
    this->templateTable->editItem(this->templateTable->item(lastRow, 1));
}

void RegConfigWindow::onBrowseTemplate()
{
    int row = this->templateTable->currentRow();
    if (row < 0) {
        if (this->templateTable->rowCount() > 0) {
            row = 0;
        } else {
            onAddCustomRow();
            row = this->templateTable->rowCount() - 1;
        }
    }

    QString currTmpl = tableItemText(this->templateTable, row, 1);
    QString startDir;
    if (!currTmpl.isEmpty()) {
        startDir = PathUtils::resolvePath(currTmpl, baseDir());
    } else {
        startDir = initialTemplateDir(this->templateFoldersList, baseDir());
    }

    QString file = QFileDialog::getOpenFileName(
        this,
        tr("Select Template File"),
        startDir,
        tr("Inja Templates (*.inja *.tmpl *.sv *.h *.cpp *.txt);;All Files (*.*)")
    );

    if (!file.isEmpty()) {
        QString relTmpl = PathUtils::toRelativePath(file, baseDir());
        setTableItemText(this->templateTable, row, 1, relTmpl);

        // Auto-populate output path if currently empty
        if (tableItemText(this->templateTable, row, 2).isEmpty()) {
            setTableItemText(this->templateTable, row, 2, computeDefaultOutputPath(relTmpl));
        }
    }
}

void RegConfigWindow::onBrowseOutputFile()
{
    int row = this->templateTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, tr("Selection Required"), tr("Please select a template mapping row first."));
        return;
    }

    QString currOut = tableItemText(this->templateTable, row, 2);
    QString startPath = currOut;
    if (startPath.isEmpty()) {
        QString defaultFolder = this->outputFolder->text().trimmed();
        startPath = defaultFolder.isEmpty() ? PathUtils::defaultOutputDir() : defaultFolder;
    }
    startPath = PathUtils::resolvePath(startPath, baseDir());

    QString file = QFileDialog::getSaveFileName(
        this,
        tr("Select Output File"),
        startPath,
        tr("All Files (*.*)")
    );

    if (!file.isEmpty()) {
        setTableItemText(this->templateTable, row, 2, PathUtils::toRelativePath(file, baseDir()));
    }
}

void RegConfigWindow::onBrowseOutputFolderItem()
{
    int row = this->templateTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, tr("Selection Required"), tr("Please select a template mapping row first."));
        return;
    }

    QString currOut = tableItemText(this->templateTable, row, 2);
    QString startDir = currOut;
    if (startDir.isEmpty()) {
        QString defaultFolder = this->outputFolder->text().trimmed();
        startDir = defaultFolder.isEmpty() ? PathUtils::defaultOutputDir() : defaultFolder;
    }
    startDir = PathUtils::resolvePath(startDir, baseDir());

    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Select Output Destination Folder"),
        startDir
    );

    if (!dir.isEmpty()) {
        setTableItemText(this->templateTable, row, 2, PathUtils::toRelativePath(dir, baseDir()));
    }
}

void RegConfigWindow::onRemoveSelected()
{
    QList<QTableWidgetItem*> selectedItems = this->templateTable->selectedItems();
    QSet<int> rows;
    for (auto* item : selectedItems) {
        rows.insert(item->row());
    }
    QList<int> sortedRows = rows.values();
    std::sort(sortedRows.begin(), sortedRows.end(), std::greater<int>());
    for (int r : sortedRows) {
        this->templateTable->removeRow(r);
    }
}

void RegConfigWindow::onClearAll()
{
    if (this->templateTable->rowCount() == 0) return;
    if (QMessageBox::question(
            this,
            tr("Clear Mappings"),
            tr("Are you sure you want to remove all template mappings?"),
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        this->templateTable->setRowCount(0);
    }
}

void RegConfigWindow::onAddParameterRow()
{
    addParameterRow("", "");
    int lastRow = this->customParametersTable->rowCount() - 1;
    this->customParametersTable->setCurrentCell(lastRow, 0);
    this->customParametersTable->editItem(this->customParametersTable->item(lastRow, 0));
}

void RegConfigWindow::onRemoveParameterRow()
{
    QList<QTableWidgetItem*> selectedItems = this->customParametersTable->selectedItems();
    QSet<int> rows;
    for (auto* item : selectedItems) {
        rows.insert(item->row());
    }
    QList<int> sortedRows = rows.values();
    std::sort(sortedRows.begin(), sortedRows.end(), std::greater<int>());
    for (int r : sortedRows) {
        this->customParametersTable->removeRow(r);
    }
}

void RegConfigWindow::saveStateFromUi()
{
    m_pythonScript = this->Ui_config::pythonScript->text().trimmed();
    m_outputFolder = this->outputFolder->text().trimmed();
    m_regWidth = this->regWidthSpinBox->value();

    m_projectName = this->Ui_config::projectName->text().trimmed();
    m_projectVersion = this->Ui_config::projectVersion->text().trimmed();
    m_strictValidation = this->strictValidation->isChecked();

    m_templateFolders = templateFolders();

    m_customParameters.clear();
    for (int i = 0; i < this->customParametersTable->rowCount(); ++i) {
        QString key = tableItemText(this->customParametersTable, i, 0);
        QString val = tableItemText(this->customParametersTable, i, 1);
        if (!key.isEmpty()) {
            m_customParameters.append(qMakePair(key, val));
        }
    }

    m_templateOutputs.clear();
    for (int i = 0; i < this->templateTable->rowCount(); ++i) {
        auto *checkItem = this->templateTable->item(i, 0);
        bool enabled = checkItem ? (checkItem->checkState() == Qt::Checked) : true;
        QString tmpl = tableItemText(this->templateTable, i, 1);
        QString out = tableItemText(this->templateTable, i, 2);
        if (!tmpl.isEmpty()) {
            m_templateOutputs.append({enabled, tmpl, out});
        }
    }
}

void RegConfigWindow::updateUiFromState()
{
    this->Ui_config::pythonScript->setText(m_pythonScript);
    this->outputFolder->setText(m_outputFolder);
    this->regWidthSpinBox->setValue(m_regWidth);

    this->Ui_config::projectName->setText(m_projectName);
    this->Ui_config::projectVersion->setText(m_projectVersion);
    this->strictValidation->setChecked(m_strictValidation);

    this->templateFoldersList->clear();
    for (const QString &f : m_templateFolders) {
        if (!f.trimmed().isEmpty()) {
            this->templateFoldersList->addItem(f.trimmed());
        }
    }
    if (this->templateFoldersList->count() == 0) {
        this->templateFoldersList->addItem(PathUtils::defaultTemplatesDir());
    }

    this->customParametersTable->setRowCount(0);
    for (const auto &pair : m_customParameters) {
        addParameterRow(pair.first, pair.second);
    }

    this->templateTable->setRowCount(0);
    for (const auto &entry : m_templateOutputs) {
        addTemplateRow(entry.enabled, entry.templateFile, entry.outputFile);
    }
}

protormap::Config* RegConfigWindow::serialize(void)
{
    saveStateFromUi();
    protormap::Config* config = new protormap::Config;
    config->set_pythonscript(m_pythonScript.toStdString());
    config->set_python_script_enabled(!m_pythonScript.isEmpty());
    config->set_outputfolder(m_outputFolder.toStdString());
    config->set_reg_width(m_regWidth);

    config->set_project_name(m_projectName.toStdString());
    config->set_project_version(m_projectVersion.toStdString());
    config->set_strict_validation(m_strictValidation);

    for (const QString &f : m_templateFolders) {
        config->add_template_folders(f.toStdString());
    }
    QString defaultTmpl = m_templateFolders.isEmpty() ? PathUtils::defaultTemplatesDir() : m_templateFolders.first();
    config->set_templatefolder(defaultTmpl.toStdString());

    for (const auto &pair : m_customParameters) {
        (*config->mutable_custom_parameters())[pair.first.toStdString()] = pair.second.toStdString();
    }

    for (const auto &entry : m_templateOutputs) {
        auto* out = config->add_template_outputs();
        out->set_template_filename(entry.templateFile.toStdString());
        out->set_output_filepath(entry.outputFile.toStdString());
        out->set_enabled(entry.enabled);
    }
    return config;
}

void RegConfigWindow::deserialize(const protormap::Config &config)
{
    if (config.has_python_script_enabled() && !config.python_script_enabled()) {
        m_pythonScript.clear();
    } else {
        m_pythonScript = QString::fromStdString(config.pythonscript()).trimmed();
    }
    m_outputFolder = QString::fromStdString(config.outputfolder());
    m_regWidth = config.reg_width() > 0 ? config.reg_width() : 32;

    m_projectName = QString::fromStdString(config.project_name());
    m_projectVersion = QString::fromStdString(config.project_version());
    m_strictValidation = config.strict_validation();

    m_templateFolders.clear();
    for (const auto &f : config.template_folders()) {
        m_templateFolders.append(QString::fromStdString(f));
    }
    if (m_templateFolders.isEmpty() && !config.templatefolder().empty()) {
        m_templateFolders.append(QString::fromStdString(config.templatefolder()));
    }
    if (m_templateFolders.isEmpty()) {
        m_templateFolders.append(PathUtils::defaultTemplatesDir());
    }

    m_customParameters.clear();
    for (const auto& [key, value] : config.custom_parameters()) {
        m_customParameters.append(std::make_pair(QString::fromStdString(key), QString::fromStdString(value)));
    }

    m_templateOutputs.clear();
    for (const auto& entry : config.template_outputs()) {
        bool enabled = entry.has_enabled() ? entry.enabled() : true;
        m_templateOutputs.append({
            enabled,
            QString::fromStdString(entry.template_filename()),
            QString::fromStdString(entry.output_filepath())
        });
    }
    updateUiFromState();
}

void RegConfigWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    if (m_firstShown) {
        m_firstShown = false;
        AppSettings::ensureWindowOnScreen(this, minimumSize(), QSize(780, 700));
    }
}

void RegConfigWindow::restoreWindowStateFromSettings()
{
    QByteArray geom = AppSettings::instance().configWindowGeometry();
    if (!geom.isEmpty()) {
        restoreGeometry(geom);
    } else {
        QSize sz = AppSettings::instance().configWindowSize();
        if (!sz.isEmpty()) {
            resize(sz);
        }
        QPoint p = AppSettings::instance().configWindowPos();
        if (!p.isNull()) {
            move(p);
        }
    }
    AppSettings::ensureWindowOnScreen(this, minimumSize(), QSize(780, 700));
}

void RegConfigWindow::saveWindowStateToSettings()
{
    AppSettings::instance().setConfigWindowGeometry(saveGeometry());
    AppSettings::instance().setConfigWindowPos(pos());
    AppSettings::instance().setConfigWindowSize(size());
}

void RegConfigWindow::closeEvent(QCloseEvent *event)
{
    saveWindowStateToSettings();
    QDialog::closeEvent(event);
}

void RegConfigWindow::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    AppSettings::instance().setConfigWindowSize(size());
}

void RegConfigWindow::moveEvent(QMoveEvent *event)
{
    QDialog::moveEvent(event);
    AppSettings::instance().setConfigWindowPos(pos());
}

void RegConfigWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi(this);
    }
    QDialog::changeEvent(event);
}

void RegConfigWindow::accept(void)
{
    saveWindowStateToSettings();
    saveStateFromUi();
    done(Accepted);
}

void RegConfigWindow::reject(void)
{
    saveWindowStateToSettings();
    updateUiFromState();
    done(Rejected);
}

void RegConfigWindow::setRegisterWidth(uint32_t width)
{
    m_regWidth = width > 0 ? width : 32;
    this->regWidthSpinBox->setValue(m_regWidth);
}

void RegConfigWindow::setProjectName(const QString &name)
{
    m_projectName = name;
    this->Ui_config::projectName->setText(m_projectName);
}

void RegConfigWindow::setProjectVersion(const QString &version)
{
    m_projectVersion = version;
    this->Ui_config::projectVersion->setText(m_projectVersion);
}

uint32_t RegConfigWindow::registerWidth() const
{
    return m_regWidth;
}

QString RegConfigWindow::projectName() const
{
    return m_projectName;
}

QString RegConfigWindow::projectVersion() const
{
    return m_projectVersion;
}

void RegConfigWindow::setPythonScript(const QString &script)
{
    m_pythonScript = script.trimmed();
    this->Ui_config::pythonScript->setText(m_pythonScript);
}

QString RegConfigWindow::pythonScript() const
{
    return m_pythonScript;
}

bool RegConfigWindow::isPythonScriptEnabled() const
{
    return !m_pythonScript.isEmpty();
}
