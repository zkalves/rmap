#include "RegMapWindow.hpp"

RegMapWindow::RegMapWindow(QString &rmap_filename, QWidget *parent) :
    QMainWindow(parent)
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    this->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    m_active_folder = ".";
    m_is_regmap_modified = false;
    m_rmap_filename = rmap_filename;
    m_default_filename = "rmap.rmt";
    m_default_window_title = windowTitle();
    // Set table model and attributes
    m_model = new RegMapTreeModel();
    connect(m_model,   &RegMapTreeModel::dataChanged, this, &RegMapWindow::regmap_modified);
    this->treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->treeView->setModel(m_model);
    this->treeView->setAlternatingRowColors(true);
    connect(actionFileNew,      &QAction::triggered, this, &RegMapWindow::btnFileNew);
    connect(actionFileOpen,     &QAction::triggered, this, &RegMapWindow::btnFileOpen);
    connect(actionFileSave,     &QAction::triggered, this, &RegMapWindow::btnFileSave);
    connect(actionFileSaveAs,   &QAction::triggered, this, &RegMapWindow::btnFileSaveAs);
    connect(actionFileReload,   &QAction::triggered, this, &RegMapWindow::btnFileReload);
    connect(actionAddMem,       &QAction::triggered, this, [this]{ insertChild(RegMapTreeItem::e_rmmKind::mem); });
    connect(actionAddRegBlock,  &QAction::triggered, this, [this]{ insertChild(RegMapTreeItem::e_rmmKind::blk); });
    connect(actionAddRegField,  &QAction::triggered, this, [this]{ insertChild(RegMapTreeItem::e_rmmKind::fld); });
    connect(actionAddRegMap,    &QAction::triggered, this, [this]{ insertChild(RegMapTreeItem::e_rmmKind::map); });
    connect(actionAddReg,       &QAction::triggered, this, [this]{ insertChild(RegMapTreeItem::e_rmmKind::reg); });
    connect(actionDeleteItem,   &QAction::triggered, this, &RegMapWindow::btnDeleteItem);
    connect(actionCheck,        &QAction::triggered, this, &RegMapWindow::btnCheck);
    connect(actionExport,       &QAction::triggered, this, &RegMapWindow::btnExport);
    connect(actionQuit,         &QAction::triggered, this, &RegMapWindow::btnQuitButton);
    connect(actionAbout,        &QAction::triggered, this, &RegMapWindow::btnAbout);
    connect(actionConfig,       &QAction::triggered, this, &RegMapWindow::btnConfig);


    m_config_window = new RegConfigWindow(this);

//        # if not self.rmap_filename:
//        #     self.rmap_filename = self.default_filename
    if(!m_rmap_filename.isNull())
    {
        fileOpen(rmap_filename);
    }
    this->treeView->setItemDelegateForColumn(1,new RegHexDecBinDelegate());
    this->treeView->setItemDelegateForColumn(2,new RegHexDecBinDelegate());
    this->treeView->setItemDelegateForColumn(3,new RegStrDelegate());
    this->treeView->setItemDelegateForColumn(4,new RegMapDelegate());

}
void RegMapWindow::btnConfig(void)
{
    m_config_window->show();
}

void RegMapWindow::btnAbout(void)
{
    QMessageBox::information(this,
                             tr("About"),
                             tr("Version: 0.1.0\n"
                                "Author: Ezequiel Alves \n"),
                             QMessageBox::Ok);
}

void RegMapWindow::btnQuitButton(void)
{
        this->close();
}

void RegMapWindow::btnFileNew(void)
{
    QMessageBox::StandardButton result;
    bool save_status = false;
    if (m_is_regmap_modified)
    {
        result = QMessageBox::warning(this, "New file",
                                           "This action will remove all unsaved data, do you wish to continue?",
                                           QMessageBox::Ok | QMessageBox::Save | QMessageBox::Cancel);
        if (result == QMessageBox::Save)
        {
            save_status = btnFileSave();
        }
    }
    if (!m_is_regmap_modified || result == QMessageBox::Ok || (result == QMessageBox::Save && save_status))
    {
        fileNew();
    }
}

bool RegMapWindow::btnFileSave(void)
{
    bool save_status = false;
    if (m_rmap_filename.isNull() || m_rmap_filename.isEmpty())
    {
        save_status = btnFileSaveAs();
    }
    else
    {
        save_status = fileSave();
    }
    return(save_status);
}

