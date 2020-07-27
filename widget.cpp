#include "widget.h"
#include "ui_widget.h"

void Widget::createAboutWidget() {
    m_aboutProgramWidget = new AboutProgramWidget(this, QString("The Investigator"),
                                                  QString("Программа для потоковой антивирусной проверки файлов."
                                                          "\n\nРазработчик: Федор Назаров (IP: 7721)"));
    m_aboutProgramWidget->hide();
}

void Widget::createTrayIcon() {
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(QIcon(":/ICONS/INVESTIGATOR.ico"));
    connect(m_trayIcon, &QSystemTrayIcon::activated, [=](QSystemTrayIcon::ActivationReason reason) {
        switch(reason) {
            case QSystemTrayIcon::Trigger:
            case QSystemTrayIcon::DoubleClick:
            case QSystemTrayIcon::MiddleClick:
                setVisible(true);
                showNormal();
                break;
            default:
                ;
            }
    });
    m_trayIcon->show();
    m_trayIcon->setVisible(true);
}

void Widget::createInvestigator() {
    m_investigator = new InvestigatorOrchestartor(nullptr);
    m_investigator->moveToThread(&m_investigatorThread);
    m_investigatorThread.start();
}

void Widget::createStatisticsWindow() {
    m_statisticsWindow = new StatisticsWindow(this, m_investigator);
}

void Widget::createSettingsWindow() {
    m_settingsWindow = new SettingsWindow(this, m_investigator, &m_isUiLocked);
}

void Widget::startHttpServer() {
    if(m_httpServer) {
        m_httpServer->close();
        delete m_httpServer;
        m_httpServer = nullptr;
    }

    // Start the HTTP server
    HttpRequestMapper *hrm = new HttpRequestMapper(nullptr, m_investigator);
    m_httpServer = new HttpListener(80, "0.0.0.0", hrm, nullptr);
}

void Widget::createSaveSettingsTimer() {
    m_saveSettingsTimer = new QTimer();
    connect(m_saveSettingsTimer, &QTimer::timeout, this, &Widget::saveSettings);
    m_saveSettingsTimer->start(60 * 1000);
}

void Widget::connectObjects() {
    connect(ui->aboutLabel, &ClickableLabel::clicked, this, &Widget::onAboutClicked);

    connect(this, &Widget::getInitialAvsScan,       m_investigator,         &InvestigatorOrchestartor::getInitialAvsScan);

    connect(m_investigator,     &InvestigatorOrchestartor::updateProgress,  m_statisticsWindow, &StatisticsWindow::updateUi);
    connect(m_investigator,     &InvestigatorOrchestartor::updateProgress,  [=](){
        ui->workTimeInfoLabel->setText(m_investigator->workTimeToString());
    });

    connect(m_investigator,     &InvestigatorOrchestartor::uiLog,           this,               &Widget::uiLog);
    connect(m_investigator,     &InvestigatorOrchestartor::updateUi,        this,               &Widget::updateUi);
    connect(this,               &Widget::start,                             m_investigator,     &InvestigatorOrchestartor::startWork);
    connect(this,               &Widget::stop,                              m_investigator,     &InvestigatorOrchestartor::stopWork);
    connect(this,               &Widget::log,                               m_investigator,     &InvestigatorOrchestartor::log);

    // перезапуск http сервера
    connect(m_settingsWindow,   &SettingsWindow::restartHttpServer,         this,               &Widget::startHttpServer);
}

// завершение работы программы
void Widget::closeProgram() {

    saveSettings();

    emit log("Работа программы завершена.", Logger::UI + Logger::SYSLOG + Logger::FILE);
    emit log("---------------------------", Logger::FILE);

    m_trayIcon->hide();

    m_investigator->stopWork();

    m_investigatorThread.quit();
    m_investigatorThread.wait();
}

Widget::Widget(QWidget *parent): QWidget(parent), ui(new Ui::Widget), m_settings("FeZar97", "TheInvestigator") {
    ui->setupUi(this);

    setWindowTitle(QString("The Investigator %1").arg(Version));
    setLayout(ui->mainLayout);
    createTrayIcon();
    createAboutWidget();
    createSaveSettingsTimer();

    createInvestigator();
    createStatisticsWindow();
    createSettingsWindow();

    connectObjects();

    startHttpServer();

    restoreSettings();

    emit getInitialAvsScan(m_settings.value("isInWork", false).toBool());

    updateUi();
}

