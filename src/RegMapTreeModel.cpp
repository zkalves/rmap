/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include <sstream>
#include <iomanip>
#include <algorithm>
#include <unordered_set>
#include <QCoreApplication>
#include "RegMapTreeModel.hpp"

// Reserved SystemVerilog and C keywords to prevent identifier collisions
static const std::unordered_set<std::string> kReservedKeywords = {
    // Verilog / SystemVerilog keywords (IEEE 1800)
    "accept_on", "alias", "always", "always_comb", "always_ff", "always_latch", "and", "assert",
    "assign", "assume", "automatic", "before", "begin", "bind", "bins", "binsof",
    "bit", "break", "buf", "bufif0", "bufif1", "byte", "case", "casex",
    "casez", "cell", "chandle", "checker", "class", "clocking", "cmos",
    "const", "constraint", "context", "continue", "cover", "covergroup", "coverpoint", "cross",
    "deassign", "default", "defparam", "design", "disable", "dist", "do", "edge",
    "else", "end", "endcase", "endchecker", "endclass", "endclocking", "endconfig", "endfunction",
    "endgenerate", "endgroup", "endinterface", "endmodule", "endpackage", "endprimitive", "endprogram", "endproperty",
    "endspecify", "endsequence", "endtable", "endtask", "enum", "event", "eventually", "expect",
    "export", "extends", "extern", "final", "first_match", "for", "force", "foreach",
    "forever", "fork", "forkjoin", "function", "generate", "genvar", "global", "highz0",
    "highz1", "if", "iff", "ifnone", "ignore_bins", "illegal_bins", "implements", "implies",
    "import", "incdir", "include", "initial", "inout", "input", "inside", "instance",
    "int", "integer", "interconnect", "interface", "intersect", "join", "join_any", "join_none",
    "large", "let", "liblist", "library", "local", "localparam", "logic", "longint",
    "macromodule", "matches", "medium", "modport", "module", "nand", "negedge", "nettype",
    "new", "nexttime", "nmos", "nor", "noshowcancelled", "not", "notif0", "notif1",
    "null", "or", "output", "package", "packed", "parameter", "pmos", "posedge",
    "primitive", "priority", "program", "property", "protected", "pull0", "pull1", "pulldown",
    "pullup", "pulsestyle_ondetect", "pulsestyle_onevent", "pure", "rand", "randc", "randcase", "randsequence",
    "rcmos", "real", "realtime", "ref", "reg", "reject_on", "release", "repeat",
    "restrict", "return", "rnmos", "rpmos", "rtran", "rtranif0", "rtranif1", "s_always",
    "s_eventually", "s_nexttime", "s_until", "s_until_with", "scalared", "sequence", "shortint", "shortreal",
    "showcancelled", "signed", "small", "soft", "solve", "specify", "specparam", "static",
    "string", "strong", "strong0", "strong1", "struct", "super", "supply0", "supply1",
    "sync_accept_on", "sync_reject_on", "table", "task", "this", "throughout", "time", "timeprecision",
    "timeunit", "tran", "tranif0", "tranif1", "tri", "tri0", "tri1", "triand",
    "trior", "trireg", "type", "typedef", "union", "unique", "unique0", "unsigned",
    "until", "until_with", "untyped", "use", "uwire", "var", "vectored", "virtual",
    "void", "wait", "wait_order", "wand", "weak", "weak0", "weak1", "while",
    "wildcard", "wire", "with", "within", "wor", "xnor", "xor",
    // C / C++ keywords
    "auto", "char", "double", "float", "goto", "inline", "long",
    "register", "short", "sizeof", "switch", "volatile", "asm", "catch",
    "delete", "explicit", "friend", "mutable", "namespace", "operator", "private",
    "protected", "public", "reinterpret_cast", "static_cast", "template", "throw", "try",
    "typename", "using"
};

// Helper to convert QVariant numbers (hex/dec/bin string) to uint64_t
static uint64_t parseNumericValue(const QVariant& var)
{
    QString str = var.toString().trimmed();
    if (str.startsWith("0x", Qt::CaseInsensitive)) {
        return str.mid(2).toULongLong(nullptr, 16);
    } else if (str.startsWith("0b", Qt::CaseInsensitive)) {
        return str.mid(2).toULongLong(nullptr, 2);
    }
    return str.toULongLong(nullptr, 10);
}

// Helper to convert uint64_t to 0x-prefixed zero-padded hex string (minimum 4 digits)
static std::string formatHex(uint64_t val, int minDigits = 4)
{
    std::stringstream ss;
    int digits = minDigits;
    if (val >= 0x100000000ULL) digits = (std::max)(minDigits, 16);
    else if (val >= 0x10000ULL) digits = (std::max)(minDigits, 8);
    ss << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(digits) << val;
    return ss.str();
}

