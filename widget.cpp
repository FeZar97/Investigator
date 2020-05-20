#include "widget.h"
#include "ui_widget.h"

#include <windows.h>

Widget::Widget(QWidget *parent): QWidget(parent),
                                 ui(new Ui::Widget),
                                 m_settings("FeZar97", "Investigator"),
                                 m_win1251Codec(QTextCodec::codecForName("Windows-1251")) {

    ui->setupUi(this);

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD size;
    size.X = 128;
    size.Y = 1536;
    SetConsoleScreenBufferSize(hOut, size);

    minuteTimer.setInterval(60000);

    setLayout(ui->mainLayout);
    setWindowTitle(QString("Investigator %1 #%2").arg(VERSION).arg(PATCH_IDENTIFICATOR));
    restoreGeometry(m_settings.value("geometry").toByteArray());

    m_investigator    = new Investigator();
    m_distributor     = new Distributor(nullptr, m_investigator);
    m_settingsWindow  = new Settings(this,   m_investigator, m_settings.value("settingsWinGeometry").toByteArray(), &m_lockUi, m_settings.value("currentSettingsTab", 0).toInt());
    m_statisticWindow = new Statistics(this, m_investigator, m_settings.value("statisticWinGeometry").toByteArray(), &m_lockUi);

    restoreSettings();
    connectObjects();

    // перенос объектов в потоки
    m_investigator->moveToThread(&m_workThread);
    m_distributor->moveToThread(&m_distributionThread);

    // старт потоков
    m_workThread.start();
    m_distributionThread.start();

    startHttpServer();

    log("Программа запущена.", LOG_CATEGORY(GUI + DEBUG));

    getInitialAvsScan();

    m_settingsWindow->setVisible(true);
    m_statisticWindow->setVisible(true);
}

Widget::~Widget() {

    saveSettings();

    if(m_investigator->m_isInProcess)
        m_process.kill();

    emit stopWatchDirEye();

    log("Программа завершила работу.", LOG_CATEGORY(DEBUG));

    m_distributionThread.quit();
    m_distributionThread.wait();

    m_workThread.quit();
    m_workThread.wait();

    delete m_settingsWindow;
    delete m_statisticWindow;
    delete m_distributor;
    delete m_investigator;

    delete ui;
}

/* логирование */
void Widget::log(QString s, LOG_CATEGORY cat) {

    if(cat & DEBUG_ROW) {
        m_investigator->m_processInfo = s;
    }

    if(cat & GUI) {
        ui->logPTE->appendPlainText(formattedCurrentDateTime() + " " + s);
    }

    if(cat & DEBUG) {
        // если директории для логов нет, но есть временная папка, то пытаемся создать ее внутри временной папки программы
        if( ( !QDir(m_investigator->m_logsDir).exists() ||
              !m_investigator->m_logsDir.isEmpty()
             ) && QDir(m_investigator->m_investigatorDir).exists()) {

            // создание папки
            QDir().mkpath("");
            if(m_investigator->m_logsDir.isEmpty()) {
                m_investigator->m_logsDir    = m_investigator->m_investigatorDir + "/" + LOGS_DIR_NAME;
                QDir().mkpath(m_investigator->m_logsDir);
            }

            // сохранение отчета
            QString txt = QString("%1 %2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")).arg(s),
                    dirPath = m_investigator->m_logsDir + "/" + QDate().currentDate().toString("yy-MM-dd") + "/";
            QDir().mkdir(dirPath);

            QFile outFile(QString("%1KAP-log-%2-%3.log").arg(dirPath).arg(QDateTime::currentDateTime().toString("hh")).arg(VERSION));
            if(!outFile.isOpen()) {
                outFile.open(QIODevice::WriteOnly | QIODevice::Append);
                m_logFileOpenTime = QDateTime::currentDateTime();
            }

            QTextStream ts(&outFile);
            ts << txt << endl;

            if(m_logFileOpenTime.secsTo(QDateTime().currentDateTime()) > LOG_FILE_REOPEN_TIME) {
                outFile.close();
            }
        }


    }

    if(cat & m_investigator->m_syslogPriority) {
        m_investigator->sendSyslogMessage(s);
    }

    // если программа не запущена, то надо принудительно обновить интерфейс
    if(!m_investigator->m_isWorking) {
        updateUi();
    }
}

