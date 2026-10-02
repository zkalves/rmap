/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "RegMapWindow.hpp"
#include "AppSettings.hpp"
#include "BlockMemoryMapWidget.hpp"
#include "LanguageManager.hpp"
#include "PathUtils.hpp"
#include "RegLockDialog.hpp"
#include "RmapVersion.hpp"
#include "ThemeManager.hpp"
#include "format/FormatManager.hpp"
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QScrollArea>
#include <QShortcut>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTextBrowser>
#include <QTextStream>
#include <QVBoxLayout>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <unistd.h>

namespace {
uint64_t parseNumericCell(const QString &str) {
  QString s = str.trimmed();
  if (s.startsWith("0b", Qt::CaseInsensitive)) {
    return s.mid(2).toULongLong(nullptr, 2);
  }
  return s.toULongLong(nullptr, 0);
}
} // namespace

// Helper to convert QVariant numbers (hex/dec/bin string) to uint64_t
static uint64_t parseNumericValue(const QVariant &var) {
  return parseNumericCell(var.toString());
}

// Helper to pad hex offset string written in hex (0x format)
static QString padHexOffsetString(const QString &input, int minDigits = 4) {
  QString s = input.trimmed();
  if (s.startsWith("0x", Qt::CaseInsensitive)) {
    bool ok = false;
    uint64_t val = s.mid(2).toULongLong(&ok, 16);
    if (ok) {
      int digits = (std::max)((int)minDigits, (int)(s.size() - 2));
      if (val >= 0x100000000ULL)
        digits = (std::max)(digits, 16);
      else if (val >= 0x10000ULL)
        digits = (std::max)(digits, 8);
      else
        digits = (std::max)(digits, minDigits);
      return QString("0x") +
             QString("%1").arg(val, digits, 16, QChar('0')).toUpper();
    }
  }
  return s;
}

// Recursive helper for strict lint checking (non-empty descriptions and byte
// alignment)
static void performStrictLintChecks(RegMapTreeItem *item, uint32_t regWidth,
                                    QStringList &warnings) {
  if (!item)
    return; // GCOV_EXCL_LINE - Defensive invariant
  QString kind = item->kindString();
  QString name = item->data("Name").toString();
  QString desc = item->data("Description").toString();

  if ((kind == "reg" || kind == "fld") && desc.trimmed().isEmpty()) {
    warnings.append(
        QString("Missing description for %1 '%2'").arg(kind.toUpper(), name));
  }
  if (kind == "reg") {
    bool ok = false;
    QString offStr = item->data("Offset/LSB").toString().trimmed();
    uint64_t off = offStr.toULongLong(&ok, 0);
    uint64_t alignBytes = regWidth / 8;
    if (ok && alignBytes > 0 &&
        (off % alignBytes !=
         0)) { // GCOV_EXCL_BR_LINE - alignBytes > 0 guaranteed for 32/64 bit
      warnings.append(
          QString("Register '%1' offset 0x%2 is not %3-byte aligned")
              .arg(name, QString::number(off, 16).toUpper(),
                   QString::number(alignBytes)));
    }
  }
  for (RegMapTreeItem *child : item->getChildItems()) {
    performStrictLintChecks(child, regWidth, warnings);
  }
}

struct RegSummary {
  QString blockName;
  QString regName;
  QString offset;
  QString access;
  QString reset;
  std::map<QString, QString> fields; // name -> "lsb:width:access:reset"
};

static void gatherRegs(RegMapTreeModel &model,
                       std::map<QString, RegSummary> &map) {
  RegMapTreeItem *root = model.getRootItem();
  if (!root)
    return; // GCOV_EXCL_LINE - Defensive invariant
  for (RegMapTreeItem *blk : root->getChildItems()) {
    if (!blk || blk->kindString() != "blk")
      continue; // GCOV_EXCL_BR_LINE - blk pointer is guaranteed non-null
    QString bName = blk->data("Name").toString();
    for (RegMapTreeItem *reg : blk->getChildItems()) {
      if (!reg || reg->kindString() != "reg")
        continue; // GCOV_EXCL_BR_LINE - reg pointer is guaranteed non-null
      QString rName = reg->data("Name").toString();
      QString key = bName + "::" + rName;
      RegSummary s;
      s.blockName = bName;
      s.regName = rName;
      s.offset = reg->data("Offset/LSB").toString();
      s.access = reg->data("SW Access").toString();
      s.reset = reg->data("Reset Value").toString();
      for (RegMapTreeItem *fld : reg->getChildItems()) {
        if (fld &&
            fld->kindString() == "fld") { // GCOV_EXCL_BR_LINE - fld pointer is
                                          // guaranteed non-null
          QString fName = fld->data("Name").toString();
          QString fSig = QString("%1:%2:%3:%4")
                             .arg(fld->data("Offset/LSB").toString())
                             .arg(fld->data("Size/Width").toString())
                             .arg(fld->data("SW Access").toString())
                             .arg(fld->data("Reset Value").toString());
          s.fields[fName] = fSig;
        }
      }
      map[key] = s;
    }
  }
}

class TreeFilterProxyModel : public QSortFilterProxyModel {
  Q_OBJECT
public:
  TreeFilterProxyModel(QObject *parent = nullptr)
      : QSortFilterProxyModel(parent) {
    setDynamicSortFilter(true);
    setSortCaseSensitivity(Qt::CaseInsensitive);
  }

  void setSearchFilter(const QString &filter) {
    m_filterText = filter.trimmed().toLower();
    invalidateFilter();
  }

  QString searchFilter() const { return m_filterText; }

protected:
  bool lessThan(const QModelIndex &source_left,
                const QModelIndex &source_right) const override {
    int col = source_left.column();
    if (col == 1 || col == 2 || col == 6) { // Offset / LSB, Size, Reset
      QString l_str =
          sourceModel()->data(source_left, Qt::DisplayRole).toString();
      QString r_str =
          sourceModel()->data(source_right, Qt::DisplayRole).toString();
      return parseNumericCell(l_str) < parseNumericCell(r_str);
    }
    return QSortFilterProxyModel::lessThan(source_left, source_right);
  }

  bool filterAcceptsRow(int source_row,
                        const QModelIndex &source_parent) const override {
    if (!sourceModel())
      return false; // GCOV_EXCL_LINE - Defensive null check
    QModelIndex index0 = sourceModel()->index(source_row, 0, source_parent);
    if (!index0.isValid())
      return false; // GCOV_EXCL_LINE - Defensive index check

    QString kind = sourceModel()->data(index0, Qt::DisplayRole).toString();
    if (kind == "fld")
      return false; // Never show fields in left tree

    if (m_filterText.isEmpty())
      return true;

    RegMapTreeItem *item =
        static_cast<RegMapTreeItem *>(index0.internalPointer());
    if (!item)
      return false; // GCOV_EXCL_LINE - Defensive null check

    if (itemMatches(item))
      return true;

    // Check if any child matches
    for (int r = 0; r < item->childCount(); ++r) {
      RegMapTreeItem *child = item->child(r);
      if (child && (itemMatches(child) ||
                    childHasMatch(child))) { // GCOV_EXCL_BR_LINE - child
                                             // pointer guaranteed non-null
        return true;
      }
    }
    return false;
  }

  bool itemMatches(RegMapTreeItem *item) const {
    if (!item)
      return false; // GCOV_EXCL_LINE - Defensive null check
    QString name = item->data("Name").toString().toLower();
    QString offset = item->data("Offset/LSB").toString().toLower();
    QString desc = item->data("Description").toString().toLower();
    QString access = item->data("SW Access").toString().toLower();
    return name.contains(m_filterText) || offset.contains(m_filterText) ||
           desc.contains(m_filterText) || access.contains(m_filterText);
  }

  bool childHasMatch(RegMapTreeItem *parent) const {
    for (int r = 0; r < parent->childCount(); ++r) {
      RegMapTreeItem *child = parent->child(r);
      if (child && (itemMatches(child) || childHasMatch(child)))
        return true; // GCOV_EXCL_BR_LINE - child pointer guaranteed non-null
    }
    return false;
  }

  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
      if (section == 1)
        return tr("Offset");
      if (section == 2)
        return tr("Size");
    }
    return QSortFilterProxyModel::headerData(section, orientation, role);
  }

private:
  QString m_filterText;
};

class FieldSortProxyModel : public QSortFilterProxyModel {
  Q_OBJECT
public:
  FieldSortProxyModel(QObject *parent = nullptr)
      : QSortFilterProxyModel(parent) {}

protected:
  bool lessThan(const QModelIndex &source_left,
                const QModelIndex &source_right) const override {
    if (source_left.column() == 1) { // Sort by Offset
      QString l_str =
          sourceModel()->data(source_left, Qt::DisplayRole).toString();
      QString r_str =
          sourceModel()->data(source_right, Qt::DisplayRole).toString();
      return parseNumericCell(l_str) < parseNumericCell(r_str);
    }
    return QSortFilterProxyModel::lessThan(source_left, source_right);
  }
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
      if (section == 1)
        return tr("LSB");
      if (section == 2)
        return tr("Size");
    }
    return QSortFilterProxyModel::headerData(section, orientation, role);
  }
};

QIcon RegMapWindow::appIcon() {
  Q_INIT_RESOURCE(resources);
  QIcon icon(QStringLiteral(":/icons/app_icon.png"));
  icon.addFile(QStringLiteral(":/icons/app_icon_512.png"));
  icon.addFile(QStringLiteral(":/icons/app_icon_256.png"));
  icon.addFile(QStringLiteral(":/icons/app_icon_128.png"));
  icon.addFile(QStringLiteral(":/icons/app_icon_64.png"));
  icon.addFile(QStringLiteral(":/icons/app_icon_48.png"));
  icon.addFile(QStringLiteral(":/icons/app_icon_32.png"));
  icon.addFile(QStringLiteral(":/icons/app_icon_16.png"));
  return icon;
}

