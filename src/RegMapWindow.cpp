#include <QtWidgets>
#include <iostream>

#include "RegMapWindow.hpp"

RegMapWindow::RegMapWindow(QString &regmap_file, QWidget *parent) :
    QMainWindow(parent)
{
    setupUi(this);
    std::cout << regmap_file.toUtf8().constData() << std::endl;

}