/* обновление GUI */
void Widget::updateUi() {
    ui->startButton->setEnabled(!m_lockUi && !m_investigator->m_isWorking);
    ui->stopButton->setEnabled(!m_lockUi  &&  m_investigator->m_isWorking);
    ui->clearButton->setEnabled(!m_lockUi);
    ui->lockButton->setIcon(m_lockUi ? QIcon(":/img/UNLOCK.jpg") : QIcon(":/img/LOCK.jpg"));

    m_settingsWindow->updateUi();
    m_statisticWindow->updateUi();
}

/* вызов процесса path с аргументами args */
void Widget::executeProcess(QString path, QStringList args) {
    if(m_investigator->m_isWorking) {
        m_investigator->m_lastProcessStartTime = QDateTime::currentDateTime();
        log("Процесс проверки...", LOG_CATEGORY(DEBUG_ROW + DEBUG));
        m_process.start(path, args);
    }
}

void Widget::parseResultOfProcess() {
    // вначале прочитать из stdOut, потом уже закрывать процесс
    QString report = m_win1251Codec->toUnicode(m_process.readAllStandardOutput());
    emit parseReport(report);

    m_process.close();
}

/* сохранение отчета АВС */
void Widget::saveReport(QString report, QString baseName) {
    if(!QDir(m_investigator->m_reportsDir).exists()) {
        QDir().mkdir(m_investigator->m_reportsDir);
    }

    QFile outFile(m_investigator->getReportFileName(baseName));
    outFile.open(QIODevice::WriteOnly);
    QTextStream ts(&outFile);
    ts << report << endl;
    outFile.close();
}

/* вызов внешнего обработчика path с аргументами args */
void Widget::startExternalHandler(QString path, QStringList args) {
    log(QString("Вызов внешнего обработчика '%1' с параметрами: %2").arg(path).arg(entryListToString(args)), LOG_CATEGORY(DEBUG));
    if(QFile(path).exists()) {
        QProcess::execute(path, args);
    } else {
        log(QString("Не удалось найти файл внешнего обработчика."), LOG_CATEGORY(DEBUG));
    }
}

/* закрытие программы через кнопку */
void Widget::closeEvent(QCloseEvent *event) {
    if(QMessageBox::warning(this,
                            QString("Подтвердите действие"),
                            QString("Вы действительно хотите завершить работу программы?"),
                            QString("Да"), QString("Нет"), QString(),
                            1) == 0) {
        event->accept();
    } else {
        event->ignore();
    }
}

/* запуск http сервера */
void Widget::startHttpServer() {

    if(m_httpServer) {
        m_httpServer->close();
        delete m_httpServer;
        m_httpServer = nullptr;
        log(QString("Http сервер остановлен."), LOG_CATEGORY(DEBUG));
    }

    if(!m_investigator->m_useHttpServer)
        return;

    // Start the HTTP server
    // m_httpServer = new HttpListener(m_investigator->m_httpServerPort,
    //                                 m_investigator->m_httpServerIp.toString(),
    //                                 new HttpJsonResponder(this, m_investigator),
    //                                 this);
    log(QString("Http сервер запущен."), LOG_CATEGORY(DEBUG));
}

/* запуск слежения за каталогом для мониторинга */
void Widget::on_startButton_clicked() {

    // если настройки корректны, запуск работы
    if(m_investigator->checkAvParams()) {
        m_investigator->m_isWorking = true;
        m_investigator->clearStatistic();

        // если есть старые непроверенные файлы
        moveOldFilesToInputDir();

        log(QString("Запущено слежение за директорией %1.").arg(m_investigator->m_watchDir), LOG_CATEGORY(DEBUG + GUI));
        emit startWatchDirEye();
    } else {
        log(QString("Не удалось начать слежение за директорией %1.").arg(m_investigator->m_watchDir), LOG_CATEGORY(DEBUG + GUI));
        m_investigator->m_isWorking = false;
    }
}

/* остановка слежения за каталогом для мониторинга */
void Widget::on_stopButton_clicked() {
    if(QMessageBox::warning(this,
                            QString("Подтвердите действие"),
                            QString("Вы действительно хотите остановить слежение за каталогом?"),
                            QString("Да"), QString("Нет"), QString(),
                            1) == 0) {
        saveSettings();

        m_investigator->m_isWorking = false;

        // m_process.close();
        m_process.kill();
        m_investigator->m_isInProcess = false;

        m_investigator->m_endTime = QDateTime::currentDateTime();

        emit stopWatchDirEye();

        m_investigator->collectStatistics();

        log(QString("Слежение за директорией %1 остановлено.").arg(m_investigator->m_watchDir), LOG_CATEGORY(DEBUG + GUI));
    }
}

