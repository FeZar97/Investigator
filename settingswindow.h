#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>
#include <QFileDialog>

#include "stylehelper.h"
#include "investigatororchestartor.h"

namespace Ui {
class SettingsWindow;
}

class SettingsWindow: public QDialog {

    Q_OBJECT

    int m_currentOpenTab;
    bool *m_isUiLocked;

public:
    explicit SettingsWindow(QWidget *parent = nullptr, InvestigatorOrchestartor *investigator = nullptr,
                            bool *isUiLocked = nullptr);
    ~SettingsWindow();

    int currentOpenTab() {
        return m_currentOpenTab;
    }
    void setCurrentOpenTab(int currentOpenTab) {
        m_currentOpenTab = currentOpenTab;
    }

    void updateUi();

private slots:
    void on_watchDirButton_clicked();
    void on_tempDirButton_clicked();
    void on_cleanDirButton_clicked();
    void on_dangerousDirButton_clicked();

    void on_avsExecFileButton_clicked();
    void on_thresholdFilesNbSB_valueChanged(int thresholdFilesNb);
    void on_thresholdFilesSizeSB_valueChanged(int thresholdFilesSize);
    void on_thresholdFilesSizeUnitCB_currentIndexChanged(int thresholdFilesUnitIdx);
    void on_threadsNumberSB_valueChanged(int workersNb);

    // внешний обработчик
    void on_externalHandlerFileButton_clicked();
    void on_externalHandlerFileCB_clicked(bool checked);

    // адрес сислог
    void on_syslogAddressLE_editingFinished();

    // изменение индекса таба
    void on_settingsTabWidget_currentChanged(int index);

    // перезапуск http сервера
    void on_restartHttpServerButton_clicked();

private:
    Ui::SettingsWindow *ui;
    bool *m_lockUi{nullptr};
    InvestigatorOrchestartor *m_investigator;

signals:
    void restartHttpServer();
};

#endif // SETTINGSWINDOW_H
