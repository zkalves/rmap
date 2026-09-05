/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#ifndef REGBITFIELDBARWIDGET_HPP
#define REGBITFIELDBARWIDGET_HPP

#include <QWidget>
#include <QVector>
#include <QString>
#include <QRectF>
#include <cstdint>
#include "RegMapTreeItem.hpp"

struct BitfieldSlice {
    RegMapTreeItem* item = nullptr;
    uint64_t lsb = 0;
    uint64_t msb = 0;
    uint64_t width = 0;
    QString name;
    QString access;    // SW access
    QString hwAccess;  // HW access
    uint64_t resetVal = 0;
    QString desc;
    bool isReserved = false;
    int childRow = -1;
};

class RegBitfieldBarWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(RegBitfieldBarWidget)

public:
    explicit RegBitfieldBarWidget(QWidget *parent = nullptr);
    ~RegBitfieldBarWidget() override = default;

    void setRegister(RegMapTreeItem *regItem, uint32_t regWidth = 32);
    void clear();
    void refresh();
    void setSelectedField(int childRow);
    const QVector<BitfieldSlice>& getSlices() const { return m_slices; }
    const QVector<BitfieldSlice>& slices() const { return m_slices; }

    void setColorBlindMode(bool enabled);
    bool isColorBlindMode() const { return m_colorBlindMode; }

signals:
    void fieldClicked(int childRow);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    void computeSlices();
    int sliceIndexAt(const QPoint &pos) const;
    QColor getAccessColor(const QString &access, bool isBackground) const;

    RegMapTreeItem *m_regItem = nullptr;
    uint32_t m_regWidth = 32;
    bool m_colorBlindMode = false;
    QVector<BitfieldSlice> m_slices;
    int m_hoveredSlice = -1;
    int m_selectedSlice = -1;
    QVector<QRectF> m_sliceRects;
};

#endif // REGBITFIELDBARWIDGET_HPP
