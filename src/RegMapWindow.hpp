/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#ifndef REGMAPWINDOW_HPP
#define REGMAPWINDOW_HPP

#include <QMainWindow>
#include <QFileInfo>
#include <QSplitter>
#include <QSortFilterProxyModel>
#include <QUndoStack>
#include <QDebug>
#include <QLineEdit>
#include <QLabel>
#include <QTableView>
#include <QActionGroup>
#include <QStackedWidget>
#include <QScrollArea>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QMoveEvent>
#include <google/protobuf/util/time_util.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "rmap.pb.h"
#include "CodeGenerator.hpp"
#include "RegConfigWindow.hpp"
#include "ThemeManager.hpp"
#include "RegMapDelegate.hpp"
#include "RegMapTreeView.hpp"
#include "RegMapTreeModel.hpp"
#include "RegMapTreeItem.hpp"
#include "RegBitfieldBarWidget.hpp"
#include "PreferencesWindow.hpp"
#include "AboutWindow.hpp"
#include "SerializationContext.hpp"
#include "ProtobufLogCollector.hpp"
#include "UndoCommands.hpp"
#include "ui_rmap.h"

namespace Ui {
    class RegMapWindow;
}

class TreeFilterProxyModel;
class FieldSortProxyModel;
class BlockMemoryMapWidget;

