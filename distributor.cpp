#include "distributor.h"

Distributor::Distributor(QObject *parent) : QObject(parent){
    connect(&watchDirEye,  &QFileSystemWatcher::directoryChanged, this, &Distributor::onWatchDirChange);
    connect(&tempDirEye,   &QFileSystemWatcher::directoryChanged, this, &Distributor::onTempDirChange);
}

QList<ProcessObject> Distributor::createWorkObjects(QFileInfoList filesToProcess) {
    QList<ProcessObject> result;
    for(int i = 0; i < filesToProcess.size(); i++) {
        result.append(ProcessObject(filesToProcess.at(i), useKasper, useDrweb, kasperFilePath, drwebFilePath,
                                    tempDir, cleanDir, dangerDir));
    }
    return result;
}

void Distributor::setWatchDir(QString _watchDir) {

    if(_watchDir.isEmpty()) {
        qDebug() << "watchDir не выбрана! Изменения не произведены.";
    } else {
        watchDir = _watchDir;
        filesInWatchDir.clear();
        emit updateUi();
    }
}

QString Distributor::getWatchDir() {
    return watchDir;
}

void Distributor::setInvestigatorDir(QString _investigatorDir) {
    if(_investigatorDir.isEmpty()) {
        qDebug() << "investigatorDir не выбрана! Изменения не произведены.";
    } else {
        investigatorDir = _investigatorDir;

        reportDir = investigatorDir + "/" + REPORT_DIR_NAME;
        tempDir   = investigatorDir + "/" + TEMP_DIR_NAME;
        kasperDir = investigatorDir + "/" + KASPER_DIR_NAME;
        drwebDir  = investigatorDir + "/" + DRWEB_DIR_NAME;

        QDir(investigatorDir).mkdir(REPORT_DIR_NAME);
        QDir(investigatorDir).mkdir(TEMP_DIR_NAME);
        QDir(investigatorDir).mkdir(KASPER_DIR_NAME);
        QDir(investigatorDir).mkdir(DRWEB_DIR_NAME);

        emit updateUi();
    }
}

QString Distributor::getInvestigatorDir() {
    return investigatorDir;
}

void Distributor::setCleanDir(QString _cleanDir) {
    if(_cleanDir.isEmpty()) {
        qDebug() << "cleanDir не выбрана! Изменения не произведены.";
    } else {
        cleanDir = _cleanDir;
        emit updateUi();
    }
}

QString Distributor::getCleanDir() {
    return cleanDir;
}

void Distributor::setDangerDir(QString _dangerDir) {
    if(_dangerDir.isEmpty()) {
        qDebug() << "dangerDir не выбрана! Изменения не произведены.";
    } else {
        dangerDir = _dangerDir;
        emit updateUi();
    }
}

QString Distributor::getDangerDir() {
    return dangerDir;
}

void Distributor::setKasperFile(QString _kasperFilePath) {
    if(_kasperFilePath.isEmpty()) {
        qDebug() << "kasperFilePath не выбран! Изменения не произведены.";
    } else {
        kasperFilePath = _kasperFilePath;
        emit updateUi();
    }
}

QString Distributor::getKasperFile() {
    return kasperFilePath;
}

void Distributor::setUseKasper(bool _useKasper) {
    useKasper = _useKasper;
    emit updateUi();
}

bool Distributor::isKasperUse() {
    return useKasper;
}

void Distributor::setDrwebFile(QString _drwebFilePath) {
    if(_drwebFilePath.isEmpty()) {
        qDebug() << "drwebFilePath не выбран! Изменения не произведены.";
    } else {
        drwebFilePath = _drwebFilePath;
        emit updateUi();
    }
}

QString Distributor::getDrwebFile() {
    return drwebFilePath;
}

void Distributor::setUseDrweb(bool _useDrweb) {
    useDrweb = _useDrweb;
    emit updateUi();
}

bool Distributor::isDrwebUse() {
    return useDrweb;
}

void Distributor::startWatchDirEye() {
    stopWatchDirEye();

    if(!watchDir.isEmpty()) {
        if(watchDirEye.addPath(watchDir)) {
            onWatchDirChange("");
        } else {
            qDebug() << "Ошибка в пути " + watchDir;
        }
    }
}

void Distributor::stopWatchDirEye() {
    if(!watchDirEye.directories().isEmpty()) {
        watchDirEye.removePaths(watchDirEye.directories());
        filesInWatchDir.clear();
    }
}

