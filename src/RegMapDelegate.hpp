#ifndef REGMAPDELEGATE_HPP
#define REGMAPDELEGATE_HPP
#include <QtWidgets>
#include <QRegularExpressionValidator>
#include <QDebug>
#include <QtCore>
#include <QtGui>

namespace Ui {
class RegMapDelegate;
}

class RegMapDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit  RegMapDelegate(QObject *parent = nullptr);
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void      paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
protected:
    QRegularExpression * m_regex;
};

class RegHexDecBinDelegate : public RegMapDelegate
{
    Q_OBJECT

public:
    explicit  RegHexDecBinDelegate(QObject *parent = nullptr);
};

class RegIntDelegate : public RegMapDelegate
{
    Q_OBJECT

public:
    explicit  RegIntDelegate(QObject *parent = nullptr);
};

class RegStrDelegate : public RegMapDelegate
{
    Q_OBJECT

public:
    explicit  RegStrDelegate(QObject *parent = nullptr);
};

#endif
