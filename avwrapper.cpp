#include "avwrapper.h"

AVRecord::AVRecord(QDateTime timeMark, AV av, QString fileName, QString description, QString reportName) {
    m_timeMark = timeMark;
    m_av = av;
    m_fileName = fileName;
    m_description = description;
    m_reportName = reportName;
}

AVRecord::AVRecord(AVRecord &record){
    m_timeMark = record.m_timeMark;
    m_av = record.m_av;
    m_fileName = record.m_fileName;
    m_description = record.m_description;
    m_reportName = record.m_reportName;
}

QString AVRecord::toString() {
    return m_timeMark.toString("yyyy/MM/dd hh:mm:ss") + " " +
           m_fileName + " " +
           m_description + " " +
            "(" + AVWrapper::getName(m_av) + " " + QFileInfo(m_reportName).baseName() + ")";
}

AVRecord &AVRecord::operator=(AVRecord &record) {
    if(&record == this)
        return *this;

    m_timeMark = record.m_timeMark;
    m_av = record.m_av;
    m_fileName = record.m_fileName;
    m_description = record.m_description;
    m_reportName = record.m_reportName;

    return *this;
}

AVWrapper::AVWrapper(QObject *parent) : QObject(parent) {
}

void AVWrapper::setType(AV type) {
    m_type = type;
}

QString AVWrapper::getName(AV type) {
    switch(type) {
        case AV::KASPER:
            return "Kaspersky";
        case AV::DRWEB:
            return "DrWeb";
    }
    return "";
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

int AVWrapper::getProcessedFilesNb() {
    return m_processedFilesNb;
}

int AVWrapper::getInprogressFilesNb() {
    return m_inprogressFilesNb;
}

double AVWrapper::getProcessedFilesSize() {
    return m_processedFilesSizeMb;
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
        if(line.contains(permitString))
            return false;
    }
    return true;
}

QString AVWrapper::extractFileName(QString reportLine) {

    switch(m_type) {

        case AV::KASPER:
            foreach(QString part, reportLine.split("\t", QString::SkipEmptyParts)) {
                if(part.contains(QDir::toNativeSeparators(m_processFolder))) {
                    return part.mid(QDir::toNativeSeparators(m_processFolder).length() + 1);
                }
            }
            break;

        case AV::DRWEB:
            foreach(QString part, reportLine.split(" ", QString::SkipEmptyParts)) {
                if(part.contains(QDir::toNativeSeparators(m_processFolder))) {
                    return part.mid(QDir::toNativeSeparators(m_processFolder).length() + 1);
                }
            }
            break;
    }
    return reportLine;
}

QString AVWrapper::extractDescription(QString reportLine, QString fileName) {

    switch(m_type) {
        case AV::KASPER:
            return reportLine.split("\t", QString::SkipEmptyParts).last();
        case AV::DRWEB:
            return reportLine.mid(reportLine.lastIndexOf(fileName) + fileName.length() + 1);
    }
    return reportLine;
}

void moveFiles(QString sourceDir, QString destinationDir) {

    if(sourceDir.isEmpty() || destinationDir.isEmpty())
        return;

    QFileInfoList filesInSourceDir = QDir(sourceDir + "/").entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    while(!QDir(sourceDir + "/").isEmpty()) {
        filesInSourceDir = QDir(sourceDir + "/").entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

        foreach(QFileInfo fileInfo, filesInSourceDir)
            QFile::rename(fileInfo.absoluteFilePath(), destinationDir + "/" + fileInfo.fileName());
    }
}

void AVWrapper::process() {

    if(!QDir(m_inputFolder).isEmpty() && m_readyToProcess) {

        QList<AVRecord> list;

        moveFiles(m_inputFolder, m_processFolder);

        m_readyToProcess = false;

        if(m_isUsed) {

            m_reportName = m_reportFolder + "/report_" + QString::number(m_reportIdx) + "." + m_reportExtension;

            if(checkParams()) {
                log("Invalid params of " + getName(m_type) + " antivirus, code: " + QString::number(checkParams()));
                m_readyToProcess = true;
                process();
            } else {
                m_reportFile.setFileName(m_reportName);

                foreach(QFileInfo fileInfo, QDir(m_processFolder).entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks)) {
                    m_processedFilesSizeMb += fileInfo.size() / (1024. * 1024.);
                }
                m_inprogressFilesNb = QDir(m_processFolder).entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks).size();
                m_processedFilesNb += m_inprogressFilesNb;
                emit updateUi();

                switch(m_type) {
                    case AV::KASPER:
                        QProcess::execute(m_avPath, QStringList() << "scan" << m_processFolder << "/i0" << QString("/R:" + m_reportName));
                        break;
                    case AV::DRWEB:
                        QProcess::execute(m_avPath, QStringList() << QString("/RP:" + m_reportName) << m_processFolder + "/");
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
                                if(m_reportLine.contains(m_endRecordsIndicator))
                                    break;
                                else if(isPayload(m_reportLine))
                                    list.append(AVRecord(QDateTime::currentDateTime(),
                                                         m_type,
                                                         extractFileName(m_reportLine),
                                                         extractDescription(m_reportLine, extractFileName(m_reportLine)),
                                                         QFileInfo(m_reportName).fileName()));
                            }
                        }
                        m_reportFile.close();
                    }
                }

                moveFiles(m_processFolder, m_outputFolder);
                m_readyToProcess = true;
                emit updateList(list);
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
    for(int  i = 0; i < base.size(); i++) {
        if(base.at(i).first.m_fileName == fileName || base.at(i).second.m_fileName == fileName) {
            return i;
        }
    }
    return -1;
}

void AVBase::add(AVRecord record) {
    int idx = findFileName(record.m_fileName);
    if(idx != -1) {
        if(record.m_av == AV::KASPER)
            base.at(idx).first = record;
        if(record.m_av == AV::DRWEB)
            base.at(idx).second = record;
    }
}

void AVBase::add(QPair<AVRecord, AVRecord> record) {
    base.append(record);
}
