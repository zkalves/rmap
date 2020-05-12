#include "RegMapTreeItem.hpp"

RegMapTreeItem::RegMapTreeItem(RegMapTreeItem::e_rmmKind kind, QVector<QVariant> &data, RegMapTreeItem *parent)
    : m_kind(kind), m_itemData(data), m_parentItem(parent)
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

QVector<RegMapTreeItem*> RegMapTreeItem::getChildItems(void)
{
    return m_childItems;
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


bool RegMapTreeItem::insertChildren(RegMapTreeItem::e_rmmKind kind, int position, int count, int columns)
{
    bool insert_status;
    //if((position < 0) || (position > this->m_childItems.size()) || kind != get_possible_children())
    if((position < 0) || (position > this->m_childItems.size()))
    {
        insert_status = false;
    }
    else
    {
        for(int row=0 ; row < count ; row++)
        {
            QVector<QVariant> data;
            data.reserve(columns);
            for(int i=0;i<columns;i++)
            {
                data.append("NA");
            }
            RegMapTreeItem item = RegMapTreeItem(kind, data, this);
            this->m_childItems.insert(position, &item);
        }
        insert_status = true;
    }

    return(insert_status);
}

bool RegMapTreeItem::insertColumns(int position, int columns)
{
    bool insert_status;
    if(position < 0 || position > m_itemData.size())
    {
        insert_status = false;
    }
    else
    {
        for(int column=0 ; column<columns ; column++)
        {
            m_itemData.insert(position, QString());
        }
        Q_FOREACH (RegMapTreeItem* child, m_childItems)
        {
            child->insertColumns(position, columns);
        }
        insert_status = true;
    }
    return(insert_status);
}

bool RegMapTreeItem::removeChildren(int position, int count)
{
    bool status = false;
    if (position < 0 || (position + count) > m_childItems.size())
    {
            status = false;
    }
    else
    {
        for (int row=0 ; row < count ; row++)
        {
            m_childItems.remove(position);
        }

        status = true;
    }
    return(status);
}

bool RegMapTreeItem::removeColumns(int position, int columns)
{
    bool status = false;
    if (position < 0 || (position + columns) > m_itemData.size())
    {
        status = false;
    }
    else
    {

        for (int column=0 ; column < columns ; column++)
        {
            m_itemData.remove(position);
        }

        Q_FOREACH (RegMapTreeItem* child, m_childItems)
        {
            child->removeColumns(position, columns);
        }
        status = true;
    }

    return status;
}

bool RegMapTreeItem::setData(int column, QVariant value)
{
    bool set_status;
    if (column < 0 || column >= m_itemData.size())
    {
        set_status = false;
    }
    else
    {
        m_itemData[column] = value;
        set_status = true;
    }

    return(set_status);
}

const QString RegMapTreeItem::getKindString(void)
{
    QString kind;
    switch(m_kind)
    {
        case RegMapTreeItem::e_rmmKind::root: kind = QString("root"); break;
        case RegMapTreeItem::e_rmmKind::map : kind = QString("map"); break;
        case RegMapTreeItem::e_rmmKind::mem : kind = QString("mem"); break;
        case RegMapTreeItem::e_rmmKind::blk : kind = QString("blk"); break;
        case RegMapTreeItem::e_rmmKind::reg : kind = QString("reg"); break;
        case RegMapTreeItem::e_rmmKind::fld : kind = QString("fld"); break;
        default: kind = QString(""); break;
    }
    return (kind);
}

const QString RegMapTreeItem::get_icon(void)
{
    QString icon;
    switch(m_kind)
    {
        case RegMapTreeItem::e_rmmKind::map : icon = QString(":/icons/uvm_map_small.png"); break;
        case RegMapTreeItem::e_rmmKind::mem : icon = QString(":/icons/uvm_mem_small.png"); break;
        case RegMapTreeItem::e_rmmKind::blk : icon = QString(":/icons/uvm_reg_block_small.png"); break;
        case RegMapTreeItem::e_rmmKind::reg : icon = QString(":/icons/uvm_reg_small.png"); break;
        case RegMapTreeItem::e_rmmKind::fld : icon = QString(":/icons/uvm_reg_field_small.png"); break;
        default: icon = QString(""); break;
    }
    return (icon);
}

QVector<RegMapTreeItem::e_rmmKind> RegMapTreeItem::get_possible_children(void)
{
    QVector<RegMapTreeItem::e_rmmKind> possible_children;
    switch(m_kind)
    {
        case RegMapTreeItem::e_rmmKind::root: possible_children = { RegMapTreeItem::e_rmmKind::blk}; break;
        case RegMapTreeItem::e_rmmKind::blk : possible_children = { RegMapTreeItem::e_rmmKind::mem, RegMapTreeItem::e_rmmKind::map, RegMapTreeItem::e_rmmKind::reg, RegMapTreeItem::e_rmmKind::blk}; break;
        case RegMapTreeItem::e_rmmKind::reg : possible_children = { RegMapTreeItem::e_rmmKind::fld}; break;
        default: break;
    }
    return (possible_children);
}

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
