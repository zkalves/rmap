/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#ifndef REGMAPTREEMODEL_HPP
#define REGMAPTREEMODEL_HPP

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QPixmap>
#include <QVariant>
#include <QStringList>
#include <QBrush>
#include <QColor>
#include <QUuid>
#include <QSet>
#include <utility>
#include <iostream>
#include <string>
#include "RegMapTreeItem.hpp"
#include <nlohmann/json.hpp>
#include <cstdint>
using json = nlohmann::json;

class RegMapTreeModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(RegMapTreeModel)

public:
    explicit RegMapTreeModel(QObject *parent = nullptr);
    ~RegMapTreeModel() override;

    RegMapTreeItem* getRootItem() const { return m_rootItem; }
    RegMapTreeItem* rootItem() const { return m_rootItem; }
    void setRootItem(RegMapTreeItem* item);

    RegMapTreeItem* getItem(const QModelIndex &index) const;
    RegMapTreeItem* item(const QModelIndex &index) const { return getItem(index); }

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    bool insertRows(int position, int rows, RegMapTreeItem::e_rmmKind kind, QModelIndex parent = QModelIndex());
    void initRow(int row, QModelIndex index);
    bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex()) override;
    bool clear();
    void recursiveCheckData(RegMapTreeItem *node, uint32_t regWidth, QStringList &errors);
    QStringList checkData(uint32_t regWidth = 32);

    bool isIndexInvalid(const QModelIndex &index) const;
    json recursiveExtractJsonData(RegMapTreeItem *node, uint32_t regWidth = 32);
    json extractJsonData(uint32_t regWidth = 32);

    static uint32_t calculateCrc32(const uint8_t *data, size_t length, uint32_t previousCrc32 = 0);
    static uint32_t computeBlockCrc32(const json &blkJson);
    static uint32_t computeTreeCrc32(const json &rootJson);

private:
    RegMapTreeItem *m_rootItem{nullptr};
    QVector<QString> m_displayColumns;
    QSet<std::pair<RegMapTreeItem*, int>> m_invalidCells;
};

#endif // REGMAPTREEMODEL_HPP
