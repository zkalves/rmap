#include "RegConfigWindow.hpp"

RegConfigWindow::RegConfigWindow(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
}

protormap::Config* RegConfigWindow::serialize(void)
{
    protormap::Config* config = new protormap::Config;
    config->set_pythonscript(m_python_script.toStdString());
    return(config);
}

void RegConfigWindow::deserialize(const protormap::Config config)
{
    m_python_script = QString::fromStdString(config.pythonscript());
    this->pythonScript->setText(m_python_script);
}

void RegConfigWindow::accept(void)
{
    m_python_script = this->pythonScript->text();
    done(Accepted);
}
void RegConfigWindow::reject(void)
{
    this->pythonScript->setText(m_python_script);
    done(Rejected);
}

