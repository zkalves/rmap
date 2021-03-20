#include <sstream>
#include <iomanip>
#include <algorithm>
#include "RegMapTreeModel.hpp"

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
        return (Qt::NoItemFlags);
    }

    RegMapTreeItem *item = getItem(index);
    return (index.column() != 0 && item && item->kind() != RegMapTreeItem::e_rmmKind::root)
        ? (Qt::ItemIsEditable | QAbstractItemModel::flags(index))
        : (QAbstractItemModel::flags(index));
}

QVariant RegMapTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 1) return tr("Offset");
        if (section == 2) return tr("Size");
        return m_displayColumns.value(section);
    }

    return QVariant();
}

QModelIndex RegMapTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    RegMapTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<RegMapTreeItem*>(parent.internalPointer());

    if (!parentItem)
        return QModelIndex();

    RegMapTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
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

        if (childItem->kindString() == "reg") {
            // Register offsets increment by size in bytes (e.g. +4 bytes for 32-bit reg)
            uint64_t byteSize = (prevSize == 0) ? 4 : (prevSize / 8);
            nextOffsetLsb = prevOffset + byteSize;
        } else if (childItem->kindString() == "fld") {
            // Field LSBs increment by field width in bits
            nextOffsetLsb = prevOffset + prevSize;
        }
    }

    for (int column = 1; column < columnCount(index); column++) {
        QModelIndex child = this->index(row, column, index);
        QString colName = m_displayColumns[column];

        if (colName == "Offset/LSB") {
            this->setData(child, (childItem->kindString() == "reg") ?
                          QString::fromStdString(formatHex(nextOffsetLsb)) :
                          QString::number(nextOffsetLsb), Qt::EditRole);
        } else if (colName == "Size/Width") {
            this->setData(child, (childItem->kindString() == "reg") ? "32" : "1", Qt::EditRole);
        } else if (colName == "Access Policy") {
            this->setData(child, "RW", Qt::EditRole);
        } else if (colName == "HW Access") {
            this->setData(child, "RO", Qt::EditRole);
        } else if (colName == "Reset Value") {
            this->setData(child, "0x0", Qt::EditRole);
        } else if (colName == "Is Rand" || colName == "Has Reset") {
            this->setData(child, "true", Qt::EditRole);
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
    if (role != Qt::EditRole) return false;

    RegMapTreeItem* item = getItem(index);
    if (!item) return false;

    QVariant finalValue = value;
    if (index.column() == 1) { // Offset/LSB
        QString kind = item->kindString();
        if (kind == "reg" || kind == "blk" || kind == "mem" || kind == "map") {
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

    std::string kind = node->kindString().toStdString();
    QString nodeName = node->data("Name").toString();

    // Validate Name is not empty for meaningful nodes
    if (kind != "root" && nodeName.trimmed().isEmpty()) {
        m_invalidCells.insert(std::make_pair(node, 3)); // Name column
        errors.append(tr("Found %1 node with an empty Name").arg(QString::fromStdString(kind).toUpper()));
    }

    // 1. Validate Register Overlaps inside a Block
    if (kind == "blk")
    {
        struct RegInfo { RegMapTreeItem* item; uint64_t start; uint64_t end; QString name; };
        std::vector<RegInfo> regs;

        for (RegMapTreeItem* child : node->getChildItems()) {
            if (child->kindString().toStdString() == "reg") {
                uint64_t offset = parseNumericValue(child->data("Offset/LSB"));
                uint64_t size_bytes = (regWidth > 0) ? (regWidth / 8) : 4;
                regs.push_back({child, offset, offset + size_bytes - 1, child->data("Name").toString()});
            }
        }

        for (size_t i = 0; i < regs.size(); ++i) {
            for (size_t j = i + 1; j < regs.size(); ++j) {
                if (!(regs[i].end < regs[j].start || regs[i].start > regs[j].end)) {
                    m_invalidCells.insert(std::make_pair(regs[i].item, 1)); // Highlight Offset column
                    m_invalidCells.insert(std::make_pair(regs[j].item, 1));

                    errors.append(tr("Register '%1' (offset 0x%2) overlaps with '%3' (offset 0x%4) in Block '%5'")
                        .arg(regs[i].name,
                             QString::number(regs[i].start, 16).toUpper(),
                             regs[j].name,
                             QString::number(regs[j].start, 16).toUpper(),
                             nodeName));
                }
            }
        }
    }
    // 2. Validate Field Overlaps & Widths inside a Register
    if (kind == "reg")
    {
        uint64_t reg_width = (regWidth == 0) ? 32 : regWidth;

        struct FieldInfo { RegMapTreeItem* item; uint64_t lsb; uint64_t msb; uint64_t width; QString name; };
        std::vector<FieldInfo> fields;

        for (RegMapTreeItem* child : node->getChildItems()) {
            if (child->kindString().toStdString() == "fld") {
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
                fields.push_back({child, lsb, msb, width, fldName});
            }
        }

        for (size_t i = 0; i < fields.size(); ++i) {
            for (size_t j = i + 1; j < fields.size(); ++j) {
                if (fields[i].width > 0 && fields[j].width > 0) {
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
        }
    }

    for (RegMapTreeItem* child : node->getChildItems()) {
        recursiveCheckData(child, regWidth, errors);
    }
}

QStringList RegMapTreeModel::checkData(uint32_t regWidth)
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

    // Structured arrays for native Inja loops
    json blocks    = json::array();
    json registers = json::array();
    json fields    = json::array();
    json memories  = json::array();

    for (RegMapTreeItem* child : node->getChildItems())
    {
        if (!child) continue;
        json child_json = recursiveExtractJsonData(child, regWidth);
        std::string kind = child->kindString().toStdString();

        if (kind == "blk") {
            blocks.push_back(child_json);
        } else if (kind == "reg") {
            registers.push_back(child_json);
        } else if (kind == "fld") {
            fields.push_back(child_json);
        } else if (kind == "mem") {
            memories.push_back(child_json);
        }
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
        item_json["registers"] = registers;
    }
    if (!fields.empty()) {
        std::sort(fields.begin(), fields.end(), [](const json &a, const json &b) {
            return a.value("offset_lsb", 0ULL) < b.value("offset_lsb", 0ULL);
        });
        item_json["fields"] = fields;
    }
    if (!memories.empty())  item_json["memories"] = memories;

    return item_json;

}

json RegMapTreeModel::extractJsonData(uint32_t regWidth)
{
    json root_json = recursiveExtractJsonData(m_rootItem, regWidth);
    root_json["reg_width"]       = regWidth;
    root_json["reg_width_bytes"] = regWidth / 8;
    return root_json;
}
