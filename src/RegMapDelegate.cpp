/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "RegMapDelegate.hpp"
#include "RegLockDialog.hpp"
#include "RegMapTreeItem.hpp"
#include "RegMapTreeModel.hpp"
#include <QAbstractProxyModel>

RegMapDelegate::RegMapDelegate(QObject *parent)
    : QStyledItemDelegate(parent), m_regex(QStringLiteral(".*")) {}

RegMapDelegate::RegMapDelegate(const QRegularExpression &regex, QObject *parent)
    : QStyledItemDelegate(parent), m_regex(regex) {}

QWidget *RegMapDelegate::createEditor(QWidget *parent,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const {
  Q_UNUSED(option);
  Q_UNUSED(index);
  QLineEdit *lineEdit = new QLineEdit(parent);
  QValidator *validator = new QRegularExpressionValidator(m_regex, lineEdit);
  lineEdit->setValidator(validator);
  return lineEdit;
}

void RegMapDelegate::paint(QPainter *painter,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index) const {
  // 1. Regex validation check
  QString value = index.data(Qt::DisplayRole).toString();
  bool isRegexValid = true;
  if (m_regex.isValid() && !value.isEmpty() && value != "NA") {
    isRegexValid = m_regex.match(value).hasMatch();
  }

  // 2. Model-level range/overlap validation check (unwrap proxy model if
  // present)
  QModelIndex sourceIndex = index;
  const RegMapTreeModel *model =
      qobject_cast<const RegMapTreeModel *>(index.model());
  if (!model) {
    const QAbstractProxyModel *proxy =
        qobject_cast<const QAbstractProxyModel *>(index.model());
    if (proxy) {
      sourceIndex = proxy->mapToSource(index);
      model = qobject_cast<const RegMapTreeModel *>(proxy->sourceModel());
    }
  }
  bool isModelInvalid = model && model->isIndexInvalid(sourceIndex);

  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  if (!isRegexValid || isModelInvalid) {
    opt.backgroundBrush =
        QBrush(ThemeManager::instance().currentTheme().errorBg);
  }

  QStyledItemDelegate::paint(painter, opt, index);
}

RegHexDecBinDelegate::RegHexDecBinDelegate(QObject *parent)
    : RegMapDelegate(QRegularExpression(QStringLiteral(
                         "(0[xX][0-9a-fA-F]+)|([0-9]+)|(0b[01]+)")),
                     parent) {}

RegIntDelegate::RegIntDelegate(QObject *parent)
    : RegMapDelegate(QRegularExpression(QStringLiteral("[0-9]+")), parent) {}

RegStrDelegate::RegStrDelegate(QObject *parent)
    : RegMapDelegate(
          QRegularExpression(QStringLiteral("[a-zA-Z_][0-9a-zA-Z_$]*")),
          parent) {}

AccessColors getAccessPolicyColors(const QString &access, bool colorBlind) {
  return ThemeManager::instance().getAccessColors(access, colorBlind);
}

AccessColors getAccessPolicyColors(const QString &access, ColorBlindMode mode) {
  return ThemeManager::instance().getAccessColors(access, mode);
}

// Access Policy Combobox Delegate (Software Access: RW, RO, WO, W1C, etc.)
RegAccessPolicyDelegate::RegAccessPolicyDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {}

void RegAccessPolicyDelegate::paint(QPainter *painter,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const {
  QString access = index.data(Qt::DisplayRole).toString().trimmed().toUpper();
  if (access.isEmpty() || access == "NA") {
    QStyledItemDelegate::paint(painter, option, index);
    return;
  }

  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  ColorBlindMode cbMode = ThemeManager::instance().colorBlindMode();
  if (cbMode == ColorBlindMode::None && parent()) {
    QVariant prop = parent()->property("colorBlindMode");
    if (prop.isValid() && prop.toBool()) {
      cbMode = ColorBlindMode::Universal;
    }
  }
  AccessColors colors = getAccessPolicyColors(access, cbMode);

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
  QString displayStr =
      (cbMode != ColorBlindMode::None) ? QString("[%1]").arg(access) : access;
  painter->drawText(badgeRect, Qt::AlignCenter, displayStr);

  painter->restore();
}

QWidget *
RegAccessPolicyDelegate::createEditor(QWidget *parent,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const {
  Q_UNUSED(option);
  Q_UNUSED(index);
  QComboBox *comboBox = new QComboBox(parent);
  comboBox->addItems({"RW",    "RO",    "WO",    "W1",  "WO1", "W1C",
                      "W1S",   "W1T",   "W0C",   "W0S", "W0T", "RC",
                      "RS",    "WRC",   "WRS",   "WC",  "WS",  "W1SRC",
                      "W1CRS", "W0SRC", "W0CRS", "WOC", "WOS", "NOACCESS"});
  return comboBox;
}

void RegAccessPolicyDelegate::setEditorData(QWidget *editor,
                                            const QModelIndex &index) const {
  QString value = index.model()->data(index, Qt::EditRole).toString();
  QComboBox *comboBox = static_cast<QComboBox *>(editor);
  int idx = comboBox->findText(value);
  if (idx >= 0)
    comboBox->setCurrentIndex(idx);
}

void RegAccessPolicyDelegate::setModelData(QWidget *editor,
                                           QAbstractItemModel *model,
                                           const QModelIndex &index) const {
  QComboBox *comboBox = static_cast<QComboBox *>(editor);
  model->setData(index, comboBox->currentText(), Qt::EditRole);
}

bool RegAccessPolicyDelegate::editorEvent(QEvent *event,
                                          QAbstractItemModel *model,
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &index) {
  if (event->type() == QEvent::MouseButtonRelease) {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    if (mouseEvent->button() == Qt::LeftButton) {
      // Single-click cycle common SW policies: RW -> RO -> WO -> W1C -> RW
      QString current =
          model->data(index, Qt::DisplayRole).toString().trimmed().toUpper();
      QString next = "RW";
      if (current == "RW")
        next = "RO";
      else if (current == "RO")
        next = "WO";
      else if (current == "WO")
        next = "W1C";
      else if (current == "W1C")
        next = "RW";
      else
        next = "RW";
      model->setData(index, next, Qt::EditRole);
      return true;
    }
  }
  return QStyledItemDelegate::editorEvent(event, model, option, index);
}

// Hardware Access Policy Delegate (HW Access: RO, RW, WO, NA, W1C, etc.)
RegHwAccessDelegate::RegHwAccessDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {}

void RegHwAccessDelegate::paint(QPainter *painter,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const {
  QString hwAccess = index.data(Qt::DisplayRole).toString().trimmed().toUpper();
  if (hwAccess.isEmpty())
    hwAccess = "RO";

  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  ColorBlindMode cbMode = ThemeManager::instance().colorBlindMode();
  if (cbMode == ColorBlindMode::None && parent()) {
    QVariant prop = parent()->property("colorBlindMode");
    if (prop.isValid() && prop.toBool()) {
      cbMode = ColorBlindMode::Universal;
    }
  }
  AccessColors colors = getAccessPolicyColors(hwAccess, cbMode);

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
  QString displayStr = (cbMode != ColorBlindMode::None)
                           ? QString("[%1]").arg(hwAccess)
                           : hwAccess;
  painter->drawText(badgeRect, Qt::AlignCenter, displayStr);

  painter->restore();
}

QWidget *RegHwAccessDelegate::createEditor(QWidget *parent,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &index) const {
  Q_UNUSED(option);
  Q_UNUSED(index);
  QComboBox *comboBox = new QComboBox(parent);
  comboBox->addItems({"RO", "RW", "WO", "WIRE", "W1T", "INCR", "DECR", "NA",
                      "W1C", "W1S", "W0C", "RS", "RC"});
  return comboBox;
}

void RegHwAccessDelegate::setEditorData(QWidget *editor,
                                        const QModelIndex &index) const {
  QString value = index.model()->data(index, Qt::EditRole).toString();
  QComboBox *comboBox = static_cast<QComboBox *>(editor);
  int idx = comboBox->findText(value);
  if (idx >= 0)
    comboBox->setCurrentIndex(idx);
}

void RegHwAccessDelegate::setModelData(QWidget *editor,
                                       QAbstractItemModel *model,
                                       const QModelIndex &index) const {
  QComboBox *comboBox = static_cast<QComboBox *>(editor);
  model->setData(index, comboBox->currentText(), Qt::EditRole);
}

bool RegHwAccessDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) {
  if (event->type() == QEvent::MouseButtonRelease) {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    if (mouseEvent->button() == Qt::LeftButton) {
      // Single-click cycle common HW policies: RO -> RW -> WO -> WIRE -> W1T ->
      // INCR -> DECR -> NA -> RO
      QString current =
          model->data(index, Qt::DisplayRole).toString().trimmed().toUpper();
      QString next = "RO";
      if (current == "RO")
        next = "RW";
      else if (current == "RW")
        next = "WO";
      else if (current == "WO")
        next = "WIRE";
      else if (current == "WIRE")
        next = "W1T";
      else if (current == "W1T")
        next = "INCR";
      else if (current == "INCR")
        next = "DECR";
      else if (current == "DECR")
        next = "NA";
      else if (current == "NA")
        next = "RO";
      else
        next = "RO";
      model->setData(index, next, Qt::EditRole);
      return true;
    }
  }
  return QStyledItemDelegate::editorEvent(event, model, option, index);
}

// Boolean Checkbox/Dropdown Delegate
RegBoolDelegate::RegBoolDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {}

QWidget *RegBoolDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const {
  Q_UNUSED(option);
  Q_UNUSED(index);
  QComboBox *comboBox = new QComboBox(parent);
  comboBox->addItems({"true", "false"});
  return comboBox;
}

void RegBoolDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const {
  QString value = index.model()->data(index, Qt::EditRole).toString();
  QComboBox *comboBox = static_cast<QComboBox *>(editor);
  int idx = comboBox->findText(value, Qt::MatchFixedString);
  if (idx >= 0)
    comboBox->setCurrentIndex(idx);
}

void RegBoolDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const {
  QComboBox *comboBox = static_cast<QComboBox *>(editor);
  model->setData(index, comboBox->currentText(), Qt::EditRole);
}

bool RegBoolDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) {
  if (event->type() == QEvent::MouseButtonRelease) {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    if (mouseEvent->button() == Qt::LeftButton) {
      // Single-click toggle: true <-> false
      QString current =
          model->data(index, Qt::DisplayRole).toString().toLower();
      bool isTrue = (current == "true" || current == "1");
      model->setData(index, isTrue ? "false" : "true", Qt::EditRole);
      return true;
    }
  }
  return QStyledItemDelegate::editorEvent(event, model, option, index);
}

// RegLockDelegate Implementation
RegLockDelegate::RegLockDelegate(const RegMapTreeModel *model, QObject *parent)
    : QStyledItemDelegate(parent), m_model(model) {}

QWidget *RegLockDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const {
  Q_UNUSED(option);
  Q_UNUSED(index);
  QComboBox *comboBox = new QComboBox(parent);
  comboBox->setEditable(true);
  comboBox->addItem(tr("(None)"), "");

  // Collect unique existing locks from model
  QSet<QString> existingLocks;
  if (m_model && m_model->getRootItem()) {
    std::function<void(RegMapTreeItem *)> collectLocks =
        [&](RegMapTreeItem *node) {
          if (!node)
            return;
          QString lk = node->data("Lock").toString().trimmed();
          if (!lk.isEmpty())
            existingLocks.insert(lk);
          for (RegMapTreeItem *child : node->getChildItems()) {
            collectLocks(child);
          }
        };
    collectLocks(m_model->getRootItem());
  }

  for (const QString &lk : existingLocks) {
    comboBox->addItem(LockParser::formatBadgeText(lk), lk);
  }

  comboBox->insertSeparator(comboBox->count());
  comboBox->addItem(tr("⚙ Configure Lock..."), "__CONFIG_LOCK__");
  return comboBox;
}

void RegLockDelegate::paint(QPainter *painter,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index) const {
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  QString lockText =
      index.model()->data(index, Qt::DisplayRole).toString().trimmed();

  painter->save();
  painter->setRenderHint(QPainter::Antialiasing, true);

  if (opt.state & QStyle::State_Selected) {
    painter->fillRect(opt.rect, opt.palette.highlight());
  }

  if (lockText.isEmpty()) {
    QFont f = opt.font;
    f.setItalic(true);
    painter->setFont(f);
    painter->setPen(opt.palette.color(QPalette::Disabled, QPalette::Text));
    painter->drawText(opt.rect.adjusted(6, 0, -6, 0),
                      Qt::AlignVCenter | Qt::AlignLeft, tr("None"));
  } else {
    QRect badgeRect = opt.rect.adjusted(4, 3, -4, -3);

    QColor badgeBg(220, 130, 20, 220); // Warm amber
    QColor badgeBorder(240, 160, 40);
    QColor textColor(255, 255, 255);

    painter->setPen(QPen(badgeBorder, 1.2));
    painter->setBrush(QBrush(badgeBg));
    painter->drawRoundedRect(badgeRect, 4, 4);

    QFont f = opt.font;
    f.setBold(true);
    f.setPointSize(8);
    painter->setFont(f);
    painter->setPen(textColor);

    QString displayText = LockParser::formatBadgeText(lockText);
    painter->drawText(badgeRect.adjusted(6, 0, -6, 0),
                      Qt::AlignVCenter | Qt::AlignLeft, displayText);
  }

  painter->restore();
}

void RegLockDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const {
  QString value = index.model()->data(index, Qt::EditRole).toString().trimmed();
  QComboBox *comboBox = static_cast<QComboBox *>(editor);
  if (value.isEmpty()) {
    comboBox->setCurrentIndex(0);
  } else {
    int idx = comboBox->findData(value);
    if (idx >= 0) {
      comboBox->setCurrentIndex(idx);
    } else {
      comboBox->setCurrentText(value);
    }
  }
}

void RegLockDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const {
  QComboBox *comboBox = static_cast<QComboBox *>(editor);
  QString selData = comboBox->currentData().toString();
  QString selText = comboBox->currentText().trimmed();

  if (selData == "__CONFIG_LOCK__" || selText == tr("⚙ Configure Lock...")) {
    QModelIndex wrIdx = model->index(index.row(), 11, index.parent());
    QModelIndex rdIdx = model->index(index.row(), 12, index.parent());
    QString wrVal = wrIdx.isValid()
                        ? model->data(wrIdx, Qt::EditRole).toString().trimmed()
                        : QString();
    QString rdVal = rdIdx.isValid()
                        ? model->data(rdIdx, Qt::EditRole).toString().trimmed()
                        : QString();
    QString itemName =
        model->index(index.row(), 3, index.parent()).data().toString();
    QWidget *parentWidget = editor->parentWidget();
    RegLockDialog dlg(wrVal, rdVal, m_model, itemName, parentWidget);
    if (dlg.exec() == QDialog::Accepted) {
      if (wrIdx.isValid())
        model->setData(wrIdx, dlg.writeExpression(), Qt::EditRole);
      if (rdIdx.isValid())
        model->setData(rdIdx, dlg.readExpression(), Qt::EditRole);
    }
  } else if (selData.isEmpty() &&
             (selText == tr("(None)") || selText.isEmpty())) {
    model->setData(index, "", Qt::EditRole);
  } else if (!selData.isEmpty()) {
    model->setData(index, selData, Qt::EditRole);
  } else {
    if (selText.startsWith("🔒 ")) {
      selText = selText.mid(2).trimmed();
    }
    model->setData(index, selText, Qt::EditRole);
  }
}

bool RegLockDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) {
  if (event->type() == QEvent::MouseButtonDblClick) {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    if (mouseEvent->button() == Qt::LeftButton) {
      QModelIndex wrIdx = model->index(index.row(), 11, index.parent());
      QModelIndex rdIdx = model->index(index.row(), 12, index.parent());
      QString wrVal =
          wrIdx.isValid()
              ? model->data(wrIdx, Qt::EditRole).toString().trimmed()
              : QString();
      QString rdVal =
          rdIdx.isValid()
              ? model->data(rdIdx, Qt::EditRole).toString().trimmed()
              : QString();
      QString itemName =
          model->index(index.row(), 3, index.parent()).data().toString();
      QWidget *parentWidget = const_cast<QWidget *>(option.widget);
      RegLockDialog dlg(wrVal, rdVal, m_model, itemName, parentWidget);
      if (dlg.exec() == QDialog::Accepted) {
        if (wrIdx.isValid())
          model->setData(wrIdx, dlg.writeExpression(), Qt::EditRole);
        if (rdIdx.isValid())
          model->setData(rdIdx, dlg.readExpression(), Qt::EditRole);
      }
      return true;
    }
  }
  return QStyledItemDelegate::editorEvent(event, model, option, index);
}
