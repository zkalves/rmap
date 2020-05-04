#ifndef REGMAPTREEVIEW_HPP
#define REGMAPTREEVIEW_HPP
#include <QtWidgets>
namespace Ui {
class RegMapTreeView;
}

class RegMapTreeView : public QTreeView
{
    Q_OBJECT

public:
    explicit RegMapTreeView(QWidget* parent = 0);

protected:
    void mousePressEvent(QMouseEvent *event);
};
#endif