void Widget::on_settingsButton_clicked() {
    m_settingsWindow->setVisible(!m_settingsWindow->isVisible());
}

void Widget::on_statisticButton_clicked() {
    m_statisticWindow->setVisible(!m_statisticWindow->isVisible());
}

/* очистка лога в главном окне */
void Widget::on_clearButton_clicked() {
    log("Лог очищен.", DEBUG);
    ui->logPTE->clear();
}

/* блокировка интерфейса */
void Widget::on_lockButton_clicked() {
    m_lockUi = !m_lockUi;
    updateUi();
}

// -----------------------------------------------------------------------------------------------------------------------

void Widget::restoreSettings() {

    // настройки интерфейса
    m_investigator->m_watchDir             = m_settings.value("watchDir",        "").toString();
    m_investigator->m_investigatorDir      = m_settings.value("investigatorDir", "").toString();
    m_investigator->m_cleanDir             = m_settings.value("cleanDir",        "").toString();
    m_investigator->m_dangerDir            = m_settings.value("dangerDir",       "").toString();
    m_investigator->m_logsDir              = m_settings.value("logsDir",         "").toString();

    m_investigator->m_avPath               = m_settings.value("avFilePath", "C:/Program Files/Primetech/M-52/AVSFileConsoleScan.exe").toString();
    m_investigator->m_infectedFileAction   = ACTION_TYPE(m_settings.value("infectAction", MOVE_TO_DIR).toInt());
    m_investigator->m_maxQueueSize         = m_settings.value("avMaxQueueSize", 20).toInt();
    m_investigator->m_maxQueueVol          = m_settings.value("avMaxQueueVol", 2).toDouble();
    m_investigator->m_maxQueueVolUnit      = m_settings.value("avVolUnit", 1).toInt();
    m_investigator->m_saveAvsReports       = m_settings.value("saveAVSReports", false).toBool();
    m_investigator->m_reportsDir           = m_settings.value("reportsDir", "").toString();
    m_investigator->m_useExternalHandler   = m_settings.value("useExternalHandler", false).toBool();
    m_investigator->m_externalHandlerPath  = m_settings.value("externalHandlerPath", "").toString();

    m_investigator->m_useSyslog            = m_settings.value("useSyslog", true).toBool();
    m_investigator->m_syslogAddress        = m_settings.value("syslogAddress", "199.199.100.120:514").toString();
    m_investigator->m_syslogPriority       = LOG_CATEGORY(m_settings.value("syslogPriority", GUI).toInt());
    m_investigator->m_useHttpServer        = m_settings.value("useHttpServer", true).toBool();
    m_investigator->m_httpServerAddress    = m_settings.value("httpServerAddress", "0.0.0.0:" + QString::number(HTTP_PORT)).toString();

    // статистика прошлой сессии
    m_investigator->m_infectedFilesNb      = m_settings.value("infectedFilesNb", 0).toInt();
    m_investigator->m_scanningErrorFilesNb = m_settings.value("scanningErrorFilesNb", 0).toInt();
    m_investigator->m_processedFilesNb     = m_settings.value("processedFilesNb", 0).toInt();
    m_investigator->m_processedFilesSizeMb = m_settings.value("processedFilesSizeMb", 0).toDouble();
    m_investigator->m_startTime            = m_settings.value("startTime", QDateTime::currentDateTime()).toDateTime();

    // дополнительное конфигурирование после восстановления
    m_investigator->configureDirs();
    m_investigator->clearStatistic();
    m_investigator->checkSyslogAddress();
    m_investigator->checkHttpAddress();
}