RegMapWindow::RegMapWindow(const QString &rmap_filename, QWidget *parent)
    : QMainWindow(parent) {
  Q_INIT_RESOURCE(resources);
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  this->setupUi(this);
  setWindowIcon(appIcon());
  m_active_folder = ".";
  m_is_regmap_modified = false;
  m_rmap_filename = rmap_filename;
  m_default_filename = "rmap.rmt";
  m_default_window_title = windowTitle();

  m_config_window = new RegConfigWindow(this);
  m_pref_window = new PreferencesWindow(this);
  m_about_window = new AboutWindow(this);

  m_undoStack = new QUndoStack(this);

  // Setup Undo / Redo connections
  connect(actionUndo, &QAction::triggered, m_undoStack, &QUndoStack::undo);
  connect(actionRedo, &QAction::triggered, m_undoStack, &QUndoStack::redo);
  connect(m_undoStack, &QUndoStack::canUndoChanged, actionUndo,
          &QAction::setEnabled);
  connect(m_undoStack, &QUndoStack::canRedoChanged, actionRedo,
          &QAction::setEnabled);
  actionUndo->setEnabled(false);
  actionRedo->setEnabled(false);

  connect(m_undoStack, &QUndoStack::undoTextChanged, this,
          [this](const QString &t) {
            actionUndo->setText(t.isEmpty() ? tr("Undo")
                                            : tr("Undo %1").arg(t));
          });
  connect(m_undoStack, &QUndoStack::redoTextChanged, this,
          [this](const QString &t) {
            actionRedo->setText(t.isEmpty() ? tr("Redo")
                                            : tr("Redo %1").arg(t));
          });

  // Model and Proxies
  m_model = new RegMapTreeModel(this);
  connectModelSignals();

  m_treeProxy = new TreeFilterProxyModel(this);
  m_fieldProxy = new FieldSortProxyModel(this);
  m_treeProxy->setSourceModel(m_model);
  m_fieldProxy->setSourceModel(m_model);

  // Left Panel: Stacked Widget (Tree View vs Empty View)
  QWidget *leftPanel = new QWidget(this);
  QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
  leftLayout->setContentsMargins(0, 0, 0, 0);
  leftLayout->setSpacing(0);

  m_leftStackedWidget = new QStackedWidget(leftPanel);
  m_leftStackedWidget->setObjectName("leftStackedWidget");

  // ==========================================
  // Page 0: Tree View (Search Bar + Tree)
  // ==========================================
  m_leftViewWidget = new QWidget(m_leftStackedWidget);
  m_leftViewWidget->setObjectName("leftViewWidget");
  QVBoxLayout *leftViewLayout = new QVBoxLayout(m_leftViewWidget);
  leftViewLayout->setContentsMargins(0, 0, 0, 0);
  leftViewLayout->setSpacing(4);

  QHBoxLayout *searchLayout = new QHBoxLayout();
  m_searchEdit = new QLineEdit(m_leftViewWidget);
  m_searchEdit->setObjectName("searchEdit");
  m_searchEdit->setPlaceholderText(
      tr("Search registers/blocks (e.g. CTRL, 0x0)..."));
  m_searchEdit->setClearButtonEnabled(true);
  connect(m_searchEdit, &QLineEdit::textChanged, this,
          &RegMapWindow::onSearchTextChanged);

  searchLayout->addWidget(m_searchEdit);
  leftViewLayout->addLayout(searchLayout);

  this->treeView->setParent(m_leftViewWidget);
  this->treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
  this->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  this->treeView->setModel(m_treeProxy);
  this->treeView->setSortingEnabled(true);
  this->treeView->sortByColumn(1, Qt::AscendingOrder);
  this->treeView->setColumnHidden(
      2, false); // Size/Width (Mem Size, Field Width, Reg Width)
  this->treeView->setColumnWidth(2, 70);
  this->treeView->setColumnHidden(4, true);  // SW Access
  this->treeView->setColumnHidden(5, true);  // HW Access Policy
  this->treeView->setColumnHidden(6, true);  // Reset Value
  this->treeView->setColumnHidden(7, true);  // Is Rand
  this->treeView->setColumnHidden(8, true);  // Volatile
  this->treeView->setColumnHidden(9, true);  // Has Reset
  this->treeView->setColumnHidden(10, true); // Write Lock
  this->treeView->setColumnHidden(11, true); // Read Lock
  this->treeView->setColumnHidden(12, true); // Decode Only
  // Note: Description column (rightmost) remains visible in treeView for all
  // items
  this->treeView->header()->setStretchLastSection(true);
  this->treeView->header()->setSectionResizeMode(QHeaderView::Interactive);
  int descCol = m_model ? m_model->columnOf("Description") : 13;
  if (descCol < 0)
    descCol = 13; // GCOV_EXCL_LINE - Defensive column fallback
  this->treeView->header()->setSectionResizeMode(descCol, QHeaderView::Stretch);

  connect(this->treeView->header(), &QHeaderView::sectionResized, this,
          [this](int logicalIndex, int /*oldSize*/, int newSize) {
            if (!this->treeView->header())
              return; // GCOV_EXCL_LINE - Defensive header check
            int minHeaderSize =
                this->treeView->header()->sectionSizeHint(logicalIndex);
            if (newSize < minHeaderSize) {
              this->treeView->header()->resizeSection(logicalIndex,
                                                      minHeaderSize);
            }
          });
  leftViewLayout->addWidget(this->treeView);

  m_leftStackedWidget->addWidget(m_leftViewWidget);

  // ==========================================
  // Page 1: Empty View (Shown when no model is loaded)
  // ==========================================
  m_leftEmptyWidget = new QWidget(m_leftStackedWidget);
  m_leftEmptyWidget->setObjectName("leftEmptyWidget");

  QVBoxLayout *leftEmptyLayout = new QVBoxLayout(m_leftEmptyWidget);
  leftEmptyLayout->setAlignment(Qt::AlignCenter);
  leftEmptyLayout->setSpacing(8);
  leftEmptyLayout->setContentsMargins(16, 16, 16, 16);

  leftEmptyLayout->addStretch(1);

  QLabel *leftEmptyLogo = new QLabel(m_leftEmptyWidget);
  leftEmptyLogo->setObjectName("leftEmptyLogoLabel");
  QPixmap leftLogoPix(":/icons/app_icon_64.png");
  leftEmptyLogo->setPixmap(leftLogoPix);
  leftEmptyLogo->setAlignment(Qt::AlignCenter);
  leftEmptyLayout->addWidget(leftEmptyLogo);

  QLabel *leftEmptyTitle = new QLabel(tr("No Register Map"), m_leftEmptyWidget);
  leftEmptyTitle->setObjectName("leftEmptyTitleLabel");
  QFont leftTitleFont = leftEmptyTitle->font();
  leftTitleFont.setPointSize(11);
  leftTitleFont.setBold(true);
  leftEmptyTitle->setFont(leftTitleFont);
  leftEmptyTitle->setAlignment(Qt::AlignCenter);
  leftEmptyLayout->addWidget(leftEmptyTitle);

  QLabel *leftEmptyHint =
      new QLabel(tr("Create a new map (Ctrl+N)\nor open a file (Ctrl+O)"),
                 m_leftEmptyWidget);
  leftEmptyHint->setObjectName("leftEmptyHintLabel");
  QFont leftHintFont = leftEmptyHint->font();
  leftHintFont.setPointSize(9);
  leftEmptyHint->setFont(leftHintFont);
  leftEmptyHint->setAlignment(Qt::AlignCenter);
  leftEmptyLayout->addWidget(leftEmptyHint);

  leftEmptyLayout->addStretch(2);

  m_leftStackedWidget->addWidget(m_leftEmptyWidget);
  m_leftStackedWidget->setCurrentWidget(m_leftEmptyWidget);

  leftLayout->addWidget(m_leftStackedWidget);

  // Right Panel: Stacked Widget (Register Bitfield View vs Block Memory Map
  // View)
  QWidget *rightPanel = new QWidget(this);
  QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
  rightLayout->setContentsMargins(0, 0, 0, 0);
  rightLayout->setSpacing(0);

  m_rightStackedWidget = new QStackedWidget(rightPanel);
  m_rightStackedWidget->setObjectName("rightStackedWidget");

  // ==========================================
  // Page 0: Register View (Header, Slice Bar, Fields Table)
  // ==========================================
  m_regViewWidget = new QWidget(m_rightStackedWidget);
  m_regViewWidget->setObjectName("regViewWidget");
  QVBoxLayout *regViewLayout = new QVBoxLayout(m_regViewWidget);
  regViewLayout->setContentsMargins(0, 0, 0, 0);
  regViewLayout->setSpacing(6);

  m_regHeaderWidget = new QWidget(m_regViewWidget);
  m_regHeaderWidget->setObjectName("regHeaderWidget");
  QVBoxLayout *regHeaderMainLayout = new QVBoxLayout(m_regHeaderWidget);
  regHeaderMainLayout->setContentsMargins(4, 2, 4, 2);
  regHeaderMainLayout->setSpacing(4);

  // Row 1: General & Access properties
  QWidget *regRow1Widget = new QWidget(m_regHeaderWidget);
  regRow1Widget->setObjectName("regRow1Widget");
  QHBoxLayout *regRow1Layout = new QHBoxLayout(regRow1Widget);
  regRow1Layout->setContentsMargins(0, 0, 0, 0);
  regRow1Layout->setSpacing(6);

  QLabel *regNameTitle = new QLabel(tr("Name:"), regRow1Widget);
  regNameTitle->setStyleSheet("font-weight: bold; font-size: 12px;");
  m_regNameEdit = new QLineEdit(regRow1Widget);
  m_regNameEdit->setObjectName("regNameEdit");
  m_regNameEdit->setPlaceholderText(tr("Register name..."));
  m_regNameEdit->setClearButtonEnabled(true);
  m_regNameEdit->setMinimumWidth(
      std::max(120, regNameTitle->sizeHint().width()));

  QLabel *regOffsetTitle = new QLabel(tr("Offset:"), regRow1Widget);
  regOffsetTitle->setStyleSheet("font-weight: bold; font-size: 12px;");
  m_regOffsetEdit = new QLineEdit(regRow1Widget);
  m_regOffsetEdit->setObjectName("regOffsetEdit");
  m_regOffsetEdit->setPlaceholderText(tr("0x00"));
  m_regOffsetEdit->setStyleSheet("font-family: monospace; font-size: 12px;");
  m_regOffsetEdit->setClearButtonEnabled(true);
  m_regOffsetEdit->setMinimumWidth(regOffsetTitle->sizeHint().width());
  m_regOffsetEdit->setMaximumWidth(85);

  QLabel *regSizeTitle = new QLabel(tr("Size:"), regRow1Widget);
  regSizeTitle->setStyleSheet("font-weight: bold; font-size: 12px;");
  m_regSizeEdit = new QLineEdit(regRow1Widget);
  m_regSizeEdit->setObjectName("regSizeEdit");
  m_regSizeEdit->setPlaceholderText(tr("32"));
  m_regSizeEdit->setStyleSheet("font-family: monospace; font-size: 12px;");
  m_regSizeEdit->setClearButtonEnabled(true);
  m_regSizeEdit->setMinimumWidth(regSizeTitle->sizeHint().width());
  m_regSizeEdit->setMaximumWidth(55);

  QLabel *regSwTitle = new QLabel(tr("SW:"), regRow1Widget);
  regSwTitle->setStyleSheet("font-weight: bold; font-size: 12px;");
  m_regSwAccessCombo = new QComboBox(regRow1Widget);
  m_regSwAccessCombo->setObjectName("regSwAccessCombo");
  m_regSwAccessCombo->addItems(
      {"RW",  "RO",    "WO",    "W1",    "WO1",   "W1C", "W1S", "W1T",
       "W0C", "W0S",   "W0T",   "RC",    "RS",    "WRC", "WRS", "WC",
       "WS",  "W1SRC", "W1CRS", "W0SRC", "W0CRS", "WOC", "WOS", "NOACCESS"});
  m_regSwAccessCombo->setMaximumWidth(95);

  QLabel *regHwTitle = new QLabel(tr("HW:"), regRow1Widget);
  regHwTitle->setStyleSheet("font-weight: bold; font-size: 12px;");
  m_regHwAccessCombo = new QComboBox(regRow1Widget);
  m_regHwAccessCombo->setObjectName("regHwAccessCombo");
  m_regHwAccessCombo->addItems({"RO", "RW", "WO", "WIRE", "W1T", "INCR", "DECR",
                                "NA", "W1C", "W1S", "W0C", "RS", "RC"});
  m_regHwAccessCombo->setMaximumWidth(85);

  QLabel *regResetTitle = new QLabel(tr("Reset:"), regRow1Widget);
  regResetTitle->setStyleSheet("font-weight: bold; font-size: 12px;");
  m_regResetEdit = new QLineEdit(regRow1Widget);
  m_regResetEdit->setObjectName("regResetEdit");
  m_regResetEdit->setPlaceholderText(tr("0x0"));
  m_regResetEdit->setStyleSheet("font-family: monospace; font-size: 12px;");
  m_regResetEdit->setClearButtonEnabled(true);
  m_regResetEdit->setMinimumWidth(regResetTitle->sizeHint().width());
  m_regResetEdit->setMaximumWidth(95);

  QLabel *descLabel = new QLabel(tr("Description:"), regRow1Widget);
  descLabel->setStyleSheet("font-weight: bold; font-size: 12px;");
  m_regDescEdit = new QLineEdit(regRow1Widget);
  m_regDescEdit->setObjectName("regDescEdit");
  m_regDescEdit->setPlaceholderText(tr("Register description..."));
  m_regDescEdit->setClearButtonEnabled(true);
  m_regDescEdit->setMinimumWidth(descLabel->sizeHint().width());

  regRow1Layout->addWidget(regNameTitle);
  regRow1Layout->addWidget(m_regNameEdit);
  regRow1Layout->addWidget(regOffsetTitle);
  regRow1Layout->addWidget(m_regOffsetEdit);
  regRow1Layout->addWidget(regSizeTitle);
  regRow1Layout->addWidget(m_regSizeEdit);
  regRow1Layout->addWidget(regSwTitle);
  regRow1Layout->addWidget(m_regSwAccessCombo);
  regRow1Layout->addWidget(regHwTitle);
  regRow1Layout->addWidget(m_regHwAccessCombo);
  regRow1Layout->addWidget(regResetTitle);
  regRow1Layout->addWidget(m_regResetEdit);
  regRow1Layout->addWidget(descLabel);
  regRow1Layout->addWidget(m_regDescEdit, 1);

  // Row 2: Security & Verification properties
  QWidget *regRow2Widget = new QWidget(m_regHeaderWidget);
  regRow2Widget->setObjectName("regRow2Widget");
  QHBoxLayout *regRow2Layout = new QHBoxLayout(regRow2Widget);
  regRow2Layout->setContentsMargins(0, 0, 0, 0);
  regRow2Layout->setSpacing(6);

  QLabel *wrLockTitle = new QLabel(tr("Wr Lock:"), regRow2Widget);
  wrLockTitle->setStyleSheet("font-weight: bold; font-size: 12px;");
  m_regWrLockEdit = new QLineEdit(regRow2Widget);
  m_regWrLockEdit->setObjectName("regWrLockEdit");
  m_regWrLockEdit->setPlaceholderText(tr("Write lock condition..."));
  m_regWrLockEdit->setClearButtonEnabled(true);
  m_regWrLockBtn = new QToolButton(regRow2Widget);
  m_regWrLockBtn->setObjectName("regWrLockBtn");
  m_regWrLockBtn->setText(QStringLiteral("🔒"));
  m_regWrLockBtn->setToolTip(tr("Configure Software Locks"));

  QLabel *rdLockTitle = new QLabel(tr("Rd Lock:"), regRow2Widget);
  rdLockTitle->setStyleSheet("font-weight: bold; font-size: 12px;");
  m_regRdLockEdit = new QLineEdit(regRow2Widget);
  m_regRdLockEdit->setObjectName("regRdLockEdit");
  m_regRdLockEdit->setPlaceholderText(tr("Read lock condition..."));
  m_regRdLockEdit->setClearButtonEnabled(true);
  m_regRdLockBtn = new QToolButton(regRow2Widget);
  m_regRdLockBtn->setObjectName("regRdLockBtn");
  m_regRdLockBtn->setText(QStringLiteral("🔒"));
  m_regRdLockBtn->setToolTip(tr("Configure Software Locks"));

  m_regDecodeOnlyCheck = new QCheckBox(tr("Decode Only"), regRow2Widget);
  m_regDecodeOnlyCheck->setObjectName("regDecodeOnlyCheck");
  m_regDecodeOnlyCheck->setToolTip(
      tr("Address decode only (no internal storage)"));

  m_regHasResetCheck = new QCheckBox(tr("Has Reset"), regRow2Widget);
  m_regHasResetCheck->setObjectName("regHasResetCheck");
  m_regHasResetCheck->setToolTip(tr("Explicit hardware reset value"));

  m_regRandCheck = new QCheckBox(tr("Is Rand"), regRow2Widget);
  m_regRandCheck->setObjectName("regRandCheck");
  m_regRandCheck->setToolTip(tr("UVM constrained-random stimulus"));

  m_regVolatileCheck = new QCheckBox(tr("Volatile"), regRow2Widget);
  m_regVolatileCheck->setObjectName("regVolatileCheck");
  m_regVolatileCheck->setToolTip(
      tr("Hardware can modify value asynchronously"));

  regRow2Layout->addWidget(wrLockTitle);
  regRow2Layout->addWidget(m_regWrLockEdit, 1);
  regRow2Layout->addWidget(m_regWrLockBtn);
  regRow2Layout->addWidget(rdLockTitle);
  regRow2Layout->addWidget(m_regRdLockEdit, 1);
  regRow2Layout->addWidget(m_regRdLockBtn);
  regRow2Layout->addWidget(m_regDecodeOnlyCheck);
  regRow2Layout->addWidget(m_regHasResetCheck);
  regRow2Layout->addWidget(m_regRandCheck);
  regRow2Layout->addWidget(m_regVolatileCheck);

  regHeaderMainLayout->addWidget(regRow1Widget);
  regHeaderMainLayout->addWidget(regRow2Widget);
  regViewLayout->addWidget(m_regHeaderWidget);

  connect(m_regNameEdit, &QLineEdit::editingFinished, this, [this]() {
    if (!m_currentRegItem || m_updatingRegHeader || !m_model)
      return;
    QString newName = m_regNameEdit->text().trimmed();
    QString oldName = m_currentRegItem->data("Name").toString();
    if (newName != oldName && !newName.isEmpty()) {
      int col = m_model->columnOf("Name");
      QModelIndex nameIndex = currentRegSourceIndex(col);
      if (nameIndex.isValid()) {
        m_undoStack->push(
            new EditCellCommand(m_model, nameIndex, oldName, newName));
      }
    }
  });

  connect(m_regOffsetEdit, &QLineEdit::editingFinished, this, [this]() {
    if (!m_currentRegItem || m_updatingRegHeader || !m_model)
      return;
    QString rawOffset = m_regOffsetEdit->text().trimmed();
    QString newOffset = padHexOffsetString(rawOffset);
    m_regOffsetEdit->setText(newOffset);
    QString oldOffset = m_currentRegItem->data("Offset/LSB").toString();
    if (newOffset != oldOffset && !newOffset.isEmpty()) {
      int col = m_model->columnOf("Offset/LSB");
      QModelIndex offsetIndex = currentRegSourceIndex(col);
      if (offsetIndex.isValid()) {
        m_undoStack->push(
            new EditCellCommand(m_model, offsetIndex, oldOffset, newOffset));
      }
    }
  });

  connect(m_regSizeEdit, &QLineEdit::editingFinished, this, [this]() {
    if (!m_currentRegItem || m_updatingRegHeader || !m_model)
      return; // GCOV_EXCL_LINE - Defensive invariant
    QString newSize = m_regSizeEdit->text().trimmed();
    QString oldSize = m_currentRegItem->data("Size/Width").toString();
    if (newSize != oldSize && !newSize.isEmpty()) {
      int col = m_model->columnOf("Size/Width");
      QModelIndex sizeIndex = currentRegSourceIndex(col);
      if (sizeIndex.isValid()) {
        m_undoStack->push(
            new EditCellCommand(m_model, sizeIndex, oldSize, newSize));
      }
    }
  });

  connect(m_regSwAccessCombo, &QComboBox::currentTextChanged, this,
          [this](const QString &newVal) {
            if (!m_currentRegItem || m_updatingRegHeader || !m_model)
              return; // GCOV_EXCL_LINE - Defensive invariant
            QString oldVal = m_currentRegItem->data("SW Access").toString();
            if (newVal != oldVal && !newVal.isEmpty()) {
              int col = m_model->columnOf("SW Access");
              QModelIndex swIndex = currentRegSourceIndex(col);
              if (swIndex.isValid()) {
                m_undoStack->push(
                    new EditCellCommand(m_model, swIndex, oldVal, newVal));
              }
            }
          });

  connect(m_regHwAccessCombo, &QComboBox::currentTextChanged, this,
          [this](const QString &newVal) {
            if (!m_currentRegItem || m_updatingRegHeader || !m_model)
              return; // GCOV_EXCL_LINE - Defensive invariant
            QString oldVal = m_currentRegItem->data("HW Access").toString();
            if (newVal != oldVal && !newVal.isEmpty()) {
              int col = m_model->columnOf("HW Access");
              QModelIndex hwIndex = currentRegSourceIndex(col);
              if (hwIndex.isValid()) {
                m_undoStack->push(
                    new EditCellCommand(m_model, hwIndex, oldVal, newVal));
              }
            }
          });

  connect(m_regResetEdit, &QLineEdit::editingFinished, this, [this]() {
    if (!m_currentRegItem || m_updatingRegHeader || !m_model)
      return; // GCOV_EXCL_LINE - Defensive invariant
    QString rawReset = m_regResetEdit->text().trimmed();
    QString newReset = padHexOffsetString(rawReset);
    m_regResetEdit->setText(newReset);
    QString oldReset = m_currentRegItem->data("Reset Value").toString();
    if (newReset != oldReset && !newReset.isEmpty()) {
      int col = m_model->columnOf("Reset Value");
      QModelIndex resetIndex = currentRegSourceIndex(col);
      if (resetIndex.isValid()) {
        m_undoStack->push(
            new EditCellCommand(m_model, resetIndex, oldReset, newReset));
      }
    }
  });

  connect(m_regDescEdit, &QLineEdit::editingFinished, this, [this]() {
    if (!m_currentRegItem || m_updatingRegHeader || !m_model)
      return;
    QString newDesc = m_regDescEdit->text();
    QString oldDesc = m_currentRegItem->data("Description").toString();
    if (newDesc != oldDesc) {
      int col = m_model ? m_model->columnOf("Description") : 13;
      if (col < 0)
        col = 13; // GCOV_EXCL_LINE - Defensive column fallback
      QModelIndex descIndex = currentRegSourceIndex(col);
      if (descIndex.isValid()) {
        m_undoStack->push(
            new EditCellCommand(m_model, descIndex, oldDesc, newDesc));
      }
    }
  });

  connect(m_regWrLockEdit, &QLineEdit::editingFinished, this, [this]() {
    if (!m_currentRegItem || m_updatingRegHeader || !m_model)
      return; // GCOV_EXCL_LINE - Defensive invariant
    QString newVal = m_regWrLockEdit->text().trimmed();
    QString oldVal = m_currentRegItem->data("Write Lock").toString();
    if (newVal != oldVal) {
      int col = m_model->columnOf("Write Lock");
      QModelIndex idx = currentRegSourceIndex(col);
      if (idx.isValid()) {
        m_undoStack->push(new EditCellCommand(m_model, idx, oldVal, newVal));
      }
    }
  });

  connect(m_regRdLockEdit, &QLineEdit::editingFinished, this, [this]() {
    if (!m_currentRegItem || m_updatingRegHeader || !m_model)
      return; // GCOV_EXCL_LINE - Defensive invariant
    QString newVal = m_regRdLockEdit->text().trimmed();
    QString oldVal = m_currentRegItem->data("Read Lock").toString();
    if (newVal != oldVal) {
      int col = m_model->columnOf("Read Lock");
      QModelIndex idx = currentRegSourceIndex(col);
      if (idx.isValid()) {
        m_undoStack->push(new EditCellCommand(m_model, idx, oldVal, newVal));
      }
    }
  });

  auto openLockDialogForReg = [this]() {
    if (!m_currentRegItem || !m_model)
      return; // GCOV_EXCL_LINE - Defensive invariant
    QString wrExpr = m_currentRegItem->data("Write Lock").toString();
    QString rdExpr = m_currentRegItem->data("Read Lock").toString();
    QString name = m_currentRegItem->data("Name").toString();
    RegLockDialog dlg(wrExpr, rdExpr, m_model, name, this);
    if (dlg.exec() == QDialog::Accepted) {
      QString newWr = dlg.writeExpression();
      QString newRd = dlg.readExpression();
      int wrCol = m_model->columnOf("Write Lock");
      int rdCol = m_model->columnOf("Read Lock");
      if (newWr != wrExpr) {
        QModelIndex wrIdx = currentRegSourceIndex(wrCol);
        if (wrIdx.isValid())
          m_undoStack->push(new EditCellCommand(m_model, wrIdx, wrExpr, newWr));
      }
      if (newRd != rdExpr) {
        QModelIndex rdIdx = currentRegSourceIndex(rdCol);
        if (rdIdx.isValid())
          m_undoStack->push(new EditCellCommand(m_model, rdIdx, rdExpr, newRd));
      }
    }
  };
  connect(m_regWrLockBtn, &QToolButton::clicked, this, openLockDialogForReg);
  connect(m_regRdLockBtn, &QToolButton::clicked, this, openLockDialogForReg);

  connect(m_regDecodeOnlyCheck, &QCheckBox::toggled, this,
          [this](bool checked) {
            if (!m_currentRegItem || m_updatingRegHeader || !m_model)
              return;
            QString newVal =
                checked ? QStringLiteral("true") : QStringLiteral("false");
            QString oldVal =
                m_currentRegItem->data("Decode Only").toString().toLower();
            if (newVal != oldVal) {
              int col = m_model->columnOf("Decode Only");
              QModelIndex idx = currentRegSourceIndex(col);
              if (idx.isValid()) {
                m_undoStack->push(
                    new EditCellCommand(m_model, idx, oldVal, newVal));
              }
            }
          });

  connect(m_regHasResetCheck, &QCheckBox::toggled, this, [this](bool checked) {
    if (!m_currentRegItem || m_updatingRegHeader || !m_model)
      return; // GCOV_EXCL_LINE - Defensive invariant
    QString newVal = checked ? QStringLiteral("true") : QStringLiteral("false");
    QString oldVal = m_currentRegItem->data("Has Reset").toString().toLower();
    if (newVal != oldVal) {
      int col = m_model->columnOf("Has Reset");
      QModelIndex idx = currentRegSourceIndex(col);
      if (idx.isValid()) {
        m_undoStack->push(new EditCellCommand(m_model, idx, oldVal, newVal));
      }
    }
  });

  connect(m_regRandCheck, &QCheckBox::toggled, this, [this](bool checked) {
    if (!m_currentRegItem || m_updatingRegHeader || !m_model)
      return; // GCOV_EXCL_LINE - Defensive invariant
    QString newVal = checked ? QStringLiteral("true") : QStringLiteral("false");
    QString oldVal = m_currentRegItem->data("Is Rand").toString().toLower();
    if (newVal != oldVal) {
      int col = m_model->columnOf("Is Rand");
      QModelIndex idx = currentRegSourceIndex(col);
      if (idx.isValid()) {
        m_undoStack->push(new EditCellCommand(m_model, idx, oldVal, newVal));
      }
    }
  });

  connect(m_regVolatileCheck, &QCheckBox::toggled, this, [this](bool checked) {
    if (!m_currentRegItem || m_updatingRegHeader || !m_model)
      return; // GCOV_EXCL_LINE - Defensive invariant
    QString newVal = checked ? QStringLiteral("true") : QStringLiteral("false");
    QString oldVal = m_currentRegItem->data("Volatile").toString().toLower();
    if (newVal != oldVal) {
      int col = m_model->columnOf("Volatile");
      QModelIndex idx = currentRegSourceIndex(col);
      if (idx.isValid()) {
        m_undoStack->push(new EditCellCommand(m_model, idx, oldVal, newVal));
      }
    }
  });

  m_bitfieldBar = new RegBitfieldBarWidget(m_regViewWidget);
  m_bitfieldBar->setObjectName("bitfieldBar");
  regViewLayout->addWidget(m_bitfieldBar);

  // Tab Bar for Detail Views
  m_regTabBar = new QTabBar(m_regViewWidget);
  m_regTabBar->setObjectName("regTabBar");
  m_regTabBar->setDocumentMode(true);
  m_regTabBar->setExpanding(false);
  m_regTabBar->addTab(tr("Fields & Layout"));
  m_regTabBar->addTab(tr("Verification & UVM"));
  m_regTabBar->addTab(tr("Security & Locks"));
  m_regTabBar->addTab(tr("Memory Map"));
  m_regTabBar->addTab(tr("All Properties"));
  regViewLayout->addWidget(m_regTabBar);

  connect(m_regTabBar, &QTabBar::currentChanged, this,
          [this](int index) { applyFieldTabColumnFilter(index); });

  // Tab Stack: Page 0: Fields Table, Page 1: Memory Map
  m_regTabStack = new QStackedWidget(m_regViewWidget);
  m_regTabStack->setObjectName("regTabStack");

  m_fieldsTableView = new QTableView(m_regTabStack);
  m_fieldsTableView->setObjectName("fieldsTableView");
  m_fieldsTableView->setAlternatingRowColors(true);
  m_fieldsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_fieldsTableView->setEditTriggers(QAbstractItemView::AllEditTriggers);
  m_fieldsTableView->setModel(m_fieldProxy);
  m_fieldsTableView->setSortingEnabled(true);
  m_fieldsTableView->sortByColumn(1, Qt::AscendingOrder);
  m_fieldsTableView->horizontalHeader()->setStretchLastSection(true);
  m_fieldsTableView->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Interactive);
  int fldDescCol = m_model ? m_model->columnOf("Description") : 13;
  if (fldDescCol < 0)
    fldDescCol = 13; // GCOV_EXCL_LINE - Defensive column fallback
  m_fieldsTableView->horizontalHeader()->setSectionResizeMode(
      fldDescCol, QHeaderView::Stretch);

  connect(m_fieldsTableView->horizontalHeader(), &QHeaderView::sectionResized,
          this, [this](int logicalIndex, int /*oldSize*/, int newSize) {
            if (!m_fieldsTableView->horizontalHeader())
              return; // GCOV_EXCL_LINE - Defensive header check
            int minHeaderSize =
                m_fieldsTableView->horizontalHeader()->sectionSizeHint(
                    logicalIndex);
            if (newSize < minHeaderSize) {
              m_fieldsTableView->horizontalHeader()->resizeSection(
                  logicalIndex, minHeaderSize);
            }
          });

  // Set field table delegates
  m_fieldsTableView->setItemDelegateForColumn(
      1, new RegHexDecBinDelegate(this)); // LSB
  m_fieldsTableView->setItemDelegateForColumn(
      2, new RegHexDecBinDelegate(this)); // Size
  m_fieldsTableView->setItemDelegateForColumn(3,
                                              new RegStrDelegate(this)); // Name
  m_fieldsTableView->setItemDelegateForColumn(
      4, new RegAccessPolicyDelegate(this)); // SW Access
  m_fieldsTableView->setItemDelegateForColumn(
      5, new RegHwAccessDelegate(this)); // HW Access Policy
  m_fieldsTableView->setItemDelegateForColumn(
      6, new RegHexDecBinDelegate(this)); // Reset Value
  m_fieldsTableView->setItemDelegateForColumn(
      7, new RegBoolDelegate(this)); // Is Rand
  m_fieldsTableView->setItemDelegateForColumn(
      8, new RegBoolDelegate(this)); // Volatile
  m_fieldsTableView->setItemDelegateForColumn(
      9, new RegBoolDelegate(this)); // Has Reset
  m_fieldsTableView->setItemDelegateForColumn(
      10, new RegLockDelegate(m_model, this)); // Write Lock
  m_fieldsTableView->setItemDelegateForColumn(
      11, new RegLockDelegate(m_model, this)); // Read Lock
  m_fieldsTableView->setItemDelegateForColumn(
      12, new RegBoolDelegate(this)); // Decode Only
  m_fieldsTableView->setItemDelegateForColumn(
      13, new RegMapDelegate(this)); // Description
  m_fieldsTableView->setColumnHidden(0, true);

  m_regMapScrollArea = new QScrollArea(m_regTabStack);
  m_regMapScrollArea->setObjectName("regMapScrollArea");
  m_regMapScrollArea->setWidgetResizable(true);
  m_regMapScrollArea->setFrameShape(QFrame::StyledPanel);

  m_regBlockMemoryMapWidget = new BlockMemoryMapWidget(m_regMapScrollArea);
  m_regBlockMemoryMapWidget->setObjectName("regBlockMemoryMapWidget");
  m_regMapScrollArea->setWidget(m_regBlockMemoryMapWidget);

  connect(m_regBlockMemoryMapWidget, &BlockMemoryMapWidget::registerClicked,
          this, [this](int childRow, RegMapTreeItem *regItem) {
            navigateToRegister(childRow, regItem);
          });

  m_regTabStack->addWidget(m_fieldsTableView);
  m_regTabStack->addWidget(m_regMapScrollArea);
  regViewLayout->addWidget(m_regTabStack, 1);

  m_rightStackedWidget->addWidget(m_regViewWidget);

  // ==========================================
  // Page 1: Block View (Header + Memory Map Diagram)
  // ==========================================
  m_blockViewWidget = new QWidget(m_rightStackedWidget);
  m_blockViewWidget->setObjectName("blockViewWidget");
  QVBoxLayout *blockViewLayout = new QVBoxLayout(m_blockViewWidget);
  blockViewLayout->setContentsMargins(0, 0, 0, 0);
  blockViewLayout->setSpacing(6);

  m_blockHeaderWidget = new QWidget(m_blockViewWidget);
  m_blockHeaderWidget->setObjectName("blockHeaderWidget");
  QVBoxLayout *blkHeaderMainLayout = new QVBoxLayout(m_blockHeaderWidget);
  blkHeaderMainLayout->setContentsMargins(4, 2, 4, 2);
  blkHeaderMainLayout->setSpacing(4);

  QWidget *blkRow1Widget = new QWidget(m_blockHeaderWidget);
  blkRow1Widget->setObjectName("blkRow1Widget");
  QHBoxLayout *blkRow1Layout = new QHBoxLayout(blkRow1Widget);
  blkRow1Layout->setContentsMargins(0, 0, 0, 0);
  blkRow1Layout->setSpacing(6);

  QLabel *blkNameTitle = new QLabel(tr("Name:"), blkRow1Widget);
  blkNameTitle->setStyleSheet("font-weight: bold; font-size: 12px;");

  m_blkNameEdit = new QLineEdit(blkRow1Widget);
  m_blkNameEdit->setObjectName("blkNameEdit");
  m_blkNameEdit->setPlaceholderText(tr("Block name..."));
  m_blkNameEdit->setClearButtonEnabled(true);
  m_blkNameEdit->setMinimumWidth(
      std::max(120, blkNameTitle->sizeHint().width()));

  QLabel *blkOffsetTitle = new QLabel(tr("Offset:"), blkRow1Widget);
  blkOffsetTitle->setStyleSheet("font-weight: bold; font-size: 12px;");

  m_blkOffsetEdit = new QLineEdit(blkRow1Widget);
  m_blkOffsetEdit->setObjectName("blkOffsetEdit");
  m_blkOffsetEdit->setPlaceholderText(tr("0x0000"));
  m_blkOffsetEdit->setStyleSheet("font-family: monospace; font-size: 12px;");
  m_blkOffsetEdit->setClearButtonEnabled(true);
  m_blkOffsetEdit->setMinimumWidth(blkOffsetTitle->sizeHint().width());
  m_blkOffsetEdit->setMaximumWidth(100);

  QLabel *blkDescTitle = new QLabel(tr("Description:"), blkRow1Widget);
  blkDescTitle->setStyleSheet("font-weight: bold; font-size: 12px;");

  m_blkDescEdit = new QLineEdit(blkRow1Widget);
  m_blkDescEdit->setObjectName("blkDescEdit");
  m_blkDescEdit->setPlaceholderText(tr("Block description..."));
  m_blkDescEdit->setClearButtonEnabled(true);
  m_blkDescEdit->setMinimumWidth(blkDescTitle->sizeHint().width());

  blkRow1Layout->addWidget(blkNameTitle);
  blkRow1Layout->addWidget(m_blkNameEdit);
  blkRow1Layout->addWidget(blkOffsetTitle);
  blkRow1Layout->addWidget(m_blkOffsetEdit);
  blkRow1Layout->addWidget(blkDescTitle);
  blkRow1Layout->addWidget(m_blkDescEdit, 1);

  QWidget *blkRow2Widget = new QWidget(m_blockHeaderWidget);
  blkRow2Widget->setObjectName("blkRow2Widget");
  QHBoxLayout *blkRow2Layout = new QHBoxLayout(blkRow2Widget);
  blkRow2Layout->setContentsMargins(0, 0, 0, 0);
  blkRow2Layout->setSpacing(6);

  QLabel *blkWrLockTitle = new QLabel(tr("Wr Lock:"), blkRow2Widget);
  blkWrLockTitle->setStyleSheet("font-weight: bold; font-size: 12px;");
  m_blkWrLockEdit = new QLineEdit(blkRow2Widget);
  m_blkWrLockEdit->setObjectName("blkWrLockEdit");
  m_blkWrLockEdit->setPlaceholderText(
      tr("Block-level write lock condition..."));
  m_blkWrLockEdit->setClearButtonEnabled(true);
  m_blkWrLockBtn = new QToolButton(blkRow2Widget);
  m_blkWrLockBtn->setObjectName("blkWrLockBtn");
  m_blkWrLockBtn->setText(QStringLiteral("🔒"));
  m_blkWrLockBtn->setToolTip(tr("Configure Block Software Locks"));

  QLabel *blkRdLockTitle = new QLabel(tr("Rd Lock:"), blkRow2Widget);
  blkRdLockTitle->setStyleSheet("font-weight: bold; font-size: 12px;");
  m_blkRdLockEdit = new QLineEdit(blkRow2Widget);
  m_blkRdLockEdit->setObjectName("blkRdLockEdit");
  m_blkRdLockEdit->setPlaceholderText(tr("Block-level read lock condition..."));
  m_blkRdLockEdit->setClearButtonEnabled(true);
  m_blkRdLockBtn = new QToolButton(blkRow2Widget);
  m_blkRdLockBtn->setObjectName("blkRdLockBtn");
  m_blkRdLockBtn->setText(QStringLiteral("🔒"));
  m_blkRdLockBtn->setToolTip(tr("Configure Block Software Locks"));

  blkRow2Layout->addWidget(blkWrLockTitle);
  blkRow2Layout->addWidget(m_blkWrLockEdit, 1);
  blkRow2Layout->addWidget(m_blkWrLockBtn);
  blkRow2Layout->addWidget(blkRdLockTitle);
  blkRow2Layout->addWidget(m_blkRdLockEdit, 1);
  blkRow2Layout->addWidget(m_blkRdLockBtn);

  blkHeaderMainLayout->addWidget(blkRow1Widget);
  blkHeaderMainLayout->addWidget(blkRow2Widget);

  blockViewLayout->addWidget(m_blockHeaderWidget);

  connect(m_blkNameEdit, &QLineEdit::editingFinished, this, [this]() {
    if (!m_currentBlkItem || m_updatingBlkHeader || !m_model)
      return;
    QString newName = m_blkNameEdit->text().trimmed();
    QString oldName = m_currentBlkItem->data("Name").toString();
    if (newName != oldName && !newName.isEmpty()) {
      int col = m_model->columnOf("Name");
      QModelIndex nameIndex = currentBlkSourceIndex(col);
      if (nameIndex.isValid()) {
        m_undoStack->push(
            new EditCellCommand(m_model, nameIndex, oldName, newName));
      }
    }
  });

  connect(m_blkOffsetEdit, &QLineEdit::editingFinished, this, [this]() {
    if (!m_currentBlkItem || m_updatingBlkHeader || !m_model)
      return;
    QString rawOffset = m_blkOffsetEdit->text().trimmed();
    QString newOffset = padHexOffsetString(rawOffset);
    m_blkOffsetEdit->setText(newOffset);
    QString oldOffset = m_currentBlkItem->data("Offset/LSB").toString();
    if (newOffset != oldOffset && !newOffset.isEmpty()) {
      int col = m_model->columnOf("Offset/LSB");
      QModelIndex offsetIndex = currentBlkSourceIndex(col);
      if (offsetIndex.isValid()) {
        m_undoStack->push(
            new EditCellCommand(m_model, offsetIndex, oldOffset, newOffset));
      }
    }
  });

  connect(m_blkDescEdit, &QLineEdit::editingFinished, this, [this]() {
    if (!m_currentBlkItem || m_updatingBlkHeader || !m_model)
      return;
    QString newDesc = m_blkDescEdit->text();
    QString oldDesc = m_currentBlkItem->data("Description").toString();
    if (newDesc != oldDesc) {
      int descCol = m_model ? m_model->columnOf("Description") : 13;
      if (descCol < 0)
        descCol = 13; // GCOV_EXCL_LINE - Defensive column fallback
      QModelIndex descIndex = currentBlkSourceIndex(descCol);
      if (descIndex.isValid()) {
        m_undoStack->push(
            new EditCellCommand(m_model, descIndex, oldDesc, newDesc));
      }
    }
  });

  connect(m_blkWrLockEdit, &QLineEdit::editingFinished, this, [this]() {
    if (!m_currentBlkItem || m_updatingBlkHeader || !m_model)
      return; // GCOV_EXCL_LINE - Defensive invariant
    QString newVal = m_blkWrLockEdit->text().trimmed();
    QString oldVal = m_currentBlkItem->data("Write Lock").toString();
    if (newVal != oldVal) {
      int col = m_model->columnOf("Write Lock");
      QModelIndex idx = currentBlkSourceIndex(col);
      if (idx.isValid()) {
        m_undoStack->push(new EditCellCommand(m_model, idx, oldVal, newVal));
      }
    }
  });

  connect(m_blkRdLockEdit, &QLineEdit::editingFinished, this, [this]() {
    if (!m_currentBlkItem || m_updatingBlkHeader || !m_model)
      return; // GCOV_EXCL_LINE - Defensive invariant
    QString newVal = m_blkRdLockEdit->text().trimmed();
    QString oldVal = m_currentBlkItem->data("Read Lock").toString();
    if (newVal != oldVal) {
      int col = m_model->columnOf("Read Lock");
      QModelIndex idx = currentBlkSourceIndex(col);
      if (idx.isValid()) {
        m_undoStack->push(new EditCellCommand(m_model, idx, oldVal, newVal));
      }
    }
  });

  auto openLockDialogForBlk = [this]() {
    if (!m_currentBlkItem || !m_model)
      return; // GCOV_EXCL_LINE - Defensive invariant
    QString wrExpr = m_currentBlkItem->data("Write Lock").toString();
    QString rdExpr = m_currentBlkItem->data("Read Lock").toString();
    QString name = m_currentBlkItem->data("Name").toString();
    RegLockDialog dlg(wrExpr, rdExpr, m_model, name, this);
    if (dlg.exec() == QDialog::Accepted) {
      QString newWr = dlg.writeExpression();
      QString newRd = dlg.readExpression();
      int wrCol = m_model->columnOf("Write Lock");
      int rdCol = m_model->columnOf("Read Lock");
      if (newWr != wrExpr) {
        QModelIndex wrIdx = currentBlkSourceIndex(wrCol);
        if (wrIdx.isValid())
          m_undoStack->push(new EditCellCommand(m_model, wrIdx, wrExpr, newWr));
      }
      if (newRd != rdExpr) {
        QModelIndex rdIdx = currentBlkSourceIndex(rdCol);
        if (rdIdx.isValid())
          m_undoStack->push(new EditCellCommand(m_model, rdIdx, rdExpr, newRd));
      }
    }
  };
  connect(m_blkWrLockBtn, &QToolButton::clicked, this, openLockDialogForBlk);
  connect(m_blkRdLockBtn, &QToolButton::clicked, this, openLockDialogForBlk);

  m_mapScrollArea = new QScrollArea(m_blockViewWidget);
  m_mapScrollArea->setObjectName("mapScrollArea");
  m_mapScrollArea->setWidgetResizable(true);
  m_mapScrollArea->setFrameShape(QFrame::StyledPanel);

  m_blockMemoryMapWidget = new BlockMemoryMapWidget(m_mapScrollArea);
  m_blockMemoryMapWidget->setObjectName("blockMemoryMapWidget");
  m_mapScrollArea->setWidget(m_blockMemoryMapWidget);
  blockViewLayout->addWidget(m_mapScrollArea);

  m_rightStackedWidget->addWidget(m_blockViewWidget);

  // ==========================================
  // Page 2: Memory View (Header + Memory Summary)
  // ==========================================
  m_memViewWidget = new QWidget(m_rightStackedWidget);
  m_memViewWidget->setObjectName("memViewWidget");
  QVBoxLayout *memViewLayout = new QVBoxLayout(m_memViewWidget);
  memViewLayout->setContentsMargins(0, 0, 0, 0);
  memViewLayout->setSpacing(6);

  m_memHeaderWidget = new QWidget(m_memViewWidget);
  m_memHeaderWidget->setObjectName("memHeaderWidget");
  QVBoxLayout *memHeaderMainLayout = new QVBoxLayout(m_memHeaderWidget);
  memHeaderMainLayout->setContentsMargins(4, 2, 4, 2);
  memHeaderMainLayout->setSpacing(4);

  // Row 1: Name, Offset, Size, SW Access, HW Access
  QWidget *memRow1Widget = new QWidget(m_memHeaderWidget);
  memRow1Widget->setObjectName("memRow1Widget");
  QHBoxLayout *memRow1Layout = new QHBoxLayout(memRow1Widget);
  memRow1Layout->setContentsMargins(0, 0, 0, 0);
  memRow1Layout->setSpacing(6);

  QLabel *memNameTitle = new QLabel(tr("Name:"), memRow1Widget);
  memNameTitle->setStyleSheet("font-weight: bold; font-size: 12px;");
  m_memNameEdit = new QLineEdit(memRow1Widget);
  m_memNameEdit->setObjectName("memNameEdit");
  m_memNameEdit->setPlaceholderText(tr("Memory name..."));
  m_memNameEdit->setClearButtonEnabled(true);
  m_memNameEdit->setMinimumWidth(
      std::max(120, memNameTitle->sizeHint().width()));

  QLabel *memOffsetTitle = new QLabel(tr("Offset:"), memRow1Widget);
  memOffsetTitle->setStyleSheet("font-weight: bold; font-size: 12px;");
  m_memOffsetEdit = new QLineEdit(memRow1Widget);
  m_memOffsetEdit->setObjectName("memOffsetEdit");
  m_memOffsetEdit->setPlaceholderText(tr("0x0000"));
  m_memOffsetEdit->setStyleSheet("font-family: monospace; font-size: 12px;");
  m_memOffsetEdit->setClearButtonEnabled(true);
  m_memOffsetEdit->setMinimumWidth(memOffsetTitle->sizeHint().width());
  m_memOffsetEdit->setMaximumWidth(100);

  QLabel *memSizeTitle = new QLabel(tr("Size (Bytes):"), memRow1Widget);
  memSizeTitle->setStyleSheet("font-weight: bold; font-size: 12px;");
  m_memSizeEdit = new QLineEdit(memRow1Widget);
  m_memSizeEdit->setObjectName("memSizeEdit");
  m_memSizeEdit->setPlaceholderText(tr("1024 or 0x400"));
  m_memSizeEdit->setStyleSheet("font-family: monospace; font-size: 12px;");
  m_memSizeEdit->setClearButtonEnabled(true);
  m_memSizeEdit->setMinimumWidth(memSizeTitle->sizeHint().width());
  m_memSizeEdit->setMaximumWidth(110);

  QLabel *memSwAccessTitle = new QLabel(tr("SW Access:"), memRow1Widget);
  memSwAccessTitle->setStyleSheet("font-weight: bold; font-size: 12px;");
  m_memSwAccessCombo = new QComboBox(memRow1Widget);
  m_memSwAccessCombo->setObjectName("memSwAccessCombo");
  m_memSwAccessCombo->addItems({"RW", "RO", "WO", "NOACCESS"});

  QLabel *memHwAccessTitle = new QLabel(tr("HW Access:"), memRow1Widget);
  memHwAccessTitle->setStyleSheet("font-weight: bold; font-size: 12px;");
  m_memHwAccessCombo = new QComboBox(memRow1Widget);
  m_memHwAccessCombo->setObjectName("memHwAccessCombo");
  m_memHwAccessCombo->addItems({"RW", "RO", "NA"});

  memRow1Layout->addWidget(memNameTitle);
  memRow1Layout->addWidget(m_memNameEdit);
  memRow1Layout->addWidget(memOffsetTitle);
  memRow1Layout->addWidget(m_memOffsetEdit);
  memRow1Layout->addWidget(memSizeTitle);
  memRow1Layout->addWidget(m_memSizeEdit);
  memRow1Layout->addWidget(memSwAccessTitle);
  memRow1Layout->addWidget(m_memSwAccessCombo);
  memRow1Layout->addWidget(memHwAccessTitle);
  memRow1Layout->addWidget(m_memHwAccessCombo);

  // Row 2: Word Width, Depth, HDL Path, Description
  QWidget *memRow2Widget = new QWidget(m_memHeaderWidget);
  memRow2Widget->setObjectName("memRow2Widget");
  QHBoxLayout *memRow2Layout = new QHBoxLayout(memRow2Widget);
  memRow2Layout->setContentsMargins(0, 0, 0, 0);
  memRow2Layout->setSpacing(6);

  QLabel *memWwTitle = new QLabel(tr("Word Width:"), memRow2Widget);
  memWwTitle->setStyleSheet("font-weight: bold; font-size: 12px;");
  m_memWordWidthEdit = new QLineEdit(memRow2Widget);
  m_memWordWidthEdit->setObjectName("memWordWidthEdit");
  m_memWordWidthEdit->setPlaceholderText(tr("32"));
  m_memWordWidthEdit->setMaximumWidth(70);

  QLabel *memDepthTitle = new QLabel(tr("Depth:"), memRow2Widget);
  memDepthTitle->setStyleSheet("font-weight: bold; font-size: 12px;");
  m_memDepthEdit = new QLineEdit(memRow2Widget);
  m_memDepthEdit->setObjectName("memDepthEdit");
  m_memDepthEdit->setPlaceholderText(tr("256"));
  m_memDepthEdit->setMaximumWidth(90);

  QLabel *memHdlTitle = new QLabel(tr("HDL Path:"), memRow2Widget);
  memHdlTitle->setStyleSheet("font-weight: bold; font-size: 12px;");
  m_memHdlPathEdit = new QLineEdit(memRow2Widget);
  m_memHdlPathEdit->setObjectName("memHdlPathEdit");
  m_memHdlPathEdit->setPlaceholderText(tr("e.g. sram_inst"));
  m_memHdlPathEdit->setClearButtonEnabled(true);
  m_memHdlPathEdit->setMinimumWidth(140);

  QLabel *memDescTitle = new QLabel(tr("Description:"), memRow2Widget);
  memDescTitle->setStyleSheet("font-weight: bold; font-size: 12px;");
  m_memDescEdit = new QLineEdit(memRow2Widget);
  m_memDescEdit->setObjectName("memDescEdit");
  m_memDescEdit->setPlaceholderText(tr("Memory description..."));
  m_memDescEdit->setClearButtonEnabled(true);

  memRow2Layout->addWidget(memWwTitle);
  memRow2Layout->addWidget(m_memWordWidthEdit);
  memRow2Layout->addWidget(memDepthTitle);
  memRow2Layout->addWidget(m_memDepthEdit);
  memRow2Layout->addWidget(memHdlTitle);
  memRow2Layout->addWidget(m_memHdlPathEdit);
  memRow2Layout->addWidget(memDescTitle);
  memRow2Layout->addWidget(m_memDescEdit, 1);

  // Row 3: Wr Lock, Rd Lock, UVM test exclusions
  QWidget *memRow3Widget = new QWidget(m_memHeaderWidget);
  memRow3Widget->setObjectName("memRow3Widget");
  QHBoxLayout *memRow3Layout = new QHBoxLayout(memRow3Widget);
  memRow3Layout->setContentsMargins(0, 0, 0, 0);
  memRow3Layout->setSpacing(6);

  QLabel *memWrLockTitle = new QLabel(tr("Wr Lock:"), memRow3Widget);
  memWrLockTitle->setStyleSheet("font-weight: bold; font-size: 12px;");
  m_memWrLockEdit = new QLineEdit(memRow3Widget);
  m_memWrLockEdit->setObjectName("memWrLockEdit");
  m_memWrLockEdit->setPlaceholderText(tr("Memory write lock condition..."));
  m_memWrLockEdit->setClearButtonEnabled(true);
  m_memWrLockBtn = new QToolButton(memRow3Widget);
  m_memWrLockBtn->setObjectName("memWrLockBtn");
  m_memWrLockBtn->setText(QStringLiteral("🔒"));
  m_memWrLockBtn->setToolTip(tr("Configure Memory Software Locks"));

  QLabel *memRdLockTitle = new QLabel(tr("Rd Lock:"), memRow3Widget);
  memRdLockTitle->setStyleSheet("font-weight: bold; font-size: 12px;");
  m_memRdLockEdit = new QLineEdit(memRow3Widget);
  m_memRdLockEdit->setObjectName("memRdLockEdit");
  m_memRdLockEdit->setPlaceholderText(tr("Memory read lock condition..."));
  m_memRdLockEdit->setClearButtonEnabled(true);
  m_memRdLockBtn = new QToolButton(memRow3Widget);
  m_memRdLockBtn->setObjectName("memRdLockBtn");
  m_memRdLockBtn->setText(QStringLiteral("🔒"));
  m_memRdLockBtn->setToolTip(tr("Configure Memory Software Locks"));

  m_memNoTestCheck = new QCheckBox(tr("No Mem Test"), memRow3Widget);
  m_memNoTestCheck->setObjectName("memNoTestCheck");
  m_memNoTestCheck->setToolTip(
      tr("Exclude memory from all UVM memory tests (NO_MEM_TEST)"));

  m_memNoWalkTestCheck = new QCheckBox(tr("No Walk Test"), memRow3Widget);
  m_memNoWalkTestCheck->setObjectName("memNoWalkTestCheck");
  m_memNoWalkTestCheck->setToolTip(
      tr("Exclude memory from UVM walk test (NO_MEM_WALK_TEST)"));

  m_memNoAccessTestCheck = new QCheckBox(tr("No Access Test"), memRow3Widget);
  m_memNoAccessTestCheck->setObjectName("memNoAccessTestCheck");
  m_memNoAccessTestCheck->setToolTip(
      tr("Exclude memory from UVM access test (NO_MEM_ACCESS_TEST)"));

  memRow3Layout->addWidget(memWrLockTitle);
  memRow3Layout->addWidget(m_memWrLockEdit, 1);
  memRow3Layout->addWidget(m_memWrLockBtn);
  memRow3Layout->addWidget(memRdLockTitle);
  memRow3Layout->addWidget(m_memRdLockEdit, 1);
  memRow3Layout->addWidget(m_memRdLockBtn);
  memRow3Layout->addWidget(m_memNoTestCheck);
  memRow3Layout->addWidget(m_memNoWalkTestCheck);
  memRow3Layout->addWidget(m_memNoAccessTestCheck);

  memHeaderMainLayout->addWidget(memRow1Widget);
  memHeaderMainLayout->addWidget(memRow2Widget);
  memHeaderMainLayout->addWidget(memRow3Widget);

  memViewLayout->addWidget(m_memHeaderWidget);

  // Connect editing signals for memory fields
  connect(m_memNameEdit, &QLineEdit::editingFinished, this, [this]() {
    if (!m_currentMemItem || m_updatingMemHeader || !m_model)
      return; // GCOV_EXCL_LINE - Defensive invariant
    QString newName = m_memNameEdit->text().trimmed();
    QString oldName = m_currentMemItem->data("Name").toString();
    if (newName != oldName && !newName.isEmpty()) {
      int col = m_model->columnOf("Name");
      QModelIndex nameIndex = currentMemSourceIndex(col);
      if (nameIndex.isValid()) {
        m_undoStack->push(
            new EditCellCommand(m_model, nameIndex, oldName, newName));
      }
    }
  });

  connect(m_memOffsetEdit, &QLineEdit::editingFinished, this, [this]() {
    if (!m_currentMemItem || m_updatingMemHeader || !m_model)
      return; // GCOV_EXCL_LINE - Defensive invariant
    QString rawOffset = m_memOffsetEdit->text().trimmed();
    QString newOffset = padHexOffsetString(rawOffset);
    m_memOffsetEdit->setText(newOffset);
    QString oldOffset = m_currentMemItem->data("Offset/LSB").toString();
    if (newOffset != oldOffset && !newOffset.isEmpty()) {
      int col = m_model->columnOf("Offset/LSB");
      QModelIndex offsetIndex = currentMemSourceIndex(col);
      if (offsetIndex.isValid()) {
        m_undoStack->push(
            new EditCellCommand(m_model, offsetIndex, oldOffset, newOffset));
      }
    }
  });

  connect(m_memSizeEdit, &QLineEdit::editingFinished, this, [this]() {
    if (!m_currentMemItem || m_updatingMemHeader || !m_model)
      return; // GCOV_EXCL_LINE - Defensive invariant
    QString newSize = m_memSizeEdit->text().trimmed();
    QString oldSize = m_currentMemItem->data("Size/Width").toString();
    if (newSize != oldSize && !newSize.isEmpty()) {
      int col = m_model->columnOf("Size/Width");
      if (col < 0)
        col = 2; // GCOV_EXCL_LINE - Defensive column fallback
      QModelIndex sizeIndex = currentMemSourceIndex(col);
      if (sizeIndex.isValid()) {
        m_undoStack->push(
            new EditCellCommand(m_model, sizeIndex, oldSize, newSize));
      }
    }
  });

  connect(m_memSwAccessCombo, &QComboBox::currentTextChanged, this,
          [this](const QString &newVal) {
            if (!m_currentMemItem || m_updatingMemHeader || !m_model)
              return; // GCOV_EXCL_LINE - Defensive invariant
            QString oldVal = m_currentMemItem->data("SW Access").toString();
            if (newVal != oldVal) {
              int col = m_model->columnOf("SW Access");
              if (col < 0)
                col = 4; // GCOV_EXCL_LINE - Defensive column fallback
              QModelIndex swIndex = currentMemSourceIndex(col);
              if (swIndex.isValid()) {
                m_undoStack->push(
                    new EditCellCommand(m_model, swIndex, oldVal, newVal));
              }
            }
          });

  connect(m_memHwAccessCombo, &QComboBox::currentTextChanged, this,
          [this](const QString &newVal) {
            if (!m_currentMemItem || m_updatingMemHeader || !m_model)
              return; // GCOV_EXCL_LINE - Defensive invariant
            QString oldVal = m_currentMemItem->data("HW Access").toString();
            if (oldVal.isEmpty())
              oldVal = m_currentMemItem->data("HW Access Policy").toString(); // GCOV_EXCL_LINE - Defensive fallback
            if (newVal != oldVal) {
              int col = m_model->columnOf("HW Access");
              // GCOV_EXCL_START - Defensive column fallback
              if (col < 0)
                col = m_model->columnOf("HW Access Policy");
              if (col < 0)
                col = 5;
              // GCOV_EXCL_STOP
              QModelIndex hwIndex = currentMemSourceIndex(col);
              if (hwIndex.isValid()) {
                m_undoStack->push(
                    new EditCellCommand(m_model, hwIndex, oldVal, newVal));
              }
            }
          });

  connect(m_memDescEdit, &QLineEdit::editingFinished, this, [this]() {
    if (!m_currentMemItem || m_updatingMemHeader || !m_model)
      return; // GCOV_EXCL_LINE - Defensive invariant
    QString newDesc = m_memDescEdit->text();
    QString oldDesc = m_currentMemItem->data("Description").toString();
    if (newDesc != oldDesc) {
      int descCol = m_model ? m_model->columnOf("Description") : 13;
      if (descCol < 0)
        descCol = 13; // GCOV_EXCL_LINE - Defensive column fallback
      QModelIndex descIndex = currentMemSourceIndex(descCol);
      if (descIndex.isValid()) {
        m_undoStack->push(
            new EditCellCommand(m_model, descIndex, oldDesc, newDesc));
      }
    }
  });

  connect(m_memWordWidthEdit, &QLineEdit::editingFinished, this, [this]() {
    if (!m_currentMemItem || m_updatingMemHeader || !m_model)
      return; // GCOV_EXCL_LINE - Defensive invariant
    QString newVal = m_memWordWidthEdit->text().trimmed();
    QString oldVal = m_currentMemItem->data("Word Width").toString();
    if (newVal != oldVal) {
      m_undoStack->push(new EditItemPropertyCommand(
          m_model, m_currentMemItem, "Word Width", oldVal, newVal));
    }
  });

  connect(m_memDepthEdit, &QLineEdit::editingFinished, this, [this]() {
    if (!m_currentMemItem || m_updatingMemHeader || !m_model)
      return; // GCOV_EXCL_LINE - Defensive invariant
    QString newVal = m_memDepthEdit->text().trimmed();
    QString oldVal = m_currentMemItem->data("Depth").toString();
    if (newVal != oldVal) {
      m_undoStack->push(new EditItemPropertyCommand(m_model, m_currentMemItem,
                                                    "Depth", oldVal, newVal));
    }
  });

  connect(m_memHdlPathEdit, &QLineEdit::editingFinished, this, [this]() {
    if (!m_currentMemItem || m_updatingMemHeader || !m_model)
      return; // GCOV_EXCL_LINE - Defensive invariant
    QString newVal = m_memHdlPathEdit->text().trimmed();
    QString oldVal = m_currentMemItem->data("HDL Path").toString();
    if (newVal != oldVal) {
      m_undoStack->push(new EditItemPropertyCommand(
          m_model, m_currentMemItem, "HDL Path", oldVal, newVal));
    }
  });

  connect(m_memWrLockEdit, &QLineEdit::editingFinished, this, [this]() {
    if (!m_currentMemItem || m_updatingMemHeader || !m_model)
      return; // GCOV_EXCL_LINE - Defensive invariant
    QString newVal = m_memWrLockEdit->text().trimmed();
    QString oldVal = m_currentMemItem->data("Write Lock").toString();
    if (newVal != oldVal) {
      int col = m_model->columnOf("Write Lock");
      if (col < 0)
        col = 10; // GCOV_EXCL_LINE - Defensive column fallback
      QModelIndex idx = currentMemSourceIndex(col);
      if (idx.isValid()) {
        m_undoStack->push(new EditCellCommand(m_model, idx, oldVal, newVal));
      }
    }
  });

  connect(m_memRdLockEdit, &QLineEdit::editingFinished, this, [this]() {
    if (!m_currentMemItem || m_updatingMemHeader || !m_model)
      return; // GCOV_EXCL_LINE - Defensive invariant
    QString newVal = m_memRdLockEdit->text().trimmed();
    QString oldVal = m_currentMemItem->data("Read Lock").toString();
    if (newVal != oldVal) {
      int col = m_model->columnOf("Read Lock");
      if (col < 0)
        col = 11; // GCOV_EXCL_LINE - Defensive column fallback
      QModelIndex idx = currentMemSourceIndex(col);
      if (idx.isValid()) {
        m_undoStack->push(new EditCellCommand(m_model, idx, oldVal, newVal));
      }
    }
  });

  auto openLockDialogForMem = [this]() {
    if (!m_currentMemItem || !m_model)
      return; // GCOV_EXCL_LINE - Defensive invariant
    QString wrExpr = m_currentMemItem->data("Write Lock").toString();
    QString rdExpr = m_currentMemItem->data("Read Lock").toString();
    QString name = m_currentMemItem->data("Name").toString();
    RegLockDialog dlg(wrExpr, rdExpr, m_model, name, this);
    if (dlg.exec() == QDialog::Accepted) {
      QString newWr = dlg.writeExpression();
      QString newRd = dlg.readExpression();
      int wrCol = m_model->columnOf("Write Lock");
      int rdCol = m_model->columnOf("Read Lock");
      if (newWr != wrExpr) {
        QModelIndex wrIdx = currentMemSourceIndex(wrCol);
        if (wrIdx.isValid())
          m_undoStack->push(new EditCellCommand(m_model, wrIdx, wrExpr, newWr));
      }
      if (newRd != rdExpr) {
        QModelIndex rdIdx = currentMemSourceIndex(rdCol);
        if (rdIdx.isValid())
          m_undoStack->push(new EditCellCommand(m_model, rdIdx, rdExpr, newRd));
      }
    }
  };
  connect(m_memWrLockBtn, &QToolButton::clicked, this, openLockDialogForMem);
  connect(m_memRdLockBtn, &QToolButton::clicked, this, openLockDialogForMem);

  connect(m_memNoTestCheck, &QCheckBox::toggled, this, [this](bool checked) {
    if (!m_currentMemItem || m_updatingMemHeader || !m_model)
      return; // GCOV_EXCL_LINE - Defensive invariant
    QVariant oldVal = m_currentMemItem->data("NO_MEM_TEST");
    QVariant newVal = checked ? "true" : "false";
    if (oldVal.toString().toLower() != newVal.toString().toLower()) {
      m_undoStack->push(new EditItemPropertyCommand(
          m_model, m_currentMemItem, "NO_MEM_TEST", oldVal, newVal));
    }
  });

  connect(
      m_memNoWalkTestCheck, &QCheckBox::toggled, this, [this](bool checked) {
        if (!m_currentMemItem || m_updatingMemHeader || !m_model)
          return; // GCOV_EXCL_LINE - Defensive invariant
        QVariant oldVal = m_currentMemItem->data("NO_MEM_WALK_TEST");
        QVariant newVal = checked ? "true" : "false";
        if (oldVal.toString().toLower() != newVal.toString().toLower()) {
          m_undoStack->push(new EditItemPropertyCommand(
              m_model, m_currentMemItem, "NO_MEM_WALK_TEST", oldVal, newVal));
        }
      });

  connect(
      m_memNoAccessTestCheck, &QCheckBox::toggled, this, [this](bool checked) {
        if (!m_currentMemItem || m_updatingMemHeader || !m_model)
          return; // GCOV_EXCL_LINE - Defensive invariant
        QVariant oldVal = m_currentMemItem->data("NO_MEM_ACCESS_TEST");
        QVariant newVal = checked ? "true" : "false";
        if (oldVal.toString().toLower() != newVal.toString().toLower()) {
          m_undoStack->push(new EditItemPropertyCommand(
              m_model, m_currentMemItem, "NO_MEM_ACCESS_TEST", oldVal, newVal));
        }
      });

  QScrollArea *memSummaryScroll = new QScrollArea(m_memViewWidget);
  memSummaryScroll->setObjectName("memSummaryScroll");
  memSummaryScroll->setWidgetResizable(true);
  memSummaryScroll->setFrameShape(QFrame::StyledPanel);
  QWidget *summaryContainer = new QWidget(memSummaryScroll);
  QVBoxLayout *summaryLayout = new QVBoxLayout(summaryContainer);
  summaryLayout->setContentsMargins(12, 12, 12, 12);
  summaryLayout->setSpacing(8);

  m_memSummaryLabel = new QLabel(summaryContainer);
  m_memSummaryLabel->setObjectName("memSummaryLabel");
  m_memSummaryLabel->setTextFormat(Qt::RichText);
  m_memSummaryLabel->setWordWrap(true);
  summaryLayout->addWidget(m_memSummaryLabel);
  summaryLayout->addStretch(1);

  memSummaryScroll->setWidget(summaryContainer);
  memViewLayout->addWidget(memSummaryScroll, 1);

  m_rightStackedWidget->addWidget(m_memViewWidget);

  // ==========================================
  // Page 3: Empty View (Shown when no register or block is selected)
  // ==========================================
  m_emptyViewWidget = new QWidget(m_rightStackedWidget);
  m_emptyViewWidget->setObjectName("emptyViewWidget");

  QVBoxLayout *emptyLayout = new QVBoxLayout(m_emptyViewWidget);
  emptyLayout->setAlignment(Qt::AlignCenter);
  emptyLayout->setSpacing(12);
  emptyLayout->setContentsMargins(24, 24, 24, 24);

  emptyLayout->addStretch(1);

  QLabel *emptyLogoLabel = new QLabel(m_emptyViewWidget);
  emptyLogoLabel->setObjectName("emptyLogoLabel");
  QPixmap emptyLogoPix(":/icons/app_icon_128.png");
  emptyLogoLabel->setPixmap(emptyLogoPix);
  emptyLogoLabel->setAlignment(Qt::AlignCenter);
  emptyLayout->addWidget(emptyLogoLabel);

  QLabel *emptyTitleLabel =
      new QLabel(QStringLiteral("rmap"), m_emptyViewWidget);
  emptyTitleLabel->setObjectName("emptyTitleLabel");
  QFont emptyTitleFont = emptyTitleLabel->font();
  emptyTitleFont.setPointSize(22);
  emptyTitleFont.setBold(true);
  emptyTitleLabel->setFont(emptyTitleFont);
  emptyTitleLabel->setAlignment(Qt::AlignCenter);
  emptyLayout->addWidget(emptyTitleLabel);

  QLabel *emptySubtitleLabel =
      new QLabel(tr("Hardware Register Map Designer & Model Generator"),
                 m_emptyViewWidget);
  emptySubtitleLabel->setObjectName("emptySubtitleLabel");
  QFont emptySubFont = emptySubtitleLabel->font();
  emptySubFont.setPointSize(11);
  emptySubtitleLabel->setFont(emptySubFont);
  emptySubtitleLabel->setAlignment(Qt::AlignCenter);
  emptyLayout->addWidget(emptySubtitleLabel);

  QLabel *emptyHintLabel = new QLabel(
      tr("Select an item in the navigation tree to inspect details,\n"
         "or use the toolbar to create a new register map (Ctrl+N) or open a "
         "file (Ctrl+O)."),
      m_emptyViewWidget);
  emptyHintLabel->setObjectName("emptyHintLabel");
  QFont emptyHintFont = emptyHintLabel->font();
  emptyHintFont.setPointSize(10);
  emptyHintLabel->setFont(emptyHintFont);
  emptyHintLabel->setAlignment(Qt::AlignCenter);
  emptyLayout->addWidget(emptyHintLabel);

  emptyLayout->addStretch(2);

  m_rightStackedWidget->addWidget(m_emptyViewWidget);
  m_rightStackedWidget->setCurrentWidget(m_emptyViewWidget);

  rightLayout->addWidget(m_rightStackedWidget);

  // Connect block memory map diagram navigation
  connect(m_blockMemoryMapWidget, &BlockMemoryMapWidget::registerClicked, this,
          &RegMapWindow::navigateToRegister);

  // Splitter setup
  m_splitter = new QSplitter(Qt::Horizontal, this);
  m_splitter->addWidget(leftPanel);
  m_splitter->addWidget(rightPanel);
  m_splitter->setSizes(QList<int>() << 350 << 850);
  this->setCentralWidget(m_splitter);

  // Synchronize Bitfield Bar click with Fields Table selection (bidirectional
  // cross-probing)
  connect(m_bitfieldBar, &RegBitfieldBarWidget::fieldClicked, this,
          [this](int childRow) {
            if (!m_fieldProxy || !m_fieldsTableView || !m_model)
              return; // GCOV_EXCL_BR_LINE - Defensive invariant
            QModelIndex currentRegProxy = this->treeView->currentIndex();
            if (!currentRegProxy.isValid())
              return;
            QModelIndex currentRegSource =
                m_treeProxy->mapToSource(currentRegProxy);
            QModelIndex regCol0 = m_model->index(currentRegSource.row(), 0,
                                                 currentRegSource.parent());
            QModelIndex fieldSource = m_model->index(childRow, 0, regCol0);
            if (!fieldSource.isValid())
              return;
            QModelIndex fieldProxy = m_fieldProxy->mapFromSource(fieldSource);
            if (fieldProxy.isValid()) {
              if (m_regTabBar && m_regTabBar->currentIndex() == 3) {
                m_regTabBar->setCurrentIndex(0);
              }
              QModelIndex root = m_fieldsTableView->rootIndex();
              QModelIndex tableIdx =
                  m_fieldProxy->index(fieldProxy.row(), 1, root);
              if (!tableIdx.isValid())
                tableIdx = fieldProxy; // GCOV_EXCL_LINE - Defensive fallback
              m_fieldsTableView->setCurrentIndex(tableIdx);
              m_fieldsTableView->selectionModel()->select(
                  tableIdx, QItemSelectionModel::ClearAndSelect |
                                QItemSelectionModel::Rows);
              m_fieldsTableView->scrollTo(tableIdx,
                                          QAbstractItemView::PositionAtCenter);
            }
          });

  connectFieldsTableSignals();

  // Connect tree selection change
  connect(this->treeView->selectionModel(),
          &QItemSelectionModel::currentChanged, this,
          &RegMapWindow::updateFieldsTable);

  // Menu and Action connections
  connect(actionFileNew, &QAction::triggered, this, &RegMapWindow::btnFileNew);
  connect(actionFileOpen, &QAction::triggered, this,
          &RegMapWindow::btnFileOpen);
  connect(actionFileClose, &QAction::triggered, this,
          &RegMapWindow::btnFileClose);
  connect(actionFileSave, &QAction::triggered, this,
          &RegMapWindow::btnFileSave);
  connect(actionFileSaveAs, &QAction::triggered, this,
          &RegMapWindow::btnFileSaveAs);
  connect(actionFileReload, &QAction::triggered, this,
          &RegMapWindow::btnFileReload);
  connect(actionAddMem, &QAction::triggered, this,
          [this] { insertChild(RegMapTreeItem::e_rmmKind::mem); });
  connect(actionAddRegBlock, &QAction::triggered, this,
          [this] { insertChild(RegMapTreeItem::e_rmmKind::blk); });
  connect(actionAddRegField, &QAction::triggered, this,
          [this] { insertChild(RegMapTreeItem::e_rmmKind::fld); });
  connect(actionAddRegMap, &QAction::triggered, this,
          [this] { insertChild(RegMapTreeItem::e_rmmKind::map); });
  connect(actionAddReg, &QAction::triggered, this,
          [this] { insertChild(RegMapTreeItem::e_rmmKind::reg); });
  connect(actionDuplicate, &QAction::triggered, this,
          &RegMapWindow::duplicateSelectedRegister);
  connect(actionDeleteItem, &QAction::triggered, this,
          &RegMapWindow::btnDeleteItem);

  // Ergonomic power-user shortcuts (primary and documented secondary
  // alternatives)
  actionAddReg->setShortcuts(
      {QKeySequence("Ctrl+Shift+R"), QKeySequence("Ctrl+Return")});
  actionAddRegField->setShortcuts(
      {QKeySequence("Ctrl+Shift+F"), QKeySequence("Ctrl+Shift+Return")});
  actionDeleteItem->setShortcuts(
      {QKeySequence::Delete, QKeySequence(Qt::Key_Backspace)});
  actionDuplicate->setShortcut(QKeySequence("Ctrl+D"));
  actionFileReload->setShortcut(QKeySequence("Ctrl+R"));

  // Focus Search Bar shortcut (Ctrl+F / Find)
  auto *searchShortcut = new QShortcut(QKeySequence::Find, this);
  connect(searchShortcut, &QShortcut::activated, this, [this]() {
    if (m_searchEdit) {
      m_searchEdit->setFocus();
      m_searchEdit->selectAll();
    }
  });
  connect(actionCheck, &QAction::triggered, this, &RegMapWindow::btnCheck);
  connect(actionExport, &QAction::triggered, this, &RegMapWindow::btnExport);
  connect(actionQuit, &QAction::triggered, this, &RegMapWindow::btnQuitButton);
  connect(actionAbout, &QAction::triggered, this, &RegMapWindow::btnAbout);
  connect(actionConfig, &QAction::triggered, this, &RegMapWindow::btnConfig);
  connect(actionPreferences, &QAction::triggered, this,
          &RegMapWindow::btnPreferences);
  connect(actionColorBlindMode, &QAction::toggled, this,
          &RegMapWindow::onToggleColorBlindMode);
  connect(actionKeyboardShortcuts, &QAction::triggered, this,
          &RegMapWindow::btnKeyBindings);

  this->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this->treeView, &QWidget::customContextMenuRequested, this,
          &RegMapWindow::showTreeContextMenu);

  m_fieldsTableView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_fieldsTableView, &QWidget::customContextMenuRequested, this,
          &RegMapWindow::showTreeContextMenu);

  setupThemeMenu();
  setupColorBlindMenu();
  setupLanguageMenu();
  setupLayoutMenu();
  setColourBlindMode(AppSettings::instance().colorBlindMode());
  setColourScheme(AppSettings::instance().colorScheme());
  setLanguage(AppSettings::instance().language());
  setLayoutMode(AppSettings::instance().layoutMode());

  restoreWindowStateFromSettings();

  if (!m_rmap_filename.isEmpty()) {
    fileOpen(rmap_filename);
  } else {
    updatePaneVisibility();
  }

  auto *hexDelegate = new RegHexDecBinDelegate(this);
  auto *boolDelegate = new RegBoolDelegate(this);

  this->treeView->setItemDelegateForColumn(1, hexDelegate); // Offset
  this->treeView->setItemDelegateForColumn(2, hexDelegate); // Size
  this->treeView->setItemDelegateForColumn(3, new RegStrDelegate(this)); // Name
  this->treeView->setItemDelegateForColumn(
      4, new RegAccessPolicyDelegate(this)); // SW Access
  this->treeView->setItemDelegateForColumn(
      5, new RegHwAccessDelegate(this));                     // HW Access
  this->treeView->setItemDelegateForColumn(6, hexDelegate);  // Reset Value
  this->treeView->setItemDelegateForColumn(7, boolDelegate); // Is Rand
  this->treeView->setItemDelegateForColumn(8, boolDelegate); // Volatile
  this->treeView->setItemDelegateForColumn(9, boolDelegate); // Has Reset
  this->treeView->setItemDelegateForColumn(
      10, new RegLockDelegate(m_model, this)); // Write Lock
  this->treeView->setItemDelegateForColumn(
      11, new RegLockDelegate(m_model, this)); // Read Lock
  this->treeView->setItemDelegateForColumn(
      12, new RegBoolDelegate(this)); // Decode Only
  this->treeView->setItemDelegateForColumn(
      13, new RegMapDelegate(this)); // Description
}

