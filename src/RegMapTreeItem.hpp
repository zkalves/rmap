#ifndef REGMAPTREEITEM_HPP
#define REGMAPTREEITEM_HPP

#include <QVariant>
#include <QVector>

class RegMapTreeItem
{
public:
    enum class e_rmmKind { root, mem, map, blk, reg, fld};

    e_rmmKind m_kind;

    QVector<RegMapTreeItem*> getChildItems(void);

    RegMapTreeItem *child(int row);
    int childCount() const;
    int childNumber();
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    RegMapTreeItem *parentItem();

    explicit RegMapTreeItem(e_rmmKind kind, QVector<QVariant> &data, RegMapTreeItem *parentItem = nullptr);
    ~RegMapTreeItem();
    void appendChild(RegMapTreeItem *child);
    bool insertChildren(e_rmmKind kind, int position, int count, int columns);
    bool insertColumns(int position, int columns);
    bool removeChildren(int position, int count);
    bool removeColumns(int position, int columns);
    bool setData(int column, QVariant value);
    QVector<e_rmmKind> get_possible_children(void);
    const QString get_icon(void);
    const QString getKindString(void);
private:
    QVector<RegMapTreeItem*> m_childItems;
    QVector<QVariant> m_itemData;
    RegMapTreeItem *m_parentItem;
};
#endif // REGMAPTREEITEM_HPP

