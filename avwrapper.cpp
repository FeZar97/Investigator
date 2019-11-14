 #include "avwrapper.h"

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

AVWrapper::AVWrapper(QObject *parent) : QObject(parent) {}

void AVWrapper::setType(AV type) {
    m_type = type;
}

void AVWrapper::setMaxQueueSize(int size) {
    m_maxQueueSize = size;
}

int AVWrapper::getMaxQueueSize() {
    return m_maxQueueSize;
}

void AVWrapper::setMaxQueueVol(double vol) {
    m_maxQueueVol = vol;
}

double AVWrapper::getMaxQueueVol() {
    return m_maxQueueVol;
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

int AVWrapper::getDangerFilesNb() {
    return m_dangerFileNb;
}

int AVWrapper::getCurrentReportIdx() {
    return m_reportIdx;
}

int AVWrapper::getProcessedFilesNb() {
    return m_processedFilesNb;
}

int AVWrapper::getInprogressFilesNb() {
    return m_inprogressFilesNb;
}

double AVWrapper::getProcessedFilesSize() {
    return m_processedFilesSizeMb;
}

double AVWrapper::getAverageSpeed(qint64 workTime) {
   if(!workTime) {
       return 0;
   } else {
       return m_processedFilesSizeMb * 8 / workTime;
   }
}

double AVWrapper::getCurrentSpeed() {
    return m_currentProcessSpeed;
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

void AVWrapper::setIndicators(QString readyIndicator, QString startRecordsIndicator, QString endRecordsIndicator, QStringList permitStrings) {
    m_reportReadyIndicator = readyIndicator;
    m_startRecordsIndicator = startRecordsIndicator;
    m_endRecordsIndicator = endRecordsIndicator;
    m_permitStrings = permitStrings;
}

bool AVWrapper::isPayload(QString line) {
    foreach(QString permitString, m_permitStrings) {
        if(!permitString.isEmpty() && line.contains(permitString)) {
            return false;
        }
    }
    return true;
}

QString AVWrapper::extractFileName(QString reportLine) {

    switch(m_type) {
        case AV::KASPER: {
            foreach(QString part, reportLine.split("\t", QString::SkipEmptyParts)) {
                if(part.contains(QDir::toNativeSeparators(m_processFolder))) {
                    foreach(QString archievePart, part.split("//", QString::SkipEmptyParts)) {
                        if(archievePart.contains(QDir::toNativeSeparators(m_processFolder))) {
                            return archievePart.mid(QDir::toNativeSeparators(m_processFolder).length() + 1);
                        }
                    }
                }
            }
            return "";
        }

        case AV::DRWEB: {
            QString fileName = reportLine.mid(QDir::toNativeSeparators(m_processFolder).length() + 1,
                                              reportLine.indexOf(QString("\\"), QDir::toNativeSeparators(m_processFolder).length() + 1) - (QDir::toNativeSeparators(m_processFolder).length() + 1));

            if(QDir(m_processFolder + "/").exists(fileName)) {
                return fileName;
            } else {
                return "";
            }
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

    while(!QDir(sourceDir + "/").isEmpty()) {

        filesInSourceDir = QDir(sourceDir + "/").entryInfoList(usingFilters);

        foreach(QFileInfo fileInfo, filesInSourceDir)
            QFile::rename(fileInfo.absoluteFilePath(), destinationDir + "/" + fileInfo.fileName());
    }
}

void AVWrapper::process() {

    m_processedLastFilesSizeMb = 0.;

    if(!QDir(m_inputFolder).isEmpty() && m_readyToProcess) {

        moveFiles(m_inputFolder, m_processFolder);

        m_readyToProcess = false;
        m_avBase.clear();

        if(m_isUsed) {

            QDateTime startTime = QDateTime::currentDateTime();
            m_reportName = m_reportFolder + "/report_" + QString::number(m_reportIdx) + "." + m_reportExtension;

            if(checkParams()) {
                log(currentDateTime() + " " + QString("Ошибка в параметрах запуска антивируса(%1).").arg(QString::number(checkParams())));
                m_readyToProcess = true;
            } else {
                m_reportFile.setFileName(m_reportName);

                foreach(QFileInfo fileInfo, QDir(m_processFolder).entryInfoList(usingFilters)) {
                    m_processedLastFilesSizeMb += fileInfo.size() / (1024. * 1024.);
                }
                m_processedFilesSizeMb += m_processedLastFilesSizeMb;
                m_inprogressFilesNb = QDir(m_processFolder).entryInfoList(usingFilters).size();
                m_processedFilesNb += m_inprogressFilesNb;
                emit updateUi();


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
                m_reportIdx++;

                m_isReportReady = false;

                while(!m_isReportReady) {
                    if(QFile::exists(m_reportName) && m_reportFile.open(QIODevice::ReadOnly)) {

                        m_stream.setDevice(&m_reportFile);
                        m_report = m_stream.readAll();

                        if(m_report.contains(m_reportReadyIndicator)) {

                            m_isReportReady = true;

                            m_stream.seek(m_report.indexOf(m_startRecordsIndicator) + m_startRecordsIndicator.length());
                            m_reportLine = m_stream.readLine();

                            while(true) {
                                m_reportLine = m_stream.readLine();
                                if(m_reportLine.contains(m_endRecordsIndicator)) {
                                    m_currentProcessSpeed = m_processedLastFilesSizeMb * 8 * 1000 / startTime.msecsTo(QDateTime::currentDateTime());
                                    break;
                                } else {
                                    if(isPayload(m_reportLine)) {
                                        QString fileName = extractFileName(m_reportLine);
                                        if(!fileName.isEmpty() && m_avBase.findFileName(fileName) == -1) {
                                            m_dangerFileNb++;

                                            QFile::rename(m_processFolder + "/" + fileName, m_dangerFolder + "/" + fileName);

                                            m_avBase.add(new AVRecord(QDateTime::currentDateTime(),
                                                                      m_type,
                                                                      fileName,
                                                                      extractDescription(m_reportLine, extractFileName(m_reportLine)),
                                                                      QFileInfo(m_reportName).fileName()));

                                            log(AVRecord(QDateTime::currentDateTime(),
                                                         m_type,
                                                         fileName,
                                                         extractDescription(m_reportLine, extractFileName(m_reportLine)),
                                                         QFileInfo(m_reportName).fileName()).toString());
                                        }
                                    }
                                }

                            }
                        }
                        m_reportFile.close();
                    }
                }

                moveFiles(m_processFolder, m_outputFolder);

                m_readyToProcess = true;
                emit updateBase(m_avBase);
                emit finalProcessing();
                process();
            }
        } else {
            moveFiles(m_processFolder, m_outputFolder);
            m_readyToProcess = true;
            emit finalProcessing();
            process();
        }
    } else {
        if(QDir(m_inputFolder).isEmpty())
            m_inprogressFilesNb = 0;
    }
    emit updateUi();
}

int AVBase::findFileName(QString fileName) {
    for(int  i = 0; i < m_base.size(); i++) {
        if(m_base.at(i).first.m_fileName == fileName || m_base.at(i).second.m_fileName == fileName) {
            return i;
        }
    }
    return -1;
}

void AVBase::add(AVRecord* record) {
    int idx = findFileName(record->m_fileName);
    if(idx != -1) {
        if(record->m_av == AV::KASPER)
            m_base[idx] = QPair<AVRecord, AVRecord>(*record, m_base[idx].second);
        if(record->m_av == AV::DRWEB)
            m_base[idx] = QPair<AVRecord, AVRecord>(m_base[idx].first, *record);
    } else {
        if(record->m_av == AV::KASPER)
            m_base.push_back(QPair<AVRecord, AVRecord>(*record, AVRecord()));
        if(record->m_av == AV::DRWEB)
            m_base.push_back(QPair<AVRecord, AVRecord>(AVRecord(), *record));
    }
}

void AVBase::add(QPair<AVRecord, AVRecord>* record) {
    m_base.append(*record);
}

void AVBase::add(AVBase& base) {

    for(int i = 0; i < base.size(); i++) {
        if(findFileName(base[i].first.m_fileName) == -1 && findFileName(base[i].second.m_fileName) == -1) {
            add(&base[i]);
        }
    }
}

void AVBase::remove(QString fileName) {
    int idx = findFileName(fileName);
    if(idx != -1) {
        m_base.removeAt(idx);
    }
}

void AVBase::remove(int idx) {
    if(!m_base.isEmpty() && idx >= 0 && idx < m_base.size())
        m_base.removeAt(idx);
}

void AVBase::clear() {
    m_base.clear();
}

QPair<AVRecord, AVRecord>& AVBase::operator[](int idx) {
    return m_base[idx];
}

int AVBase::size() {
    return m_base.size();
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