void Distributor::onWatchDirChange(const QString &path) {
    Q_UNUSED(path)

    filesInWatchDir = QDir(watchDir + "/").entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    foreach(QFileInfo fileInfo, filesInWatchDir) {
        QFile::rename(fileInfo.absoluteFilePath(), tempDir + "/" + fileInfo.fileName());
        filesInWatchDir.removeAll(fileInfo);
    }

    updateUi();
}

void Distributor::startTempDirEye() {
    stopTempDirEye();

    if(!tempDir.isEmpty()) {
        if(!tempDirEye.addPath(tempDir)) {
            qDebug() << "Ошибка в пути " + tempDir;
        }
    }
}

void Distributor::stopTempDirEye() {
    if(!tempDirEye.directories().isEmpty()) {
        tempDirEye.removePaths(tempDirEye.directories());
    }
}

void Distributor::onTempDirChange(const QString &path) {

    Q_UNUSED(path)

    filesInTempDir = QDir(tempDir + "/").entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    // log("onTempDirChange, readyFlag = " + QString(readyFlag));

    // blockingMapped(createWorkObjects(QDir(tempDir).entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks)), processByKasper);

    if(readyFlag) {
        // if temp dir is empty => readyFlag is true
        setReadyFlag(filesInTempDir.isEmpty());

        // if exist files to check
        if(!filesInTempDir.isEmpty()) {
            foreach(QFileInfo fileInfo, filesInTempDir) {
                QFile::rename(fileInfo.absoluteFilePath(), tempDir + "/" + fileInfo.fileName());
                filesInTempDir.removeAll(fileInfo);
            }

            processDirByKasper(tempDir + "/");
        }
    }
}

int Distributor::getKasperProcessedFilesNb(){
    return kasperProcessedFilesNb;
}

double Distributor::getKasperProcessedFilesSize() {
    return kasperProcessedFilesSizeMb;
}

int Distributor::getDrwebProcessedFilesNb() {
    return drwebProcessedFilesNb;
}

double Distributor::getDrwebProcessedFilesSize() {
    return drwebProcessedFilesSizeMb;
}

/*
//void Distributor::startKasperDirEye(){
//    stopKasperDirEye();
//
//    if(kasperDirEye.addPath(kasperDir)) {
//        onKasperDirChange("");
//    } else {
//        qDebug() << "Ошибка в пути: " + kasperDir;
//    }
//}

//void Distributor::stopKasperDirEye() {
//    if(!kasperDirEye.directories().isEmpty()) {
//        kasperDirEye.removePaths(kasperDirEye.directories());
//        filesInKasperDir.clear();
//    }
//}

// void Distributor::onKasperDirChange(const QString &path) {
//
//     Q_UNUSED(path)
//
//     blockingMapped(createWorkObjects(QDir(kasperDir).entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks)), processByDrweb);
//
//     updateUi();
// }

// void Distributor::startDrwebDirEye() {
//     stopDrwebDirEye();
//
//     if(drwebDirEye.addPath(drwebDir)) {
//         onDrwebDirChange("");
//     } else {
//         qDebug() << "Ошибка в пути: " + drwebDir;
//     }
// }

// void Distributor::stopDrwebDirEye() {
//     if(!drwebDirEye.directories().isEmpty()) {
//         drwebDirEye.removePaths(drwebDirEye.directories());
//         filesInDrwebDir.clear();
//     }
// }

// void Distributor::onDrwebDirChange(const QString &path) {
//     Q_UNUSED(path)
//
//     processDangerFiles(blockingMapped(createWorkObjects(QDir(drwebDir).entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks)), processResults));
//
//     updateUi();
// }
*/

int Distributor::getQueueSize() {
    return filesInWatchDir.size();
}

/*
ProcessObject Distributor::processByKasper(ProcessObject obj) {

    QString KReportFileName = obj.reportDir + "/" + obj.fileInfo.baseName() + ".kres",
            KReport;
    QFile KReportFile;
    KReportFile.setFileName(KReportFileName);
    bool isReportReady = false;

    if(obj.useKasper && !QFile::exists(KReportFileName) && QFile::exists(obj.tempDir + "/" + obj.fileInfo.fileName())) {
        QProcess::execute(obj.kasperPath, QStringList() << "scan"
                                                        << obj.tempDir + "/" + obj.fileInfo.fileName()
                                                        << "/i0"
                                                        << QString("/R:" + obj.tempDir + "/reports/" + obj.fileInfo.baseName() + ".kres"));
    }

    while(!isReportReady) {
        if(obj.useKasper && QFile::exists(KReportFileName)) {
            if(KReportFile.open(QIODevice::ReadOnly)) {
                KReport = QTextStream(&KReportFile).readAll();
                KReportFile.close();
            }

            if(KReport.indexOf(QString("Total detected:   	")) != -1)
                isReportReady = true;
        } else {
            if(!obj.useKasper)
                isReportReady = true;
        }
    }

    QFile::rename(obj.tempDir   + "/" + obj.fileInfo.fileName(),
                  obj.kasperDir + "/" + obj.fileInfo.fileName());

    return obj;
}
*/

