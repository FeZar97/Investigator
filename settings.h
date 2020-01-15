#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>

#include "distributor.h"

namespace Ui {
    class Settings;
}

class Settings : public QDialog {

    Q_OBJECT

public:
    explicit Settings(QWidget *parent = nullptr, Distributor* distributor = nullptr, QByteArray geometry = nullptr);
    ~Settings();

    void updateUi();
    int getVolUnits(AV av);

private slots:
    void on_watchDirButton_clicked();
    void on_tempDirButton_clicked();
    void on_cleanDirButton_clicked();
    void on_dangerousDirButton_clicked();
    void on_kasperFileButton_clicked();
    void on_drwebFileButton_clicked();
    void on_kasperCB_clicked(bool isUsed);
    void on_drwebCB_clicked(bool isUsed);
    void on_kasperMaxQueueSizeSB_valueChanged(int size);
    void on_drwebMaxQueueSizeSB_valueChanged(int size);
    void on_kasperMaxQueueVolSB_valueChanged(double kasperMaxQueueVolMb);
    void on_drwebMaxQueueVolSB_valueChanged(double drwebMaxQueueVolMb);
    void on_kasperMaxQueueVolUnitCB_currentIndexChanged(int unitIdx);
    void on_drwebMaxQueueVolUnitCB_currentIndexChanged(int unitIdx);

    void on_clearWatchDirButton_clicked();
    void on_clearTempDirButton_clicked();
    void on_clearCleanDirButton_clicked();
    void on_clearDangerDirButton_clicked();

    void on_upgradeAVButton_clicked();

private:
    Ui::Settings *ui;
    Distributor* m_distributor;

signals:
    void clearDir(QString dirPath);
    void updateAV(QString updaterPath);
};

#endif // SETTINGS_H
