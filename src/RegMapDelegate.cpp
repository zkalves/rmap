#include "RegMapDelegate.hpp"
#include "RegMapTreeModel.hpp"
#include <QAbstractProxyModel>

RegMapDelegate::RegMapDelegate(QObject *parent)
    : QStyledItemDelegate(parent), m_regex(QStringLiteral(".*"))
{
}

RegMapDelegate::RegMapDelegate(const QRegularExpression &regex, QObject *parent)
    : QStyledItemDelegate(parent), m_regex(regex)
{
}

QWidget * RegMapDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    QLineEdit * lineEdit = new QLineEdit(parent);
    QValidator *validator = new QRegularExpressionValidator(m_regex, lineEdit);
    lineEdit->setValidator(validator);
    return lineEdit;
}

void RegMapDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // 1. Regex validation check
    QString value = index.data(Qt::DisplayRole).toString();
    bool isRegexValid = true;
    if (m_regex.isValid() && !value.isEmpty() && value != "NA") {
        isRegexValid = m_regex.match(value).hasMatch();
    }

    // 2. Model-level range/overlap validation check (unwrap proxy model if present)
    QModelIndex sourceIndex = index;
    const RegMapTreeModel* model = qobject_cast<const RegMapTreeModel*>(index.model());
    if (!model) {
        const QAbstractProxyModel* proxy = qobject_cast<const QAbstractProxyModel*>(index.model());
        if (proxy) {
            sourceIndex = proxy->mapToSource(index);
            model = qobject_cast<const RegMapTreeModel*>(proxy->sourceModel());
        }
    }
    bool isModelInvalid = model && model->isIndexInvalid(sourceIndex);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    if (!isRegexValid || isModelInvalid)
    {
        opt.backgroundBrush = QBrush(ThemeManager::instance().currentTheme().errorBg);
    }

    QStyledItemDelegate::paint(painter, opt, index);
}

RegHexDecBinDelegate::RegHexDecBinDelegate(QObject *parent)
    : RegMapDelegate(QRegularExpression(QStringLiteral("(0[xX][0-9a-fA-F]+)|([0-9]+)|(0b[01]+)")), parent)
{
}

RegIntDelegate::RegIntDelegate(QObject *parent)
    : RegMapDelegate(QRegularExpression(QStringLiteral("[0-9]+")), parent)
{
}

RegStrDelegate::RegStrDelegate(QObject *parent)
    : RegMapDelegate(QRegularExpression(QStringLiteral("[a-zA-Z_][0-9a-zA-Z_$]*")), parent)
{
}

AccessColors getAccessPolicyColors(const QString &access, bool colorBlind)
{
    return ThemeManager::instance().getAccessColors(access, colorBlind);
}

// Access Policy Combobox Delegate (Software Access: RW, RO, WO, W1C, etc.)
RegAccessPolicyDelegate::RegAccessPolicyDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

void RegAccessPolicyDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString access = index.data(Qt::DisplayRole).toString().trimmed().toUpper();
    if (access.isEmpty() || access == "NA") {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    bool colorBlind = parent() ? parent()->property("colorBlindMode").toBool() : false;
    AccessColors colors = getAccessPolicyColors(access, colorBlind);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    // Draw selection background if selected
    if (opt.state & QStyle::State_Selected) {
        painter->fillRect(opt.rect, opt.palette.highlight());
    }

    // Draw rounded badge in cell
    QRect badgeRect = opt.rect.adjusted(4, 3, -4, -3);
    if (badgeRect.width() > 64) {
        badgeRect.setWidth(64);
        badgeRect.moveCenter(opt.rect.center());
    }

    painter->setPen(QPen(colors.border, 1.2));
    painter->setBrush(QBrush(colors.bg));
    painter->drawRoundedRect(badgeRect, 4, 4);

    QFont f = opt.font;
    f.setBold(true);
    f.setPointSize(8);
    painter->setFont(f);
    painter->setPen(colors.text);
    QString displayStr = colorBlind ? QString("[%1]").arg(access) : access;
    painter->drawText(badgeRect, Qt::AlignCenter, displayStr);

    painter->restore();
}

