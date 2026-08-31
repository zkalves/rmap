#include <unistd.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHeaderView>
#include <QStackedWidget>
#include <QTableWidget>
#include <QScrollArea>
#include <QTextBrowser>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QMenu>
#include <QFileDialog>
#include "RegMapWindow.hpp"
#include "BlockMemoryMapWidget.hpp"
#include "ThemeManager.hpp"
#include "AppSettings.hpp"
#include "PathUtils.hpp"
#include "format/FormatManager.hpp"

// Helper to pad hex offset string written in hex (0x format)
static QString padHexOffsetString(const QString &input, int minDigits = 4)
{
    QString s = input.trimmed();
    if (s.startsWith("0x", Qt::CaseInsensitive)) {
        bool ok = false;
        uint64_t val = s.mid(2).toULongLong(&ok, 16);
        if (ok) {
            int digits = (std::max)((int)minDigits, (int)(s.size() - 2));
            if (val >= 0x100000000ULL) digits = (std::max)(digits, 16);
            else if (val >= 0x10000ULL) digits = (std::max)(digits, 8);
            else digits = (std::max)(digits, minDigits);
            return QString("0x") + QString("%1").arg(val, digits, 16, QChar('0')).toUpper();
        }
    }
    return s;
}

class TreeFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    TreeFilterProxyModel(QObject* parent = nullptr) : QSortFilterProxyModel(parent) {
        setDynamicSortFilter(true);
        setSortCaseSensitivity(Qt::CaseInsensitive);
    }

    void setSearchFilter(const QString &filter) {
        m_filterText = filter.trimmed().toLower();
        invalidateFilter();
    }

    QString searchFilter() const { return m_filterText; }

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override {
        if (!sourceModel() || !source_left.isValid() || !source_right.isValid()) return false;

        // Offset / LSB column (1) -> sort numerically
        if (source_left.column() == 1) {
            QString l_str = sourceModel()->data(source_left, Qt::DisplayRole).toString().trimmed();
            QString r_str = sourceModel()->data(source_right, Qt::DisplayRole).toString().trimmed();

            uint64_t l_val = 0, r_val = 0;
            if (l_str.startsWith("0x", Qt::CaseInsensitive)) l_val = l_str.mid(2).toULongLong(nullptr, 16);
            else if (l_str.startsWith("0b", Qt::CaseInsensitive)) l_val = l_str.mid(2).toULongLong(nullptr, 2);
            else l_val = l_str.toULongLong(nullptr, 10);

            if (r_str.startsWith("0x", Qt::CaseInsensitive)) r_val = r_str.mid(2).toULongLong(nullptr, 16);
            else if (r_str.startsWith("0b", Qt::CaseInsensitive)) r_val = r_str.mid(2).toULongLong(nullptr, 2);
            else r_val = r_str.toULongLong(nullptr, 10);

            return l_val < r_val;
        } else if (source_left.column() == 2 || source_left.column() == 6) { // Size, Reset
            QString l_str = sourceModel()->data(source_left, Qt::DisplayRole).toString().trimmed();
            QString r_str = sourceModel()->data(source_right, Qt::DisplayRole).toString().trimmed();

            uint64_t l_val = 0, r_val = 0;
            if (l_str.startsWith("0x", Qt::CaseInsensitive)) l_val = l_str.mid(2).toULongLong(nullptr, 16);
            else if (l_str.startsWith("0b", Qt::CaseInsensitive)) l_val = l_str.mid(2).toULongLong(nullptr, 2);
            else l_val = l_str.toULongLong(nullptr, 10);

            if (r_str.startsWith("0x", Qt::CaseInsensitive)) r_val = r_str.mid(2).toULongLong(nullptr, 16);
            else if (r_str.startsWith("0b", Qt::CaseInsensitive)) r_val = r_str.mid(2).toULongLong(nullptr, 2);
            else r_val = r_str.toULongLong(nullptr, 10);

            return l_val < r_val;
        }
        return QSortFilterProxyModel::lessThan(source_left, source_right);
    }

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override {
        if (!sourceModel()) return false;
        QModelIndex index0 = sourceModel()->index(source_row, 0, source_parent);
        if (!index0.isValid()) return false;

        QString kind = sourceModel()->data(index0, Qt::DisplayRole).toString();
        if (kind == "fld") return false; // Never show fields in left tree

        if (m_filterText.isEmpty()) return true;

        RegMapTreeItem *item = static_cast<RegMapTreeItem*>(index0.internalPointer());
        if (!item) return false;

        if (itemMatches(item)) return true;

        // Check if any child matches
        for (int r = 0; r < item->childCount(); ++r) {
            RegMapTreeItem *child = item->child(r);
            if (child && (itemMatches(child) || childHasMatch(child))) {
                return true;
            }
        }
        return false;
    }

    bool itemMatches(RegMapTreeItem *item) const {
        if (!item) return false;
        QString name = item->data("Name").toString().toLower();
        QString offset = item->data("Offset/LSB").toString().toLower();
        QString desc = item->data("Description").toString().toLower();
        QString access = item->data("Access Policy").toString().toLower();
        return name.contains(m_filterText) || offset.contains(m_filterText) ||
               desc.contains(m_filterText) || access.contains(m_filterText);
    }

    bool childHasMatch(RegMapTreeItem *parent) const {
        for (int r = 0; r < parent->childCount(); ++r) {
            RegMapTreeItem *child = parent->child(r);
            if (child && (itemMatches(child) || childHasMatch(child))) return true;
        }
        return false;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            if (section == 1) return tr("Offset");
            if (section == 2) return tr("Size");
        }
        return QSortFilterProxyModel::headerData(section, orientation, role);
    }

private:
    QString m_filterText;
};

class FieldSortProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    FieldSortProxyModel(QObject* parent = nullptr) : QSortFilterProxyModel(parent) {}
protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override {
        if (!sourceModel() || !source_left.isValid() || !source_right.isValid()) return false;
        if (source_left.column() == 1) { // Sort by Offset
            QString l_str = sourceModel()->data(source_left, Qt::DisplayRole).toString().trimmed();
            QString r_str = sourceModel()->data(source_right, Qt::DisplayRole).toString().trimmed();

            uint64_t l_val = 0, r_val = 0;
            if (l_str.startsWith("0x", Qt::CaseInsensitive)) l_val = l_str.mid(2).toULongLong(nullptr, 16);
            else if (l_str.startsWith("0b", Qt::CaseInsensitive)) l_val = l_str.mid(2).toULongLong(nullptr, 2);
            else l_val = l_str.toULongLong(nullptr, 10);

            if (r_str.startsWith("0x", Qt::CaseInsensitive)) r_val = r_str.mid(2).toULongLong(nullptr, 16);
            else if (r_str.startsWith("0b", Qt::CaseInsensitive)) r_val = r_str.mid(2).toULongLong(nullptr, 2);
            else r_val = r_str.toULongLong(nullptr, 10);

            return l_val < r_val;
        }
        return QSortFilterProxyModel::lessThan(source_left, source_right);
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            if (section == 1) return tr("LSB");
            if (section == 2) return tr("Size");
        }
        return QSortFilterProxyModel::headerData(section, orientation, role);
    }
};

