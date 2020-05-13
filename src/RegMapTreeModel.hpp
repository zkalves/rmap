#ifndef REGMAPTREEMODEL_HPP
#define REGMAPTREEMODEL_HPP

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QDebug>
#include <QPixmap>
#include <QVariant>
#include <iostream>
#include <string>
#include "RegMapTreeItem.hpp"


class RegMapTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit RegMapTreeModel(QObject *parent = nullptr);
    ~RegMapTreeModel();

    RegMapTreeItem* getRootItem(void);
    RegMapTreeItem* getItem(QModelIndex index);
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    bool insertRows(int position, int rows, RegMapTreeItem::e_rmmKind kind, QModelIndex parent=QModelIndex());
    void initRow(int row,QModelIndex index);
    bool removeRows(int position, int rows, const QModelIndex &parent=QModelIndex());

private:
    RegMapTreeItem *m_rootItem;

};

#endif // REGMAPTREEMODEL_HPP
