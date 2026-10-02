/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "RegLockDialog.hpp"
#include "RegMapTreeItem.hpp"
#include "RegMapTreeModel.hpp"
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QtTest>

class TestRegLockDialog : public QObject {
  Q_OBJECT

private slots:
  void initTestCase() {
    Q_INIT_RESOURCE(resources);
    QDir("work").removeRecursively();
    QDir().mkpath("work");
  }

  void cleanupTestCase() {
    QDir("work").removeRecursively();
  }

  void testConstructorsAndGetters();
  void testSingleExpressionConstructor();
  void testUiInteractionAndOperators();
  void testValidationAndPreview();
  void testModelSignalsAndFieldsPopulation();
  void testDialogButtons();
};

void TestRegLockDialog::testConstructorsAndGetters() {
  RegMapTreeModel model;

  // 1. Two-expression constructor with both write and read locks
  {
    RegLockDialog dlg("hw_sec_lock_i", "hw_read_lock_i", &model, "REG_TEST");
    QCOMPARE(dlg.writeExpression(), QString("hw_sec_lock_i"));
    QCOMPARE(dlg.readExpression(), QString("hw_read_lock_i"));
    QCOMPARE(dlg.expression(), QString("hw_sec_lock_i"));
    QCOMPARE(dlg.scope(), LockScope::Both);
    QVERIFY(dlg.windowTitle().contains("REG_TEST"));
  }

  // 2. Write-only lock
  {
    RegLockDialog dlg("hw_sec_lock_i", "", &model, "");
    QCOMPARE(dlg.writeExpression(), QString("hw_sec_lock_i"));
    QCOMPARE(dlg.readExpression(), QString(""));
    QCOMPARE(dlg.scope(), LockScope::Write);
    QVERIFY(dlg.windowTitle().contains("Configure Register Locks"));
  }

  // 3. Read-only lock
  {
    RegLockDialog dlg("", "hw_read_lock_i", nullptr, "");
    QCOMPARE(dlg.writeExpression(), QString(""));
    QCOMPARE(dlg.readExpression(), QString("hw_read_lock_i"));
    QCOMPARE(dlg.scope(), LockScope::Read);
  }

  // 4. Empty lock
  {
    RegLockDialog dlg("", "", nullptr, "");
    QCOMPARE(dlg.writeExpression(), QString(""));
    QCOMPARE(dlg.readExpression(), QString(""));
    QCOMPARE(dlg.scope(), LockScope::Write);
  }
}

void TestRegLockDialog::testSingleExpressionConstructor() {
  RegMapTreeModel model;

  // 1. [w] expression
  {
    RegLockDialog dlg("[w] hw_write_lock_i", &model, "TEST");
    QCOMPARE(dlg.writeExpression(), QString("hw_write_lock_i"));
    QCOMPARE(dlg.readExpression(), QString(""));
  }

  // 2. [r] expression
  {
    RegLockDialog dlg("[r] hw_read_lock_i", &model, "TEST");
    QCOMPARE(dlg.writeExpression(), QString(""));
    QCOMPARE(dlg.readExpression(), QString("hw_read_lock_i"));
  }

  // 3. [rw] expression
  {
    RegLockDialog dlg("[rw] hw_both_lock_i", &model, "TEST");
    QCOMPARE(dlg.writeExpression(), QString("hw_both_lock_i"));
    QCOMPARE(dlg.readExpression(), QString("hw_both_lock_i"));
  }

  // 4. Semicolon combination
  {
    RegLockDialog dlg("[w] sig_w; [r] sig_r", &model, "TEST");
    QCOMPARE(dlg.writeExpression(), QString("sig_w"));
    QCOMPARE(dlg.readExpression(), QString("sig_r"));
  }
}

