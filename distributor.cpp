#include "distributor.h"

Distributor::Distributor(QObject *parent) : QObject(parent){
    readyFlag = true;
    connect(&watchDirEye,  &QFileSystemWatcher::directoryChanged, this, &Distributor::onWatchDirChange);
    connect(&tempDirEye,   &QFileSystemWatcher::directoryChanged, this, &Distributor::onTempDirChange);
    connect(&kasperDirEye, &QFileSystemWatcher::directoryChanged, this, &Distributor::onKasperDirChange);
    connect(&drwebDirEye,  &QFileSystemWatcher::directoryChanged, this, &Distributor::onDrwebDirChange);
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

void Distributor::setTempDir(QString _tempDir) {
    if(_tempDir.isEmpty()) {
        qDebug() << "tempDir не выбрана! Изменения не произведены.";
    } else {
        tempDir = _tempDir;
        filesInTempDir.clear();
        processedFilesNb = 0;
        processedFilesSizeMb = 0;

        reportDir = tempDir + "/reports";
        QDir(tempDir).mkdir("reports");
        QDir(tempDir).mkdir("kasper");
        QDir(tempDir).mkdir("drweb");

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

    if(readyFlag) {

        filesInWatchDir = QDir(watchDir + "/").entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

        foreach(QFileInfo fileInfo, filesInWatchDir) {
            QFile::rename(fileInfo.absoluteFilePath(), tempDir + "/" + fileInfo.fileName());

            filesInWatchDir.removeAll(fileInfo);
        }
    }

    readyFlag = false;
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

    blockingMapped(createWorkObjects(QDir(tempDir).entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks)), processByKasper);

    updateUi();
}

void Distributor::startKasperDirEye(){
    stopKasperDirEye();

    if(kasperDirEye.addPath(tempDir + "/" + KASPER_DIR_NAME)) {
        onKasperDirChange("");
    } else {
        qDebug() << "Ошибка в пути " + tempDir + "/" + KASPER_DIR_NAME;
    }
}

void Distributor::stopKasperDirEye() {
    if(!kasperDirEye.directories().isEmpty()) {
        kasperDirEye.removePaths(kasperDirEye.directories());
        filesInKasperDir.clear();
    }
}

void Distributor::onKasperDirChange(const QString &path) {

    Q_UNUSED(path)

    blockingMapped(createWorkObjects(QDir(tempDir + "/" + KASPER_DIR_NAME).entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks)), processByDrweb);

    updateUi();
}

void Distributor::startDrwebDirEye() {
    stopDrwebDirEye();

    if(drwebDirEye.addPath(tempDir + "/" + DRWEB_DIR_NAME)) {
        onDrwebDirChange("");
    } else {
        qDebug() << "Ошибка в пути " + tempDir + "/" + DRWEB_DIR_NAME;
    }
}

void Distributor::stopDrwebDirEye() {
    if(!drwebDirEye.directories().isEmpty()) {
        drwebDirEye.removePaths(drwebDirEye.directories());
        filesInDrwebDir.clear();
    }
}

void Distributor::onDrwebDirChange(const QString &path) {
    Q_UNUSED(path)
    processDangerFiles(blockingMapped(createWorkObjects(QDir(tempDir + "/" + DRWEB_DIR_NAME).entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks)), processResults));

    updateUi();
}

int Distributor::getProcessedFilesNb() {
    return processedFilesNb;
}

double Distributor::getProcessedFilesSize() {
    return processedFilesSizeMb;
}

int Distributor::getQueueSize() {
    return filesInWatchDir.size();
}

ProcessObject Distributor::processByKasper(ProcessObject obj) {

    QString KReportFileName = obj.reportDir + "/" + obj.fileInfo.baseName() + ".kres",
            KReport;
    QFile KReportFile;
    KReportFile.setFileName(KReportFileName);
    bool isReportReady = false;

    // команда на сканирование
    if(obj.useKasper && !QFile::exists(KReportFileName) && QFile::exists(obj.tempDir + "/" + obj.fileInfo.fileName())) {
        QProcess::execute(obj.kasperPath, QStringList() << "scan"
                                                        << obj.tempDir + "/" + obj.fileInfo.fileName()
                                                        << "/i0"
                                                        << QString("/R:" + obj.tempDir + "/reports/" + obj.fileInfo.baseName() + ".kres"));
    }

    // ждем пока отчет сформируется
    while(!isReportReady) {
        if(obj.useKasper && QFile::exists(KReportFileName)) {
            if(KReportFile.open(QIODevice::ReadOnly)) {
                KReport = QTextStream(&KReportFile).readAll();
                KReportFile.close();
            }

            if(KReport.indexOf(QString("Total detected:   	")) != -1)
                isReportReady = true;
        } else {
            isReportReady = true;
        }
    }

    // перемещение файла в папку каспера для дальнейшего сканирования доктором вебом
    // только если есть отчет
    if((obj.useKasper && QFile::exists(KReportFileName)) || !obj.useKasper)
        QFile::rename(obj.tempDir   + "/" + obj.fileInfo.fileName(),
                      obj.kasperDir + "/" + obj.fileInfo.fileName());
    return obj;
}