class RegMapWindow : public QMainWindow, private Ui::rmap
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(RegMapWindow)

    friend class TestRegMapWindow;

    public:
        explicit RegMapWindow(const QString &rmap_filename = QString(), QWidget *parent = nullptr);
        ~RegMapWindow() override;

        bool headlessExport(const QString &out_dir);
        bool headlessLint(bool strict, const QString &format, const QString &outFile);
        static bool semanticDiff(const QString &file1, const QString &file2, const QString &format, const QString &outFile);

        void fileOpen(QString fname);
        bool fileSave(QString fname = "");

        RegMapTreeModel* getModel() const { return m_model; }
        RegMapTreeModel* model() const { return m_model; }
        QUndoStack* getUndoStack() const { return m_undoStack; }
        QUndoStack* undoStack() const { return m_undoStack; }
        void insertChild(RegMapTreeItem::e_rmmKind kind);

        RegConfigWindow* configWindow() const { return m_config_window; }
        PreferencesWindow* preferencesWindow() const { return m_pref_window; }
        AboutWindow* aboutWindow() const { return m_about_window; }
        RegBitfieldBarWidget* bitfieldWidget() const { return m_bitfieldBar; }
        BlockMemoryMapWidget* memoryMapWidget() const { return m_blockMemoryMapWidget; }

        bool isModelLoaded() const;
        bool hasModel() const { return isModelLoaded(); }
        QStackedWidget* leftStackedWidget() const { return m_leftStackedWidget; }
        QWidget* leftViewWidget() const { return m_leftViewWidget; }
        QWidget* leftEmptyWidget() const { return m_leftEmptyWidget; }
        QStackedWidget* rightStackedWidget() const { return m_rightStackedWidget; }
        QWidget* emptyViewWidget() const { return m_emptyViewWidget; }

        void setColourBlindMode(bool enabled);
        bool isColourBlindMode() const;
        void setColorBlindMode(bool enabled) { setColourBlindMode(enabled); }
        bool isColorBlindMode() const { return isColourBlindMode(); }
        void setColourBlindType(ColorBlindMode mode);
        ColorBlindMode colourBlindType() const;
        void setColorBlindType(ColorBlindMode mode) { setColourBlindType(mode); }
        ColorBlindMode colorBlindType() const { return colourBlindType(); }

        void setColourScheme(const QString &scheme);
        QString colourScheme() const;
        void setColorScheme(const QString &scheme) { setColourScheme(scheme); }
        QString colorScheme() const { return colourScheme(); }

        void setLanguage(const QString &code);
        QString language() const;

        void saveWindowStateToSettings();
        void restoreWindowStateFromSettings();

    protected:
        void closeEvent(QCloseEvent *event) override;
        void resizeEvent(QResizeEvent *event) override;
        void moveEvent(QMoveEvent *event) override;
        void changeEvent(QEvent *event) override;

    private:
        RegConfigWindow       * m_config_window = nullptr;
        PreferencesWindow     * m_pref_window = nullptr;
        AboutWindow           * m_about_window = nullptr;
        RegMapTreeModel       * m_model = nullptr;
        TreeFilterProxyModel  * m_treeProxy = nullptr;
        FieldSortProxyModel   * m_fieldProxy = nullptr;

        // Left Pane Stacked View (Tree View vs Empty View)
        QStackedWidget        * m_leftStackedWidget = nullptr;
        QWidget               * m_leftViewWidget = nullptr;
        QWidget               * m_leftEmptyWidget = nullptr;

        // Right Pane Stacked View (Register Bitfield View vs Block Memory Map View)
        QStackedWidget        * m_rightStackedWidget = nullptr;
        QWidget               * m_regViewWidget = nullptr;
        QWidget               * m_blockViewWidget = nullptr;
        QWidget               * m_emptyViewWidget = nullptr;

        // Register View Components
        QTableView            * m_fieldsTableView = nullptr;
        RegBitfieldBarWidget  * m_bitfieldBar = nullptr;
        QWidget               * m_regHeaderWidget = nullptr;
        QLineEdit             * m_regNameEdit = nullptr;
        QLineEdit             * m_regOffsetEdit = nullptr;
        QLineEdit             * m_regDescEdit = nullptr;
        RegMapTreeItem        * m_currentRegItem = nullptr;

        // Block View Components
        QWidget               * m_blockHeaderWidget = nullptr;
        QLineEdit             * m_blkNameEdit = nullptr;
        QLineEdit             * m_blkOffsetEdit = nullptr;
        QLineEdit             * m_blkDescEdit = nullptr;
        RegMapTreeItem        * m_currentBlkItem = nullptr;
        BlockMemoryMapWidget  * m_blockMemoryMapWidget = nullptr;
        QScrollArea           * m_mapScrollArea = nullptr;

        QLineEdit             * m_searchEdit = nullptr;
        QLabel                * m_searchCountLabel = nullptr;
        QSplitter             * m_splitter = nullptr;
        QUndoStack            * m_undoStack = nullptr;

        QString                 m_rmap_filename;
        QString                 m_default_filename;
        QString                 m_active_folder;
        QString                 m_default_window_title;
        bool                    m_is_regmap_modified = false;

        void fileNew(void);
        void regmap_modified(void);
        void regmap_notModified(void);
        void connectModelSignals(void);
        void connectFieldsTableSignals(void);
        void updatePaneVisibility(void);
        void btnFileNew(void);
        void btnFileOpen(void);
        void btnFileClose(void);
        bool btnFileSave(void);
        bool btnFileSaveAs(void);
        void btnFileReload(void);
        void btnDeleteItem(void);
        void btnCheck(void);
        void btnExport(void);
        void btnQuitButton(void);
        void btnConfig(void);
        void btnPreferences(void);
        void btnAbout(void);
        void btnKeyBindings(void);
        void updateFieldsTable(const QModelIndex &current, const QModelIndex &previous);
        void updateBlockView(RegMapTreeItem *blkItem);
        void navigateToRegister(int childRow, RegMapTreeItem *regItem);
        void showTreeContextMenu(const QPoint &pos);
        void duplicateItem(const QModelIndex &index);
        void duplicateSelectedRegister(void);
        void onSearchTextChanged(const QString &text);
        void onToggleColorBlindMode(bool checked);
        void setupThemeMenu(void);
        void rebuildThemeMenu(void);
        void setupColorBlindMenu(void);
        void rebuildColorBlindMenu(void);
        void setupLanguageMenu(void);
        void updateDynamicTranslations(void);

        QMenu                 * m_themeMenu{nullptr};
        QActionGroup          * m_themeActionGroup{nullptr};
        QMenu                 * m_colorBlindMenu{nullptr};
        QActionGroup          * m_colorBlindActionGroup{nullptr};
        QMenu                 * m_languageMenu{nullptr};
        QActionGroup          * m_languageActionGroup{nullptr};
};

#endif // REGMAPWINDOW_HPP