void TestRegLockDialog::testUiInteractionAndOperators() {
  RegMapTreeModel model;
  RegLockDialog dlg("", "", &model, "REG_TEST");
  dlg.show();
  QApplication::processEvents();

  auto *wrEdit = dlg.findChild<QLineEdit *>();
  QVERIFY(wrEdit != nullptr);

  auto lineEdits = dlg.findChildren<QLineEdit *>();
  QVERIFY(lineEdits.size() >= 2);
  QLineEdit *editWr = lineEdits[0];
  QLineEdit *editRd = lineEdits[1];

  // 1. Operator insertion into write edit
  editWr->setFocus();
  const QStringList ops = {"&&", "||", "!", "^", "==", "!=", "(", ")"};
  auto buttons = dlg.findChildren<QPushButton *>();
  for (const QString &op : ops) {
    for (auto *btn : buttons) {
      if (btn->text() == op) {
        btn->click();
        QApplication::processEvents();
        break;
      }
    }
  }
  QVERIFY(!editWr->text().isEmpty());

  // 2. Clear all
  for (auto *btn : buttons) {
    if (btn->text().contains("Clear All")) {
      btn->click();
      QApplication::processEvents();
      break;
    }
  }
  QCOMPARE(editWr->text(), QString(""));
  QCOMPARE(editRd->text(), QString(""));

  // 3. Insert signal and field
  auto combos = dlg.findChildren<QComboBox *>();
  QVERIFY(combos.size() >= 2);
  QComboBox *sigCombo = combos[0];
  QComboBox *fldCombo = combos[1];

  sigCombo->addItem("hw_my_signal_i");
  sigCombo->setCurrentText("hw_my_signal_i");
  fldCombo->addItem("CTRL_REG.ENABLE");
  fldCombo->setCurrentText("CTRL_REG.ENABLE");

  editWr->setFocus();
  for (auto *btn : buttons) {
    if (btn->text() == "Insert Signal") {
      btn->click();
      QApplication::processEvents();
    }
  }
  QVERIFY(editWr->text().contains("hw_my_signal_i"));

  // Insert operator &&
  for (auto *btn : buttons) {
    if (btn->text() == "&&") {
      btn->click();
      QApplication::processEvents();
      break;
    }
  }

  // Insert field
  for (auto *btn : buttons) {
    if (btn->text() == "Insert Field") {
      btn->click();
      QApplication::processEvents();
    }
  }
  QVERIFY(editWr->text().contains("CTRL_REG.ENABLE"));

  // 4. Test insert into read edit when focused
  editRd->setFocus();
  editRd->clear();
  for (auto *btn : buttons) {
    if (btn->text() == "Insert Signal") {
      btn->click();
      QApplication::processEvents();
    }
  }
  QVERIFY(editRd->text().contains("hw_my_signal_i"));

  // 5. Test copy write to read
  editWr->setText("hw_copied_lock");
  for (auto *btn : buttons) {
    if (btn->text().contains("Copy Write Lock")) {
      btn->click();
      QApplication::processEvents();
      break;
    }
  }
  QCOMPARE(editRd->text(), QString("hw_copied_lock"));

  // 6. Test operator insertion in middle of text
  editWr->setText("AB");
  editWr->setFocus();
  editWr->setCursorPosition(1);
  QMetaObject::invokeMethod(&dlg, "insertOperator", Q_ARG(QString, "&&"));
  QVERIFY(editWr->text().contains(" && "));

  // 7. Test signal/field insertion after non-space char
  editWr->setText("A");
  editWr->setFocus();
  editWr->setCursorPosition(1);
  sigCombo->setCurrentText("hw_signal_2");
  QMetaObject::invokeMethod(&dlg, "insertSignal");
  QVERIFY(editWr->text().contains(" hw_signal_2"));

  editWr->setText("A");
  editWr->setFocus();
  editWr->setCursorPosition(1);
  fldCombo->setCurrentText("REG.FLD");
  QMetaObject::invokeMethod(&dlg, "insertField");
  QVERIFY(editWr->text().contains(" REG.FLD"));

  // 8. Test insertion with empty text
  sigCombo->setCurrentText("");
  QMetaObject::invokeMethod(&dlg, "insertSignal");
  fldCombo->setCurrentText("");
  QMetaObject::invokeMethod(&dlg, "insertField");
}

