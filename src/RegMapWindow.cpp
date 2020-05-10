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
    if(!m_rmap_filename.isNull())
    {
        fname = m_default_filename;
    }
    else
    {
        fname = m_rmap_filename;
        if (!fname.endsWith(".yaml"))
        {
                //fname  = fname.strip(".")
                fname.append(".yaml");
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
//        model = self.treeView.model()
//        model.checkData()
}

void RegMapWindow::btnExport(void)
{
//        print("export")
}

void RegMapWindow::btnAddMem(void)
{
//        self.insertChild(RegMapMemItem)
}

void RegMapWindow::btnAddRegBlock(void)
{
//        self.insertChild(RegMapBlockItem)
}


void RegMapWindow::btnAddRegField(void)
{
//        self.insertChild(RegMapFieldItem)
}

void RegMapWindow::btnAddRegMap(void)
{
//        self.insertChild(RegMapMapItem)
}

void RegMapWindow::btnAddReg(void)
{
//        self.insertChild(RegMapRegItem)
}

void RegMapWindow::btnDeleteItem(void)
{
//        if len(self.treeView.selectedIndexes()) > 0:
//            index = self.treeView.selectedIndexes()[0]
//        else:
//            index = self.treeView.selectionModel().currentIndex()
//        # index = self.treeView.selectionModel().currentIndex()
//        if index.row() >= 0:
//            model = self.treeView.model()
//            model.removeRows(index.row(), 1, index.parent())
}

void RegMapWindow::fileNew(void)
{
//        self.recursive_delete(self.__model.rootItem)
//        del(self.__model)
//        gc.collect()
//        self.__model = RegMapModel()
//        self.__model.dataChanged.connect(self.regmap_modified)
//        self.treeView.setModel(self.__model)
//        self.regmap_notModified()
//        self.rmap_filename = ''
}

void RegMapWindow::fileOpen(QString fname)
{
//        if os.path.exists(fname):
//            self.fileNew()
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

bool RegMapWindow::fileSave(QString fname)
{
//        save_status = False
//        if not fname:
//            fname = self.rmap_filename
//        if not fname.endswith(".yaml"):
//            fname  = fname.strip(".")
//            fname += ".yaml"
//        try:
//            fh = open(fname,"w")
//            yaml.safe_dump(self.__model.rootItem, fh)
//            fh.close()
//            self.regmap_notModified()
//            save_status = True
//        except Exception as e:
//            print("[ERROR] Could not save file", e)
//        return save_status
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

//    def recursive_delete(self,obj):
//        for child in obj.childItems:
//            self.recursive_delete(child)
//            del(child)

//    def insertChild(self, kind):
//        # The issue is when:
//        # 1. Create item.
//        # 2. Click on the second column
//        # 3. Create second item
//        # Works OK if only click on first column when creating items

//        if len(self.treeView.selectedIndexes()) > 0:
//            index = self.treeView.selectedIndexes()[0]
//        else:
//            index = self.treeView.selectionModel().currentIndex()
//        model = self.treeView.model()
//        if model.insertRows(0, 1, kind, index):
//            self.treeView.setExpanded(index, True)
//            for col in range(model.columnCount()):
//                self.treeView.resizeColumnToContents(col)

