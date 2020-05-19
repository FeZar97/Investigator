#include "investigator.h"

Investigator::Investigator(QObject *parent) : QObject(parent){
    qRegisterMetaType<LOG_CATEGORY>("LOG_CATEGORY");
    m_syslogSocket = new QUdpSocket(this);
}

bool Investigator::checkSyslogAddress() {

    QStringList parts = m_syslogAddress.split(":");

    if(parts.size() != 2) return false;

    m_syslogIpAddress = QHostAddress(parts[0]);
    m_syslogPort = quint16(parts[1].toInt());

    if(parts[0].isNull())
        return false;
    if(m_syslogIpAddress.toString() + ":" + QString::number(m_syslogPort) == m_syslogAddress)
        return true;
    else
        return false;
}

bool Investigator::checkHttpAddress() {
    QStringList parts = m_httpServerAddress.split(":");

    if(parts.size() != 2) return false;

    m_httpServerIp = parts[0].isEmpty() ? QHostAddress::Any : QHostAddress(parts[0]);
    m_httpServerPort = quint16(parts[1].toInt());

    if(parts[0].isNull())
        return false;

    if(m_httpServerIp.toString() + ":" + QString::number(m_httpServerPort) == m_httpServerAddress)
        return true;
    else
        return false;
}

bool Investigator::checkAvParams() {

    bool problem = false;

    if(m_watchDir.isEmpty()) {
        log("Watching directory is not selected!", LOG_CATEGORY(DEBUG + GUI));
        problem = true;
    }
    if(!QDir(m_watchDir).exists()) {
        log("Watching directory does not exist!", LOG_CATEGORY(DEBUG + GUI));
        problem = true;
    }

    if(m_investigatorDir.isEmpty()) {
        log("Temporary directory is not selected!", LOG_CATEGORY(DEBUG + GUI));
        problem = true;
    }
    if(!QDir(m_investigatorDir).exists()) {
        log("Temporary directory does not exist!", LOG_CATEGORY(DEBUG + GUI));
        problem = true;
    }

    if(m_cleanDir.isEmpty()) {
        log("Directory for clean files is not selected!", LOG_CATEGORY(DEBUG + GUI));
        problem = true;
    }
    if(!QDir(m_cleanDir).exists()) {
        log("Directory for clean files does not exist!", LOG_CATEGORY(DEBUG + GUI));
        problem = true;
    }

    if(m_dangerDir.isEmpty()) {
        log("Directory for infected files is not selected!", LOG_CATEGORY(DEBUG + GUI));
        problem = true;
    }
    if(!QDir(m_dangerDir).exists()) {
        log("Directory for infected files does not exist!", LOG_CATEGORY(DEBUG + GUI));
        problem = true;
    }

    if(m_avPath.isEmpty()) {
        log("AVS executable file is not selected!", LOG_CATEGORY(DEBUG + GUI));
        problem = true;
    }
    if(!QFile(m_avPath).exists()) {
        log("AVS executable file does not exist!", LOG_CATEGORY(DEBUG + GUI));
        problem = true;
    }

    return !problem;
}

QDateTime Investigator::getEndTime() {
    if(m_isWorking) {
        m_endTime = QDateTime::currentDateTime();
    }
    return m_endTime;
}

void Investigator::clearStatistic(bool force) {
    m_endTime = QDateTime::currentDateTime();

    if(force) {
        m_startTime = QDateTime::currentDateTime();

        m_infectedFilesNb = 0;
        m_inProgressFilesNb = 0;
        m_processedFilesNb = 0;
        m_averageProcessSpeed = 0;
        m_processedFilesSizeMb = 0;
        m_processedLastFilesSizeMb = 0;
        m_currentProcessSpeed = 0;
        m_scanningErrorFilesNb = 0;
    }
}

void Investigator::configureDirs() {
    if(m_investigatorDir.isEmpty()) {
        log(QString("Can not creating temporary dirs, because investigatorDir is not choosed."), LOG_CATEGORY(DEBUG + GUI));
    } else {
        if(m_inputDir.isEmpty()) {
            m_inputDir   = m_investigatorDir + "/" + INPUT_DIR_NAME;
            QDir().mkpath(m_inputDir);
        }

        if(m_processDir.isEmpty()) {
            m_processDir = m_investigatorDir + "/" + OUTPUT_DIR_NAME;
            QDir().mkpath(m_processDir);
        }

        if(m_cleanDir.isEmpty()) {
            m_cleanDir   = m_investigatorDir + "/" + CLEAN_DIR_NAME;
            QDir().mkpath(m_cleanDir);
        }

        if(m_dangerDir.isEmpty()) {
            m_dangerDir  = m_investigatorDir + "/" + DANGER_DIR_NAME;
            QDir().mkpath(m_dangerDir);
        }

        if(m_logsDir.isEmpty()) {
            m_logsDir    = m_investigatorDir + "/" + LOGS_DIR_NAME;
            QDir().mkpath(m_logsDir);
        }

        if(m_reportsDir.isEmpty()) {
            m_reportsDir = m_investigatorDir + "/" + REPORTS_DIR_NAME;
            QDir().mkpath(m_reportsDir);
        }
    }
}

