 #include "avwrapper.h"

void AVWrapper::flushTempVariables() {
    m_processedLastFilesSizeMb = 0.;
    m_reportLine = "";
    m_report = "";
    m_inProgressFilesNb = 0;
}

void AVWrapper::saveTempVariables() {
    m_startProcessTime = QDateTime::currentDateTime();
    m_inProgressFilesNb = QDir(m_processFolder + "/").entryInfoList(usingFilters).size();
}

void AVWrapper::executeAVProgram() {

    logWrapper(QString("Запуск проверки %1").arg(getName(m_type)), LOG_DST(LOG_FILE | LOG_ROW));

    switch(m_type) {
        case AV::KASPER:
            QProcess::execute(m_avPath, QStringList() << "scan" << m_processFolder << "/i0" << QString("/R:" + m_reportName));
            break;
        case AV::DRWEB:
            QProcess::execute(m_avPath, QStringList() << QString("/RP:" + m_reportName) << m_processFolder);
            break;
        default:
            break;
    }
}

void AVWrapper::waitForReportReady() {

    logWrapper(QString("Ожидание отчета %1(%2 файлов)").arg(m_reportName).arg(m_inProgressFilesNb), LOG_DST(LOG_FILE | LOG_ROW));

    while(!m_report.contains(m_reportReadyIndicator)) {
        if(m_reportFile.open(QIODevice::ReadOnly)) {
            m_stream.setDevice(&m_reportFile);
            m_report = m_stream.readAll();
            m_reportFile.close();
        }

        if(m_startProcessTime.msecsTo(QDateTime::currentDateTime()) > qMin(m_inProgressFilesNb * 8000, 60 * 1000) && m_inProgressFilesNb) {
            logWrapper(QString("%1 завис на отчете %2(%3 файлов), выход из цикла проверки").arg(getName(m_type)).arg(m_reportName).arg(m_inProgressFilesNb),
                       LOG_DST(LOG_GUI | LOG_FILE | LOG_ROW));
            break;
        }
    }
}

void AVWrapper::parseReportFile() {

    if(m_reportFile.exists()) {

        logWrapper(QString("Разбор отчета %1").arg(m_reportName), LOG_DST(LOG_FILE | LOG_ROW));

        if(m_reportFile.size() == 0) {
            m_hasStarvation = true;
        } else {
            accumulateStatistic();

            if(m_reportFile.open(QIODevice::ReadOnly)) {
                m_stream.setDevice(&m_reportFile);

                // seek to position with infected files information
                m_stream.seek(m_report.indexOf(m_startRecordsIndicator));

                // extract info from report file
                do {
                    m_reportLine = m_stream.readLine();

                    if(isPayload(m_reportLine)) {
                        QString infectedFileName = extractInfectedFileName(m_reportLine);

                        if(!infectedFileName.isEmpty()) {

                            m_dangerFileNb++;

                            logWrapper(QString("%1 %2 (%3 %4)").arg(infectedFileName)
                                                               .arg(extractDescription(m_reportLine, extractInfectedFileName(m_reportLine)))
                                                               .arg(getName(m_type)).arg(QFileInfo(m_reportName).baseName()),
                                       LOG_GUI);

                            if(!QFile::rename(m_processFolder + "/" + infectedFileName, m_dangerFolder + "/" + infectedFileName)) {
                                logWrapper(QString("Не удалось перенести зараженный файл %1").arg(infectedFileName), LOG_DST(LOG_FILE | LOG_ROW));
                            }
                            flushTempVariables();
                        }
                    }
                } while(!m_stream.atEnd());

                m_reportFile.close();
            }
        }

    } else {
        logWrapper(QString("Отчет %1 не существует").arg(m_reportName), LOG_DST(LOG_ROW | LOG_FILE));
        m_reportIdx--;
    }
}

