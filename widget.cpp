#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent): QWidget(parent),
                                 ui(new Ui::Widget),
                                 m_settings("FeZar97", "Investigator"),
                                 m_win1251Codec(QTextCodec::codecForName("Windows-1251")) {

    ui->setupUi(this);

    setLayout(ui->mainLayout);
    setWindowTitle(QString("Investigator ") + VERSION);
    restoreGeometry(m_settings.value("geometry").toByteArray());

    m_investigator = new Investigator();
    m_investigator->m_watchDir        = m_settings.value("watchDir", "C:/").toString();
    m_investigator->m_investigatorDir = m_settings.value("investigatorDir", QDir::tempPath()).toString();
    m_investigator->m_cleanDir        = m_settings.value("cleanDir", "C:/").toString();
    m_investigator->m_dangerDir       = m_settings.value("dangerDir", "C:/").toString();
    m_investigator->m_avPath          = m_settings.value("avFilePath", "C:/Program Files/Primetech/M-52/AVSFileConsoleScan.exe").toString();
    m_investigator->m_maxQueueSize    = m_settings.value("avMaxQueueSize", 10).toInt();
    m_investigator->m_maxQueueVolMb   = m_settings.value("avMaxQueueVol", 128.).toDouble();
    m_investigator->m_maxQueueVolUnit = m_settings.value("avVolUnit", 0).toInt();

    m_investigator->m_infectedFileAction = ACTION_TYPE(m_settings.value("infectAction", 0).toInt());

    m_investigator->m_useExternalHandler = m_settings.value("useExternalHandler", false).toBool();
    m_investigator->m_externalHandlerPath = m_settings.value("externalHandlerPath", "").toString();

    m_investigator->m_useSyslog = m_settings.value("useSyslog", false).toBool();
    m_investigator->m_syslogAddress = m_settings.value("syslogAddress", "127.0.0.1:514").toString();

    m_investigator->configureDirs();
    m_investigator->clearStatistic();
    m_investigator->checkSyslogAddress();
    m_investigator->moveToThread(&m_workThread);

    m_distributor     = new Distributor(nullptr, m_investigator);
    m_distributor->moveToThread(&m_workThread);

    m_settingsWindow  = new Settings(this, m_investigator, m_settings.value("settingsWinGeometry").toByteArray());
    m_statisticWindow = new Statistics(this, m_investigator, m_settings.value("statisticWinGeometry").toByteArray());

    connect(this,             &Widget::startWork,                  m_distributor,     &Distributor::startWatchDirEye);
    connect(this,             &Widget::stopWork,                   m_distributor,     &Distributor::stopWatchDirEye);

    connect(m_distributor,    &Distributor::updateUi,              this,              &Widget::updateUi);
    connect(m_investigator,   &Investigator::updateUi,             m_statisticWindow, &Statistics::updateUi);

    connect(m_investigator,   &Investigator::log,                  this,              &Widget::log);
    connect(m_settingsWindow, &Settings::log,                      this,              &Widget::log);
    connect(m_distributor,    &Distributor::log,                   this,              &Widget::log);

    connect(m_settingsWindow, &Settings::clearDir,                 m_distributor,     &Distributor::clearDir);

    connect(m_investigator,   &Investigator::process,              this,              &Widget::startProcess);
    connect(&m_process,       &QProcess::readyReadStandardOutput,  this,              &Widget::parseResultOfProcess);
    connect(m_investigator,   &Investigator::saveReport,           this,              &Widget::saveReport);
    connect(this,             &Widget::parseReport,                m_investigator,    &Investigator::parseReport);

    connect(m_investigator,   &Investigator::startExternalHandler, this,              &Widget::startExternalHandler);

    connect(m_investigator,   &Investigator::stopProcess,          &m_process,        &QProcess::close);

    m_workThread.start();

    log("Программа запущена.", MSG_CATEGORY(INFO + LOG_GUI));
    m_investigator->sendSyslogMessage("Program is starting", SYS_INFO, SYS_USER);

    if(QFile(m_investigator->m_avPath).exists()) {
        m_investigator->m_isWorking = true;
        startProcess(m_investigator->m_avPath, QStringList() << "/?");
        m_investigator->m_isWorking = false;
    } else {
        log("Не удалось определить версию АВС.", CRITICAL);
    }

    m_investigator->collectStatistics();
}

