#include "RegMapTreeModel.hpp"

#include <QStringList>

RegMapTreeModel::RegMapTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    QVector<QVariant> headers {tr("Type"), tr("Offset"), tr("Size"), tr("Name"), tr("Description")};
    m_rootItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::root, headers);
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

    RegMapTreeItem *item = static_cast<RegMapTreeItem*>(index.internalPointer());

    if(role == Qt::DecorationRole)
    {
        if(index.column() == 0)
        {
            return(QPixmap(item->get_icon()).scaled(QSize(20,20),  Qt::KeepAspectRatio));
        }
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();


    return item->data(index.column());

}

Qt::ItemFlags RegMapTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return (Qt::NoItemFlags);
    }

    if (index.column() == 0)
    {
        return (QAbstractItemModel::flags(index));
    }
    else
    {
        return (Qt::ItemIsEditable | QAbstractItemModel::flags(index));
    }
}

QVariant RegMapTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return m_rootItem->data(section);

    return QVariant();
}

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

//    def rowCount(self, parent=QModelIndex()):
//        parentItem = self.getItem(parent)

//        return parentItem.childCount()

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


//    def print_hier(self,start, dlm = ""):
//        dlm = dlm + " "
//        for item in start.childItems:
//            print(dlm + item.kind)
//            self.print_hier(item,dlm)


RegMapTreeItem* RegMapTreeModel::getItem(QModelIndex index)
{
    if (index.isValid())
    {
        RegMapTreeItem* item = (RegMapTreeItem*)index.internalPointer();
        if(item)
        {
            return (item);
        }
    }
    return this->m_rootItem;
}

RegMapTreeItem* RegMapTreeModel::getRootItem(void)
{
    return this->m_rootItem;
}

bool RegMapTreeModel::insertRows(int position, int rows, RegMapTreeItem::e_rmmKind kind, QModelIndex parent)
{
    bool success;
    RegMapTreeItem* parentItem = getItem(parent);
    QVector<RegMapTreeItem::e_rmmKind> possible_children = parentItem->get_possible_children();

    if(possible_children.contains(kind))
    {
        this->beginInsertRows(parent, position, position + rows - 1);
        success = parentItem->insertChildren( kind,
                                              position,
                                              rows,
                                              m_rootItem->columnCount());
        this->endInsertRows();
        initRow(position,parent);
    }
    else
    {
        success = false;
    }
    return(success);
}

void RegMapTreeModel::initRow(int row, QModelIndex index)
{
    RegMapTreeItem* parentItem = getItem(index);
    RegMapTreeItem* childItem = parentItem->child(row);
    for(int column=0; column < columnCount(index) ; column++)
    {
        QModelIndex child = this->index(row, column, index);
        if (column == 0)
        {
            this->setData(child, (childItem->getKindString()), Qt::EditRole);
        }
        else
        {
            this->setData(child, "", Qt::EditRole);
        }
    }
}

//    def parent(self, index):
//        if not index.isValid():
//            return QModelIndex()

//        childItem = self.getItem(index)
//        parentItem = childItem.parent()

//        if parentItem == self.rootItem:
//            return QModelIndex()

//        return self.createIndex(parentItem.childNumber(), 0, parentItem)

bool RegMapTreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    bool success;
    RegMapTreeItem* parentItem = getItem(parent);
    this->beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    this->endRemoveRows();
    return(success);
}

bool RegMapTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool set_data_status;
    if (role != Qt::EditRole)
    {
        set_data_status = false;
    }
    else
    {
        RegMapTreeItem* item = getItem(index);
        set_data_status = item->setData(index.column(), value);
        if(set_data_status)
        {
            emit dataChanged(index, index);
        }
    }
    return (set_data_status);
}

void RegMapTreeModel::recursiveCheckData(RegMapTreeItem *node)
{
//        # self.checkItemData(node.itemData)
//        # if node.isValid():
//        #     print("aaa")
    Q_FOREACH (RegMapTreeItem* child, (QVector<RegMapTreeItem*>)node->getChildItems())
    {
        recursiveCheckData(child);
    }
}

void RegMapTreeModel::checkData(void)
{
    recursiveCheckData(m_rootItem);
}
