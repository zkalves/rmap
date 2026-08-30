#include "RegConfigWindow.hpp"
#include "ThemeManager.hpp"
#include "AppSettings.hpp"
#include "PathUtils.hpp"
#include <QFileInfo>
#include <QDir>
#include <algorithm>

RegConfigWindow::RegConfigWindow(QWidget *parent) :
    QDialog(parent, Qt::Window),
    m_regWidth(32)
{
    setupUi(this);

    setWindowTitle(tr("Configuration"));
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    resize(750, 560);
    setMinimumSize(600, 450);

    this->templateTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    this->templateTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    // Connect General Settings browse buttons
    connect(this->btnBrowseTemplateFolder, &QPushButton::clicked, this, &RegConfigWindow::onBrowseTemplateFolder);
    connect(this->btnBrowseOutputFolder,   &QPushButton::clicked, this, &RegConfigWindow::onBrowseOutputFolder);
    connect(this->btnBrowsePythonScript,   &QPushButton::clicked, this, &RegConfigWindow::onBrowsePythonScript);

    // Connect Template & Output Table buttons
    connect(this->btnAddTemplateFiles,       &QPushButton::clicked, this, &RegConfigWindow::onAddTemplateFiles);
    connect(this->btnAddRow,                 &QPushButton::clicked, this, &RegConfigWindow::onAddCustomRow);
    connect(this->btnBrowseTemplate,         &QPushButton::clicked, this, &RegConfigWindow::onBrowseTemplate);
    connect(this->btnBrowseOutputFile,       &QPushButton::clicked, this, &RegConfigWindow::onBrowseOutputFile);
    connect(this->btnBrowseOutputFolderItem, &QPushButton::clicked, this, &RegConfigWindow::onBrowseOutputFolderItem);
    connect(this->btnRemoveRow,              &QPushButton::clicked, this, &RegConfigWindow::onRemoveSelected);
    connect(this->btnClearAll,              &QPushButton::clicked, this, &RegConfigWindow::onClearAll);

    // Connect Parameter Table buttons
    connect(this->btnAddParameter, &QPushButton::clicked, this, &RegConfigWindow::onAddParameterRow);
    connect(this->btnRemoveParameter, &QPushButton::clicked, this, &RegConfigWindow::onRemoveParameterRow);

    this->customParametersTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    this->customParametersTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    restoreWindowStateFromSettings();
}

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

void RegConfigWindow::addParameterRow(const QString &key, const QString &val)
{
    int row = this->customParametersTable->rowCount();
    this->customParametersTable->insertRow(row);
    this->customParametersTable->setItem(row, 0, new QTableWidgetItem(key));
    this->customParametersTable->setItem(row, 1, new QTableWidgetItem(val));
}

void RegConfigWindow::addTemplateRow(const QString &tmpl, const QString &out)
{
    int row = this->templateTable->rowCount();
    this->templateTable->insertRow(row);
    this->templateTable->setItem(row, 0, new QTableWidgetItem(tmpl));
    this->templateTable->setItem(row, 1, new QTableWidgetItem(out));
}

void RegConfigWindow::onBrowseTemplateFolder()
{
    QString initialDir = this->templateFolder->text().trimmed();
    if (initialDir.isEmpty()) initialDir = baseDir();
    else initialDir = PathUtils::resolvePath(initialDir, baseDir());
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Default Template Folder"), initialDir);
    if (!dir.isEmpty()) {
        this->templateFolder->setText(PathUtils::toRelativePath(dir, baseDir()));
    }
}

void RegConfigWindow::onBrowseOutputFolder()
{
    QString initialDir = this->outputFolder->text().trimmed();
    if (initialDir.isEmpty()) initialDir = baseDir();
    else initialDir = PathUtils::resolvePath(initialDir, baseDir());
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Default Output Folder"), initialDir);
    if (!dir.isEmpty()) {
        this->outputFolder->setText(PathUtils::toRelativePath(dir, baseDir()));
    }
}

void RegConfigWindow::onBrowsePythonScript()
{
    QString initial = this->pythonScript->text().trimmed();
    if (initial.isEmpty()) initial = baseDir();
    else initial = PathUtils::resolvePath(initial, baseDir());
    QString file = QFileDialog::getOpenFileName(
        this,
        tr("Select Python Script"),
        initial,
        tr("Python Files (*.py);;All Files (*.*)")
    );
    if (!file.isEmpty()) {
        this->pythonScript->setText(PathUtils::toRelativePath(file, baseDir()));
    }
}

