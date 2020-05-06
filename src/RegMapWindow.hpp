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
    QString           m_rmap_filename;
    QString           m_default_filename;
    QString           m_active_folder;
    QString           m_default_window_title;
    bool              m_is_regmap_modified;
};

#endif
