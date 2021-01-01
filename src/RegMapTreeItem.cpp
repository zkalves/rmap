#include "RegMapTreeItem.hpp"

RegMapTreeItem::RegMapTreeItem(RegMapTreeItem::e_rmmKind kind, QVariantMap &data, RegMapTreeItem *parentItem)
    : m_kind(kind), m_itemData(data), m_parentItem(parentItem)
{}

RegMapTreeItem::RegMapTreeItem(void)
{}

RegMapTreeItem::~RegMapTreeItem()
{
    qDeleteAll(m_childItems);
}

void RegMapTreeItem::serialize( QVariantMap& data, SerializationContext* context ) const
{
    QList<QVariant> childItemsList;
    RegMapTreeItem* child;

    data[ "kind" ] = QVariant::fromValue(m_kind);
    data[ "id" ] = context->serialize<RegMapTreeItem>( const_cast <RegMapTreeItem*> (this) );
    data[ "parent" ] = context->serialize<RegMapTreeItem>( m_parentItem );
    foreach (child, m_childItems)
    {
        childItemsList.append(context->serialize<RegMapTreeItem>( child ));
    }
    data[ "childItems" ] = QVariant(childItemsList);
    data[ "itemData" ] = QVariant(m_itemData);

}

void RegMapTreeItem::deserialize( const QVariantMap& data, SerializationContext* context )
{
    QVector<RegMapTreeItem*> childItems;
    QVariant child;

    m_kind = data["kind"].value<RegMapTreeItem::e_rmmKind>();
    m_parentItem = context->deserialize<RegMapTreeItem>(data[ "parent" ]);
    QList<QVariant> childItemsList = data[ "childItems" ].toList();
    foreach (child, childItemsList)
    {
        childItems.append(context->deserialize<RegMapTreeItem>(child));
    }
    m_childItems = childItems;
    m_itemData = data[ "itemData" ].toMap() ;
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

int RegMapTreeItem::columnCount() const
{
    return m_itemData.count();
}

QVariant RegMapTreeItem::data(QString column) const
{
    QVariant ret_val;
    if (m_itemData.contains(column))
    {
        ret_val = m_itemData[column];
    }
    else
    {
        ret_val = QVariant();
    }
    return(ret_val);
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


bool RegMapTreeItem::insertChildren(RegMapTreeItem::e_rmmKind kind, int position, int count, QVector<QString> displayColumns)
{
    bool insert_status;
    if((position < 0) || (position > this->m_childItems.size()))
    {
        insert_status = false;
    }
    else
    {
        for(int row=0 ; row < count ; row++)
        {
            QVariantMap data;
            QString str;
            foreach (str, displayColumns)
            {
                data[str]="NA";
            }
            RegMapTreeItem *item = new RegMapTreeItem(kind, data, this);
            this->m_childItems.insert(position, item);
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

bool RegMapTreeItem::setData(QString column, QVariant value)
{
    bool set_status;
    if (m_itemData.contains(column))
    {
        set_status          = true;
        m_itemData[column]  = value;
    } else {
        set_status = false;
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
