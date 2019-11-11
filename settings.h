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
    explicit Settings(QWidget *parent = nullptr, Distributor* distributor = nullptr, QByteArray geometry = nullptr, bool visible = false);
    ~Settings();

    int m_kasperVolUnit{0};
    int m_drwebVolUnit{0};

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
    void on_kasperMaxQueueVolSB_valueChanged(double kasperMaxQueueVol);
    void on_drwebMaxQueueVolSB_valueChanged(double drwebMaxQueueVol);
    void on_kasperMaxQueueVolCB_currentIndexChanged(int index);
    void on_drwebMaxQueueVolCB_currentIndexChanged(int index);

    void on_clearWatchDirButton_clicked();
    void on_clearTempDirButton_clicked();
    void on_clearCleanDirButton_clicked();
    void on_clearDangerDirButton_clicked();

private:
    Ui::Settings *ui;
    Distributor* m_distributor;
};

#endif // SETTINGS_H
