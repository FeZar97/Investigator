#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>

#include "investigator.h"

namespace Ui {
    class Settings;
}

class Settings : public QDialog {

    Q_OBJECT

public:
    explicit Settings(QWidget *parent = nullptr, Investigator* investigator = nullptr, QByteArray geometry = nullptr, bool* lockUi = nullptr);
    ~Settings();

    void updateUi();
    int getVolUnits();

private slots:
    void on_watchDirButton_clicked();
    void on_tempDirButton_clicked();
    void on_cleanDirButton_clicked();
    void on_dangerousDirButton_clicked();

    void on_avFileButton_clicked();
    void on_avMaxQueueSizeSB_valueChanged(int size);
    void on_avMaxQueueVolSB_valueChanged(double maxQueueVolMb);
    void on_avMaxQueueVolUnitCB_currentIndexChanged(int unitIdx);

    void on_clearWatchDirButton_clicked();
    void on_clearTempDirButton_clicked();
    void on_clearCleanDirButton_clicked();
    void on_clearDangerDirButton_clicked();

    void on_infectActionCB_currentIndexChanged(int actionIdx);

    void on_externalHandlerFileButton_clicked();
    void on_externalHandlerFileCB_clicked(bool checked);

    void on_syslogCB_clicked(bool checked);
    void on_syslogAddressLE_textChanged(const QString &arg1);

private:
    Ui::Settings *ui;
    bool* m_lockUi{nullptr};
    Investigator* m_investigator;

signals:
    void log(QString s, MSG_CATEGORY cat);
    void clearDir(QString dirPath);
    void s_updateUi();
};

#endif // SETTINGS_H
