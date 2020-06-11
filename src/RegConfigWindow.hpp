#ifndef REGCONFIGWINDOW_HPP
#define REGCONFIGWINDOW_HPP
#include <QDebug>
#include <QWidget>
#include "rmap.pb.h"
#include "ui_config.h"

namespace Ui {
class RegConfigWindow;
}

class RegConfigWindow : public QDialog, private Ui::config
{
    Q_OBJECT

public:
    explicit RegConfigWindow(QWidget *parent = nullptr);
    protormap::Config* serialize(void);
    void deserialize(const protormap::Config config);

private:
    QString m_python_script;
    void accept(void);
    void reject(void);

};

#endif
