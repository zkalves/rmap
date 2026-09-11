/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "RegBitfieldBarWidget.hpp"
#include "RegMapDelegate.hpp"
#include "AppSettings.hpp"
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QToolTip>
#include <QFontMetrics>
#include <algorithm>

static uint64_t parseNum(const QVariant& var) {
    QString str = var.toString().trimmed();
    if (str.startsWith("0x", Qt::CaseInsensitive)) return str.mid(2).toULongLong(nullptr, 16);
    if (str.startsWith("0b", Qt::CaseInsensitive)) return str.mid(2).toULongLong(nullptr, 2);
    return str.toULongLong(nullptr, 10);
}

RegBitfieldBarWidget::RegBitfieldBarWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumHeight(76);

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this]() {
        update();
    });
}

RegBitfieldBarWidget::~RegBitfieldBarWidget() = default;

void RegBitfieldBarWidget::setRegister(RegMapTreeItem *regItem, uint32_t regWidth)
{
    m_regItem = regItem;
    m_regWidth = (regWidth == 0) ? 32 : regWidth;
    m_hoveredSlice = -1;
    m_selectedSlice = -1;
    computeSlices();
    update();
}

void RegBitfieldBarWidget::clear()
{
    m_regItem = nullptr;
    m_slices.clear();
    m_sliceRects.clear();
    m_hoveredSlice = -1;
    m_selectedSlice = -1;
    update();
}

void RegBitfieldBarWidget::refresh()
{
    if (!m_regItem) return;
    int savedSelectedRow = -1;
    if (m_selectedSlice >= 0 && m_selectedSlice < m_slices.size()) {
        savedSelectedRow = m_slices[m_selectedSlice].childRow;
    }
    computeSlices();
    if (savedSelectedRow >= 0) {
        setSelectedField(savedSelectedRow);
    } else {
        update();
    }
}

void RegBitfieldBarWidget::setSelectedField(int childRow)
{
    m_selectedSlice = -1;
    for (int i = 0; i < m_slices.size(); ++i) {
        if (!m_slices[i].isReserved && m_slices[i].childRow == childRow) {
            m_selectedSlice = i;
            break;
        }
    }
    update();
}

void RegBitfieldBarWidget::computeSlices()
{
    m_slices.clear();
    m_sliceRects.clear();

    if (!m_regItem || m_regWidth == 0) {
        return;
    }

    struct FieldEntry {
        RegMapTreeItem* item;
        uint64_t lsb;
        uint64_t msb;
        uint64_t width;
        QString name;
        QString access;
        QString hwAccess;
        uint64_t resetVal;
        QString desc;
        int childRow;
    };

    QVector<FieldEntry> entries;
    const auto& children = m_regItem->getChildItems();
    for (int row = 0; row < children.size(); ++row) {
        RegMapTreeItem *child = children[row];
        if (child && child->kindString() == "fld") {
            uint64_t lsb = parseNum(child->data("Offset/LSB"));
            uint64_t width = parseNum(child->data("Size/Width"));
            if (width == 0) width = 1;
            uint64_t msb = lsb + width - 1;
            QString name = child->data("Name").toString();
            QString access = child->data("SW Access").toString().trimmed().toUpper();
            QString hwAccess = child->data("HW Access").toString().trimmed().toUpper();
            if (hwAccess.isEmpty()) hwAccess = "RO";
            uint64_t resetVal = parseNum(child->data("Reset Value"));
            QString desc = child->data("Description").toString();
            entries.push_back({child, lsb, msb, width, name, access.isEmpty() ? "RW" : access, hwAccess, resetVal, desc, row});
        }
    }

    // Sort entries descending by MSB (so bits are ordered [regWidth-1] downto 0 from left to right)
    std::sort(entries.begin(), entries.end(), [](const FieldEntry& a, const FieldEntry& b) {
        return a.msb > b.msb;
    });

    int64_t currentBit = static_cast<int64_t>(m_regWidth) - 1;

    for (const auto& entry : entries) {
        // If there's an unmapped gap between currentBit and entry.msb, fill with a reserved slice
        if (currentBit > static_cast<int64_t>(entry.msb)) {
            BitfieldSlice resSlice;
            resSlice.isReserved = true;
            resSlice.msb = currentBit;
            resSlice.lsb = entry.msb + 1;
            resSlice.width = resSlice.msb - resSlice.lsb + 1;
            resSlice.name = "Reserved";
            resSlice.access = "-";
            resSlice.hwAccess = "-";
            resSlice.resetVal = 0;
            resSlice.childRow = -1;
            m_slices.push_back(resSlice);
            currentBit = entry.msb;
        }

        // Add actual field slice
        BitfieldSlice fSlice;
        fSlice.item = entry.item;
        fSlice.isReserved = false;
        fSlice.lsb = entry.lsb;
        fSlice.msb = entry.msb;
        fSlice.width = entry.width;
        fSlice.name = entry.name.isEmpty() ? "UNNAMED" : entry.name;
        fSlice.access = entry.access;
        fSlice.hwAccess = entry.hwAccess;
        fSlice.resetVal = entry.resetVal;
        fSlice.desc = entry.desc;
        fSlice.childRow = entry.childRow;
        m_slices.push_back(fSlice);

        currentBit = static_cast<int64_t>(entry.lsb) - 1;
    }

    // If there's a trailing gap down to bit 0
    if (currentBit >= 0) {
        BitfieldSlice resSlice;
        resSlice.isReserved = true;
        resSlice.msb = currentBit;
        resSlice.lsb = 0;
        resSlice.width = resSlice.msb - resSlice.lsb + 1;
        resSlice.name = "Reserved";
        resSlice.access = "-";
        resSlice.hwAccess = "-";
        resSlice.resetVal = 0;
        resSlice.childRow = -1;
        m_slices.push_back(resSlice);
    }
}