RegMapWindow::~RegMapWindow() {
  saveWindowStateToSettings();
  if (m_undoStack) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_undoStack->clear();
  }
  if (m_fieldsTableView) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_fieldsTableView->setModel(nullptr);
  }
  if (this->treeView) { // GCOV_EXCL_BR_LINE - Defensive invariant
    this->treeView->setModel(nullptr);
  }
  if (m_treeProxy) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_treeProxy->setSourceModel(nullptr);
  }
  if (m_fieldProxy) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_fieldProxy->setSourceModel(nullptr);
  }
  delete m_pref_window;
  m_pref_window = nullptr;
  delete m_config_window;
  m_config_window = nullptr;
  delete m_model;
  m_model = nullptr;
}

void RegMapWindow::onSearchTextChanged(const QString &text) {
  if (m_treeProxy) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_treeProxy->setSearchFilter(text);
    if (!text.isEmpty()) {
      this->treeView->expandAll();
    }
  }
}

void RegMapWindow::btnConfig(void) {
  m_config_window->show();
  m_config_window->raise();
  m_config_window->activateWindow();
}

void RegMapWindow::btnPreferences(void) {
  if (m_pref_window) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_pref_window->show();
    m_pref_window->raise();
    m_pref_window->activateWindow();
  }
}

void RegMapWindow::onToggleColorBlindMode(bool checked) {
  this->setProperty("colorBlindMode", checked);
  AppSettings::instance().setColorBlindMode(checked);
  ColorBlindMode mode =
      checked ? AppSettings::instance().colorBlindType() : ColorBlindMode::None;
  ThemeManager::instance().setColorBlindMode(mode);
  if (m_bitfieldBar) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_bitfieldBar->setColorBlindMode(mode);
  }
  if (m_blockMemoryMapWidget) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_blockMemoryMapWidget->setColorBlindMode(mode);
  }
  if (m_regBlockMemoryMapWidget) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_regBlockMemoryMapWidget->setColorBlindMode(mode);
  }
  if (m_pref_window) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_pref_window->setColourBlindMode(checked);
    m_pref_window->setColourBlindType(AppSettings::instance().colorBlindType());
  }
  if (actionColorBlindMode && actionColorBlindMode->isChecked() != checked) {
    actionColorBlindMode->setChecked(checked);
  }
  if (m_colorBlindActionGroup) { // GCOV_EXCL_BR_LINE - Defensive invariant
    if (!checked) {
      for (auto *act : m_colorBlindActionGroup->actions()) {
        act->setChecked(false);
      }
    } else {
      QString curType = AppSettings::instance().colorBlindTypeString();
      for (auto *act : m_colorBlindActionGroup->actions()) {
        act->setChecked(act->data().toString() == curType);
      }
    }
  }
  if (this->treeView) { // GCOV_EXCL_BR_LINE - Defensive invariant
    this->treeView->viewport()->update();
  }
  if (m_fieldsTableView) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_fieldsTableView->viewport()->update();
  }
  if (this->statusBar()) { // GCOV_EXCL_BR_LINE - Defensive invariant
    QString msg;
    if (checked) {
      msg = tr("Colour-Blind Mode (%1) Enabled")
                .arg(colorBlindModeToString(
                    AppSettings::instance().colorBlindType()));
    } else {
      msg = tr("Standard Colour Palette Active");
    }
    this->statusBar()->showMessage(msg, 3000);
  }
}

