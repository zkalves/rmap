/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "BlockMemoryMapWidget.hpp"
#include "ThemeManager.hpp"
#include "AppSettings.hpp"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QToolTip>
#include <QFontMetrics>
#include <algorithm>

static uint64_t parseNumber(const QVariant &val)
{
    QString s = val.toString().trimmed();
    if (s.isEmpty()) return 0;
    bool ok = false;
    if (s.startsWith("0x", Qt::CaseInsensitive)) {
        return s.mid(2).toULongLong(&ok, 16);
    } else if (s.startsWith("0b", Qt::CaseInsensitive)) {
        return s.mid(2).toULongLong(&ok, 2);
    }
    uint64_t dec = s.toULongLong(&ok, 10);
    return ok ? dec : 0;
}

BlockMemoryMapWidget::BlockMemoryMapWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    setMinimumWidth(260);
    m_colorBlindMode = AppSettings::instance().colorBlindMode() ? AppSettings::instance().colorBlindType() : ColorBlindMode::None;

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this]() {
        update();
    });
}

void BlockMemoryMapWidget::setColorBlindMode(bool enabled)
{
    ColorBlindMode mode = enabled ? AppSettings::instance().colorBlindType() : ColorBlindMode::None;
    if (mode == ColorBlindMode::None && enabled) {
        mode = ColorBlindMode::Universal;
    }
    setColorBlindMode(mode);
}

void BlockMemoryMapWidget::setColorBlindMode(ColorBlindMode mode)
{
    if (m_colorBlindMode != mode) {
        m_colorBlindMode = mode;
        update();
    }
}

void BlockMemoryMapWidget::setBlock(RegMapTreeItem *blkItem, uint32_t regWidthBits)
{
    m_blkItem = blkItem;
    m_globalRegWidthBits = (regWidthBits == 0) ? 32 : regWidthBits;
    m_hoveredIndex = -1;
    m_selectedIndex = -1;
    computeBlocks();
    updateGeometry();
    update();
}

void BlockMemoryMapWidget::clear()
{
    m_blkItem = nullptr;
    m_blocks.clear();
    m_blockRects.clear();
    m_hoveredIndex = -1;
    m_selectedIndex = -1;
    m_totalHeight = 100;
    updateGeometry();
    update();
}

void BlockMemoryMapWidget::refresh()
{
    if (!m_blkItem) return;
    int savedRow = -1;
    if (m_selectedIndex >= 0 && m_selectedIndex < m_blocks.size()) {
        savedRow = m_blocks[m_selectedIndex].childRow;
    }
    computeBlocks();
    if (savedRow >= 0) {
        setSelectedRegister(savedRow);
    } else {
        update();
    }
}

void BlockMemoryMapWidget::setSelectedRegister(int childRow)
{
    m_selectedIndex = -1;
    for (int i = 0; i < m_blocks.size(); ++i) {
        if (!m_blocks[i].isReserved && m_blocks[i].childRow == childRow) {
            m_selectedIndex = i;
            break;
        }
    }
    update();
}

