#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDir>
#include <QFileDialog>
#include <QSettings>
#include <QProgressBar>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QLabel>

#include "httplistener.h"
#include "httprequestmapper.h"

#include "settingswindow.h"
#include "statisticswindow.h"

#include "investigatororchestartor.h"

#include "../FeZarSource/aboutprogramwidget.h"

/* CHANGELOG 2.5
 */

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget: public QWidget {
    Q_OBJECT

private:
    Ui::Widget *ui;

    QSettings m_settings;

    bool m_isUiLocked{false}; // флаг блокировки интерфейса

    // индикаторы работы
    QVector<QLabel *> m_workerIndicators;
    QVector<QPixmap> m_circles;
    void createIndicators();

    // трей
    QSystemTrayIcon *m_trayIcon;
    void createTrayIcon(); // создание иконки трея

    // investigator
    void createInvestigator();

    // окно статистики
    StatisticsWindow *m_statisticsWindow;
    void createStatisticsWindow();

    // окно настроек
    SettingsWindow *m_settingsWindow;
    void createSettingsWindow();

    // http сервер
    HttpListener *m_httpServer{nullptr};

    // таймер для сохранения настроек
    QTimer *m_saveSettingsTimer;
    void createSaveSettingsTimer();

    // соединение объектов
    void connectObjects();

    // завершение работы программы
    void closeProgram();

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void uiLog(QString message);

    void closeEvent(QCloseEvent *event) override;
    void hideEvent(QHideEvent *event) override;

    InvestigatorOrchestartor *m_investigator;
    QThread m_investigatorThread;

    void startHttpServer();

    void clearLog();

    void updateIndicators(int workerId, bool state);

private slots:
    void on_startButton_clicked();
    void on_stopButton_clicked();
    void updateUi();

    void restoreSettings();
    void saveSettings();

    void on_clearButton_clicked();

    void on_settingsButton_clicked();
    void on_statisticsButton_clicked();

    void on_lockUiButton_clicked();

signals:
    void getInitialAvsScan(bool needStart);
    void start();
    void stop();
    void log(QString message, int logCtx);
    void updateSettingsWindow();
    void updateStatisticWindow();
};
#endif // WIDGET_H