void AVWrapper::accumulateStatistic() {
    foreach(QFileInfo fileInfo, QDir(m_processFolder).entryInfoList(usingFilters))
        m_processedLastFilesSizeMb += fileInfo.size() / (1024. * 1024.);
    m_processedFilesSizeMb += m_processedLastFilesSizeMb;
    m_processedFilesNb += m_inProgressFilesNb;
    m_currentProcessSpeed = m_processedLastFilesSizeMb * 8 * 1000 / m_startProcessTime.msecsTo(QDateTime::currentDateTime());
    m_totalWorkTimeInMsec += m_startProcessTime.msecsTo(QDateTime::currentDateTime());
}

AVWrapper::AVWrapper(QObject *parent) : QObject(parent) {
    flushTempVariables();
}

void AVWrapper::setType(AV type) {
    m_type = type;
}

void AVWrapper::connectProcessingFlag(bool* isProcessing) {
    m_isProcessing = isProcessing;
}

void AVWrapper::setMaxQueueSize(int size) {
    m_maxQueueSize = size;
}

int AVWrapper::getMaxQueueSize() {
    return m_maxQueueSize;
}

void AVWrapper::setMaxQueueVolMb(double volMb) {
    m_maxQueueVolMb = volMb;
}

double AVWrapper::getMaxQueueVolMb() {
    return m_maxQueueVolMb;
}

void AVWrapper::setMaxQueueVolUnit(int maxQueueVolUnit) {
    m_maxQueueVolUnit = maxQueueVolUnit;
}

int AVWrapper::getMaxQueueVolUnit() {
    return m_maxQueueVolUnit;
}

void AVWrapper::setUsage(bool newState) {
    m_isUsed = newState;
}

bool AVWrapper::getUsage() {
    return m_isUsed;
}

void AVWrapper::setAVPath(QString avPath) {
    m_avPath = avPath;
}

QString AVWrapper::getAVPath() {
    return m_avPath;
}

void AVWrapper::setReportExtension(QString extension) {
    m_reportExtension = extension;
}

QString AVWrapper::getReportExtension() {
    return m_reportExtension;
}

void AVWrapper::setReportFolder(QString reportFolder){
    m_reportFolder = reportFolder;
}

QString AVWrapper::getReportFolder() {
    return m_reportFolder;
}

void AVWrapper::setInputFolder(QString inputFolder) {
    m_inputFolder = inputFolder;
    if(!m_watcher.directories().isEmpty()) {
        m_watcher.removePaths(m_watcher.directories());
    }
    if(!m_inputFolder.isEmpty()) {
        m_watcher.addPath(m_inputFolder);
    }
}

QString AVWrapper::getInputFolder() {
    return m_inputFolder;
}

void AVWrapper::setProcessFolder(QString processFolder) {
    m_processFolder = processFolder;
}

QString AVWrapper::getProcessFolder() {
    return m_processFolder;
}

void AVWrapper::setOutputFolder(QString outputFolder) {
    m_outputFolder = outputFolder;
}

QString AVWrapper::getOutputFolder() {
    return m_outputFolder;
}

void AVWrapper::setDangerFolder(QString dangerFolder) {
    m_dangerFolder = dangerFolder;
}

QString AVWrapper::getDangerFolder() {
    return m_dangerFolder;
}

void AVWrapper::setInvestigatorFolder(QString investigatorDir) {
    m_investigatorDir = investigatorDir;
}

void AVWrapper::setFolders(QString investigatorDir,
                           QString inputFolder,
                           QString processFolder,
                           QString reportFolder) {
    setInvestigatorFolder(investigatorDir);
    setInputFolder(inputFolder);
    setProcessFolder(processFolder);
    setReportFolder(reportFolder);
}

int AVWrapper::getDangerFilesNb() {
    return m_dangerFileNb;
}

int AVWrapper::getCurrentReportIdx() {
    return m_reportIdx;
}

int AVWrapper::getProcessedFilesNb() {
    return m_processedFilesNb;
}

int AVWrapper::getInProgressFilesNb() {
    return m_inProgressFilesNb;
}

double AVWrapper::getProcessedFilesSize() {
    return m_processedFilesSizeMb;
}

