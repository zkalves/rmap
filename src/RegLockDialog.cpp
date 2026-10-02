/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "RegLockDialog.hpp"
#include "LockParser.hpp"
#include "RegMapTreeItem.hpp"
#include "RegMapTreeModel.hpp"
#include "RegMapWindow.hpp"
#include <QBoxLayout>
#include <QComboBox>
#include <QCompleter>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSet>
#include <QTabWidget>

RegLockDialog::RegLockDialog(const QString &writeExpr, const QString &readExpr,
                             const RegMapTreeModel *model,
                             const QString &contextItemName, QWidget *parent)
    : QDialog(parent), m_model(model), m_contextItemName(contextItemName) {
  setWindowIcon(RegMapWindow::appIcon());
  setWindowTitle(contextItemName.isEmpty()
                     ? tr("Configure Register Locks")
                     : tr("Configure Locks: %1").arg(contextItemName));
  resize(700, 580);
  populateAvailableSignalsAndFields();
  setupUi();

  if (m_wrExprEdit)
    m_wrExprEdit->setText(writeExpr.trimmed());
  if (m_rdExprEdit)
    m_rdExprEdit->setText(readExpr.trimmed());

  validateExpressions();
}

RegLockDialog::RegLockDialog(const QString &currentExpr,
                             const RegMapTreeModel *model,
                             const QString &contextItemName, QWidget *parent)
    : QDialog(parent), m_model(model), m_contextItemName(contextItemName) {
  setWindowIcon(RegMapWindow::appIcon());
  setWindowTitle(contextItemName.isEmpty()
                     ? tr("Configure Register Locks")
                     : tr("Configure Locks: %1").arg(contextItemName));
  resize(700, 580);
  populateAvailableSignalsAndFields();
  setupUi();

  ParsedLockResult parsed = LockParser::parse(currentExpr.trimmed(), m_model);
  if (parsed.hasWriteLock && m_wrExprEdit) {
    m_wrExprEdit->setText(parsed.writeExpr);
  }
  if (parsed.hasReadLock && m_rdExprEdit) {
    m_rdExprEdit->setText(parsed.readExpr);
  }

  validateExpressions();
}

RegLockDialog::~RegLockDialog() = default;

QString RegLockDialog::writeExpression() const {
  return m_wrExprEdit ? m_wrExprEdit->text().trimmed() : QString();
}

QString RegLockDialog::readExpression() const {
  return m_rdExprEdit ? m_rdExprEdit->text().trimmed() : QString();
}

QLineEdit *RegLockDialog::activeLineEdit() const {
  if (m_rdExprEdit && m_rdExprEdit->hasFocus())
    return m_rdExprEdit;
  return m_wrExprEdit;
}

void RegLockDialog::populateAvailableSignalsAndFields() {
  m_availableSignals.clear();
  m_availableFields.clear();

  if (!m_model || !m_model->rootItem()) {
    return;
  }

  QSet<QString> knownSignals;
  RegMapTreeItem *root = m_model->rootItem();
  for (RegMapTreeItem *blk : root->childItems()) {
    if (!blk || blk->kind() != RegMapTreeItem::e_rmmKind::blk)
      continue;

    for (RegMapTreeItem *reg : blk->childItems()) {
      if (!reg || reg->kind() != RegMapTreeItem::e_rmmKind::reg)
        continue;
      QString rName = reg->data("Name").toString().trimmed();

      auto collectSignals = [&](const QString &colName) {
        QString lk = reg->data(colName).toString().trimmed();
        if (!lk.isEmpty()) {
          for (const QString &sig : LockParser::extractExternalSignals(lk)) {
            knownSignals.insert(sig);
          }
        }
      };
      collectSignals("Write Lock");
      collectSignals("Read Lock");
      collectSignals("Lock");

      for (RegMapTreeItem *fld : reg->childItems()) {
        if (!fld || fld->kind() != RegMapTreeItem::e_rmmKind::fld)
          continue; // GCOV_EXCL_LINE - Defensive kind guard
        QString fName = fld->data("Name").toString().trimmed();
        if (!rName.isEmpty() && !fName.isEmpty()) {
          m_availableFields.append(QString("%1.%2").arg(rName, fName));
        }

        auto collectFldSignals = [&](const QString &colName) {
          QString lk = fld->data(colName).toString().trimmed();
          if (!lk.isEmpty()) {
            for (const QString &sig : LockParser::extractExternalSignals(lk)) {
              knownSignals.insert(sig);
            }
          }
        };
        collectFldSignals("Write Lock");
        collectFldSignals("Read Lock");
        collectFldSignals("Lock");
      }
    }
  }

  m_availableSignals = knownSignals.values();
  std::sort(m_availableSignals.begin(), m_availableSignals.end());
  std::sort(m_availableFields.begin(), m_availableFields.end());
}

