#ifndef REGMAPTREEITEM_HPP
#define REGMAPTREEITEM_HPP

#include <QVariant>
#include <QVector>
#include "Serializable.hpp"
#include "SerializationContext.hpp"

class RegMapTreeItem : public QObject, public Serializable
{
    Q_OBJECT
public:
    enum class e_rmmKind { root, mem, map, blk, reg, fld};
    Q_ENUM(e_rmmKind)

    QVector<RegMapTreeItem*> getChildItems(void);

    RegMapTreeItem *child(int row);
    int childCount(void) const;
    int childNumber(void);
    int columnCount(void) const;
    QVariant data(int column) const;
    int row() const;
    RegMapTreeItem *parentItem();

    explicit RegMapTreeItem(e_rmmKind kind, QVector<QString> &displayColumns, QVariantMap &data, RegMapTreeItem *parentItem = nullptr);
    explicit RegMapTreeItem();
    ~RegMapTreeItem();
    void appendChild(RegMapTreeItem *child);
    bool insertChildren(e_rmmKind kind, int position, int count, int columns);
    bool removeChildren(int position, int count);
    bool setData(int column, QVariant value);
    QVector<e_rmmKind> get_possible_children(void);
    const QString get_icon(void);
    const QString getKindString(void);
    void serialize( QVariantMap& data, SerializationContext* context ) const;
    void deserialize( const QVariantMap& data, SerializationContext* context );
private:
    e_rmmKind m_kind;
    QVector<QString> m_displayColumns;
    QVariantMap m_itemData;
    QVector<RegMapTreeItem*> m_childItems;
    RegMapTreeItem *m_parentItem;
};
#endif // REGMAPTREEITEM_HPP

