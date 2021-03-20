#ifndef REGMAPTREEVIEW_HPP
#define REGMAPTREEVIEW_HPP

#include <QTreeView>
#include <QMouseEvent>

namespace Ui {
class RegMapTreeView;
}

class RegMapTreeView : public QTreeView
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(RegMapTreeView)

public:
    explicit RegMapTreeView(QWidget* parent = nullptr);
    ~RegMapTreeView() override = default;

protected:
    void mousePressEvent(QMouseEvent *event) override;
};

#endif // REGMAPTREEVIEW_HPP