void Investigator::onProcessFinished() {

    int inputFilesNumber = QDir(m_inputDir).entryInfoList(usingFilters).size();

    // если слежение запущено, процесс не занят и есть что переносить
    if(m_isWorking &&
       !m_isInProcess &&
       inputFilesNumber) {

        // надо закрыть возможность переноса из inputDir в processDir на время проверки
        m_isInProcess = true;

        log(QString("Transferring %1 files from inputDir(%2) into processDir(%3) with limit %4 files.").arg(inputFilesNumber)
                                                                                                       .arg(m_inputDir)
                                                                                                       .arg(m_processDir)
                                                                                                       .arg(m_maxQueueSize), LOG_CATEGORY(DEBUG + DEBUG_ROW));
        m_inProcessFileList.clear();
        m_inProcessFileList = QDir(m_inputDir).entryList(usingFilters);

        if(m_inProcessFileList.size()) {
            log(QString("Checking files: %1").arg(entryListToString(m_inProcessFileList)), LOG_CATEGORY(DEBUG));
        }

        moveFiles(m_inputDir, m_processDir, m_maxQueueSize);

        m_inQueueFileSizeMb = 0;
        foreach(QFileInfo fi, QDir(m_processDir).entryInfoList(usingFilters)) {
            m_inQueueFileSizeMb += double(fi.size()) / 1024. / 1024.;
        }

        // если каталог с файлами не пуст
        if(!QDir(m_processDir).isEmpty()) {
            log("Start checking...", LOG_CATEGORY(DEBUG));

            emit process(QDir::toNativeSeparators(m_avPath), QStringList() << QDir::toNativeSeparators(m_processDir));
        }
    }

    collectStatistics();
}

/* обновление статистики */
void Investigator::collectStatistics() {
    m_inQueueFilesNb = QDir(m_inputDir).entryList(usingFilters).size();
    m_inProgressFilesNb = QDir(m_processDir).entryList(usingFilters).size();

    emit updateUi();
}

bool Investigator::beginWork() {
    if(checkAvParams()) {
        m_isWorking = true;
        clearStatistic();
        log(QString("Start watching to directory %1.").arg(m_watchDir), LOG_CATEGORY(DEBUG + GUI));
        return true;
    } else {
        m_isWorking = false;
        log(QString("Failed start watching to directory %1.").arg(m_watchDir), LOG_CATEGORY(DEBUG + GUI));
        return false;
    }
}

void Investigator::stopWork() {
    m_isWorking = false;
    m_isInProcess = false;
    m_endTime = QDateTime::currentDateTime();

    collectStatistics();

    log(QString("Program is suspended."), LOG_CATEGORY(DEBUG + GUI));
    log("Слежение остановлено", DEBUG_ROW);

    emit stopProcess();
}

