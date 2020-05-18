#ifndef REGMAPTREEITEM_HPP
#define REGMAPTREEITEM_HPP

#include <QMap>
#include <QVariant>
#include <QVector>

class RegMapTreeItem
{
public:
    enum class e_rmmKind { root, mem, map, blk, reg, fld};


    QVector<RegMapTreeItem*> getChildItems(void);

    RegMapTreeItem *child(int row);
    int childCount() const;
    int childNumber();
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    RegMapTreeItem *parentItem();

    explicit RegMapTreeItem(e_rmmKind kind, QVector<QVariant> &displayColumns, QMap<QVariant,QVariant> &data, RegMapTreeItem *parentItem = nullptr);
    ~RegMapTreeItem();
    void appendChild(RegMapTreeItem *child);
    bool insertChildren(e_rmmKind kind, int position, int count, int columns);
    bool removeChildren(int position, int count);
    bool setData(int column, QVariant value);
    QVector<e_rmmKind> get_possible_children(void);
    const QString get_icon(void);
    const QString getKindString(void);
private:
    e_rmmKind m_kind;
    QVector<QVariant> m_displayColumns;
    QMap<QVariant,QVariant> m_itemData;
    QVector<RegMapTreeItem*> m_childItems;
    RegMapTreeItem *m_parentItem;
};
#endif // REGMAPTREEITEM_HPP