void BlockMemoryMapWidget::computeBlocks()
{
    m_blocks.clear();
    m_blockRects.clear();

    if (!m_blkItem) {
        m_totalHeight = 100;
        return;
    }

    struct RawReg {
        RegMapTreeItem *item = nullptr;
        uint64_t offset = 0;
        uint64_t byteWidth = 4;
        QString name;
        QString access;
        QString desc;
        int childRow = -1;
    };

    QVector<RawReg> regList;
    const auto &children = m_blkItem->getChildItems();
    uint64_t defaultByteWidth = (m_globalRegWidthBits > 0) ? (m_globalRegWidthBits / 8) : 4;
    if (defaultByteWidth == 0) defaultByteWidth = 4;

    for (int row = 0; row < children.size(); ++row) {
        RegMapTreeItem *child = children[row];
        if (child && child->kindString() == "reg") {
            uint64_t offset = parseNumber(child->data("Offset/LSB"));
            uint64_t customWidth = parseNumber(child->data("Size/Width"));
            uint64_t byteWidth = defaultByteWidth;
            if (customWidth > 0) {
                byteWidth = (customWidth >= 8) ? (customWidth / 8) : customWidth;
            }

            QString name = child->data("Name").toString();
            QString access = child->data("Access Policy").toString().trimmed().toUpper();
            QString desc = child->data("Description").toString();

            // If access policy is not set on the register, infer from its bitfields
            if (access.isEmpty()) {
                bool hasRW = false;
                bool hasRO = false;
                bool hasWO = false;
                for (RegMapTreeItem *fld : child->getChildItems()) {
                    if (fld && fld->kindString() == "fld") {
                        QString fAcc = fld->data("Access Policy").toString().trimmed().toUpper();
                        if (fAcc == "RW") hasRW = true;
                        else if (fAcc == "RO") hasRO = true;
                        else if (fAcc == "WO") hasWO = true;
                    }
                }
                if (hasRW) access = "RW";
                else if (hasRO && !hasWO) access = "RO";
                else if (hasWO && !hasRO) access = "WO";
                else access = "RW";
            }

            regList.push_back({child, offset, byteWidth, name, access, desc, row});
        }
    }

    // Sort registers ascending by offset
    std::sort(regList.begin(), regList.end(), [](const RawReg &a, const RawReg &b) {
        if (a.offset != b.offset) return a.offset < b.offset;
        return a.childRow < b.childRow;
    });

    // Traverse and insert explicit RESERVED gap blocks
    uint64_t curOffset = 0;
    if (!regList.isEmpty()) {
        curOffset = regList.first().offset;
    }

    for (int i = 0; i < regList.size(); ++i) {
        const RawReg &r = regList[i];
        if (r.offset > curOffset) {
            // Address gap detected! Fill with explicit RESERVED block
            uint64_t gapBytes = r.offset - curOffset;
            MemoryMapBlock gap;
            gap.item = nullptr;
            gap.offset = curOffset;
            gap.sizeBytes = gapBytes;
            gap.endOffset = r.offset - 1;
            gap.name = tr("RESERVED");
            gap.access = "NA";
            gap.desc = tr("Unmapped address space gap (%1 bytes)").arg(gapBytes);
            gap.isReserved = true;
            gap.childRow = -1;
            m_blocks.push_back(gap);
        }

        MemoryMapBlock regBlock;
        regBlock.item = r.item;
        regBlock.offset = r.offset;
        regBlock.sizeBytes = r.byteWidth;
        regBlock.endOffset = r.offset + r.byteWidth - 1;
        regBlock.name = r.name.isEmpty() ? tr("REG_%1").arg(r.offset, 0, 16) : r.name;
        regBlock.access = r.access;
        regBlock.desc = r.desc;
        regBlock.isReserved = false;
        regBlock.childRow = r.childRow;
        m_blocks.push_back(regBlock);

        curOffset = (std::max)(curOffset, r.offset + r.byteWidth);
    }

    // Calculate total height proportionally based on byte widths
    double pxPerByte = 14.0;
    int calculatedH = 30; // Top/bottom margin
    for (const auto &b : m_blocks) {
        double blockH = (std::max)(38.0, (std::min)(160.0, b.sizeBytes * pxPerByte));
        calculatedH += static_cast<int>(blockH) + 6; // height + gap
    }
    m_totalHeight = (std::max)(200, calculatedH);
    setMinimumHeight(m_totalHeight);
}

int BlockMemoryMapWidget::blockIndexAt(const QPoint &pos) const
{
    for (int i = 0; i < m_blockRects.size(); ++i) {
        if (m_blockRects[i].contains(pos)) {
            return i;
        }
    }
    return -1;
}

void BlockMemoryMapWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    const ColorScheme &theme = ThemeManager::instance().currentTheme();
    painter.fillRect(rect(), theme.panelBg);

    if (m_blocks.isEmpty()) {
        painter.setPen(theme.textMuted);
        QFont f = font();
        f.setPointSize(11);
        f.setItalic(true);
        painter.setFont(f);
        painter.drawText(rect(), Qt::AlignCenter, tr("No registers in selected block"));
        return;
    }

    m_blockRects.resize(m_blocks.size());

    int leftGutter = 78;
    int rightMargin = 16;
    int topMargin = 12;
    int curY = topMargin;
    int blockWidth = width() - leftGutter - rightMargin;
    if (blockWidth < 120) blockWidth = 120;

    double pxPerByte = 14.0;

    for (int i = 0; i < m_blocks.size(); ++i) {
        const MemoryMapBlock &b = m_blocks[i];
        double blockH = (std::max)(38.0, (std::min)(160.0, b.sizeBytes * pxPerByte));
        QRectF blockRect(leftGutter, curY, blockWidth, blockH);
        m_blockRects[i] = blockRect;

        // 1. Draw Address Ruler / Ticks on the left
        painter.setPen(theme.textMuted);
        QFont rulerFont = font();
        rulerFont.setPointSize(9);
        rulerFont.setFamily("monospace");
        painter.setFont(rulerFont);

        QString offsetStr = QString("0x%1").arg(b.offset, 4, 16, QChar('0')).toUpper();
        QRectF rulerRect(4, curY - 2, leftGutter - 12, 18);
        painter.drawText(rulerRect, Qt::AlignRight | Qt::AlignVCenter, offsetStr);

        // Tick line from ruler to block
        painter.drawLine(QPointF(leftGutter - 6, curY + 2), QPointF(leftGutter, curY + 2));

        // 2. Draw Block Body
        QPainterPath path;
        path.addRoundedRect(blockRect, 5, 5);

        if (b.isReserved) {
            // Reserved / Unmapped Gap Block
            painter.fillPath(path, theme.rsvdBg);

            // Diagonal stripe pattern
            painter.save();
            painter.setClipPath(path);
            painter.setPen(QPen(theme.rsvdStripe, 1.5, Qt::SolidLine));
            for (int x = -100; x < blockRect.right() + 100; x += 10) {
                painter.drawLine(QPointF(x, curY), QPointF(x + blockH + 20, curY + blockH + 20));
            }
            painter.restore();

            QPen borderPen(theme.rsvdBorder, (i == m_hoveredIndex) ? 2.0 : 1.0, Qt::DashLine);
            painter.strokePath(path, borderPen);

            // Text inside Reserved Block
            painter.setPen(theme.rsvdText);
            QFont resFont = font();
            resFont.setPointSize(10);
            resFont.setBold(true);
            painter.setFont(resFont);

            QString resText = tr("RESERVED");
            QString gapText = tr("[%1 Bytes Gap]").arg(b.sizeBytes);
            QRectF textRect = blockRect.adjusted(12, 0, -12, 0);
            painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, resText + "  " + gapText);

        } else {
            // Active Register Block
            AccessColors ac = ThemeManager::instance().getAccessColors(b.access, m_colorBlindMode);
            painter.fillPath(path, ac.bg);

            // Accent strip on left edge of block
            QPainterPath accentPath;
            accentPath.addRoundedRect(QRectF(blockRect.left(), blockRect.top(), 6, blockRect.height()), 5, 5);
            painter.fillPath(accentPath, ac.border);

            // Border
            bool isSelected = (i == m_selectedIndex);
            bool isHovered = (i == m_hoveredIndex);
            QColor borderColor = isSelected ? theme.selectionText : (isHovered ? ac.border.lighter(130) : ac.border);
            double borderWidth = (isSelected || isHovered) ? 2.0 : 1.0;
            painter.strokePath(path, QPen(borderColor, borderWidth));

            // Content: Register Name, Access Badge, Byte Width Badge, Description
            QRectF contentRect = blockRect.adjusted(14, 4, -8, -4);

            // Top Row: Register Name + Badges
            QFont nameFont = font();
            nameFont.setPointSize(10);
            nameFont.setBold(true);
            painter.setFont(nameFont);
            painter.setPen(theme.isDark ? QColor("#f8fafc") : ac.text);

            QFontMetrics fm(nameFont);
            QString displayName = fm.elidedText(b.name, Qt::ElideRight, contentRect.width() - 120);
            painter.drawText(contentRect.left(), contentRect.top() + fm.ascent() + 2, displayName);

            // Access Policy & Width Badges on right side of block
            int badgeRight = contentRect.right();
            QString accessBadge = (m_colorBlindMode != ColorBlindMode::None) ? QString("[%1]").arg(b.access) : b.access;
            QFont badgeFont = font();
            badgeFont.setPointSize(8);
            badgeFont.setBold(true);
            painter.setFont(badgeFont);
            QFontMetrics bfm(badgeFont);

            // Width Badge (e.g. 4B / 8B)
            QString widthStr = QString("%1B").arg(b.sizeBytes);
            int widthBadgeW = bfm.horizontalAdvance(widthStr) + 8;
            QRectF widthBadgeRect(badgeRight - widthBadgeW, contentRect.top() + 2, widthBadgeW, 18);
            QPainterPath widthBadgePath;
            widthBadgePath.addRoundedRect(widthBadgeRect, 3, 3);
            painter.fillPath(widthBadgePath, theme.headerBg);
            painter.setPen(theme.textColor);
            painter.drawText(widthBadgeRect, Qt::AlignCenter, widthStr);

            // Access Policy Badge
            int accBadgeW = bfm.horizontalAdvance(accessBadge) + 10;
            QRectF accBadgeRect(widthBadgeRect.left() - accBadgeW - 4, contentRect.top() + 2, accBadgeW, 18);
            QPainterPath accBadgePath;
            accBadgePath.addRoundedRect(accBadgeRect, 3, 3);
            painter.fillPath(accBadgePath, ac.border);
            painter.setPen(QColor("#ffffff"));
            painter.drawText(accBadgeRect, Qt::AlignCenter, accessBadge);

            // Sub-row: Address range and optional description
            if (blockH >= 46) {
                QFont subFont = font();
                subFont.setPointSize(8);
                painter.setFont(subFont);
                painter.setPen(theme.textMuted);

                QString rangeStr = QString("Range: 0x%1..0x%2 (%3 bytes)")
                                       .arg(b.offset, 2, 16, QChar('0'))
                                       .arg(b.endOffset, 2, 16, QChar('0'))
                                       .arg(b.sizeBytes);
                if (!b.desc.isEmpty()) {
                    rangeStr += QString(" — %1").arg(b.desc);
                }
                QFontMetrics sfm(subFont);
                QString elidedSub = sfm.elidedText(rangeStr, Qt::ElideRight, contentRect.width() - 8);
                painter.drawText(contentRect.left(), contentRect.bottom() - 2, elidedSub);
            }
        }

        curY += static_cast<int>(blockH) + 6;
    }

    // Bottom boundary offset label for the last block
    if (!m_blocks.isEmpty()) {
        const MemoryMapBlock &last = m_blocks.last();
        QString endOffsetStr = QString("0x%1").arg(last.endOffset + 1, 4, 16, QChar('0')).toUpper();
        QRectF rulerRect(4, curY - 2, leftGutter - 12, 18);
        painter.setPen(theme.textMuted);
        QFont rulerFont = font();
        rulerFont.setPointSize(8);
        rulerFont.setFamily("monospace");
        painter.setFont(rulerFont);
        painter.drawText(rulerRect, Qt::AlignRight | Qt::AlignVCenter, endOffsetStr);
        painter.drawLine(QPointF(leftGutter - 6, curY + 2), QPointF(leftGutter, curY + 2));
    }
}

