#include "distributor.h"

Distributor::Distributor(QObject *parent) : QObject(parent) {
    
    qRegisterMetaType<AVBase>("AVBase");

// initial settings
    kasperWrapper.setType(AV::KASPER);
    drwebWrapper.setType(AV::DRWEB);

    kasperWrapper.setReportExtension("kres");
    drwebWrapper.setReportExtension("dres");

    kasperWrapper.setIndicators("; Completion:       	100%",
                                "; ------------------",
                                ";  --- Statistics ---",
                                QStringList() << "running" << "completed" << "password protected");

    drwebWrapper.setIndicators("Scan session completed",
                               "The mask was translated to \"\" filter",
                               "WARNING! Restore points directories have not been scanned",
                               QStringList() << "password protected");

// SSC
    connect(&kasperWrapper, &AVWrapper::log,                        this,           &Distributor::log);
    connect(&drwebWrapper,  &AVWrapper::log,                        this,           &Distributor::log);

    connect(&kasperWrapper, &AVWrapper::updateUi,                   this,           &Distributor::updateUi);
    connect(&drwebWrapper,  &AVWrapper::updateUi,                   this,           &Distributor::updateUi);

    connect(&watchDirEye,   &QFileSystemWatcher::directoryChanged,  this,           &Distributor::onWatchDirChange);

    connect(&kasperWrapper, &AVWrapper::finalProcessing,            &drwebWrapper,  &AVWrapper::process);
    connect(&drwebWrapper,  &AVWrapper::finalProcessing,            this,           &Distributor::sortingProcessedFiles);

    connect(&kasperWrapper, &AVWrapper::updateBase,                 this,           &Distributor::updateBase);
    connect(&drwebWrapper,  &AVWrapper::updateBase,                 this,           &Distributor::updateBase);

    kasperWrapper.moveToThread(&kasperThread);
    drwebWrapper.moveToThread(&drwebThread);
    kasperThread.start();
    drwebThread.start();

    configureAV();
}

Distributor::~Distributor() {
    kasperThread.quit();
    kasperThread.wait();

    drwebThread.quit();
    drwebThread.wait();
}

void Distributor::setWatchDir(QString _watchDir) {

    if(_watchDir.isEmpty()) {
        log(currentDateTime() + " " + "Не найдена директория для слежения.");
    } else {
        m_watchDir = _watchDir;
    }
}

QString Distributor::getWatchDir() {
    return m_watchDir;
}

void Distributor::setInvestigatorDir(QString _investigatorDir) {

    if(_investigatorDir.isEmpty()) {
        log(currentDateTime() + " " + "Не найдена директория для временных файлов.");
    } else {
        m_investigatorDir = _investigatorDir;

        m_inputDir  = m_investigatorDir + "/" + KASPER_DIR_NAME + "/" + INPUT_DIR_NAME;
        QDir().mkpath(m_inputDir);
        QDir().mkpath(m_investigatorDir + "/" + KASPER_DIR_NAME + "/" + OUTPUT_DIR_NAME);

        QDir().mkpath(m_investigatorDir + "/" + DRWEB_DIR_NAME + "/" + INPUT_DIR_NAME);
        QDir().mkpath(m_investigatorDir + "/" + DRWEB_DIR_NAME + "/" + OUTPUT_DIR_NAME);

        m_outputDir = m_investigatorDir + "/" + PROCESSED_DIR_NAME;
        QDir().mkpath(m_outputDir);

        m_reportDir = m_investigatorDir + "/" + REPORT_DIR_NAME;
        QDir().mkpath(m_reportDir);

        configureAV();
    }
}

QString Distributor::getInvestigatorDir() {
    return m_investigatorDir;
}

void Distributor::setCleanDir(QString _cleanDir) {
    if(_cleanDir.isEmpty()) {
        log(currentDateTime() + " " + "Не найдена директория для чистых файлов.");
    } else {
        m_cleanDir = _cleanDir;
    }
}

QString Distributor::getCleanDir() {
    return m_cleanDir;
}

void Distributor::setDangerDir(QString _dangerDir) {
    if(_dangerDir.isEmpty()) {
        log(currentDateTime() + " " + "Не найдена директория для зараженных файлов.");
    } else {
        m_dangerDir = _dangerDir;
        kasperWrapper.setDangerFolder(m_dangerDir);
        drwebWrapper.setDangerFolder(m_dangerDir);
    }
}

QString Distributor::getDangerDir() {
    return m_dangerDir;
}