void RegBitfieldBarWidget::setColorBlindMode(bool enabled)
{
    ColorBlindMode mode = enabled ? AppSettings::instance().colorBlindType() : ColorBlindMode::None;
    if (mode == ColorBlindMode::None && enabled) {
        mode = ColorBlindMode::Universal;
    }
    setColorBlindMode(mode);
}

void RegBitfieldBarWidget::setColorBlindMode(ColorBlindMode mode)
{
    if (m_colorBlindMode != mode) {
        m_colorBlindMode = mode;
        update();
    }
}

const QVector<BitfieldSlice>& RegBitfieldBarWidget::getSlices() const
{
    return m_slices;
}

const QVector<BitfieldSlice>& RegBitfieldBarWidget::slices() const
{
    return m_slices;
}

bool RegBitfieldBarWidget::isColorBlindMode() const
{
    return m_colorBlindMode != ColorBlindMode::None;
}

ColorBlindMode RegBitfieldBarWidget::colorBlindMode() const
{
    return m_colorBlindMode;
}

QColor RegBitfieldBarWidget::getAccessColor(const QString &access, bool isBackground) const
{
    AccessColors c = getAccessPolicyColors(access, m_colorBlindMode);
    return isBackground ? c.bg : c.border;
}

void RegBitfieldBarWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int totalWidth = width();
    const int totalHeight = height();
    const int marginX = 8;
    const int topRulerHeight = 22;
    const int barY = topRulerHeight + 2;
    const int barHeight = totalHeight - barY - 6;
    const double usableWidth = totalWidth - 2 * marginX;

    const ColorScheme &theme = ThemeManager::instance().currentTheme();
    bool cbActive = (m_colorBlindMode != ColorBlindMode::None);

    if (!m_regItem || m_slices.isEmpty() || m_regWidth == 0) {
        p.setPen(theme.textMuted);
        QFont f = font();
        f.setItalic(true);
        p.setFont(f);
        p.drawText(rect(), Qt::AlignCenter, tr("Select a register to view bitfield layout"));
        return;
    }

    m_sliceRects.resize(m_slices.size());
    const double bitWidthPx = usableWidth / static_cast<double>(m_regWidth);

    // Draw top bit ruler labels
    QFont rulerFont = font();
    rulerFont.setPointSize(11);
    rulerFont.setBold(true);
    p.setFont(rulerFont);
    p.setPen(theme.rulerText);

    // Paint bit indices
    for (uint32_t b = 0; b < m_regWidth; ++b) {
        if (b == 0 || b == m_regWidth - 1 || b % 8 == 0 || b % 8 == 7) {
            double x = marginX + (m_regWidth - 1 - b) * bitWidthPx;
            QRectF bitTextRect(x - 14, 0, bitWidthPx + 28, topRulerHeight);
            p.drawText(bitTextRect, Qt::AlignCenter, QString::number(b));
        }
    }

    // Paint each bitfield slice
    for (int i = 0; i < m_slices.size(); ++i) {
        const BitfieldSlice &slice = m_slices[i];
        double startX = marginX + (m_regWidth - 1 - slice.msb) * bitWidthPx;
        double sliceW = slice.width * bitWidthPx;
        QRectF sliceRect(startX, barY, sliceW, barHeight);
        m_sliceRects[i] = sliceRect;

        bool isHovered = (i == m_hoveredSlice);
        bool isSelected = (i == m_selectedSlice);

        if (slice.isReserved) {
            // Reserved slice background & stripes from theme
            p.save();
            QColor resBg = cbActive ? QColor(95, 100, 108) : theme.rsvdBg;
            QColor resBorder = cbActive ? QColor(65, 70, 78) : theme.rsvdBorder;
            QColor resStripe = cbActive ? QColor(80, 85, 92) : theme.rsvdStripe;

            p.setPen(QPen(resBorder, 1.0, Qt::SolidLine));
            p.setBrush(QBrush(resBg));
            p.drawRoundedRect(sliceRect.adjusted(1, 1, -1, -1), 3, 3);

            // Paint diagonal stripes
            p.setClipRect(sliceRect.adjusted(1, 1, -1, -1));
            p.setPen(QPen(resStripe, 1.2));
            double step = 8.0;
            double xmin = sliceRect.left() - sliceRect.height();
            double xmax = sliceRect.right();
            for (double sx = xmin; sx < xmax; sx += step) {
                p.drawLine(QPointF(sx, sliceRect.bottom()), QPointF(sx + sliceRect.height(), sliceRect.top()));
            }
            p.restore();

            if (sliceW > 24) {
                QFont f = font();
                f.setPointSize(sliceW > 60 ? 8 : 7);
                f.setBold(true);
                p.setFont(f);
                p.setPen(cbActive ? QColor(235, 238, 242) : theme.rsvdText);
                p.drawText(sliceRect, Qt::AlignCenter, sliceW > 60 ? "RESERVED" : "RSVD");
            }
        } else {
            // Active field slice
            AccessColors ac = getAccessPolicyColors(slice.access, m_colorBlindMode);
            QColor bgColor = ac.bg;
            QColor borderColor = ac.border;

            if (isSelected) {
                bgColor = bgColor.lighter(theme.isDark ? 125 : 105);
                borderColor = cbActive ? QColor(0, 60, 150) : (theme.isDark ? theme.selectionText : QColor(0, 120, 215));
            } else if (isHovered) {
                bgColor = bgColor.lighter(theme.isDark ? 120 : 110);
            }

            p.save();
            p.setPen(QPen(borderColor, isSelected ? 2.5 : (isHovered ? 1.8 : 1.0)));
            p.setBrush(bgColor);
            p.drawRoundedRect(sliceRect.adjusted(1, 1, -1, -1), 4, 4);
            p.restore();

            // Text inside slice
            if (sliceW >= 14) {
                p.save();
                p.setClipRect(sliceRect.adjusted(2, 2, -2, -2));

                QFont nameFont = font();
                nameFont.setPointSize(sliceW < 40 ? 7 : (sliceW < 80 ? 8 : 9));
                nameFont.setBold(true);
                p.setFont(nameFont);
                p.setPen(isSelected ? (theme.isDark ? theme.selectionText : QColor(0, 60, 150)) : (cbActive ? ac.text : (theme.isDark ? theme.textColor : borderColor.darker(150))));

                QFontMetrics fm(nameFont);
                QString elidedName = fm.elidedText(slice.name, Qt::ElideRight, static_cast<int>(sliceW - 4));
                QRectF nameRect(sliceRect.x() + 2, sliceRect.y() + 2, sliceW - 4, sliceRect.height() * 0.45);
                p.drawText(nameRect, Qt::AlignCenter, elidedName);

                // Bit range & Access badge
                QFont subFont = font();
                subFont.setPointSize(7);
                p.setFont(subFont);
                p.setPen(theme.textMuted);

                QString rangeStr = (slice.msb == slice.lsb) ? QString("[%1]").arg(slice.lsb) : QString("[%1:%2]").arg(slice.msb).arg(slice.lsb);
                QRectF rangeRect(sliceRect.x() + 2, sliceRect.y() + sliceRect.height() * 0.45, sliceW - 4, sliceRect.height() * 0.3);
                p.drawText(rangeRect, Qt::AlignCenter, rangeStr);

                if (sliceW >= 35) {
                    QRectF accessRect(sliceRect.x() + 2, sliceRect.y() + sliceRect.height() * 0.72, sliceW - 4, sliceRect.height() * 0.25);
                    p.setPen(borderColor);
                    QString accessText = cbActive ? QString("[%1]").arg(slice.access) : slice.access;
                    p.drawText(accessRect, Qt::AlignCenter, accessText);
                }

                p.restore();
            }
        }
    }
}

