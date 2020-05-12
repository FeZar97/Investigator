#include "investigator.h"

void moveFiles(QString sourceDir, QString destinationDir, int limit) {

    if(!QDir(sourceDir).exists() || !QDir(destinationDir).exists())
        return;

    QFileInfoList filesInSourceDir = QDir(sourceDir + "/").entryInfoList(usingFilters),
                  filesInDestinationDir = QDir(destinationDir + "/").entryInfoList(usingFilters);

    QDateTime startMoveTime = QDateTime::currentDateTime();

    limit = qMin(limit, filesInSourceDir.size());
    if(limit > 0) {
        filesInSourceDir = filesInSourceDir.mid(0, limit);
    }

    while(!filesInSourceDir.isEmpty()) {
        foreach(QFileInfo fileInfo, filesInSourceDir) {
            QFileInfo oldFileInfo(fileInfo);
            if(QFile(fileInfo.absoluteFilePath()).exists()) {
                if(QFile::rename(fileInfo.absoluteFilePath(), destinationDir + "/" + fileInfo.fileName())) {
                    filesInSourceDir.removeAll(oldFileInfo);
                }
            }
        }

        if(startMoveTime.msecsTo(QDateTime::currentDateTime()) > qMin(filesInSourceDir.size() * 3000, 60 * 1000) && filesInSourceDir.size()) {
            break;
        }
    }
}

void moveFile(QString fileName, QString sourceDir, QString destinationDir) {
    if(!QFile(sourceDir + "/" + fileName).exists()
            || !QDir(sourceDir).exists()
            || !QDir(destinationDir).exists())
        return;

    QDateTime startMoveTime = QDateTime::currentDateTime();

    while((QDir(sourceDir).entryList()).contains(fileName)) {
        QFile::rename(sourceDir + "/" + fileName, destinationDir + "/" + fileName);

        if(startMoveTime.msecsTo(QDateTime::currentDateTime()) >  10 * 1000 || QFile(destinationDir + fileName).exists()) {
            break;
        }
    }
}

QString currentDateTime() {
    return QDateTime::currentDateTime().toString(dateTimePattern);
}

double dirSizeMb(QString dirName) {
    double volMb{0};
    foreach(QFileInfo fileInfo, QDir(dirName).entryInfoList(usingFilters)) {
        volMb += fileInfo.size();
    }
    volMb = (volMb * 8 / 1024) / 1024;
    return volMb;
}

