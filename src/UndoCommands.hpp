#ifndef UNDOCOMMANDS_HPP
#define UNDOCOMMANDS_HPP

#include <QUndoCommand>
#include <QModelIndex>
#include <QPersistentModelIndex>
#include <QVariant>
#include <QString>
#include <QMap>
#include <QVector>
#include "RegMapTreeModel.hpp"
#include "RegMapTreeItem.hpp"

class EditCellCommand : public QUndoCommand
{
public:
    EditCellCommand(RegMapTreeModel *model, const QModelIndex &index, const QVariant &oldVal, const QVariant &newVal, QUndoCommand *parent = nullptr)
        : QUndoCommand(parent), m_model(model), m_index(index), m_oldVal(oldVal), m_newVal(newVal)
    {
        setText(QObject::tr("Edit %1").arg(model->headerData(index.column(), Qt::Horizontal).toString()));
    }

    void undo() override {
        if (m_index.isValid()) {
            m_model->setData(m_index, m_oldVal, Qt::EditRole);
        }
    }

    void redo() override {
        if (m_index.isValid()) {
            m_model->setData(m_index, m_newVal, Qt::EditRole);
        }
    }

private:
    RegMapTreeModel *m_model;
    QPersistentModelIndex m_index;
    QVariant m_oldVal;
    QVariant m_newVal;
};

class InsertItemCommand : public QUndoCommand
{
public:
    InsertItemCommand(RegMapTreeModel *model, RegMapTreeItem::e_rmmKind kind, int row, const QModelIndex &parentIndex, QUndoCommand *parent = nullptr)
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

    void undo() override {
        m_model->removeRows(m_row, 1, m_parentIndex);
    }

    void redo() override {
        m_model->insertRows(m_row, 1, m_kind, m_parentIndex);
    }

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

    DeleteItemCommand(RegMapTreeModel *model, int row, const QModelIndex &parentIndex, QUndoCommand *parent = nullptr)
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

    static void captureItem(RegMapTreeItem *item, StoredNode &node) {
        if (!item) return;
        node.kind = item->kind();
        const QStringList cols = {"Type", "Offset/LSB", "Size/Width", "Name", "Access Policy", "HW Access", "Reset Value", "Is Rand", "Volatile", "Has Reset", "Description"};
        for (const QString &col : cols) {
            node.colData[col] = item->data(col);
        }
        for (RegMapTreeItem *c : item->getChildItems()) {
            StoredNode childNode;
            captureItem(c, childNode);
            node.children.push_back(childNode);
        }
    }

    static void restoreItem(RegMapTreeModel *model, const QModelIndex &index, const StoredNode &node) {
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

    void undo() override {
        m_model->insertRows(m_row, 1, m_kind, m_parentIndex);
        QModelIndex restoredIdx = m_model->index(m_row, 0, m_parentIndex);
        restoreItem(m_model, restoredIdx, m_storedData);
    }

    void redo() override {
        m_model->removeRows(m_row, 1, m_parentIndex);
    }

private:
    RegMapTreeModel *m_model;
    int m_row;
    QPersistentModelIndex m_parentIndex;
    RegMapTreeItem::e_rmmKind m_kind;
    QString m_name;
    StoredNode m_storedData;
};

#endif // UNDOCOMMANDS_HPP
