#include "investigatororchestartor.h"

InvestigatorOrchestartor::InvestigatorOrchestartor(QObject *parent):
    QObject(parent) {

    qRegisterMetaType<QFileInfoList>("QFileInfoList");

    m_isInWork = false;

    m_udpSocket = new QUdpSocket();

    // создание потоков
    m_workThreads = new QThread[MaxThreadNb];

    // создание воркеров, помещение их в собственные потоки и запуск воркеров
    createWorkers();

    // кол-во используемых потоков
    m_currentWorkersNb = MaxThreadNb;

    // таймер времени работы
    createWorkTimer();

    // таймер статистики
    createUpdateStatisticTimer();

    // таймер обработки
    createProcessTimer();
}

InvestigatorOrchestartor::~InvestigatorOrchestartor() {
    stopWorkers();
    stopThreads();
}

void InvestigatorOrchestartor::createInitialAvsProcess(bool needLaterStart) {

    // пересоздание процесса для АВС
    if (m_initialAvsProcess) {
        m_initialAvsProcess->close();
        delete m_initialAvsProcess;
    }
    m_initialAvsProcess = new QProcess(nullptr);

    // слот окончания проверки
    connect(m_initialAvsProcess,  QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
    [ = ](int exitCode, QProcess::ExitStatus exitStatus) {
        Q_UNUSED(exitCode)

        if (exitStatus == QProcess::NormalExit) {

            // ----
            if (m_initialAvsProcess) {
                m_initialAvsProcess->deleteLater();
            }

            if (m_successInitialScan) {
                return;
            }

            m_successInitialScan = true;
            log("Исполняемый файл АВС найден.", Logger::FILE + Logger::UI);

            if (needLaterStart) {
                log("Автоматическое возобновление работы программы после перезапуска.",
                    Logger::FILE + Logger::UI + Logger::SYSLOG);
                startWork();
            }

            updateUi();
            // ----

            /*
            QString m_avsReport = m_initialAvsProcess->readAll();

            // если ответ не пустой, то процесс можно использовать
            if(!m_avsReport.isEmpty()) {

                if(m_initialAvsProcess) {
                    m_initialAvsProcess->deleteLater();
                }

                // если инициализирующий запуск уже был осуществлен
                if(m_successInitialScan) {
                    return;
                }

                m_successInitialScan = true;
                log(QString("Исполняемый файл АВС найден."));


                // если нужен перезапуск после процесса инициализации
                if(needLaterStart) {
                    log("Автоматическое возобновление работы программы после перезапуска.");
                    startWork();
                }

                updateUi();
            }
            */
        } else {
            log(QString("Процесс инициализирующей проверки был аварийно завершен."),
                Logger::UI + Logger::SYSLOG);
        }
    }
           );
}

// создание воркеров, помещение их в собственные потоки и запуск воркеров
void InvestigatorOrchestartor::createWorkers() {
    m_workers = new InvestigatorWorker*[MaxThreadNb];
    for (int i = 0; i < MaxThreadNb; i++) {
        m_workers[i] = new InvestigatorWorker(nullptr);
        m_workers[i]->moveToThread(&m_workThreads[i]);

        connect(this,           &InvestigatorOrchestartor::tryWorkerCheckFiles,     m_workers[i],
                &InvestigatorWorker::tryCheckFiles);
        connect(m_workers[i],   &InvestigatorWorker::finish,                        this,
                &InvestigatorOrchestartor::onWorkerFinished);
        connect(m_workers[i],   &InvestigatorWorker::log,                           this,
                &InvestigatorOrchestartor::log);

        m_workThreads[i].start();
    }
}

// создание таймера для подсчета времени работы программы
void InvestigatorOrchestartor::createWorkTimer() {
    m_workTimer = new QTimer(this);
    dumpWorkTimer();
    connect(m_workTimer, &QTimer::timeout, [ = ]() {

        // если не в работе, то времени работы нет
        if (m_isInWork) {
            // 0 - ss, 1 - mm, 2 - hh, 3 - dd
            m_workTimeV[0]++;
            if (m_workTimeV[0] / 60) {
                m_workTimeV[1]++;
                m_workTimeV[0] %= 60;
            }
            if (m_workTimeV[1] / 60) {
                m_workTimeV[2]++;
                m_workTimeV[1] %= 60;
            }
            if (m_workTimeV[2] / 24) {
                m_workTimeV[3]++;
                m_workTimeV[2] %= 24;
            }
        }
    });
    m_workTimer->start(1000);
}

