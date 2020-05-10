#ifndef REGMAPTREEITEM_HPP
#define REGMAPTREEITEM_HPP

#include <QVariant>
#include <QVector>

class RegMapTreeItem
{
public:
    explicit RegMapTreeItem(QVector<QVariant> &data, RegMapTreeItem *parentItem = nullptr);
    ~RegMapTreeItem();

    void appendChild(RegMapTreeItem *child);

    RegMapTreeItem *child(int row);
    int childCount() const;
    int childNumber();
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    RegMapTreeItem *parentItem();

private:
    QVector<RegMapTreeItem*> m_childItems;
    QVector<QVariant> m_itemData;
    RegMapTreeItem *m_parentItem;
};

#endif // REGMAPTREEITEM_HPP