// Helper to pad hex offset string written in hex (0x format)
static QString padHexOffset(const QString &input, int minDigits = 4)
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

RegMapTreeModel::RegMapTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    QStringList headerlist;
    QVariantMap data;
    headerlist << tr("Type")
               << tr("Offset/LSB")
               << tr("Size/Width")
               << tr("Name")
               << tr("Access Policy")
               << tr("HW Access")
               << tr("Reset Value")
               << tr("Is Rand")
               << tr("Volatile")
               << tr("Has Reset")
               << tr("Description");

    for (const QString &str : headerlist)
    {
        data[str] = str;
    }
    m_displayColumns = QVector<QString>::fromList(headerlist);
    m_rootItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::root, data);
}

RegMapTreeModel::~RegMapTreeModel()
{
    delete m_rootItem;
}

int RegMapTreeModel::columnCount(const QModelIndex &parent) const
{
    return m_displayColumns.size();
}

QVariant RegMapTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    RegMapTreeItem *item = static_cast<RegMapTreeItem*>(index.internalPointer());
    if (!item)
        return QVariant();

    if (role == Qt::DecorationRole)
    {
        if (index.column() == 0)
        {
            const QString iconPath = item->icon();
            if (!iconPath.isEmpty()) {
                QPixmap px(iconPath);
                if (!px.isNull()) {
                    return px.scaled(QSize(20, 20), Qt::KeepAspectRatio);
                }
            }
            return QVariant();
        }
    }

    if (role == Qt::BackgroundRole)
    {
        if (isIndexInvalid(index)) {
            return QBrush(QColor(255, 200, 200));
        }
        return QVariant();
    }

    if (role == Qt::ToolTipRole)
    {
        if (isIndexInvalid(index)) {
            return tr("Invalid value, address collision, or register width violation");
        }
        return QVariant();
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();


    if(index.column() == 0)
    {
        return item->kindString();
    }
    else
    {
        return item->data(m_displayColumns[index.column()]);
    }

}

Qt::ItemFlags RegMapTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags f = QAbstractItemModel::flags(index);
    if (index.column() != 0) {
        f |= Qt::ItemIsEditable;
    }
    return f;
}

QVariant RegMapTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 1) return tr("Offset");
        if (section == 2) return tr("Size");
        QString col = m_displayColumns.value(section);
        return QCoreApplication::translate("RegMapTreeModel", col.toUtf8().constData());
    }

    return QVariant();
}

void RegMapTreeModel::refreshHeaderData()
{
    if (!m_displayColumns.isEmpty()) {
        emit headerDataChanged(Qt::Horizontal, 0, m_displayColumns.size() - 1);
    }
}

QModelIndex RegMapTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    RegMapTreeItem *parentItem = parent.isValid()
        ? static_cast<RegMapTreeItem*>(parent.internalPointer())
        : m_rootItem;

    return createIndex(row, column, parentItem->child(row));
}

QModelIndex RegMapTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    RegMapTreeItem *childItem = static_cast<RegMapTreeItem*>(index.internalPointer());
    if (!childItem)
        return QModelIndex();

    RegMapTreeItem *parentItem = childItem->parentItem();

    if (!parentItem || parentItem == m_rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int RegMapTreeModel::rowCount(const QModelIndex &parent) const
{
    RegMapTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<RegMapTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

RegMapTreeItem* RegMapTreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid())
    {
        RegMapTreeItem* item = static_cast<RegMapTreeItem*>(index.internalPointer());
        if(item)
        {
            return item;
        }
    }
    return this->m_rootItem;
}

void RegMapTreeModel::setRootItem(RegMapTreeItem* item)
{
    beginResetModel();
    if (m_rootItem && m_rootItem != item) {
        delete m_rootItem;
    }
    this->m_rootItem = item;
    endResetModel();
}

bool RegMapTreeModel::insertRows(int position, int rows, RegMapTreeItem::e_rmmKind kind, QModelIndex parent)
{
    bool success;
    RegMapTreeItem* parentItem = getItem(parent);
    QVector<RegMapTreeItem::e_rmmKind> possible_children = parentItem->possibleChildren();

    if(possible_children.contains(kind))
    {
        this->beginInsertRows(parent, position, position + rows - 1);
        success = parentItem->insertChildren( kind,
                                              position,
                                              rows,
                                              m_displayColumns);
        this->endInsertRows();
        initRow(position,parent);
    }
    else
    {
        success = false;
    }
    return(success);
}

