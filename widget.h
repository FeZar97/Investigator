#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QIcon>
#include <QMenu>
#include <QSettings>

#include "settings.h"
#include "statistics.h"

namespace Ui {
    class Widget;
}

class Widget : public QWidget {

    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

    void log(QString s);
    void updateUi();

private slots:
    void on_startButton_clicked();
    void on_stopButton_clicked();
    void on_settingsButton_clicked();
    void on_statisticButton_clicked();
    void on_clearButton_clicked();

private:
    Ui::Widget *ui;

    Distributor distributor;
    QSettings settings;
    QThread workThread;

    Settings *settingsWindow;
    Statistics *statisticWindow;
};

#endif // WIDGET_H
