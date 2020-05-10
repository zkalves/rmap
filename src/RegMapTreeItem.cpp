#include "RegMapTreeItem.hpp"

RegMapTreeItem::RegMapTreeItem(QVector<QVariant> &data, RegMapTreeItem *parent)
    : m_itemData(data), m_parentItem(parent)
{}

RegMapTreeItem::~RegMapTreeItem()
{
    qDeleteAll(m_childItems);
}

RegMapTreeItem *RegMapTreeItem::child(int row)
{
    if (row < 0 || row >= m_childItems.size())
        return nullptr;
    return m_childItems.at(row);
}

int RegMapTreeItem::childCount() const
{
    return m_childItems.count();
}

int RegMapTreeItem::childNumber()
{
    if (m_parentItem != nullptr)
    {
        return(m_parentItem->m_childItems.indexOf(this));
    }
    else
    {
        return(-1);
    }
}

int RegMapTreeItem::columnCount() const
{
    return m_itemData.count();
}

QVariant RegMapTreeItem::data(int column) const
{
    if (column < 0 || column >= m_itemData.size())
        return QVariant();
    return m_itemData.at(column);
}

RegMapTreeItem *RegMapTreeItem::parentItem()
{
    return m_parentItem;
}

void RegMapTreeItem::appendChild(RegMapTreeItem *item)
{
    m_childItems.append(item);
}

int RegMapTreeItem::row() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<RegMapTreeItem*>(this));

    return 0;
}


//    def insertChildren(self, position, count, columns, kind):
//        if position < 0 or position > len(self.childItems) or kind.__name__ not in self.get_possible_children():
//            insert_status = False
//        else:
//            for row in range(count):
//                data = ["NA" for v in range(columns)]
//                item = kind(data, self)
//                self.childItems.insert(position, item)
//            insert_status = True

//        return insert_status

//    def insertColumns(self, position, columns):
//        if position < 0 or position > len(self.itemData):
//            insert_status = False
//        else:
//            for column in range(columns):
//                self.itemData.insert(position, None)

//            for child in self.childItems:
//                child.insertColumns(position, columns)
//            insert_status = True

//        return insert_status

//    def removeChildren(self, position, count):
//        if position < 0 or position + count > len(self.childItems):
//            return False

//        for row in range(count):
//            self.childItems.pop(position)

//        return True

//    def removeColumns(self, position, columns):
//        if position < 0 or position + columns > len(self.itemData):
//            return False

//        for column in range(columns):
//            self.itemData.pop(position)

//        for child in self.childItems:
//            child.removeColumns(position, columns)

//        return True

//    def setData(self, column, value):
//        if column < 0 or column >= len(self.itemData):
//            set_status = False
//        else:
//            self.itemData[column] = value
//            set_status = True

//        return set_status

//class RegMapRootItem(RegMapItem):
//    yaml_loader = yaml.SafeLoader
//    yaml_dumper = yaml.SafeDumper
//    yaml_tag    = u'!root'
//    def __init__(self, data, parent=None):
//        super().__init__(data, parent)
//        self.kind = "root"
//    def get_icon(self):
//        return("")
//    def get_possible_children(self):
//        return(['RegMapBlockItem'])

//class RegMapMemItem(RegMapItem):
//    yaml_loader = yaml.SafeLoader
//    yaml_dumper = yaml.SafeDumper
//    yaml_tag    = u'!mem'
//    def __init__(self, data, parent=None):
//        super().__init__(data, parent)
//        self.kind = "mem"
//    def get_icon(self):
//        return(":/icons/uvm_mem_small.png")
//    def get_possible_children(self):
//        return([])

//class RegMapMapItem(RegMapItem):
//    yaml_loader = yaml.SafeLoader
//    yaml_dumper = yaml.SafeDumper
//    yaml_tag    = u'!map'
//    def __init__(self, data, parent=None):
//        super().__init__(data, parent)
//        self.kind = "map"
//    def get_icon(self):
//        return(":/icons/uvm_map_small.png")
//    def get_possible_children(self):
//        return([])

//class RegMapFieldItem(RegMapItem):
//    yaml_loader = yaml.SafeLoader
//    yaml_dumper = yaml.SafeDumper
//    yaml_tag    = u'!fld'
//    def __init__(self, data, parent=None):
//        super().__init__(data, parent)
//        self.kind = "fld"
//    def get_icon(self):
//        return(":/icons/uvm_reg_field_small.png")
//    def get_possible_children(self):
//        return([])

//class RegMapRegItem(RegMapItem):
//    yaml_loader = yaml.SafeLoader
//    yaml_dumper = yaml.SafeDumper
//    yaml_tag    = u'!reg'
//    def __init__(self, data, parent=None):
//        super().__init__(data, parent)
//        self.kind = "reg"
//    def get_icon(self):
//        return(":/icons/uvm_reg_small.png")
//    def get_possible_children(self):
//        return(['RegMapFieldItem'])

//class RegMapBlockItem(RegMapItem):
//    yaml_loader = yaml.SafeLoader
//    yaml_dumper = yaml.SafeDumper
//    yaml_tag    = u'!blk'
//    def __init__(self, data, parent=None):
//        super().__init__(data, parent)
//        self.kind = "blk"
//    def get_icon(self):
//        return(":/icons/uvm_reg_block_small.png")
//    def get_possible_children(self):
//        return(['RegMapMemItem', 'RegMapMapItem', 'RegMapRegItem', 'RegMapBlockItem'])