RegMapWindow::RegMapWindow(const QString &rmap_filename, QWidget *parent) :
    QMainWindow(parent)
{
    Q_INIT_RESOURCE(resources);
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    QIcon appIcon(":/icons/app_icon.png");
    appIcon.addFile(":/icons/app_icon_512.png", QSize(512, 512));
    appIcon.addFile(":/icons/app_icon_256.png", QSize(256, 256));
    appIcon.addFile(":/icons/app_icon_128.png", QSize(128, 128));
    appIcon.addFile(":/icons/app_icon_64.png", QSize(64, 64));
    appIcon.addFile(":/icons/app_icon_48.png", QSize(48, 48));
    appIcon.addFile(":/icons/app_icon_32.png", QSize(32, 32));
    appIcon.addFile(":/icons/app_icon_16.png", QSize(16, 16));
    this->setupUi(this);
    setWindowIcon(appIcon);
    m_active_folder = ".";
    m_is_regmap_modified = false;
    m_rmap_filename = rmap_filename;
    m_default_filename = "rmap.rmt";
    m_default_window_title = windowTitle();

    m_config_window = new RegConfigWindow(this);
    m_pref_window = new PreferencesWindow(this);

    m_undoStack = new QUndoStack(this);

    // Setup Undo / Redo connections
    connect(actionUndo, &QAction::triggered, m_undoStack, &QUndoStack::undo);
    connect(actionRedo, &QAction::triggered, m_undoStack, &QUndoStack::redo);
    connect(m_undoStack, &QUndoStack::canUndoChanged, actionUndo, &QAction::setEnabled);
    connect(m_undoStack, &QUndoStack::canRedoChanged, actionRedo, &QAction::setEnabled);
    actionUndo->setEnabled(false);
    actionRedo->setEnabled(false);

    connect(m_undoStack, &QUndoStack::undoTextChanged, this, [this](const QString &t) {
        actionUndo->setText(t.isEmpty() ? tr("Undo") : tr("Undo %1").arg(t));
    });
    connect(m_undoStack, &QUndoStack::redoTextChanged, this, [this](const QString &t) {
        actionRedo->setText(t.isEmpty() ? tr("Redo") : tr("Redo %1").arg(t));
    });

    // Model and Proxies
    m_model = new RegMapTreeModel(this);
    connectModelSignals();

    m_treeProxy = new TreeFilterProxyModel(this);
    m_fieldProxy = new FieldSortProxyModel(this);
    m_treeProxy->setSourceModel(m_model);
    m_fieldProxy->setSourceModel(m_model);

    // Left Panel: Search Bar + Tree
    QWidget *leftPanel = new QWidget(this);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(4);

    QHBoxLayout *searchLayout = new QHBoxLayout();
    m_searchEdit = new QLineEdit(leftPanel);
    m_searchEdit->setPlaceholderText(tr("Search registers/blocks (e.g. CTRL, 0x0)..."));
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &RegMapWindow::onSearchTextChanged);

    searchLayout->addWidget(m_searchEdit);
    leftLayout->addLayout(searchLayout);

    this->treeView->setParent(leftPanel);
    this->treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->treeView->setModel(m_treeProxy);
    this->treeView->setSortingEnabled(true);
    this->treeView->sortByColumn(1, Qt::AscendingOrder);
    this->treeView->setAlternatingRowColors(true);
    this->treeView->setColumnHidden(2, true); // Size (Register size is global in Config)
    this->treeView->setColumnHidden(4, true); // SW Access Policy
    this->treeView->setColumnHidden(5, true); // HW Access Policy
    this->treeView->setColumnHidden(6, true); // Reset Value
    this->treeView->setColumnHidden(7, true); // Is Rand
    this->treeView->setColumnHidden(8, true); // Volatile
    this->treeView->setColumnHidden(9, true); // Has Reset
    // Note: Column 10 (Description) remains visible in treeView for all items
    this->treeView->header()->setStretchLastSection(true);
    this->treeView->header()->setSectionResizeMode(QHeaderView::Interactive);
    this->treeView->header()->setSectionResizeMode(10, QHeaderView::Stretch);
    leftLayout->addWidget(this->treeView);

    // Right Panel: Stacked Widget (Register Bitfield View vs Block Memory Map View)
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
    QHBoxLayout *regHeaderLayout = new QHBoxLayout(m_regHeaderWidget);
    regHeaderLayout->setContentsMargins(4, 2, 4, 2);
    regHeaderLayout->setSpacing(8);

    QLabel *regNameTitle = new QLabel(tr("Name:"), m_regHeaderWidget);
    regNameTitle->setStyleSheet("font-weight: bold; font-size: 12px;");

    m_regNameEdit = new QLineEdit(m_regHeaderWidget);
    m_regNameEdit->setObjectName("regNameEdit");
    m_regNameEdit->setPlaceholderText(tr("Register name..."));
    m_regNameEdit->setClearButtonEnabled(true);
    m_regNameEdit->setMinimumWidth(120);

    QLabel *regOffsetTitle = new QLabel(tr("Offset:"), m_regHeaderWidget);
    regOffsetTitle->setStyleSheet("font-weight: bold; font-size: 12px;");

    m_regOffsetEdit = new QLineEdit(m_regHeaderWidget);
    m_regOffsetEdit->setObjectName("regOffsetEdit");
    m_regOffsetEdit->setPlaceholderText(tr("0x00"));
    m_regOffsetEdit->setStyleSheet("font-family: monospace; font-size: 12px;");
    m_regOffsetEdit->setClearButtonEnabled(true);
    m_regOffsetEdit->setMaximumWidth(100);

    QLabel *descLabel = new QLabel(tr("Description:"), m_regHeaderWidget);
    descLabel->setStyleSheet("font-weight: bold; font-size: 12px;");

    m_regDescEdit = new QLineEdit(m_regHeaderWidget);
    m_regDescEdit->setObjectName("regDescEdit");
    m_regDescEdit->setPlaceholderText(tr("Register description..."));
    m_regDescEdit->setClearButtonEnabled(true);

    regHeaderLayout->addWidget(regNameTitle);
    regHeaderLayout->addWidget(m_regNameEdit);
    regHeaderLayout->addWidget(regOffsetTitle);
    regHeaderLayout->addWidget(m_regOffsetEdit);
    regHeaderLayout->addWidget(descLabel);
    regHeaderLayout->addWidget(m_regDescEdit, 1);

    regViewLayout->addWidget(m_regHeaderWidget);

    connect(m_regNameEdit, &QLineEdit::editingFinished, this, [this]() {
        if (!m_currentRegItem || !m_model) return;
        QString newName = m_regNameEdit->text().trimmed();
        QString oldName = m_currentRegItem->data("Name").toString();
        if (newName != oldName && !newName.isEmpty()) {
            QModelIndex currentRegProxy = this->treeView->currentIndex();
            if (currentRegProxy.isValid()) {
                QModelIndex currentRegSource = m_treeProxy->mapToSource(currentRegProxy);
                QModelIndex nameIndex = m_model->index(currentRegSource.row(), 3, currentRegSource.parent());
                m_undoStack->push(new EditCellCommand(m_model, nameIndex, oldName, newName));
            }
        }
    });

    connect(m_regOffsetEdit, &QLineEdit::editingFinished, this, [this]() {
        if (!m_currentRegItem || !m_model) return;
        QString rawOffset = m_regOffsetEdit->text().trimmed();
        QString newOffset = padHexOffsetString(rawOffset);
        m_regOffsetEdit->setText(newOffset);
        QString oldOffset = m_currentRegItem->data("Offset/LSB").toString();
        if (newOffset != oldOffset && !newOffset.isEmpty()) {
            QModelIndex currentRegProxy = this->treeView->currentIndex();
            if (currentRegProxy.isValid()) {
                QModelIndex currentRegSource = m_treeProxy->mapToSource(currentRegProxy);
                QModelIndex offsetIndex = m_model->index(currentRegSource.row(), 1, currentRegSource.parent());
                m_undoStack->push(new EditCellCommand(m_model, offsetIndex, oldOffset, newOffset));
            }
        }
    });

    connect(m_regDescEdit, &QLineEdit::editingFinished, this, [this]() {
        if (!m_currentRegItem || !m_model) return;
        QString newDesc = m_regDescEdit->text();
        QString oldDesc = m_currentRegItem->data("Description").toString();
        if (newDesc != oldDesc) {
            QModelIndex currentRegProxy = this->treeView->currentIndex();
            if (currentRegProxy.isValid()) {
                QModelIndex currentRegSource = m_treeProxy->mapToSource(currentRegProxy);
                QModelIndex descIndex = m_model->index(currentRegSource.row(), 10, currentRegSource.parent());
                m_undoStack->push(new EditCellCommand(m_model, descIndex, oldDesc, newDesc));
            }
        }
    });

    m_bitfieldBar = new RegBitfieldBarWidget(m_regViewWidget);
    m_bitfieldBar->setObjectName("bitfieldBar");
    regViewLayout->addWidget(m_bitfieldBar);

    m_fieldsTableView = new QTableView(m_regViewWidget);
    m_fieldsTableView->setObjectName("fieldsTableView");
    m_fieldsTableView->setAlternatingRowColors(true);
    m_fieldsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_fieldsTableView->setEditTriggers(QAbstractItemView::AllEditTriggers);
    m_fieldsTableView->setModel(m_fieldProxy);
    m_fieldsTableView->setSortingEnabled(true);
    m_fieldsTableView->sortByColumn(1, Qt::AscendingOrder);
    m_fieldsTableView->horizontalHeader()->setStretchLastSection(true);
    m_fieldsTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_fieldsTableView->horizontalHeader()->setSectionResizeMode(10, QHeaderView::Stretch);

    // Set field table delegates
    m_fieldsTableView->setItemDelegateForColumn(1, new RegHexDecBinDelegate(this));       // LSB
    m_fieldsTableView->setItemDelegateForColumn(2, new RegHexDecBinDelegate(this));       // Size
    m_fieldsTableView->setItemDelegateForColumn(3, new RegStrDelegate(this));             // Name
    m_fieldsTableView->setItemDelegateForColumn(4, new RegAccessPolicyDelegate(this));   // SW Access Policy
    m_fieldsTableView->setItemDelegateForColumn(5, new RegHwAccessDelegate(this));       // HW Access Policy
    m_fieldsTableView->setItemDelegateForColumn(6, new RegHexDecBinDelegate(this));       // Reset Value
    m_fieldsTableView->setItemDelegateForColumn(7, new RegBoolDelegate(this));           // Is Rand
    m_fieldsTableView->setItemDelegateForColumn(8, new RegBoolDelegate(this));           // Volatile
    m_fieldsTableView->setItemDelegateForColumn(9, new RegBoolDelegate(this));           // Has Reset
    m_fieldsTableView->setItemDelegateForColumn(10, new RegMapDelegate(this));          // Description
    m_fieldsTableView->setColumnHidden(0, true);
    regViewLayout->addWidget(m_fieldsTableView);

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
    QHBoxLayout *blockHeaderLayout = new QHBoxLayout(m_blockHeaderWidget);
    blockHeaderLayout->setContentsMargins(4, 2, 4, 2);
    blockHeaderLayout->setSpacing(8);

    QLabel *blkNameTitle = new QLabel(tr("Name:"), m_blockHeaderWidget);
    blkNameTitle->setStyleSheet("font-weight: bold; font-size: 12px;");

    m_blkNameEdit = new QLineEdit(m_blockHeaderWidget);
    m_blkNameEdit->setObjectName("blkNameEdit");
    m_blkNameEdit->setPlaceholderText(tr("Block name..."));
    m_blkNameEdit->setClearButtonEnabled(true);
    m_blkNameEdit->setMinimumWidth(120);

    QLabel *blkOffsetTitle = new QLabel(tr("Offset:"), m_blockHeaderWidget);
    blkOffsetTitle->setStyleSheet("font-weight: bold; font-size: 12px;");

    m_blkOffsetEdit = new QLineEdit(m_blockHeaderWidget);
    m_blkOffsetEdit->setObjectName("blkOffsetEdit");
    m_blkOffsetEdit->setPlaceholderText(tr("0x0000"));
    m_blkOffsetEdit->setStyleSheet("font-family: monospace; font-size: 12px;");
    m_blkOffsetEdit->setClearButtonEnabled(true);
    m_blkOffsetEdit->setMaximumWidth(100);

    QLabel *blkDescTitle = new QLabel(tr("Description:"), m_blockHeaderWidget);
    blkDescTitle->setStyleSheet("font-weight: bold; font-size: 12px;");

    m_blkDescEdit = new QLineEdit(m_blockHeaderWidget);
    m_blkDescEdit->setObjectName("blkDescEdit");
    m_blkDescEdit->setPlaceholderText(tr("Block description..."));
    m_blkDescEdit->setClearButtonEnabled(true);

    blockHeaderLayout->addWidget(blkNameTitle);
    blockHeaderLayout->addWidget(m_blkNameEdit);
    blockHeaderLayout->addWidget(blkOffsetTitle);
    blockHeaderLayout->addWidget(m_blkOffsetEdit);
    blockHeaderLayout->addWidget(blkDescTitle);
    blockHeaderLayout->addWidget(m_blkDescEdit, 1);

    blockViewLayout->addWidget(m_blockHeaderWidget);

    connect(m_blkNameEdit, &QLineEdit::editingFinished, this, [this]() {
        if (!m_currentBlkItem || !m_model) return;
        QString newName = m_blkNameEdit->text().trimmed();
        QString oldName = m_currentBlkItem->data("Name").toString();
        if (newName != oldName && !newName.isEmpty()) {
            QModelIndex currentBlkProxy = this->treeView->currentIndex();
            if (currentBlkProxy.isValid()) {
                QModelIndex currentBlkSource = m_treeProxy->mapToSource(currentBlkProxy);
                QModelIndex nameIndex = m_model->index(currentBlkSource.row(), 3, currentBlkSource.parent());
                m_undoStack->push(new EditCellCommand(m_model, nameIndex, oldName, newName));
            }
        }
    });

    connect(m_blkOffsetEdit, &QLineEdit::editingFinished, this, [this]() {
        if (!m_currentBlkItem || !m_model) return;
        QString rawOffset = m_blkOffsetEdit->text().trimmed();
        QString newOffset = padHexOffsetString(rawOffset);
        m_blkOffsetEdit->setText(newOffset);
        QString oldOffset = m_currentBlkItem->data("Offset/LSB").toString();
        if (newOffset != oldOffset && !newOffset.isEmpty()) {
            QModelIndex currentBlkProxy = this->treeView->currentIndex();
            if (currentBlkProxy.isValid()) {
                QModelIndex currentBlkSource = m_treeProxy->mapToSource(currentBlkProxy);
                QModelIndex offsetIndex = m_model->index(currentBlkSource.row(), 1, currentBlkSource.parent());
                m_undoStack->push(new EditCellCommand(m_model, offsetIndex, oldOffset, newOffset));
            }
        }
    });

    connect(m_blkDescEdit, &QLineEdit::editingFinished, this, [this]() {
        if (!m_currentBlkItem || !m_model) return;
        QString newDesc = m_blkDescEdit->text();
        QString oldDesc = m_currentBlkItem->data("Description").toString();
        if (newDesc != oldDesc) {
            QModelIndex currentBlkProxy = this->treeView->currentIndex();
            if (currentBlkProxy.isValid()) {
                QModelIndex currentBlkSource = m_treeProxy->mapToSource(currentBlkProxy);
                QModelIndex descIndex = m_model->index(currentBlkSource.row(), 10, currentBlkSource.parent());
                m_undoStack->push(new EditCellCommand(m_model, descIndex, oldDesc, newDesc));
            }
        }
    });

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
    // Page 2: Empty View (Shown when no register or block is selected)
    // ==========================================
    m_emptyViewWidget = new QWidget(m_rightStackedWidget);
    m_emptyViewWidget->setObjectName("emptyViewWidget");
    m_rightStackedWidget->addWidget(m_emptyViewWidget);
    m_rightStackedWidget->setCurrentWidget(m_emptyViewWidget);

    rightLayout->addWidget(m_rightStackedWidget);

    // Connect block memory map diagram navigation
    connect(m_blockMemoryMapWidget, &BlockMemoryMapWidget::registerClicked, this, &RegMapWindow::navigateToRegister);

    // Splitter setup
    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_splitter->addWidget(leftPanel);
    m_splitter->addWidget(rightPanel);
    m_splitter->setSizes(QList<int>() << 350 << 850);
    this->setCentralWidget(m_splitter);

    // Synchronize Bitfield Bar click with Fields Table selection (bidirectional cross-probing)
    connect(m_bitfieldBar, &RegBitfieldBarWidget::fieldClicked, this, [this](int childRow) {
        if (!m_fieldProxy || !m_fieldsTableView || !m_model) return;
        QModelIndex currentRegProxy = this->treeView->currentIndex();
        if (!currentRegProxy.isValid()) return;
        QModelIndex currentRegSource = m_treeProxy->mapToSource(currentRegProxy);
        QModelIndex regCol0 = m_model->index(currentRegSource.row(), 0, currentRegSource.parent());
        QModelIndex fieldSource = m_model->index(childRow, 0, regCol0);
        if (!fieldSource.isValid()) return;
        QModelIndex fieldProxy = m_fieldProxy->mapFromSource(fieldSource);
        if (fieldProxy.isValid()) {
            QModelIndex root = m_fieldsTableView->rootIndex();
            QModelIndex tableIdx = m_fieldProxy->index(fieldProxy.row(), 1, root);
            if (!tableIdx.isValid()) tableIdx = fieldProxy;
            m_fieldsTableView->setCurrentIndex(tableIdx);
            m_fieldsTableView->selectionModel()->select(tableIdx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            m_fieldsTableView->scrollTo(tableIdx, QAbstractItemView::PositionAtCenter);
        }
    });

    connectFieldsTableSignals();

    // Connect tree selection change
    connect(this->treeView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &RegMapWindow::updateFieldsTable);

    // Menu and Action connections
    connect(actionFileNew,      &QAction::triggered, this, &RegMapWindow::btnFileNew);
    connect(actionFileOpen,     &QAction::triggered, this, &RegMapWindow::btnFileOpen);
    connect(actionFileClose,    &QAction::triggered, this, &RegMapWindow::btnFileClose);
    connect(actionFileSave,     &QAction::triggered, this, &RegMapWindow::btnFileSave);
    connect(actionFileSaveAs,   &QAction::triggered, this, &RegMapWindow::btnFileSaveAs);
    connect(actionFileReload,   &QAction::triggered, this, &RegMapWindow::btnFileReload);
    connect(actionAddMem,       &QAction::triggered, this, [this]{ insertChild(RegMapTreeItem::e_rmmKind::mem); });
    connect(actionAddRegBlock,  &QAction::triggered, this, [this]{ insertChild(RegMapTreeItem::e_rmmKind::blk); });
    connect(actionAddRegField,  &QAction::triggered, this, [this]{ insertChild(RegMapTreeItem::e_rmmKind::fld); });
    connect(actionAddRegMap,    &QAction::triggered, this, [this]{ insertChild(RegMapTreeItem::e_rmmKind::map); });
    connect(actionAddReg,       &QAction::triggered, this, [this]{ insertChild(RegMapTreeItem::e_rmmKind::reg); });
    connect(actionDeleteItem,   &QAction::triggered, this, &RegMapWindow::btnDeleteItem);
    connect(actionCheck,        &QAction::triggered, this, &RegMapWindow::btnCheck);
    connect(actionExport,       &QAction::triggered, this, &RegMapWindow::btnExport);
    connect(actionQuit,         &QAction::triggered, this, &RegMapWindow::btnQuitButton);
    connect(actionAbout,        &QAction::triggered, this, &RegMapWindow::btnAbout);
    connect(actionConfig,       &QAction::triggered, this, &RegMapWindow::btnConfig);
    connect(actionPreferences,  &QAction::triggered, this, &RegMapWindow::btnPreferences);
    connect(actionColorBlindMode, &QAction::toggled, this, &RegMapWindow::onToggleColorBlindMode);
    connect(actionKeyboardShortcuts, &QAction::triggered, this, &RegMapWindow::btnKeyBindings);

    this->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this->treeView, &QWidget::customContextMenuRequested, this, &RegMapWindow::showTreeContextMenu);

    m_fieldsTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_fieldsTableView, &QWidget::customContextMenuRequested, this, &RegMapWindow::showTreeContextMenu);

    setupThemeMenu();
    setColourBlindMode(AppSettings::instance().colorBlindMode());
    setColourScheme(AppSettings::instance().colorScheme());

    restoreWindowStateFromSettings();

    if (!m_rmap_filename.isNull() && !m_rmap_filename.isEmpty()) {
        fileOpen(rmap_filename);
    }

    this->treeView->setItemDelegateForColumn(1, new RegHexDecBinDelegate(this));       // Offset
    this->treeView->setItemDelegateForColumn(2, new RegHexDecBinDelegate(this));       // Size
    this->treeView->setItemDelegateForColumn(3, new RegStrDelegate(this));             // Name
    this->treeView->setItemDelegateForColumn(4, new RegAccessPolicyDelegate(this));   // SW Access
    this->treeView->setItemDelegateForColumn(5, new RegHwAccessDelegate(this));       // HW Access
    this->treeView->setItemDelegateForColumn(6, new RegHexDecBinDelegate(this));       // Reset Value
    this->treeView->setItemDelegateForColumn(7, new RegBoolDelegate(this));           // Is Rand
    this->treeView->setItemDelegateForColumn(8, new RegBoolDelegate(this));           // Volatile
    this->treeView->setItemDelegateForColumn(9, new RegBoolDelegate(this));           // Has Reset
    this->treeView->setItemDelegateForColumn(10, new RegMapDelegate(this));          // Description
}

