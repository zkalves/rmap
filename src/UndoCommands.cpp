/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "UndoCommands.hpp"

// ============================================================================
// EditCellCommand
// ============================================================================

EditCellCommand::EditCellCommand(RegMapTreeModel *model, const QModelIndex &index, const QVariant &oldVal, const QVariant &newVal, QUndoCommand *parent)
    : QUndoCommand(parent), m_model(model), m_index(index), m_oldVal(oldVal), m_newVal(newVal)
{
    setText(QObject::tr("Edit %1").arg(model->headerData(index.column(), Qt::Horizontal).toString()));
}

void EditCellCommand::undo()
{
    if (m_index.isValid()) {
        m_model->setData(m_index, m_oldVal, Qt::EditRole);
    }
}

void EditCellCommand::redo()
{
    if (m_index.isValid()) {
        m_model->setData(m_index, m_newVal, Qt::EditRole);
    }
}

// ============================================================================
// InsertItemCommand
// ============================================================================

InsertItemCommand::InsertItemCommand(RegMapTreeModel *model, RegMapTreeItem::e_rmmKind kind, int row, const QModelIndex &parentIndex, QUndoCommand *parent)
    : QUndoCommand(parent), m_model(model), m_kind(kind), m_row(row), m_parentIndex(parentIndex)
{
    QString kindName;
    switch (kind) {
        case RegMapTreeItem::e_rmmKind::blk: kindName = "Block"; break;
        case RegMapTreeItem::e_rmmKind::reg: kindName = "Register"; break;
        case RegMapTreeItem::e_rmmKind::fld: kindName = "Field"; break;
        case RegMapTreeItem::e_rmmKind::mem: kindName = "Memory"; break;
        case RegMapTreeItem::e_rmmKind::map: kindName = "Map"; break;
        default: kindName = "Item"; break;
    }
    setText(QObject::tr("Add %1").arg(kindName));
}

void InsertItemCommand::undo()
{
    m_model->removeRows(m_row, 1, m_parentIndex);
}

void InsertItemCommand::redo()
{
    m_model->insertRows(m_row, 1, m_kind, m_parentIndex);
}

// ============================================================================
// DeleteItemCommand
// ============================================================================

DeleteItemCommand::DeleteItemCommand(RegMapTreeModel *model, int row, const QModelIndex &parentIndex, QUndoCommand *parent)
    : QUndoCommand(parent), m_model(model), m_row(row), m_parentIndex(parentIndex), m_kind(RegMapTreeItem::e_rmmKind::root)
{
    QModelIndex target = m_model->index(row, 0, parentIndex);
    RegMapTreeItem *item = m_model->getItem(target);
    if (item) {
        m_kind = item->kind();
        m_name = item->data("Name").toString();
        captureItem(item, m_storedData);
    }
    setText(QObject::tr("Delete %1").arg(m_name.isEmpty() ? "Item" : m_name));
}

void DeleteItemCommand::captureItem(RegMapTreeItem *item, StoredNode &node)
{
    if (!item) return;
    node.kind = item->kind();
    const QStringList cols = {"Type", "Offset/LSB", "Size/Width", "Name", "SW Access", "HW Access", "Reset Value", "Is Rand", "Volatile", "Has Reset", "Description"};
    for (const QString &col : cols) {
        node.colData[col] = item->data(col);
    }
    for (RegMapTreeItem *c : item->getChildItems()) {
        StoredNode childNode;
        captureItem(c, childNode);
        node.children.push_back(childNode);
    }
}

void DeleteItemCommand::restoreItem(RegMapTreeModel *model, const QModelIndex &index, const StoredNode &node)
{
    RegMapTreeItem *item = model->getItem(index);
    if (!item) return;
    for (auto it = node.colData.cbegin(); it != node.colData.cend(); ++it) {
        item->setData(it.key(), it.value());
    }
    for (int i = 0; i < node.children.size(); ++i) {
        model->insertRows(i, 1, node.children[i].kind, index);
        QModelIndex childIdx = model->index(i, 0, index);
        restoreItem(model, childIdx, node.children[i]);
    }
}

void DeleteItemCommand::undo()
{
    m_model->insertRows(m_row, 1, m_kind, m_parentIndex);
    QModelIndex restoredIdx = m_model->index(m_row, 0, m_parentIndex);
    restoreItem(m_model, restoredIdx, m_storedData);
}

void DeleteItemCommand::redo()
{
    m_model->removeRows(m_row, 1, m_parentIndex);
}

// ============================================================================
// DuplicateItemCommand
// ============================================================================

DuplicateItemCommand::DuplicateItemCommand(RegMapTreeModel *model, int row, const QModelIndex &parentIndex, const DeleteItemCommand::StoredNode &data, QUndoCommand *parent)
    : QUndoCommand(parent), m_model(model), m_row(row), m_parentIndex(parentIndex), m_storedData(data)
{
    QString name = m_storedData.colData.value("Name").toString();
    setText(QObject::tr("Duplicate %1").arg(name.isEmpty() ? "Item" : name));
}

void DuplicateItemCommand::undo()
{
    m_model->removeRows(m_row, 1, m_parentIndex);
}

void DuplicateItemCommand::redo()
{
    m_model->insertRows(m_row, 1, m_storedData.kind, m_parentIndex);
    QModelIndex restoredIdx = m_model->index(m_row, 0, m_parentIndex);
    DeleteItemCommand::restoreItem(m_model, restoredIdx, m_storedData);
}
