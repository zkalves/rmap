#include "RegMapWindow.hpp"

RegMapWindow::RegMapWindow(QString &rmap_filename, QWidget *parent) :
    QMainWindow(parent)
{
    this->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    m_active_folder = ".";
    m_is_regmap_modified = false;
    m_rmap_filename = rmap_filename;
    m_default_filename = "rmap.yaml";
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
    connect(actionAddMem,       &QAction::triggered, this, &RegMapWindow::btnAddMem);
    connect(actionAddRegBlock,  &QAction::triggered, this, &RegMapWindow::btnAddRegBlock);
    connect(actionAddRegField,  &QAction::triggered, this, &RegMapWindow::btnAddRegField);
    connect(actionDeleteItem,   &QAction::triggered, this, &RegMapWindow::btnDeleteItem);
    connect(actionAddRegMap,    &QAction::triggered, this, &RegMapWindow::btnAddRegMap);
    connect(actionAddReg,       &QAction::triggered, this, &RegMapWindow::btnAddReg);
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
    this->treeView->setItemDelegateForColumn(1,new RegHexDecBinDelegate);
    this->treeView->setItemDelegateForColumn(2,new RegHexDecBinDelegate);
    this->treeView->setItemDelegateForColumn(3,new RegStrDelegate);
    this->treeView->setItemDelegateForColumn(4,new RegMapDelegate);

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
    if (!m_is_regmap_modified)
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
    dialog.setNameFilter("rmap files (*.yaml)");
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if(m_rmap_filename.isNull() || m_rmap_filename.isEmpty())
    {
        fname = m_default_filename;
    }
    else
    {
        fname = m_rmap_filename;
        if (!fname.endsWith(".yaml"))
        {
            QString croped_fname=fname.split(".",QString::SkipEmptyParts).at(0);
            croped_fname.append(".yaml");
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
                                                         "rmap files (*.yaml)");
            fileOpen(fname);
            filename = m_default_window_title;
            filename.append(" - ");
            filename.append(fname);
            this->setWindowTitle(filename);
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

void RegMapWindow::btnExport(void)
{
    qDebug() << "export";
}

void RegMapWindow::btnAddMem(void)
{
    insertChild(RegMapTreeItem::e_rmmKind::mem);
}

void RegMapWindow::btnAddRegBlock(void)
{
    insertChild(RegMapTreeItem::e_rmmKind::blk);
}


void RegMapWindow::btnAddRegField(void)
{
    insertChild(RegMapTreeItem::e_rmmKind::fld);
}

void RegMapWindow::btnAddRegMap(void)
{
    insertChild(RegMapTreeItem::e_rmmKind::map);
}

void RegMapWindow::btnAddReg(void)
{
    insertChild(RegMapTreeItem::e_rmmKind::reg);
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
    recursive_delete(m_model->getRootItem());
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
//            try:
//                fh = open(fname,"r")
//                self.__model.rootItem = yaml.safe_load(fh)
//                fh.close()
//                self.rmap_filename = fname
//                self.treeView.setItemsExpandable(True)
//                self.treeView.expandAll()
//                for col in range(self.__model.columnCount()):
//                    self.treeView.resizeColumnToContents(col)
//            except Exception as E:
//                print("[ERROR] Could not open file", e)
    }
}

bool RegMapWindow::fileSave(QString fname)
{
    bool save_status = false;
    if(m_rmap_filename.isNull() || m_rmap_filename.isEmpty())
    {
        fname = m_rmap_filename;
        if (!fname.endsWith(".yaml"))
        {
            QString croped_fname=fname.split(".",QString::SkipEmptyParts).at(0);
            croped_fname.append(".yaml");
            fname=croped_fname;
        }
//        try:
//            fh = open(fname,"w")
//            yaml.safe_dump(self.__model.rootItem, fh)
//            fh.close()
//            self.regmap_notModified()
//            save_status = True
//        except Exception as e:
//            print("[ERROR] Could not save file", e)
    }
    return(save_status);
}

void RegMapWindow::regmap_modified(void)
{
    if(!m_is_regmap_modified)
    {
        m_is_regmap_modified = true;
        QString win_title = this->windowTitle();
        if (win_title.endsWith('*'))
        {
            win_title.append('*');
        }
        this->setWindowTitle(win_title);
    }
}

void RegMapWindow::regmap_notModified(void)
{
    if(!m_is_regmap_modified)
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

void RegMapWindow::recursive_delete(RegMapTreeItem* obj)
{
    Q_FOREACH (RegMapTreeItem* child, (QVector<RegMapTreeItem*>)obj->getChildItems())
    {
        recursive_delete(child);
        delete child;
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