void RegMapTreeModel::initRow(int row, QModelIndex index)
{
    RegMapTreeItem* parentItem = getItem(index);
    RegMapTreeItem* childItem = parentItem->child(row);

    uint64_t nextOffsetLsb = 0;

    // Auto-calculate next LSB or Offset based on preceding sibling
    if (row > 0) {
        RegMapTreeItem* prevItem = parentItem->child(row - 1);
        uint64_t prevOffset = parseNumericValue(prevItem->data("Offset/LSB"));
        uint64_t prevSize   = parseNumericValue(prevItem->data("Size/Width"));

        if (childItem->kind() == RegMapTreeItem::e_rmmKind::reg) {
            // Register offsets increment by size in bytes (e.g. +4 bytes for 32-bit reg)
            uint64_t byteSize = (prevSize == 0) ? 4 : (prevSize / 8);
            nextOffsetLsb = prevOffset + byteSize;
        } else if (childItem->kind() == RegMapTreeItem::e_rmmKind::fld) {
            // Field LSBs increment by field width in bits
            nextOffsetLsb = prevOffset + prevSize;
        }
    }

    const auto kind = childItem->kind();
    const bool isHexOffset = (kind == RegMapTreeItem::e_rmmKind::reg || kind == RegMapTreeItem::e_rmmKind::mem || kind == RegMapTreeItem::e_rmmKind::blk || kind == RegMapTreeItem::e_rmmKind::map);
    QString offsetVal;
    if (isHexOffset) {
        offsetVal = QString::fromStdString(formatHex(nextOffsetLsb));
    } else {
        offsetVal = QString::number(nextOffsetLsb);
    }

    QString sizeVal;
    if (kind == RegMapTreeItem::e_rmmKind::mem) {
        sizeVal = QStringLiteral("1024");
    } else if (kind == RegMapTreeItem::e_rmmKind::reg) {
        sizeVal = QStringLiteral("32");
    } else if (kind == RegMapTreeItem::e_rmmKind::map) {
        sizeVal = QStringLiteral("4");
    } else {
        sizeVal = QStringLiteral("1");
    }

    const bool isStructuralNode = (kind == RegMapTreeItem::e_rmmKind::map || kind == RegMapTreeItem::e_rmmKind::blk || kind == RegMapTreeItem::e_rmmKind::mem);

    for (int column = 1; column < columnCount(index); column++) {
        QModelIndex child = this->index(row, column, index);
        QString colName = m_displayColumns[column];

        if (colName == "Offset/LSB") {
            this->setData(child, offsetVal, Qt::EditRole);
        } else if (colName == "Size/Width") {
            this->setData(child, sizeVal, Qt::EditRole);
        } else if (colName == "Access Policy") {
            this->setData(child, "RW", Qt::EditRole);
        } else if (colName == "HW Access") {
            if (kind == RegMapTreeItem::e_rmmKind::map || kind == RegMapTreeItem::e_rmmKind::blk) {
                this->setData(child, "NA", Qt::EditRole);
            } else if (kind == RegMapTreeItem::e_rmmKind::mem) {
                this->setData(child, "RW", Qt::EditRole);
            } else {
                this->setData(child, "RO", Qt::EditRole);
            }
        } else if (colName == "Reset Value") {
            if (isStructuralNode) {
                this->setData(child, "", Qt::EditRole);
            } else {
                this->setData(child, "0x0", Qt::EditRole);
            }
        } else if (colName == "Is Rand" || colName == "Has Reset") {
            if (isStructuralNode) {
                this->setData(child, "false", Qt::EditRole);
            } else {
                this->setData(child, "true", Qt::EditRole);
            }
        } else if (colName == "Volatile") {
            this->setData(child, "false", Qt::EditRole);
        } else {
            this->setData(child, "", Qt::EditRole);
        }
    }
}

bool RegMapTreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    bool success;
    RegMapTreeItem* parentItem = getItem(parent);
    this->beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    this->endRemoveRows();
    return(success);
}

bool RegMapTreeModel::clear(void)
{
    bool success;
    this->beginResetModel();
    success = m_rootItem->removeChildren(0,m_rootItem->childCount());
    this->endResetModel();
    return(success);
}

bool RegMapTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole || !index.isValid()) return false;

    RegMapTreeItem* item = getItem(index);
    if (!item) return false;

    QVariant finalValue = value;
    if (index.column() == 1) { // Offset/LSB
        if (item->kind() != RegMapTreeItem::e_rmmKind::fld) {
            finalValue = padHexOffset(value.toString());
        }
    }

    bool set_data_status = item->setData(m_displayColumns[index.column()], finalValue);

    if (set_data_status) {
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    }
    return set_data_status;
}

