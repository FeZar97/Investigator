#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QSettings>
#include <QProcess>
#include <QThread>
#include <QDebug>
#include <QtDebug>

#include "settings.h"
#include "statistics.h"
#include "distributor.h"
#include "httplistener.h"
#include "httprequestmapper.h"

namespace Ui {
    class Widget;
}

class Widget : public QWidget {

    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

    void log(LOG_CATEGORY cat = DEBUG, QString s = "");
    void updateUi();
    void executeProcess(QString path, QStringList args);
    void saveReport(QString report = "", QString baseName = "");
    void startExternalHandler(QString path, QStringList args);
    void closeEvent(QCloseEvent *event) override;
    void startHttpServer();
    void turnOff(int code);
    void processStarted();
    void restartProcess(); // рестарт процесса проверки после таймаута
    void processErrorOccured(QProcess::ProcessError er); // обработка ошибок процесса проверки

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
    int m_daysVersion{1}; // идентификатор версии в пределах одного дня

    QSettings m_settings;
    QProcess m_process;

    Investigator *m_investigator;
    Distributor *m_distributor;

    QThread m_workThread;
    QThread m_distributionThread;

    Settings *m_settingsWindow;
    Statistics *m_statisticWindow;

    QDateTime m_logFileOpenTime;

    QTimer minuteTimer;
    QTimer m_reportTimer;

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
    void parseReport();
    void startWatchDirEye();
    void stopWatchDirEye();
    void investigatorMoveFiles(QString sourceDir, QString destinationDir, int limit);
};

#endif // WIDGET_H