void Distributor::processDirByKasper(QString dirToProcess) {

    if(!QDir(dirToProcess).isEmpty()) {

        // transfer all files to kasper dir
        kasperProcessedFilesNb += QDir(dirToProcess).entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks).size();
        foreach(QFileInfo fileInfo, QDir(dirToProcess).entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks)) {
            kasperProcessedFilesSizeMb += fileInfo.size() / (1024. * 1024.);
            QFile::rename(fileInfo.absoluteFilePath(), kasperDir + "/" + fileInfo.fileName());
        }

        QString KReportFileName = reportDir + "/report_" + QString::number(kasperReportIdx) + ".kres", KReport;
        QFile KReportFile;
        KReportFile.setFileName(KReportFileName);
        bool isReportReady = false;

        if(useKasper && !QFile::exists(KReportFileName)) {
            QProcess::execute(kasperFilePath, QStringList() << "scan" << kasperDir << "/i0" << QString("/R:" + KReportFileName));
            kasperReportIdx += 1;
        }

        QTextStream res;
        int progress = -1;
        QString temp;

        while(!isReportReady) {
            if(useKasper && QFile::exists(KReportFileName)) {
                if(KReportFile.open(QIODevice::ReadOnly)) {
                    res.setDevice(&KReportFile);
                    KReport = res.readAll();

                    res.seek(KReport.indexOf(QString("Completion:       	")) + QString("Completion:       	").length());
                    res >> progress;

                    if(progress == 100) {

                        isReportReady = true;

                        res.seek(KReport.indexOf(QString("------------------")) + QString("------------------").length());
                        temp = res.readLine();

                        while(true) {
                            temp = res.readLine();
                            if(temp.contains(";  --- Statistics ---"))
                                break;
                            else if(!temp.contains("running") && !temp.contains("completed"))
                                log(temp + "\tfinded by Kaspersky (report_" + QString::number(kasperReportIdx - 1) + ")");
                        }
                    }
                    KReportFile.close();

                }
            } else {
                if(!useKasper)
                    isReportReady = true;
            }
        }

        updateUi();

        processDirByDrweb(kasperDir + "/");
    }
}

void Distributor::processDirByDrweb(QString dirToProcess) {

    if(!QDir(dirToProcess).isEmpty()) {

        // transfer all files to drweb dir
        drwebProcessedFilesNb += QDir(dirToProcess).entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks).size();

        foreach(QFileInfo fileInfo, QDir(dirToProcess).entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks)) {
            drwebProcessedFilesSizeMb += fileInfo.size() / (1024. * 1024.);
            QFile::rename(fileInfo.absoluteFilePath(), drwebDir + "/" + fileInfo.fileName());
        }

        QString DReportFileName = reportDir + "/report_" + QString::number(drwebReportIdx) + ".dres", DReport;
        QFile DReportFile;
        DReportFile.setFileName(DReportFileName);
        bool isReportReady = false;

        if(useDrweb && !QFile::exists(DReportFileName)) {
            QProcess::execute(drwebFilePath, QStringList() << QString("/RP:" + DReportFileName) << drwebDir + "/");
            drwebReportIdx += 1;
        }

        QTextStream res;
        QString temp;

        while(!isReportReady) {
            if(useDrweb && QFile::exists(DReportFileName)) {
                if(DReportFile.open(QIODevice::ReadOnly)) {
                    res.setDevice(&DReportFile);
                    DReport = res.readAll();

                    if(DReport.contains("Scan session completed")) {
                        isReportReady = true;

                        res.seek(DReport.indexOf(QString("The mask was translated to")) + QString("The mask was translated to \"\" filter").length());

                        temp = res.readLine();
                        while(true) {
                            temp = res.readLine();
                            if(temp.contains("WARNING! Restore points directories have not been scanned"))
                                break;
                            else
                                log(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss\t") + temp + "\tfinded by DrWeb (report_" + QString::number(drwebReportIdx - 1) + ")");
                        }
                    }
                    DReportFile.close();
                }
            } else {
                if(!useDrweb)
                    isReportReady = true;
            }
        }

        foreach(QFileInfo fileInfo, QDir(drwebDir).entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks)) {
            QFile::rename(fileInfo.absoluteFilePath(), cleanDir + "/" + fileInfo.fileName());
        }

        setReadyFlag(true);
        if(!filesInWatchDir.isEmpty())
            onWatchDirChange("");

        updateUi();
    }
}