void BlockMemoryMapWidget::mouseMoveEvent(QMouseEvent *event)
{
    int idx = blockIndexAt(event->pos());
    if (idx != m_hoveredIndex) {
        m_hoveredIndex = idx;
        update();

        if (idx >= 0 && idx < m_blocks.size()) {
            const MemoryMapBlock &b = m_blocks[idx];
            if (b.isReserved) {
                setCursor(Qt::ArrowCursor);
                QToolTip::showText(event->globalPosition().toPoint(),
                    tr("<b>RESERVED GAP</b><br>Offset Range: 0x%1..0x%2 (%3 bytes)")
                        .arg(b.offset, 0, 16)
                        .arg(b.endOffset, 0, 16)
                        .arg(b.sizeBytes),
                    this);
            } else {
                setCursor(Qt::PointingHandCursor);
                QString tip = tr("<b>Register: %1</b><br>"
                                 "Offset: 0x%2..0x%3 (%4 bytes)<br>"
                                 "Access: %5<br>"
                                 "Description: %6<br><i>Click to inspect bitfields</i>")
                                  .arg(b.name)
                                  .arg(b.offset, 0, 16)
                                  .arg(b.endOffset, 0, 16)
                                  .arg(b.sizeBytes)
                                  .arg(b.access)
                                  .arg(b.desc.isEmpty() ? tr("None") : b.desc);
                QToolTip::showText(event->globalPosition().toPoint(), tip, this);
            }
        } else {
            setCursor(Qt::ArrowCursor);
            QToolTip::hideText();
        }
    }
}

void BlockMemoryMapWidget::mousePressEvent(QMouseEvent *event)
{
    int idx = blockIndexAt(event->pos());
    if (idx >= 0 && idx < m_blocks.size()) {
        const MemoryMapBlock &b = m_blocks[idx];
        if (!b.isReserved && b.childRow >= 0) {
            m_selectedIndex = idx;
            update();
            emit registerClicked(b.childRow, b.item);
        }
    }
}

void BlockMemoryMapWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    if (m_hoveredIndex != -1) {
        m_hoveredIndex = -1;
        setCursor(Qt::ArrowCursor);
        QToolTip::hideText();
        update();
    }
}

QSize BlockMemoryMapWidget::sizeHint() const
{
    return QSize(360, m_totalHeight);
}

QSize BlockMemoryMapWidget::minimumSizeHint() const
{
    return QSize(220, 150);
}
