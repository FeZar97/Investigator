 #include "avwrapper.h"

// ----------------------------------------- AVRecord -----------------------------------------

AVRecord::AVRecord() {
    m_timeMark = QDateTime::currentDateTime();
    m_av = AV::NONE;
    m_fileName = "";
    m_description = "";
    m_reportName = "";
}

AVRecord::AVRecord(QDateTime timeMark, AV av, QString fileName, QString description, QString reportName) {
    m_timeMark = timeMark;
    m_av = av;
    m_fileName = fileName;
    m_description = description;
    m_reportName = reportName;
}

AVRecord::AVRecord(const AVRecord &record){
    m_timeMark = record.m_timeMark;
    m_av = record.m_av;
    m_fileName = record.m_fileName;
    m_description = record.m_description;
    m_reportName = record.m_reportName;
}

QString AVRecord::toString() {
    return m_timeMark.toString(dateTimePattern) + " " +
           m_fileName + " " +
           m_description + " " +
            "(" + getName(m_av) + " " + QFileInfo(m_reportName).baseName() + ")";
}

AVRecord &AVRecord::operator=(const AVRecord &record) {
    if(&record == this)
        return *this;

    m_timeMark = record.m_timeMark;
    m_av = record.m_av;
    m_fileName = record.m_fileName;
    m_description = record.m_description;
    m_reportName = record.m_reportName;

    return *this;
}

// ----------------------------------------- AVBase -----------------------------------------
int AVBase::findFileName(QString fileName) {
    for(int  i = 0; i < m_base.size(); i++) {
        if(m_base.at(i).first.m_fileName == fileName || m_base.at(i).second.m_fileName == fileName) {
            return i;
        }
    }
    return -1;
}

void AVBase::add(AVRecord record) {
    int idx = findFileName(record.m_fileName);
    if(idx != -1) {
        if(record.m_av == AV::KASPER)
            m_base[idx] = QPair<AVRecord, AVRecord>(record, m_base[idx].second);
        if(record.m_av == AV::DRWEB)
            m_base[idx] = QPair<AVRecord, AVRecord>(m_base[idx].first, record);
    } else {
        if(record.m_av == AV::KASPER)
            m_base.push_back(QPair<AVRecord, AVRecord>(record, AVRecord()));
        if(record.m_av == AV::DRWEB)
            m_base.push_back(QPair<AVRecord, AVRecord>(AVRecord(), record));
    }
}

void AVBase::add(QPair<AVRecord, AVRecord>& record) {
    if(findFileName(record.first.m_fileName)  == -1 &&
       findFileName(record.second.m_fileName) == -1) {
        m_base.push_back(record);
    }
}

QPair<AVRecord, AVRecord>& AVBase::operator[](int idx) {
    return m_base[idx];
}

int AVBase::size() {
    return m_base.size();
}

// ----------------------------------------- AVWrapper -----------------------------------------

AVWrapper::AVWrapper(QObject *parent) : QObject(parent) {
}

void AVWrapper::setType(AV type) {
    m_type = type;
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
                           QString outputFolder,
                           QString reportFolder) {
    setInvestigatorFolder(investigatorDir);
    setInputFolder(inputFolder);
    setProcessFolder(processFolder);
    setOutputFolder(outputFolder);
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
    if(QFile::exists(m_reportName))         return 7;
    if(m_reportReadyIndicator.isEmpty())    return 8;
    if(m_startRecordsIndicator.isEmpty())   return 9;
    if(m_endRecordsIndicator.isEmpty())     return 10;
    return 0;
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

void moveFiles(QString sourceDir, QString destinationDir) {

    if(!QDir(sourceDir).exists() || !QDir(destinationDir).exists())
        return;

    QFileInfoList filesInSourceDir = QDir(sourceDir + "/").entryInfoList(usingFilters);

    while(!filesInSourceDir.isEmpty()) {
        foreach(QFileInfo fileInfo, filesInSourceDir) {
            if(QFile::rename(fileInfo.absoluteFilePath(), destinationDir + "/" + fileInfo.fileName())) {
                filesInSourceDir.removeAll(fileInfo);
            }
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

    // clear temp variables
    m_processedLastFilesSizeMb = 0.;
    m_reportLine = "";
    m_report = "";
    m_inProgressFilesNb = 0;

    if(m_readyToProcess && !QDir(m_inputFolder).isEmpty()) {

        // block wrapper
        m_readyToProcess = false;

        // move files from input dir to process dir
        moveFiles(m_inputFolder, m_processFolder);

        // if av is used, then create report name
        if(m_isUsed) {

            AVBase* m_dynamicBase = new AVBase();

            // save temp  variables
            m_startProcessTime = QDateTime::currentDateTime();
            m_reportName = m_reportFolder + "/report_" + QString::number(m_reportIdx) + "." + m_reportExtension;
            m_inProgressFilesNb = QDir(m_processFolder + "/").entryInfoList(usingFilters).size();

            // if execution params is correct, then execute av
            if(checkParams()) {
                log(currentDateTime() + " " + QString("Ошибка в параметрах запуска антивируса(%1).").arg(QString::number(checkParams())));
                return;
            }

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

            // calculate statistics
            m_reportIdx++;
            foreach(QFileInfo fileInfo, QDir(m_processFolder).entryInfoList(usingFilters))
                m_processedLastFilesSizeMb += fileInfo.size() / (1024. * 1024.);
            m_processedFilesSizeMb += m_processedLastFilesSizeMb;
            m_processedFilesNb += m_inProgressFilesNb;
            m_currentProcessSpeed = m_processedLastFilesSizeMb * 8 * 1000 / m_startProcessTime.msecsTo(QDateTime::currentDateTime());
            m_totalWorkTimeInMsec += m_startProcessTime.msecsTo(QDateTime::currentDateTime());

            m_reportFile.setFileName(m_reportName);

            // wait for report ready
            while(!m_report.contains(m_reportReadyIndicator)) {
                if(m_reportFile.open(QIODevice::ReadOnly)) {
                    m_stream.setDevice(&m_reportFile);
                    m_report = m_stream.readAll();
                    m_reportFile.close();
                }
            }

            // parse report file
            if(m_reportFile.open(QIODevice::ReadOnly)) {
                m_stream.setDevice(&m_reportFile);

                // seek to position with infected files information
                m_stream.seek(m_report.indexOf(m_startRecordsIndicator));

                // extract info from report file
                do {
                    m_reportLine = m_stream.readLine();

                    if(isPayload(m_reportLine)) {
                        QString infectedFileName = extractInfectedFileName(m_reportLine);

                        if(!infectedFileName.isEmpty() && m_dynamicBase->findFileName(infectedFileName) == -1) {

                            m_dangerFileNb++;

                            m_dynamicBase->add(AVRecord(QDateTime::currentDateTime(),
                                                        m_type,
                                                        infectedFileName,
                                                        extractDescription(m_reportLine, extractInfectedFileName(m_reportLine)),
                                                        QFileInfo(m_reportName).fileName()));

                            if(!QFile::rename(m_processFolder + "/" + infectedFileName, m_dangerFolder + "/" + infectedFileName)) {
                                log("Не удалось перенести зараженный файл " + infectedFileName);
                            }
                        }
                    }
                } while(!m_reportLine.contains(m_endRecordsIndicator));

                m_reportFile.close();
            }

            emit updateBase(m_dynamicBase);
        }

        moveFiles(m_processFolder, m_outputFolder);

        m_readyToProcess = true;

        emit finishProcess();

        if(QDir(m_inputFolder).entryList(usingFilters).size())
            process();
    }
}