RegMapWindow::~RegMapWindow()
{
    saveWindowStateToSettings();
    if (m_undoStack) {
        m_undoStack->clear();
    }
    if (m_fieldsTableView) {
        m_fieldsTableView->setModel(nullptr);
    }
    if (this->treeView) {
        this->treeView->setModel(nullptr);
    }
    if (m_treeProxy) {
        m_treeProxy->setSourceModel(nullptr);
    }
    if (m_fieldProxy) {
        m_fieldProxy->setSourceModel(nullptr);
    }
    delete m_pref_window;
    m_pref_window = nullptr;
    delete m_config_window;
    m_config_window = nullptr;
    delete m_model;
    m_model = nullptr;
}

void RegMapWindow::onSearchTextChanged(const QString &text)
{
    if (m_treeProxy) {
        m_treeProxy->setSearchFilter(text);
        if (!text.isEmpty()) {
            this->treeView->expandAll();
        }
    }
}

void RegMapWindow::btnConfig(void)
{
    m_config_window->show();
    m_config_window->raise();
    m_config_window->activateWindow();
}

void RegMapWindow::btnPreferences(void)
{
    if (m_pref_window) {
        m_pref_window->show();
        m_pref_window->raise();
        m_pref_window->activateWindow();
    }
}

void RegMapWindow::onToggleColorBlindMode(bool checked)
{
    this->setProperty("colorBlindMode", checked);
    AppSettings::instance().setColorBlindMode(checked);
    if (m_bitfieldBar) {
        m_bitfieldBar->setColorBlindMode(checked);
    }
    if (m_blockMemoryMapWidget) {
        m_blockMemoryMapWidget->setColorBlindMode(checked);
    }
    if (m_pref_window) {
        m_pref_window->setColourBlindMode(checked);
    }
    if (actionColorBlindMode && actionColorBlindMode->isChecked() != checked) {
        actionColorBlindMode->setChecked(checked);
    }
    if (this->treeView) {
        this->treeView->viewport()->update();
    }
    if (m_fieldsTableView) {
        m_fieldsTableView->viewport()->update();
    }
    if (this->statusBar()) {
        this->statusBar()->showMessage(
            checked ? tr("Colour-Blind Mode (Barrier-Free CVD Palette) Enabled")
                    : tr("Standard Colour Palette Active"),
            3000
        );
    }
}

void RegMapWindow::setColourBlindMode(bool enabled)
{
    onToggleColorBlindMode(enabled);
}

