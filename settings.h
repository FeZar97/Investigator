#ifndef SETTINGS_H
#define SETTINGS_H

#include "investigator.h"

namespace Ui {
    class Settings;
}

class Settings : public QDialog {

    Q_OBJECT

public:
    explicit Settings(QWidget *parent = nullptr, Investigator* investigator = nullptr, QByteArray geometry = nullptr, bool* lockUi = nullptr, int currentTabIdx = 0);
    ~Settings();

    int m_currentTab;

    void updateUi();
    int getVolUnits();

private slots:
    void on_watchDirButton_clicked();
    void on_tempDirButton_clicked();
    void on_cleanDirButton_clicked();
    void on_dangerousDirButton_clicked();
    void on_logsDirButton_clicked();

    void on_avFileButton_clicked();
    void on_avMaxQueueSizeSB_valueChanged(int size);
    void on_avMaxQueueVolSB_valueChanged(double maxQueueVol);
    void on_avMaxQueueVolUnitCB_currentIndexChanged(int unitIdx);

    void on_infectActionCB_currentIndexChanged(int actionIdx);

    void on_externalHandlerFileButton_clicked();
    void on_externalHandlerFileCB_clicked(bool checked);

    void on_saveAVSReportsCB_clicked(bool checked);
    void on_reportsDirButton_clicked();

    void on_syslogCB_clicked(bool checked);
    void on_syslogLevelCB_currentIndexChanged(int level);
    void on_syslogAddressLE_editingFinished();
    void on_syslogAddressLE_textChanged(const QString &newAddress);

    void on_httpServerCB_clicked(bool checked);
    void on_httpServerAddressLE_editingFinished();
    void on_httpServerAddressLE_textChanged(const QString &newAddress);

    void on_settingsTabWidget_currentChanged(int index);

private:
    Ui::Settings *ui;
    bool* m_lockUi{nullptr};
    Investigator* m_investigator;

    QString getLEStyleSheet(bool isCorrect);

signals:
    void log(QString s, LOG_CATEGORY cat);
    void restartWatching();
    void clearDir(QString dirPath);
    void s_updateUi();
    void startHttpServer();
};

#endif // SETTINGS_H