double AVWrapper::getAverageSpeed() {
   if(!m_totalWorkTimeInMsec) {
       return 0;
   } else {
       return m_processedFilesSizeMb * 8  * 1000 / m_totalWorkTimeInMsec;
   }
}

double AVWrapper::getCurrentSpeed() {
    return m_currentProcessSpeed;
}

void AVWrapper::clearStatistic() {
    m_totalWorkTimeInMsec = 0;
    m_dangerFileNb = 0;
    m_reportIdx = 0;
    m_inProgressFilesNb = 0;
    m_processedFilesNb = 0;
    m_processedFilesSizeMb = 0;
    m_processedLastFilesSizeMb = 0;
    m_currentProcessSpeed = 0;
}

void AVWrapper::setExecArgs(QStringList execArgs) {
    m_execArgs = execArgs;
}

void AVWrapper::addExecArgs(QStringList additionExecArgs) {
    m_execArgs.append(additionExecArgs);
}

QStringList AVWrapper::getExecArgs() {
    return m_execArgs;
}

void AVWrapper::deleteExecArgs(QStringList delExecArgs) {
    for(auto arg: delExecArgs)
        m_execArgs.removeAll(arg);
}

int AVWrapper::checkParams() {
    if(m_reportExtension.isEmpty())         return 1;
    if(!QFile::exists(m_avPath))            return 2;
    if(!QDir(m_inputFolder).exists())       return 3;
    if(!QDir(m_processFolder).exists())     return 4;
    if(!QDir(m_outputFolder).exists())      return 5;
    if(!QDir(m_reportFolder).exists())      return 6;
    // if(QFile::exists(m_reportName))         return 7;
    if(m_reportReadyIndicator.isEmpty())    return 8;
    if(m_startRecordsIndicator.isEmpty())   return 9;
    if(m_endRecordsIndicator.isEmpty())     return 10;
    return 0;
}

bool AVWrapper::hasStarvation() {
    return m_hasStarvation;
}

bool AVWrapper::isReadyToProcess() {
    return m_readyToProcess;
}

void AVWrapper::setIndicators(QString readyIndicator, QString startRecordsIndicator, QString endRecordsIndicator, QStringList denyStrings) {
    m_reportReadyIndicator = readyIndicator;
    m_startRecordsIndicator = startRecordsIndicator;
    m_endRecordsIndicator = endRecordsIndicator;
    m_denyStrings = denyStrings;
}

bool AVWrapper::isPayload(QString line) {
    foreach(QString denyString, m_denyStrings) {
        if(!denyString.isEmpty() && line.contains(denyString)) {
            return true;
        }
    }

    return false;
}

QString AVWrapper::extractInfectedFileName(QString reportLine) {

    QString relativePath = QDir(m_investigatorDir).dirName() + "\\" + KASPER_DIR_NAME + "\\" + OUTPUT_DIR_NAME;

    switch(m_type) {
        case AV::KASPER: {
            foreach(QString part, reportLine.split("\t", QString::SkipEmptyParts)) {
                if(part.contains( relativePath )) {
                    foreach(QString archievePart, part.split("//", QString::SkipEmptyParts)) {
                        if(archievePart.contains(relativePath)) {
                            return archievePart.mid(archievePart.indexOf(relativePath) + relativePath.length() + 1);
                        }
                    }
                }
            }
            return "";
        }

        case AV::DRWEB: {
            QString fileName = reportLine.mid(QDir::toNativeSeparators(m_processFolder).length() + 1,
                                              reportLine.indexOf(QString("\\"), QDir::toNativeSeparators(m_processFolder).length() + 1) - (QDir::toNativeSeparators(m_processFolder).length() + 1));

            if(!fileName.isEmpty()) {
                if(QDir(m_processFolder + "/").exists(fileName)) {
                    return fileName;
                }
            }
            return "";
        }

        default:
            return reportLine;
    }
}

QString AVWrapper::extractDescription(QString reportLine, QString fileName) {

    switch(m_type) {
        case AV::KASPER:
            return reportLine.split("\t", QString::SkipEmptyParts).last();
        case AV::DRWEB:
            return reportLine.mid(reportLine.lastIndexOf(fileName) + fileName.length() + 1);
        default:
            return reportLine;
    }
}

