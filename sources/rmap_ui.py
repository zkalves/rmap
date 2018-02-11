# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui/rmap.ui'
#
# Created by: PyQt5 UI code generator 5.9.2
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_rmap(object):
    def setupUi(self, rmap):
        rmap.setObjectName("rmap")
        rmap.resize(640, 483)
        self.centralwidget = QtWidgets.QWidget(rmap)
        self.centralwidget.setMaximumSize(QtCore.QSize(640, 16777215))
        self.centralwidget.setObjectName("centralwidget")
        self.horizontalLayout_2 = QtWidgets.QHBoxLayout(self.centralwidget)
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        self.tabWidget = QtWidgets.QTabWidget(self.centralwidget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.tabWidget.sizePolicy().hasHeightForWidth())
        self.tabWidget.setSizePolicy(sizePolicy)
        self.tabWidget.setMinimumSize(QtCore.QSize(127, 0))
        self.tabWidget.setObjectName("tabWidget")
        self.config = QtWidgets.QWidget()
        self.config.setObjectName("config")
        self.formLayout = QtWidgets.QFormLayout(self.config)
        self.formLayout.setObjectName("formLayout")
        self.label = QtWidgets.QLabel(self.config)
        self.label.setObjectName("label")
        self.formLayout.setWidget(0, QtWidgets.QFormLayout.LabelRole, self.label)
        self.dataBusWidth = QtWidgets.QLineEdit(self.config)
        self.dataBusWidth.setObjectName("dataBusWidth")
        self.formLayout.setWidget(0, QtWidgets.QFormLayout.FieldRole, self.dataBusWidth)
        self.label_2 = QtWidgets.QLabel(self.config)
        self.label_2.setObjectName("label_2")
        self.formLayout.setWidget(1, QtWidgets.QFormLayout.LabelRole, self.label_2)
        self.addressBusWidth = QtWidgets.QLineEdit(self.config)
        self.addressBusWidth.setObjectName("addressBusWidth")
        self.formLayout.setWidget(1, QtWidgets.QFormLayout.FieldRole, self.addressBusWidth)
        self.updateButton = QtWidgets.QPushButton(self.config)
        self.updateButton.setObjectName("updateButton")
        self.formLayout.setWidget(2, QtWidgets.QFormLayout.LabelRole, self.updateButton)
        self.tabWidget.addTab(self.config, "")
        self.direct_map = QtWidgets.QWidget()
        self.direct_map.setObjectName("direct_map")
        self.verticalLayout = QtWidgets.QVBoxLayout(self.direct_map)
        self.verticalLayout.setObjectName("verticalLayout")
        self.tableView = EAQTableView(self.direct_map)
        self.tableView.setObjectName("tableView")
        self.verticalLayout.addWidget(self.tableView)
        self.tabWidget.addTab(self.direct_map, "")
        self.horizontalLayout_2.addWidget(self.tabWidget)
        rmap.setCentralWidget(self.centralwidget)
        self.menubar = QtWidgets.QMenuBar(rmap)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 640, 25))
        self.menubar.setObjectName("menubar")
        rmap.setMenuBar(self.menubar)

        self.retranslateUi(rmap)
        self.tabWidget.setCurrentIndex(1)
        QtCore.QMetaObject.connectSlotsByName(rmap)

    def retranslateUi(self, rmap):
        _translate = QtCore.QCoreApplication.translate
        rmap.setWindowTitle(_translate("rmap", "Register Map Generation Tool"))
        self.label.setText(_translate("rmap", "Data bus width"))
        self.label_2.setText(_translate("rmap", "Address bus width"))
        self.updateButton.setText(_translate("rmap", "Update"))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.config), _translate("rmap", "Configure"))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.direct_map), _translate("rmap", "Direct map"))

from eaqtableview import EAQTableView

if __name__ == "__main__":
    import sys
    app = QtWidgets.QApplication(sys.argv)
    rmap = QtWidgets.QMainWindow()
    ui = Ui_rmap()
    ui.setupUi(rmap)
    rmap.show()
    sys.exit(app.exec_())