void TestRegLockDialog::testValidationAndPreview() {
  RegMapTreeModel model;
  RegLockDialog dlg("", "", &model, "VALIDATE_TEST");
  dlg.show();
  QApplication::processEvents();

  auto lineEdits = dlg.findChildren<QLineEdit *>();
  QLineEdit *editWr = lineEdits[0];
  QLineEdit *editRd = lineEdits[1];

  auto *btnBox = dlg.findChild<QDialogButtonBox *>();
  QVERIFY(btnBox != nullptr);
  auto *okBtn = btnBox->button(QDialogButtonBox::Ok);
  QVERIFY(okBtn != nullptr);

  auto labels = dlg.findChildren<QLabel *>();

  // 1. Empty expressions -> Valid
  editWr->setText("");
  editRd->setText("");
  QApplication::processEvents();
  QVERIFY(okBtn->isEnabled());

  // 2. Valid write expression only
  editWr->setText("hw_lock_a_i || hw_lock_b_i");
  editRd->setText("");
  QApplication::processEvents();
  QVERIFY(okBtn->isEnabled());

  // 3. Valid read expression only
  editWr->setText("");
  editRd->setText("hw_lock_c_i");
  QApplication::processEvents();
  QVERIFY(okBtn->isEnabled());

  // 4. Valid both expressions
  editWr->setText("hw_lock_a_i");
  editRd->setText("hw_lock_b_i");
  QApplication::processEvents();
  QVERIFY(okBtn->isEnabled());

  // 5. Write expression syntax error
  editWr->setText("&& invalid");
  editRd->setText("");
  QApplication::processEvents();
  QVERIFY(!okBtn->isEnabled());

  // 6. Read expression syntax error
  editWr->setText("hw_lock_a_i");
  editRd->setText("|| invalid");
  QApplication::processEvents();
  QVERIFY(!okBtn->isEnabled());
}

void TestRegLockDialog::testModelSignalsAndFieldsPopulation() {
  // Construct a model with blocks, registers, fields, and other kinds to test filtering
  RegMapTreeModel model;
  QVERIFY(model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::blk, QModelIndex()));
  QModelIndex blkIdx = model.index(0, 0, QModelIndex());
  model.setData(model.index(0, model.columnOf("Name"), QModelIndex()), "SEC_BLK", Qt::EditRole);

  // Non-blk child at root (e.g. mem)
  QVERIFY(model.insertRows(1, 1, RegMapTreeItem::e_rmmKind::mem, QModelIndex()));

  QVERIFY(model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::reg, blkIdx));
  QModelIndex regIdx = model.index(0, 0, blkIdx);
  model.setData(model.index(0, model.columnOf("Name"), blkIdx), "SEC_CTRL", Qt::EditRole);
  model.setData(model.index(0, model.columnOf("Write Lock"), blkIdx), "hw_sec_lock_i", Qt::EditRole);
  model.setData(model.index(0, model.columnOf("Read Lock"), blkIdx), "hw_rd_sec_i", Qt::EditRole);

  // Non-reg child under blk (e.g. mem)
  QVERIFY(model.insertRows(1, 1, RegMapTreeItem::e_rmmKind::mem, blkIdx));

  QVERIFY(model.insertRows(0, 1, RegMapTreeItem::e_rmmKind::fld, regIdx));
  model.setData(model.index(0, model.columnOf("Name"), regIdx), "LOCK_BIT", Qt::EditRole);
  model.setData(model.index(0, model.columnOf("Write Lock"), regIdx), "hw_fld_lock_i", Qt::EditRole);

  RegLockDialog dlg("", "", &model, "TEST_POPULATE");
  auto combos = dlg.findChildren<QComboBox *>();
  QVERIFY(combos.size() >= 2);
  QComboBox *sigCombo = combos[0];
  QComboBox *fldCombo = combos[1];

  QVERIFY(sigCombo->count() > 0);
  QVERIFY(fldCombo->count() > 0);
}

void TestRegLockDialog::testDialogButtons() {
  RegLockDialog dlg("hw_sec_lock_i", "", nullptr, "TEST_BTNS");
  dlg.show();
  QApplication::processEvents();

  dlg.accept();
  QCOMPARE(dlg.result(), static_cast<int>(QDialog::Accepted));

  dlg.reject();
  QCOMPARE(dlg.result(), static_cast<int>(QDialog::Rejected));

  // Test polymorphic heap deletion
  std::unique_ptr<QDialog> dynDlg = std::make_unique<RegLockDialog>("hw_sec_lock_i");
  dynDlg.reset();
}

QTEST_MAIN(TestRegLockDialog)
#include "test_RegLockDialog.moc"
