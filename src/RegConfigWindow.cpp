#include "RegConfigWindow.hpp"

RegConfigWindow::RegConfigWindow(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
}

void RegConfigWindow::accept(void)
{
    qDebug()<<"Accept button";
    done(Accepted);
}
void RegConfigWindow::reject(void)
{
    qDebug()<<"Reject button";
    done(Rejected);
}

