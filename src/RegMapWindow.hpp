#ifndef REGMAPWINDOW_HPP
#define REGMAPWINDOW_HPP
#include <fstream>
#include <fcntl.h>
#include <iostream>
#include <sys/stat.h>
#include <QMainWindow>
#include <QFileInfo>
#include <QtWidgets>
#include <QDebug>
#include <google/protobuf/util/time_util.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "rmap.pb.h"
#include "RegConfigWindow.hpp"
#include "RegMapDelegate.hpp"
#include "RegMapTreeView.hpp"
#include "RegMapTreeModel.hpp"
#include "RegMapTreeItem.hpp"
#include "SerializationContext.hpp"
#include "ProtobufLogCollector.hpp"
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

    RegConfigWindow * m_config_window;
    RegMapTreeModel * m_model;
    QString           m_rmap_filename;
    QString           m_default_filename;
    QString           m_active_folder;
    QString           m_default_window_title;
    bool              m_is_regmap_modified;

    void fileNew(void);
    void fileOpen(QString fname);
    bool fileSave(QString fname = nullptr);
    void insertChild(RegMapTreeItem::e_rmmKind kind);
    void regmap_modified(void);
    void regmap_notModified(void);
    void btnFileNew(void);
    void btnFileOpen(void);
    bool btnFileSave(void);
    bool btnFileSaveAs(void);
    void btnFileReload(void);
    void btnDeleteItem(void);
    void btnCheck(void);
    void btnExport(void);
    void btnQuitButton(void);
    void btnConfig(void);
    void btnAbout(void);
};

#endif