void RegConfigWindow::onAddTemplateFiles()
{
    QString startDir = this->templateFolder->text().trimmed();
    if (startDir.isEmpty()) startDir = baseDir();
    else startDir = PathUtils::resolvePath(startDir, baseDir());

    QStringList files = QFileDialog::getOpenFileNames(
        this,
        tr("Select Template Files"),
        startDir,
        tr("Inja Templates (*.inja *.tmpl *.sv *.h *.cpp *.txt);;All Files (*.*)")
    );

    if (!files.isEmpty()) {
        QString defaultOutDir = this->outputFolder->text().trimmed();
        if (defaultOutDir.isEmpty()) defaultOutDir = PathUtils::defaultOutputDir();

        for (const QString &file : files) {
            QFileInfo fi(file);
            QString outName = fi.fileName();
            if (outName.endsWith(".inja", Qt::CaseInsensitive)) {
                outName.chop(5);
            } else if (outName.endsWith(".tmpl", Qt::CaseInsensitive)) {
                outName.chop(5);
            }
            QString defaultOut = defaultOutDir + "/" + outName;
            addTemplateRow(PathUtils::toRelativePath(file, baseDir()), defaultOut);
        }
    }
}

void RegConfigWindow::onAddCustomRow()
{
    addTemplateRow("", "");
    int lastRow = this->templateTable->rowCount() - 1;
    this->templateTable->setCurrentCell(lastRow, 0);
    this->templateTable->editItem(this->templateTable->item(lastRow, 0));
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

    QString currTmpl = this->templateTable->item(row, 0) ? this->templateTable->item(row, 0)->text().trimmed() : "";
    QString startDir = currTmpl.isEmpty()
        ? (this->templateFolder->text().trimmed().isEmpty() ? baseDir() : PathUtils::resolvePath(this->templateFolder->text().trimmed(), baseDir()))
        : PathUtils::resolvePath(currTmpl, baseDir());

    QString file = QFileDialog::getOpenFileName(
        this,
        tr("Select Template File"),
        startDir,
        tr("Inja Templates (*.inja *.tmpl *.sv *.h *.cpp *.txt);;All Files (*.*)")
    );

    if (!file.isEmpty()) {
        if (!this->templateTable->item(row, 0)) {
            this->templateTable->setItem(row, 0, new QTableWidgetItem());
        }
        this->templateTable->item(row, 0)->setText(PathUtils::toRelativePath(file, baseDir()));

        // Auto-populate output path if currently empty
        if (!this->templateTable->item(row, 1) || this->templateTable->item(row, 1)->text().trimmed().isEmpty()) {
            QFileInfo fi(file);
            QString outName = fi.fileName();
            if (outName.endsWith(".inja", Qt::CaseInsensitive)) {
                outName.chop(5);
            } else if (outName.endsWith(".tmpl", Qt::CaseInsensitive)) {
                outName.chop(5);
            }
            QString defaultOutDir = this->outputFolder->text().trimmed();
            if (defaultOutDir.isEmpty()) defaultOutDir = PathUtils::defaultOutputDir();

            if (!this->templateTable->item(row, 1)) {
                this->templateTable->setItem(row, 1, new QTableWidgetItem());
            }
            this->templateTable->item(row, 1)->setText(defaultOutDir + "/" + outName);
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

    QString currOut = this->templateTable->item(row, 1) ? this->templateTable->item(row, 1)->text().trimmed() : "";
    QString startPath = currOut.isEmpty()
        ? (this->outputFolder->text().trimmed().isEmpty() ? PathUtils::defaultOutputDir() : this->outputFolder->text().trimmed())
        : currOut;
    startPath = PathUtils::resolvePath(startPath, baseDir());

    QString file = QFileDialog::getSaveFileName(
        this,
        tr("Select Output File"),
        startPath,
        tr("All Files (*.*)")
    );

    if (!file.isEmpty()) {
        if (!this->templateTable->item(row, 1)) {
            this->templateTable->setItem(row, 1, new QTableWidgetItem());
        }
        this->templateTable->item(row, 1)->setText(PathUtils::toRelativePath(file, baseDir()));
    }
}

void RegConfigWindow::onBrowseOutputFolderItem()
{
    int row = this->templateTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, tr("Selection Required"), tr("Please select a template mapping row first."));
        return;
    }

    QString currOut = this->templateTable->item(row, 1) ? this->templateTable->item(row, 1)->text().trimmed() : "";
    QString startDir = currOut.isEmpty()
        ? (this->outputFolder->text().trimmed().isEmpty() ? PathUtils::defaultOutputDir() : this->outputFolder->text().trimmed())
        : currOut;
    startDir = PathUtils::resolvePath(startDir, baseDir());

    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Select Output Destination Folder"),
        startDir
    );

    if (!dir.isEmpty()) {
        if (!this->templateTable->item(row, 1)) {
            this->templateTable->setItem(row, 1, new QTableWidgetItem());
        }
        this->templateTable->item(row, 1)->setText(PathUtils::toRelativePath(dir, baseDir()));
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
    m_pythonScript = this->pythonScript->text().trimmed();
    m_templateFolder = this->templateFolder->text().trimmed();
    m_outputFolder = this->outputFolder->text().trimmed();
    m_regWidth = this->regWidthSpinBox->value();

    m_projectName = this->Ui_config::projectName->text().trimmed();
    m_projectVersion = this->Ui_config::projectVersion->text().trimmed();
    m_strictValidation = this->strictValidation->isChecked();

    m_customParameters.clear();
    for (int i = 0; i < this->customParametersTable->rowCount(); ++i) {
        QString key = this->customParametersTable->item(i, 0) ? this->customParametersTable->item(i, 0)->text().trimmed() : "";
        QString val = this->customParametersTable->item(i, 1) ? this->customParametersTable->item(i, 1)->text().trimmed() : "";
        if (!key.isEmpty()) {
            m_customParameters.append(qMakePair(key, val));
        }
    }


    m_templateOutputs.clear();
    for (int i = 0; i < this->templateTable->rowCount(); ++i) {
        QString tmpl = this->templateTable->item(i, 0) ? this->templateTable->item(i, 0)->text().trimmed() : "";
        QString out = this->templateTable->item(i, 1) ? this->templateTable->item(i, 1)->text().trimmed() : "";
        if (!tmpl.isEmpty() && !out.isEmpty()) {
            m_templateOutputs.append(qMakePair(tmpl, out));
        }
    }
}

