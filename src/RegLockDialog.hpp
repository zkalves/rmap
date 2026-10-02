/* This file is part of rmap.
 *
 * rmap is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * rmap is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rmap.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#ifndef REG_LOCK_DIALOG_HPP
#define REG_LOCK_DIALOG_HPP

#include "LockParser.hpp"
#include <QDialog>
#include <QString>
#include <QStringList>

class QComboBox;
class QLineEdit;
class QLabel;
class QPushButton;
class QTabWidget;
class RegMapTreeModel;

class RegLockDialog : public QDialog {
  Q_OBJECT

public:
  explicit RegLockDialog(const QString &writeExpr, const QString &readExpr,
                         const RegMapTreeModel *model = nullptr,
                         const QString &contextItemName = QString(),
                         QWidget *parent = nullptr);

  // Backward-compatible single expression constructor:
  explicit RegLockDialog(const QString &currentExpr,
                         const RegMapTreeModel *model = nullptr,
                         const QString &contextItemName = QString(),
                         QWidget *parent = nullptr);

  ~RegLockDialog() override;

  QString writeExpression() const;
  QString readExpression() const;

  // Backward-compatible getters:
  QString expression() const { return writeExpression(); }
  LockScope scope() const {
    bool hasWr = !writeExpression().isEmpty();
    bool hasRd = !readExpression().isEmpty();
    if (hasWr && hasRd)
      return LockScope::Both;
    if (hasRd)
      return LockScope::Read;
    return LockScope::Write;
  }

private slots:
  void validateExpressions();
  void insertOperator(const QString &op);
  void insertSignal();
  void insertField();
  void copyWriteToRead();
  void clearAll();

private:
  void setupUi();
  void populateAvailableSignalsAndFields();
  QLineEdit *activeLineEdit() const;

  const RegMapTreeModel *m_model = nullptr;
  QString m_contextItemName;
  QStringList m_availableSignals;
  QStringList m_availableFields;

  QLineEdit *m_wrExprEdit = nullptr;
  QLineEdit *m_rdExprEdit = nullptr;
  QComboBox *m_signalsCombo = nullptr;
  QComboBox *m_fieldsCombo = nullptr;
  QLabel *m_statusLabel = nullptr;
  QLabel *m_previewSvLabel = nullptr;
  QLabel *m_previewVhdLabel = nullptr;
  QPushButton *m_okBtn = nullptr;
};

#endif // REG_LOCK_DIALOG_HPP
