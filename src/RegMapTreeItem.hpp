/* This file is part of rmap.
 *
 * rmap is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * rmap is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rmap.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#ifndef REGMAPTREEITEM_HPP
#define REGMAPTREEITEM_HPP

#include <QObject>
#include <QVariant>
#include <QVector>
#include "Serializable.hpp"
#include "SerializationContext.hpp"

class RegMapTreeItem : public QObject, public Serializable
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(RegMapTreeItem)

public:
    enum class e_rmmKind { root, mem, map, blk, reg, fld, };

    explicit RegMapTreeItem(e_rmmKind kind, QVariantMap &data, RegMapTreeItem *parentItem = nullptr);
    explicit RegMapTreeItem();
    ~RegMapTreeItem() override;

    QVector<RegMapTreeItem*> getChildItems() const;
    QVector<RegMapTreeItem*> childItems() const;
    const QVector<RegMapTreeItem*>& childItemsRef() const;

    RegMapTreeItem *child(int row) const;
    int childCount() const;
    int columnCount() const;
    QVariant data(const QString &column) const;
    int row() const;
    RegMapTreeItem *parentItem() const;

    void appendChild(RegMapTreeItem *child);
    bool insertChildren(e_rmmKind kind, int position, int count, const QVector<QString> &displayColumns);
    bool removeChildren(int position, int count);
    bool setData(const QString &column, const QVariant &value);

    QVector<e_rmmKind> possibleChildren() const noexcept;
    QString icon() const noexcept;
    QString kindString() const noexcept;
    e_rmmKind kind() const noexcept;



    void serialize( QVariantMap& data, SerializationContext* context ) const override;
    void deserialize( const QVariantMap& data, SerializationContext* context ) override;

private:
    e_rmmKind m_kind = e_rmmKind::root;
    QVector<QString> m_displayColumns;
    QVariantMap m_itemData;
    QVector<RegMapTreeItem*> m_childItems;
    RegMapTreeItem *m_parentItem = nullptr;
};

#endif // REGMAPTREEITEM_HPP