Investigator::Investigator(QObject *parent) : QObject(parent){
    qRegisterMetaType<MSG_CATEGORY>("MSG_CATEGORY");
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
        log("Watching directory is not selected!", MSG_CATEGORY(DEBUG + LOG_GUI));
        problem = true;
    }
    if(!QDir(m_watchDir).exists()) {
        log("Watching directory does not exist!", MSG_CATEGORY(DEBUG + LOG_GUI));
        problem = true;
    }

    if(m_investigatorDir.isEmpty()) {
        log("Temporary directory is not selected!", MSG_CATEGORY(DEBUG + LOG_GUI));
        problem = true;
    }
    if(!QDir(m_investigatorDir).exists()) {
        log("Temporary directory does not exist!", MSG_CATEGORY(DEBUG + LOG_GUI));
        problem = true;
    }

    if(m_cleanDir.isEmpty()) {
        log("Directory for clean files is not selected!", MSG_CATEGORY(DEBUG + LOG_GUI));
        problem = true;
    }
    if(!QDir(m_cleanDir).exists()) {
        log("Directory for clean files does not exist!", MSG_CATEGORY(DEBUG + LOG_GUI));
        problem = true;
    }

    if(m_dangerDir.isEmpty()) {
        log("Directory for infected files is not selected!", MSG_CATEGORY(DEBUG + LOG_GUI));
        problem = true;
    }
    if(!QDir(m_dangerDir).exists()) {
        log("Directory for infected files does not exist!", MSG_CATEGORY(DEBUG + LOG_GUI));
        problem = true;
    }

    if(m_avPath.isEmpty()) {
        log("AVS executable file is not selected!", MSG_CATEGORY(DEBUG + LOG_GUI));
        problem = true;
    }
    if(!QFile(m_avPath).exists()) {
        log("AVS executable file does not exist!", MSG_CATEGORY(DEBUG + LOG_GUI));
        problem = true;
    }

    if(m_isWorking) {
        log("Watching has already begun!", MSG_CATEGORY(DEBUG + LOG_GUI));
        problem = true;
    }

    if(m_isInProcess) {
        log("AVS process has already begun!", MSG_CATEGORY(DEBUG + LOG_GUI));
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

void Investigator::clearStatistic() {
    m_startTime = QDateTime::currentDateTime();
    m_endTime = QDateTime::currentDateTime();

    m_infectedFilesNb = 0;
    m_inProgressFilesNb = 0;
    m_processedFilesNb = 0;
    m_averageProcessSpeed = 0;
    m_processedFilesSizeMb = 0;
    m_processedLastFilesSizeMb = 0;
    m_currentProcessSpeed = 0;
}

void Investigator::configureDirs() {
    if(m_investigatorDir.isEmpty()) {
        log(QString("Can not creating temporary dirs, because investigatorDir is not choosed."), MSG_CATEGORY(DEBUG + LOG_GUI));
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

    int filesNumber = QDir(m_inputDir).entryInfoList(usingFilters).size();

    // если слежение запущено, процесс не занят и есть что переносить
    if(m_isWorking && !m_isInProcess && filesNumber) {

        m_isInProcess = true;

        log(QString("Transferring %1 files from inputDir(%2) into processDir(%3) with limit %4 files.").arg(filesNumber)
                                                                                                       .arg(m_inputDir)
                                                                                                       .arg(m_processDir)
                                                                                                       .arg(m_maxQueueSize), MSG_CATEGORY(DEBUG + LOG_ROW));
        m_inProcessFileList.clear();
        m_inProcessFileList = QDir(m_inputDir).entryList(usingFilters);

        if(m_inProcessFileList.size()) {
            log(QString("Checking files: %1").arg(entryListToString(m_inProcessFileList)), MSG_CATEGORY(DEBUG));
        }

        moveFiles(m_inputDir, m_processDir, m_maxQueueSize);

        m_inQueueFileSizeMb = 0;
        foreach(QFileInfo fi, QDir(m_processDir).entryInfoList(usingFilters)) {
            m_inQueueFileSizeMb += double(fi.size()) / 1024. / 1024.;
        }

        // если каталог с файлами не пуст
        if(!QDir(m_processDir).isEmpty()) {
            log("Start checking...", MSG_CATEGORY(DEBUG + LOG_ROW));
            emit process(QDir::toNativeSeparators(m_avPath), QStringList() << QDir::toNativeSeparators(m_processDir));
        }
    }

    collectStatistics();
}

void Investigator::collectStatistics() {
    m_inQueueFilesNb = QDir(m_inputDir).entryList(usingFilters).size();
    m_inProgressFilesNb = QDir(m_processDir).entryList(usingFilters).size();

    emit updateUi();
}

bool Investigator::beginWork() {
    if(checkAvParams()) {
        clearStatistic();
        log(QString("Start watching to directory %1.").arg(m_watchDir), MSG_CATEGORY(DEBUG + LOG_GUI));
        m_isWorking = true;
        return true;
    } else {
        log(QString("Failed start watching to directory %1.").arg(m_watchDir), MSG_CATEGORY(DEBUG + LOG_GUI));
        return false;
    }
}

void Investigator::stopWork() {
    m_isWorking = false;
    m_endTime = QDateTime::currentDateTime();

    collectStatistics();

    log(QString("Program is suspended."), MSG_CATEGORY(DEBUG + LOG_GUI));

    emit stopProcess();
}

void Investigator::parseReport(QString report) {
    m_lastReport = report;

    log(QString("Start report pearsing"), DEBUG);
    if(m_saveAvsReports)
        emit saveReport(QString(m_lastReport));

    if(m_lastReport.contains("Сканирование объектов: ") && m_lastReport.contains("Сканирование завершено")) {

        clearParserTemps();
        m_lastProcessWorkTimeInSec = m_lastProcessStartTime.secsTo(QDateTime::currentDateTime());

        m_reportLines = m_lastReport.split("\n");

        if(m_reportLines.size() > 5) {
            m_baseVersion       = m_reportLines[2].remove("Версия баз: ");
            m_m52coreVersion    = m_reportLines[3].remove("Версия ядра M-52: ");
            m_drwebCoreVersion  = m_reportLines[4].remove("Версия ядра Dr.Web: ");
            m_kasperCoreVersion = m_reportLines[5].remove("Версия ядра Касперский: ");

            m_baseVersion.chop(1);
            m_m52coreVersion.chop(1);

            m_drwebCoreVersion.truncate(m_drwebCoreVersion.lastIndexOf(" количество записей"));
            m_drwebCoreVersion.replace(m_drwebCoreVersion.lastIndexOf(","), 1, ")");

            m_kasperCoreVersion.truncate(m_kasperCoreVersion.lastIndexOf(" количество записей"));
            m_kasperCoreVersion.replace(m_kasperCoreVersion.lastIndexOf(","), 1, ")");

            QString newVersion = QString("Версия баз: %1\nЯдро M-52: %2\nЯдро Dr.Web: %3\nЯдро Kaspersky: %4").arg(m_baseVersion).arg(m_m52coreVersion).arg(m_drwebCoreVersion).arg(m_kasperCoreVersion);
            if(m_avVersion != newVersion) {
                m_avVersion = newVersion;
                log(QString("AVS version: %1").arg(newVersion), DEBUG);
            }
        }

        // accumulate statistic
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

            emit saveReport(QString(m_lastReport), QTime::currentTime().toString("hh-mm-ss"));

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

        // обработка зараженных
        foreach(auto infectedFile, m_infectedFiles) {

            log(QString("Detected infected file: %1 %2.").arg(infectedFile.first).arg(infectedFile.second), MSG_CATEGORY(LOG_GUI + INFO));

            switch(m_infectedFileAction) {

                case MOVE_TO_DIR:
                    log(QString("Transferring file %1 into directory for infected files(%2).").arg(infectedFile.first).arg(m_dangerDir), MSG_CATEGORY(DEBUG));
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
                        log(QString("Can't deleting infected file %1.").arg(infectedFile.first), MSG_CATEGORY(LOG_GUI + DEBUG));
                    else
                        log(QString("File %1 has been deleted.").arg(infectedFile.first), MSG_CATEGORY(DEBUG));
                    break;
            }
        }

        // перенос чистых
        QStringList clearFiles = QDir(m_processDir).entryList(usingFilters);
        log(QString("Transferring %1 checked files into clean directory(%2): %3").arg(QDir(m_processDir).entryList(usingFilters).size())
                                                                             .arg(m_cleanDir)
                                                                             .arg(entryListToString(clearFiles)), MSG_CATEGORY(DEBUG));
        // перенос только тех, которые отправлялись на проверку
        foreach(QString fileName, clearFiles) {
            if(m_inProcessFileList.contains(fileName)) {
                moveFile(fileName, m_processDir, m_cleanDir);
            }
        }

        // все оставшиеся файлы возвращаются обратно в m_inputDir
        moveFiles(m_processDir, m_inputDir, ALL_FILES);

        collectStatistics();

        log(QString("Work time: %1, cheked %2 files with total volume: %3, detected %4 infected files.").arg(m_workTimeEn)
                                                                                                        .arg(m_processedFilesNb)
                                                                                                        .arg(volumeToString(m_processedFilesSizeMb))
                                                                                                        .arg(m_infectedFilesNb), MSG_CATEGORY(INFO));
    } else {
        log(QString("Error with report parsing: report is broken. Files will be checked again. Queue size changed form %1 to %2 files.").arg(m_maxQueueSize).arg(m_maxQueueSize/2), MSG_CATEGORY(DEBUG));

        if(m_maxQueueSize > 10) {
            m_maxQueueSize /= 2;
        }
        moveFiles(m_processDir, m_inputDir, ALL_FILES);
    }

    m_isInProcess = false;

    onProcessFinished();
}

void Investigator::sendSyslogMessage(QString msg, int pri) {
    if(m_useSyslog && m_syslogSocket && checkSyslogAddress()) {
        msg.prepend(QString("<%1>%2 %3 ").arg(pri).arg(QNetworkInterface::allAddresses().first().toString()).arg(currentDateTime()));
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
    return QString("%1report_%2_(%3).txt")
            .arg(m_reportsDir + "/")
            .arg(baseName.isEmpty() ? QString::number(m_reportCnt++) : baseName)
            .arg(QDate::currentDate().toString("dd.MM.yy"));
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

QString entryListToString(QStringList &list) {
    QString res = "";
    if(list.size()) {
        for(int i = 0; i < list.size() - 1; i++) {
            res += list[i] + ", ";
        }
        res += list[list.size() - 1] + ".";
    }
    return res;
}

bool isContainedFile(QList<QPair<QString, QString>>& fileList, QString fileName) {
    foreach(auto pair, fileList)
        if(pair.first == fileName)
            return true;
    return false;
}

QString volumeToString(double volumeInMb) {
    if(volumeInMb < (1 << 10)) {
        return QString("%1 MB").arg(QString::number(volumeInMb, 'f', 2));
    } else if (volumeInMb/(1 << 10) < (1 << 10)) {
        return QString("%1 GB").arg(QString::number(volumeInMb/(1 << 10), 'f', 2));
    } else {
        return QString("%1 TB").arg(QString::number(volumeInMb/(1 << 20), 'f', 2));
    }
}