bool RegMapWindow::btnFileSaveAs(void)
{
    QString fname;
    bool save_status = false;
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter("*.rmt");
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if(m_rmap_filename.isNull() || m_rmap_filename.isEmpty())
    {
        fname = m_default_filename;
    }
    else
    {
        fname = m_rmap_filename;
        if (!fname.endsWith(".rmt"))
        {
            QString croped_fname=fname.split(".",QString::SkipEmptyParts).at(0);
            croped_fname.append(".rmt");
            fname=croped_fname;
        }
    }
    dialog.selectFile(fname);
    if(dialog.exec())
    {
        QStringList selectedFiles = dialog.selectedFiles();
        if (!selectedFiles.isEmpty())
        {
            QString filename;
            fname = selectedFiles.at(0);
            save_status = fileSave(fname);
            if(save_status)
            {
                m_rmap_filename = fname;
            }
            filename = m_default_window_title;
            filename.append(" - ");
            filename.append(fname);
            this->setWindowTitle(filename);
        }
    }
    return(save_status);
}


void RegMapWindow::btnFileOpen(void)
{
    QMessageBox::StandardButton result;
    if (m_is_regmap_modified)
    {
        result = QMessageBox::warning(this, "Open file",
                                           "This action will remove all unsaved data, do you wish to continue?",
                                           QMessageBox::Ok | QMessageBox::Cancel);
    }
    if(!m_is_regmap_modified || result == QMessageBox::Ok)
    {
            QString filename;
            QString fname = QFileDialog::getOpenFileName( this,
                                                         "Open file",
                                                         m_active_folder,
                                                         "*.rmt");
            if(!fname.isEmpty()&& !fname.isNull()){
                fileOpen(fname);
                filename = m_default_window_title;
                filename.append(" - ");
                filename.append(fname);
                this->setWindowTitle(filename);
            }
    }
}

void RegMapWindow::btnFileReload(void)
{
    QMessageBox::StandardButton result;
    if (m_is_regmap_modified)
    {
        result = QMessageBox::warning(this, "Reload file",
                                           "This action will remove all unsaved data, do you wish to continue?",
                                           QMessageBox::Ok | QMessageBox::Cancel);
    }
    if(!m_is_regmap_modified || result == QMessageBox::Ok)
    {
        QString filename;
        QString fname;
        fname = m_rmap_filename;
        fileOpen(fname);
        filename = m_default_window_title;
        filename.append(" - ");
        filename.append(fname);
        this->setWindowTitle(filename);
    }
}

void RegMapWindow::btnCheck(void)
{
    m_model->checkData();
}

#include <QDebug>
void RegMapWindow::btnExport(void)
{
    qDebug() << "export";
}

void RegMapWindow::btnDeleteItem(void)
{
    QModelIndex index;
    index = this->treeView->currentIndex();
    if (index.row() >= 0)
    {
        m_model->removeRows(index.row(), 1, index.parent());
    }
}

void RegMapWindow::fileNew(void)
{
    m_model->clear();
    //this->treeView->reset();
    delete m_model;
    m_model = new RegMapTreeModel();
    connect(m_model,   &RegMapTreeModel::dataChanged, this, &RegMapWindow::regmap_modified);
    this->treeView->setModel(m_model);
    this->regmap_notModified();
    this->m_rmap_filename = QString();
}

void RegMapWindow::fileOpen(QString fname)
{
    QFileInfo check_file(fname);
    if (check_file.exists() && check_file.isFile()) {
        fileNew();
        this->m_rmap_filename = fname;

        protormap::RegModel reg_model;
        {

            int fileDescriptor = open(fname.toStdString().c_str(), O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);

            google::protobuf::TextFormat::Parser parser;
            ProtobufLogCollector error_collector;
            parser.RecordErrorsTo(&error_collector);

            if (fileDescriptor < 0) {
                QMessageBox::critical(this,
                        tr("Error opening file"),
                        tr("Filename: %1\nError no: %2\nError description: %3").arg(fname).arg(errno).arg(strerror(errno)),
                        QMessageBox::Ok);
            } else {
                google::protobuf::io::FileInputStream fileInput(fileDescriptor);
                fileInput.SetCloseOnDelete( true );
                if (!parser.Parse(&fileInput,&reg_model)) {
                    QMessageBox::critical(this,
                            tr("Failed to parse file"),
                            tr("Failed to parse file %1.\n%2").arg(fname).arg(QString::fromStdString(error_collector.get_string())),
                            QMessageBox::Ok);
                }
                else {
                    //this->m_model.m_rootItem = rmt.safe_load(fh);
                    //qDebug() << QString::fromStdString(reg_model.DebugString()) << endl;
                    if(reg_model.has_config())
                    {
                        m_config_window->deserialize(reg_model.config());
                    }
                    if(reg_model.has_config())
                    {
                    }

                    this->treeView->setItemsExpandable(true);
                    this->treeView->expandAll();
                    for (int col = 0 ; col < this->m_model->columnCount() ; col++) {
                        this->treeView->resizeColumnToContents(col);
                    }
                }
            }
        }
    }
    else {
        QMessageBox::critical(this,
                tr("Error file not found"),
                tr("Filename: %1 not found").arg(fname),
                QMessageBox::Ok);
    }
}