bool RegMapTreeModel::isIndexInvalid(const QModelIndex &index) const
{
    if (!index.isValid()) return false;
    RegMapTreeItem* item = static_cast<RegMapTreeItem*>(index.internalPointer());
    if (!item) return false;
    return m_invalidCells.contains(std::make_pair(item, index.column()));
}

void RegMapTreeModel::recursiveCheckData(RegMapTreeItem *node, uint32_t regWidth, QStringList &errors)
{
    if (!node) return;

    const auto nodeKind = node->kind();
    const QString nodeName = node->data("Name").toString();

    // Validate Name is not empty for meaningful nodes
    if (nodeKind != RegMapTreeItem::e_rmmKind::root) {
        QString trimmedName = nodeName.trimmed();
        if (trimmedName.isEmpty()) {
            m_invalidCells.insert(std::make_pair(node, 3)); // Name column
            errors.append(tr("Found %1 node with an empty Name").arg(node->kindString().toUpper()));
        } else {
            std::string lowerName = trimmedName.toLower().toStdString();
            if (kReservedKeywords.find(lowerName) != kReservedKeywords.end()) {
                m_invalidCells.insert(std::make_pair(node, 3)); // Name column
                errors.append(tr("%1 '%2' uses reserved keyword '%3' as identifier")
                    .arg(node->kindString().toUpper(), nodeName, nodeName));
            }
        }
    }

    // 1. Validate Register & Memory Overlaps inside a Block
    if (nodeKind == RegMapTreeItem::e_rmmKind::blk)
    {
        struct RegionInfo { RegMapTreeItem* item; uint64_t start; uint64_t end; QString name; QString type; };
        std::vector<RegionInfo> regions;

        for (RegMapTreeItem* child : node->getChildItems()) {
            const auto childKind = child->kind();
            if (childKind == RegMapTreeItem::e_rmmKind::reg) {
                uint64_t offset = parseNumericValue(child->data("Offset/LSB"));
                uint64_t size_bytes = (regWidth > 0) ? (regWidth / 8) : 4;
                regions.push_back({child, offset, offset + size_bytes - 1, child->data("Name").toString(), "Register"});
            } else if (childKind == RegMapTreeItem::e_rmmKind::mem) {
                uint64_t offset = parseNumericValue(child->data("Offset/LSB"));
                uint64_t size_bytes = parseNumericValue(child->data("Size/Width"));
                if (size_bytes == 0) size_bytes = 4;
                regions.push_back({child, offset, offset + size_bytes - 1, child->data("Name").toString(), "Memory"});
            }
        }

        for (size_t i = 0; i < regions.size(); ++i) {
            for (size_t j = i + 1; j < regions.size(); ++j) {
                if (!(regions[i].end < regions[j].start || regions[i].start > regions[j].end)) {
                    m_invalidCells.insert(std::make_pair(regions[i].item, 1)); // Highlight Offset column
                    m_invalidCells.insert(std::make_pair(regions[j].item, 1));

                    errors.append(tr("%1 '%2' (offset 0x%3) overlaps with %4 '%5' (offset 0x%6) in Block '%7'")
                        .arg(regions[i].type, regions[i].name,
                             QString::number(regions[i].start, 16).toUpper(),
                             regions[j].type, regions[j].name,
                             QString::number(regions[j].start, 16).toUpper(),
                             nodeName));
                }
            }
        }
    }
    // 2. Validate Field Overlaps & Widths inside a Register
    if (nodeKind == RegMapTreeItem::e_rmmKind::reg)
    {
        uint64_t reg_width = (regWidth == 0) ? 32 : regWidth;

        struct FieldInfo { RegMapTreeItem* item; uint64_t lsb; uint64_t msb; uint64_t width; QString name; };
        std::vector<FieldInfo> fields;

        for (RegMapTreeItem* child : node->getChildItems()) {
            if (child->kind() == RegMapTreeItem::e_rmmKind::fld) {
                uint64_t lsb = parseNumericValue(child->data("Offset/LSB"));
                uint64_t width = parseNumericValue(child->data("Size/Width"));
                uint64_t msb = (width > 0) ? (lsb + width - 1) : lsb;
                QString fldName = child->data("Name").toString();

                // Check field with zero width
                if (width == 0) {
                    m_invalidCells.insert(std::make_pair(child, 2)); // Size/Width column
                    errors.append(tr("Field '%1' in Register '%2' has invalid bit width 0")
                        .arg(fldName, nodeName));
                }

                // Check field exceeding register width
                if (msb >= reg_width) {
                    m_invalidCells.insert(std::make_pair(child, 1)); // Highlight LSB
                    m_invalidCells.insert(std::make_pair(child, 2)); // Highlight Width
                    errors.append(tr("Field '%1' in Register '%2' exceeds register width (%3 bits): bit [%4:%5]")
                        .arg(fldName, nodeName, QString::number(reg_width), QString::number(msb), QString::number(lsb)));
                }

                // Check field reset value overflow
                uint64_t reset_val = parseNumericValue(child->data("Reset Value"));
                if (width > 0 && width < 64) {
                    uint64_t max_val = (1ULL << width) - 1ULL;
                    if (reset_val > max_val) {
                        m_invalidCells.insert(std::make_pair(child, 6)); // Reset Value column
                        errors.append(tr("Field '%1' in Register '%2' reset value 0x%3 overflows bit width %4 (max allowed is 0x%5)")
                            .arg(fldName, nodeName, QString::number(reset_val, 16).toUpper(), QString::number(width), QString::number(max_val, 16).toUpper()));
                    }
                }

                // Check contradictory access policy (SW=WO and HW=WO)
                QString swAccess = child->data("Access Policy").toString().toUpper().trimmed();
                QString hwAccess = child->data("HW Access").toString().toUpper().trimmed();
                if (swAccess == "WO" && hwAccess == "WO") {
                    m_invalidCells.insert(std::make_pair(child, 4)); // Access Policy
                    m_invalidCells.insert(std::make_pair(child, 5)); // HW Access
                    errors.append(tr("Field '%1' in Register '%2' has contradictory access policy: both SW and HW are Write-Only")
                        .arg(fldName, nodeName));
                }

                // Check reset consistency
                bool hasReset = (child->data("Has Reset").toString().toLower() == "true");
                if (!hasReset && reset_val != 0) {
                    m_invalidCells.insert(std::make_pair(child, 6)); // Reset Value
                    m_invalidCells.insert(std::make_pair(child, 9)); // Has Reset
                    errors.append(tr("Field '%1' in Register '%2' specifies non-zero reset value (0x%3) but 'Has Reset' is disabled")
                        .arg(fldName, nodeName, QString::number(reset_val, 16).toUpper()));
                }

                if (width > 0) {
                    fields.push_back({child, lsb, msb, width, fldName});
                }
            }
        }

        for (size_t i = 0; i < fields.size(); ++i) {
            for (size_t j = i + 1; j < fields.size(); ++j) {
                if (!(fields[i].msb < fields[j].lsb || fields[i].lsb > fields[j].msb)) {
                    m_invalidCells.insert(std::make_pair(fields[i].item, 1));
                    m_invalidCells.insert(std::make_pair(fields[j].item, 1));
                    errors.append(tr("Field '%1' [bits %2:%3] overlaps with '%4' [bits %5:%6] in Register '%7'")
                        .arg(fields[i].name, QString::number(fields[i].msb), QString::number(fields[i].lsb),
                             fields[j].name, QString::number(fields[j].msb), QString::number(fields[j].lsb),
                             nodeName));
                }
            }
        }

        if (node->getChildItems().isEmpty()) {
            uint64_t reg_reset = parseNumericValue(node->data("Reset Value"));
            if (reg_width < 64 && reg_reset > ((1ULL << reg_width) - 1ULL)) {
                m_invalidCells.insert(std::make_pair(node, 6));
                errors.append(tr("Register '%1' reset value 0x%2 overflows register width (%3 bits)")
                    .arg(nodeName, QString::number(reg_reset, 16).toUpper(), QString::number(reg_width)));
            }
        }
    }

    for (RegMapTreeItem* child : node->getChildItems()) {
        recursiveCheckData(child, regWidth, errors);
    }
}

