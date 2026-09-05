/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
    Q_ENUM(e_rmmKind)

    explicit RegMapTreeItem(e_rmmKind kind, QVariantMap &data, RegMapTreeItem *parentItem = nullptr);
    explicit RegMapTreeItem();
    ~RegMapTreeItem() override;

    QVector<RegMapTreeItem*> getChildItems() const { return m_childItems; }
    QVector<RegMapTreeItem*> childItems() const { return m_childItems; }
    const QVector<RegMapTreeItem*>& childItemsRef() const { return m_childItems; }

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

    QVector<e_rmmKind> possibleChildren() const;
    QString icon() const;
    QString kindString() const;
    e_rmmKind kind() const { return m_kind; }



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

