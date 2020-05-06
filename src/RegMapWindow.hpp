#ifndef REGMAPWINDOW_HPP
#define REGMAPWINDOW_HPP
#include <QMainWindow>
#include <QtWidgets>
#include <iostream>
#include "RegConfigWindow.hpp"
#include "RegMapTreeView.hpp"
#include "ui_rmap.h"


namespace Ui {
class RegMapWindow;
}

class RegMapWindow : public QMainWindow, private Ui::rmap
{
    Q_OBJECT

public:
    explicit RegMapWindow(QString &rmap_filename, QWidget *parent = nullptr);

private:
    void btnConfig(void);
    RegConfigWindow * m_config_window;
};

#endif
