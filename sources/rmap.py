#!/usr/bin/env python3
from PyQt5 import QtWidgets
from PyQt5.QtWidgets import QAbstractItemView
from PyQt5.QtCore import pyqtSlot
from RegMapTableModel import RegMapTableModel
from rmap_ui import Ui_rmap
import sys

class RegMapWindow(QtWidgets.QMainWindow, Ui_rmap):
    def __init__(self):
        super(RegMapWindow, self).__init__()
        self.setupUi(self)
        self.__model    = RegMapTableModel([['']*32]*16, ['b'+str(x) for x in range(31,-1,-1)])
        self.tableView.setModel(self.__model)
        self.tableView.verticalHeader().setDefaultSectionSize(30)
        self.tableView.verticalHeader().setSectionsClickable(False)
        self.tableView.horizontalHeader().setDefaultSectionSize(30)
        self.tableView.horizontalHeader().setSectionsClickable(False)
        self.tableView.setEditTriggers(QAbstractItemView.NoEditTriggers)
        self.tableView.SelectionBehavior(QAbstractItemView.SelectItems)
        self.tableView.SelectionMode(QAbstractItemView.ContiguousSelection)
        self.tableView.doubleClicked.connect(self.onDoubleClick)
        self.__max_addr = 0
        self.__max_data = 0
        self.updateButton.clicked.connect(self.btn_updateButton_clicked)

    @pyqtSlot()
    def onDoubleClick(self):
        row = self.tableView.currentIndex().row()
        self.tableView.selectRow(row)

    def btn_updateButton_clicked(self):
        self.__max_addr = self.addressBusWidth.text()
        self.__max_data = self.dataBusWidth.text()
        self.__model.setAddressBusWidth()

def main():
    app = QtWidgets.QApplication(sys.argv)
    main_window = RegMapWindow()
    main_window.show()
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()