ProcessObject Distributor::processByDrweb(ProcessObject obj) {

    QString DReportFileName = obj.reportDir + "/" + obj.fileInfo.baseName() + ".dres",
            DReport;
    QFile DReportFile;
    DReportFile.setFileName(DReportFileName);

    // команда на сканирование
    if(obj.useDrweb && !QFile::exists(DReportFileName) && QFile::exists(obj.kasperDir + "/" + obj.fileInfo.fileName())) {
        QProcess::execute(obj.drwebPath, QStringList() << "/DR"
                                                       << QString("/RP:" + obj.tempDir + "/reports/" + obj.fileInfo.baseName() + ".dres")
                                                       << obj.kasperDir + "/" + obj.fileInfo.fileName());
    }

    // ждем пока отчет сформируется
    if(obj.useDrweb && QFile::exists(DReportFileName)) {
        if(DReportFile.open(QIODevice::ReadOnly)) {
            DReport = QTextStream(&DReportFile).readAll();
            DReportFile.close();
        }
    }

    // перемещение файла в папку drweb для дальнейшей проверки отчетов
    // только если есть отчет
    if((obj.useDrweb && QFile::exists(DReportFileName)) || !obj.useDrweb)
        QFile::rename(obj.kasperDir + "/" + obj.fileInfo.fileName(),
                      obj.drwebDir  + "/" + obj.fileInfo.fileName());

    return obj;
}


ProcessObject Distributor::processResults(ProcessObject obj) {

    /*
    QString KReportFileName = obj.reportDir + "/" + obj.fileInfo.baseName() + ".kres",
            DReportFileName = obj.reportDir + "/" + obj.fileInfo.baseName() + ".dres",
            KReport,
            DReport;
    QFile KReportFile, DReportFile;
    bool isReportReady = false;

    KReportFile.setFileName(KReportFileName);
    DReportFile.setFileName(DReportFileName);

    if(obj.useKasper && !QFile::exists(KReportFileName)) {
        QProcess::execute(obj.kasperPath, QStringList() << "scan"
                                                        << obj.tempDir + "/" + obj.fileInfo.fileName()
                                                        << "/i0"
                                                        << QString("/R:" + obj.tempDir + "/reports/" + obj.fileInfo.baseName() + ".kres"));
    }

    if(obj.useDrweb && !QFile::exists(DReportFileName)) {
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

            if(KReport.indexOf(QString("Total detected:   	")) != -1)
                isReportReady = true;
        } else {
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
    */

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
        // перенос в dangerDir
        QFile::rename(obj.drwebDir  + "/" + obj.fileInfo.fileName(),
                      obj.dangerDir + "/" + obj.fileInfo.fileName());

        // перенос репорта KASPER
        if(obj.useKasper && QFile::exists(obj.reportDir + "/" + obj.fileInfo.baseName() + ".kres")) {
            while(QFile::exists(obj.reportDir + "/" + obj.fileInfo.baseName() + ".kres"))
                QFile::rename(obj.reportDir + "/" + obj.fileInfo.baseName() + ".kres",
                              obj.dangerDir + "/" + obj.fileInfo.baseName() + ".kres");
        }

        // перенос репорта DRWEB
        if(obj.useDrweb) {
            QFile::rename(obj.reportDir + "/" + obj.fileInfo.baseName() + ".dres",
                          obj.dangerDir + "/" + obj.fileInfo.baseName() + ".dres");
        }

    } else {
        // перенос в cleanDir
        QFile::rename(obj.drwebDir + "/" + obj.fileInfo.fileName(),
                      obj.cleanDir + "/" + obj.fileInfo.fileName());

        // удаление репорта KASPER
        if(obj.useKasper) {
            while(QFile::exists(obj.reportDir + "/" + obj.fileInfo.baseName() + ".kres"))
                QFile::remove(obj.reportDir + "/" + obj.fileInfo.baseName() + ".kres");
        }

        // удаление репорта DRWEB
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
        processedFilesSizeMb += ro.fileSize;
        if(ro.kasperDetect) report += "Kaspersky ";
        if(ro.drwebDetect)  report += "DrWeb ";
        if(ro.kasperDetect || ro.drwebDetect) log(report);
    }

    readyFlag = true;
    onWatchDirChange("");
}