void RegLockDialog::setupUi() {
  auto *mainLayout = new QVBoxLayout(this);

  // Group 1: Lock Expressions
  auto *exprGroup = new QGroupBox(
      tr("Independent Lock Expressions (Language-Agnostic)"), this);
  auto *exprLayout = new QVBoxLayout(exprGroup);

  auto *wrLabel = new QLabel(tr("<b>Write Lock Expression:</b> (Protects from "
                                "SW writes, suppresses write strobe)"),
                             exprGroup);
  m_wrExprEdit = new QLineEdit(exprGroup);
  m_wrExprEdit->setPlaceholderText(
      tr("e.g. hw_sec_lock_i || (SEC_CTRL.LOCK == 0)"));
  m_wrExprEdit->setClearButtonEnabled(true);

  auto *rdLabel = new QLabel(tr("<b>Read Lock Expression:</b> (Protects from "
                                "SW reads, returns 0, suppresses read strobe)"),
                             exprGroup);
  m_rdExprEdit = new QLineEdit(exprGroup);
  m_rdExprEdit->setPlaceholderText(tr("e.g. hw_read_lock_i"));
  m_rdExprEdit->setClearButtonEnabled(true);

  auto *copyBtn =
      new QPushButton(tr("⇄ Copy Write Lock to Read Lock"), exprGroup);
  connect(copyBtn, &QPushButton::clicked, this,
          &RegLockDialog::copyWriteToRead);

  exprLayout->addWidget(wrLabel);
  exprLayout->addWidget(m_wrExprEdit);
  exprLayout->addWidget(rdLabel);
  exprLayout->addWidget(m_rdExprEdit);
  exprLayout->addWidget(copyBtn, 0, Qt::AlignLeft);
  mainLayout->addWidget(exprGroup);

  // Group 2: Insert Operands & Operators
  auto *insertGroup = new QGroupBox(tr("Insert Operands & Operators"), this);
  auto *insertLayout = new QVBoxLayout(insertGroup);

  // Hardware Signals Row
  auto *sigRow = new QHBoxLayout();
  sigRow->addWidget(new QLabel(tr("Hardware Signal:"), insertGroup));
  m_signalsCombo = new QComboBox(insertGroup);
  m_signalsCombo->setEditable(true);
  m_signalsCombo->addItems(m_availableSignals);
  if (m_signalsCombo->completer())
    m_signalsCombo->completer()->setFilterMode(Qt::MatchContains);
  sigRow->addWidget(m_signalsCombo, 1);

  auto *insSigBtn = new QPushButton(tr("Insert Signal"), insertGroup);
  connect(insSigBtn, &QPushButton::clicked, this, &RegLockDialog::insertSignal);
  sigRow->addWidget(insSigBtn);
  insertLayout->addLayout(sigRow);

  // Register Fields Row
  auto *fldRow = new QHBoxLayout();
  fldRow->addWidget(new QLabel(tr("Register Field:"), insertGroup));
  m_fieldsCombo = new QComboBox(insertGroup);
  m_fieldsCombo->setEditable(true);
  m_fieldsCombo->addItems(m_availableFields);
  if (m_fieldsCombo->completer())
    m_fieldsCombo->completer()->setFilterMode(Qt::MatchContains);
  fldRow->addWidget(m_fieldsCombo, 1);

  auto *insFldBtn = new QPushButton(tr("Insert Field"), insertGroup);
  connect(insFldBtn, &QPushButton::clicked, this, &RegLockDialog::insertField);
  fldRow->addWidget(insFldBtn);
  insertLayout->addLayout(fldRow);

  // Operators Row
  auto *opRow = new QHBoxLayout();
  opRow->addWidget(new QLabel(tr("Operators:"), insertGroup));

  const QStringList ops = {"&&", "||", "!", "^", "==", "!=", "(", ")"};
  for (const QString &op : ops) {
    auto *btn = new QPushButton(op, insertGroup);
    btn->setFixedWidth(38);
    connect(btn, &QPushButton::clicked, this,
            [this, op]() { insertOperator(op); });
    opRow->addWidget(btn);
  }
  opRow->addStretch();
  insertLayout->addLayout(opRow);
  mainLayout->addWidget(insertGroup);

  // Group 3: Live RTL Preview
  auto *prevGroup =
      new QGroupBox(tr("Live Synthesizable RTL Preview (Via Templates)"), this);
  auto *prevLayout = new QVBoxLayout(prevGroup);
  auto *tabWidget = new QTabWidget(prevGroup);

  m_previewSvLabel = new QLabel(tabWidget);
  m_previewSvLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
  m_previewSvLabel->setWordWrap(true);
  tabWidget->addTab(m_previewSvLabel, tr("SystemVerilog / Verilog"));

  m_previewVhdLabel = new QLabel(tabWidget);
  m_previewVhdLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
  m_previewVhdLabel->setWordWrap(true);
  tabWidget->addTab(m_previewVhdLabel, tr("VHDL"));

  prevLayout->addWidget(tabWidget);
  mainLayout->addWidget(prevGroup);

  // Status Label
  m_statusLabel = new QLabel(this);
  m_statusLabel->setWordWrap(true);
  mainLayout->addWidget(m_statusLabel);

  // Dialog Buttons
  auto *btnBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Reset,
      this);
  m_okBtn = btnBox->button(QDialogButtonBox::Ok);
  QPushButton *clearBtn = btnBox->button(QDialogButtonBox::Reset);
  if (clearBtn) {
    clearBtn->setText(tr("Clear All"));
    connect(clearBtn, &QPushButton::clicked, this, &RegLockDialog::clearAll);
  }

  connect(btnBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  mainLayout->addWidget(btnBox);

  connect(m_wrExprEdit, &QLineEdit::textChanged, this,
          &RegLockDialog::validateExpressions);
  connect(m_rdExprEdit, &QLineEdit::textChanged, this,
          &RegLockDialog::validateExpressions);
}

