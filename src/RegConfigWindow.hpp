/* This file is part of rmap.
 *
 * rmap is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * rmap is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rmap.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#ifndef REGCONFIGWINDOW_HPP
#define REGCONFIGWINDOW_HPP

#include <QDialog>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QString>
#include <QStringList>
#include <QList>
#include <utility>
#include "rmap.pb.h"
#include "ui_config.h"

namespace Ui {
    class config;
}

struct TemplateEntry {
    bool enabled = true;
    QString templateFile;
    QString outputFile;
};

class RegConfigWindow : public QDialog, private Ui::config
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(RegConfigWindow)

    public:
        explicit RegConfigWindow(QWidget *parent = nullptr);
        ~RegConfigWindow() override;

        protormap::Config* serialize(void);
        void deserialize(const protormap::Config &config);

        void setRegisterWidth(uint32_t width);
        void setProjectName(const QString &name);
        void setProjectVersion(const QString &version);
        uint32_t registerWidth() const;
        QString projectName() const;
        QString projectVersion() const;
        void setHwPrecedence(bool precedence);
        bool hwPrecedence() const;

        void setPythonScript(const QString &script);
        QString pythonScript() const;
        bool isPythonScriptEnabled() const;

        void setBaseDir(const QString &baseDir);
        QString baseDir() const;

        void setTemplateFolders(const QStringList &folders);
        QStringList templateFolders() const noexcept;

        void saveWindowStateToSettings();
        void restoreWindowStateFromSettings();

    public slots:
        void addTemplateRow(bool enabled, const QString &tmpl = "", const QString &out = "");
        void addTemplateRow(const QString &tmpl = "", const QString &out = "");
        void addParameterRow(const QString &key = "", const QString &val = "");
        void addTemplateFolder(const QString &folder);
        void scanTemplateFolders();
        void onSyncDefaultOutputs();
        void onOutputFolderEdited(const QString &newFolder);
        void accept(void) override;
        void reject(void) override;

    protected:
        void showEvent(QShowEvent *event) override;
        void closeEvent(QCloseEvent *event) override;
        void resizeEvent(QResizeEvent *event) override;
        void moveEvent(QMoveEvent *event) override;
        void changeEvent(QEvent *event) override;

    private slots:
        void onAddTemplateFolder();
        void onRemoveTemplateFolder();
        void onScanTemplates();
        void onEnableAll();
        void onDisableAll();
        void onBrowseOutputFolder();
        void onBrowsePythonScript();
        void onAddTemplateFiles();
        void onAddCustomRow();
        void onBrowseTemplate();
        void onBrowseOutputFile();
        void onBrowseOutputFolderItem();
        void onRemoveSelected();
        void onClearAll();
        void onAddParameterRow();
        void onRemoveParameterRow();

    private:
        QString m_baseDir;
        QString m_pythonScript;
        QStringList m_templateFolders;
        QString m_outputFolder;
        QString m_projectName;
        QString m_projectVersion;
        bool m_strictValidation = true;
        bool m_hwPrecedence = true;
        bool m_firstShown = true;
        uint32_t m_regWidth = 32;
        QList<TemplateEntry> m_templateOutputs;
        QList<std::pair<QString, QString>> m_customParameters;

        void updateUiFromState();
        void saveStateFromUi();
        QString computeDefaultOutputPath(const QString &tmplRelPath, const QString &outFolderOverride = QString());
};

#endif // REGCONFIGWINDOW_HPP
