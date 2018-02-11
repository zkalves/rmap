from PyQt5 import QtCore, QtWidgets

class EAQTableView(QtWidgets.QTableView):
    def setSelection(self,rect,flags):
        top_left_point     = rect.topLeft()
        bottom_right_point = rect.bottomRight()
        bottom_right_point.setY(top_left_point.y())

        top_left     = self.indexAt(top_left_point)
        bottom_right = self.indexAt(bottom_right_point)
        selection    = QtCore.QItemSelection(top_left,bottom_right)
        self.selectionModel().select(selection, flags)

