from PyQt5 import QtCore, QtGui

class RegMapTableModel(QtCore.QAbstractTableModel):
    def __init__(self, regData = [[]], headers = [], parent = None):
        QtCore.QAbstractTableModel.__init__(self, parent)
        self.__regData = regData
        self.__headers = headers

    def rowCount(self, parent):
        return len(self.__regData)

    def columnCount(self, parent):
        return len(self.__regData[0])

    def flags(self, index):
        return QtCore.Qt.ItemIsEditable | QtCore.Qt.ItemIsEnabled | QtCore.Qt.ItemIsSelectable

    def data(self, index, role):
        if role == QtCore.Qt.EditRole:
            row    = index.row()
            column = index.column()
            return self.__regData[row][column]
        # if role == QtCore.Qt.ToolTipRole:
        #     row    = index.row()
        #     column = index.column()
        #     return "Hex code: " + self.__regData[row][column]
        # if role == QtCore.Qt.DecorationRole:
        #     row    = index.row()
        #     column = index.column()
        #     value  = self.__regData[row][column]
        #     pixmap = QtGui.QPixmap(26, 26)
        #     pixmap.fill(value)
        #     icon   = QtGui.QIcon(pixmap)
        #     return icon
        if role == QtCore.Qt.DisplayRole:
            row    = index.row()
            column = index.column()
            value  = self.__regData[row][column]
            return value

    def setData(self, index, value, role = QtCore.Qt.EditRole):
        if role == QtCore.Qt.EditRole:
            row    = index.row()
            column = index.column()
            # color  = QtGui.QColor(value)
            # if color.isValid():
                # self.__regData[row][column] = color
            self.__regData[row][column] = value
            self.dataChanged.emit(index, index)
            return True
        return False

    def headerData(self, section, orientation, role):
        if role == QtCore.Qt.DisplayRole:
            if orientation == QtCore.Qt.Horizontal:
                if section < len(self.__headers):
                    return self.__headers[section]
                else:
                    return "not implemented"
            else:
                return '0x{0}'.format(section)

    #=====================================================#
    #INSERTING & REMOVING
    #=====================================================#
    def setAddressBusWidth(self):
        self.__regData = [[0]*32]*16

    def insertRows(self, position, rows, parent = QtCore.QModelIndex()):
        self.beginInsertRows(parent, position, position + rows - 1)
        for i in range(rows):
            defaultValues = [QtGui.QColor("#000000") for i in range(self.columnCount(None))]
            self.__regData.insert(position, defaultValues)
        self.endInsertRows()
        return True

    def insertColumns(self, position, columns, parent = QtCore.QModelIndex()):
        self.beginInsertColumns(parent, position, position + columns - 1)
        rowCount = len(self.__regData)
        for i in range(columns):
            for j in range(rowCount):
                self.__regData[j].insert(position, QtGui.QColor("#000000"))
        self.endInsertColumns()
        return True


