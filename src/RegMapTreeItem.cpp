#include "RegMapTreeItem.hpp"

RegMapTreeItem::RegMapTreeItem(RegMapTreeItem::e_rmmKind kind, QVector<QString> &displayColumns, QMap<QString,QVariant> &data, RegMapTreeItem *parent)
    : m_kind(kind), m_displayColumns(displayColumns), m_itemData(data), m_parentItem(parent)
{}

RegMapTreeItem::RegMapTreeItem(void)
{}

RegMapTreeItem::~RegMapTreeItem()
{
    qDeleteAll(m_childItems);
}

void RegMapTreeItem::serialize( QVariantMap& data, SerializationContext* context ) const
{

    data[ "kind" ] = QVariant::fromValue(m_kind);
    //data[ "displayColumns" ] = QVariant::fromValue<QList<QString>>(m_displayColumns.toList());
    data[ "displayColumns" ] = QVariant(m_displayColumns.toList());
    data[ "itemData" ] == QVariant(m_itemData);
    //data[ "parent" ] << m_parentItem;
    //const QVector<RegMapTreeItem *> vec = m_childItems;
    //for ( Obj *i : list )
    //data[ "childItems" ] = context->serialize( m_childItems );
}

void RegMapTreeItem::deserialize( const QVariantMap& data, SerializationContext* context )
{
    m_kind = data["kind"].value<RegMapTreeItem::e_rmmKind>();
    m_displayColumns.fromList(data[ "displayColumns" ].toStringList());
    m_itemData = data[ "itemData" ].toMap() ;
    //data[ "parent" ] >> m_parentItem;
    //m_childItems = context->deserialize<RegMapTreeItem>( data[ "childItems" ] );
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
    return m_displayColumns.count();
}

QVariant RegMapTreeItem::data(int column) const
{
    if (column < 0 || column >= m_displayColumns.size())
        return QVariant();

    QString col = m_displayColumns[column];
    return m_itemData[col];
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
    (void) columns;
    bool insert_status;
    if((position < 0) || (position > this->m_childItems.size()))
    {
        insert_status = false;
    }
    else
    {
        for(int row=0 ; row < count ; row++)
        {
            QMap<QString,QVariant> data;
            QString str;
            foreach (str, m_displayColumns)
            {
                data[str]="NA";
            }
            RegMapTreeItem *item = new RegMapTreeItem(kind, m_displayColumns, data, this);
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

bool RegMapTreeItem::setData(int column, QVariant value)
{
    bool set_status;
    if (column < 0 || column >= m_displayColumns.size())
    {
        set_status = false;
    }
    else
    {
        QString col = m_displayColumns[column];
        m_itemData[col] = value;
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