QStringList RegMapTreeModel::checkData(uint32_t regWidth) noexcept
{
    m_invalidCells.clear();
    QStringList errors;
    if (m_rootItem) {
        recursiveCheckData(m_rootItem, regWidth, errors);
    }
    return errors;
}

json RegMapTreeModel::recursiveExtractJsonData(RegMapTreeItem *node, uint32_t regWidth)
{
    json item_json;

    if (!node) {
        return item_json;
    }

    std::string kind = node->kindString().toStdString();
    item_json["kind"] = kind;

    std::string name = node->data("Name").toString().toStdString();
    std::string access = node->data("Access Policy").toString().toStdString();
    std::string sw_access = access.empty() ? "RW" : access;
    std::string hw_access_str = node->data("HW Access").toString().toStdString();
    std::string hw_access = hw_access_str.empty() ? "RO" : hw_access_str;
    std::string desc = node->data("Description").toString().toStdString();

    uint64_t offset_lsb = parseNumericValue(node->data("Offset/LSB"));
    uint64_t size_width = (kind == "reg") ? regWidth : parseNumericValue(node->data("Size/Width"));
    if (kind == "mem" && size_width == 0) size_width = 1024;
    uint64_t reset_val  = parseNumericValue(node->data("Reset Value"));

    bool is_rand     = (node->data("Is Rand").toString().toLower() == "true");
    bool is_volatile = (node->data("Volatile").toString().toLower() == "true");
    bool has_reset  = (node->data("Has Reset").toString().toLower() == "true");

    item_json["name"]        = name;
    item_json["offset_lsb"]  = offset_lsb;
    item_json["offset_hex"]  = formatHex(offset_lsb);
    item_json["size_width"]  = size_width;
    item_json["access"]      = sw_access;
    item_json["sw_access"]   = sw_access;
    item_json["hw_access"]   = hw_access;
    item_json["reset_val"]   = reset_val;
    item_json["reset_hex"]   = formatHex(reset_val);
    item_json["is_rand"]     = is_rand;
    item_json["volatile"]    = is_volatile;
    item_json["has_reset"]   = has_reset;
    item_json["description"] = desc;

    // Node-specific properties for UVM RAL
    if (kind == "reg") {
        QString regHdl = node->data("HDL Path").toString().trimmed();
        if (regHdl.isEmpty()) {
            QString lowerRegName = QString::fromStdString(name).toLower();
            regHdl = QString("reg_%1_q").arg(lowerRegName);
        }
        item_json["hdl_path"] = regHdl.toStdString();

        bool no_reg_test = (node->data("NO_REG_TEST").toString().toLower() == "true" || node->data("No Reg Test").toString().toLower() == "true");
        bool no_reset_test = (no_reg_test || node->data("NO_REG_HW_RESET_TEST").toString().toLower() == "true" || node->data("No Reset Test").toString().toLower() == "true");
        bool no_bit_bash_test = (no_reg_test || node->data("NO_REG_BIT_BASH_TEST").toString().toLower() == "true" || node->data("No Bit Bash Test").toString().toLower() == "true");
        bool no_access_test = (no_reg_test || node->data("NO_REG_ACCESS_TEST").toString().toLower() == "true" || node->data("No Access Test").toString().toLower() == "true");
        item_json["no_reg_test"] = no_reg_test;
        item_json["no_reset_test"] = no_reset_test;
        item_json["no_bit_bash_test"] = no_bit_bash_test;
        item_json["no_access_test"] = no_access_test;

        QString regType = node->data("Reg Type").toString().trimmed().toLower();
        bool is_fifo = (regType == "fifo" || node->data("Is FIFO").toString().toLower() == "true");
        uint64_t fifo_depth = parseNumericValue(node->data("FIFO Depth"));
        if (fifo_depth == 0) fifo_depth = 8;
        item_json["is_fifo"] = is_fifo;
        item_json["fifo_depth"] = fifo_depth;

        bool is_indirect = (regType == "indirect" || node->data("Is Indirect").toString().toLower() == "true");
        QString idxReg = node->data("Index Reg").toString().trimmed();
        item_json["is_indirect"] = is_indirect;
        item_json["index_reg"] = idxReg.toStdString();

        bool has_callbacks = (node->data("Has Callbacks").toString().toLower() == "true" || node->data("Callbacks").toString().toLower() == "true");
        item_json["has_callbacks"] = has_callbacks;
    } else if (kind == "mem") {
        uint64_t word_width = parseNumericValue(node->data("Word Width"));
        if (word_width == 0) word_width = regWidth;
        uint64_t depth = parseNumericValue(node->data("Depth"));
        if (depth == 0) {
            uint64_t wordBytes = (word_width >= 8 ? word_width : 32) / 8;
            depth = (size_width >= wordBytes && wordBytes > 0) ? (size_width / wordBytes) : (size_width > 0 ? size_width : 1024);
        }
        item_json["depth"] = depth;
        item_json["word_width"] = word_width;
        item_json["hdl_path"] = node->data("HDL Path").toString().trimmed().toStdString();

        bool no_mem_test = (node->data("NO_MEM_TEST").toString().toLower() == "true" || node->data("No Mem Test").toString().toLower() == "true");
        bool no_walk_test = (no_mem_test || node->data("NO_MEM_WALK_TEST").toString().toLower() == "true" || node->data("No Walk Test").toString().toLower() == "true");
        bool no_mem_access_test = (no_mem_test || node->data("NO_MEM_ACCESS_TEST").toString().toLower() == "true" || node->data("No Access Test").toString().toLower() == "true");
        item_json["no_mem_test"] = no_mem_test;
        item_json["no_walk_test"] = no_walk_test;
        item_json["no_access_test"] = no_mem_access_test;
    } else if (kind == "fld") {
        bool ind_acc = true;
        if (node->data("Individually Accessible").isValid()) {
            QString s = node->data("Individually Accessible").toString().toLower();
            ind_acc = (s == "true" || s == "1");
        }
        item_json["individually_accessible"] = ind_acc ? 1 : 0;
    } else if (kind == "map") {
        uint64_t n_bytes = size_width > 0 ? size_width : ((regWidth >= 8 ? regWidth : 32) / 8);
        QString endianness = node->data("Endianness").toString().trimmed();
        if (endianness.isEmpty()) endianness = QStringLiteral("UVM_LITTLE_ENDIAN");
        bool byte_addressing = (node->data("Byte Addressing").toString().toLower() != "false" && node->data("Byte Addressing").toString() != "0");
        item_json["base_addr"] = offset_lsb;
        item_json["base_hex"] = formatHex(offset_lsb);
        item_json["n_bytes"] = n_bytes;
        item_json["endianness"] = endianness.toStdString();
        item_json["byte_addressing"] = byte_addressing ? 1 : 0;
    } else if (kind == "blk") {
        QString blkHdl = node->data("HDL Path").toString().trimmed();
        item_json["hdl_path"] = blkHdl.isEmpty() ? "DUT" : blkHdl.toStdString();
    }

    // Structured arrays for native Inja loops
    json blocks    = json::array();
    json registers = json::array();
    json fields    = json::array();
    json memories  = json::array();
    json maps      = json::array();

    for (RegMapTreeItem* child : node->getChildItems())
    {
        if (!child) continue;
        json child_json = recursiveExtractJsonData(child, regWidth);
        switch (child->kind()) {
        case RegMapTreeItem::e_rmmKind::blk:
            blocks.push_back(child_json);
            break;
        case RegMapTreeItem::e_rmmKind::reg:
            registers.push_back(child_json);
            break;
        case RegMapTreeItem::e_rmmKind::fld:
            fields.push_back(child_json);
            break;
        case RegMapTreeItem::e_rmmKind::mem:
            memories.push_back(child_json);
            break;
        case RegMapTreeItem::e_rmmKind::map:
            maps.push_back(child_json);
            break;
        default:
            break;
        }
    }

    if (kind == "blk" || kind == "root") {
        if (maps.empty()) {
            json def_map;
            def_map["name"] = "default_map";
            def_map["base_addr"] = 0;
            def_map["base_hex"] = "0";
            def_map["n_bytes"] = (regWidth >= 8 ? regWidth : 32) / 8;
            def_map["endianness"] = "UVM_LITTLE_ENDIAN";
            def_map["byte_addressing"] = 1;
            maps.push_back(def_map);
        }
        std::sort(maps.begin(), maps.end(), [](const json &a, const json &b) {
            return a.value("base_addr", 0ULL) < b.value("base_addr", 0ULL);
        });
        item_json["maps"] = maps;
    } else if (!maps.empty()) {
        item_json["maps"] = maps;
    }

    if (!blocks.empty()) {
        std::sort(blocks.begin(), blocks.end(), [](const json &a, const json &b) {
            return a.value("offset_lsb", 0ULL) < b.value("offset_lsb", 0ULL);
        });
        item_json["blocks"] = blocks;
    }
    if (!registers.empty()) {
        std::sort(registers.begin(), registers.end(), [](const json &a, const json &b) {
            return a.value("offset_lsb", 0ULL) < b.value("offset_lsb", 0ULL);
        });

        // Compute contiguous offset gaps (memory holes) for struct padding
        uint64_t regBytes = (regWidth >= 8 ? regWidth : 32) / 8;
        uint64_t currentOffset = 0;
        for (auto &r : registers) {
            uint64_t regOffset = r.value("offset_lsb", 0ULL);
            uint64_t padBytes = (regOffset > currentOffset) ? (regOffset - currentOffset) : 0;
            uint64_t padWords = padBytes / regBytes;
            r["pad_bytes_before"] = padBytes;
            r["pad_words_before"] = padWords;
            currentOffset = regOffset + regBytes;
        }

        item_json["registers"] = registers;
    }
    if (!fields.empty()) {
        std::sort(fields.begin(), fields.end(), [](const json &a, const json &b) {
            return a.value("offset_lsb", 0ULL) < b.value("offset_lsb", 0ULL);
        });
        item_json["fields"] = fields;
    }
    if (!memories.empty()) {
        std::sort(memories.begin(), memories.end(), [](const json &a, const json &b) {
            return a.value("offset_lsb", 0ULL) < b.value("offset_lsb", 0ULL);
        });
        item_json["memories"] = memories;
    }

    return item_json;
}