protormap::RegModel& operator <<( protormap::RegModel& reg_model, const SerializationContext& context )
{
    for(SerializationContext::Record rec : context.m_records)
    {
        protormap::RegItem* item = reg_model.add_item();
        item->set_id(rec.m_data["id"].toUInt());
        switch (rec.m_data["kind"].value<RegMapTreeItem::e_rmmKind>()){
            case RegMapTreeItem::e_rmmKind::root: item->set_kind(protormap::RegItem_Kind_ROOT); break;
            case RegMapTreeItem::e_rmmKind::mem:  item->set_kind(protormap::RegItem_Kind_MEM);  break;
            case RegMapTreeItem::e_rmmKind::map:  item->set_kind(protormap::RegItem_Kind_MAP);  break;
            case RegMapTreeItem::e_rmmKind::blk:  item->set_kind(protormap::RegItem_Kind_BLK);  break;
            case RegMapTreeItem::e_rmmKind::reg:  item->set_kind(protormap::RegItem_Kind_REG);  break;
            case RegMapTreeItem::e_rmmKind::fld:  item->set_kind(protormap::RegItem_Kind_FLD);  break;
        }
    }
    return(reg_model);
}

QDataStream& operator <<( QDataStream& stream, const SerializationContext& context )
{
    for(SerializationContext::Record rec : context.m_records)
    {
        stream << rec.m_type;
    }
    return (stream);
}

QDebug  operator <<( QDebug  stream, const SerializationContext& context )
{
    for(SerializationContext::Record rec : context.m_records)
    {
        stream << endl;
        stream << rec.m_data;
        stream << endl;
    }
    return (stream);
}

bool RegMapWindow::fileSave(QString fname)
{
    bool save_status = false;
    if(fname.isNull() || fname.isEmpty())
    {
        fname = m_rmap_filename;
    }
    if (!fname.endsWith(".rmt"))
    {
        QString croped_fname=fname.split(".",QString::SkipEmptyParts).at(0);
        croped_fname.append(".rmt");
        fname=croped_fname;
    }
    int fileDescriptor = open(fname.toStdString().c_str(), O_TRUNC  | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);

    if (fileDescriptor < 0) {
        QMessageBox::critical(this,
                tr("Error opening file"),
                tr("Filename: %1\nError no: %2\nError description: %3").arg(fname).arg(errno).arg(strerror(errno)),
                QMessageBox::Ok);
    } else {
        protormap::RegModel reg_model;
        reg_model.set_allocated_config( m_config_window->serialize());
        SerializationContext context;
        context.serialize( m_model->getRootItem() );

        //for(SerializationContext::Record rec : context.m_records)
        //{
            //protormap::RegItem* item = reg_model.add_item();
            ////stream << rec.m_type;
        //}
        reg_model << context;
        //qDebug() << context;
        // Set timestamp
        google::protobuf::Timestamp timestamp;
        timestamp.set_seconds(time(NULL));
        timestamp.set_nanos(0);
        *(reg_model.mutable_last_updated()) = timestamp;

        google::protobuf::io::FileOutputStream fileOutput(fileDescriptor);
        fileOutput.SetCloseOnDelete( true );
        if (!google::protobuf::TextFormat::Print(reg_model, &fileOutput)) {
            QMessageBox::critical(this,
                    tr("Failed to write output file"),
                    tr("Error writing to Filename: %1\n").arg(fname),
                    QMessageBox::Ok);
        }
        else {
            regmap_notModified();
            save_status = true;
        }
    }
    return(save_status);
}

void RegMapWindow::regmap_modified(void)
{
    if(!m_is_regmap_modified)
    {
        m_is_regmap_modified = true;
        QString win_title = this->windowTitle();
        if (!win_title.endsWith('*'))
        {
            win_title.append('*');
        }
        this->setWindowTitle(win_title);
    }
}

void RegMapWindow::regmap_notModified(void)
{
    if(m_is_regmap_modified)
    {
        m_is_regmap_modified = false;
        QString win_title = this->windowTitle();
        if (win_title.endsWith('*'))
        {
            win_title.remove(win_title.size()-1,1);
        }
        this->setWindowTitle(win_title);
    }
}

void RegMapWindow::insertChild(RegMapTreeItem::e_rmmKind kind)
{
//        The issue is when:
//        1. Create item.
//        2. Click on the second column
//        3. Create second item
//        Works OK if only click on first column when creating items

    QModelIndexList indexes = this->treeView->selectionModel()->selectedIndexes();
    QModelIndex index;
    if (indexes.size() > 0)
    {
        index = indexes.at(0);
    }
    else
    {
        index = this->treeView->selectionModel()->currentIndex();
    }
    if (m_model->insertRows(0, 1, kind, index))
    {
        this->treeView->setExpanded(index, true);
        for(int col=0 ; col<m_model->columnCount() ; col++)
        {
            this->treeView->resizeColumnToContents(col);
        }
    }
}