void RegMapWindow::setColourBlindMode(bool enabled) {
  onToggleColorBlindMode(enabled);
}

bool RegMapWindow::isColourBlindMode() const {
  return m_bitfieldBar ? m_bitfieldBar->isColorBlindMode()
                       : false; // GCOV_EXCL_BR_LINE - Defensive invariant
}

void RegMapWindow::setColourBlindType(ColorBlindMode mode) {
  if (mode == ColorBlindMode::None) {
    onToggleColorBlindMode(false);
    return;
  }
  AppSettings::instance().setColorBlindType(mode);
  if (!isColourBlindMode()) {
    onToggleColorBlindMode(true);
  } else {
    ThemeManager::instance().setColorBlindMode(mode);
    if (m_bitfieldBar) { // GCOV_EXCL_BR_LINE - Defensive invariant
      m_bitfieldBar->setColorBlindMode(mode);
    }
    if (m_blockMemoryMapWidget) { // GCOV_EXCL_BR_LINE - Defensive invariant
      m_blockMemoryMapWidget->setColorBlindMode(mode);
    }
    if (m_regBlockMemoryMapWidget) { // GCOV_EXCL_BR_LINE - Defensive invariant
      m_regBlockMemoryMapWidget->setColorBlindMode(mode);
    }
    if (m_pref_window) { // GCOV_EXCL_BR_LINE - Defensive invariant
      m_pref_window->setColourBlindType(mode);
    }
    if (m_colorBlindActionGroup) { // GCOV_EXCL_BR_LINE - Defensive invariant
      QString curType = colorBlindModeToString(mode);
      for (auto *act : m_colorBlindActionGroup->actions()) {
        act->setChecked(act->data().toString() == curType);
      }
    }
    if (this->treeView) { // GCOV_EXCL_BR_LINE - Defensive invariant
      this->treeView->viewport()->update();
    }
    if (m_fieldsTableView) { // GCOV_EXCL_BR_LINE - Defensive invariant
      m_fieldsTableView->viewport()->update();
    }
    if (this->statusBar()) { // GCOV_EXCL_BR_LINE - Defensive invariant
      this->statusBar()->showMessage(
          tr("Colour-Blind Profile: %1").arg(colorBlindModeToString(mode)),
          3000);
    }
  }
}

ColorBlindMode RegMapWindow::colourBlindType() const {
  if (!isColourBlindMode()) {
    return ColorBlindMode::None;
  }
  return AppSettings::instance().colorBlindType();
}

void RegMapWindow::setupColorBlindMenu(void) {
  if (!menuView)
    return; // GCOV_EXCL_LINE - Defensive invariant

  m_colorBlindMenu = new QMenu(tr("Colour-&Blind Profile"), menuView);
  m_colorBlindMenu->setObjectName("menuColorBlindProfile");

  rebuildColorBlindMenu();

  connect(m_colorBlindMenu, &QMenu::aboutToShow, this, [this]() {
    if (m_colorBlindActionGroup) { // GCOV_EXCL_BR_LINE - Defensive invariant
      if (!isColourBlindMode()) {
        for (auto *act : m_colorBlindActionGroup->actions()) {
          act->setChecked(false);
        }
      } else {
        QString activeType = AppSettings::instance().colorBlindTypeString();
        for (auto *act : m_colorBlindActionGroup->actions()) {
          act->setChecked(act->data().toString() == activeType);
        }
      }
    }
  });

  connect(&ThemeManager::instance(), &ThemeManager::colorBlindModeChanged, this,
          [this](ColorBlindMode mode) {
            if (m_colorBlindActionGroup) { // GCOV_EXCL_BR_LINE - Defensive
                                           // invariant
              if (mode == ColorBlindMode::None) {
                for (auto *act : m_colorBlindActionGroup->actions()) {
                  act->setChecked(false);
                }
              } else {
                QString curType = colorBlindModeToString(mode);
                for (auto *act : m_colorBlindActionGroup->actions()) {
                  act->setChecked(act->data().toString() == curType);
                }
              }
            }
          });

  menuView->addMenu(m_colorBlindMenu);
}

void RegMapWindow::rebuildColorBlindMenu(void) {
  if (!m_colorBlindMenu)
    return; // GCOV_EXCL_LINE - Defensive invariant

  m_colorBlindMenu->clear();
  delete m_colorBlindActionGroup;
  m_colorBlindActionGroup = new QActionGroup(m_colorBlindMenu);
  m_colorBlindActionGroup->setExclusive(true);

  bool isCb = isColourBlindMode();
  QString activeType = AppSettings::instance().colorBlindTypeString();

  for (const auto &info : availableColorBlindModes()) {
    QAction *act = m_colorBlindMenu->addAction(info.name);
    act->setCheckable(true);
    act->setData(info.id);
    m_colorBlindActionGroup->addAction(act);
    if (isCb && info.id == activeType) {
      act->setChecked(true);
    }

    connect(act, &QAction::triggered, this,
            [this, mode = info.mode]() { setColourBlindType(mode); });
  }
}

void RegMapWindow::setupThemeMenu(void) {
  if (!menuView)
    return; // GCOV_EXCL_LINE - Defensive invariant

  m_themeMenu = new QMenu(tr("&Colour Scheme"), menuView);
  m_themeMenu->setObjectName("menuColourScheme");

  rebuildThemeMenu();

  connect(m_themeMenu, &QMenu::aboutToShow, this, [this]() {
    ThemeManager::instance().scanThemes();
    QString cur = ThemeManager::instance().currentThemeId();
    if (m_themeActionGroup) { // GCOV_EXCL_BR_LINE - Defensive invariant
      for (auto *act : m_themeActionGroup->actions()) {
        act->setChecked(act->data().toString() == cur);
      }
    }
  });

  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this,
          [this](const ColorScheme &theme) {
            if (m_themeActionGroup) { // GCOV_EXCL_BR_LINE - Defensive invariant
              for (auto *act : m_themeActionGroup->actions()) {
                act->setChecked(act->data().toString() == theme.id);
              }
            }
          });

  connect(&ThemeManager::instance(), &ThemeManager::themesUpdated, this,
          [this]() { rebuildThemeMenu(); });

  menuView->addMenu(m_themeMenu);
}

void RegMapWindow::rebuildThemeMenu(void) {
  if (!m_themeMenu)
    return; // GCOV_EXCL_LINE - Defensive invariant

  m_themeMenu->clear();
  delete m_themeActionGroup;
  m_themeActionGroup = new QActionGroup(m_themeMenu);
  m_themeActionGroup->setExclusive(true);

  QString currentTheme = ThemeManager::instance().currentThemeId();

  for (const auto &t : ThemeManager::instance().availableThemes()) {
    QAction *act = m_themeMenu->addAction(t.name);
    act->setCheckable(true);
    act->setData(t.id);
    m_themeActionGroup->addAction(act);
    if (t.id == currentTheme) {
      act->setChecked(true);
    }

    connect(act, &QAction::triggered, this,
            [this, id = t.id]() { setColourScheme(id); });
  }
}

void RegMapWindow::setColourScheme(const QString &scheme) {
  ThemeManager::instance().setTheme(scheme);
  AppSettings::instance().setColorScheme(scheme);
  if (m_pref_window) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_pref_window->setColourScheme(scheme);
  }
  if (m_themeActionGroup) { // GCOV_EXCL_BR_LINE - Defensive invariant
    QString cur = ThemeManager::instance().currentThemeId();
    for (auto *act : m_themeActionGroup->actions()) {
      act->setChecked(act->data().toString() == cur);
    }
  }
  if (this->treeView) { // GCOV_EXCL_BR_LINE - Defensive invariant
    this->treeView->viewport()->update();
  }
  if (m_fieldsTableView) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_fieldsTableView->viewport()->update();
  }
  if (m_bitfieldBar) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_bitfieldBar->update();
  }
  if (m_blockMemoryMapWidget) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_blockMemoryMapWidget->update();
  }
  if (this->statusBar()) { // GCOV_EXCL_BR_LINE - Defensive invariant
    this->statusBar()->showMessage(
        tr("Colour Scheme: %1")
            .arg(ThemeManager::instance().currentThemeName()),
        3000);
  }
}

