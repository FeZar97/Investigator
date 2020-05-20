#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QSettings>
#include <QProcess>
#include <QThread>
#include <QDebug>

#include "settings.h"
#include "statistics.h"
#include "distributor.h"
#include "httplistener.h"
#include "httpjsonresponder.h"

namespace Ui {
    class Widget;
}

class Widget : public QWidget {

    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

    void log(QString s = "", LOG_CATEGORY cat = DEBUG);
    void updateUi();
    void executeProcess(QString path, QStringList args);
    void parseResultOfProcess();
    void saveReport(QString report = "", QString baseName = "");
    void startExternalHandler(QString path, QStringList args);
    void closeEvent(QCloseEvent *event) override;
    void startHttpServer();

private slots:
    void on_startButton_clicked();
    void on_stopButton_clicked();
    void on_settingsButton_clicked();
    void on_statisticButton_clicked();
    void on_clearButton_clicked();
    void on_lockButton_clicked();

private:
    Ui::Widget *ui;

    bool m_lockUi{false}; // флаг блокировки интерфейса

    QSettings m_settings;
    QTextCodec *m_win1251Codec;
    QProcess m_process;

    Investigator *m_investigator;
    Distributor *m_distributor;

    QThread m_workThread;
    QThread m_distributionThread;

    Settings *m_settingsWindow;
    Statistics *m_statisticWindow;

    QDateTime m_logFileOpenTime;

    QTimer minuteTimer;

    // http сервер
    HttpListener *m_httpServer{nullptr};

    // настройки программы
    void restoreSettings();
    void saveSettings();

    // сигнально-слотовые соединения
    void connectObjects();

    // первночалаьное сканирование для получения версий баз
    void getInitialAvsScan();

    // перенос старых файлов из временной директории во входную директорию
    void moveOldFilesToInputDir();

    void minuteUpdate();

signals:
    void parseReport(QString report);
    void startWatchDirEye();
    void stopWatchDirEye();
};

#endif // WIDGET_H