void Investigator::parseReport(QString report) {

    m_lastReport = report;

    log(QString("Start report parsing"), LOG_CATEGORY(DEBUG + DEBUG_ROW));

    // если включено сохранение отчетов АВС
    if(m_saveAvsReports)
        emit saveReport(QString(m_lastReport), "autosave");

    // если отчет цельный
    if(m_lastReport.contains("Сканирование объектов: ") && m_lastReport.contains("Сканирование завершено")) {

        clearParserTemps();
        m_lastProcessWorkTimeInSec = m_lastProcessStartTime.secsTo(QDateTime::currentDateTime());

        m_reportLines = m_lastReport.split("\n");

        // версии баз и ядер
        if(m_reportLines.size() > 5) {
            m_baseVersion       = m_reportLines[2].remove("Версия баз: ");
            m_m52coreVersion    = m_reportLines[3].remove("Версия ядра M-52: ");
            m_drwebCoreVersion  = m_reportLines[4].remove("Версия ядра Dr.Web: ");
            m_kasperCoreVersion = m_reportLines[5].remove("Версия ядра Касперский: ");

            m_baseVersion.chop(1);
            m_m52coreVersion.chop(1);

            m_drwebCoreVersion.truncate(m_drwebCoreVersion.lastIndexOf(" количество записей"));
            m_drwebCoreVersion.replace(m_drwebCoreVersion.lastIndexOf(","), 1, ")");
            m_drwebCoreVersion.replace("база ", "");

            m_kasperCoreVersion.truncate(m_kasperCoreVersion.lastIndexOf(" количество записей"));
            m_kasperCoreVersion.replace(m_kasperCoreVersion.lastIndexOf(","), 1, ")");
            m_kasperCoreVersion.replace("база ", "");

            QString newVersion = QString("Версия баз: %1\nЯдро M-52: %2\nЯдро Dr.Web: %3\nЯдро Kaspersky: %4").arg(m_baseVersion).arg(m_m52coreVersion).arg(m_drwebCoreVersion).arg(m_kasperCoreVersion);
            if(m_avVersion != newVersion) {
                m_avVersion = newVersion;
                log(QString("AVS version: %1").arg(newVersion), DEBUG);
            }
        }

        // накопление статистики по последнему блоку сканирования
        if(m_isWorking) {
            m_processedFilesNb += QDir(m_processDir).entryList(usingFilters).size();
            foreach(QFileInfo fi, QDir(m_processDir).entryInfoList(usingFilters)) {
                m_lastProcessedFilesSizeMb += double(fi.size()) / 1024. / 1024.;
            }
            m_processedFilesSizeMb += m_lastProcessedFilesSizeMb;

            m_averageProcessSpeed = m_startTime.secsTo(getEndTime()) ? (m_processedFilesSizeMb / m_startTime.secsTo(getEndTime())) : 0;
            m_currentProcessSpeed = m_lastProcessWorkTimeInSec ? (m_lastProcessedFilesSizeMb / m_lastProcessWorkTimeInSec) : 0;
            emit updateUi();
        }

        // если есть зараженные файлы
        if(m_reportLines.size() > 13) {

            emit saveReport(QString(m_lastReport), "infected");

            for(int i = 6; i < m_reportLines.size() - 7; i++) {
                // если часть строки репорта содержит путь к папке проверки, то в этой строке инфа о зараженном файле
                if(m_reportLines[i].contains(QDir::toNativeSeparators(m_processDir)) && m_reportLines[i].contains("M-52:")) {
                    m_tempSplitList = m_reportLines[i].split(QDir::toNativeSeparators(m_processDir) + "\\"); // разделитель - путь к папке с файлами
                    m_tempSplitList = m_tempSplitList[1].split("'");

                    if(m_tempSplitList[0].contains("//")) {
                        m_tempFileName = m_tempSplitList[0].split("//")[0];
                    } else {
                        m_tempFileName = m_tempSplitList[0];
                    }

                    // извлечение информации о вирусе
                    m_tempSplitList = m_reportLines[i].split("инфицирован ");
                    if(m_tempSplitList.size()) {
                        m_tempVirusInfo = m_tempSplitList[1];

                        m_tempVirusInfo.remove(" - Файл пропущен");
                        m_tempVirusInfo.truncate(m_tempVirusInfo.lastIndexOf(")") + 1);

                        // в список зараженных файлов файл добавляется только в том случае, если его там еще нет
                        if(!isContainedFile(m_infectedFiles, m_tempFileName) && m_tempVirusInfo.length() > 3) {
                            m_infectedFiles.push_back(QPair<QString,QString>{m_tempFileName, m_tempVirusInfo});
                        }
                    }
                }
            }
        }

        m_infectedFilesNb += m_infectedFiles.size();

        // поиск ошибок сканирования
        bool existLineWithError = false;
        for(int i = m_reportLines.size() - 1; i > 0; i++) {
            if(m_reportLines[i].contains("Ошибки сканирования: ")) {
                existLineWithError = true;
                QString tempStr = m_reportLines[i].remove("Ошибки сканирования: ");
                tempStr.chop(1);
                m_scanningErrorFilesNb += tempStr.toInt();
            }
        }
        if(!existLineWithError) {
            log(QString("Не найдена информация об ошибках сканирования."), LOG_CATEGORY(GUI));
            saveReport(m_lastReport, "scanError");
        }


        // обработка зараженных
        foreach(auto infectedFile, m_infectedFiles) {

            log(QString("Detected infected file: %1 %2.").arg(infectedFile.first).arg(infectedFile.second), LOG_CATEGORY(GUI + DEBUG));

            switch(m_infectedFileAction) {

                case MOVE_TO_DIR:
                    log(QString("Transferring file %1 into directory for infected files(%2).").arg(infectedFile.first).arg(m_dangerDir), LOG_CATEGORY(DEBUG));
                    moveFile(infectedFile.first, m_processDir, m_dangerDir);

                    if(m_useExternalHandler) {
                        emit startExternalHandler(m_externalHandlerPath,
                                                  QStringList()
                                                    << QString("'%1'").arg(m_dangerDir + "/" + infectedFile.first)
                                                    << QString("'%1'").arg(infectedFile.second)
                                                    << QString("'%1'").arg(m_baseVersion));
                    }
                    break;

                case DELETE:
                    if(!QFile(m_processDir + "/" + infectedFile.first).remove())
                        log(QString("Can't deleting infected file %1.").arg(infectedFile.first), LOG_CATEGORY(GUI + DEBUG));
                    else
                        log(QString("File %1 has been deleted.").arg(infectedFile.first), LOG_CATEGORY(DEBUG));
                    break;
            }
        }

        // перенос чистых
        QStringList clearFiles = QDir(m_processDir).entryList(usingFilters);
        log(QString("Transferring %1 checked files into clean directory(%2): %3").arg(QDir(m_processDir).entryList(usingFilters).size())
                                                                             .arg(m_cleanDir)
                                                                             .arg(entryListToString(clearFiles)), LOG_CATEGORY(DEBUG));
        // перенос только тех, которые отправлялись на проверку
        foreach(QString fileName, clearFiles) {
            if(m_inProcessFileList.contains(fileName)) {
                moveFile(fileName, m_processDir, m_cleanDir);
            }
        }

        // все оставшиеся файлы возвращаются обратно в m_inputDir
        moveFiles(m_processDir, m_inputDir, ALL_FILES);

        collectStatistics();

        log(getCurrentStatistic(), LOG_CATEGORY(DEBUG));

        // возвращение кол-ва переносимых файлов на классический минимум
        if(m_maxQueueSize < 20) {
            m_maxQueueSize = 20;
        }
    } else {

        if(m_maxQueueSize == 1) {
            QStringList problemFiles = QDir(m_dangerDir).entryList(usingFilters);
            log(QString("Error with processing file %1. Move to infected...")
                        .arg(entryListToString(problemFiles)),
                LOG_CATEGORY(DEBUG + GUI));

            moveFiles(m_processDir, m_dangerDir, ALL_FILES);
        } else {
            log(QString("Error with report parsing: report is broken. Files will be checked again. Queue size changed form %1 to %2 files.")
                        .arg(m_maxQueueSize)
                        .arg(m_maxQueueSize/2),
                LOG_CATEGORY(DEBUG + GUI));

            if(m_maxQueueSize > 2) {
                m_maxQueueSize /= 2;
            }
            moveFiles(m_processDir, m_inputDir, ALL_FILES);
        }
    }

    m_isInProcess = false;
    log("", LOG_CATEGORY(DEBUG_ROW));

    onProcessFinished();
}