void RegLockDialog::copyWriteToRead() {
  if (m_wrExprEdit && m_rdExprEdit) {
    m_rdExprEdit->setText(m_wrExprEdit->text());
  }
}

void RegLockDialog::clearAll() {
  if (m_wrExprEdit)
    m_wrExprEdit->clear();
  if (m_rdExprEdit)
    m_rdExprEdit->clear();
}

void RegLockDialog::insertOperator(const QString &op) {
  QLineEdit *target = activeLineEdit();
  if (!target)
    return; // GCOV_EXCL_LINE - Defensive target check
  QString cur = target->text();
  int cursor = target->cursorPosition();
  QString insertStr = op;
  if (op != "(" && op != ")") {
    if (cursor > 0 && !cur[cursor - 1].isSpace())
      insertStr = " " + insertStr;
    if (cursor < cur.length() && !cur[cursor].isSpace())
      insertStr += " ";
  }
  target->insert(insertStr);
  target->setFocus();
}

void RegLockDialog::insertSignal() {
  QLineEdit *target = activeLineEdit();
  if (!m_signalsCombo || !target)
    return;
  QString sig = m_signalsCombo->currentText().trimmed();
  if (sig.isEmpty())
    return;

  int cursor = target->cursorPosition();
  QString cur = target->text();
  QString insertStr = sig;
  if (cursor > 0 && !cur[cursor - 1].isSpace() && cur[cursor - 1] != '(' &&
      cur[cursor - 1] != '!') {
    insertStr = " " + insertStr;
  }
  target->insert(insertStr);
  target->setFocus();
}