/*
ProcessObject Distributor::processByDrweb(ProcessObject obj) {

    QString DReportFileName = obj.reportDir + "/" + obj.fileInfo.baseName() + ".dres",
            DReport;
    QFile DReportFile;
    DReportFile.setFileName(DReportFileName);

    if(obj.useDrweb && !QFile::exists(DReportFileName) && QFile::exists(obj.kasperDir + "/" + obj.fileInfo.fileName())) {
        QProcess::execute(obj.drwebPath, QStringList() << "/DR"
                                                       << QString("/RP:" + obj.tempDir + "/reports/" + obj.fileInfo.baseName() + ".dres")
                                                       << obj.kasperDir + "/" + obj.fileInfo.fileName());
    }

    if(obj.useDrweb && QFile::exists(DReportFileName)) {
        if(DReportFile.open(QIODevice::ReadOnly)) {
            DReport = QTextStream(&DReportFile).readAll();
            DReportFile.close();
        }
    }

    if((obj.useDrweb && QFile::exists(DReportFileName)) || !obj.useDrweb)
        QFile::rename(obj.kasperDir + "/" + obj.fileInfo.fileName(),
                      obj.drwebDir  + "/" + obj.fileInfo.fileName());

    return obj;
}
*/

/*
ProcessObject Distributor::processResults(ProcessObject obj) {

    QString KReportFileName = obj.reportDir + "/" + obj.fileInfo.baseName() + ".kres",
            DReportFileName = obj.reportDir + "/" + obj.fileInfo.baseName() + ".dres",
            KReport, DReport;
    QFile KReportFile, DReportFile;

    KReportFile.setFileName(KReportFileName);
    DReportFile.setFileName(DReportFileName);

    if(obj.useKasper) {
        if(KReportFile.open(QIODevice::ReadOnly)) {
            KReport = QTextStream(&KReportFile).readAll();
            KReportFile.close();
        }
    }

    if(obj.useDrweb) {
        if(DReportFile.open(QIODevice::ReadOnly)) {
            DReport = QTextStream(&DReportFile).readAll();
            DReportFile.close();
        }
    }

    obj.kasperDetect = KReport.mid(KReport.indexOf(QString("Total detected:")), 20).contains("1");
    obj.drwebDetect = DReport.indexOf("file are infected") != -1;

    if(obj.kasperDetect || obj.drwebDetect) {

        QFile::rename(obj.drwebDir  + "/" + obj.fileInfo.fileName(),
                      obj.dangerDir + "/" + obj.fileInfo.fileName());

        if(obj.useKasper && QFile::exists(obj.reportDir + "/" + obj.fileInfo.baseName() + ".kres")) {
            while(QFile::exists(obj.reportDir + "/" + obj.fileInfo.baseName() + ".kres"))
                QFile::rename(obj.reportDir + "/" + obj.fileInfo.baseName() + ".kres",
                              obj.dangerDir + "/" + obj.fileInfo.baseName() + ".kres");
        }

        if(obj.useDrweb) {
            QFile::rename(obj.reportDir + "/" + obj.fileInfo.baseName() + ".dres",
                          obj.dangerDir + "/" + obj.fileInfo.baseName() + ".dres");
        }

    } else {

        QFile::rename(obj.drwebDir + "/" + obj.fileInfo.fileName(),
                      obj.cleanDir + "/" + obj.fileInfo.fileName());

        if(obj.useKasper) {
            while(QFile::exists(obj.reportDir + "/" + obj.fileInfo.baseName() + ".kres"))
                QFile::remove(obj.reportDir + "/" + obj.fileInfo.baseName() + ".kres");
        }

        if(obj.useDrweb) {
            QFile::remove(obj.reportDir + "/" + obj.fileInfo.baseName() + ".dres");
        }
    }

    return obj;
}
*/

/*
void Distributor::processDangerFiles(QList<ProcessObject> resultObjects) {
    QString report;
    processedFilesNb += resultObjects.size();
    foreach(ProcessObject ro, resultObjects){
        report = "Результат проверки файла " + ro.fileInfo.fileName() + ": ";
        processedFilesSizeMb += ro.fileSize;
        if(ro.kasperDetect) report += "Kaspersky ";
        if(ro.drwebDetect)  report += "DrWeb ";
        if(ro.kasperDetect || ro.drwebDetect) log(report);
    }

    setReadyFlag(true);
    onWatchDirChange("");
}
*/

void Distributor::setReadyFlag(bool state) {
    readyFlag = state;
}
