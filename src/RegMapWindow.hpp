#ifndef REGMAPWINDOW_HPP
#define REGMAPWINDOW_HPP
#include <QMainWindow>
#include "RegMapTreeView.hpp"
#include "ui_rmap.h"

namespace Ui {
class RegMapWindow;
}

class RegMapWindow : public QMainWindow, private Ui::rmap
{
    Q_OBJECT

public:
    explicit RegMapWindow(QString &regmap_file, QWidget *parent = nullptr);
};

#endif