uint32_t RegMapTreeModel::calculateCrc32(const uint8_t *data, size_t length, uint32_t previousCrc32)
{
    static const uint32_t crcTable[256] = {
        0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
        0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
        0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
        0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
        0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
        0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
        0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
        0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
        0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
        0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
        0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
        0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
        0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
        0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
        0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
        0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
        0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
        0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
        0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
        0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
        0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
        0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
        0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
        0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
        0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
        0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
        0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
        0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
        0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
        0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
        0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
        0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
    };

    uint32_t crc = ~previousCrc32;
    for (size_t i = 0; i < length; ++i) {
        crc = crcTable[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    }
    return ~crc;
}

uint32_t RegMapTreeModel::computeBlockCrc32(const json &blkJson)
{
    std::string repr;
    repr += "blk:" + blkJson.value("name", "") + ":" + std::to_string(blkJson.value("offset_lsb", 0ULL)) + ";";
    auto regIt = blkJson.find("registers");
    if (regIt != blkJson.end() && regIt->is_array()) {
        for (const auto &reg : *regIt) {
            repr += "reg:" + reg.value("name", "") + ":" + std::to_string(reg.value("offset_lsb", 0ULL))
                 + ":" + std::to_string(reg.value("size_width", 0ULL)) + ":" + reg.value("access", "")
                 + ":" + std::to_string(reg.value("reset_val", 0ULL)) + ";";
            auto fldIt = reg.find("fields");
            if (fldIt != reg.end() && fldIt->is_array()) {
                for (const auto &fld : *fldIt) {
                    repr += "fld:" + fld.value("name", "") + ":" + std::to_string(fld.value("offset_lsb", 0ULL))
                         + ":" + std::to_string(fld.value("size_width", 0ULL)) + ":" + fld.value("access", "")
                         + ":" + std::to_string(fld.value("reset_val", 0ULL)) + ";";
                }
            }
        }
    }
    return calculateCrc32(reinterpret_cast<const uint8_t*>(repr.data()), repr.size());
}

uint32_t RegMapTreeModel::computeTreeCrc32(const json &rootJson)
{
    std::string repr;
    auto blkIt = rootJson.find("blocks");
    if (blkIt != rootJson.end() && blkIt->is_array()) {
        for (const auto &blk : *blkIt) {
            repr += "b:" + blk.value("name", "") + ":" + std::to_string(blk.value("crc32", 0U)) + ";";
        }
    }
    auto regIt = rootJson.find("registers");
    if (regIt != rootJson.end() && regIt->is_array()) {
        for (const auto &reg : *regIt) {
            repr += "r:" + reg.value("name", "") + ":" + std::to_string(reg.value("offset_lsb", 0ULL))
                 + ":" + std::to_string(reg.value("size_width", 0ULL)) + ":" + reg.value("access", "")
                 + ":" + std::to_string(reg.value("reset_val", 0ULL)) + ";";
        }
    }
    return calculateCrc32(reinterpret_cast<const uint8_t*>(repr.data()), repr.size());
}

json RegMapTreeModel::extractJsonData(uint32_t regWidth) noexcept
{
    json root_json = recursiveExtractJsonData(m_rootItem, regWidth);
    root_json["reg_width"]       = regWidth;
    root_json["reg_width_bytes"] = regWidth / 8;

    if (root_json.value("name", "").empty()) {
        root_json["name"] = "regmap";
    }
    if (root_json.value("description", "").empty()) {
        root_json["description"] = "Hardware Register Map Specification";
    }

    auto blkIt = root_json.find("blocks");
    if (blkIt != root_json.end() && blkIt->is_array()) {
        for (auto &blk : *blkIt) {
            uint32_t bCrc = computeBlockCrc32(blk);
            blk["crc32"] = bCrc;
            blk["crc32_hex"] = ("0x" + QString("%1").arg(bCrc, 8, 16, QChar('0')).toUpper()).toStdString();
        }
    }

    uint32_t treeCrc = computeTreeCrc32(root_json);
    root_json["regmap_crc32"] = treeCrc;
    root_json["regmap_crc32_hex"] = ("0x" + QString("%1").arg(treeCrc, 8, 16, QChar('0')).toUpper()).toStdString();

    return root_json;
}