Widget::~Widget() {

    if(m_investigator->m_isInProcess)
        m_process.close();

    m_distributor->stopWatchDirEye();

    m_settings.setValue("geometry",               saveGeometry());

    m_settings.setValue("settingsWinGeometry",    m_settingsWindow->saveGeometry());
    m_settings.setValue("settingsWinVisible",     m_settingsWindow->isVisible());
    m_settings.setValue("statisticWinGeometry",   m_statisticWindow->saveGeometry());
    m_settings.setValue("statisticWinVisible",    m_statisticWindow->isVisible());

    m_settings.setValue("watchDir",               m_investigator->m_watchDir);
    m_settings.setValue("investigatorDir",        m_investigator->m_investigatorDir);
    m_settings.setValue("cleanDir",               m_investigator->m_cleanDir);
    m_settings.setValue("dangerDir",              m_investigator->m_dangerDir);

    m_settings.setValue("avFilePath",             m_investigator->m_avPath);
    m_settings.setValue("avMaxQueueSize",         m_investigator->m_maxQueueSize);
    m_settings.setValue("avMaxQueueVol",          m_investigator->m_maxQueueVolMb);
    m_settings.setValue("avVolUnit",              m_investigator->m_maxQueueVolUnit);

    m_settings.setValue("infectAction",           m_investigator->m_infectedFileAction);

    m_settings.setValue("useExternalHandler",     m_investigator->m_useExternalHandler);
    m_settings.setValue("externalHandlerPath",    m_investigator->m_externalHandlerPath);

    m_settings.setValue("useSyslog",              m_investigator->m_useSyslog);
    m_settings.setValue("syslogAddress",          m_investigator->m_syslogAddress);

    m_investigator->sendSyslogMessage("Programm shutting down.", SYS_INFO, SYS_USER);

    m_workThread.quit();
    m_workThread.wait();

    log("Завершение работы программы.", INFO);

    delete m_settingsWindow;
    delete m_statisticWindow;
    delete m_distributor;
    delete m_investigator;

    delete ui;
}

void Widget::log(QString s, MSG_CATEGORY cat) {
    if(cat & INFO)     qInfo() << s << "\r\n";
    if(cat & DEBUG)    qDebug() << s << "\r\n";
    if(cat & CRITICAL) qCritical() << s << "\r\n";
    if(cat & LOG_ROW)  m_investigator->m_processInfo = s;
    if(cat & LOG_GUI)  ui->logPTE->appendPlainText(currentDateTime() + " " + s);
    // если программа не запущена, то надо принудительно обновить интерфейс
    if(!m_investigator->m_isWorking) updateUi();
}

void Widget::updateUi() {
    ui->startButton->setEnabled(!m_investigator->m_isWorking);
    ui->stopButton->setEnabled(m_investigator->m_isWorking);
    m_settingsWindow->updateUi();
    m_statisticWindow->updateUi();
}

void Widget::startProcess(QString path, QStringList args) {
    if(m_investigator->m_isWorking) {
        m_investigator->m_lastProcessStartTime = QDateTime::currentDateTime();
        m_process.start(path, args);
    }
}

void Widget::parseResultOfProcess() {
    emit parseReport(m_win1251Codec->toUnicode(m_process.readAllStandardOutput()));
    m_process.close();
}

void Widget::saveReport(QString report, unsigned long long reportIdx) {

    if(!QDir(m_investigator->m_reportsDir).exists()) {
        QDir().mkdir(m_investigator->m_reportsDir);
    }

    QFile outFile(QString("%1report_%2_(%3).txt")
                  .arg(m_investigator->m_reportsDir + "/")
                  .arg(reportIdx)
                  .arg(QDate::currentDate().toString("dd.MM.yy")));
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << report << endl;
    outFile.close();
}

void Widget::startExternalHandler(QString path, QStringList args) {
    log(QString("Вызов внешнего обработчика %1 с аргументами: %2").arg(path).arg(entryListToString(args)), MSG_CATEGORY(INFO));
    if(QFile(path).exists()) {
        QProcess::execute(path, args);
    } else {
        log(QString("Внешний обработчик не найден!"), MSG_CATEGORY(INFO));
    }
}

void Widget::on_startButton_clicked() {
    emit startWork();
}

void Widget::on_stopButton_clicked() {
    if(QMessageBox::warning(this,
                            tr("Подтвердите действие"),
                            QString("Вы действительно хотите остановить слежение за каталогом?"),
                            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
        log(QString("Слежение за каталогом %1 остановлено.").arg(m_investigator->m_watchDir), MSG_CATEGORY(INFO + LOG_GUI));
        emit stopWork();
    }
}

void Widget::on_settingsButton_clicked() {
    m_settingsWindow->setVisible(!m_settingsWindow->isVisible());
}

void Widget::on_statisticButton_clicked() {
    m_statisticWindow->setVisible(!m_statisticWindow->isVisible());
}

void Widget::on_clearButton_clicked() {
    log("Окно отображения лога очищено.", INFO);
    ui->logPTE->clear();
}
