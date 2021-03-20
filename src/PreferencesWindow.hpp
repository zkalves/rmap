#ifndef PREFERENCESWINDOW_HPP
#define PREFERENCESWINDOW_HPP

#include <QDialog>
#include <QString>
#include "ui_preferences.h"

namespace Ui {
    class preferences;
}

class PreferencesWindow : public QDialog, private Ui::preferences
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(PreferencesWindow)

public:
    explicit PreferencesWindow(QWidget *parent = nullptr);
    ~PreferencesWindow() override = default;

    void setColourScheme(const QString &scheme);
    QString colourScheme() const;
    QString colorScheme() const { return colourScheme(); }

    void setColourBlindMode(bool enabled);
    bool isColourBlindMode() const;
    bool colorBlindMode() const { return isColourBlindMode(); }

    void saveWindowStateToSettings();
    void restoreWindowStateFromSettings();

public slots:
    void accept() override;
    void reject() override;
    void apply();

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void moveEvent(QMoveEvent *event) override;

private:
    QString m_colourScheme = QStringLiteral("solarized8");
    bool m_colourBlindMode = false;

    void updateUiFromState();
    void saveStateFromUi();
};

#endif // PREFERENCESWINDOW_HPP
