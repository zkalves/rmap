#include "RegMapTreeModel.hpp"

#include <QStringList>

RegMapTreeModel::RegMapTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    QVector<QVariant> headers {tr("Type"), tr("Offset"), tr("Size"), tr("Name"), tr("Description")};
    m_rootItem = new RegMapTreeItem(headers);
}

RegMapTreeModel::~RegMapTreeModel()
{
    delete m_rootItem;
}

int RegMapTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<RegMapTreeItem*>(parent.internalPointer())->columnCount();
    return m_rootItem->columnCount();
}

QVariant RegMapTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    RegMapTreeItem *item = static_cast<RegMapTreeItem*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags RegMapTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index);
}

QVariant RegMapTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return m_rootItem->data(section);

    return QVariant();
}

QModelIndex RegMapTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    RegMapTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<RegMapTreeItem*>(parent.internalPointer());

    RegMapTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex RegMapTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    RegMapTreeItem *childItem = static_cast<RegMapTreeItem*>(index.internalPointer());
    RegMapTreeItem *parentItem = childItem->parentItem();

    if (parentItem == m_rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int RegMapTreeModel::rowCount(const QModelIndex &parent) const
{
    RegMapTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<RegMapTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void RegMapTreeModel::setupModelData(const QStringList &lines, RegMapTreeItem *parent)
{
    QVector<RegMapTreeItem*> parents;
    QVector<int> indentations;
    parents << parent;
    indentations << 0;

    int number = 0;

    while (number < lines.count()) {
        int position = 0;
        while (position < lines[number].length()) {
            if (lines[number].at(position) != ' ')
                break;
            position++;
        }

        const QString lineData = lines[number].mid(position).trimmed();

        if (!lineData.isEmpty()) {
            // Read the column data from the rest of the line.
            const QStringList columnStrings = lineData.split('\t', QString::SkipEmptyParts);
            QVector<QVariant> columnData;
            columnData.reserve(columnStrings.count());
            for (const QString &columnString : columnStrings)
                columnData << columnString;

            if (position > indentations.last()) {
                // The last child of the current parent is now the new parent
                // unless the current parent has no children.

                if (parents.last()->childCount() > 0) {
                    parents << parents.last()->child(parents.last()->childCount()-1);
                    indentations << position;
                }
            } else {
                while (position < indentations.last() && parents.count() > 0) {
                    parents.pop_back();
                    indentations.pop_back();
                }
            }

            // Append a new item to the current parent's list of children.
            parents.last()->appendChild(new RegMapTreeItem(columnData, parents.last()));
        }
        ++number;
    }
}

//class RegMapModel(QAbstractItemModel):
//    def __init__(self, parent=None):
//        super(RegMapModel, self).__init__(parent)

//        self.headers      = ["Type", "Offset", "Size", "Name", "Description"]
//        self.rootItem = RegMapRootItem(self.headers)

//    def columnCount(self, parent=QModelIndex()):
//        return self.rootItem.columnCount()

//    def data(self, index, role):
//        if not index.isValid():
//            return None

//        item = self.getItem(index)

//        if role == Qt.DecorationRole:
//            if index.column() == 0:
//                return QPixmap(item.get_icon()).scaled(QSize(20,20),  Qt.KeepAspectRatio)
//        # if role == Qt.BackgroundColorRole:
//        #     return (QBrush(Qt.blue))

//        if role != Qt.DisplayRole and role != Qt.EditRole:
//            return None

//        return item.data(index.column())

//    def print_hier(self,start, dlm = ""):
//        dlm = dlm + " "
//        for item in start.childItems:
//            print(dlm + item.kind)
//            self.print_hier(item,dlm)

//    def flags(self, index):
//        if not index.isValid():
//            return 0
//        if index.column() == 0:
//            return super(RegMapModel, self).flags(index)
//        else:
//            return Qt.ItemIsEditable | super(RegMapModel, self).flags(index)

//    def getItem(self, index):
//        if index.isValid():
//            item = index.internalPointer()
//            if item:
//                return item

//        return self.rootItem

//    def headerData(self, section, orientation, role=Qt.DisplayRole):
//        if orientation == Qt.Horizontal and role == Qt.DisplayRole:
//            return self.rootItem.data(section)

//        return None

//    def index(self, row, column, parent=QModelIndex()):
//        if self.hasIndex(row,column,parent):
//            if parent.isValid() and parent.column() != 0:
//                return QModelIndex()

//            parentItem = self.getItem(parent)
//            childItem = parentItem.child(row)
//            if childItem:
//                return self.createIndex(row, column, childItem)
//            else:
//                return QModelIndex()
//        else:
//            return QModelIndex()

//    def insertColumns(self, position, columns, parent=QModelIndex()):
//        self.beginInsertColumns(parent, position, position + columns - 1)
//        success = self.rootItem.insertColumns(position, columns)
//        self.endInsertColumns()

//        return success

//    def insertRows(self, position, rows, kind, parent=QModelIndex()):
//        parentItem = self.getItem(parent)
//        if kind.__name__ in parentItem.get_possible_children():
//            self.beginInsertRows(parent, position, position + rows - 1)
//            success = parentItem.insertChildren( position,
//                                                 rows,
//                                                 self.rootItem.columnCount(),
//                                                 kind)
//            self.endInsertRows()
//            self.initRow(position,parent)
//        else:
//            success = False
//        return success

//    def initRow(self,row,index):
//        parentItem = self.getItem(index)
//        childItem = parentItem.child(row)
//        for column in range(self.columnCount(index)):
//            child = self.index(row, column, index)
//            if column == 0:
//                self.setData(child, childItem.kind, Qt.EditRole)
//            # elif column > 2:
//            #     self.setData(child, "[No data]", Qt.EditRole)
//            else:
//                self.setData(child, "", Qt.EditRole)

//    def resetItems(self, path):
//       self.beginResetModel()
//       self.rootItem = self.rootItem
//       self.endResetModel()

//    def parent(self, index):
//        if not index.isValid():
//            return QModelIndex()

//        childItem = self.getItem(index)
//        parentItem = childItem.parent()

//        if parentItem == self.rootItem:
//            return QModelIndex()

//        return self.createIndex(parentItem.childNumber(), 0, parentItem)

//    def removeColumns(self, position, columns, parent=QModelIndex()):
//        self.beginRemoveColumns(parent, position, position + columns - 1)
//        success = self.rootItem.removeColumns(position, columns)
//        self.endRemoveColumns()

//        if self.rootItem.columnCount() == 0:
//            self.removeRows(0, self.rowCount())

//        return success

//    def removeRows(self, position, rows, parent=QModelIndex()):
//        parentItem = self.getItem(parent)
//        self.beginRemoveRows(parent, position, position + rows - 1)
//        success = parentItem.removeChildren(position, rows)
//        self.endRemoveRows()
//        return success

//    def rowCount(self, parent=QModelIndex()):
//        parentItem = self.getItem(parent)

//        return parentItem.childCount()

//    def setData(self, index, value, role=Qt.EditRole):
//        if role != Qt.EditRole:
//            set_data_status = False
//        else:
//            item = self.getItem(index)
//            set_data_status = item.setData(index.column(), value)
//            if set_data_status:
//                self.dataChanged.emit(index, index)
//        return set_data_status

//    def setHeaderData(self, section, orientation, value, role=Qt.EditRole):
//        if role != Qt.EditRole or orientation != Qt.Horizontal:
//            return False

//        result = self.rootItem.setData(section, value)
//        if result:
//            self.headerDataChanged.emit(orientation, section, section)

//        return result

//    # def checkItemData(self,itemData):
//    #     sdf
//    def recursiveCheckData(self,node):
//        # self.checkItemData(node.itemData)
//        # if node.isValid():
//        #     print("aaa")
//        for child in node.childItems:
//            self.recursiveCheckData(child)

//    def checkData(self):
//        self.recursiveCheckData(self.rootItem)
