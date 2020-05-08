#include "RegMapWindow.hpp"

RegMapWindow::RegMapWindow(QString &rmap_filename, QWidget *parent) :
    QMainWindow(parent)
{
    setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    m_active_folder = ".";
    m_is_regmap_modified = false;
    m_rmap_filename = rmap_filename;
    m_default_filename = "rmap.yaml";
    m_default_window_title = windowTitle();
//        # Set table model and attributes
//        self.__model    = RegMapModel()
//        self.__model.dataChanged.connect(self.regmap_modified)

//        self.treeView.setSelectionBehavior(QAbstractItemView.SelectRows)
//        self.treeView.setModel(self.__model)
//        self.treeView.setAlternatingRowColors(True)
//        self.actionFileNew.triggered.connect(self.btnFileNew)
//        self.actionFileOpen.triggered.connect(self.btnFileOpen)
//        self.actionFileSave.triggered.connect(self.btnFileSave)
//        self.actionFileSaveAs.triggered.connect(self.btnFileSaveAs)
//        self.actionFileReload.triggered.connect(self.btnFileReload)
//        self.actionAddMem.triggered.connect(self.btnAddMem)
//        self.actionAddRegBlock.triggered.connect(self.btnAddRegBlock)
//        self.actionAddRegField.triggered.connect(self.btnAddRegField)
//        self.actionDeleteItem.triggered.connect(self.btnDeleteItem)
//        self.actionAddRegMap.triggered.connect(self.btnAddRegMap)
//        self.actionAddReg.triggered.connect(self.btnAddReg)
//        self.actionCheck.triggered.connect(self.btnCheck)
//        self.actionExport.triggered.connect(self.btnExport)
    connect(actionQuit,   &QAction::triggered, this, &RegMapWindow::btnQuitButton);
    connect(actionAbout,  &QAction::triggered, this, &RegMapWindow::btnAbout);
    connect(actionConfig, &QAction::triggered, this, &RegMapWindow::btnConfig);

    m_config_window = new RegConfigWindow(this);

//        # if not self.rmap_filename:
//        #     self.rmap_filename = self.default_filename
//        if self.rmap_filename:
//            self.fileOpen(self.rmap_filename)
//        self.__hexDelegate = RegHexDecBinDelegate()
//        self.__strDelegate = RegStrDelegate()
//        self.__allDelegate = RegMapDelegate()
//        self.treeView.setItemDelegateForColumn(1,self.__hexDelegate)
//        self.treeView.setItemDelegateForColumn(2,self.__hexDelegate)
//        self.treeView.setItemDelegateForColumn(3,self.__strDelegate)
//        self.treeView.setItemDelegateForColumn(4,self.__allDelegate)

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

//    def btnFileNew(self):
//        if self.is_regmap_modified:
//            flags  = QMessageBox.Ok
//            flags |= QMessageBox.Save
//            flags |= QMessageBox.Cancel

//            result = QMessageBox.warning(self, "New file",
//                                                "This action will remove all unsaved data, do you wish to continue?",
//                                                flags)

//            if result == QMessageBox.Save:
//                save_status = self.btnFileSave()

//        if not self.is_regmap_modified or result == QMessageBox.Ok or (result == QMessageBox.Save and save_status):
//            self.fileNew()

//    def btnFileSave(self):
//        save_status = False
//        if not self.rmap_filename:
//            save_status = self.btnFileSaveAs()
//        else:
//            save_status = self.fileSave()
//        return save_status

//    def btnFileSaveAs(self):
//        save_status = False
//        dialog = QFileDialog(self)
//        dialog.setFileMode(QFileDialog.AnyFile)
//        dialog.setNameFilter("rmap files (*.yaml)");
//        dialog.setViewMode(QFileDialog.Detail)
//        dialog.setAcceptMode(QFileDialog.AcceptSave)
//        if not self.rmap_filename:
//            fname = self.default_filename
//        else:
//            fname = self.rmap_filename
//            if not fname.endswith(".yaml"):
//                fname  = fname.strip(".")
//                fname += ".yaml"
//        dialog.selectFile(fname)
//        if dialog.exec_():
//            fname = dialog.selectedFiles()
//            fname = fname[0]
//            save_status = self.fileSave(fname)
//            if save_status:
//                self.rmap_filename = fname
//            self.setWindowTitle(self.default_window_title + " - " + fname)
//        return save_status


//    def btnFileOpen(self):
//        if self.is_regmap_modified:
//            flags  = QMessageBox.Ok
//            flags |= QMessageBox.Cancel

//            result = QMessageBox.warning(self, "Open file",
//                                                "This action will remove all unsaved data, do you wish to continue?",
//                                                flags)
//        if not self.is_regmap_modified or result   == QMessageBox.Ok:
//            fname, _ = QFileDialog.getOpenFileName( self,
//                                                    'Open file',
//                                                    self.active_folder,
//                                                    "rmap files (*.yaml)")
//            self.fileOpen(fname)
//            self.setWindowTitle(self.default_window_title + " - " + fname)

//    def btnFileReload(self):
//        if self.is_regmap_modified:
//            flags  = QMessageBox.Ok
//            flags |= QMessageBox.Cancel

//            result = QMessageBox.warning(self, "Reload file",
//                                                "This action will remove all unsaved data, do you wish to continue?",
//                                                flags)
//        if not self.is_regmap_modified or result   == QMessageBox.Ok:
//            fname = self.rmap_filename
//            self.fileOpen(fname)
//            self.setWindowTitle(self.default_window_title + " - " + fname)

//    def btnCheck(self):
//        model = self.treeView.model()
//        model.checkData()

//    def btnExport(self):
//        print("export")

//    def btnAddMem(self):
//        self.insertChild(RegMapMemItem)

//    def btnAddRegBlock(self):
//        self.insertChild(RegMapBlockItem)

//    def btnAddRegField(self):
//        self.insertChild(RegMapFieldItem)

//    def btnAddRegMap(self):
//        self.insertChild(RegMapMapItem)

//    def btnAddReg(self):
//        self.insertChild(RegMapRegItem)

//    def btnDeleteItem(self):
//        if len(self.treeView.selectedIndexes()) > 0:
//            index = self.treeView.selectedIndexes()[0]
//        else:
//            index = self.treeView.selectionModel().currentIndex()
//        # index = self.treeView.selectionModel().currentIndex()
//        if index.row() >= 0:
//            model = self.treeView.model()
//            model.removeRows(index.row(), 1, index.parent())

//    def fileNew(self):
//        self.recursive_delete(self.__model.rootItem)
//        del(self.__model)
//        gc.collect()
//        self.__model = RegMapModel()
//        self.__model.dataChanged.connect(self.regmap_modified)
//        self.treeView.setModel(self.__model)
//        self.regmap_notModified()
//        self.rmap_filename = ''

//    def fileOpen(self,fname):
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

//    def fileSave(self, fname=None):
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

//    def regmap_modified(self):
//        if not self.is_regmap_modified:
//            self.is_regmap_modified = True
//            win_title = self.windowTitle()
//            if not win_title.endswith('*'):
//                win_title += '*'
//            self.setWindowTitle(win_title)

//    def regmap_notModified(self):
//        if self.is_regmap_modified:
//            self.is_regmap_modified = False
//            win_title = self.windowTitle()
//            if win_title.endswith('*'):
//                win_title = win_title.strip('*')
//            self.setWindowTitle(win_title)

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

