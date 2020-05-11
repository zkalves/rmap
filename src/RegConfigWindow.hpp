#ifndef REGCONFIGWINDOW_HPP
#define REGCONFIGWINDOW_HPP
#include <QDebug>
#include <QWidget>
#include "ui_config.h"

namespace Ui {
class RegConfigWindow;
}

class RegConfigWindow : public QDialog, private Ui::config
{
    Q_OBJECT

public:
    explicit RegConfigWindow(QWidget *parent = nullptr);
private:
    void accept(void);
    void reject(void);
};

#endif