void Distributor::setAVFile(AV AVName, QString AVFilePath) {

    if(AVFilePath.isEmpty()) {
        log(currentDateTime() + " " + QString("Не найдена исполняемый файл антивируса %1.").arg(getName(AVName)));
        return;
    }

    switch(AVName) {
        case AV::KASPER:
            kasperWrapper.setAVPath(AVFilePath);
            break;

        case AV::DRWEB:
            drwebWrapper.setAVPath(AVFilePath);
            break;

        default:
            break;
    }
}

QString Distributor::getAVFile(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getAVPath();

        case AV::DRWEB:
            return drwebWrapper.getAVPath();

        default:
            return "";
    }
}

void Distributor::setAVUse(AV AVName, bool use) {
    switch(AVName) {
        case AV::KASPER:
            kasperWrapper.setUsage(use);
            break;

        case AV::DRWEB:
            drwebWrapper.setUsage(use);
            break;

        default:
            break;
    }
}

bool Distributor::getAVUse(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getUsage();

        case AV::DRWEB:
            return drwebWrapper.getUsage();

        default:
            return true;
    }
}

void Distributor::setMaxQueueSize(AV AVName, int size) {
    switch(AVName) {
        case AV::KASPER:
            kasperWrapper.setMaxQueueSize(size);
            break;

        case AV::DRWEB:
            drwebWrapper.setMaxQueueSize(size);
            break;

        default:
            break;
    }
}

int Distributor::getMaxQueueSize(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getMaxQueueSize();

        case AV::DRWEB:
            return drwebWrapper.getMaxQueueSize();

        default:
            return 0;
    }
}

void Distributor::setMaxQueueVol(AV AVName, double vol) {
    switch(AVName) {
        case AV::KASPER:
            kasperWrapper.setMaxQueueVol(vol);
            break;

        case AV::DRWEB:
            drwebWrapper.setMaxQueueVol(vol);
            break;

        default:
            break;
    }
}

double Distributor::getMaxQueueVolMb(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getMaxQueueVol();

        case AV::DRWEB:
            return drwebWrapper.getMaxQueueVol();

        default:
            return 0;
    }
}

int Distributor::getAVDangerFilesNb(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getDangerFilesNb();

        case AV::DRWEB:
            return drwebWrapper.getDangerFilesNb();

        default:
            return 0;
    }
}

int Distributor::getAVCurrentReportIdx(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getCurrentReportIdx();

        case AV::DRWEB:
            return drwebWrapper.getCurrentReportIdx();

        default:
            return 0;
    }
}

int Distributor::getAVQueueFilesNb(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return QDir(m_investigatorDir + "/" + KASPER_DIR_NAME + "/" + INPUT_DIR_NAME).entryList(usingFilters).size();

        case AV::DRWEB:
            return QDir(m_investigatorDir + "/" + DRWEB_DIR_NAME + "/" + INPUT_DIR_NAME).entryList(usingFilters).size();

        default:
            return 0;
    }
}

double Distributor::getAVQueueFilesVolMb(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return dirSizeMb(m_investigatorDir + "/" + KASPER_DIR_NAME + "/" + INPUT_DIR_NAME);

        case AV::DRWEB:
            return dirSizeMb(m_investigatorDir + "/" + DRWEB_DIR_NAME + "/" + INPUT_DIR_NAME);

        default:
            return 0;
    }
}

int Distributor::getAVProcessedFilesNb(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getProcessedFilesNb();

        case AV::DRWEB:
            return drwebWrapper.getProcessedFilesNb();

        default:
            return 0;
    }
}

int Distributor::getAVInprogressFilesNb(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getInprogressFilesNb();

        case AV::DRWEB:
            return drwebWrapper.getInprogressFilesNb();

        default:
            return 0;
    }
}

double Distributor::getAVProcessedFilesSize(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getProcessedFilesSize();

        case AV::DRWEB:
            return drwebWrapper.getProcessedFilesSize();

        default:
            return 0;
    }
}