QString RegMapWindow::colourScheme() const {
  return ThemeManager::instance().currentThemeId();
}

void RegMapWindow::setupLanguageMenu(void) {
  if (!menuView)
    return; // GCOV_EXCL_LINE - Defensive invariant

  m_languageMenu = new QMenu(tr("&Language"), menuView);
  m_languageMenu->setObjectName("menuLanguage");
  m_languageActionGroup = new QActionGroup(m_languageMenu);
  m_languageActionGroup->setExclusive(true);

  QString currentLang = LanguageManager::instance().currentLanguage();

  for (const auto &lang : LanguageManager::instance().availableLanguages()) {
    QString displayName = lang.displayName();

    QAction *act = m_languageMenu->addAction(displayName);
    act->setCheckable(true);
    act->setData(lang.code);
    m_languageActionGroup->addAction(act);
    if (lang.code.compare(currentLang, Qt::CaseInsensitive) == 0) {
      act->setChecked(true);
    }

    connect(act, &QAction::triggered, this,
            [this, code = lang.code]() { setLanguage(code); });
  }

  // Always keep checkmarks perfectly synchronized when menu is opened
  connect(m_languageMenu, &QMenu::aboutToShow, this, [this]() {
    QString cur = LanguageManager::instance().currentLanguage();
    if (m_languageActionGroup) { // GCOV_EXCL_BR_LINE - Defensive invariant
      for (auto *act : m_languageActionGroup->actions()) {
        act->setChecked(
            act->data().toString().compare(cur, Qt::CaseInsensitive) == 0);
      }
    }
  });

  // Synchronize checkmarks whenever language changes from anywhere
  connect(
      &LanguageManager::instance(), &LanguageManager::languageChanged, this,
      [this](const QString &code) {
        if (m_languageActionGroup) { // GCOV_EXCL_BR_LINE - Defensive invariant
          for (auto *act : m_languageActionGroup->actions()) {
            act->setChecked(
                act->data().toString().compare(code, Qt::CaseInsensitive) == 0);
          }
        }
        if (m_pref_window) { // GCOV_EXCL_BR_LINE - Defensive invariant
          m_pref_window->setLanguage(code);
        }
        if (this->statusBar()) { // GCOV_EXCL_BR_LINE - Defensive invariant
          this->statusBar()->showMessage(
              tr("Language: %1")
                  .arg(LanguageManager::instance().currentLanguageName()),
              3000);
        }
      });

  menuView->addMenu(m_languageMenu);
}

void RegMapWindow::setLanguage(const QString &code) {
  LanguageManager::instance().setLanguage(code);
  AppSettings::instance().setLanguage(code);
  if (m_pref_window) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_pref_window->setLanguage(code);
  }

  QString currentLang = LanguageManager::instance().currentLanguage();
  if (m_languageActionGroup) { // GCOV_EXCL_BR_LINE - Defensive invariant
    for (auto *act : m_languageActionGroup->actions()) {
      act->setChecked(act->data().toString().compare(currentLang,
                                                     Qt::CaseInsensitive) == 0);
    }
  }

  if (this->statusBar()) { // GCOV_EXCL_BR_LINE - Defensive invariant
    this->statusBar()->showMessage(
        tr("Language: %1")
            .arg(LanguageManager::instance().currentLanguageName()),
        3000);
  }
}

QString RegMapWindow::language() const {
  return LanguageManager::instance().currentLanguage();
}

void RegMapWindow::setupLayoutMenu(void) {
  if (!menuView) {
    return; // GCOV_EXCL_LINE - Defensive menuView check
  }

  m_layoutMenu = new QMenu(tr("La&yout"), menuView);
  m_layoutMenu->setObjectName("menuLayout");
  m_layoutActionGroup = new QActionGroup(this);
  m_layoutActionGroup->setExclusive(true);

  QAction *actTabbed = m_layoutMenu->addAction(tr("&Tabbed (Task-Centric)"));
  actTabbed->setCheckable(true);
  actTabbed->setData(QStringLiteral("tabbed"));
  m_layoutActionGroup->addAction(actTabbed);

  QAction *actClassic =
      m_layoutMenu->addAction(tr("&Classic (Horizontal Split)"));
  actClassic->setCheckable(true);
  actClassic->setData(AppSettings::classicLayoutId());
  m_layoutActionGroup->addAction(actClassic);

  QAction *actTable =
      m_layoutMenu->addAction(tr("Full &Table (Vertical Split)"));
  actTable->setCheckable(true);
  actTable->setData(QStringLiteral("table"));
  m_layoutActionGroup->addAction(actTable);

  connect(m_layoutActionGroup, &QActionGroup::triggered, this,
          [this](QAction *action) {
            if (action) {
              setLayoutMode(action->data().toString());
            }
          });

  connect(&AppSettings::instance(), &AppSettings::layoutModeChanged, this,
          [this](const QString &mode) { setLayoutMode(mode); });

  menuView->addMenu(m_layoutMenu);
}

void RegMapWindow::setLayoutMode(const QString &mode) {
  QString m = mode.trimmed().toLower();
  if (m == AppSettings::classicLayoutId() || m == "classic_split" ||
      m == "classic_horizontal" || m == "horizontal") {
    m = AppSettings::classicLayoutId();
  } else if (m == "table") {
    m = "table";
  } else {
    m = QStringLiteral("tabbed");
  }

  m_layoutMode = m;
  AppSettings::instance().setLayoutMode(m);

  if (m_layoutActionGroup) {
    for (auto *act : m_layoutActionGroup->actions()) {
      act->setChecked(act->data().toString() == m_layoutMode);
    }
  }

  if (m_pref_window) {
    m_pref_window->setLayoutMode(m_layoutMode);
  }

  if (m_layoutMode == AppSettings::classicLayoutId()) {
    if (m_splitter) {
      m_splitter->setOrientation(Qt::Vertical);
      m_splitter->setSizes({250, 450});
    }
    if (m_regTabBar) {
      m_regTabBar->setVisible(false);
    }
    if (m_regTabStack) {
      m_regTabStack->setCurrentIndex(0);
    }
    if (m_fieldsTableView && m_model) {
      m_fieldsTableView->setColumnHidden(0, true);
      for (int col = 1; col < m_model->columnCount(); ++col) {
        m_fieldsTableView->setColumnHidden(col, false);
        m_fieldsTableView->resizeColumnToContents(col);
        int minHeader =
            m_fieldsTableView->horizontalHeader()->sectionSizeHint(col);
        if (m_fieldsTableView->columnWidth(col) < minHeader) {
          m_fieldsTableView->setColumnWidth(col, minHeader);
        }
      }
      int descCol = m_model ? m_model->columnOf("Description") : 13;
      if (descCol < 0)
        descCol = 13; // GCOV_EXCL_LINE - Defensive column fallback
      m_fieldsTableView->horizontalHeader()->setStretchLastSection(true);
      m_fieldsTableView->horizontalHeader()->setSectionResizeMode(
          descCol, QHeaderView::Stretch);
    }
  } else if (m_layoutMode == "table") {
    if (m_splitter) {
      m_splitter->setOrientation(Qt::Horizontal);
      m_splitter->setSizes({350, 850});
    }
    if (m_regTabBar) {
      m_regTabBar->setVisible(false);
    }
    if (m_regTabStack) {
      m_regTabStack->setCurrentIndex(0);
    }
    if (m_fieldsTableView && m_model) {
      m_fieldsTableView->setColumnHidden(0, true);
      for (int col = 1; col < m_model->columnCount(); ++col) {
        m_fieldsTableView->setColumnHidden(col, false);
        m_fieldsTableView->resizeColumnToContents(col);
        int minHeader =
            m_fieldsTableView->horizontalHeader()->sectionSizeHint(col);
        if (m_fieldsTableView->columnWidth(col) < minHeader) {
          m_fieldsTableView->setColumnWidth(col, minHeader);
        }
      }
      int descCol = m_model ? m_model->columnOf("Description") : 13;
      if (descCol < 0)
        descCol = 13; // GCOV_EXCL_LINE - Defensive column fallback
      m_fieldsTableView->horizontalHeader()->setStretchLastSection(true);
      m_fieldsTableView->horizontalHeader()->setSectionResizeMode(
          descCol, QHeaderView::Stretch);
    }
  } else {
    if (m_splitter) {
      m_splitter->setOrientation(Qt::Horizontal);
      m_splitter->setSizes({350, 850});
    }
    if (m_regTabBar) {
      m_regTabBar->setVisible(true);
    }
    applyFieldTabColumnFilter(m_regTabBar ? m_regTabBar->currentIndex() : 0);
  }

  if (this->statusBar()) {
    QString label;
    if (m_layoutMode == AppSettings::classicLayoutId()) {
      label = tr("Classic Layout");
    } else if (m_layoutMode == "table") {
      label = tr("Full Table Layout");
    } else {
      label = tr("Tabbed Layout");
    }
    this->statusBar()->showMessage(tr("Layout: %1").arg(label), 3000);
  }
}

void RegMapWindow::updateDynamicTranslations(void) {
  if (m_languageMenu) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_languageMenu->setTitle(tr("&Language"));
    QString currentLang = LanguageManager::instance().currentLanguage();
    if (m_languageActionGroup) { // GCOV_EXCL_BR_LINE - Defensive invariant
      for (auto *act : m_languageActionGroup->actions()) {
        act->setChecked(act->data().toString().compare(
                            currentLang, Qt::CaseInsensitive) == 0);
      }
    }
  }
  if (m_themeMenu) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_themeMenu->setTitle(tr("&Colour Scheme"));
    QString currentTheme = ThemeManager::instance().currentThemeId();
    if (m_themeActionGroup) { // GCOV_EXCL_BR_LINE - Defensive invariant
      for (auto *act : m_themeActionGroup->actions()) {
        act->setChecked(act->data().toString() == currentTheme);
      }
    }
  }
  if (m_colorBlindMenu) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_colorBlindMenu->setTitle(tr("Colour-&Blind Profile"));
    rebuildColorBlindMenu();
  }
  if (m_layoutMenu) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_layoutMenu->setTitle(tr("La&yout"));
    if (m_layoutActionGroup) {
      for (auto *act : m_layoutActionGroup->actions()) {
        QString data = act->data().toString();
        if (data == "tabbed") {
          act->setText(tr("&Tabbed (Task-Centric)"));
        } else if (data == AppSettings::classicLayoutId()) {
          act->setText(tr("&Classic (Horizontal Split)"));
        } else if (data == "table") {
          act->setText(tr("Full &Table (Vertical Split)"));
        }
      }
    }
  }
  if (m_model) {
    m_model->refreshHeaderData();
  }
  if (m_regTabBar) {
    m_regTabBar->setTabText(0, tr("Fields & Layout"));
    m_regTabBar->setTabText(1, tr("Verification & UVM"));
    m_regTabBar->setTabText(2, tr("Security & Locks"));
    m_regTabBar->setTabText(3, tr("Memory Map"));
    m_regTabBar->setTabText(4, tr("All Properties"));
  }
  if (m_rmap_filename.isEmpty()) {
    setWindowTitle(tr("Register Map Generation Tool"));
  } else {
    setWindowTitle(QString("%1 — %2").arg(QFileInfo(m_rmap_filename).fileName(),
                                          tr("Register Map Generation Tool")));
  }
}

void RegMapWindow::changeEvent(QEvent *event) {
  if (event->type() == QEvent::LanguageChange) {
    retranslateUi(this);
    updateDynamicTranslations();
  }
  QMainWindow::changeEvent(event);
}

void RegMapWindow::restoreWindowStateFromSettings() {
  QByteArray geom = AppSettings::instance().mainWindowGeometry();
  if (!geom.isEmpty()) {
    restoreGeometry(geom);
  } else {
    QSize sz = AppSettings::instance().mainWindowSize();
    QPoint p = AppSettings::instance().mainWindowPos();
    if (!sz.isEmpty()) {
      resize(sz);
    }
    if (!p.isNull()) {
      move(p);
    }
  }
  AppSettings::ensureWindowOnScreen(this, QSize(600, 450), QSize(1200, 800));

  QByteArray state = AppSettings::instance().mainWindowState();
  if (!state.isEmpty()) {
    restoreState(state);
  }
  QByteArray splitterState = AppSettings::instance().mainWindowSplitter();
  if (!splitterState.isEmpty() && m_splitter) {
    m_splitter->restoreState(splitterState);
    Qt::Orientation expectedOrientation =
        (m_layoutMode == AppSettings::classicLayoutId()) ? Qt::Vertical
                                                         : Qt::Horizontal;
    // GCOV_EXCL_START - Defensive orientation mismatch fallback
    if (m_splitter->orientation() != expectedOrientation) {
      m_splitter->setOrientation(expectedOrientation);
      if (expectedOrientation == Qt::Vertical) {
        m_splitter->setSizes({250, 450});
      } else {
        m_splitter->setSizes({350, 850});
      }
    }
    // GCOV_EXCL_STOP
  }
}

void RegMapWindow::saveWindowStateToSettings() {
  AppSettings::instance().setMainWindowGeometry(saveGeometry());
  AppSettings::instance().setMainWindowState(saveState());
  AppSettings::instance().setMainWindowPos(pos());
  AppSettings::instance().setMainWindowSize(size());
  if (m_splitter) { // GCOV_EXCL_BR_LINE - Defensive invariant
    AppSettings::instance().setMainWindowSplitter(m_splitter->saveState());
  }
}

void RegMapWindow::closeEvent(QCloseEvent *event) {
  saveWindowStateToSettings();
  QMainWindow::closeEvent(event);
}

void RegMapWindow::resizeEvent(QResizeEvent *event) {
  QMainWindow::resizeEvent(event);
  AppSettings::instance().setMainWindowSize(size());
}

void RegMapWindow::moveEvent(QMoveEvent *event) {
  QMainWindow::moveEvent(event);
  AppSettings::instance().setMainWindowPos(pos());
}

void RegMapWindow::btnAbout(void) {
  if (m_about_window) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_about_window->show();
    m_about_window->raise();
    m_about_window->activateWindow();
  }
}

void RegMapWindow::btnKeyBindings(void) {
  QDialog dialog(this, Qt::Window);
  dialog.setWindowIcon(appIcon());
  dialog.setWindowTitle(tr("Keyboard Shortcuts & Key Bindings"));
  dialog.resize(580, 480);
  dialog.setMinimumSize(450, 350);

  QVBoxLayout *layout = new QVBoxLayout(&dialog);

  QTextBrowser *browser = new QTextBrowser(&dialog);
  browser->setOpenExternalLinks(true);
  browser->setHtml(
      "<h3>rmap — Keyboard Shortcuts & Key Bindings</h3>"
      "<table border='0' cellspacing='4' cellpadding='4' width='100%'>"
      "<tr style='background-color:#EAECEE;'><th "
      "align='left'><b>Category</b></th><th "
      "align='left'><b>Shortcut</b></th><th "
      "align='left'><b>Description</b></th></tr>"
      "<tr><td colspan='3' style='padding-top:8px;'><b>File "
      "Operations</b></td></tr>"
      "<tr><td>File</td><td><kbd>Ctrl+N</kbd></td><td>Create New Register "
      "Map</td></tr>"
      "<tr><td>File</td><td><kbd>Ctrl+O</kbd></td><td>Open File (SVD, RDL, "
      "XML, JSON, CSV, RMT, RMB)</td></tr>"
      "<tr><td>File</td><td><kbd>Ctrl+W</kbd></td><td>Close Register Map "
      "Model</td></tr>"
      "<tr><td>File</td><td><kbd>Ctrl+S</kbd></td><td>Save Register "
      "Map</td></tr>"
      "<tr><td>File</td><td><kbd>Ctrl+Shift+S</kbd></td><td>Save Register Map "
      "As...</td></tr>"
      "<tr><td>File</td><td><kbd>Ctrl+R</kbd></td><td>Reload Active "
      "File</td></tr>"
      "<tr><td>File</td><td><kbd>Ctrl+Q</kbd></td><td>Quit "
      "Application</td></tr>"
      "<tr><td colspan='3' style='padding-top:8px;'><b>Edit & "
      "History</b></td></tr>"
      "<tr><td>Edit</td><td><kbd>Ctrl+Z</kbd></td><td>Undo Last "
      "Action</td></tr>"
      "<tr><td>Edit</td><td><kbd>Ctrl+Y</kbd></td><td>Redo Last "
      "Action</td></tr>"
      "<tr><td colspan='3' style='padding-top:8px;'><b>Hardware Structure "
      "Elements</b></td></tr>"
      "<tr><td>Structure</td><td><kbd>Ctrl+Shift+B</kbd></td><td>Add Register "
      "Block (blk)</td></tr>"
      "<tr><td>Structure</td><td><kbd>Ctrl+Shift+R</kbd> / "
      "<kbd>Ctrl+Return</kbd></td><td>Add Register (reg)</td></tr>"
      "<tr><td>Structure</td><td><kbd>Ctrl+Shift+F</kbd> / "
      "<kbd>Ctrl+Shift+Return</kbd></td><td>Add Bitfield (fld)</td></tr>"
      "<tr><td>Structure</td><td><kbd>Ctrl+D</kbd></td><td>Duplicate Selected "
      "Register &amp; Bitfields</td></tr>"
      "<tr><td>Structure</td><td><kbd>Ctrl+Shift+M</kbd></td><td>Add Memory "
      "Region (mem)</td></tr>"
      "<tr><td>Structure</td><td><kbd>Ctrl+M</kbd></td><td>Add Address Map "
      "(map)</td></tr>"
      "<tr><td>Structure</td><td><kbd>Delete</kbd> / <kbd>Del</kbd> / "
      "<kbd>Backspace</kbd></td><td>Delete Selected Item</td></tr>"
      "<tr><td colspan='3' style='padding-top:8px;'><b>Validation & Code "
      "Generation</b></td></tr>"
      "<tr><td>Tools</td><td><kbd>Ctrl+K</kbd></td><td>Run Architectural "
      "Linter / Overlap Check</td></tr>"
      "<tr><td>Tools</td><td><kbd>Ctrl+E</kbd></td><td>Export & Generate "
      "Hardware/Software Models</td></tr>"
      "<tr><td>Tools</td><td><kbd>Ctrl+P</kbd></td><td>Open Project "
      "Configuration</td></tr>"
      "<tr><td>Tools</td><td><kbd>Ctrl+,</kbd></td><td>Open Application "
      "Preferences</td></tr>"
      "<tr><td colspan='3' style='padding-top:8px;'><b>View & "
      "Accessibility</b></td></tr>"
      "<tr><td>View</td><td><kbd>Ctrl+F</kbd></td><td>Focus Search "
      "Bar</td></tr>"
      "<tr><td>View</td><td><kbd>Ctrl+Alt+C</kbd></td><td>Toggle Color-Blind "
      "Mode (Barrier-Free CVD Palette)</td></tr>"
      "<tr><td>Help</td><td><kbd>F1</kbd></td><td>Show Key Bindings & "
      "Shortcuts</td></tr>"
      "<tr><td>Help</td><td><kbd>Ctrl+I</kbd></td><td>Show About "
      "Window</td></tr>"
      "</table>");
  layout->addWidget(browser);

  QDialogButtonBox *btnBox =
      new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
  connect(btnBox, &QDialogButtonBox::rejected, &dialog, &QDialog::accept);
  layout->addWidget(btnBox);

  dialog.exec();
}

void RegMapWindow::btnQuitButton(void) { this->close(); }

void RegMapWindow::btnFileNew(void) {
  if (m_is_regmap_modified) {
    QMessageBox::StandardButton result = QMessageBox::warning(
        this, tr("New file"),
        tr("This action will remove all unsaved data, do you wish to "
           "continue?"),
        QMessageBox::Ok | QMessageBox::Save | QMessageBox::Cancel);
    if (result == QMessageBox::Cancel) {
      return;
    }
    if (result == QMessageBox::Save && !btnFileSave()) {
      return;
    }
  }
  fileNew();
}

void RegMapWindow::btnFileClose(void) {
  if (m_is_regmap_modified) {
    QMessageBox::StandardButton result = QMessageBox::warning(
        this, tr("Close model"),
        tr("This action will remove all unsaved data, do you wish to "
           "continue?"),
        QMessageBox::Ok | QMessageBox::Save | QMessageBox::Cancel);
    if (result == QMessageBox::Cancel) {
      return;
    }
    if (result == QMessageBox::Save && !btnFileSave()) {
      return;
    }
  }
  fileNew();
}

bool RegMapWindow::btnFileSave(void) {
  bool save_status = false;
  if (m_rmap_filename.isEmpty()) {
    save_status = btnFileSaveAs();
  } else {
    save_status = fileSave();
  }
  return save_status;
}

bool RegMapWindow::btnFileSaveAs(void) {
  QString fname =
      m_rmap_filename.isEmpty() ? m_default_filename : m_rmap_filename;
  QString selectedFilter;
  QString chosen = QFileDialog::getSaveFileName(
      this, tr("Save Register Map As"), fname,
      FormatManager::instance().allFilterString(), &selectedFilter);

  if (chosen.isEmpty()) {
    return false;
  }

  bool save_status = fileSave(chosen);
  if (save_status) {
    m_rmap_filename = chosen;
    QString filename = m_default_window_title + " - " + chosen;
    this->setWindowTitle(filename);
  }
  return save_status;
}

void RegMapWindow::btnFileOpen(void) {
  QMessageBox::StandardButton result;
  if (m_is_regmap_modified) {
    result = QMessageBox::warning(this, tr("Open file"),
                                  tr("This action will remove all unsaved "
                                     "data, do you wish to continue?"),
                                  QMessageBox::Ok | QMessageBox::Cancel);
  }
  if (!m_is_regmap_modified || result == QMessageBox::Ok) {
    QString fname = QFileDialog::getOpenFileName(
        this, tr("Open Register Map File"), m_active_folder,
        FormatManager::instance().allFilterString());
    if (!fname.isEmpty()) {
      fileOpen(fname);
      QString filename = m_default_window_title + " - " + fname;
      this->setWindowTitle(filename);
    }
  }
}

void RegMapWindow::btnFileReload(void) {
  QMessageBox::StandardButton result;
  if (m_is_regmap_modified) {
    result = QMessageBox::warning(this, tr("Reload file"),
                                  tr("This action will remove all unsaved "
                                     "data, do you wish to continue?"),
                                  QMessageBox::Ok | QMessageBox::Cancel);
  }
  if (!m_is_regmap_modified || result == QMessageBox::Ok) {
    QString fname = m_rmap_filename;
    fileOpen(fname);
    QString filename = m_default_window_title + " - " + fname;
    this->setWindowTitle(filename);
  }
}

void RegMapWindow::btnCheck(void) {
  protormap::Config *cfg = m_config_window->serialize();
  uint32_t regWidth = cfg->reg_width() > 0 ? cfg->reg_width() : 32;
  delete cfg;

  QStringList errors = m_model->checkData(regWidth);
  this->treeView->viewport()->update();
  if (m_fieldsTableView) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_fieldsTableView->viewport()->update();
  }
  if (m_bitfieldBar) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_bitfieldBar->update();
  }

  if (errors.isEmpty()) {
    QMessageBox::information(
        this, tr("Check Successful"),
        tr("✓ Register Map validation successful!\n\nNo overlapping addresses, "
           "bitfield collisions, or register width violations were found."),
        QMessageBox::Ok);
  } else {
    QString errorSummary =
        tr("⚠ Validation found %1 issue(s):\n\n").arg(errors.size());
    for (const QString &err : errors) {
      errorSummary += QString("• %1\n").arg(err);
    }
    errorSummary += tr("\nProblematic cells have been highlighted in red.");

    QMessageBox::warning(this, tr("Check Issues Found"), errorSummary,
                         QMessageBox::Ok);
  }
}

