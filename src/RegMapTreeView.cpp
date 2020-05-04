#include "RegMapTreeView.hpp"

RegMapTreeView::RegMapTreeView(QWidget *parent) : QTreeView(parent)
{
}

void RegMapTreeView::mousePressEvent(QMouseEvent *event)
{
        clearSelection();
        setCurrentIndex(rootIndex());
        QTreeView::mousePressEvent(event);
}