void InvestigatorOrchestartor::createUpdateStatisticTimer() {
    m_updateStatisticTimer = new QTimer(this);
    connect(m_updateStatisticTimer, &QTimer::timeout, this, &InvestigatorOrchestartor::updateProgress);
    m_updateStatisticTimer->start(UpdatePeriodMs);
}
// создание таймера запуска проверки
void InvestigatorOrchestartor::createProcessTimer() {
    m_processTimer = new QTimer(this);
    m_processTimer->setInterval(SleepIntervalAfterScanMs);
    connect(m_processTimer, &QTimer::timeout, this, &InvestigatorOrchestartor::tryCheckFiles);
}

// остановка ворекров
void InvestigatorOrchestartor::stopWorkers() {
    for (int i = 0; i < m_currentWorkersNb; i++) {
        m_workers[i]->stopWork();
    }
}

// остановка потоков
void InvestigatorOrchestartor::stopThreads() {
    for (int i = 0; i < m_currentWorkersNb; i++) {
        m_workThreads[i].quit();
        m_workThreads[i].wait();
    }
}

/* конфигурирование воркеров */
bool InvestigatorOrchestartor::reconfigureWorkers() {

    log("Процесс конфигурирования воркеров...", Logger::FILE);

    bool errorExist = false;

    if (!QDir(m_sourceDir).exists() || m_sourceDir.isEmpty()) {
        log("Не найдена входная директория.", Logger::FILE + Logger::UI);
        errorExist = true;
    }

    if (!QDir(m_processDir).exists() || m_processDir.isEmpty()) {
        log("Не найдена временная директория.", Logger::FILE + Logger::UI);
        errorExist = true;
    }

    if (!QDir(m_infectedDir).exists() || m_infectedDir.isEmpty()) {
        log("Не найдена директория для зараженных файлов.",
            Logger::FILE + Logger::UI);
        errorExist = true;
    }

    if (!QDir(m_cleanDir).exists() || m_cleanDir.isEmpty()) {
        log("Не найдена директория для чистых файлов.",
            Logger::FILE + Logger::UI);
        errorExist = true;
    }

    if (!QFile(m_avsExecFileName).exists() || m_avsExecFileName.isEmpty()) {
        log("Не найден исполняемый файл АВС.", Logger::FILE + Logger::UI);
        errorExist = true;
    }

    if (m_tempCurrentWorkersNb < 1 || m_tempCurrentWorkersNb > MaxThreadNb) {
        log("Указано некорректное количество воркеров.",
            Logger::FILE + Logger::UI);
        errorExist = true;
    }

    if (m_thresholdFilesNb < 1) {
        log("Указано некорректное значение порога по количеству файлов.",
            Logger::FILE + Logger::UI);
        errorExist = true;
    }

    if (m_thresholdFilesSize < 1) {
        log("Указано некорректное значение порога по объему файлов.",
            Logger::FILE + Logger::UI);
        errorExist = true;
    }

    if (!errorExist) {

        // обновление текущего кол-ва тредов
        m_currentWorkersNb = m_tempCurrentWorkersNb;

        // конфигурирование вокреров
        for (int i = 0; i < MaxThreadNb; i++) {

            // id, (input, process, danger, clean), avsExec, useExternalHandler, externalHandlerPath
            m_workers[i]->configure(i, QStringList{m_sourceDir, m_processDir, m_infectedDir, m_cleanDir, m_processDir + "\\reports\\"},
                                    m_avsExecFileName, m_useExternalHandler, m_externalHandlerPath, m_saveXmlReports);

            if (i < m_currentWorkersNb) {
                log(QString("Запуск воркера %1.").arg(i), Logger::FILE);
                m_workers[i]->startWork();
            } else {
                log(QString("Остановка воркера %1.").arg(i), Logger::FILE);
                m_workers[i]->stopWork();
            }
        }
        return true;
    }
    log(QString("В процессе конфигурирования воркеров возникли ошибки."),
        Logger::FILE);

    m_isInWork = false;

    return false;
}

// логирование процесса работы в файлы
void InvestigatorOrchestartor::fileLog(QString message) {

    // создание папки
    m_logsDir = m_processDir + "\\logs";
    QDir().mkpath(m_logsDir);

    // дневная папка логов
    QString dirPath = m_logsDir + "\\" + QDate().currentDate().toString("yy-MM-dd") + "\\";
    QDir().mkdir(dirPath);

    // имя лог файла в соответствии с часом работы
    QString outFileName = dirPath + "\\" + QString("log-%1.log").arg(QTime::currentTime().hour());

    // если имя не поменялось
    if (m_logFile.fileName() == outFileName) {

        // если файл существует, он проверяется на время последней записи
        if (m_logFile.exists()) {

            // если файл был перезаписан более X времени назад, то надо его закрыть и открыть по новой
            if (m_logFile.isOpen()
                    && QFileInfo(m_logFile).fileTime(QFileDevice::FileModificationTime).msecsTo(
                        QDateTime::currentDateTime()) > 5000) {
                m_logFile.close();
            }
        }
    } else {
        // если имя файла не совпадает, то закрываем текущий файл и создаем новый
        m_logFile.close();
        m_logFile.setFileName(outFileName);
    }

    // если файл закрыт или файла нет
    if (!m_logFile.isOpen()) {
        m_logFile.open(QIODevice::WriteOnly | QIODevice::Append);
    }

    // запись в файл
    if (m_logFile.isOpen()) {
        QTextStream ts(&m_logFile);
        ts << QString("%1 %2\n")
           .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
           .arg(message);
    }

    if (m_logFile.isOpen()) {
        m_logFile.close();
    }
}