void moveFiles(QString sourceDir, QString destinationDir, bool* isProcessing) {

    if(!QDir(sourceDir).exists() || !QDir(destinationDir).exists() || !*isProcessing)
        return;

    QFileInfoList filesInSourceDir = QDir(sourceDir + "/").entryInfoList(usingFilters),
                  filesInDestinationDir = QDir(destinationDir + "/").entryInfoList(usingFilters);

    QDateTime startMoveTime = QDateTime::currentDateTime();

    while(!filesInSourceDir.isEmpty()) {

        foreach(QFileInfo fileInfo, filesInSourceDir) {
            QFileInfo oldFileInfo(fileInfo);
            if(QFile::rename(fileInfo.absoluteFilePath(), destinationDir + "/" + fileInfo.fileName())) {
                filesInSourceDir.removeAll(oldFileInfo);
            }
        }

        if(startMoveTime.msecsTo(QDateTime::currentDateTime()) > qMin(filesInSourceDir.size() * 3000, 60 * 1000) && filesInSourceDir.size()) {
            // logWrapper(QString("Зависание при переносе файлов из %1 в %2...").arg(sourceDir).arg(destinationDir),
            //            LOG_DST(LOG_GUI | LOG_FILE | LOG_ROW));
            break;
        }
    }


}

QString getName(AV type) {
    switch(type) {
        case AV::KASPER:
            return "Kaspersky";
        case AV::DRWEB:
            return "DrWeb";
        default:
            return "NONE";
    }
}

QString currentDateTime() {
    return QDateTime::currentDateTime().toString(dateTimePattern);
}

void AVWrapper::process() {

    if(m_readyToProcess && !QDir(m_inputFolder).isEmpty()) {

        m_readyToProcess = false; // block wrapper

        logWrapper(QString("Перенос файлов из %1 в %2").arg(m_inputFolder).arg(m_processFolder), LOG_DST(LOG_FILE | LOG_ROW));
        moveFiles(m_inputFolder, m_processFolder, m_isProcessing);

        if(m_isUsed) {
            QString prevReportName = m_reportFolder + "/report_" + QString::number(m_reportIdx) + "." + m_reportExtension;
            m_hasStarvation = ((QFile(prevReportName).size() == 0) || (!QFile(prevReportName).exists())) && (m_reportIdx != 0);

            logWrapper(QString("Предыдущий отчет: %1, размер: %2 байт, состояние антивируса: %3").arg(prevReportName).arg(QString::number(QFile(prevReportName).size())).arg(m_hasStarvation ? "завис" : "в норме"),
                       LOG_DST(LOG_FILE));

            // если размер файла 0 или его не существует
            if(m_hasStarvation) {
                logWrapper(QString("Антивирус %1 не отвечает, переход к следующему этапу проверки").arg(getName(m_type)), LOG_DST(LOG_GUI | LOG_FILE | LOG_ROW));
            } else {
                m_reportIdx++;
                m_reportName = m_reportFolder + "/report_" + QString::number(m_reportIdx) + "." + m_reportExtension;
                m_reportFile.setFileName(m_reportName);

                // if execution params is correct, then execute av
                if(checkParams()) {
                    logWrapper(QString("Ошибка в параметрах вызова антивируса(%1)").arg(QString::number(checkParams())), LOG_DST(LOG_GUI | LOG_FILE | LOG_ROW));
                } else {
                    saveTempVariables();
                    executeAVProgram();
                    waitForReportReady();
                    parseReportFile();
                    flushTempVariables();
                }
            }
        }

        logWrapper(QString("Перенос файлов из %1 в %2").arg(m_processFolder).arg(m_outputFolder), LOG_DST(LOG_FILE | LOG_ROW));
        moveFiles(m_processFolder, m_outputFolder, m_isProcessing);

        m_readyToProcess = true;

        emit finishProcess();
    }
}
