#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QFileDialog>

#include "distributor.h"


namespace Ui {
    class Settings;
}

class Settings : public QDialog {

    Q_OBJECT

public:
    explicit Settings(QWidget *parent = nullptr, Distributor* distributor = nullptr, QByteArray geometry = nullptr, bool visible = false);
    ~Settings();

    void updateUi();

private slots:
    void on_watchDirButton_clicked();
    void on_tempDirButton_clicked();
    void on_cleanDirButton_clicked();
    void on_dangerousDirButton_clicked();
    void on_kasperFileButton_clicked();
    void on_drwebFileButton_clicked();
    void on_kasperCB_clicked(bool isUsed);
    void on_drwebCB_clicked(bool isUsed);

private:
    Ui::Settings *ui;
    Distributor* m_distributor;
};

#endif // SETTINGS_H