// логгирование
// ctx == "UI" - вывод сообщения в главное окно программы
// ctx == "SYSLOG" - вывод сообщения в syslog
// ctx == "FILE" - вывод сообщения в лог файл программы
void InvestigatorOrchestartor::log(QString message, int logCtx) {
    if (logCtx & Logger::UI) {
        emit uiLog(message);
    }

    if (logCtx & Logger::SYSLOG) {
        sendSyslogMessage(message);
    }

    if (logCtx & Logger::FILE) {
        fileLog(message);
    }
}

// обновление списка файлов на обработку
bool InvestigatorOrchestartor::updateTotalFileList() {
    if (!m_totalFileList.isEmpty()) {
        return true;
    } else {
        m_totalFileList = QDir(m_sourceDir).entryInfoList(usingFilters, usingFlags);
        log(QString("Список файлов на проверку обновлен. В очереди %1 файлов.").arg(
                m_totalFileList.size()), Logger::FILE);
    }
    return m_totalFileList.size() != 0;
}

// запуск обработки
void InvestigatorOrchestartor::tryCheckFiles() {

    if (m_isInWork) {
        for (int i = 0; i < m_currentWorkersNb; i++) {
            if (!m_workers[i]->isInProcess()) { // если воркер не занят
                if (updateTotalFileList()) { // если есть файлы для проверки
                    QFileInfoList filesToCheck = getFilesToCheckList();
                    log(QString("Воркеру %1 назначено %2 файлов на проверку.").arg(
                            i).arg(filesToCheck.size()), Logger::FILE);
                    tryWorkerCheckFiles(i, filesToCheck);
                }
            }
        }
    }
}

// слот обработки получения сигнала от воркера
void InvestigatorOrchestartor::onWorkerFinished(int workerId) {
    if (workerId >= 0 && workerId < MaxThreadNb) {

        WorkerStatistic workerStatistic = m_workers[workerId]->getLastStatistics();
        m_totalProcessedFilesNb += workerStatistic.processedFilesNb;
        m_totalProcessedFilesSize += workerStatistic.processedFilesSize;
        m_totalPwdFilesNb += workerStatistic.pwdFilesNb;
        m_totalInfectedFilesNb += workerStatistic.infFilesNb;

        log(QString("Воркер %1 завершил проверку: всего %2(%3), запароленых %4, инфицированных %5.")
            .arg(workerId)
            .arg(workerStatistic.processedFilesNb)
            .arg(SizeConverter::sizeToString(workerStatistic.processedFilesSize))
            .arg(workerStatistic.pwdFilesNb)
            .arg(workerStatistic.infFilesNb),
            Logger::FILE);

        // проверка обновления версии АВС
        if (m_avVersion != workerStatistic.avsVersion && // если версия другая
                !workerStatistic.avsVersion.isEmpty()) { // если строка с версией не пуста
            QString avsVersion = workerStatistic.avsVersion;
            avsVersion.replace("Версия баз: ", "");
            avsVersion.replace(avsVersion.indexOf("Ядро M-52") - 1,
                               avsVersion.indexOf(" (от") - avsVersion.indexOf("Ядро M-52"),
                               "");

            log(QString("Найдена новая версия баз АВС: %1.").arg(avsVersion),
                Logger::FILE + Logger::UI);

            m_avVersion = workerStatistic.avsVersion;
        }

        tryCheckFiles();
    }
}

// получение списк ана проверку для воркера
QFileInfoList InvestigatorOrchestartor::getFilesToCheckList() {

    QFileInfoList resultList; // результирующий список файлов
    quint64 resultListSize = 0; // счетчик объема файлов
    QFileInfo currentFileInfo; // текущий файл

    while (m_totalFileList.size() && // пока есть файлы в m_totalFileList
            resultListSize < (m_thresholdFilesSize * m_thresholdFilesSizeUnit)
            && // и пока не превышен порог по объему
            quint64(resultList.size()) <
            m_thresholdFilesNb ) { // и пока не превышен порог по кол-ву файлов

        currentFileInfo = m_totalFileList.takeFirst();
        resultList.append(currentFileInfo);
        resultListSize += currentFileInfo.size();
    }
    return resultList;
}