void RegLockDialog::insertField() {
  QLineEdit *target = activeLineEdit();
  if (!m_fieldsCombo || !target)
    return;
  QString fld = m_fieldsCombo->currentText().trimmed();
  if (fld.isEmpty())
    return;

  int cursor = target->cursorPosition();
  QString cur = target->text();
  QString insertStr = fld;
  if (cursor > 0 && !cur[cursor - 1].isSpace() && cur[cursor - 1] != '(' &&
      cur[cursor - 1] != '!') {
    insertStr = " " + insertStr;
  }
  target->insert(insertStr);
  target->setFocus();
}

void RegLockDialog::validateExpressions() {
  if (!m_statusLabel || !m_okBtn)
    return;

  QString wr = writeExpression();
  QString rd = readExpression();

  if (wr.isEmpty() && rd.isEmpty()) {
    m_statusLabel->setText(
        tr("<font color='#6c757d'><b>No lock configured</b> — Register is "
           "always accessible by software.</font>"));
    m_previewSvLabel->clear();
    m_previewVhdLabel->clear();
    m_okBtn->setEnabled(true);
    return;
  }

  ParsedLockResult resWr;
  ParsedLockResult resRd;

  if (!wr.isEmpty()) {
    resWr = LockParser::parse(wr, m_model);
    if (!resWr.valid) {
      m_statusLabel->setText(
          tr("<font color='#dc3545'><b>Write Lock Syntax Error:</b> %1</font>")
              .arg(resWr.errorMessage));
      m_previewSvLabel->clear();
      m_previewVhdLabel->clear();
      m_okBtn->setEnabled(false);
      return;
    }
  }

  if (!rd.isEmpty()) {
    resRd = LockParser::parse(rd, m_model);
    if (!resRd.valid) {
      m_statusLabel->setText(
          tr("<font color='#dc3545'><b>Read Lock Syntax Error:</b> %1</font>")
              .arg(resRd.errorMessage));
      m_previewSvLabel->clear();
      m_previewVhdLabel->clear();
      m_okBtn->setEnabled(false);
      return;
    }
  }

  QSet<QString> ports;
  for (const QString &s : resWr.externalSignals)
    ports.insert(s);
  for (const QString &s : resRd.externalSignals)
    ports.insert(s);

  QStringList portList = ports.values();
  std::sort(portList.begin(), portList.end());

  QString portInfo;
  if (!portList.isEmpty()) {
    portInfo = tr("<br><b>External Top-Level Ports (Deduplicated):</b> %1")
                   .arg(portList.join(", "));
  }

  QString statusText;
  if (!wr.isEmpty() && !rd.isEmpty()) {
    statusText = tr("<b>Valid Write & Read Locks configured.</b>");
  } else if (!wr.isEmpty()) {
    statusText = tr("<b>Valid Write Lock configured.</b> (Reads unrestricted)");
  } else {
    statusText = tr("<b>Valid Read Lock configured.</b> (Writes unrestricted, "
                    "reads return 0)");
  }

  m_statusLabel->setText(
      tr("<font color='#28a745'>%1</font>%2").arg(statusText, portInfo));

  QString svLines;
  QString vhdLines;

  if (!wr.isEmpty()) {
    QString sv = LockParser::toSystemVerilog(wr, m_model);
    QString vhd = LockParser::toVhdl(wr, m_model);
    svLines += tr("<b>Write Lock:</b> <code>wire reg_wr_locked = (%1); // "
                  "gates wr_strobe & next-state</code><br>")
                   .arg(sv);
    vhdLines += tr("<b>Write Lock:</b> <code>reg_wr_locked &lt;= '1' when (%1) "
                   "else '0';</code><br>")
                    .arg(vhd);
  }
  if (!rd.isEmpty()) {
    QString sv = LockParser::toSystemVerilog(rd, m_model);
    QString vhd = LockParser::toVhdl(rd, m_model);
    svLines += tr("<b>Read Lock:</b> <code>wire reg_rd_locked = (%1); // "
                  "returns '0' & gates rd_strobe</code><br>")
                   .arg(sv);
    vhdLines += tr("<b>Read Lock:</b> <code>reg_rd_locked &lt;= '1' when (%1) "
                   "else '0';</code><br>")
                    .arg(vhd);
  }

  m_previewSvLabel->setText(svLines);
  m_previewVhdLabel->setText(vhdLines);
  m_okBtn->setEnabled(true);
}