void RegConfigWindow::updateUiFromState()
{
    this->pythonScript->setText(m_pythonScript);
    this->templateFolder->setText(m_templateFolder);
    this->outputFolder->setText(m_outputFolder);
    this->regWidthSpinBox->setValue(m_regWidth > 0 ? m_regWidth : 32);

    this->Ui_config::projectName->setText(m_projectName);
    this->Ui_config::projectVersion->setText(m_projectVersion);
    this->strictValidation->setChecked(m_strictValidation);
    
    this->customParametersTable->setRowCount(0);
    for (const auto &pair : m_customParameters) {
        addParameterRow(pair.first, pair.second);
    }


    this->templateTable->setRowCount(0);
    for (const auto &pair : m_templateOutputs) {
        addTemplateRow(pair.first, pair.second);
    }
}

protormap::Config* RegConfigWindow::serialize(void)
{
    saveStateFromUi();
    protormap::Config* config = new protormap::Config;
    config->set_pythonscript(m_pythonScript.toStdString());
    config->set_templatefolder(m_templateFolder.toStdString());
    config->set_outputfolder(m_outputFolder.toStdString());
    config->set_reg_width(m_regWidth > 0 ? m_regWidth : 32);

    config->set_project_name(m_projectName.toStdString());
    config->set_project_version(m_projectVersion.toStdString());
    config->set_strict_validation(m_strictValidation);
    for (const auto &pair : m_customParameters) {
        (*config->mutable_custom_parameters())[pair.first.toStdString()] = pair.second.toStdString();
    }


    for (const auto &pair : m_templateOutputs) {
        auto* out = config->add_template_outputs();
        out->set_template_filename(pair.first.toStdString());
        out->set_output_filepath(pair.second.toStdString());
    }
    return config;
}

void RegConfigWindow::deserialize(const protormap::Config &config)
{
    m_pythonScript = QString::fromStdString(config.pythonscript());
    m_templateFolder = QString::fromStdString(config.templatefolder());
    m_outputFolder = QString::fromStdString(config.outputfolder());
    m_regWidth = config.reg_width() > 0 ? config.reg_width() : 32;

    m_projectName = QString::fromStdString(config.project_name());
    m_projectVersion = QString::fromStdString(config.project_version());
    m_strictValidation = config.strict_validation();

    m_customParameters.clear();
    for (const auto& [key, value] : config.custom_parameters()) {
        m_customParameters.append(std::make_pair(QString::fromStdString(key), QString::fromStdString(value)));
    }


    m_templateOutputs.clear();
    for (const auto& entry : config.template_outputs()) {
        m_templateOutputs.append(std::make_pair(
            QString::fromStdString(entry.template_filename()),
            QString::fromStdString(entry.output_filepath())
        ));
    }
    updateUiFromState();
}

void RegConfigWindow::restoreWindowStateFromSettings()
{
    QByteArray geom = AppSettings::instance().configWindowGeometry();
    if (!geom.isEmpty()) {
        restoreGeometry(geom);
    } else {
        QSize sz = AppSettings::instance().configWindowSize();
        QPoint p = AppSettings::instance().configWindowPos();
        if (sz.isValid() && sz.width() > 0 && sz.height() > 0) {
            resize(sz);
        } else {
            resize(750, 560);
        }
        if (!p.isNull()) {
            move(p);
        }
    }
    AppSettings::ensureWindowOnScreen(this, QSize(600, 450), QSize(750, 560));
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
    if (this->regWidthSpinBox) {
        this->regWidthSpinBox->setValue(m_regWidth);
    }
}

void RegConfigWindow::setProjectName(const QString &name)
{
    m_projectName = name;
    if (this->Ui_config::projectName) {
        this->Ui_config::projectName->setText(m_projectName);
    }
}

void RegConfigWindow::setProjectVersion(const QString &version)
{
    m_projectVersion = version;
    if (this->Ui_config::projectVersion) {
        this->Ui_config::projectVersion->setText(m_projectVersion);
    }
}

uint32_t RegConfigWindow::registerWidth() const
{
    return m_regWidth > 0 ? m_regWidth : 32;
}

QString RegConfigWindow::projectName() const
{
    return m_projectName;
}

QString RegConfigWindow::projectVersion() const
{
    return m_projectVersion;
}

