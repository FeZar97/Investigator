#include "distributor.h"

Distributor::Distributor(QObject *parent) : QObject(parent){

    qRegisterMetaType<QList<AVRecord>>("QList<AVRecord>");

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
                                QStringList() << "running" << "completed");

    drwebWrapper.setIndicators("Scan session completed",
                               "The mask was translated to \"\" filter",
                               "WARNING! Restore points directories have not been scanned",
                               QStringList());

    configureAV();

    connect(&kasperWrapper, &AVWrapper::log,                        this,           &Distributor::log);
    connect(&drwebWrapper,  &AVWrapper::log,                        this,           &Distributor::log);

    connect(&kasperWrapper, &AVWrapper::updateUi,                   this,           &Distributor::updateUi);
    connect(&drwebWrapper,  &AVWrapper::updateUi,                   this,           &Distributor::updateUi);

    connect(&watchDirEye,   &QFileSystemWatcher::directoryChanged,  this,           &Distributor::onWatchDirChange);

    connect(&kasperWrapper, &AVWrapper::finalProcessing,            &drwebWrapper,  &AVWrapper::process);

    connect(&kasperWrapper, &AVWrapper::updateList,                 this,           &Distributor::updateBase);
    connect(&drwebWrapper,  &AVWrapper::updateList,                 this,           &Distributor::updateBase);
}

Distributor::~Distributor() {
    kasperThread.quit();
    kasperThread.wait();

    drwebThread.quit();
    drwebThread.wait();
}

void Distributor::setWatchDir(QString _watchDir) {

    if(_watchDir.isEmpty()) {
        log("watchDir не выбрана! Изменения не произведены.");
    } else {
        watchDir = _watchDir;

        startWatchDirEye();
    }
    updateUi();
}

QString Distributor::getWatchDir() {
    return watchDir;
}

void Distributor::setInvestigatorDir(QString _investigatorDir) {

    if(_investigatorDir.isEmpty()) {
        log("investigatorDir не выбрана! Изменения не произведены.");
    } else {
        investigatorDir = _investigatorDir;

        // start folder, it`s equal to temp dir, secon dir after watch dir
        inputDir  = investigatorDir + "/" + KASPER_DIR_NAME + "/" + INPUT_DIR_NAME;

        QDir(investigatorDir).mkdir(KASPER_DIR_NAME);
        QDir(investigatorDir + "/" + KASPER_DIR_NAME).mkdir(INPUT_DIR_NAME);
        QDir(investigatorDir + "/" + KASPER_DIR_NAME).mkdir(OUTPUT_DIR_NAME);

        QDir(investigatorDir).mkdir(DRWEB_DIR_NAME);
        QDir(investigatorDir + "/" + DRWEB_DIR_NAME).mkdir(INPUT_DIR_NAME);
        QDir(investigatorDir + "/" + DRWEB_DIR_NAME).mkdir(OUTPUT_DIR_NAME);

        QDir(investigatorDir).mkdir(REPORT_DIR_NAME);
        QDir(investigatorDir).mkdir(PROCESSED_DIR_NAME);

        outputDir = investigatorDir + "/" + PROCESSED_DIR_NAME;
        reportDir = investigatorDir + "/" + REPORT_DIR_NAME;

        configureAV();
    }
    updateUi();
}

QString Distributor::getInvestigatorDir() {
    return investigatorDir;
}

void Distributor::setCleanDir(QString _cleanDir) {
    if(_cleanDir.isEmpty()) {
        log("cleanDir не выбрана! Изменения не произведены.");
    } else {
        cleanDir = _cleanDir;
    }
    updateUi();
}

QString Distributor::getCleanDir() {
    return cleanDir;
}

void Distributor::setDangerDir(QString _dangerDir) {
    if(_dangerDir.isEmpty()) {
        log("dangerDir не выбрана! Изменения не произведены.");
    } else {
        dangerDir = _dangerDir;
    }
    updateUi();
}

QString Distributor::getDangerDir() {
    return dangerDir;
}

void Distributor::setAVFile(AV AVName, QString AVFilePath) {

    if(AVFilePath.isEmpty()) {
        log("AVFilePath не выбран! Изменения не произведены.");
        return;
    }

    switch(AVName) {
        case AV::KASPER:
            kasperWrapper.setAVPath(AVFilePath);
            break;

        case AV::DRWEB:
            drwebWrapper.setAVPath(AVFilePath);
            break;
    }

    updateUi();
}

QString Distributor::getAVFile(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getAVPath();

        case AV::DRWEB:
            return drwebWrapper.getAVPath();
    }
    return "";
}

void Distributor::setAVUse(AV AVName, bool use) {
    switch(AVName) {
        case AV::KASPER:
            kasperWrapper.setUsage(use);
            break;

        case AV::DRWEB:
            drwebWrapper.setUsage(use);
            break;
    }
    updateUi();
}

bool Distributor::getAVUse(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getUsage();

        case AV::DRWEB:
            return drwebWrapper.getUsage();
    }
    return true;
}

int Distributor::getAVProcessedFilesNb(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getProcessedFilesNb();

        case AV::DRWEB:
            return drwebWrapper.getProcessedFilesNb();
    }
    return 0;
}

int Distributor::getAVInprogressFilesNb(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getInprogressFilesNb();

        case AV::DRWEB:
            return drwebWrapper.getInprogressFilesNb();
    }
    return 0;
}

double Distributor::getAVProcessedFilesSize(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getProcessedFilesSize();

        case AV::DRWEB:
            return drwebWrapper.getProcessedFilesSize();
    }
    return 0;
}

void Distributor::configureAV() {
    kasperWrapper.setInputFolder(investigatorDir    + "/" + KASPER_DIR_NAME + "/" + INPUT_DIR_NAME);
    kasperWrapper.setProcessFolder(investigatorDir  + "/" + KASPER_DIR_NAME + "/" + OUTPUT_DIR_NAME);
    kasperWrapper.setOutputFolder(investigatorDir   + "/" + DRWEB_DIR_NAME  + "/" + INPUT_DIR_NAME);
    kasperWrapper.setReportFolder(reportDir);

    drwebWrapper.setInputFolder(investigatorDir   + "/" + DRWEB_DIR_NAME + "/" + INPUT_DIR_NAME);
    drwebWrapper.setProcessFolder(investigatorDir + "/" + DRWEB_DIR_NAME + "/" + OUTPUT_DIR_NAME);
    drwebWrapper.setOutputFolder(investigatorDir  + "/" + PROCESSED_DIR_NAME);
    drwebWrapper.setReportFolder(reportDir);
}

void Distributor::startWatchDirEye() {
    if(!watchDirEye.directories().isEmpty())
        watchDirEye.removePaths(watchDirEye.directories());

    if(!watchDir.isEmpty()) {
        if(watchDirEye.addPath(watchDir)) {
            onWatchDirChange("");
        } else {
            log("Ошибка в пути " + watchDir);
        }
    }
}

void Distributor::onWatchDirChange(const QString &path) {
    Q_UNUSED(path)

    moveFiles(watchDir, inputDir);

    tryProcessing();
}

void Distributor::tryProcessing() {

    kasperWrapper.process();
}

void Distributor::sortingProcessedFiles() {

}

void Distributor::updateBase(QList<AVRecord> list){
    foreach(AVRecord avrec, list) {
        log(avrec.toString());
    }
}
