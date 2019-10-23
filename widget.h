#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QFileDialog>
#include <QTime>
#include <QIcon>
#include <QMenu>
#include <QDebug>
#include <QSettings>

#include "distributor.h"


namespace Ui {
    class Widget;
}

class Widget : public QWidget {

    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_watchDirButton_clicked();
    void on_tempDirButton_clicked();
    void on_cleanDirButton_clicked();
    void on_dangerousDirButton_clicked();

    void on_kasperFileButton_clicked();
    void on_drwebFileButton_clicked();

    void on_kasperCB_clicked(bool isUsed);
    void on_drwebCB_clicked(bool isUsed);

    void on_clearButton_clicked();

    void log(const QString &s);
    void updateUi();

private:
    Ui::Widget *ui;

    QSettings settings;
    Distributor distributor;
    QThread workThread;
};

#endif // WIDGET_H