bool RegMapWindow::isColourBlindMode() const
{
    return m_bitfieldBar ? m_bitfieldBar->isColorBlindMode() : false;
}

void RegMapWindow::setupThemeMenu(void)
{
    if (!menuView) return;

    QMenu* themeMenu = new QMenu(tr("&Colour Scheme"), this);
    themeMenu->setObjectName("menuColourScheme");
    m_themeActionGroup = new QActionGroup(this);
    m_themeActionGroup->setExclusive(true);

    for (const auto& t : ThemeManager::instance().availableThemes()) {
        QAction* act = themeMenu->addAction(t.name);
        act->setCheckable(true);
        act->setData(t.id);
        if (t.id == ThemeManager::instance().currentThemeId()) {
            act->setChecked(true);
        }
        m_themeActionGroup->addAction(act);
        connect(act, &QAction::triggered, this, [this, id = t.id]() {
            setColourScheme(id);
        });
    }

    menuView->addMenu(themeMenu);
}

void RegMapWindow::setColourScheme(const QString &scheme)
{
    ThemeManager::instance().setTheme(scheme);
    AppSettings::instance().setColorScheme(scheme);
    if (m_pref_window) {
        m_pref_window->setColourScheme(scheme);
    }
    if (m_themeActionGroup) {
        for (auto *act : m_themeActionGroup->actions()) {
            if (act->data().toString() == ThemeManager::instance().currentThemeId()) {
                act->setChecked(true);
                break;
            }
        }
    }
    if (this->treeView) {
        this->treeView->viewport()->update();
    }
    if (m_fieldsTableView) {
        m_fieldsTableView->viewport()->update();
    }
    if (m_bitfieldBar) {
        m_bitfieldBar->update();
    }
    if (m_blockMemoryMapWidget) {
        m_blockMemoryMapWidget->update();
    }
    if (this->statusBar()) {
        this->statusBar()->showMessage(
            tr("Colour Scheme: %1").arg(ThemeManager::instance().currentThemeName()),
            3000
        );
    }
}

QString RegMapWindow::colourScheme() const
{
    return ThemeManager::instance().currentThemeId();
}

void RegMapWindow::restoreWindowStateFromSettings()
{
    QByteArray geom = AppSettings::instance().mainWindowGeometry();
    if (!geom.isEmpty()) {
        restoreGeometry(geom);
    } else {
        QSize sz = AppSettings::instance().mainWindowSize();
        QPoint p = AppSettings::instance().mainWindowPos();
        if (sz.isValid() && sz.width() > 0 && sz.height() > 0) {
            resize(sz);
        } else {
            resize(1200, 800);
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
    }
}

void RegMapWindow::saveWindowStateToSettings()
{
    AppSettings::instance().setMainWindowGeometry(saveGeometry());
    AppSettings::instance().setMainWindowState(saveState());
    AppSettings::instance().setMainWindowPos(pos());
    AppSettings::instance().setMainWindowSize(size());
    if (m_splitter) {
        AppSettings::instance().setMainWindowSplitter(m_splitter->saveState());
    }
}

void RegMapWindow::closeEvent(QCloseEvent *event)
{
    saveWindowStateToSettings();
    QMainWindow::closeEvent(event);
}

void RegMapWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    AppSettings::instance().setMainWindowSize(size());
}

void RegMapWindow::moveEvent(QMoveEvent *event)
{
    QMainWindow::moveEvent(event);
    AppSettings::instance().setMainWindowPos(pos());
}

void RegMapWindow::btnAbout(void)
{
    QMessageBox::information(this,
            tr("About"),
            tr("Version: 0.2.0\n"
               "rmap — Hardware Register Map Designer & Model Generator\n"
               "Designed for ASIC, FPGA, Verification, and Embedded Engineers.\n"),
            QMessageBox::Ok);
}

void RegMapWindow::btnKeyBindings(void)
{
    QDialog dialog(this, Qt::Window);
    dialog.setWindowTitle(tr("Keyboard Shortcuts & Key Bindings"));
    dialog.resize(580, 480);
    dialog.setMinimumSize(450, 350);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QTextBrowser *browser = new QTextBrowser(&dialog);
    browser->setOpenExternalLinks(true);
    browser->setHtml(
        "<h3>rmap — Keyboard Shortcuts & Key Bindings</h3>"
        "<table border='0' cellspacing='4' cellpadding='4' width='100%'>"
        "<tr style='background-color:#EAECEE;'><th align='left'><b>Category</b></th><th align='left'><b>Shortcut</b></th><th align='left'><b>Description</b></th></tr>"
        "<tr><td colspan='3' style='padding-top:8px;'><b>File Operations</b></td></tr>"
        "<tr><td>File</td><td><kbd>Ctrl+N</kbd></td><td>Create New Register Map</td></tr>"
        "<tr><td>File</td><td><kbd>Ctrl+O</kbd></td><td>Open File (SVD, RDL, XML, JSON, CSV, RMT, RMB)</td></tr>"
        "<tr><td>File</td><td><kbd>Ctrl+W</kbd></td><td>Close Register Map Model</td></tr>"
        "<tr><td>File</td><td><kbd>Ctrl+S</kbd></td><td>Save Register Map</td></tr>"
        "<tr><td>File</td><td><kbd>Ctrl+Shift+S</kbd></td><td>Save Register Map As...</td></tr>"
        "<tr><td>File</td><td><kbd>Ctrl+R</kbd></td><td>Reload Active File</td></tr>"
        "<tr><td>File</td><td><kbd>Ctrl+Q</kbd></td><td>Quit Application</td></tr>"
        "<tr><td colspan='3' style='padding-top:8px;'><b>Edit & History</b></td></tr>"
        "<tr><td>Edit</td><td><kbd>Ctrl+Z</kbd></td><td>Undo Last Action</td></tr>"
        "<tr><td>Edit</td><td><kbd>Ctrl+Y</kbd></td><td>Redo Last Action</td></tr>"
        "<tr><td colspan='3' style='padding-top:8px;'><b>Hardware Structure Elements</b></td></tr>"
        "<tr><td>Structure</td><td><kbd>Ctrl+Shift+B</kbd></td><td>Add Register Block (blk)</td></tr>"
        "<tr><td>Structure</td><td><kbd>Ctrl+Shift+R</kbd></td><td>Add Register (reg)</td></tr>"
        "<tr><td>Structure</td><td><kbd>Ctrl+Shift+F</kbd></td><td>Add Bitfield (fld)</td></tr>"
        "<tr><td>Structure</td><td><kbd>Ctrl+Shift+M</kbd></td><td>Add Memory Region (mem)</td></tr>"
        "<tr><td>Structure</td><td><kbd>Ctrl+M</kbd></td><td>Add Address Map (map)</td></tr>"
        "<tr><td>Structure</td><td><kbd>Delete</kbd></td><td>Delete Selected Item</td></tr>"
        "<tr><td colspan='3' style='padding-top:8px;'><b>Validation & Code Generation</b></td></tr>"
        "<tr><td>Tools</td><td><kbd>Ctrl+K</kbd></td><td>Run Architectural Linter / Overlap Check</td></tr>"
        "<tr><td>Tools</td><td><kbd>Ctrl+E</kbd></td><td>Export & Generate Hardware/Software Models</td></tr>"
        "<tr><td>Tools</td><td><kbd>Ctrl+P</kbd></td><td>Open Preferences & Configuration Dialog</td></tr>"
        "<tr><td colspan='3' style='padding-top:8px;'><b>View & Accessibility</b></td></tr>"
        "<tr><td>View</td><td><kbd>Ctrl+Alt+C</kbd></td><td>Toggle Color-Blind Mode (Barrier-Free CVD Palette)</td></tr>"
        "<tr><td>Help</td><td><kbd>F1</kbd></td><td>Show Key Bindings & Shortcuts</td></tr>"
        "</table>"
    );
    layout->addWidget(browser);

    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    connect(btnBox, &QDialogButtonBox::rejected, &dialog, &QDialog::accept);
    layout->addWidget(btnBox);

    dialog.exec();
}

void RegMapWindow::btnQuitButton(void)
{
    this->close();
}

void RegMapWindow::btnFileNew(void)
{
    QMessageBox::StandardButton result;
    bool save_status = false;
    if (m_is_regmap_modified) {
        result = QMessageBox::warning(this, tr("New file"),
                tr("This action will remove all unsaved data, do you wish to continue?"),
                QMessageBox::Ok | QMessageBox::Save | QMessageBox::Cancel);
        if (result == QMessageBox::Save) {
            save_status = btnFileSave();
        }
    }
    if (!m_is_regmap_modified || result == QMessageBox::Ok || (result == QMessageBox::Save && save_status)) {
        fileNew();
    }
}

void RegMapWindow::btnFileClose(void)
{
    QMessageBox::StandardButton result;
    bool save_status = false;
    if (m_is_regmap_modified) {
        result = QMessageBox::warning(this, tr("Close model"),
                tr("This action will remove all unsaved data, do you wish to continue?"),
                QMessageBox::Ok | QMessageBox::Save | QMessageBox::Cancel);
        if (result == QMessageBox::Save) {
            save_status = btnFileSave();
        }
    }
    if (!m_is_regmap_modified || result == QMessageBox::Ok || (result == QMessageBox::Save && save_status)) {
        fileNew();
    }
}

bool RegMapWindow::btnFileSave(void)
{
    bool save_status = false;
    if (m_rmap_filename.isNull() || m_rmap_filename.isEmpty()) {
        save_status = btnFileSaveAs();
    } else {
        save_status = fileSave();
    }
    return save_status;
}