void RegMapWindow::btnExport(void) {
  QString baseDir = m_config_window->baseDir();

  protormap::Config *cfg = m_config_window->serialize();
  uint32_t regWidth = cfg->reg_width() > 0 ? cfg->reg_width() : 32;

  QStringList validationErrors = m_model->checkData(regWidth);
  this->treeView->viewport()->update();
  if (m_fieldsTableView) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_fieldsTableView->viewport()->update();
  }

  if (!validationErrors.isEmpty()) {
    QString msg =
        tr("Validation found %1 issue(s):\n\n").arg(validationErrors.size());
    for (const QString &err : validationErrors) {
      msg += QString("• %1\n").arg(err);
    }
    msg += tr("\nDo you want to proceed with export anyway?");

    auto reply = QMessageBox::question(this, tr("Validation Warnings"), msg,
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) {
      delete cfg;
      return;
    }
  }

  CodeGenerator cg;
  std::string template_folder = cfg->templatefolder();
  std::string default_output = resolveExportOutputFolder(QString(), cfg);

  bool hwPrec =
      cfg->has_hw_precedence()
          ? cfg->hw_precedence()
          // GCOV_EXCL_START - Defensive fallback
          : (m_config_window ? m_config_window->hwPrecedence() : true);
  // GCOV_EXCL_STOP
  json jsonData = m_model->extractJsonData(regWidth, hwPrec);
  resolveExportProjectName(cfg, m_rmap_filename, jsonData);
  jsonData["project_name"] = cfg->project_name();
  jsonData["project_version"] = cfg->project_version();
  jsonData["project_vendor"] = cfg->project_vendor();
  jsonData["project_library"] = cfg->project_library();
  jsonData["project_description"] = cfg->project_description();
  for (const auto &[key, value] : cfg->custom_parameters()) {
    jsonData[key] = value;
  }

  std::vector<TemplateMapping> mappings;
  for (const auto &entry : cfg->template_outputs()) {
    if (!entry.template_filename().empty() &&
        (!entry.has_enabled() || entry.enabled())) {
      mappings.push_back({entry.template_filename(), entry.output_filepath()});
    }
  }

  std::string pythonScript = "";
  bool pyEnabled = isExportPythonEnabled(cfg);
  if (pyEnabled && !cfg->pythonscript().empty()) {
    pythonScript = cfg->pythonscript();
  }

  GenerationReport report =
      cg.generate(jsonData, template_folder, default_output, mappings,
                  baseDir.toStdString(), pythonScript);

  if (!report.errors.empty()) {
    QString errorMsg = tr("Code generation completed with errors:\n\n");
    for (const auto &err : report.errors) {
      errorMsg += QString("• %1: %2\n")
                      .arg(QString::fromStdString(err.first),
                           QString::fromStdString(err.second));
    }
    if (!report.success_files.empty()) {
      errorMsg += tr("\nSuccessfully generated files:\n");
      for (const auto &f : report.success_files) {
        errorMsg += QString("• %1\n").arg(QString::fromStdString(f));
      }
    }
    QMessageBox::warning(this, tr("Export Issues Detected"), errorMsg,
                         QMessageBox::Ok);
  } else if (report.success_files.empty()) {
    QMessageBox::information(
        this, tr("Export Notice"),
        tr("No template files were found or specified for generation.\n"
           "Please check template paths in the Configuration dialog."),
        QMessageBox::Ok);
  } else {
    QString successMsg =
        tr("Code generation completed successfully!\n\nGenerated files:\n");
    for (const auto &f : report.success_files) {
      successMsg += QString("• %1\n").arg(QString::fromStdString(f));
    }
    QMessageBox::information(this, tr("Export Successful"), successMsg,
                             QMessageBox::Ok);
  }

  delete cfg;
}

void RegMapWindow::btnDeleteItem(void) {
  QModelIndex index = this->treeView->currentIndex();
  if (index.isValid() && index.row() >= 0) {
    QModelIndex source_index = m_treeProxy->mapToSource(index);
    m_undoStack->push(new DeleteItemCommand(m_model, source_index.row(),
                                            source_index.parent()));
  }
}

bool RegMapWindow::isModelLoaded() const {
  return (m_model != nullptr &&
          m_model->rowCount() >
              0); // GCOV_EXCL_BR_LINE - m_model guaranteed non-null
}

void RegMapWindow::updatePaneVisibility(void) {
  const bool modelLoaded = isModelLoaded();
  if (m_leftStackedWidget && m_leftViewWidget &&
      m_leftEmptyWidget) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_leftStackedWidget->setCurrentWidget(modelLoaded ? m_leftViewWidget
                                                      : m_leftEmptyWidget);
  }
  if (!modelLoaded && m_rightStackedWidget &&
      m_emptyViewWidget) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_rightStackedWidget->setCurrentWidget(m_emptyViewWidget);
  }
}

void RegMapWindow::connectModelSignals(void) {
  if (!m_model)
    return; // GCOV_EXCL_LINE - Defensive invariant
  connect(
      m_model, &RegMapTreeModel::dataChanged, this,
      [this](const QModelIndex &topLeft, const QModelIndex &bottomRight) {
        Q_UNUSED(topLeft);
        Q_UNUSED(bottomRight);
        regmap_modified();
        if (m_bitfieldBar) { // GCOV_EXCL_BR_LINE - Defensive invariant
          m_bitfieldBar->refresh();
        }
        if (m_blockMemoryMapWidget) { // GCOV_EXCL_BR_LINE - Defensive invariant
          m_blockMemoryMapWidget->refresh();
        }
        if (m_currentRegItem) {
          m_updatingRegHeader = true;
          if (m_regNameEdit && !m_regNameEdit->hasFocus())
            m_regNameEdit->setText(m_currentRegItem->data("Name").toString());
          if (m_regOffsetEdit && !m_regOffsetEdit->hasFocus())
            m_regOffsetEdit->setText(padHexOffsetString(
                m_currentRegItem->data("Offset/LSB").toString()));
          if (m_regSizeEdit && !m_regSizeEdit->hasFocus())
            m_regSizeEdit->setText(
                m_currentRegItem->data("Size/Width").toString());
          if (m_regSwAccessCombo && !m_regSwAccessCombo->hasFocus()) {
            int idx = m_regSwAccessCombo->findText(
                m_currentRegItem->data("SW Access").toString());
            if (idx >= 0)
              m_regSwAccessCombo->setCurrentIndex(idx);
          }
          if (m_regHwAccessCombo && !m_regHwAccessCombo->hasFocus()) {
            int idx = m_regHwAccessCombo->findText(
                m_currentRegItem->data("HW Access").toString());
            if (idx >= 0)
              m_regHwAccessCombo->setCurrentIndex(idx);
          }
          if (m_regResetEdit && !m_regResetEdit->hasFocus())
            m_regResetEdit->setText(padHexOffsetString(
                m_currentRegItem->data("Reset Value").toString()));
          if (m_regDescEdit && !m_regDescEdit->hasFocus())
            m_regDescEdit->setText(
                m_currentRegItem->data("Description").toString());
          if (m_regWrLockEdit && !m_regWrLockEdit->hasFocus())
            m_regWrLockEdit->setText(
                m_currentRegItem->data("Write Lock").toString());
          if (m_regRdLockEdit && !m_regRdLockEdit->hasFocus())
            m_regRdLockEdit->setText(
                m_currentRegItem->data("Read Lock").toString());
          if (m_regDecodeOnlyCheck && !m_regDecodeOnlyCheck->hasFocus())
            m_regDecodeOnlyCheck->setChecked(
                m_currentRegItem->data("Decode Only").toString().toLower() ==
                "true");
          if (m_regHasResetCheck && !m_regHasResetCheck->hasFocus())
            m_regHasResetCheck->setChecked(
                m_currentRegItem->data("Has Reset").toString().toLower() ==
                "true");
          if (m_regRandCheck && !m_regRandCheck->hasFocus())
            m_regRandCheck->setChecked(
                m_currentRegItem->data("Is Rand").toString().toLower() ==
                "true");
          if (m_regVolatileCheck && !m_regVolatileCheck->hasFocus())
            m_regVolatileCheck->setChecked(
                m_currentRegItem->data("Volatile").toString().toLower() ==
                "true");
          m_updatingRegHeader = false;
        }
        if (m_currentBlkItem) {
          m_updatingBlkHeader = true;
          if (m_blkNameEdit && !m_blkNameEdit->hasFocus())
            m_blkNameEdit->setText(m_currentBlkItem->data("Name").toString());
          if (m_blkOffsetEdit && !m_blkOffsetEdit->hasFocus())
            m_blkOffsetEdit->setText(padHexOffsetString(
                m_currentBlkItem->data("Offset/LSB").toString()));
          if (m_blkDescEdit && !m_blkDescEdit->hasFocus())
            m_blkDescEdit->setText(
                m_currentBlkItem->data("Description").toString());
          if (m_blkWrLockEdit && !m_blkWrLockEdit->hasFocus())
            m_blkWrLockEdit->setText(
                m_currentBlkItem->data("Write Lock").toString());
          if (m_blkRdLockEdit && !m_blkRdLockEdit->hasFocus())
            m_blkRdLockEdit->setText(
                m_currentBlkItem->data("Read Lock").toString());
          m_updatingBlkHeader = false;
        }
        if (m_currentMemItem) {
          m_updatingMemHeader = true;
          if (m_memNameEdit && !m_memNameEdit->hasFocus())
            m_memNameEdit->setText(m_currentMemItem->data("Name").toString());
          if (m_memOffsetEdit && !m_memOffsetEdit->hasFocus())
            m_memOffsetEdit->setText(padHexOffsetString(
                m_currentMemItem->data("Offset/LSB").toString()));
          if (m_memSizeEdit && !m_memSizeEdit->hasFocus())
            m_memSizeEdit->setText(
                m_currentMemItem->data("Size/Width").toString());
          if (m_memSwAccessCombo && !m_memSwAccessCombo->hasFocus()) {
            int idx = m_memSwAccessCombo->findText(
                m_currentMemItem->data("SW Access").toString());
            if (idx >= 0)
              m_memSwAccessCombo->setCurrentIndex(idx);
            // GCOV_EXCL_START - Defensive fallback
            else
              m_memSwAccessCombo->setCurrentText(
                  m_currentMemItem->data("SW Access").toString());
            // GCOV_EXCL_STOP
          }
          if (m_memHwAccessCombo && !m_memHwAccessCombo->hasFocus()) {
            QString hwVal = m_currentMemItem->data("HW Access").toString();
            // GCOV_EXCL_START - Defensive fallback
            if (hwVal.isEmpty())
              hwVal = m_currentMemItem->data("HW Access Policy").toString();
            // GCOV_EXCL_STOP
            int idx = m_memHwAccessCombo->findText(hwVal);
            if (idx >= 0)
              m_memHwAccessCombo->setCurrentIndex(idx);
            // GCOV_EXCL_START - Defensive fallback
            else
              m_memHwAccessCombo->setCurrentText(hwVal);
            // GCOV_EXCL_STOP
          }
          if (m_memDescEdit && !m_memDescEdit->hasFocus())
            m_memDescEdit->setText(
                m_currentMemItem->data("Description").toString());
          if (m_memWordWidthEdit && !m_memWordWidthEdit->hasFocus())
            m_memWordWidthEdit->setText(
                m_currentMemItem->data("Word Width").toString());
          if (m_memDepthEdit && !m_memDepthEdit->hasFocus())
            m_memDepthEdit->setText(m_currentMemItem->data("Depth").toString());
          if (m_memHdlPathEdit && !m_memHdlPathEdit->hasFocus())
            m_memHdlPathEdit->setText(
                m_currentMemItem->data("HDL Path").toString());
          if (m_memWrLockEdit && !m_memWrLockEdit->hasFocus())
            m_memWrLockEdit->setText(
                m_currentMemItem->data("Write Lock").toString());
          if (m_memRdLockEdit && !m_memRdLockEdit->hasFocus())
            m_memRdLockEdit->setText(
                m_currentMemItem->data("Read Lock").toString());
          if (m_memNoTestCheck && !m_memNoTestCheck->hasFocus())
            m_memNoTestCheck->setChecked(
                m_currentMemItem->data("NO_MEM_TEST").toString().toLower() ==
                    "true" ||
                m_currentMemItem->data("No Mem Test").toString().toLower() ==
                    "true");
          if (m_memNoWalkTestCheck && !m_memNoWalkTestCheck->hasFocus())
            m_memNoWalkTestCheck->setChecked(
                m_currentMemItem->data("NO_MEM_WALK_TEST")
                        .toString()
                        .toLower() == "true" ||
                m_currentMemItem->data("No Walk Test").toString().toLower() ==
                    "true");
          if (m_memNoAccessTestCheck && !m_memNoAccessTestCheck->hasFocus())
            m_memNoAccessTestCheck->setChecked(
                m_currentMemItem->data("NO_MEM_ACCESS_TEST")
                        .toString()
                        .toLower() == "true" ||
                m_currentMemItem->data("No Access Test").toString().toLower() ==
                    "true");
          m_updatingMemHeader = false;
          updateMemSummary();
        }
      });
  connect(
      m_model, &QAbstractItemModel::rowsInserted, this,
      [this](const QModelIndex &parent, int first, int last) {
        Q_UNUSED(parent);
        Q_UNUSED(first);
        Q_UNUSED(last);
        regmap_modified();
        updatePaneVisibility();
        if (m_bitfieldBar) { // GCOV_EXCL_BR_LINE - Defensive invariant
          m_bitfieldBar->refresh();
        }
        if (m_blockMemoryMapWidget) { // GCOV_EXCL_BR_LINE - Defensive invariant
          m_blockMemoryMapWidget->refresh();
        }
      });
  connect(
      m_model, &QAbstractItemModel::rowsRemoved, this,
      [this](const QModelIndex &parent, int first, int last) {
        Q_UNUSED(parent);
        Q_UNUSED(first);
        Q_UNUSED(last);
        regmap_modified();
        updatePaneVisibility();
        if (m_bitfieldBar) { // GCOV_EXCL_BR_LINE - Defensive invariant
          m_bitfieldBar->refresh();
        }
        if (m_blockMemoryMapWidget) { // GCOV_EXCL_BR_LINE - Defensive invariant
          m_blockMemoryMapWidget->refresh();
        }
      });
  connect(m_model, &QAbstractItemModel::modelReset, this, [this]() {
    updatePaneVisibility();
    if (m_bitfieldBar) { // GCOV_EXCL_BR_LINE - Defensive invariant
      m_bitfieldBar->refresh();
    }
    if (m_blockMemoryMapWidget) { // GCOV_EXCL_BR_LINE - Defensive invariant
      m_blockMemoryMapWidget->refresh();
    }
  });
}

void RegMapWindow::connectFieldsTableSignals(void) {
  if (!m_fieldsTableView || !m_fieldsTableView->selectionModel())
    return; // GCOV_EXCL_LINE - Defensive invariant

  disconnect(m_fieldsTableView->selectionModel(), nullptr, this, nullptr);

  connect(m_fieldsTableView->selectionModel(),
          &QItemSelectionModel::currentRowChanged, this,
          [this](const QModelIndex &curr, const QModelIndex &prev) {
            Q_UNUSED(prev);
            if (curr.isValid()) {
              QModelIndex sourceIdx = m_fieldProxy->mapToSource(curr);
              m_bitfieldBar->setSelectedField(sourceIdx.row());
            } else {
              m_bitfieldBar->setSelectedField(-1);
            }
          });

  connect(
      m_fieldsTableView->selectionModel(),
      &QItemSelectionModel::selectionChanged, this,
      [this](const QItemSelection &selected, const QItemSelection &deselected) {
        Q_UNUSED(deselected);
        if (selected.isEmpty()) {
          if (m_bitfieldBar)
            m_bitfieldBar->setSelectedField(
                -1); // GCOV_EXCL_BR_LINE - Defensive invariant
          return;
        }
        QModelIndex firstIdx = selected.indexes().value(0);
        if (firstIdx.isValid()) { // GCOV_EXCL_BR_LINE - Non-empty selection
                                  // index is guaranteed valid
          QModelIndex sourceIdx = m_fieldProxy->mapToSource(firstIdx);
          m_bitfieldBar->setSelectedField(sourceIdx.row());
        }
      });
}

void RegMapWindow::fileNew(void) {
  m_model->clear();
  delete m_model;
  m_model = new RegMapTreeModel(this);
  connectModelSignals();

  m_treeProxy->setSourceModel(m_model);
  m_treeProxy->setSearchFilter(QString());
  m_fieldProxy->setSourceModel(m_model);
  this->treeView->setModel(m_treeProxy);
  this->treeView->setSortingEnabled(true);
  this->treeView->sortByColumn(1, Qt::AscendingOrder);
  this->treeView->clearSelection();
  this->treeView->setCurrentIndex(QModelIndex());
  this->treeView->reset();

  connect(this->treeView->selectionModel(),
          &QItemSelectionModel::currentChanged, this,
          &RegMapWindow::updateFieldsTable);

  if (m_searchEdit) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_searchEdit->clear();
  }

  if (m_fieldsTableView) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_fieldsTableView->setModel(m_fieldProxy);
    m_fieldsTableView->setRootIndex(QModelIndex());
    connectFieldsTableSignals();
  }
  if (m_bitfieldBar) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_bitfieldBar->clear();
  }
  if (m_blockMemoryMapWidget) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_blockMemoryMapWidget->clear();
  }
  if (m_regNameEdit)
    m_regNameEdit->clear(); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_regOffsetEdit)
    m_regOffsetEdit->clear(); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_regSizeEdit)
    m_regSizeEdit->clear(); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_regResetEdit)
    m_regResetEdit->clear(); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_regDescEdit)
    m_regDescEdit->clear(); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_regWrLockEdit)
    m_regWrLockEdit->clear(); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_regRdLockEdit)
    m_regRdLockEdit->clear(); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_regDecodeOnlyCheck)
    m_regDecodeOnlyCheck->setChecked(
        false); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_regHasResetCheck)
    m_regHasResetCheck->setChecked(
        false); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_regRandCheck)
    m_regRandCheck->setChecked(
        false); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_regVolatileCheck)
    m_regVolatileCheck->setChecked(
        false); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_blkNameEdit)
    m_blkNameEdit->clear(); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_blkOffsetEdit)
    m_blkOffsetEdit->clear(); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_blkDescEdit)
    m_blkDescEdit->clear(); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_blkWrLockEdit)
    m_blkWrLockEdit->clear(); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_blkRdLockEdit)
    m_blkRdLockEdit->clear(); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_memNameEdit)
    m_memNameEdit->clear(); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_memOffsetEdit)
    m_memOffsetEdit->clear(); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_memSizeEdit)
    m_memSizeEdit->clear(); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_memDescEdit)
    m_memDescEdit->clear(); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_memWordWidthEdit)
    m_memWordWidthEdit->clear(); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_memDepthEdit)
    m_memDepthEdit->clear(); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_memHdlPathEdit)
    m_memHdlPathEdit->clear(); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_memWrLockEdit)
    m_memWrLockEdit->clear(); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_memRdLockEdit)
    m_memRdLockEdit->clear(); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_memNoTestCheck)
    m_memNoTestCheck->setChecked(
        false); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_memNoWalkTestCheck)
    m_memNoWalkTestCheck->setChecked(
        false); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_memNoAccessTestCheck)
    m_memNoAccessTestCheck->setChecked(
        false); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_memSummaryLabel)
    m_memSummaryLabel->clear(); // GCOV_EXCL_BR_LINE - Defensive invariant
  m_currentRegItem = nullptr;
  m_currentBlkItem = nullptr;
  m_currentMemItem = nullptr;

  updatePaneVisibility();

  if (m_undoStack) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_undoStack->clear();
  }

  this->regmap_notModified();
  this->m_rmap_filename = QString();
  this->setWindowTitle(this->m_default_window_title);

  if (m_config_window) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_config_window->deserialize(protormap::Config());
  }
}

void RegMapWindow::fileOpen(QString fname) {
  QString expanded = PathUtils::expandEnvVars(fname);
  QFileInfo check_file(expanded);
  if (check_file.exists() && check_file.isFile()) {
    fileNew();
    this->m_rmap_filename =
        PathUtils::normalizeSeparators(check_file.filePath());
    m_config_window->setBaseDir(check_file.absolutePath());

    FormatResult res =
        FormatManager::instance().loadFile(expanded, m_model, m_config_window);
    if (!res.success) {
      fileNew();
      QMessageBox::warning(
          this, tr("Open Error"),
          tr("Failed to open %1:\n%2").arg(fname, res.errorMessage));
      return;
    }

    this->regmap_notModified();
    this->setWindowTitle(this->m_default_window_title + " (" +
                         this->m_rmap_filename + ")");

    updatePaneVisibility();

    if (this->treeView->model() && this->treeView->model()->rowCount() > 0) {
      this->treeView->expandAll();
      QModelIndex firstIdx = this->treeView->model()->index(0, 0);
      this->treeView->setCurrentIndex(firstIdx);
    }
  } else {
    QMessageBox::critical(this, tr("Error file not found"),
                          tr("Filename: %1 not found").arg(fname),
                          QMessageBox::Ok);
  }
}

protormap::RegModel &operator<<(protormap::RegModel &reg_model,
                                const SerializationContext &context) {
  for (SerializationContext::Record rec : context.m_records) {
    protormap::RegItem *item = reg_model.add_item();
    switch (rec.m_data["kind"].value<RegMapTreeItem::e_rmmKind>()) {
    case RegMapTreeItem::e_rmmKind::root:
      item->set_kind(protormap::RegItem_Kind_ROOT);
      break;
    case RegMapTreeItem::e_rmmKind::mem:
      item->set_kind(protormap::RegItem_Kind_MEM);
      break;
    case RegMapTreeItem::e_rmmKind::map:
      item->set_kind(protormap::RegItem_Kind_MAP);
      break;
    case RegMapTreeItem::e_rmmKind::blk:
      item->set_kind(protormap::RegItem_Kind_BLK);
      break;
    case RegMapTreeItem::e_rmmKind::reg:
      item->set_kind(protormap::RegItem_Kind_REG);
      break;
    case RegMapTreeItem::e_rmmKind::fld:
      item->set_kind(protormap::RegItem_Kind_FLD);
      break;
    }
    item->set_id(rec.m_data["id"].toUInt());
    item->set_parent_id(rec.m_data["parent"].toUInt());
    QList<QVariant> childItems = rec.m_data["childItems"].toList();
    for (int i = 0; i < childItems.size(); i++) {
      item->add_child_id(childItems.at(i).toUInt());
    }
    QVariantMap itemData = rec.m_data["itemData"].toMap();
    auto &itd = *item->mutable_itemdata();
    for (auto key : itemData.keys()) {
      itd[key.toStdString()] = itemData.value(key).toString().toStdString();
    }
  }
  return (reg_model);
}

protormap::RegModel &operator>>(protormap::RegModel &reg_model,
                                SerializationContext &context) {
  for (int j = 0; j < reg_model.item_size(); j++) {
    QVariantMap m_data;
    QVariantMap itemData;
    QList<QVariant> childItemsList;
    QObject *object = NULL;

    const protormap::RegItem &item = reg_model.item(j);
    switch (item.kind()) {
    case protormap::RegItem_Kind_ROOT:
      m_data["kind"] = QVariant::fromValue(RegMapTreeItem::e_rmmKind::root);
      break;
    case protormap::RegItem_Kind_MEM:
      m_data["kind"] = QVariant::fromValue(RegMapTreeItem::e_rmmKind::mem);
      break;
    case protormap::RegItem_Kind_MAP:
      m_data["kind"] = QVariant::fromValue(RegMapTreeItem::e_rmmKind::map);
      break;
    case protormap::RegItem_Kind_BLK:
      m_data["kind"] = QVariant::fromValue(RegMapTreeItem::e_rmmKind::blk);
      break;
    case protormap::RegItem_Kind_REG:
      m_data["kind"] = QVariant::fromValue(RegMapTreeItem::e_rmmKind::reg);
      break;
    case protormap::RegItem_Kind_FLD:
      m_data["kind"] = QVariant::fromValue(RegMapTreeItem::e_rmmKind::fld);
      break;
    default:;
    }
    m_data["id"] = item.id();
    if (item.kind() == protormap::RegItem_Kind_ROOT ||
        (item.parent_id() == item.id() && item.id() == 0)) {
      m_data["parent"] = QVariant();
    } else {
      m_data["parent"] = item.parent_id();
    }
    for (const auto &child : item.child_id()) {
      childItemsList.append(child);
    }
    m_data["childItems"] = childItemsList;

    for (const auto &[key, value] : item.itemdata()) {
      itemData[QString(key.c_str())] = QVariant(value.c_str());
    }
    if (itemData.contains("Access Policy") && !itemData.contains("SW Access")) {
      itemData["SW Access"] = itemData["Access Policy"];
    } else if (itemData.contains("SW Access") &&
               !itemData.contains("Access Policy")) {
      itemData["Access Policy"] = itemData["SW Access"];
    }
    m_data["itemData"] = QVariant(itemData);

    context.append_record(object, m_data);
  }
  return (reg_model);
}

bool RegMapWindow::fileSave(QString fname) {
  if (fname.isEmpty()) {
    fname = m_rmap_filename;
  }
  if (fname.isEmpty()) {
    return btnFileSaveAs();
  }

  QString expanded = PathUtils::expandEnvVars(fname);
  QFileInfo saveFi(expanded);
  m_config_window->setBaseDir(saveFi.absolutePath());

  FormatResult res =
      FormatManager::instance().saveFile(expanded, m_model, m_config_window);
  if (!res.success) {
    if (this->isVisible()) {
      QMessageBox::critical(
          this, tr("Failed to write output file"),
          tr("Error writing to file: %1\n%2").arg(fname, res.errorMessage),
          QMessageBox::Ok);
    } else {
      std::cerr << "Error writing to file: " << fname.toStdString() << " - "
                << res.errorMessage.toStdString() << std::endl;
    }
    return false;
  }

  this->m_rmap_filename = PathUtils::normalizeSeparators(saveFi.filePath());
  regmap_notModified();
  return true;
}

void RegMapWindow::regmap_modified(void) {
  if (!m_is_regmap_modified) {
    m_is_regmap_modified = true;
    QString win_title = this->windowTitle();
    if (!win_title.endsWith('*')) {
      win_title.append('*');
    }
    this->setWindowTitle(win_title);
  }
}