Widget::~Widget() {
    closeProgram();
    delete ui;
}

void Widget::uiLog(QString message) {
    ui->logTB->append(QString("%1 %2")
                      .arg(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss"))
                      .arg(message));
}

void Widget::closeEvent(QCloseEvent *event) {

    saveSettings();

    if(QMessageBox::warning(this,
                            QString("Подтвердите действие"),
                            QString("Вы действительно хотите завершить работу программы?"),
                            QString("Да"), QString("Нет")) == 0) {
        closeProgram();
        exit(0);
        event->accept();
    } else {
        event->ignore();
    }
}

void Widget::hideEvent(QHideEvent *event) {
    setVisible(false);
    m_trayIcon->showMessage("TheInvestigator",
                            "Окно программы свернуто в трей",
                            QIcon(":/ICONS/INVESTIGATOR.ico"),
                            5000);
    event->accept();
}

void Widget::on_startButton_clicked() {
    ui->startButton->setEnabled(false);
    emit start();
}

void Widget::on_stopButton_clicked() {
    ui->stopButton->setEnabled(false);
    emit stop();
}

void Widget::updateUi() {
    ui->startButton->setEnabled(!m_isUiLocked && m_investigator->resultOfInitialScan() && !m_investigator->isInWork());
    ui->stopButton->setEnabled(!m_isUiLocked && m_investigator->resultOfInitialScan() && m_investigator->isInWork());
    ui->clearButton->setEnabled(!m_isUiLocked);

    ui->lockUiButton->setIcon(QIcon(m_isUiLocked ? ":/ICONS/UNLOCK.ico" : ":/ICONS/LOCK.ico"));

    m_statisticsWindow->updateUi();
    m_settingsWindow->updateUi();
}

// восстановление настроек
void Widget::restoreSettings() {
    // настройки директорий
    m_investigator->setSourceDir(m_settings.value("sourceDir",      "C:\\investigator\\input").toString());
    m_investigator->setProcessDir(m_settings.value("processDir",    "C:\\investigator\\temp").toString());
    m_investigator->setCleanDir(m_settings.value("cleanDir",        "C:\\investigator\\clean").toString());
    m_investigator->setInfectedDir(m_settings.value("infectedDir",  "C:\\investigator\\danger").toString());

    // настройки воркеров
    m_investigator->setWorkersNb(m_settings.value("threadsNb",                      QThread::idealThreadCount()).toInt());
    m_investigator->setThresholdFilesNb(m_settings.value("thresholdFilesNb",        100).toInt());
    m_investigator->setThresholdFilesSize(m_settings.value("thresholdFilesSize",    100).toInt());
    m_investigator->setThresholdFilesSizeUnit(m_settings.value("thresholdFilesSizeUnit", SizeConverter::MEGABYTE).toLongLong());

    m_investigator->setAvsExecFileName(m_settings.value("avsPath", "C:/Program Files/Primetech/M-52/AVSFileConsoleScan.exe").toString());

    m_investigator->setSyslogAddress(m_settings.value("syslogAddress", "0.0.0.0").toString());

    // статистика общая
    m_investigator->setTotalProcessedFilesNb(m_settings.value("totalProcessedFilesNb",      0).toLongLong());
    m_investigator->setTotalProcessedFilesSize(m_settings.value("totalProcessedFilesSize",  0).toLongLong());
    m_investigator->setTotalInfectedFilesNb(m_settings.value("totalInfectedFilesNb",        0).toLongLong());
    m_investigator->setTotalPwdFilesNb(m_settings.value("totalPwdFilesNb",                  0).toLongLong());

    // вкладка окна настроек
    m_settingsWindow->setCurrentOpenTab(m_settings.value("settingsWinCurrentTab", 0).toInt());

    // геометрия окна
    restoreGeometry(m_settings.value("mainWinGeometry").toByteArray());
    m_settingsWindow->restoreGeometry(m_settings.value("settingsWinGeometry").toByteArray());
    m_statisticsWindow->restoreGeometry(m_settings.value("statisticsWinGeometry").toByteArray());

    // видимость окон
    m_settingsWindow->setVisible(m_settings.value("settingsWinVisible", false).toBool());
    m_statisticsWindow->setVisible(m_settings.value("statisticsWinVisible", false).toBool());

    // блокировка интерфейса
    m_isUiLocked = m_settings.value("isUiLocked", false).toBool();

    emit log("Программа запущена.", Logger::UI + Logger::SYSLOG + Logger::FILE);
    emit log(QString("Настройки программы восстановлены."), Logger::FILE);
}

// сохранение настроек
void Widget::saveSettings() {

    emit log(m_investigator->workStatisticToString(), Logger::SYSLOG + Logger::FILE);

    // настройки директорий
    m_settings.setValue("sourceDir", m_investigator->sourceDir());
    m_settings.setValue("processDir", m_investigator->processDir());
    m_settings.setValue("infectedDir", m_investigator->infectedDir());
    m_settings.setValue("cleanDir", m_investigator->cleanDir());

    // настройки воркеров
    m_settings.setValue("threadsNb", m_investigator->tempCurrentWorkersNb());
    m_settings.setValue("thresholdFilesNb", m_investigator->thresholdFilesNb());
    m_settings.setValue("thresholdFilesSize", m_investigator->thresholdFilesSize());
    m_settings.setValue("thresholdFilesSizeUnit", m_investigator->thresholdFilesSizeUnit());
    m_settings.setValue("avsPath", m_investigator->avsExecFileName());
    m_settings.setValue("syslogAddress", m_investigator->syslogAddress());

    // флаг работы
    m_settings.setValue("isInWork", m_investigator->isInWork());

    // статистика общая
    m_settings.setValue("totalProcessedFilesNb", m_investigator->totalProcessedFilesNb());
    m_settings.setValue("totalProcessedFilesSize", m_investigator->totalProcessedFilesSize());
    m_settings.setValue("totalInfectedFilesNb", m_investigator->totalInfectedFilesNb());
    m_settings.setValue("totalPwdFilesNb", m_investigator->totalPwdFilesNb());

    // вкладка окна настроек
    m_settings.setValue("settingsWinCurrentTab", m_settingsWindow->currentOpenTab());

    // геометрия окна
    m_settings.setValue("mainWinGeometry", saveGeometry());
    m_settings.setValue("settingsWinGeometry", m_settingsWindow->saveGeometry());
    m_settings.setValue("statisticsWinGeometry", m_statisticsWindow->saveGeometry());

    // видимость окон
    m_settings.setValue("settingsWinVisible", m_settingsWindow->isVisible());
    m_settings.setValue("statisticsWinVisible", m_statisticsWindow->isVisible());

    // блокировка интерфейса
    m_settings.setValue("isUiLocked", m_isUiLocked);

    emit log(QString("Настройки программы сохранены."), Logger::FILE);
}

// очистка
void Widget::on_clearButton_clicked() {

    int reply = QMessageBox::question(this, QString("Очистка"), QString("Выберите тип очистки:\n\t'Окно логов' - очистить окно вывода логов;\n\t'Полная очистка' - очистить логи и накопленную статистику."),
                                          QString("Окно логов"), QString("Полная очистка"), QString("Отмена"));

    if(reply == -1 || reply == 2) {
        return;
    } else if(reply == 1) {
        m_investigator->dumpWorkTimer();
        m_investigator->setTotalProcessedFilesNb(0);
        m_investigator->setTotalProcessedFilesSize(0);
        m_investigator->setTotalInfectedFilesNb(0);
        m_investigator->setTotalPwdFilesNb(0);

        emit log(QString("Выполнен сброс статистики."), Logger::FILE);
    }

    ui->logTB->clear();

    updateUi();
}

// о программе
void Widget::onAboutClicked() {
    m_aboutProgramWidget->show();
}

void Widget::on_settingsButton_clicked() {
    m_settingsWindow->setVisible(!m_settingsWindow->isVisible());
}

void Widget::on_statisticsButton_clicked() {
    m_statisticsWindow->setVisible(!m_statisticsWindow->isVisible());
}

void Widget::on_lockUiButton_clicked() {
    m_isUiLocked = !m_isUiLocked;
    updateUi();
}
