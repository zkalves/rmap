#include "RegMapDelegate.hpp"

RegMapDelegate::RegMapDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
    m_regex = new QRegularExpression(".*");
}

QWidget * RegMapDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    (void)option;
    (void)index;
    QLineEdit * lineEdit  = new QLineEdit(parent);
    QValidator *validator = new QRegularExpressionValidator(*m_regex);
    lineEdit->setValidator(validator);
    return(lineEdit);
}
void RegMapDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setPen(QPen(Qt::NoPen));
    QValidator *validator = new QRegularExpressionValidator(*m_regex);
    QString value = index.data(Qt::DisplayRole).toString();
    int pos = 0;
    QValidator::State isValid = validator->validate(value,pos);
    if (isValid != QValidator::Acceptable)
    {
        painter->setBrush(QBrush(Qt::red));
    }
    else
    {
        painter->setBrush(QBrush(Qt::transparent));
    }
    painter->drawRect(option.rect);
    // set text color
    painter->setPen(QPen(Qt::black));
    painter->drawText(option.rect, Qt::AlignLeft, value);
    painter->drawLine(QLine(option.rect.topLeft(), option.rect.bottomLeft()));
    painter->restore();
}

RegHexDecBinDelegate::RegHexDecBinDelegate(QObject *parent) :
    RegMapDelegate(parent)
{
    m_regex = new QRegularExpression("(0[xX][0-9a-fA-F]+)|([0-9]+)|(0b[01]+)");
}

RegIntDelegate::RegIntDelegate(QObject *parent) :
    RegMapDelegate(parent)
{
    m_regex = new QRegularExpression("[0-9a]+");
}

RegStrDelegate::RegStrDelegate(QObject *parent) :
    RegMapDelegate(parent)
{
    m_regex = new QRegularExpression("[a-zA-Z_][0-9a-zA-Z_$]*");
}