void RegMapWindow::regmap_notModified(void) {
  if (m_is_regmap_modified) {
    m_is_regmap_modified = false;
    QString win_title = this->windowTitle();
    if (win_title.endsWith('*')) {
      win_title.remove(win_title.size() - 1, 1);
    }
    this->setWindowTitle(win_title);
  }
}

void RegMapWindow::insertChild(RegMapTreeItem::e_rmmKind kind) {
  QModelIndexList indexes = this->treeView->selectionModel()->selectedIndexes();
  QModelIndex index;
  if (indexes.size() > 0) {
    index = indexes.at(0);
  } else {
    index = this->treeView->selectionModel()->currentIndex();
  }
  QModelIndex source_index =
      index.isValid() ? m_treeProxy->mapToSource(index) : QModelIndex();

  RegMapTreeItem *targetParentItem = nullptr;
  QModelIndex targetParentSource;
  int insertRow = 0;

  if (source_index.isValid()) {
    RegMapTreeItem *selectedItem = m_model->getItem(source_index);
    if (selectedItem) {
      if (selectedItem->possibleChildren().contains(kind)) {
        targetParentItem = selectedItem;
        targetParentSource = source_index;
        insertRow = selectedItem->childCount();
      } else {
        // Ascend ancestor hierarchy to find the nearest parent capable of
        // accepting 'kind'
        RegMapTreeItem *curr = selectedItem;
        QModelIndex currSource = source_index;
        while (curr && curr->parentItem() && curr->parentItem() != curr) {
          RegMapTreeItem *p = curr->parentItem();
          QModelIndex pSource = currSource.parent();
          if (p->possibleChildren().contains(kind)) {
            targetParentItem = p;
            targetParentSource = pSource;
            insertRow = curr->row() +
                        1; // Insert as sibling immediately following current
            break;
          }
          curr = p;
          currSource = pSource;
        }
      }
    }
  }

  if (!targetParentItem) {
    // Fallback to root or top-level item if possible
    RegMapTreeItem *root = m_model->getRootItem();
    if (root && root->possibleChildren().contains(kind)) {
      targetParentItem = root;
      targetParentSource = QModelIndex();
      insertRow = root->childCount();
    } else if (root && root->childCount() > 0) {
      // Check if first child (e.g. first block) can accept it
      RegMapTreeItem *firstChild = root->child(0);
      if (firstChild && firstChild->possibleChildren().contains(kind)) {
        targetParentItem = firstChild;
        targetParentSource = m_model->index(0, 0, QModelIndex());
        insertRow = firstChild->childCount();
      }
    }
  }

  if (!targetParentItem) {
    return;
  }

  m_undoStack->push(
      new InsertItemCommand(m_model, kind, insertRow, targetParentSource));

  for (int col = 0; col < m_treeProxy->columnCount(); col++) {
    this->treeView->resizeColumnToContents(col);
    if (this->treeView->header()) {
      int minHeader = this->treeView->header()->sectionSizeHint(col);
      if (this->treeView->columnWidth(col) < minHeader) {
        this->treeView->setColumnWidth(col, minHeader);
      }
    }
  }

  // Expand parent in treeView
  if (targetParentSource.isValid()) {
    QModelIndex parentProxy = m_treeProxy->mapFromSource(targetParentSource);
    if (parentProxy.isValid()) {
      this->treeView->setExpanded(parentProxy, true);
    }
  }

  // Automatically navigate / move to the newly created item
  QModelIndex newSourceIdx = m_model->index(insertRow, 0, targetParentSource);
  if (newSourceIdx.isValid()) {
    QModelIndex newProxyIdx = m_treeProxy->mapFromSource(newSourceIdx);
    if (newProxyIdx.isValid()) {
      this->treeView->setCurrentIndex(newProxyIdx);
      this->treeView->selectionModel()->setCurrentIndex(
          newProxyIdx,
          QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
      this->treeView->scrollTo(newProxyIdx, QAbstractItemView::EnsureVisible);
      updateFieldsTable(newProxyIdx, QModelIndex());
    }
  }

  // For fields, also ensure the fields table highlights the new field
  if (kind == RegMapTreeItem::e_rmmKind::fld && m_fieldsTableView &&
      m_fieldProxy) {
    QModelIndex newFldSource = m_model->index(insertRow, 0, targetParentSource);
    QModelIndex newFldProxy = m_fieldProxy->mapFromSource(newFldSource);
    if (newFldProxy.isValid()) {
      m_fieldsTableView->setCurrentIndex(newFldProxy);
      m_fieldsTableView->selectionModel()->setCurrentIndex(
          newFldProxy,
          QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
      m_fieldsTableView->scrollTo(newFldProxy,
                                  QAbstractItemView::EnsureVisible);
    }
  }
}

void RegMapWindow::updateFieldsTable(const QModelIndex &current,
                                     const QModelIndex &previous) {
  Q_UNUSED(previous);

  if (!current.isValid()) {
    if (m_fieldsTableView)
      m_fieldsTableView->setRootIndex(
          QModelIndex()); // GCOV_EXCL_BR_LINE - Defensive invariant
    if (m_bitfieldBar)
      m_bitfieldBar->clear(); // GCOV_EXCL_BR_LINE - Defensive invariant
    m_currentRegItem = nullptr;
    m_currentBlkItem = nullptr;
    m_currentMemItem = nullptr;
    if (m_rightStackedWidget &&
        m_emptyViewWidget) { // GCOV_EXCL_BR_LINE - Defensive invariant
      m_rightStackedWidget->setCurrentWidget(m_emptyViewWidget);
    }
    return;
  }

  QModelIndex source_current = m_treeProxy->mapToSource(current);
  QModelIndex source_col0 =
      m_model->index(source_current.row(), 0, source_current.parent());
  RegMapTreeItem *item = m_model->getItem(source_col0);
  if (item->kindString() == "reg") {
    m_currentRegItem = item;
    m_currentBlkItem = nullptr;
    m_currentMemItem = nullptr;
    if (m_regHeaderWidget) {
      m_regHeaderWidget->setVisible(true);
      m_updatingRegHeader = true;
      if (m_regNameEdit)
        m_regNameEdit->setText(
            item->data("Name")
                .toString()); // GCOV_EXCL_BR_LINE - Defensive invariant
      if (m_regOffsetEdit)
        m_regOffsetEdit->setText(padHexOffsetString(
            item->data("Offset/LSB")
                .toString())); // GCOV_EXCL_BR_LINE - Defensive invariant
      if (m_regSizeEdit)
        m_regSizeEdit->setText(
            item->data("Size/Width")
                .toString()); // GCOV_EXCL_BR_LINE - Defensive invariant
      if (m_regSwAccessCombo) {
        int idx =
            m_regSwAccessCombo->findText(item->data("SW Access").toString());
        if (idx >= 0)
          m_regSwAccessCombo->setCurrentIndex(idx);
        else
          m_regSwAccessCombo->setCurrentText(
              item->data("SW Access").toString());
      }
      if (m_regHwAccessCombo) {
        int idx =
            m_regHwAccessCombo->findText(item->data("HW Access").toString());
        if (idx >= 0)
          m_regHwAccessCombo->setCurrentIndex(idx);
        else
          m_regHwAccessCombo->setCurrentText(
              item->data("HW Access").toString());
      }
      if (m_regResetEdit)
        m_regResetEdit->setText(padHexOffsetString(
            item->data("Reset Value")
                .toString())); // GCOV_EXCL_BR_LINE - Defensive invariant
      if (m_regDescEdit)
        m_regDescEdit->setText(
            item->data("Description")
                .toString()); // GCOV_EXCL_BR_LINE - Defensive invariant
      if (m_regWrLockEdit)
        m_regWrLockEdit->setText(
            item->data("Write Lock")
                .toString()); // GCOV_EXCL_BR_LINE - Defensive invariant
      if (m_regRdLockEdit)
        m_regRdLockEdit->setText(
            item->data("Read Lock")
                .toString()); // GCOV_EXCL_BR_LINE - Defensive invariant
      if (m_regDecodeOnlyCheck)
        m_regDecodeOnlyCheck->setChecked(
            item->data("Decode Only").toString().toLower() ==
            "true"); // GCOV_EXCL_BR_LINE - Defensive invariant
      if (m_regHasResetCheck)
        m_regHasResetCheck->setChecked(
            item->data("Has Reset").toString().toLower() ==
            "true"); // GCOV_EXCL_BR_LINE - Defensive invariant
      if (m_regRandCheck)
        m_regRandCheck->setChecked(
            item->data("Is Rand").toString().toLower() ==
            "true"); // GCOV_EXCL_BR_LINE - Defensive invariant
      if (m_regVolatileCheck)
        m_regVolatileCheck->setChecked(
            item->data("Volatile").toString().toLower() ==
            "true"); // GCOV_EXCL_BR_LINE - Defensive invariant
      m_updatingRegHeader = false;
    }

    if (m_fieldsTableView) { // GCOV_EXCL_BR_LINE - Defensive invariant
      if (m_fieldsTableView->model() != m_fieldProxy) {
        m_fieldsTableView->setModel(m_fieldProxy);
        connectFieldsTableSignals();
      }
      m_fieldsTableView->setRootIndex(m_fieldProxy->mapFromSource(source_col0));
      if (m_layoutMode == "table") {
        m_fieldsTableView->setColumnHidden(0, true);
        for (int col = 1; col < m_model->columnCount(); ++col) {
          m_fieldsTableView->setColumnHidden(col, false);
          m_fieldsTableView->resizeColumnToContents(col);
          int minHeader =
              m_fieldsTableView->horizontalHeader()->sectionSizeHint(col);
          if (m_fieldsTableView->columnWidth(col) < minHeader) {
            m_fieldsTableView->setColumnWidth(col, minHeader);
          }
        }
        int descCol = m_model->columnOf("Description");
        if (descCol < 0)
          descCol = 13; // GCOV_EXCL_LINE - Defensive column fallback
        m_fieldsTableView->horizontalHeader()->setStretchLastSection(true);
        m_fieldsTableView->horizontalHeader()->setSectionResizeMode(
            descCol, QHeaderView::Stretch);
      } else {
        applyFieldTabColumnFilter(m_regTabBar ? m_regTabBar->currentIndex()
                                              : 0);
      }
    }

    uint32_t regWidth = 32;
    if (m_config_window) { // GCOV_EXCL_BR_LINE - Defensive invariant
      protormap::Config *cfg = m_config_window->serialize();
      if (cfg && cfg->reg_width() > 0)
        regWidth = cfg->reg_width();
      delete cfg;
    }

    if (m_bitfieldBar) { // GCOV_EXCL_BR_LINE - Defensive invariant
      m_bitfieldBar->setRegister(item, regWidth);
    }

    if (m_rightStackedWidget &&
        m_regViewWidget) { // GCOV_EXCL_BR_LINE - Defensive invariant
      m_rightStackedWidget->setCurrentWidget(m_regViewWidget);
    }
  } else if (item->kindString() == "blk" || item->kindString() == "map") {
    m_currentMemItem = nullptr;
    updateBlockView(item);
  } else if (item->kindString() == "mem") {
    m_currentRegItem = nullptr;
    m_currentBlkItem = nullptr;
    m_currentMemItem = item;
    updateMemView(item);
  // GCOV_EXCL_START - Defensive fallback for unhandled item kinds
  } else {
    m_currentRegItem = nullptr;
    m_currentBlkItem = nullptr;
    m_currentMemItem = nullptr;
    if (m_rightStackedWidget &&
        m_emptyViewWidget) { // GCOV_EXCL_BR_LINE - Defensive invariant
      m_rightStackedWidget->setCurrentWidget(m_emptyViewWidget);
    }
  }
  // GCOV_EXCL_STOP
}

void RegMapWindow::applyFieldTabColumnFilter(int tabIndex) {
  if (!m_fieldsTableView || !m_model)
    return; // GCOV_EXCL_LINE - Defensive invariant

  if (tabIndex == 3) {
    if (m_regTabStack)
      m_regTabStack->setCurrentIndex(1);
    if (m_currentRegItem && m_regBlockMemoryMapWidget) {
      RegMapTreeItem *parentBlk = m_currentRegItem->parentItem();
      if (parentBlk) {
        uint32_t regWidth = 32;
        if (m_config_window) {
          protormap::Config *cfg = m_config_window->serialize();
          if (cfg && cfg->reg_width() > 0)
            regWidth = cfg->reg_width();
          delete cfg;
        }
        m_regBlockMemoryMapWidget->setBlock(parentBlk, regWidth);
        m_regBlockMemoryMapWidget->setSelectedRegister(m_currentRegItem->row());
      }
    }
    return;
  }

  if (m_regTabStack)
    m_regTabStack->setCurrentIndex(0);

  int hwAccessCol = m_model->columnOf("HW Access Policy");
  int resetCol = m_model->columnOf("Reset Value");
  int randCol = m_model->columnOf("Is Rand");
  int volCol = m_model->columnOf("Volatile");
  int hasResetCol = m_model->columnOf("Has Reset");
  int wrLockCol = m_model->columnOf("Write Lock");
  int rdLockCol = m_model->columnOf("Read Lock");
  int decOnlyCol = m_model->columnOf("Decode Only");
  int descCol = m_model->columnOf("Description");

  // GCOV_EXCL_START - Defensive column fallbacks
  if (hwAccessCol < 0)
    hwAccessCol = 5;
  if (resetCol < 0)
    resetCol = 6;
  if (randCol < 0)
    randCol = 7;
  if (volCol < 0)
    volCol = 8;
  if (hasResetCol < 0)
    hasResetCol = 9;
  if (wrLockCol < 0)
    wrLockCol = 10;
  if (rdLockCol < 0)
    rdLockCol = 11;
  if (decOnlyCol < 0)
    decOnlyCol = 12;
  if (descCol < 0)
    descCol = 13;
  // GCOV_EXCL_STOP

  m_fieldsTableView->setColumnHidden(0, true);

  for (int col = 1; col < m_model->columnCount(); ++col) {
    bool hide = false;
    if (tabIndex == 0) {
      // Tab 0: Fields & Layout (Core view)
      // Visible: LSB, Size, Name, SW Access, Reset Value, Description
      if (col == hwAccessCol || col == randCol || col == volCol ||
          col == hasResetCol || col == wrLockCol || col == rdLockCol ||
          col == decOnlyCol) {
        hide = true;
      }
    } else if (tabIndex == 1) {
      // Tab 1: Verification & UVM
      // Visible: LSB, Size, Name, SW Access, HW Access, Reset Value, Is Rand,
      // Volatile, Has Reset
      if (col == wrLockCol || col == rdLockCol || col == decOnlyCol ||
          col == descCol) {
        hide = true;
      }
    } else if (tabIndex == 2) {
      // Tab 2: Security & Locks
      // Visible: LSB, Size, Name, SW Access, Write Lock, Read Lock, Decode
      // Only, Description
      if (col == hwAccessCol || col == resetCol || col == randCol ||
          col == volCol || col == hasResetCol) {
        hide = true;
      }
    }
    // Tab 4: All Properties (nothing hidden except col 0)

    m_fieldsTableView->setColumnHidden(col, hide);

    if (!hide) {
      m_fieldsTableView->resizeColumnToContents(col);
      int minHeader =
          m_fieldsTableView->horizontalHeader()->sectionSizeHint(col);
      if (m_fieldsTableView->columnWidth(col) < minHeader) {
        m_fieldsTableView->setColumnWidth(col, minHeader);
      }
    }
  }

  // Set the rightmost visible column to Stretch
  int lastVis = -1;
  for (int col = m_model->columnCount() - 1; col >= 1; --col) {
    if (!m_fieldsTableView->isColumnHidden(col)) {
      lastVis = col;
      break;
    }
  }
  if (lastVis >= 0) {
    m_fieldsTableView->horizontalHeader()->setStretchLastSection(true);
    m_fieldsTableView->horizontalHeader()->setSectionResizeMode(
        lastVis, QHeaderView::Stretch);
  }
}

void RegMapWindow::updateBlockView(RegMapTreeItem *blkItem) {
  if (!blkItem)
    return;
  m_currentBlkItem = blkItem;
  m_currentRegItem = nullptr;
  m_currentMemItem = nullptr;

  m_updatingBlkHeader = true;
  if (m_blkNameEdit)
    m_blkNameEdit->setText(
        blkItem->data("Name")
            .toString()); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_blkOffsetEdit)
    m_blkOffsetEdit->setText(padHexOffsetString(
        blkItem->data("Offset/LSB")
            .toString())); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_blkDescEdit)
    m_blkDescEdit->setText(
        blkItem->data("Description")
            .toString()); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_blkWrLockEdit)
    m_blkWrLockEdit->setText(
        blkItem->data("Write Lock")
            .toString()); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_blkRdLockEdit)
    m_blkRdLockEdit->setText(
        blkItem->data("Read Lock")
            .toString()); // GCOV_EXCL_BR_LINE - Defensive invariant
  m_updatingBlkHeader = false;

  uint32_t regWidth = 32;
  if (m_config_window) { // GCOV_EXCL_BR_LINE - Defensive invariant
    protormap::Config *cfg = m_config_window->serialize();
    if (cfg && cfg->reg_width() > 0)
      regWidth = cfg->reg_width();
    delete cfg;
  }

  if (m_blockMemoryMapWidget) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_blockMemoryMapWidget->setBlock(blkItem, regWidth);
  }

  if (m_rightStackedWidget &&
      m_blockViewWidget) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_rightStackedWidget->setCurrentWidget(m_blockViewWidget);
  }
}

void RegMapWindow::updateMemView(RegMapTreeItem *memItem) {
  if (!memItem)
    return; // GCOV_EXCL_LINE - Defensive invariant
  m_currentMemItem = memItem;
  m_currentRegItem = nullptr;
  m_currentBlkItem = nullptr;

  m_updatingMemHeader = true;
  if (m_memNameEdit)
    m_memNameEdit->setText(
        memItem->data("Name")
            .toString()); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_memOffsetEdit)
    m_memOffsetEdit->setText(padHexOffsetString(
        memItem->data("Offset/LSB")
            .toString())); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_memSizeEdit)
    m_memSizeEdit->setText(
        memItem->data("Size/Width")
            .toString()); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_memSwAccessCombo) {
    int idx =
        m_memSwAccessCombo->findText(memItem->data("SW Access").toString());
    if (idx >= 0)
      m_memSwAccessCombo->setCurrentIndex(idx);
    // GCOV_EXCL_START - Defensive fallback
    else
      m_memSwAccessCombo->setCurrentText(memItem->data("SW Access").toString());
    // GCOV_EXCL_STOP
  }
  if (m_memHwAccessCombo) {
    QString hwVal = memItem->data("HW Access").toString();
    // GCOV_EXCL_START - Defensive fallback
    if (hwVal.isEmpty())
      hwVal = memItem->data("HW Access Policy").toString();
    // GCOV_EXCL_STOP
    int idx = m_memHwAccessCombo->findText(hwVal);
    if (idx >= 0)
      m_memHwAccessCombo->setCurrentIndex(idx);
    // GCOV_EXCL_START - Defensive fallback
    else
      m_memHwAccessCombo->setCurrentText(hwVal);
    // GCOV_EXCL_STOP
  }
  if (m_memDescEdit)
    m_memDescEdit->setText(
        memItem->data("Description")
            .toString()); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_memWordWidthEdit)
    m_memWordWidthEdit->setText(
        memItem->data("Word Width")
            .toString()); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_memDepthEdit)
    m_memDepthEdit->setText(
        memItem->data("Depth")
            .toString()); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_memHdlPathEdit)
    m_memHdlPathEdit->setText(
        memItem->data("HDL Path")
            .toString()); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_memWrLockEdit)
    m_memWrLockEdit->setText(
        memItem->data("Write Lock")
            .toString()); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_memRdLockEdit)
    m_memRdLockEdit->setText(
        memItem->data("Read Lock")
            .toString()); // GCOV_EXCL_BR_LINE - Defensive invariant
  if (m_memNoTestCheck)
    m_memNoTestCheck->setChecked(
        memItem->data("NO_MEM_TEST").toString().toLower() == "true" ||
        memItem->data("No Mem Test").toString().toLower() == "true");
  if (m_memNoWalkTestCheck)
    m_memNoWalkTestCheck->setChecked(
        memItem->data("NO_MEM_WALK_TEST").toString().toLower() == "true" ||
        memItem->data("No Walk Test").toString().toLower() == "true");
  if (m_memNoAccessTestCheck)
    m_memNoAccessTestCheck->setChecked(
        memItem->data("NO_MEM_ACCESS_TEST").toString().toLower() == "true" ||
        memItem->data("No Access Test").toString().toLower() == "true");
  m_updatingMemHeader = false;

  updateMemSummary();

  if (m_rightStackedWidget &&
      m_memViewWidget) { // GCOV_EXCL_BR_LINE - Defensive invariant
    m_rightStackedWidget->setCurrentWidget(m_memViewWidget);
  }
}

void RegMapWindow::updateMemSummary() {
  if (!m_memSummaryLabel)
    return; // GCOV_EXCL_LINE - Defensive invariant
  if (!m_currentMemItem) {
    m_memSummaryLabel->clear();
    return;
  }

  uint64_t offset = parseNumericValue(m_currentMemItem->data("Offset/LSB"));
  uint64_t sizeBytes = parseNumericValue(m_currentMemItem->data("Size/Width"));
  if (sizeBytes == 0)
    sizeBytes = 1024;

  uint64_t endAddr = offset + (sizeBytes > 0 ? (sizeBytes - 1) : 0);

  uint32_t regWidth = 32;
  if (m_config_window) {
    protormap::Config *cfg = m_config_window->serialize();
    if (cfg && cfg->reg_width() > 0)
      regWidth = cfg->reg_width();
    delete cfg;
  }

  uint64_t wordWidth = parseNumericValue(m_currentMemItem->data("Word Width"));
  if (wordWidth == 0)
    wordWidth = regWidth;

  uint64_t wordBytes = (wordWidth >= 8 ? wordWidth : 32) / 8;
  uint64_t depth = parseNumericValue(m_currentMemItem->data("Depth"));
  if (depth == 0) {
    depth = (wordBytes > 0 && sizeBytes >= wordBytes)
                ? (sizeBytes / wordBytes)
                : (sizeBytes > 0 ? sizeBytes : 1024); // GCOV_EXCL_LINE - Defensive fallback for tiny memory depth
  }

  QString sizeHuman;
  if (sizeBytes >= 1024 * 1024) {
    sizeHuman = QString("%1 MB (%2 bytes)")
                    .arg(double(sizeBytes) / (1024.0 * 1024.0), 0, 'f', 2)
                    .arg(sizeBytes);
  } else if (sizeBytes >= 1024) {
    sizeHuman = QString("%1 KB (%2 bytes)")
                    .arg(double(sizeBytes) / 1024.0, 0, 'f', 2)
                    .arg(sizeBytes);
  } else {
    sizeHuman = QString("%1 bytes").arg(sizeBytes);
  }

  QString summary =
      QString(
          "<div style='font-family: sans-serif; line-height: 1.6;'>"
          "<h3 style='margin: 0 0 8px 0;'>Memory Region Details</h3>"
          "<table cellpadding='4' cellspacing='0' style='border-collapse: "
          "collapse; width: 100%;'>"
          "<tr><td><b>Address Range:</b></td><td><code>0x%1 - 0x%2</code></td>"
          "<td><b>Total Size:</b></td><td>%3 (<code>0x%4</code>)</td></tr>"
          "<tr><td><b>Word Width:</b></td><td>%5 bits</td>"
          "<td><b>Depth:</b></td><td>%6 words</td></tr>"
          "<tr><td><b>SW Access:</b></td><td>%7</td>"
          "<td><b>HW Access:</b></td><td>%8</td></tr>"
          "<tr><td><b>HDL Backdoor Path:</b></td><td "
          "colspan='3'><code>%9</code></td></tr>"
          "<tr><td><b>Write Lock:</b></td><td><code>%10</code></td>"
          "<td><b>Read Lock:</b></td><td><code>%11</code></td></tr>"
          "</table></div>")
          .arg(QString("%1").arg(offset, 8, 16, QChar('0')).toUpper())
          .arg(QString("%1").arg(endAddr, 8, 16, QChar('0')).toUpper())
          .arg(sizeHuman)
          .arg(QString("%1").arg(sizeBytes, 0, 16).toUpper())
          .arg(wordWidth)
          .arg(depth)
          .arg(m_currentMemItem->data("SW Access").toString())
          .arg(m_currentMemItem->data("HW Access").toString().isEmpty()
                   ? m_currentMemItem->data("HW Access Policy").toString()
                   : m_currentMemItem->data("HW Access").toString())
          .arg(m_currentMemItem->data("HDL Path").toString().isEmpty()
                   ? QStringLiteral("—")
                   : m_currentMemItem->data("HDL Path").toString())
          .arg(m_currentMemItem->data("Write Lock").toString().isEmpty()
                   ? QStringLiteral("None")
                   : m_currentMemItem->data("Write Lock").toString())
          .arg(m_currentMemItem->data("Read Lock").toString().isEmpty()
                   ? QStringLiteral("None")
                   : m_currentMemItem->data("Read Lock").toString());

  m_memSummaryLabel->setText(summary);
}