bool RegMapWindow::btnFileSaveAs(void)
{
    QString fname = m_rmap_filename.isEmpty() ? m_default_filename : m_rmap_filename;
    QString selectedFilter;
    QString chosen = QFileDialog::getSaveFileName(
        this,
        tr("Save Register Map As"),
        fname,
        FormatManager::instance().allFilterString(),
        &selectedFilter
    );

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

void RegMapWindow::btnFileOpen(void)
{
    QMessageBox::StandardButton result;
    if (m_is_regmap_modified) {
        result = QMessageBox::warning(this, tr("Open file"),
                tr("This action will remove all unsaved data, do you wish to continue?"),
                QMessageBox::Ok | QMessageBox::Cancel);
    }
    if (!m_is_regmap_modified || result == QMessageBox::Ok) {
        QString fname = QFileDialog::getOpenFileName(
                this,
                tr("Open Register Map File"),
                m_active_folder,
                FormatManager::instance().allFilterString());
        if (!fname.isEmpty() && !fname.isNull()) {
            fileOpen(fname);
            QString filename = m_default_window_title + " - " + fname;
            this->setWindowTitle(filename);
        }
    }
}

void RegMapWindow::btnFileReload(void)
{
    QMessageBox::StandardButton result;
    if (m_is_regmap_modified) {
        result = QMessageBox::warning(this, tr("Reload file"),
                tr("This action will remove all unsaved data, do you wish to continue?"),
                QMessageBox::Ok | QMessageBox::Cancel);
    }
    if (!m_is_regmap_modified || result == QMessageBox::Ok) {
        QString fname = m_rmap_filename;
        fileOpen(fname);
        QString filename = m_default_window_title + " - " + fname;
        this->setWindowTitle(filename);
    }
}

void RegMapWindow::btnCheck(void)
{
    protormap::Config* cfg = m_config_window->serialize();
    uint32_t regWidth = cfg->reg_width() > 0 ? cfg->reg_width() : 32;
    delete cfg;

    QStringList errors = m_model->checkData(regWidth);
    this->treeView->viewport()->update();
    if (m_fieldsTableView) {
        m_fieldsTableView->viewport()->update();
    }
    if (m_bitfieldBar) {
        m_bitfieldBar->update();
    }

    if (errors.isEmpty()) {
        QMessageBox::information(
            this,
            tr("Check Successful"),
            tr("✓ Register Map validation successful!\n\nNo overlapping addresses, bitfield collisions, or register width violations were found."),
            QMessageBox::Ok
        );
    } else {
        QString errorSummary = tr("⚠ Validation found %1 issue(s):\n\n").arg(errors.size());
        for (const QString &err : errors) {
            errorSummary += QString("• %1\n").arg(err);
        }
        errorSummary += tr("\nProblematic cells have been highlighted in red.");

        QMessageBox::warning(
            this,
            tr("Check Issues Found"),
            errorSummary,
            QMessageBox::Ok
        );
    }
}

void RegMapWindow::btnExport(void)
{
    QString baseDir = m_config_window->baseDir();

    protormap::Config* cfg = m_config_window->serialize();
    uint32_t regWidth = cfg->reg_width() > 0 ? cfg->reg_width() : 32;

    QStringList validationErrors = m_model->checkData(regWidth);
    this->treeView->viewport()->update();
    if (m_fieldsTableView) {
        m_fieldsTableView->viewport()->update();
    }

    if (!validationErrors.isEmpty()) {
        QString msg = tr("Validation found %1 issue(s):\n\n").arg(validationErrors.size());
        for (const QString &err : validationErrors) {
            msg += QString("• %1\n").arg(err);
        }
        msg += tr("\nDo you want to proceed with export anyway?");

        auto reply = QMessageBox::question(
            this,
            tr("Validation Warnings"),
            msg,
            QMessageBox::Yes | QMessageBox::No
        );
        if (reply != QMessageBox::Yes) {
            delete cfg;
            return;
        }
    }

    CodeGenerator cg;
    std::string template_folder = cfg->templatefolder().empty() ? PathUtils::DEFAULT_TEMPLATES_DIR : cfg->templatefolder();
    std::string default_output  = cfg->outputfolder().empty() ? PathUtils::DEFAULT_OUTPUT_DIR : cfg->outputfolder();

    try {
        json jsonData = m_model->extractJsonData(regWidth);
        jsonData["project_name"] = cfg->project_name();
        jsonData["project_version"] = cfg->project_version();
        for (const auto& [key, value] : cfg->custom_parameters()) {
            jsonData[key] = value;
        }

        std::vector<TemplateMapping> mappings;
        for (const auto& entry : cfg->template_outputs()) {
            if (!entry.template_filename().empty()) {
                mappings.push_back({entry.template_filename(), entry.output_filepath()});
            }
        }

        GenerationReport report = cg.generate(jsonData, template_folder, default_output, mappings, baseDir.toStdString());

        if (!report.errors.empty()) {
            QString errorMsg = tr("Code generation completed with errors:\n\n");
            for (const auto& err : report.errors) {
                errorMsg += QString("• %1: %2\n").arg(QString::fromStdString(err.first), QString::fromStdString(err.second));
            }
            if (!report.success_files.empty()) {
                errorMsg += tr("\nSuccessfully generated files:\n");
                for (const auto& f : report.success_files) {
                    errorMsg += QString("• %1\n").arg(QString::fromStdString(f));
                }
            }
            QMessageBox::warning(this, tr("Export Issues Detected"), errorMsg, QMessageBox::Ok);
        } else if (report.success_files.empty()) {
            QMessageBox::information(this, tr("Export Notice"),
                tr("No template files were found or specified for generation.\n"
                   "Please check template paths in the Configuration dialog."), QMessageBox::Ok);
        } else {
            QString successMsg = tr("Code generation completed successfully!\n\nGenerated files:\n");
            for (const auto& f : report.success_files) {
                successMsg += QString("• %1\n").arg(QString::fromStdString(f));
            }
            QMessageBox::information(this, tr("Export Successful"), successMsg, QMessageBox::Ok);
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(this, tr("Export Error"),
                tr("Failed to generate files:\n%1").arg(e.what()), QMessageBox::Ok);
    }

    delete cfg;
}

void RegMapWindow::btnDeleteItem(void)
{
    QModelIndex index = this->treeView->currentIndex();
    if (index.isValid() && index.row() >= 0) {
        QModelIndex source_index = m_treeProxy->mapToSource(index);
        m_undoStack->push(new DeleteItemCommand(m_model, source_index.row(), source_index.parent()));
    }
}

void RegMapWindow::connectModelSignals(void)
{
    if (!m_model) return;
    connect(m_model, &RegMapTreeModel::dataChanged, this, [this](const QModelIndex &topLeft, const QModelIndex &bottomRight) {
        Q_UNUSED(topLeft); Q_UNUSED(bottomRight);
        if (m_bitfieldBar) {
            m_bitfieldBar->refresh();
        }
        if (m_blockMemoryMapWidget) {
            m_blockMemoryMapWidget->refresh();
        }
        if (m_currentRegItem) {
            if (m_regNameEdit && !m_regNameEdit->hasFocus()) m_regNameEdit->setText(m_currentRegItem->data("Name").toString());
            if (m_regOffsetEdit && !m_regOffsetEdit->hasFocus()) m_regOffsetEdit->setText(padHexOffsetString(m_currentRegItem->data("Offset/LSB").toString()));
            if (m_regDescEdit && !m_regDescEdit->hasFocus()) m_regDescEdit->setText(m_currentRegItem->data("Description").toString());
        }
        if (m_currentBlkItem) {
            if (m_blkNameEdit && !m_blkNameEdit->hasFocus()) m_blkNameEdit->setText(m_currentBlkItem->data("Name").toString());
            if (m_blkOffsetEdit && !m_blkOffsetEdit->hasFocus()) m_blkOffsetEdit->setText(padHexOffsetString(m_currentBlkItem->data("Offset/LSB").toString()));
            if (m_blkDescEdit && !m_blkDescEdit->hasFocus()) m_blkDescEdit->setText(m_currentBlkItem->data("Description").toString());
        }
    });
    connect(m_model, &QAbstractItemModel::rowsInserted, this, [this](const QModelIndex &parent, int first, int last) {
        Q_UNUSED(parent); Q_UNUSED(first); Q_UNUSED(last);
        if (m_bitfieldBar) {
            m_bitfieldBar->refresh();
        }
        if (m_blockMemoryMapWidget) {
            m_blockMemoryMapWidget->refresh();
        }
    });
    connect(m_model, &QAbstractItemModel::rowsRemoved, this, [this](const QModelIndex &parent, int first, int last) {
        Q_UNUSED(parent); Q_UNUSED(first); Q_UNUSED(last);
        if (m_bitfieldBar) {
            m_bitfieldBar->refresh();
        }
        if (m_blockMemoryMapWidget) {
            m_blockMemoryMapWidget->refresh();
        }
    });
    connect(m_model, &QAbstractItemModel::modelReset, this, [this]() {
        if (m_bitfieldBar) {
            m_bitfieldBar->refresh();
        }
        if (m_blockMemoryMapWidget) {
            m_blockMemoryMapWidget->refresh();
        }
    });
}

void RegMapWindow::connectFieldsTableSignals(void)
{
    if (!m_fieldsTableView || !m_fieldsTableView->selectionModel()) return;

    disconnect(m_fieldsTableView->selectionModel(), nullptr, this, nullptr);

    connect(m_fieldsTableView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [this](const QModelIndex &curr, const QModelIndex &prev) {
        Q_UNUSED(prev);
        if (curr.isValid() && m_bitfieldBar && m_fieldProxy) {
            QModelIndex sourceIdx = m_fieldProxy->mapToSource(curr);
            m_bitfieldBar->setSelectedField(sourceIdx.row());
        } else if (!curr.isValid() && m_bitfieldBar) {
            m_bitfieldBar->setSelectedField(-1);
        }
    });

    connect(m_fieldsTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this](const QItemSelection &selected, const QItemSelection &deselected) {
        Q_UNUSED(deselected);
        if (selected.isEmpty()) {
            if (m_bitfieldBar) m_bitfieldBar->setSelectedField(-1);
            return;
        }
        QModelIndex firstIdx = selected.indexes().value(0);
        if (firstIdx.isValid() && m_bitfieldBar && m_fieldProxy) {
            QModelIndex sourceIdx = m_fieldProxy->mapToSource(firstIdx);
            m_bitfieldBar->setSelectedField(sourceIdx.row());
        }
    });
}

void RegMapWindow::fileNew(void)
{
    m_model->clear();
    delete m_model;
    m_model = new RegMapTreeModel();
    connectModelSignals();

    m_treeProxy->setSourceModel(m_model);
    m_fieldProxy->setSourceModel(m_model);
    this->treeView->setModel(m_treeProxy);
    this->treeView->setSortingEnabled(true);
    this->treeView->sortByColumn(1, Qt::AscendingOrder);

    connect(this->treeView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &RegMapWindow::updateFieldsTable);

    if (m_fieldsTableView) {
        m_fieldsTableView->setModel(m_fieldProxy);
        m_fieldsTableView->setRootIndex(QModelIndex());
        connectFieldsTableSignals();
    }
    if (m_bitfieldBar) {
        m_bitfieldBar->clear();
    }
    if (m_blockMemoryMapWidget) {
        m_blockMemoryMapWidget->clear();
    }
    if (m_regNameEdit) m_regNameEdit->clear();
    if (m_regOffsetEdit) m_regOffsetEdit->clear();
    if (m_regDescEdit) m_regDescEdit->clear();
    if (m_blkNameEdit) m_blkNameEdit->clear();
    if (m_blkOffsetEdit) m_blkOffsetEdit->clear();
    if (m_blkDescEdit) m_blkDescEdit->clear();
    m_currentRegItem = nullptr;
    m_currentBlkItem = nullptr;

    if (m_rightStackedWidget && m_emptyViewWidget) {
        m_rightStackedWidget->setCurrentWidget(m_emptyViewWidget);
    }

    if (m_undoStack) {
        m_undoStack->clear();
    }

    this->regmap_notModified();
    this->m_rmap_filename = QString();

    if (m_config_window) {
        m_config_window->deserialize(protormap::Config());
    }
}

void RegMapWindow::fileOpen(QString fname)
{
    QString expanded = PathUtils::expandEnvVars(fname);
    QFileInfo check_file(expanded);
    if (check_file.exists() && check_file.isFile()) {
        fileNew();
        this->m_rmap_filename = PathUtils::normalizeSeparators(check_file.filePath());
        m_config_window->setBaseDir(check_file.absolutePath());

        FormatResult res = FormatManager::instance().loadFile(expanded, m_model, m_config_window);
        if (!res.success) {
            QMessageBox::warning(this, tr("Open Error"),
                                tr("Failed to open %1:\n%2").arg(fname, res.errorMessage));
            return;
        }

        this->regmap_notModified();
        this->setWindowTitle(this->m_default_window_title + " (" + this->m_rmap_filename + ")");

        if (this->treeView->model() && this->treeView->model()->rowCount() > 0) {
            this->treeView->expandAll();
            QModelIndex firstIdx = this->treeView->model()->index(0, 0);
            this->treeView->setCurrentIndex(firstIdx);
        }
    } else {
        QMessageBox::critical(this,
                tr("Error file not found"),
                tr("Filename: %1 not found").arg(fname),
                QMessageBox::Ok);
    }
}

protormap::RegModel& operator <<( protormap::RegModel& reg_model, const SerializationContext& context )
{
    for(SerializationContext::Record rec : context.m_records) {
        protormap::RegItem* item = reg_model.add_item();
        switch (rec.m_data["kind"].value<RegMapTreeItem::e_rmmKind>()){
            case RegMapTreeItem::e_rmmKind::root: item->set_kind(protormap::RegItem_Kind_ROOT); break;
            case RegMapTreeItem::e_rmmKind::mem:  item->set_kind(protormap::RegItem_Kind_MEM);  break;
            case RegMapTreeItem::e_rmmKind::map:  item->set_kind(protormap::RegItem_Kind_MAP);  break;
            case RegMapTreeItem::e_rmmKind::blk:  item->set_kind(protormap::RegItem_Kind_BLK);  break;
            case RegMapTreeItem::e_rmmKind::reg:  item->set_kind(protormap::RegItem_Kind_REG);  break;
            case RegMapTreeItem::e_rmmKind::fld:  item->set_kind(protormap::RegItem_Kind_FLD);  break;
        }
        item->set_id(rec.m_data["id"].toUInt());
        item->set_parent_id(rec.m_data["parent"].toUInt());
        QList<QVariant> childItems = rec.m_data["childItems"].toList();
        for(int i=0; i<childItems.size(); i++) {
            item->add_child_id(childItems.at(i).toUInt());
        }
        QVariantMap itemData = rec.m_data["itemData"].toMap();
        auto & itd = *item->mutable_itemdata();
        for(auto key : itemData.keys()) {
            itd[key.toStdString()] = itemData.value(key).toString().toStdString();
        }
    }
    return(reg_model);
}

protormap::RegModel& operator >>( protormap::RegModel& reg_model, SerializationContext& context )
{
    for (int j = 0; j < reg_model.item_size(); j++) {
        QVariantMap m_data;
        QVariantMap itemData;
        QList<QVariant> childItemsList;
        QObject* object = NULL;

        const protormap::RegItem& item = reg_model.item(j);
        switch (item.kind()){
            case protormap::RegItem_Kind_ROOT: m_data["kind"] = QVariant::fromValue(RegMapTreeItem::e_rmmKind::root); break;
            case protormap::RegItem_Kind_MEM:  m_data["kind"] = QVariant::fromValue(RegMapTreeItem::e_rmmKind::mem);  break;
            case protormap::RegItem_Kind_MAP:  m_data["kind"] = QVariant::fromValue(RegMapTreeItem::e_rmmKind::map);  break;
            case protormap::RegItem_Kind_BLK:  m_data["kind"] = QVariant::fromValue(RegMapTreeItem::e_rmmKind::blk);  break;
            case protormap::RegItem_Kind_REG:  m_data["kind"] = QVariant::fromValue(RegMapTreeItem::e_rmmKind::reg);  break;
            case protormap::RegItem_Kind_FLD:  m_data["kind"] = QVariant::fromValue(RegMapTreeItem::e_rmmKind::fld);  break;
            default: ;
        }
        m_data["id"]   = item.id();
        m_data["parent"]= item.parent_id();
        for (const auto &child : item.child_id()) {
            childItemsList.append(child);
        }
        m_data["childItems"] = childItemsList;

        for (const auto & [key, value] : item.itemdata()) {
            itemData[QString(key.c_str())] = QVariant(value.c_str());
        }
        m_data["itemData"] = QVariant(itemData);

        context.append_record(object, m_data );
    }
    return(reg_model);
}

bool RegMapWindow::fileSave(QString fname)
{
    if (fname.isNull() || fname.isEmpty()) {
        fname = m_rmap_filename;
    }
    if (fname.isNull() || fname.isEmpty()) {
        return btnFileSaveAs();
    }

    QString expanded = PathUtils::expandEnvVars(fname);
    QFileInfo saveFi(expanded);
    m_config_window->setBaseDir(saveFi.absolutePath());

    FormatResult res = FormatManager::instance().saveFile(expanded, m_model, m_config_window);
    if (!res.success) {
        QMessageBox::critical(this,
                tr("Failed to write output file"),
                tr("Error writing to file: %1\n%2").arg(fname, res.errorMessage),
                QMessageBox::Ok);
        return false;
    }

    this->m_rmap_filename = PathUtils::normalizeSeparators(saveFi.filePath());
    regmap_notModified();
    return true;
}

void RegMapWindow::regmap_modified(void)
{
    if(!m_is_regmap_modified) {
        m_is_regmap_modified = true;
        QString win_title = this->windowTitle();
        if (!win_title.endsWith('*')) {
            win_title.append('*');
        }
        this->setWindowTitle(win_title);
    }
}

void RegMapWindow::regmap_notModified(void)
{
    if(m_is_regmap_modified) {
        m_is_regmap_modified = false;
        QString win_title = this->windowTitle();
        if (win_title.endsWith('*')) {
            win_title.remove(win_title.size()-1,1);
        }
        this->setWindowTitle(win_title);
    }
}

void RegMapWindow::insertChild(RegMapTreeItem::e_rmmKind kind)
{
    QModelIndexList indexes = this->treeView->selectionModel() ? this->treeView->selectionModel()->selectedIndexes() : QModelIndexList();
    QModelIndex index;
    if (indexes.size() > 0) {
        index = indexes.at(0);
    } else if (this->treeView->selectionModel()) {
        index = this->treeView->selectionModel()->currentIndex();
    }
    QModelIndex source_index = index.isValid() ? m_treeProxy->mapToSource(index) : QModelIndex();

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
                // Ascend ancestor hierarchy to find the nearest parent capable of accepting 'kind'
                RegMapTreeItem *curr = selectedItem;
                QModelIndex currSource = source_index;
                while (curr && curr->parentItem()) {
                    RegMapTreeItem *p = curr->parentItem();
                    QModelIndex pSource = currSource.parent();
                    if (p->possibleChildren().contains(kind)) {
                        targetParentItem = p;
                        targetParentSource = pSource;
                        insertRow = curr->row() + 1; // Insert as sibling immediately following current
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

    m_undoStack->push(new InsertItemCommand(m_model, kind, insertRow, targetParentSource));

    for (int col = 0; col < m_treeProxy->columnCount(); col++) {
        this->treeView->resizeColumnToContents(col);
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
            if (this->treeView->selectionModel()) {
                this->treeView->selectionModel()->setCurrentIndex(newProxyIdx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            }
            this->treeView->scrollTo(newProxyIdx, QAbstractItemView::EnsureVisible);
            updateFieldsTable(newProxyIdx, QModelIndex());
        }
    }

    // For fields, also ensure the fields table highlights the new field
    if (kind == RegMapTreeItem::e_rmmKind::fld && m_fieldsTableView && m_fieldProxy) {
        QModelIndex newFldSource = m_model->index(insertRow, 0, targetParentSource);
        QModelIndex newFldProxy = m_fieldProxy->mapFromSource(newFldSource);
        if (newFldProxy.isValid()) {
            m_fieldsTableView->setCurrentIndex(newFldProxy);
            if (m_fieldsTableView->selectionModel()) {
                m_fieldsTableView->selectionModel()->setCurrentIndex(newFldProxy, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            }
            m_fieldsTableView->scrollTo(newFldProxy, QAbstractItemView::EnsureVisible);
        }
    }
}

void RegMapWindow::updateFieldsTable(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);

    if (!current.isValid()) {
        if (m_fieldsTableView) m_fieldsTableView->setRootIndex(QModelIndex());
        if (m_bitfieldBar) m_bitfieldBar->clear();
        m_currentRegItem = nullptr;
        m_currentBlkItem = nullptr;
        if (m_rightStackedWidget && m_emptyViewWidget) {
            m_rightStackedWidget->setCurrentWidget(m_emptyViewWidget);
        }
        return;
    }

    QModelIndex source_current = m_treeProxy->mapToSource(current);
    QModelIndex source_col0 = m_model->index(source_current.row(), 0, source_current.parent());
    RegMapTreeItem *item = m_model->getItem(source_col0);
    if (!item) {
        if (m_fieldsTableView) m_fieldsTableView->setRootIndex(QModelIndex());
        if (m_bitfieldBar) m_bitfieldBar->clear();
        m_currentRegItem = nullptr;
        m_currentBlkItem = nullptr;
        if (m_rightStackedWidget && m_emptyViewWidget) {
            m_rightStackedWidget->setCurrentWidget(m_emptyViewWidget);
        }
        return;
    }

    if (item->kindString() == "reg") {
        m_currentRegItem = item;
        m_currentBlkItem = nullptr;
        if (m_regHeaderWidget) {
            m_regHeaderWidget->setVisible(true);
            if (m_regNameEdit) m_regNameEdit->setText(item->data("Name").toString());
            if (m_regOffsetEdit) m_regOffsetEdit->setText(padHexOffsetString(item->data("Offset/LSB").toString()));
            if (m_regDescEdit) m_regDescEdit->setText(item->data("Description").toString());
        }

        if (m_fieldsTableView) {
            if (m_fieldsTableView->model() != m_fieldProxy) {
                m_fieldsTableView->setModel(m_fieldProxy);
                connectFieldsTableSignals();
            }
            m_fieldsTableView->setRootIndex(m_fieldProxy->mapFromSource(source_col0));
            for (int col = 1; col < 10; ++col) {
                m_fieldsTableView->resizeColumnToContents(col);
            }
            m_fieldsTableView->horizontalHeader()->setStretchLastSection(true);
            m_fieldsTableView->horizontalHeader()->setSectionResizeMode(10, QHeaderView::Stretch);
        }

        uint32_t regWidth = 32;
        if (m_config_window) {
            protormap::Config* cfg = m_config_window->serialize();
            if (cfg && cfg->reg_width() > 0) regWidth = cfg->reg_width();
            delete cfg;
        }

        if (m_bitfieldBar) {
            m_bitfieldBar->setRegister(item, regWidth);
        }

        if (m_rightStackedWidget && m_regViewWidget) {
            m_rightStackedWidget->setCurrentWidget(m_regViewWidget);
        }
    } else if (item->kindString() == "blk" || item->kindString() == "map") {
        updateBlockView(item);
    } else {
        m_currentRegItem = nullptr;
        m_currentBlkItem = nullptr;
        if (m_rightStackedWidget && m_emptyViewWidget) {
            m_rightStackedWidget->setCurrentWidget(m_emptyViewWidget);
        }
    }
}

void RegMapWindow::updateBlockView(RegMapTreeItem *blkItem)
{
    if (!blkItem) return;
    m_currentBlkItem = blkItem;
    m_currentRegItem = nullptr;

    if (m_blkNameEdit) m_blkNameEdit->setText(blkItem->data("Name").toString());
    if (m_blkOffsetEdit) m_blkOffsetEdit->setText(padHexOffsetString(blkItem->data("Offset/LSB").toString()));
    if (m_blkDescEdit) m_blkDescEdit->setText(blkItem->data("Description").toString());

    uint32_t regWidth = 32;
    if (m_config_window) {
        protormap::Config* cfg = m_config_window->serialize();
        if (cfg && cfg->reg_width() > 0) regWidth = cfg->reg_width();
        delete cfg;
    }

    if (m_blockMemoryMapWidget) {
        m_blockMemoryMapWidget->setBlock(blkItem, regWidth);
    }

    if (m_rightStackedWidget && m_blockViewWidget) {
        m_rightStackedWidget->setCurrentWidget(m_blockViewWidget);
    }
}

void RegMapWindow::navigateToRegister(int childRow, RegMapTreeItem *regItem)
{
    Q_UNUSED(regItem);
    if (!m_model || !m_treeProxy || !this->treeView || !m_currentBlkItem) return;

    QModelIndex blkProxy = this->treeView->currentIndex();
    if (!blkProxy.isValid()) return;
    QModelIndex blkSource = m_treeProxy->mapToSource(blkProxy);
    QModelIndex blkCol0 = m_model->index(blkSource.row(), 0, blkSource.parent());

    QModelIndex regSource = m_model->index(childRow, 0, blkCol0);
    if (!regSource.isValid()) return;

    QModelIndex regProxy = m_treeProxy->mapFromSource(regSource);
    if (regProxy.isValid()) {
        this->treeView->setCurrentIndex(regProxy);
        this->treeView->selectionModel()->select(regProxy, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        this->treeView->scrollTo(regProxy, QAbstractItemView::PositionAtCenter);
    }
}

void RegMapWindow::showTreeContextMenu(const QPoint &pos)
{
    QPoint globalPos;
    QModelIndex index;
    QWidget* senderWidget = qobject_cast<QWidget*>(sender());

    if (senderWidget == this->treeView) {
        index = this->treeView->indexAt(pos);
        globalPos = this->treeView->viewport()->mapToGlobal(pos);
    } else if (senderWidget == m_fieldsTableView) {
        index = m_fieldsTableView->indexAt(pos);
        globalPos = m_fieldsTableView->viewport()->mapToGlobal(pos);
    }

    QMenu menu(this);
    if (index.isValid()) {
        QAction* duplicateAction = menu.addAction(tr("Duplicate"));
        QAction* deleteAction = menu.addAction(tr("Delete"));
        connect(duplicateAction, &QAction::triggered, this, [this, index, senderWidget]() {
            if (senderWidget == this->treeView) {
                duplicateItem(index);
            } else {
                QModelIndex source = m_fieldProxy->mapToSource(index);
                duplicateItem(m_treeProxy->mapFromSource(source));
            }
        });
        connect(deleteAction, &QAction::triggered, this, &RegMapWindow::btnDeleteItem);
    }
    menu.exec(globalPos);
}

void RegMapWindow::duplicateItem(const QModelIndex &index)
{
    QModelIndex source_index = m_treeProxy->mapToSource(index);
    RegMapTreeItem* item = m_model->getItem(source_index);
    if (!item || item->kindString() == "root") return;

    QVariantMap rootSerial;
    SerializationContext context;
    item->serialize(rootSerial, &context);

    int row = source_index.row() + 1;
    QModelIndex parent = source_index.parent();

    m_model->insertRows(row, 1, item->kind(), parent);
    QModelIndex new_source = m_model->index(row, 0, parent);
    RegMapTreeItem* newItem = m_model->getItem(new_source);

    newItem->deserialize(rootSerial, &context);

    QString oldName = item->data("Name").toString();
    newItem->setData("Name", oldName + "_copy");

    bool ok1, ok2;
    uint64_t offset = item->data("Offset/LSB").toString().toULongLong(&ok1, 16);
    if (!ok1) offset = item->data("Offset/LSB").toString().toULongLong(&ok1, 10);
    uint64_t size = item->data("Size/Width").toString().toULongLong(&ok2, 10);
    if (item->kindString() == "reg") {
        size = 4;
    }
    if (ok1 && ok2) {
        QString newOff = QString("0x%1").arg(offset + size, 0, 16);
        newItem->setData("Offset/LSB", newOff);
    }

    this->treeView->viewport()->update();
}

bool RegMapWindow::headlessExport(const QString &out_dir)
{
    if (m_rmap_filename.isEmpty()) {
        std::cerr << "No input file specified for headless export" << std::endl;
        return false;
    }

    QString baseDir = m_config_window->baseDir();

    protormap::Config* cfg = m_config_window->serialize();
    uint32_t regWidth = cfg->reg_width() > 0 ? cfg->reg_width() : 32;

    QStringList validationErrors = m_model->checkData(regWidth);
    if (!validationErrors.isEmpty()) {
        std::cerr << "Validation warnings found:" << std::endl;
        for (const QString &err : validationErrors) {
            std::cerr << " - " << err.toStdString() << std::endl;
        }
    }

    CodeGenerator cg;
    std::string template_folder = cfg->templatefolder().empty() ? PathUtils::DEFAULT_TEMPLATES_DIR : cfg->templatefolder();
    std::string default_output = out_dir.isEmpty() ? (cfg->outputfolder().empty() ? PathUtils::DEFAULT_OUTPUT_DIR : cfg->outputfolder()) : PathUtils::expandEnvVars(out_dir).toStdString();

    try {
        json jsonData = m_model->extractJsonData(regWidth);
        jsonData["project_name"] = cfg->project_name();
        jsonData["project_version"] = cfg->project_version();
        for (const auto& [key, value] : cfg->custom_parameters()) {
            jsonData[key] = value;
        }

        std::vector<TemplateMapping> mappings;
        for (const auto& entry : cfg->template_outputs()) {
            if (!entry.template_filename().empty()) {
                if (!out_dir.isEmpty()) {
                    QFileInfo fi(QString::fromStdString(entry.output_filepath()));
                    QString filename = fi.fileName();
                    QString expOutDir = PathUtils::expandEnvVars(out_dir);
                    QString absOutDir = QDir(QDir::currentPath()).absoluteFilePath(expOutDir);
                    QString customOut = PathUtils::normalizeSeparators(QDir(absOutDir).filePath(filename));
                    mappings.push_back({entry.template_filename(), customOut.toStdString()});
                } else {
                    mappings.push_back({entry.template_filename(), entry.output_filepath()});
                }
            }
        }

        GenerationReport report = cg.generate(jsonData, template_folder, default_output, mappings, baseDir.toStdString());

        if (!report.errors.empty()) {
            std::cerr << "Code generation completed with errors:" << std::endl;
            for (const auto& err : report.errors) {
                std::cerr << err.first << ": " << err.second << std::endl;
            }
            delete cfg;
            return false;
        }
        std::cout << "Successfully exported " << report.success_files.size() << " files." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Export Exception: " << e.what() << std::endl;
        delete cfg;
        return false;
    }

    delete cfg;
    return true;
}

bool RegMapWindow::headlessLint(bool strict, const QString &format, const QString &outFile)
{
    protormap::Config* cfg = m_config_window->serialize();
    uint32_t regWidth = cfg->reg_width() > 0 ? cfg->reg_width() : 32;
    delete cfg;

    QStringList errors = m_model->checkData(regWidth);
    QStringList warnings;

    // Strict checks: check for empty descriptions or unaligned offsets
    if (strict && m_model->getRootItem()) {
        std::function<void(RegMapTreeItem*)> strictCheck = [&](RegMapTreeItem* item) {
            if (!item) return;
            QString kind = item->kindString();
            QString name = item->data("Name").toString();
            QString desc = item->data("Description").toString();

            if ((kind == "reg" || kind == "fld") && desc.trimmed().isEmpty()) {
                warnings.append(QString("Missing description for %1 '%2'").arg(kind.toUpper(), name));
            }
            if (kind == "reg") {
                bool ok;
                uint64_t off = item->data("Offset/LSB").toString().toULongLong(&ok, 16);
                if (!ok) off = item->data("Offset/LSB").toString().toULongLong(&ok, 10);
                uint64_t alignBytes = regWidth / 8;
                if (ok && alignBytes > 0 && (off % alignBytes != 0)) {
                    warnings.append(QString("Register '%1' offset 0x%2 is not %3-byte aligned").arg(name, QString::number(off, 16).toUpper(), QString::number(alignBytes)));
                }
            }
            for (RegMapTreeItem* child : item->getChildItems()) {
                strictCheck(child);
            }
        };
        strictCheck(m_model->getRootItem());
    }

    bool isPassed = errors.isEmpty() && (!strict || warnings.isEmpty());
    QString reportContent;
    QString fmt = format.toLower().trimmed();

    if (fmt == "json") {
        QJsonObject rootObj;
        rootObj["status"] = isPassed ? "PASS" : "FAIL";
        rootObj["file"] = m_rmap_filename;
        QJsonArray errArr, warnArr;
        for (const QString &e : errors) errArr.append(e);
        for (const QString &w : warnings) warnArr.append(w);
        rootObj["errors"] = errArr;
        rootObj["warnings"] = warnArr;
        reportContent = QJsonDocument(rootObj).toJson(QJsonDocument::Indented);
    } else if (fmt == "sarif") {
        QJsonObject sarif;
        sarif["$schema"] = "https://schemastore.azurewebsites.net/schemas/json/sarif-2.1.0-rtm.5.json";
        sarif["version"] = "2.1.0";
        QJsonArray runs;
        QJsonObject run;
        QJsonObject tool;
        QJsonObject driver;
        driver["name"] = "rmap-lint";
        driver["version"] = "0.2.0";
        tool["driver"] = driver;
        run["tool"] = tool;

        QJsonArray results;
        for (const QString &e : errors) {
            QJsonObject res;
            res["level"] = "error";
            QJsonObject msg; msg["text"] = e; res["message"] = msg;
            results.append(res);
        }
        for (const QString &w : warnings) {
            QJsonObject res;
            res["level"] = "warning";
            QJsonObject msg; msg["text"] = w; res["message"] = msg;
            results.append(res);
        }
        run["results"] = results;
        runs.append(run);
        sarif["runs"] = runs;
        reportContent = QJsonDocument(sarif).toJson(QJsonDocument::Indented);
    } else if (fmt == "junit") {
        int totalTests = 1 + warnings.size();
        int failures = errors.size() + (strict ? warnings.size() : 0);
        reportContent = QString(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<testsuites name=\"rmap-lint\" tests=\"%1\" failures=\"%2\">\n"
            "  <testsuite name=\"RegisterMapValidation\" tests=\"%1\" failures=\"%2\">\n"
        ).arg(totalTests).arg(failures);

        if (errors.isEmpty()) {
            reportContent += "    <testcase name=\"AddressAndOverlapCheck\"/>\n";
        } else {
            reportContent += QString("    <testcase name=\"AddressAndOverlapCheck\"><failure message=\"Validation Errors\">%1</failure></testcase>\n")
                             .arg(errors.join("\n"));
        }
        for (int i = 0; i < warnings.size(); ++i) {
            if (strict) {
                reportContent += QString("    <testcase name=\"StrictLint_%1\"><failure message=\"Strict Rule Violation\">%2</failure></testcase>\n")
                                 .arg(i + 1).arg(warnings[i]);
            } else {
                reportContent += QString("    <testcase name=\"StrictLint_%1\"/>\n").arg(i + 1);
            }
        }
        reportContent += "  </testsuite>\n</testsuites>\n";
    } else {
        // Text format
        reportContent = QString("=== rmap Linter Report: %1 ===\n").arg(m_rmap_filename);
        if (errors.isEmpty()) {
            reportContent += "✓ Validation Check: PASSED (0 errors)\n";
        } else {
            reportContent += QString("✗ Validation Check: FAILED (%1 errors)\n").arg(errors.size());
            for (const QString &e : errors) reportContent += QString("  • ERROR: %1\n").arg(e);
        }
        if (!warnings.isEmpty()) {
            reportContent += QString("⚠ Strict Lint Warnings: (%1 warnings)\n").arg(warnings.size());
            for (const QString &w : warnings) reportContent += QString("  • WARN: %1\n").arg(w);
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
            std::cout << "Lint report saved to: " << expOut.toStdString() << std::endl;
        } else {
            std::cerr << "Failed to write lint report to: " << expOut.toStdString() << std::endl;
        }
    } else {
        std::cout << reportContent.toStdString() << std::endl;
    }

    return isPassed;
}

bool RegMapWindow::semanticDiff(const QString &file1, const QString &file2, const QString &format, const QString &outFile)
{
    QString expFile1 = PathUtils::expandEnvVars(file1);
    QString expFile2 = PathUtils::expandEnvVars(file2);
    QString expOut = PathUtils::expandEnvVars(outFile);

    RegMapTreeModel model1, model2;
    RegConfigWindow cfg1, cfg2;

    FormatResult r1 = FormatManager::instance().loadFile(expFile1, &model1, &cfg1);
    if (!r1.success) {
        std::cerr << "Diff error: Failed to load file1: " << r1.errorMessage.toStdString() << std::endl;
        return false;
    }
    FormatResult r2 = FormatManager::instance().loadFile(expFile2, &model2, &cfg2);
    if (!r2.success) {
        std::cerr << "Diff error: Failed to load file2: " << r2.errorMessage.toStdString() << std::endl;
        return false;
    }

    // Build register lookup maps
    struct RegSummary {
        QString blockName;
        QString regName;
        QString offset;
        QString access;
        QString reset;
        std::map<QString, QString> fields; // name -> "lsb:width:access:reset"
    };

    auto gatherRegs = [](RegMapTreeModel &model) -> std::map<QString, RegSummary> {
        std::map<QString, RegSummary> map;
        RegMapTreeItem *root = model.getRootItem();
        if (!root) return map;
        for (RegMapTreeItem *blk : root->getChildItems()) {
            if (!blk || blk->kindString() != "blk") continue;
            QString bName = blk->data("Name").toString();
            for (RegMapTreeItem *reg : blk->getChildItems()) {
                if (!reg || reg->kindString() != "reg") continue;
                QString rName = reg->data("Name").toString();
                QString key = bName + "::" + rName;
                RegSummary s;
                s.blockName = bName;
                s.regName = rName;
                s.offset = reg->data("Offset/LSB").toString();
                s.access = reg->data("Access Policy").toString();
                s.reset = reg->data("Reset Value").toString();
                for (RegMapTreeItem *fld : reg->getChildItems()) {
                    if (fld && fld->kindString() == "fld") {
                        QString fName = fld->data("Name").toString();
                        QString fSig = QString("%1:%2:%3:%4")
                            .arg(fld->data("Offset/LSB").toString())
                            .arg(fld->data("Size/Width").toString())
                            .arg(fld->data("Access Policy").toString())
                            .arg(fld->data("Reset Value").toString());
                        s.fields[fName] = fSig;
                    }
                }
                map[key] = s;
            }
        }
        return map;
    };

    auto map1 = gatherRegs(model1);
    auto map2 = gatherRegs(model2);

    QStringList addedRegs, removedRegs, modifiedRegs;

    for (const auto& [key, s2] : map2) {
        auto it1 = map1.find(key);
        if (it1 == map1.end()) {
            addedRegs.append(key);
        } else {
            const auto &s1 = it1->second;
            QStringList diffs;
            if (s1.offset != s2.offset) diffs.append(QString("Offset: %1 -> %2").arg(s1.offset).arg(s2.offset));
            if (s1.access != s2.access) diffs.append(QString("Access: %1 -> %2").arg(s1.access).arg(s2.access));
            if (s1.reset != s2.reset) diffs.append(QString("Reset: %1 -> %2").arg(s1.reset).arg(s2.reset));

            // Compare fields
            for (const auto& [fName, fVal] : s2.fields) {
                auto fit1 = s1.fields.find(fName);
                if (fit1 == s1.fields.end()) {
                    diffs.append(QString("Added field '%1' (%2)").arg(fName).arg(fVal));
                } else if (fit1->second != fVal) {
                    diffs.append(QString("Modified field '%1': %2 -> %3").arg(fName).arg(fit1->second).arg(fVal));
                }
            }
            for (const auto& [fName, fVal] : s1.fields) {
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

    for (const auto& [key, val] : map1) {
        Q_UNUSED(val);
        if (map2.find(key) == map2.end()) {
            removedRegs.append(key);
        }
    }

    QString outputStr;
    if (format.toLower() == "markdown") {
        outputStr = QString("# Register Map Diff: `%1` vs `%2`\n\n").arg(file1, file2);
        outputStr += QString("### Summary\n- **Added Registers:** %1\n- **Removed Registers:** %2\n- **Modified Registers:** %3\n\n")
            .arg(addedRegs.size()).arg(removedRegs.size()).arg(modifiedRegs.size());

        if (!addedRegs.isEmpty()) {
            outputStr += "#### ➕ Added Registers\n";
            for (const QString &r : addedRegs) outputStr += QString("- `%1`\n").arg(r);
            outputStr += "\n";
        }
        if (!removedRegs.isEmpty()) {
            outputStr += "#### ➖ Removed Registers\n";
            for (const QString &r : removedRegs) outputStr += QString("- `%1`\n").arg(r);
            outputStr += "\n";
        }
        if (!modifiedRegs.isEmpty()) {
            outputStr += "#### 📝 Modified Registers\n";
            for (const QString &r : modifiedRegs) outputStr += QString("- `%1`\n").arg(r);
            outputStr += "\n";
        }
    } else {
        outputStr = QString("=== Register Map Diff: %1 vs %2 ===\n").arg(file1, file2);
        outputStr += QString("Added: %1 | Removed: %2 | Modified: %3\n\n").arg(addedRegs.size()).arg(removedRegs.size()).arg(modifiedRegs.size());
        for (const QString &r : addedRegs) outputStr += QString("+ ADDED:    %1\n").arg(r);
        for (const QString &r : removedRegs) outputStr += QString("- REMOVED:  %1\n").arg(r);
        for (const QString &r : modifiedRegs) outputStr += QString("~ MODIFIED: %1\n").arg(r);
    }

    if (!expOut.isEmpty()) {
        QFileInfo fi(expOut);
        QDir().mkpath(fi.absolutePath());
        QFile f(expOut);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream(&f) << outputStr;
            f.close();
            std::cout << "Diff saved to: " << expOut.toStdString() << std::endl;
        }
    } else {
        std::cout << outputStr.toStdString() << std::endl;
    }

    return true;
}

#include "RegMapWindow.moc"
