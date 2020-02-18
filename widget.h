#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QIcon>
#include <QMenu>
#include <QSettings>

#include "settings.h"
#include "statistics.h"
#include "distributor.h"

namespace Ui {
    class Widget;
}

class Widget : public QWidget {

    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

    void log(QString s, MSG_CATEGORY cat);
    void updateUi();
    void startProcess(QString path, QStringList args);
    void parseResultOfProcess();
    void saveReport(QString report, unsigned long long reportIdx);
    void startExternalHandler(QString path, QStringList args);

private slots:
    void on_startButton_clicked();
    void on_stopButton_clicked();
    void on_settingsButton_clicked();
    void on_statisticButton_clicked();
    void on_clearButton_clicked();

private:
    Ui::Widget *ui;

    QSettings m_settings;
    QTextCodec *m_win1251Codec;
    QProcess m_process;

    Investigator *m_investigator;
    Distributor *m_distributor;
    QThread m_workThread;

    Settings *m_settingsWindow;
    Statistics *m_statisticWindow;

signals:
    void parseReport(QString report);
    void startWork();
    void stopWork();
};

#endif // WIDGET_H
