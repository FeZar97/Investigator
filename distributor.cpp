#include "distributor.h"

Distributor::Distributor(QObject *parent) : QObject(parent) {

    qRegisterMetaType<AVBase>("AVBase");

    kasperWrapper.moveToThread(&kasperThread);
    drwebWrapper.moveToThread(&drwebThread);
    kasperThread.start();
    drwebThread.start();

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

    configureAV();

    connect(&kasperWrapper, &AVWrapper::log,                        this,           &Distributor::log);
    connect(&drwebWrapper,  &AVWrapper::log,                        this,           &Distributor::log);

    connect(&kasperWrapper, &AVWrapper::updateUi,                   this,           &Distributor::updateUi);
    connect(&drwebWrapper,  &AVWrapper::updateUi,                   this,           &Distributor::updateUi);

    connect(&watchDirEye,   &QFileSystemWatcher::directoryChanged,  this,           &Distributor::onWatchDirChange);

    connect(&kasperWrapper, &AVWrapper::finalProcessing,            &drwebWrapper,  &AVWrapper::process);
    connect(&drwebWrapper,  &AVWrapper::finalProcessing,            this,           &Distributor::sortingProcessedFiles);

    connect(&kasperWrapper, &AVWrapper::updateBase,                 this,           &Distributor::updateBase);
    connect(&drwebWrapper,  &AVWrapper::updateBase,                 this,           &Distributor::updateBase);
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
        watchDir = _watchDir;
    }
}

QString Distributor::getWatchDir() {
    return watchDir;
}

void Distributor::setInvestigatorDir(QString _investigatorDir) {

    if(_investigatorDir.isEmpty()) {
        log(currentDateTime() + " " + "Не найдена директория для временных файлов.");
    } else {
        investigatorDir = _investigatorDir;

        inputDir  = investigatorDir + "/" + KASPER_DIR_NAME + "/" + INPUT_DIR_NAME;
        QDir().mkpath(inputDir);
        QDir().mkpath(investigatorDir + "/" + KASPER_DIR_NAME + "/" + OUTPUT_DIR_NAME);

        QDir().mkpath(investigatorDir + "/" + DRWEB_DIR_NAME + "/" + INPUT_DIR_NAME);
        QDir().mkpath(investigatorDir + "/" + DRWEB_DIR_NAME + "/" + OUTPUT_DIR_NAME);

        outputDir = investigatorDir + "/" + PROCESSED_DIR_NAME;
        QDir().mkpath(outputDir);

        reportDir = investigatorDir + "/" + REPORT_DIR_NAME;
        QDir().mkpath(reportDir);

        configureAV();
    }
}

QString Distributor::getInvestigatorDir() {
    return investigatorDir;
}

void Distributor::setCleanDir(QString _cleanDir) {
    if(_cleanDir.isEmpty()) {
        log(currentDateTime() + " " + "Не найдена директория для чистых файлов.");
    } else {
        cleanDir = _cleanDir;
    }
}

QString Distributor::getCleanDir() {
    return cleanDir;
}

void Distributor::setDangerDir(QString _dangerDir) {
    if(_dangerDir.isEmpty()) {
        log(currentDateTime() + " " + "Не найдена директория для зараженных файлов.");
    } else {
        dangerDir = _dangerDir;
    }
}

QString Distributor::getDangerDir() {
    return dangerDir;
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
            return QDir(investigatorDir + "/" + KASPER_DIR_NAME + "/" + INPUT_DIR_NAME).entryList(usingFilters).size();

        case AV::DRWEB:
            return QDir(investigatorDir + "/" + DRWEB_DIR_NAME + "/" + INPUT_DIR_NAME).entryList(usingFilters).size();

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
    kasperWrapper.setInputFolder(investigatorDir   + "/" + KASPER_DIR_NAME + "/" + INPUT_DIR_NAME);
    kasperWrapper.setProcessFolder(investigatorDir + "/" + KASPER_DIR_NAME + "/" + OUTPUT_DIR_NAME);
    kasperWrapper.setOutputFolder(investigatorDir  + "/" + DRWEB_DIR_NAME  + "/" + INPUT_DIR_NAME);
    kasperWrapper.setReportFolder(reportDir);

    drwebWrapper.setInputFolder(investigatorDir   + "/" + DRWEB_DIR_NAME + "/" + INPUT_DIR_NAME);
    drwebWrapper.setProcessFolder(investigatorDir + "/" + DRWEB_DIR_NAME + "/" + OUTPUT_DIR_NAME);
    drwebWrapper.setOutputFolder(outputDir);
    drwebWrapper.setReportFolder(reportDir);
}

void Distributor::startWatchDirEye() {

    if(!watchDir.isEmpty()) {
        if(watchDirEye.addPath(watchDir)) {
            startTime = QDateTime::currentDateTime();
            m_isProcessing = true;
            log(currentDateTime() + " " + QString("Запущено слежение за директорией %1.").arg(watchDir));
            onWatchDirChange("");
        } else {
            log(currentDateTime() + " " + QString("Не удалось начать слежение за папкой %1.").arg(watchDir));
        }
    }
    updateUi();
}

void Distributor::stopWatchDirEye() {
    if(!watchDirEye.directories().isEmpty()) {
        m_isProcessing = false;
        endTime = QDateTime::currentDateTime();
        log(currentDateTime() + " " + QString("Слежение за директорией %1 остановлено.").arg(watchDir));
        watchDirEye.removePaths(watchDirEye.directories());
    }
    updateUi();
}

void Distributor::onWatchDirChange(const QString &path) {
    Q_UNUSED(path)
    moveFiles(watchDir, inputDir);
    kasperWrapper.process();
}

void Distributor::sortingProcessedFiles() {
    int idx;

    if(!QDir(dangerDir).exists()) QDir().mkpath(dangerDir);
    if(!QDir(cleanDir).exists())  QDir().mkpath(cleanDir);

    foreach(QFileInfo avRecord, QDir(outputDir).entryInfoList(usingFilters)) {
        idx = mainBase.findFileName(avRecord.fileName());
        if(idx != -1) {
            QFile::rename(avRecord.absoluteFilePath(), dangerDir + "/" + avRecord.fileName());
            mainBase.remove(idx);
        } else {
            QFile::rename(avRecord.absoluteFilePath(), cleanDir + "/" + avRecord.fileName());
        }
    }
}

void Distributor::updateBase(AVBase& singleAVBase){
    mainBase.add(singleAVBase);
}

qint64 Distributor::getWorkTimeInSecs() {
    if(m_isProcessing)
        endTime = QDateTime::currentDateTime();
    return startTime.secsTo(endTime);
}

bool Distributor::isInProcessing(){
    return m_isProcessing;
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