int RegBitfieldBarWidget::sliceIndexAt(const QPoint &pos) const
{
    for (int i = 0; i < m_sliceRects.size(); ++i) {
        if (m_sliceRects[i].contains(pos) ||
            (pos.x() >= m_sliceRects[i].left() && pos.x() <= m_sliceRects[i].right() &&
             pos.y() >= 0 && pos.y() <= height())) {
            return i;
        }
    }
    return -1;
}

void RegBitfieldBarWidget::mouseMoveEvent(QMouseEvent *event)
{
    int idx = sliceIndexAt(event->pos());
    if (idx != m_hoveredSlice) {
        m_hoveredSlice = idx;
        update();

        if (idx >= 0 && idx < m_slices.size()) {
            const BitfieldSlice &s = m_slices[idx];
            if (s.isReserved) {
                QToolTip::showText(event->globalPosition().toPoint(),
                    tr("<b>Reserved / Unmapped Bits</b><br>Bits: [%1:%2]<br>Width: %3 bits")
                    .arg(s.msb).arg(s.lsb).arg(s.width), this);
            } else {
                QString tip = tr("<b>Field: %1</b><br><b>Bits:</b> [%2:%3] (%4 bits)<br><b>SW Access:</b> %5<br><b>HW Access:</b> %6<br><b>Reset:</b> 0x%7")
                    .arg(s.name)
                    .arg(s.msb).arg(s.lsb).arg(s.width)
                    .arg(s.access)
                    .arg(s.hwAccess)
                    .arg(QString::number(s.resetVal, 16).toUpper());
                if (!s.desc.isEmpty()) {
                    tip += tr("<br><b>Description:</b> %1").arg(s.desc);
                }
                QToolTip::showText(event->globalPosition().toPoint(), tip, this);
            }
        } else {
            QToolTip::hideText();
        }
    }
}

void RegBitfieldBarWidget::mousePressEvent(QMouseEvent *event)
{
    int idx = sliceIndexAt(event->pos());
    if (idx >= 0 && idx < m_slices.size()) {
        const BitfieldSlice &s = m_slices[idx];
        if (!s.isReserved && s.childRow >= 0) {
            m_selectedSlice = idx;
            update();
            emit fieldClicked(s.childRow);
        }
    }
}

void RegBitfieldBarWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    if (m_hoveredSlice != -1) {
        m_hoveredSlice = -1;
        update();
    }
}

QSize RegBitfieldBarWidget::sizeHint() const
{
    return QSize(600, 80);
}

QSize RegBitfieldBarWidget::minimumSizeHint() const
{
    return QSize(300, 68);
}
