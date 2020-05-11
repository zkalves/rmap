#ifndef REGMAPTREEITEM_HPP
#define REGMAPTREEITEM_HPP

#include <QVariant>
#include <QVector>

class RegMapTreeItem
{
public:

    enum class e_rmitKind { root, mem, map, blk, reg, fld};

    RegMapTreeItem *child(int row);
    int childCount() const;
    int childNumber();
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    RegMapTreeItem *parentItem();

    explicit RegMapTreeItem(QVector<QVariant> &data, RegMapTreeItem *parentItem = nullptr);
    ~RegMapTreeItem();
    void appendChild(RegMapTreeItem *child);
    bool insertChildren(int position, int count, int columns, e_rmitKind kind);
    bool insertColumns(int position, int columns);
    bool removeChildren(int position, int count);
    bool removeColumns(int position, int columns);
    bool setData(int column, QVariant value);
private:
    QVector<RegMapTreeItem*> m_childItems;
    QVector<QVariant> m_itemData;
    RegMapTreeItem *m_parentItem;
};

#endif // REGMAPTREEITEM_HPP

