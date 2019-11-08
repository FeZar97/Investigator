#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QIcon>
#include <QMenu>
#include <QSettings>

#include "settings.h"
#include "statistics.h"

/*
 * TODO
 * 0. default temp path
 * 1. start/stop buttons
 * 2. settings window
 * 3. auto-creating temp subdirs on every iterations
 * 4. behavior with old report, that exists when program started
 * 5. individual window to setting and statistics
 */

namespace Ui {
    class Widget;
}

class Widget : public QWidget {

    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

    void log(const QString &s);
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
