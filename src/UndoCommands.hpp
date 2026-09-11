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

#ifndef UNDOCOMMANDS_HPP
#define UNDOCOMMANDS_HPP

#include <QUndoCommand>
#include <QModelIndex>
#include <QPersistentModelIndex>
#include <QVariant>
#include <QString>
#include <QMap>
#include <QVector>
#include <QDebug>
#include "RegMapTreeModel.hpp"
#include "RegMapTreeItem.hpp"

class EditCellCommand : public QUndoCommand
{
public:
    EditCellCommand(RegMapTreeModel *model, const QModelIndex &index, const QVariant &oldVal, const QVariant &newVal, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    RegMapTreeModel *m_model;
    QPersistentModelIndex m_index;
    QVariant m_oldVal;
    QVariant m_newVal;
};

class InsertItemCommand : public QUndoCommand
{
public:
    InsertItemCommand(RegMapTreeModel *model, RegMapTreeItem::e_rmmKind kind, int row, const QModelIndex &parentIndex, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    RegMapTreeModel *m_model;
    RegMapTreeItem::e_rmmKind m_kind;
    int m_row;
    QPersistentModelIndex m_parentIndex;
};

class DeleteItemCommand : public QUndoCommand
{
public:
    struct StoredNode {
        RegMapTreeItem::e_rmmKind kind = RegMapTreeItem::e_rmmKind::root;
        QVariantMap colData;
        QVector<StoredNode> children;
    };

    DeleteItemCommand(RegMapTreeModel *model, int row, const QModelIndex &parentIndex, QUndoCommand *parent = nullptr);
    static void captureItem(RegMapTreeItem *item, StoredNode &node);
    static void restoreItem(RegMapTreeModel *model, const QModelIndex &index, const StoredNode &node);
    void undo() override;
    void redo() override;

private:
    RegMapTreeModel *m_model;
    int m_row;
    QPersistentModelIndex m_parentIndex;
    RegMapTreeItem::e_rmmKind m_kind;
    QString m_name;
    StoredNode m_storedData;
};

class DuplicateItemCommand : public QUndoCommand
{
public:
    DuplicateItemCommand(RegMapTreeModel *model, int row, const QModelIndex &parentIndex, const DeleteItemCommand::StoredNode &data, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    RegMapTreeModel *m_model;
    int m_row;
    QPersistentModelIndex m_parentIndex;
    DeleteItemCommand::StoredNode m_storedData;
};

#endif // UNDOCOMMANDS_HPP
