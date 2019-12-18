#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QIcon>
#include <QMenu>
#include <QSettings>

#include "settings.h"
#include "statistics.h"

/* TODO
 *      default temp path (04.11)
 *      start/stop buttons (07.11)
 *      settings window (07.11)
 *      auto-creating temp subdirs on every iterations (03.11)
 *      behavior with old report, that exists when program started (-)
 *      individual window to setting and statistics (07.11)
 *      fix multiple accounting same danger files (08.11)
 *      add colorisation table when queue size exceed some threshold (08.11 )
 *      ability to clear temp dirs, report dir, and clean\danger dir (11.11)
 *      add ability to set queue threshold with max volume of dirs (11.11)
 *      deny editing statistic table (11.11)
 *      add warning window, when user clicked on dir clean buttons (11.11)
 */

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
