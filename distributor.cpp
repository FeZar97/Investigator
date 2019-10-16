#include "distributor.h"

Distributor::Distributor(QObject *parent) : QObject(parent){
    connect(&watchDirEye, &QFileSystemWatcher::directoryChanged, this, &Distributor::onWatchDirChange);
    connect(&tempDirEye, &QFileSystemWatcher::directoryChanged, this, &Distributor::onTempDirChange);
}

QList<ProcessObject> Distributor::createWorkObjects(QFileInfoList filesToProcess) {
    QList<ProcessObject> result;
    for(int i = 0; i < filesToProcess.size(); i++) {
        result.append(ProcessObject(filesToProcess.at(i).absoluteFilePath(), useKasper, useDrweb, kasperFilePath, drwebFilePath,
                                    tempDir, reportDir, cleanDir, dangerDir));
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

void Distributor::setTempDir(QString _tempDir) {
    if(_tempDir.isEmpty()) {
        qDebug() << "tempDir не выбрана! Изменения не произведены.";
    } else {
        tempDir = _tempDir;
        filesInTempDir.clear();
        processedFilesNb = 0;

        reportDir = tempDir + "/reports";
        QDir(tempDir).mkdir("reports");

        emit updateUi();
    }
}

QString Distributor::getTempDir() {
    return tempDir;
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
}

void Distributor::startTempDirEye() {
    stopTempDirEye();

    if(!tempDir.isEmpty()) {
        if(tempDirEye.addPath(tempDir)) {
            onTempDirChange("");
        } else {
            qDebug() << "Ошибка в пути " + tempDir;
        }
    }
}

void Distributor::stopTempDirEye() {
    if(!tempDirEye.directories().isEmpty()) {
        tempDirEye.removePaths(tempDirEye.directories());
        filesInTempDir.clear();
    }
}

void Distributor::onTempDirChange(const QString &path) {

    Q_UNUSED(path)

// v1
    // filesInTempDir.clear();
    //
    // QFileInfoList tempList = QDir(tempDir).entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    // QFileInfoList tempReportList = QDir(reportDir).entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    //
    // bool containsInReportDir;
    //
    // foreach(QFileInfo tempFileInfo, tempList) {
    //
    //     containsInReportDir = false;
    //
    //     foreach(QFileInfo reportFileInfo, tempReportList) {
    //         if(reportFileInfo.baseName() == tempFileInfo.baseName()) {
    //             containsInReportDir = true;
    //         }
    //     }
    //
    //     if(!containsInReportDir) {
    //         filesInTempDir.append(tempFileInfo);
    //     }
    // }
    
// v2
    // QFileInfoList newFilesInTempDir = QDir(tempDir).entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks),
    //               filesToProcess;
    //
    // foreach(QFileInfo fileInfo, newFilesInTempDir)
    //     if(!filesInTempDir.contains(fileInfo))
    //         filesToProcess.append(fileInfo);
    //
    // filesInTempDir = newFilesInTempDir;
    //
    // if(!filesToProcess.isEmpty())
    //     processDangerFiles(blockingMapped(createWorkObjects(filesToProcess), processFile));

// v3
    processDangerFiles(blockingMapped(createWorkObjects(QDir(tempDir).entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks)), processFile));

    updateUi();
}

int Distributor::getProcessedFilesNb() {
    return processedFilesNb;
}

int Distributor::getQueueSize() {
    return filesInTempDir.size();
}

ProcessObject Distributor::processFile(ProcessObject obj) {

    QString KReportFileName = obj.reportDir + "/" + obj.fileInfo.baseName() + ".kres",
            DReportFileName = obj.reportDir + "/" + obj.fileInfo.baseName() + ".dres",
            KReport, DReport;
    QFile KReportFile, DReportFile;
    bool isReportReady = false;

    KReportFile.setFileName(KReportFileName);
    DReportFile.setFileName(DReportFileName);

    if(!QFile::exists(KReportFileName) && !QFile::exists(DReportFileName)) {
        if(obj.useKasper)
            QProcess::execute(obj.kasperPath, QStringList() << "scan"
                                                            << obj.tempDir + "/" + obj.fileInfo.fileName()
                                                            << "/i0"
                                                            << QString("/R:" + obj.tempDir + "/reports/" + obj.fileInfo.baseName() + ".kres"));

        if(obj.useDrweb)
            QProcess::execute(obj.drwebPath, QStringList() << "/DR"
                                                           << QString("/RP:" + obj.tempDir + "/reports/" + obj.fileInfo.baseName() + ".dres")
                                                           << obj.tempDir + "/" + obj.fileInfo.fileName());
    }

    while(!isReportReady) {

        if(obj.useKasper) {
            if(KReportFile.open(QIODevice::ReadOnly)) {
                KReport = QTextStream(&KReportFile).readAll();
                KReportFile.close();
            }

            obj.kasperResult = QString::number(obj.fileInfo.lastModified().toSecsSinceEpoch());

            if(KReport.indexOf(QString("Total detected:   	")) != -1 && obj.fileInfo.lastModified().toSecsSinceEpoch() > 5)
                isReportReady = true;
        }

        if(obj.useDrweb) {
            if(DReportFile.open(QIODevice::ReadOnly)) {
                DReport = QTextStream(&DReportFile).readAll();
                DReportFile.close();
            }
        }

    }

    obj.kasperDetect = KReport.mid(KReport.indexOf(QString("Total detected:")), 20).contains("1");
    obj.drwebDetect = DReport.indexOf("file are infected") != -1;

    if(obj.kasperDetect || obj.drwebDetect) {
        QFile::rename(obj.tempDir   + "/" + obj.fileInfo.fileName(),
                      obj.dangerDir + "/" + obj.fileInfo.fileName());

        if(obj.useKasper) {
            while(QFile::exists(obj.reportDir + "/" + obj.fileInfo.baseName() + ".kres"))
                QFile::rename(obj.reportDir + "/" + obj.fileInfo.baseName() + ".kres",
                              obj.dangerDir + "/" + obj.fileInfo.baseName() + ".kres");
        }

        if(obj.useDrweb) {
            QFile::rename(obj.reportDir + "/" + obj.fileInfo.baseName() + ".dres",
                          obj.dangerDir + "/" + obj.fileInfo.baseName() + ".dres");
        }
    } else {
        QFile::rename(obj.tempDir   + "/" + obj.fileInfo.fileName(),
                      obj.cleanDir  + "/" + obj.fileInfo.fileName());

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

void Distributor::processDangerFiles(QList<ProcessObject> resultObjects) {
    QString report;
    processedFilesNb += resultObjects.size();
    foreach(ProcessObject ro, resultObjects){
        report = "Результат проверки файла " + ro.fileInfo.fileName() + ": ";
        // log(ro.kasperResult + "\n\n");
        if(ro.kasperDetect) report += "Kaspersky ";
        if(ro.drwebDetect)  report += "DrWeb ";
        if(ro.kasperDetect || ro.drwebDetect) log(report);
    }
}
