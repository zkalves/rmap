/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "RegMapTreeItem.hpp"

RegMapTreeItem::RegMapTreeItem(RegMapTreeItem::e_rmmKind kind, QVariantMap &data, RegMapTreeItem *parentItem)
    : m_kind(kind), m_itemData(data), m_parentItem(parentItem)
{}

RegMapTreeItem::RegMapTreeItem(void)
    : m_kind(RegMapTreeItem::e_rmmKind::root), m_parentItem(nullptr)
{}

RegMapTreeItem::~RegMapTreeItem()
{
    for (RegMapTreeItem *child : m_childItems) {
        delete child;
    }
    m_childItems.clear();
}

void RegMapTreeItem::serialize( QVariantMap& data, SerializationContext* context ) const
{
    QList<QVariant> childItemsList;

    data[ "kind" ] = QVariant::fromValue(m_kind);
    data[ "id" ] = context->serialize<RegMapTreeItem>( const_cast <RegMapTreeItem*> (this) );
    data[ "parent" ] = context->serialize<RegMapTreeItem>( m_parentItem );
    for (RegMapTreeItem* child : m_childItems)
    {
        childItemsList.append(context->serialize<RegMapTreeItem>( child ));
    }
    data[ "childItems" ] = QVariant(childItemsList);
    data[ "itemData" ] = QVariant(m_itemData);
}

void RegMapTreeItem::deserialize( const QVariantMap& data, SerializationContext* context )
{
    QVector<RegMapTreeItem*> childItems;

    m_kind = data["kind"].value<RegMapTreeItem::e_rmmKind>();
    m_parentItem = context->deserialize<RegMapTreeItem>(data[ "parent" ]);
    QList<QVariant> childItemsList = data[ "childItems" ].toList();
    for (const QVariant &child : childItemsList)
    {
        childItems.append(context->deserialize<RegMapTreeItem>(child));
    }
    m_childItems = childItems;
    m_itemData = data[ "itemData" ].toMap();
}

RegMapTreeItem *RegMapTreeItem::child(int row) const
{
    if (row < 0 || row >= m_childItems.size())
        return nullptr;
    return m_childItems.at(row);
}

int RegMapTreeItem::childCount() const
{
    return m_childItems.size();
}

int RegMapTreeItem::columnCount() const
{
    return m_itemData.size();
}

QVariant RegMapTreeItem::data(const QString &column) const
{
    QVariant ret_val;
    if (m_itemData.contains(column))
    {
        ret_val = m_itemData[column];
    }
    else if ((column == "Offset" || column == "LSB") && m_itemData.contains("Offset/LSB"))
    {
        ret_val = m_itemData["Offset/LSB"];
    }
    else if (column == "Offset/LSB" && m_itemData.contains("Offset"))
    {
        ret_val = m_itemData["Offset"];
    }
    else if ((column == "Size" || column == "Width") && m_itemData.contains("Size/Width"))
    {
        ret_val = m_itemData["Size/Width"];
    }
    else if (column == "Size/Width" && m_itemData.contains("Size"))
    {
        ret_val = m_itemData["Size"];
    }
    else
    {
        ret_val = QVariant();
    }
    return ret_val;
}

RegMapTreeItem *RegMapTreeItem::parentItem() const
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

bool RegMapTreeItem::insertChildren(RegMapTreeItem::e_rmmKind kind, int position, int count, const QVector<QString> &displayColumns)
{
    bool insert_status;
    if((position < 0) || (position > this->m_childItems.size()))
    {
        insert_status = false;
    }
    else
    {
        for(int r = 0; r < count; r++)
        {
            QVariantMap dataMap;
            for (const QString &str : displayColumns)
            {
                dataMap[str] = "NA";
            }
            RegMapTreeItem *item = new RegMapTreeItem(kind, dataMap, this);
            this->m_childItems.insert(position, item);
        }
        insert_status = true;
    }

    return insert_status;
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
        for (int r = 0; r < count; r++)
        {
            m_childItems.remove(position);
        }
        status = true;
    }
    return status;
}

bool RegMapTreeItem::setData(const QString &column, const QVariant &value)
{
    m_itemData[column] = value;
    if (column == "Offset" || column == "LSB" || column == "Offset/LSB") {
        m_itemData["Offset"] = value;
        m_itemData["Offset/LSB"] = value;
    }
    if (column == "Size" || column == "Width" || column == "Size/Width") {
        m_itemData["Size"] = value;
        m_itemData["Size/Width"] = value;
    }
    return true;
}

QString RegMapTreeItem::kindString() const
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
    return kind;
}

QString RegMapTreeItem::icon() const
{
    QString iconPath;
    switch(m_kind)
    {
        case RegMapTreeItem::e_rmmKind::map : iconPath = QString(":/icons/uvm_map_small.png"); break;
        case RegMapTreeItem::e_rmmKind::mem : iconPath = QString(":/icons/uvm_mem_small.png"); break;
        case RegMapTreeItem::e_rmmKind::blk : iconPath = QString(":/icons/uvm_reg_block_small.png"); break;
        case RegMapTreeItem::e_rmmKind::reg : iconPath = QString(":/icons/uvm_reg_small.png"); break;
        case RegMapTreeItem::e_rmmKind::fld : iconPath = QString(":/icons/uvm_reg_field_small.png"); break;
        default: iconPath = QString(""); break;
    }
    return iconPath;
}

QVector<RegMapTreeItem::e_rmmKind> RegMapTreeItem::possibleChildren() const
{
    QVector<RegMapTreeItem::e_rmmKind> possible_children;
    switch(m_kind)
    {
        case RegMapTreeItem::e_rmmKind::root: possible_children = { RegMapTreeItem::e_rmmKind::blk, RegMapTreeItem::e_rmmKind::mem, RegMapTreeItem::e_rmmKind::map }; break;
        case RegMapTreeItem::e_rmmKind::blk : possible_children = { RegMapTreeItem::e_rmmKind::mem, RegMapTreeItem::e_rmmKind::map, RegMapTreeItem::e_rmmKind::reg, RegMapTreeItem::e_rmmKind::blk}; break;
        case RegMapTreeItem::e_rmmKind::reg : possible_children = { RegMapTreeItem::e_rmmKind::fld}; break;
        default: break;
    }
    return possible_children;
}
