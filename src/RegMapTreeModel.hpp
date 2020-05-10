#ifndef REGMAPTREEMODEL_HPP
#define REGMAPTREEMODEL_HPP

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include "RegMapTreeItem.hpp"


class RegMapTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit RegMapTreeModel(QObject *parent = nullptr);
    ~RegMapTreeModel();

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    void setupModelData(const QStringList &lines, RegMapTreeItem *parent);

    RegMapTreeItem *m_rootItem;
};
//! [0]

#endif // REGMAPTREEMODEL_HPP
