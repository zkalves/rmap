#ifndef REGMAPWINDOW_HPP
#define REGMAPWINDOW_HPP
#include <QMainWindow>
#include <QtWidgets>
#include <QDebug>
#include "RegConfigWindow.hpp"
#include "RegMapDelegate.hpp"
#include "RegMapTreeView.hpp"
#include "RegMapTreeModel.hpp"
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
    void fileNew(void);
    void fileOpen(QString fname);
    bool fileSave(QString fname = nullptr);
    void regmap_modified(void);
    void regmap_notModified(void);
    void btnFileNew(void);
    void btnFileOpen(void);
    bool btnFileSave(void);
    bool btnFileSaveAs(void);
    void btnFileReload(void);
    void btnAddMem(void);
    void btnAddRegBlock(void);
    void btnAddRegField(void);
    void btnDeleteItem(void);
    void btnAddRegMap(void);
    void btnAddReg(void);
    void btnCheck(void);
    void btnExport(void);
    void btnQuitButton(void);
    void btnConfig(void);
    void btnAbout(void);

    RegConfigWindow * m_config_window;
    RegMapTreeModel * m_model;
    QString           m_rmap_filename;
    QString           m_default_filename;
    QString           m_active_folder;
    QString           m_default_window_title;
    bool              m_is_regmap_modified;
};

#endif
