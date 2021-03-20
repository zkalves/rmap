#ifndef REGCONFIGWINDOW_HPP
#define REGCONFIGWINDOW_HPP

#include <QDialog>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QString>
#include <QList>
#include <utility>
#include "rmap.pb.h"
#include "ui_config.h"

namespace Ui {
    class config;
}

class RegConfigWindow : public QDialog, private Ui::config
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(RegConfigWindow)

    public:
        explicit RegConfigWindow(QWidget *parent = nullptr);
        ~RegConfigWindow() override = default;

        protormap::Config* serialize(void);
        void deserialize(const protormap::Config &config);

        void setRegisterWidth(uint32_t width);
        void setProjectName(const QString &name);
        void setProjectVersion(const QString &version);
        uint32_t registerWidth() const;
        QString projectName() const;
        QString projectVersion() const;

        void setBaseDir(const QString &baseDir);
        QString baseDir() const;

        void saveWindowStateToSettings();
        void restoreWindowStateFromSettings();

    public slots:
        void addTemplateRow(const QString &tmpl = "", const QString &out = "");
        void addParameterRow(const QString &key = "", const QString &val = "");
        void accept(void) override;
        void reject(void) override;

    protected:
        void closeEvent(QCloseEvent *event) override;
        void resizeEvent(QResizeEvent *event) override;
        void moveEvent(QMoveEvent *event) override;

    private slots:
        void onBrowseTemplateFolder();
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
        QString m_templateFolder;
        QString m_outputFolder;
        QString m_projectName;
        QString m_projectVersion;
        bool m_strictValidation = true;
        uint32_t m_regWidth = 32;
        QList<std::pair<QString, QString>> m_templateOutputs;
        QList<std::pair<QString, QString>> m_customParameters;

        void updateUiFromState();
        void saveStateFromUi();
};

#endif // REGCONFIGWINDOW_HPP