void Investigator::sendSyslogMessage(QString msg, int pri) {
    if(m_useSyslog && m_syslogSocket && checkSyslogAddress()) {
        msg.prepend(QString("<%1>%2 %3 ").arg(pri).arg(QNetworkInterface::allAddresses().first().toString()).arg(formattedCurrentDateTime()));
        QByteArray m_msg = msg.toUtf8();
        m_syslogSocket->writeDatagram(m_msg, m_msg.size(), QHostAddress(m_syslogIpAddress), 514);
    }
}

void Investigator::clearParserTemps() {
    m_infectedFiles.clear();
    m_tempSplitList.clear();
    m_reportLines.clear();
    m_tempFileName.clear();
    m_tempVirusInfo.clear();
    m_lastProcessedFilesSizeMb = 0.;
}

QString Investigator::getReportFileName(QString baseName) {
    return QString("%1report_%2_%3_(%4).txt")
            .arg(m_reportsDir + "/")
            .arg(baseName)
            .arg(QString::number(m_reportCnt++))
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss"));
}

QString Investigator::getCurrentStatistic() {
    return QString("Work time: %1, cheked %2 files with total volume: %3, detected %4 infected files, files in queue: %5, error with scanning: %6.")
                                              .arg(m_workTimeEn)
                                              .arg(m_processedFilesNb)
                                              .arg(volumeToString(m_processedFilesSizeMb))
                                              .arg(m_infectedFilesNb)
                                              .arg(m_inQueueFilesNb)
                                              .arg(m_scanningErrorFilesNb);
}

QString Investigator::getWorkTime() {
    return m_workTimeEn;
}

int Investigator::getInfectedFilesNb() {
    return m_infectedFilesNb;
}

int Investigator::getProcessedFilesNb() {
    return m_processedFilesNb;
}

long long Investigator::getProcessedFilesSizeMb() {
    return m_processedFilesSizeMb;
}