QWidget *RegAccessPolicyDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    QComboBox *comboBox = new QComboBox(parent);
    comboBox->addItems({"RW", "RO", "WO", "W1C", "W1S", "W1T", "W0C", "W0S", "W0T", "RC", "RS", "WC", "WS"});
    return comboBox;
}

void RegAccessPolicyDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    int idx = comboBox->findText(value);
    if (idx >= 0) comboBox->setCurrentIndex(idx);
}

void RegAccessPolicyDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    model->setData(index, comboBox->currentText(), Qt::EditRole);
}

bool RegAccessPolicyDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            // Single-click cycle common SW policies: RW -> RO -> WO -> W1C -> RW
            QString current = model->data(index, Qt::DisplayRole).toString().trimmed().toUpper();
            QString next = "RW";
            if (current == "RW") next = "RO";
            else if (current == "RO") next = "WO";
            else if (current == "WO") next = "W1C";
            else if (current == "W1C") next = "RW";
            else next = "RW";
            model->setData(index, next, Qt::EditRole);
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

// Hardware Access Policy Delegate (HW Access: RO, RW, WO, NA, W1C, etc.)
RegHwAccessDelegate::RegHwAccessDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

void RegHwAccessDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString hwAccess = index.data(Qt::DisplayRole).toString().trimmed().toUpper();
    if (hwAccess.isEmpty()) hwAccess = "RO";

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    bool colorBlind = parent() ? parent()->property("colorBlindMode").toBool() : false;
    AccessColors colors = getAccessPolicyColors(hwAccess, colorBlind);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    if (opt.state & QStyle::State_Selected) {
        painter->fillRect(opt.rect, opt.palette.highlight());
    }

    QRect badgeRect = opt.rect.adjusted(4, 3, -4, -3);
    if (badgeRect.width() > 64) {
        badgeRect.setWidth(64);
        badgeRect.moveCenter(opt.rect.center());
    }

    painter->setPen(QPen(colors.border, 1.2, Qt::DashLine));
    painter->setBrush(QBrush(colors.bg.lighter(108)));
    painter->drawRoundedRect(badgeRect, 4, 4);

    QFont f = opt.font;
    f.setBold(true);
    f.setPointSize(8);
    painter->setFont(f);
    painter->setPen(colors.text);
    painter->drawText(badgeRect, Qt::AlignCenter, hwAccess);

    painter->restore();
}

QWidget *RegHwAccessDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    QComboBox *comboBox = new QComboBox(parent);
    comboBox->addItems({"RO", "RW", "WO", "NA", "W1C", "W1S", "W0C", "RS", "RC"});
    return comboBox;
}

void RegHwAccessDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    int idx = comboBox->findText(value);
    if (idx >= 0) comboBox->setCurrentIndex(idx);
}

void RegHwAccessDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    model->setData(index, comboBox->currentText(), Qt::EditRole);
}

bool RegHwAccessDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            // Single-click cycle common HW policies: RO -> RW -> WO -> NA -> RO
            QString current = model->data(index, Qt::DisplayRole).toString().trimmed().toUpper();
            QString next = "RO";
            if (current == "RO") next = "RW";
            else if (current == "RW") next = "WO";
            else if (current == "WO") next = "NA";
            else if (current == "NA") next = "RO";
            else next = "RO";
            model->setData(index, next, Qt::EditRole);
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

// Boolean Checkbox/Dropdown Delegate
RegBoolDelegate::RegBoolDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

QWidget *RegBoolDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    QComboBox *comboBox = new QComboBox(parent);
    comboBox->addItems({"true", "false"});
    return comboBox;
}

void RegBoolDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    int idx = comboBox->findText(value, Qt::MatchFixedString);
    if (idx >= 0) comboBox->setCurrentIndex(idx);
}

void RegBoolDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    model->setData(index, comboBox->currentText(), Qt::EditRole);
}

bool RegBoolDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            // Single-click toggle: true <-> false
            QString current = model->data(index, Qt::DisplayRole).toString().toLower();
            bool isTrue = (current == "true" || current == "1");
            model->setData(index, isTrue ? "false" : "true", Qt::EditRole);
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