void Widget::saveSettings() {

    // настройки интерфейса
    m_settings.setValue("geometry",               saveGeometry());

    m_settings.setValue("settingsWinGeometry",    m_settingsWindow->saveGeometry());
    m_settings.setValue("statisticWinGeometry",   m_statisticWindow->saveGeometry());

    m_settings.setValue("watchDir",               m_investigator->m_watchDir);
    m_settings.setValue("investigatorDir",        m_investigator->m_investigatorDir);
    m_settings.setValue("cleanDir",               m_investigator->m_cleanDir);
    m_settings.setValue("dangerDir",              m_investigator->m_dangerDir);
    m_settings.setValue("logsDir  ",              m_investigator->m_logsDir);

    m_settings.setValue("avFilePath",             m_investigator->m_avPath);
    m_settings.setValue("infectAction",           m_investigator->m_infectedFileAction);
    m_settings.setValue("avMaxQueueSize",         m_investigator->m_maxQueueSize);
    m_settings.setValue("avMaxQueueVol",          m_investigator->m_maxQueueVol);
    m_settings.setValue("avVolUnit",              m_investigator->m_maxQueueVolUnit);
    m_settings.setValue("reportsDir",             m_investigator->m_reportsDir);
    m_settings.setValue("saveAVSReports",         m_investigator->m_saveAvsReports);
    m_settings.setValue("useExternalHandler",     m_investigator->m_useExternalHandler);
    m_settings.setValue("externalHandlerPath",    m_investigator->m_externalHandlerPath);

    m_settings.setValue("useSyslog",              m_investigator->m_useSyslog);
    m_settings.setValue("syslogAddress",          m_investigator->m_syslogAddress);
    m_settings.setValue("syslogPriority",         m_investigator->m_syslogPriority);
    m_settings.setValue("useHttpServer",          m_investigator->m_useHttpServer);
    m_settings.setValue("httpServerAddress",      m_investigator->m_httpServerAddress);

    m_settings.setValue("currentSettingsTab",     m_settingsWindow->m_currentTab);

    // статистика текущей сессии
    m_settings.setValue("infectedFilesNb",        m_investigator->m_infectedFilesNb);
    m_settings.setValue("scanningErrorFilesNb",   m_investigator->m_scanningErrorFilesNb);
    m_settings.setValue("processedFilesNb",       m_investigator->m_processedFilesNb);
    m_settings.setValue("processedFilesSizeMb",   m_investigator->m_processedFilesSizeMb);
    m_settings.setValue("startTime",              m_investigator->m_startTime);
}

void Widget::connectObjects() {
    connect(this,              &Widget::startWatchDirEye,           m_distributor,     &Distributor::startWatchDirEye);
    connect(this,              &Widget::stopWatchDirEye,            m_distributor,     &Distributor::stopWatchDirEye);

    connect(m_investigator,    &Investigator::updateUi,             this,              &Widget::updateUi);
    connect(m_settingsWindow,  &Settings::s_updateUi,               this,              &Widget::updateUi);

    connect(m_investigator,    &Investigator::log,                  this,              &Widget::log);
    connect(m_settingsWindow,  &Settings::log,                      this,              &Widget::log);
    connect(m_distributor,     &Distributor::log,                   this,              &Widget::log);

    connect(m_statisticWindow, &Statistics::saveSettings,           this,              &Widget::saveSettings);

    connect(m_settingsWindow,  &Settings::restartWatching,          m_distributor,     &Distributor::startWatchDirEye);

    connect(m_distributor,     &Distributor::tryProcess,            m_investigator,    &Investigator::onProcessFinished);

    connect(m_investigator,    &Investigator::process,              this,              &Widget::executeProcess);
    connect(&m_process,        &QProcess::readyReadStandardOutput,  this,              &Widget::parseResultOfProcess);
    connect(m_investigator,    &Investigator::saveReport,           this,              &Widget::saveReport);
    connect(this,              &Widget::parseReport,                m_investigator,    &Investigator::parseReport);

    connect(m_investigator,    &Investigator::startExternalHandler, this,              &Widget::startExternalHandler);

    connect(m_settingsWindow,  &Settings::startHttpServer,          this,              &Widget::startHttpServer);

    connect(m_investigator,    &Investigator::stopProcess,          &m_process,        &QProcess::close);

    connect(&minuteTimer,      &QTimer::timeout,                    this,              &Widget::minuteUpdate);
}

/* первночалаьное сканирование для получения версий баз
 * используется фиктивный путь '/?' для сканирования пустого множества объектов */
void Widget::getInitialAvsScan() {

    // если исполняемый файл АВС существует
    if(QFile(m_investigator->m_avPath).exists()) {

        m_investigator->m_isWorking = true;

        // запуск процесса с аргментом '/?'
        executeProcess(m_investigator->m_avPath, QStringList() << "/?");

        m_investigator->m_isWorking = false;
    } else {
        log("Не удалось определить версию АВС.", LOG_CATEGORY(GUI + DEBUG));
    }
}

void Widget::moveOldFilesToInputDir() {
    if(QDir(m_investigator->m_processDir).entryList(usingFilters).size()) {
        m_distributor->distributorMoveFiles(m_investigator->m_processDir, m_investigator->m_inputDir, MAX_FILES_TO_MOVE);
    }
}

/* ежеминутные действия */
void Widget::minuteUpdate() {
    saveSettings();
    m_investigator->sendSyslogMessage(m_investigator->getCurrentStatistic());
    log(m_investigator->getCurrentStatistic(), DEBUG);
}
