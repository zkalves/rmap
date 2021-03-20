#ifndef BLOCKMEMORYMAPWIDGET_HPP
#define BLOCKMEMORYMAPWIDGET_HPP

#include <QWidget>
#include <QVector>
#include <QString>
#include <QRectF>
#include <cstdint>
#include "RegMapTreeItem.hpp"

struct MemoryMapBlock {
    RegMapTreeItem* item = nullptr;
    uint64_t offset = 0;
    uint64_t sizeBytes = 4;
    uint64_t endOffset = 3;
    QString name;
    QString access;
    QString desc;
    bool isReserved = false;
    int childRow = -1;
};

class BlockMemoryMapWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(BlockMemoryMapWidget)

public:
    explicit BlockMemoryMapWidget(QWidget *parent = nullptr);
    ~BlockMemoryMapWidget() override = default;

    void setBlock(RegMapTreeItem *blkItem, uint32_t regWidthBits = 32);
    void clear();
    void refresh();
    void setSelectedRegister(int childRow);

    const QVector<MemoryMapBlock>& getBlocks() const { return m_blocks; }
    const QVector<MemoryMapBlock>& blocks() const { return m_blocks; }

    void setColorBlindMode(bool enabled);
    bool isColorBlindMode() const { return m_colorBlindMode; }

signals:
    void registerClicked(int childRow, RegMapTreeItem *regItem);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    void computeBlocks();
    int blockIndexAt(const QPoint &pos) const;

    RegMapTreeItem *m_blkItem = nullptr;
    uint32_t m_globalRegWidthBits = 32;
    bool m_colorBlindMode = false;
    QVector<MemoryMapBlock> m_blocks;
    QVector<QRectF> m_blockRects;
    int m_hoveredIndex = -1;
    int m_selectedIndex = -1;
    int m_totalHeight = 300;
};

#endif // BLOCKMEMORYMAPWIDGET_HPP