void InvestigatorOrchestartor::startWork() {

    if (m_isInWork) {
        stopWork();
    }

    if (reconfigureWorkers()) {
        log(QString("Потоковая проверка запущена."), Logger::FILE + Logger::UI);
        m_isInWork = true;
        m_processTimer->start();
        tryCheckFiles();
    } else {
        log(QString("Не удалось запустить потоковую проверку."),
            Logger::FILE + Logger::UI);
        m_processTimer->stop();
        m_isInWork = false;
    }

    emit updateUi();
}

void InvestigatorOrchestartor::stopWork() {
    m_processTimer->stop();
    if (m_isInWork) {
        log(QString("Потоковая проверка остановлена."),
            Logger::FILE + Logger::UI);
    }
    m_isInWork = false;
    stopWorkers();
    dumpWorkTimer();
    emit updateUi();
}

void InvestigatorOrchestartor::setAvsExecFileName(QString avsFileName) {
    m_avsExecFileName = avsFileName;
    for (int i = 0; i < MaxThreadNb; i++) {
        m_workers[i]->setAvsExecFileName(m_avsExecFileName);
    }
}

// количество файлов, находящихся на проверке в данный момент
qint64 InvestigatorOrchestartor::inProcessFilesNb() {
    qint64 filesInProcessNb = 0;
    for (int i = 0; i < m_currentWorkersNb; i++) {
        filesInProcessNb += m_workers[i]->filesInProcessNb();
    }
    return filesInProcessNb;
}

// начальное сканирование для определения версии баз
void InvestigatorOrchestartor::getInitialAvsScan(bool needLaterStart) {

    // если исполняемый файл АВС существует
    if (QFile(m_avsExecFileName).exists() && !m_avsExecFileName.isEmpty()) {

        log(QString("Начат процесс инициализирующей проверки АВС."),
            Logger::FILE);

        // пока не будет налажена свзяь с АВС
        while (!m_successInitialScan) {
            log(QString("Попытка запуска инициализирующей проверки АВС..."),
                Logger::FILE);
            createInitialAvsProcess(needLaterStart);
            // запуск пустой проверки
            m_initialAvsProcess->start(m_avsExecFileName, QStringList{"/?"});
            QThread::msleep(10000);
        }
    }

    emit updateUi();
}

// кол-во файлов в очереди
quint64 InvestigatorOrchestartor::queueFilesNb() {

    if (!m_sourceDir.isEmpty() && QDir(m_sourceDir).exists()) {
        return QDir(m_sourceDir).entryList(usingFilters).size();
    } else {
        return 0;
    }
}

// объем файлов в очереди
quint64 InvestigatorOrchestartor::queueFilesSize() {
    return [ = ]() {
        if (!m_sourceDir.isEmpty() && QDir(m_sourceDir).exists()) {
            quint64 listSize = 0;
            foreach (QFileInfo info, QDir(m_sourceDir).entryInfoList(usingFilters)) {
                listSize += info.size();
            }
            return listSize;
        } else {
            return quint64(0);
        }
    }
    ();
}

// время работы в секундах
quint64 InvestigatorOrchestartor::getWorkTimeInSec() {
    return m_workTimeV[0] + m_workTimeV[1] * 60 + m_workTimeV[2] * 3600 + m_workTimeV[3] * 86400;
}

quint64 InvestigatorOrchestartor::currentSpeed() {
    quint64 summarySpeed = 0;
    for (int i = 0; i < m_currentWorkersNb; i++) {
        summarySpeed += m_workers[i]->speed();
    }
    return summarySpeed;
}

// время работы в виде строки
QString InvestigatorOrchestartor::workTimeToString() {
    return QString("%1 дн %2 час %3 мин %4 сек")
           .arg(m_workTimeV[3])
           .arg(m_workTimeV[2])
           .arg(m_workTimeV[1])
           .arg(m_workTimeV[0]);
}

// статистика работы в виде строки
QString InvestigatorOrchestartor::workStatisticToString() {
    return QString("Время работы %1, просканировано %2 файлов(%3), запароленых %4, инфицированных %5.")
           .arg(workTimeToString())
           .arg(m_totalProcessedFilesNb)
           .arg(SizeConverter::sizeToString(m_totalProcessedFilesSize))
           .arg(m_totalPwdFilesNb)
           .arg(m_totalInfectedFilesNb);
}

void InvestigatorOrchestartor::sendSyslogMessage(QString msg) {
    msg.prepend(QString("<%1>%2 %3 ").arg(59).arg(
                    QNetworkInterface::allAddresses().first().toString()).arg(
                    QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss")));
    QByteArray m_msg = msg.toUtf8();
    m_udpSocket->writeDatagram(m_msg, m_msg.size(), QHostAddress(m_syslogAddress), 514);
}