void RegMapWindow::navigateToRegister(int childRow, RegMapTreeItem *regItem) {
  Q_UNUSED(regItem);
  if (!m_currentBlkItem)
    return;

  QModelIndex blkProxy = this->treeView->currentIndex();
  if (!blkProxy.isValid())
    return; // GCOV_EXCL_LINE - Defensive index guard
  QModelIndex blkSource = m_treeProxy->mapToSource(blkProxy);
  QModelIndex blkCol0 = m_model->index(blkSource.row(), 0, blkSource.parent());

  QModelIndex regSource = m_model->index(childRow, 0, blkCol0);
  if (!regSource.isValid())
    return;

  QModelIndex regProxy = m_treeProxy->mapFromSource(regSource);
  if (regProxy.isValid()) {
    this->treeView->setCurrentIndex(regProxy);
    this->treeView->selectionModel()->select(
        regProxy,
        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    this->treeView->scrollTo(regProxy, QAbstractItemView::PositionAtCenter);
  }
}

void RegMapWindow::showTreeContextMenu(const QPoint &pos) {
  QPoint globalPos;
  QModelIndex index;
  QWidget *senderWidget = qobject_cast<QWidget *>(sender());

  if (senderWidget == this->treeView) {
    index = this->treeView->indexAt(pos);
    globalPos = this->treeView->viewport()->mapToGlobal(pos);
  } else if (senderWidget == m_fieldsTableView) {
    index = m_fieldsTableView->indexAt(pos);
    globalPos = m_fieldsTableView->viewport()->mapToGlobal(pos);
  }

  QMenu menu(this);
  menu.setObjectName("treeContextMenu");
  if (index.isValid()) {
    QAction *duplicateAction = menu.addAction(tr("Duplicate"));
    QAction *deleteAction = menu.addAction(tr("Delete"));
    connect(duplicateAction, &QAction::triggered, this,
            [this, index, senderWidget]() {
              if (senderWidget == this->treeView) {
                duplicateItem(index);
              } else {
                duplicateItem(index);
              }
            });
    connect(deleteAction, &QAction::triggered, this,
            &RegMapWindow::btnDeleteItem);
  }
  menu.exec(globalPos);
}

void RegMapWindow::duplicateSelectedRegister(void) {
  QModelIndex proxyIndex;
  if (m_fieldsTableView && m_fieldsTableView->hasFocus()) {
    QModelIndex fieldProxyIdx = m_fieldsTableView->currentIndex();
    if (fieldProxyIdx.isValid()) {
      proxyIndex = fieldProxyIdx;
    }
  }
  if (!proxyIndex.isValid()) {
    proxyIndex = this->treeView->currentIndex();
  }
  if (!proxyIndex.isValid() && m_currentRegItem) {
    RegMapTreeItem *blk = m_currentRegItem->parentItem();
    if (blk) {
      QModelIndex blkIdx = (blk->parentItem() == m_model->getRootItem())
                               ? m_model->index(blk->row(), 0, QModelIndex())
                               : QModelIndex();
      QModelIndex regSrcIdx =
          m_model->index(m_currentRegItem->row(), 0, blkIdx);
      proxyIndex = m_treeProxy->mapFromSource(regSrcIdx);
    }
  }

  if (proxyIndex.isValid()) {
    duplicateItem(proxyIndex);
  }
}

QModelIndex RegMapWindow::currentRegSourceIndex(int col) const {
  if (!m_currentRegItem || !m_model)
    return QModelIndex();
  QModelIndex currentRegProxy = this->treeView->currentIndex();
  if (currentRegProxy.isValid()) {
    QModelIndex currentRegSource = m_treeProxy->mapToSource(currentRegProxy);
    RegMapTreeItem *item = m_model->getItem(currentRegSource);
    if (item == m_currentRegItem) {
      return m_model->index(currentRegSource.row(), col,
                            currentRegSource.parent());
    }
  }
  // GCOV_EXCL_START - Defensive index fallback
  RegMapTreeItem *blk = m_currentRegItem->parentItem();
  if (blk) {
    QModelIndex blkIdx = (blk->parentItem() == m_model->getRootItem())
                             ? m_model->index(blk->row(), 0, QModelIndex())
                             : QModelIndex();
    return m_model->index(m_currentRegItem->row(), col, blkIdx);
  }
  return QModelIndex();
  // GCOV_EXCL_STOP
}

QModelIndex RegMapWindow::currentBlkSourceIndex(int col) const {
  if (!m_currentBlkItem || !m_model)
    return QModelIndex();
  QModelIndex currentBlkProxy = this->treeView->currentIndex();
  if (currentBlkProxy.isValid()) {
    QModelIndex currentBlkSource = m_treeProxy->mapToSource(currentBlkProxy);
    RegMapTreeItem *item = m_model->getItem(currentBlkSource);
    if (item == m_currentBlkItem) {
      return m_model->index(currentBlkSource.row(), col,
                            currentBlkSource.parent());
    }
  }
  // GCOV_EXCL_START - Defensive index fallback
  return m_model->index(m_currentBlkItem->row(), col, QModelIndex());
  // GCOV_EXCL_STOP
}

QModelIndex RegMapWindow::currentMemSourceIndex(int col) const {
  if (!m_currentMemItem || !m_model)
    return QModelIndex();
  QModelIndex currentMemProxy = this->treeView->currentIndex();
  if (currentMemProxy.isValid()) {
    QModelIndex currentMemSource = m_treeProxy->mapToSource(currentMemProxy);
    RegMapTreeItem *item = m_model->getItem(currentMemSource);
    if (item == m_currentMemItem) {
      return m_model->index(currentMemSource.row(), col,
                            currentMemSource.parent());
    }
  }
  // GCOV_EXCL_START - Defensive index fallback
  RegMapTreeItem *parent = m_currentMemItem->parentItem();
  if (parent && parent != m_model->getRootItem()) {
    QModelIndex parentIdx = m_model->index(parent->row(), 0, QModelIndex());
    return m_model->index(m_currentMemItem->row(), col, parentIdx);
  }
  return m_model->index(m_currentMemItem->row(), col, QModelIndex());
  // GCOV_EXCL_STOP
}

void RegMapWindow::duplicateItem(const QModelIndex &index) {
  QModelIndex source_index;
  if (index.model() == m_fieldProxy) {
    source_index = m_fieldProxy->mapToSource(index);
  } else if (index.model() == m_model) {
    source_index = index;
  } else {
    source_index = m_treeProxy->mapToSource(index);
  }
  RegMapTreeItem *item = m_model->getItem(source_index);
  if (!item || item->kindString() == "root")
    return;

  DeleteItemCommand::StoredNode storedData;
  DeleteItemCommand::captureItem(item, storedData);

  protormap::Config *cfg = m_config_window->serialize();
  uint32_t regWidth = (cfg && cfg->reg_width() > 0) ? cfg->reg_width() : 32;
  delete cfg;
  uint64_t regBytes = (regWidth > 0 ? regWidth : 32) / 8;
  if (regBytes == 0)
    regBytes = 4; // GCOV_EXCL_LINE - Defensive fallback

  QString oldName = storedData.colData.value("Name").toString();
  storedData.colData["Name"] = oldName + "_COPY";

  if (item->kindString() == "reg") {
    uint64_t offset = parseNumericValue(storedData.colData.value("Offset/LSB"));
    uint64_t newOff = offset + regBytes;
    storedData.colData["Offset/LSB"] = padHexOffsetString(
        QString("0x") + QString("%1").arg(newOff, 4, 16, QChar('0')).toUpper());
  } else if (item->kindString() == "mem") {
    uint64_t offset = parseNumericValue(storedData.colData.value("Offset/LSB"));
    uint64_t memSize =
        parseNumericValue(storedData.colData.value("Size/Width"));
    if (memSize == 0)
      memSize = 1024;
    uint64_t newOff = offset + memSize;
    storedData.colData["Offset/LSB"] = padHexOffsetString(
        QString("0x") + QString("%1").arg(newOff, 4, 16, QChar('0')).toUpper());
  } else if (item->kindString() == "fld") {
    uint64_t lsb = parseNumericValue(storedData.colData.value("Offset/LSB"));
    uint64_t width = parseNumericValue(storedData.colData.value("Size/Width"));
    storedData.colData["Offset/LSB"] = QString::number(lsb + width);
  }

  int row = source_index.row() + 1;
  QModelIndex parent = source_index.parent();

  m_undoStack->push(new DuplicateItemCommand(m_model, row, parent, storedData));

  QModelIndex new_source = m_model->index(row, 0, parent);
  if (index.model() == m_fieldProxy && m_fieldsTableView && m_fieldProxy) {
    QModelIndex new_fld_proxy = m_fieldProxy->mapFromSource(new_source);
    if (new_fld_proxy.isValid()) {
      m_fieldsTableView->setCurrentIndex(new_fld_proxy);
      m_fieldsTableView->scrollTo(new_fld_proxy);
      m_fieldsTableView->selectionModel()->select(
          new_fld_proxy,
          QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }
  } else {
    QModelIndex new_proxy = m_treeProxy->mapFromSource(new_source);
    if (new_proxy.isValid()) {
      this->treeView->setCurrentIndex(new_proxy);
      this->treeView->scrollTo(new_proxy);
      this->treeView->selectionModel()->select(
          new_proxy,
          QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }
  }
  this->treeView->viewport()->update();
}

bool RegMapWindow::headlessExport(const QString &out_dir) {
  if (m_rmap_filename.isEmpty()) {
    std::cerr << "No input file specified for headless export" << std::endl;
    return false;
  }

  QString baseDir = m_config_window->baseDir();

  protormap::Config *cfg = m_config_window->serialize();
  uint32_t regWidth = cfg->reg_width() > 0 ? cfg->reg_width() : 32;

  QStringList validationErrors = m_model->checkData(regWidth);
  if (!validationErrors.isEmpty()) {
    std::cerr << "Validation warnings found:" << std::endl;
    for (const QString &err : validationErrors) {
      std::cerr << " - " << err.toStdString() << std::endl;
    }
  }

  CodeGenerator cg;
  std::string template_folder = cfg->templatefolder();
  std::string default_output = resolveExportOutputFolder(out_dir, cfg);

  bool hwPrec =
      cfg->has_hw_precedence()
          ? cfg->hw_precedence()
          // GCOV_EXCL_START - Defensive fallback
          : (m_config_window ? m_config_window->hwPrecedence() : true);
  // GCOV_EXCL_STOP
  json jsonData = m_model->extractJsonData(regWidth, hwPrec);
  resolveExportProjectName(cfg, m_rmap_filename, jsonData);
  jsonData["project_name"] = cfg->project_name();
  jsonData["project_version"] = cfg->project_version();
  jsonData["project_vendor"] = cfg->project_vendor();
  jsonData["project_library"] = cfg->project_library();
  jsonData["project_description"] = cfg->project_description();
  for (const auto &[key, value] : cfg->custom_parameters()) {
    jsonData[key] = value;
  }

  std::string defaultOut = cfg->outputfolder().empty()
                               ? PathUtils::DEFAULT_OUTPUT_DIR
                               : cfg->outputfolder();
  QString cfgOut =
      PathUtils::normalizeSeparators(QString::fromStdString(defaultOut));

  std::vector<TemplateMapping> mappings;
  for (const auto &entry : cfg->template_outputs()) {
    if (entry.template_filename().empty())
      continue; // GCOV_EXCL_LINE - Defensive empty entry guard
    if (entry.has_enabled() && !entry.enabled())
      continue;

    if (!out_dir.isEmpty()) {
      QString entryOut = QString::fromStdString(entry.output_filepath());
      QString normalizedEntryOut = PathUtils::normalizeSeparators(entryOut);

      QString relPath;
      if (normalizedEntryOut.startsWith(cfgOut + "/", Qt::CaseInsensitive)) {
        relPath = normalizedEntryOut.mid(cfgOut.length() + 1);
      } else if (normalizedEntryOut.startsWith("./" + cfgOut + "/",
                                               Qt::CaseInsensitive)) {
        relPath = normalizedEntryOut.mid(cfgOut.length() + 3);
      } else if (normalizedEntryOut.startsWith("work/", Qt::CaseInsensitive)) {
        relPath = normalizedEntryOut.mid(5);
      } else if (normalizedEntryOut.startsWith("./work/",
                                               Qt::CaseInsensitive)) {
        relPath = normalizedEntryOut.mid(7);
      } else {
        relPath = normalizedEntryOut;
      }

      QString expOutDir = PathUtils::expandEnvVars(out_dir);
      QString absOutDir = QDir(QDir::currentPath()).absoluteFilePath(expOutDir);
      QString customOut =
          PathUtils::normalizeSeparators(QDir(absOutDir).filePath(relPath));
      mappings.push_back({entry.template_filename(), customOut.toStdString()});
    } else {
      mappings.push_back({entry.template_filename(), entry.output_filepath()});
    }
  }

  std::string pythonScript = "";
  bool pyEnabled = isExportPythonEnabled(cfg);
  if (pyEnabled && !cfg->pythonscript().empty()) {
    pythonScript = cfg->pythonscript();
  }

  GenerationReport report =
      cg.generate(jsonData, template_folder, default_output, mappings,
                  baseDir.toStdString(), pythonScript);

  if (!report.errors.empty()) {
    std::cerr << "Code generation completed with errors:" << std::endl;
    for (const auto &err : report.errors) {
      std::cerr << err.first << ": " << err.second << std::endl;
    }
    delete cfg;
    return false;
  }
  std::cout << "Successfully exported " << report.success_files.size()
            << " files." << std::endl;

  delete cfg;
  return true;
}

std::string
RegMapWindow::resolveExportOutputFolder(const QString &outDir,
                                        const protormap::Config *cfg) {
  if (!outDir.isEmpty()) {
    return PathUtils::expandEnvVars(outDir).toStdString();
  }
  if (cfg && !cfg->outputfolder().empty()) {
    return cfg->outputfolder();
  }
  return PathUtils::DEFAULT_OUTPUT_DIR;
}

void RegMapWindow::resolveExportProjectName(const protormap::Config *cfg,
                                            const QString &filename,
                                            nlohmann::json &jsonData) {
  if (cfg && !cfg->project_name().empty()) {
    jsonData["name"] = cfg->project_name();
    return;
  }
  if (!filename.isEmpty()) {
    std::string currentName;
    if (jsonData.is_object()) {
      currentName = jsonData.value("name", "");
    }
    if (currentName.empty() || currentName == "regmap") {
      QFileInfo fi(filename);
      QString base = fi.baseName();
      if (!base.isEmpty()) {
        jsonData["name"] = base.toStdString();
      }
    }
  }
}

bool RegMapWindow::isExportPythonEnabled(const protormap::Config *cfg) const {
  if (!cfg) {
    return false;
  }
  if (cfg->has_python_script_enabled()) {
    return cfg->python_script_enabled();
  }
  if (m_config_window) { // GCOV_EXCL_BR_LINE - Defensive invariant
    return m_config_window->isPythonScriptEnabled();
  }
  return !cfg->pythonscript().empty();
}

bool RegMapWindow::headlessLint(bool strict, const QString &format,
                                const QString &outFile) {
  protormap::Config *cfg = m_config_window->serialize();
  uint32_t regWidth = cfg->reg_width() > 0 ? cfg->reg_width() : 32;
  delete cfg;

  QStringList errors = m_model->checkData(regWidth);
  QStringList warnings;

  // Strict checks: check for empty descriptions or unaligned offsets
  if (strict && m_model->getRootItem()) { // GCOV_EXCL_BR_LINE - getRootItem
                                          // guaranteed non-null
    performStrictLintChecks(m_model->getRootItem(), regWidth, warnings);
  }

  bool isPassed = errors.isEmpty() && (!strict || warnings.isEmpty());
  QString reportContent;
  QString fmt = format.toLower().trimmed();

  if (fmt == "json") {
    QJsonObject rootObj;
    rootObj["status"] = isPassed ? "PASS" : "FAIL";
    rootObj["file"] = m_rmap_filename;
    QJsonArray errArr, warnArr;
    for (const QString &e : errors)
      errArr.append(e);
    for (const QString &w : warnings)
      warnArr.append(w);
    rootObj["errors"] = errArr;
    rootObj["warnings"] = warnArr;
    reportContent = QJsonDocument(rootObj).toJson(QJsonDocument::Indented);
  } else if (fmt == "sarif") {
    QJsonObject sarif;
    sarif["$schema"] = "https://schemastore.azurewebsites.net/schemas/json/"
                       "sarif-2.1.0-rtm.5.json";
    sarif["version"] = "2.1.0";
    QJsonArray runs;
    QJsonObject run;
    QJsonObject tool;
    QJsonObject driver;
    driver["name"] = "rmap-lint";
    driver["version"] = RMAP_VERSION_STRING;
    tool["driver"] = driver;
    run["tool"] = tool;

    QJsonArray results;
    for (const QString &e : errors) {
      QJsonObject res;
      res["level"] = "error";
      QJsonObject msg;
      msg["text"] = e;
      res["message"] = msg;
      results.append(res);
    }
    for (const QString &w : warnings) {
      QJsonObject res;
      res["level"] = "warning";
      QJsonObject msg;
      msg["text"] = w;
      res["message"] = msg;
      results.append(res);
    }
    run["results"] = results;
    runs.append(run);
    sarif["runs"] = runs;
    reportContent = QJsonDocument(sarif).toJson(QJsonDocument::Indented);
  } else if (fmt == "junit") {
    int totalTests = 1 + warnings.size();
    int failures = errors.size() + (strict ? warnings.size() : 0);
    reportContent =
        QString(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<testsuites "
            "name=\"rmap-lint\" tests=\"%1\" failures=\"%2\">\n  <testsuite "
            "name=\"RegisterMapValidation\" tests=\"%1\" failures=\"%2\">\n")
            .arg(totalTests)
            .arg(failures);

    if (errors.isEmpty()) {
      reportContent += "    <testcase name=\"AddressAndOverlapCheck\"/>\n";
    } else {
      reportContent +=
          QString("    <testcase name=\"AddressAndOverlapCheck\"><failure "
                  "message=\"Validation Errors\">%1</failure></testcase>\n")
              .arg(errors.join("\n"));
    }
    for (int i = 0; i < warnings.size(); ++i) {
      reportContent +=
          QString("    <testcase name=\"StrictLint_%1\"><failure "
                  "message=\"Strict Rule Violation\">%2</failure></testcase>\n")
              .arg(i + 1)
              .arg(warnings[i]);
    }
    reportContent += "  </testsuite>\n</testsuites>\n";
  } else {
    // Text format
    reportContent =
        QString("=== rmap Linter Report: %1 ===\n").arg(m_rmap_filename);
    if (errors.isEmpty()) {
      reportContent += "✓ Validation Check: PASSED (0 errors)\n";
    } else {
      reportContent += QString("✗ Validation Check: FAILED (%1 errors)\n")
                           .arg(errors.size());
      for (const QString &e : errors)
        reportContent += QString("  • ERROR: %1\n").arg(e);
    }
    if (!warnings.isEmpty()) {
      reportContent += QString("⚠ Strict Lint Warnings: (%1 warnings)\n")
                           .arg(warnings.size());
      for (const QString &w : warnings)
        reportContent += QString("  • WARN: %1\n").arg(w);
    }
    reportContent += isPassed ? "\nResult: SUCCESS\n" : "\nResult: FAILED\n";
  }

  QString expOut = PathUtils::expandEnvVars(outFile);
  if (!expOut.isEmpty()) {
    QFileInfo fi(expOut);
    QDir().mkpath(fi.absolutePath());
    QFile f(expOut);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QTextStream(&f) << reportContent;
      f.close();
      std::cout << "Lint report saved to: " << expOut.toStdString()
                << std::endl;
    } else {
      std::cerr << "Failed to write lint report to: " << expOut.toStdString()
                << std::endl;
      return false;
    }
  } else {
    std::cout << reportContent.toStdString() << std::endl;
  }

  return isPassed;
}

bool RegMapWindow::semanticDiff(const QString &file1, const QString &file2,
                                const QString &format, const QString &outFile) {
  QString expFile1 = PathUtils::expandEnvVars(file1);
  QString expFile2 = PathUtils::expandEnvVars(file2);
  QString expOut = PathUtils::expandEnvVars(outFile);

  RegMapTreeModel model1, model2;
  RegConfigWindow cfg1, cfg2;

  FormatResult r1 =
      FormatManager::instance().loadFile(expFile1, &model1, &cfg1);
  if (!r1.success) {
    std::cerr << "Diff error: Failed to load file1: "
              << r1.errorMessage.toStdString() << std::endl;
    return false;
  }
  FormatResult r2 =
      FormatManager::instance().loadFile(expFile2, &model2, &cfg2);
  if (!r2.success) {
    std::cerr << "Diff error: Failed to load file2: "
              << r2.errorMessage.toStdString() << std::endl;
    return false;
  }

  std::map<QString, RegSummary> map1, map2;
  gatherRegs(model1, map1);
  gatherRegs(model2, map2);

  QStringList addedRegs, removedRegs, modifiedRegs;

  for (const auto &[key, s2] : map2) {
    auto it1 = map1.find(key);
    if (it1 == map1.end()) {
      addedRegs.append(key);
    } else {
      const auto &s1 = it1->second;
      QStringList diffs;
      if (s1.offset != s2.offset)
        diffs.append(QString("Offset: %1 -> %2").arg(s1.offset).arg(s2.offset));
      if (s1.access != s2.access)
        diffs.append(QString("Access: %1 -> %2").arg(s1.access).arg(s2.access));
      if (s1.reset != s2.reset)
        diffs.append(QString("Reset: %1 -> %2").arg(s1.reset).arg(s2.reset));

      // Compare fields
      for (const auto &[fName, fVal] : s2.fields) {
        auto fit1 = s1.fields.find(fName);
        if (fit1 == s1.fields.end()) {
          diffs.append(QString("Added field '%1' (%2)").arg(fName).arg(fVal));
        } else if (fit1->second != fVal) {
          diffs.append(QString("Modified field '%1': %2 -> %3")
                           .arg(fName)
                           .arg(fit1->second)
                           .arg(fVal));
        }
      }
      for (const auto &[fName, fVal] : s1.fields) {
        Q_UNUSED(fVal);
        if (s2.fields.find(fName) == s2.fields.end()) {
          diffs.append(QString("Removed field '%1'").arg(fName));
        }
      }

      if (!diffs.isEmpty()) {
        modifiedRegs.append(key + " (" + diffs.join("; ") + ")");
      }
    }
  }

  for (const auto &[key, val] : map1) {
    Q_UNUSED(val);
    if (map2.find(key) == map2.end()) {
      removedRegs.append(key);
    }
  }

  QString outputStr;
  if (format.toLower() == "markdown") {
    outputStr =
        QString("# Register Map Diff: `%1` vs `%2`\n\n").arg(file1, file2);
    outputStr += QString("### Summary\n- **Added Registers:** %1\n- **Removed "
                         "Registers:** %2\n- **Modified Registers:** %3\n\n")
                     .arg(addedRegs.size())
                     .arg(removedRegs.size())
                     .arg(modifiedRegs.size());

    if (!addedRegs.isEmpty()) {
      outputStr += "#### ➕ Added Registers\n";
      for (const QString &r : addedRegs)
        outputStr += QString("- `%1`\n").arg(r);
      outputStr += "\n";
    }
    if (!removedRegs.isEmpty()) {
      outputStr += "#### ➖ Removed Registers\n";
      for (const QString &r : removedRegs)
        outputStr += QString("- `%1`\n").arg(r);
      outputStr += "\n";
    }
    if (!modifiedRegs.isEmpty()) {
      outputStr += "#### 📝 Modified Registers\n";
      for (const QString &r : modifiedRegs)
        outputStr += QString("- `%1`\n").arg(r);
      outputStr += "\n";
    }
  } else {
    outputStr =
        QString("=== Register Map Diff: %1 vs %2 ===\n").arg(file1, file2);
    outputStr += QString("Added: %1 | Removed: %2 | Modified: %3\n\n")
                     .arg(addedRegs.size())
                     .arg(removedRegs.size())
                     .arg(modifiedRegs.size());
    for (const QString &r : addedRegs)
      outputStr += QString("+ ADDED:    %1\n").arg(r);
    for (const QString &r : removedRegs)
      outputStr += QString("- REMOVED:  %1\n").arg(r);
    for (const QString &r : modifiedRegs)
      outputStr += QString("~ MODIFIED: %1\n").arg(r);
  }

  if (!expOut.isEmpty()) {
    QFileInfo fi(expOut);
    QDir().mkpath(fi.absolutePath());
    QFile f(expOut);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QTextStream(&f) << outputStr;
      f.close();
      std::cout << "Diff saved to: " << expOut.toStdString() << std::endl;
    } else {
      std::cerr << "Failed to write diff report to: " << expOut.toStdString()
                << std::endl;
      return false;
    }
  }

  std::cout << outputStr.toStdString() << std::endl;

  return true;
}

#include "RegMapWindow.moc"