void Distributor::configureAV() {
    kasperWrapper.setInputFolder(m_investigatorDir   + "/" + KASPER_DIR_NAME + "/" + INPUT_DIR_NAME);
    kasperWrapper.setProcessFolder(m_investigatorDir + "/" + KASPER_DIR_NAME + "/" + OUTPUT_DIR_NAME);
    kasperWrapper.setOutputFolder(m_investigatorDir  + "/" + DRWEB_DIR_NAME  + "/" + INPUT_DIR_NAME);
    kasperWrapper.setReportFolder(m_reportDir);

    drwebWrapper.setInputFolder(m_investigatorDir   + "/" + DRWEB_DIR_NAME + "/" + INPUT_DIR_NAME);
    drwebWrapper.setProcessFolder(m_investigatorDir + "/" + DRWEB_DIR_NAME + "/" + OUTPUT_DIR_NAME);
    drwebWrapper.setOutputFolder(m_outputDir);
    drwebWrapper.setReportFolder(m_reportDir);
}

void Distributor::startWatchDirEye() {

    stopWatchDirEye();

    if(!m_watchDir.isEmpty()) {
        if(watchDirEye.addPath(m_watchDir)) {
            m_startTime = QDateTime::currentDateTime();
            m_isProcessing = true;
            log(currentDateTime() + " " + QString("Запущено слежение за директорией %1.").arg(m_watchDir));
            onWatchDirChange("");
        } else {
            log(currentDateTime() + " " + QString("Не удалось начать слежение за папкой %1.").arg(m_watchDir));
        }
    }
    updateUi();
}

void Distributor::stopWatchDirEye() {
    if(!watchDirEye.directories().isEmpty()) {
        m_isProcessing = false;
        m_endTime = QDateTime::currentDateTime();
        log(currentDateTime() + " " + QString("Слежение за директорией %1 остановлено.").arg(m_watchDir));
        watchDirEye.removePaths(watchDirEye.directories());
    }
    updateUi();
}

void Distributor::onWatchDirChange(const QString &path) {
    Q_UNUSED(path)
    moveFiles(m_watchDir, m_inputDir);
    // kasperWrapper.process();
}

void Distributor::sortingProcessedFiles() {
    int idx;

    if(!QDir(m_dangerDir).exists()) QDir().mkpath(m_dangerDir);
    if(!QDir(m_cleanDir).exists())  QDir().mkpath(m_cleanDir);

    foreach(QFileInfo avRecord, QDir(m_outputDir).entryInfoList(usingFilters)) {
        idx = mainBase.findFileName(avRecord.fileName());
        if(idx != -1) {
            QFile::rename(avRecord.absoluteFilePath(), m_dangerDir + "/" + avRecord.fileName());
            mainBase.remove(idx);
        } else {
            QFile::rename(avRecord.absoluteFilePath(), m_cleanDir + "/" + avRecord.fileName());
        }
    }
}

void Distributor::updateBase(AVBase& singleAVBase){
    mainBase.add(singleAVBase);
}

qint64 Distributor::getWorkTimeInSecs() {
    if(m_isProcessing)
        m_endTime = QDateTime::currentDateTime();
    return m_startTime.secsTo(m_endTime);
}

QDateTime Distributor::getStartTime() const {
    return m_startTime;
}

QDateTime Distributor::getEndTime() const {
    return m_endTime;
}

bool Distributor::isInProcessing(){
    return m_isProcessing;
}

void Distributor::clearDir(QString dirName) {
    foreach(QFileInfo fileInfo, QDir(dirName).entryInfoList(usingFilters)) {
        QFile::remove(fileInfo.absoluteFilePath());
    }
}

double Distributor::dirSizeMb(QString dirName) {
    double volMb{0};
    foreach(QFileInfo fileInfo, QDir(dirName).entryInfoList(usingFilters)) {
        volMb += fileInfo.size() * 128;
    }
    return volMb;
}

void Distributor::moveFilesToInputDir() {
    moveFiles(m_investigatorDir + "/" + KASPER_DIR_NAME + "/" + OUTPUT_DIR_NAME, m_inputDir);
    moveFiles(m_investigatorDir + "/" + DRWEB_DIR_NAME + "/" + INPUT_DIR_NAME, m_inputDir);
    moveFiles(m_investigatorDir + "/" + DRWEB_DIR_NAME + "/" + OUTPUT_DIR_NAME, m_inputDir);
    moveFiles(m_outputDir, m_inputDir);
}

double Distributor::getAVAverageSpeed(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getAverageSpeed(getWorkTimeInSecs());

        case AV::DRWEB:
            return drwebWrapper.getAverageSpeed(getWorkTimeInSecs());

        default:
            return 0;
    }
}

double Distributor::getAVCurrentSpeed(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getCurrentSpeed();

        case AV::DRWEB:
            return drwebWrapper.getCurrentSpeed();

        default:
            return 0;
    }
}
