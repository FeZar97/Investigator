#include "distributor.h"

Distributor::Distributor(QObject *parent) : QObject(parent){
    connect(&watchDirEye, &QFileSystemWatcher::directoryChanged, this, &Distributor::onWatchDirChange);
    connect(&tempDirEye, &QFileSystemWatcher::directoryChanged, this, &Distributor::onTempDirChange);
    connect(&reportDirEye, &QFileSystemWatcher::directoryChanged, this, &Distributor::onReportDirChange);
}

QList<ProcessObject> Distributor::createWorkObjects() {

    QList<ProcessObject> result;
    for(int i = 0; i < filesInTempDir.size(); i++) {
        result.append(ProcessObject(filesInTempDir.at(i).absoluteFilePath(), useKasper, useDrweb, kasperFilePath, drwebFilePath));
    }
    return result;
}

QList<ResultObject> Distributor::createResultObjects() {
    QList<ResultObject> result;
    for(int i = 0; i < filesInReportDir.size(); i++) {
        if(!result.contains(filesInReportDir.at(i))) {
            result.append(ResultObject(filesInTempDir.at(i), tempDir, reportDir, cleanDir, dangerDir));
        }
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
    qDebug() << "onWatchDirChange";

    QFileInfo fileInfo;
    filesInWatchDir = QDir(watchDir + "/").entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    foreach(fileInfo, filesInWatchDir) {
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
    qDebug() << "onTempDirChange START";

    QFileInfo fileInfo;

    filesInTempDir.clear();

    QFileInfoList tempList = QDir(tempDir).entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    QFileInfoList tempReportList = QDir(reportDir).entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    for(int i = 0; i < tempList.size(); i++) {
        QString baseName = tempList.at(i).baseName();

        bool containsInReportDir = false;
        for(int j = 0; j < tempReportList.size(); j++) {

            if(tempReportList.at(j).baseName() == baseName) {
                containsInReportDir = true;
            }
        }

        if(!containsInReportDir) {
            filesInTempDir.append(tempList.at(i));
        }
    }

    qDebug() << "FILES TO SCAN: " << filesInTempDir.size() << endl;

    //filesInTempDir.append(QDir(tempDir).entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks));

    blockingMapped(createWorkObjects(), processFile);

    updateUi();

    qDebug() << "onTempDirChange END";
}

void Distributor::startReportDirEye() {
    stopReportDirEye();

    if(!reportDir.isEmpty()) {
        if(reportDirEye.addPath(reportDir)) {
            onReportDirChange("");
        } else {
            qDebug() << "Ошибка в пути " + reportDir;
        }
    }
}

void Distributor::stopReportDirEye() {
    if(!reportDirEye.directories().isEmpty()) {
        reportDirEye.removePaths(reportDirEye.directories());
        filesInReportDir.clear();
    }
}

void Distributor::onReportDirChange(const QString &path) {
    Q_UNUSED(path)

    filesInReportDir = QDir(reportDir).entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    // processDangerFiles(blockingMapped(createResultObjects(), processResult));

    updateUi();
}

int Distributor::getProcessedFilesNb() {
    return processedFilesNb;
}

int Distributor::getQueueSize() {
    return filesInTempDir.size();
}

ProcessObject Distributor::processFile(ProcessObject obj) {

    if(obj.useKasper)
        QProcess::execute(obj.kasperPath, QStringList() << "scan"
                                                        << obj.fileName
                                                        << "/i0"
                                                        << QString("/R:" + QFileInfo(obj.fileName).path() + "/reports/" + QFileInfo(obj.fileName).baseName() + ".kres"));

    if(obj.useDrweb)
        QProcess::execute(obj.drwebPath, QStringList() << "/DR"
                                                       << QString("/RP:" + QFileInfo(obj.fileName).path() + "/reports/" + QFileInfo(obj.fileName).baseName() + ".dres")
                                                       << obj.fileName);

    return obj;
}

ResultObject Distributor::processResult(ResultObject resultObject) {

    QString KReportFileName = resultObject.reportDir + "/" + resultObject.fileInfo.baseName() + ".kres",
            DReportFileName = resultObject.reportDir + "/" + resultObject.fileInfo.baseName() + ".dres";
    QFile KReportFile, DReportFile;

    if(QFile::exists(KReportFileName) && QFile::exists(DReportFileName)) {
        KReportFile.setFileName(KReportFileName);
        DReportFile.setFileName(DReportFileName);

        if(KReportFile.open(QIODevice::ReadOnly) && DReportFile.open(QIODevice::ReadOnly)) {
            const QString KReport = QTextStream(&KReportFile).readAll();
            const QString DReport = QTextStream(&DReportFile).readAll();

            KReportFile.close();
            DReportFile.close();

            resultObject.kasperDetect = KReport[KReport.indexOf(QString("Total detected:   	")) + QString("Total detected:   	").length()] != "0";
            resultObject.drwebDetect = DReport.indexOf("file are infected") != -1;

            if(resultObject.kasperDetect || resultObject.drwebDetect) {
                QFile::rename(resultObject.tempDir   + "/" + resultObject.fileInfo.fileName(),
                              resultObject.dangerDir + "/" + resultObject.fileInfo.fileName());

                QFile::rename(resultObject.reportDir + "/" + resultObject.fileInfo.baseName() + ".kres",
                              resultObject.dangerDir + "/" + resultObject.fileInfo.baseName() + ".kres");
                QFile::rename(resultObject.reportDir + "/" + resultObject.fileInfo.baseName() + ".dres",
                              resultObject.dangerDir + "/" + resultObject.fileInfo.baseName() + ".dres");
            } else {
                QFile::rename(resultObject.tempDir   + "/" +resultObject.fileInfo.fileName(),
                              resultObject.cleanDir  + "/" +resultObject.fileInfo.fileName());
                QFile::remove(resultObject.reportDir + "/" +resultObject.fileInfo.baseName() + ".kres");
                QFile::remove(resultObject.reportDir + "/" +resultObject.fileInfo.baseName() + ".dres");
            }
        }
    }

    return resultObject;
}

void Distributor::processDangerFiles(QList<ResultObject> resultObjects) {
    QString report;
    foreach(ResultObject ro, resultObjects){
        report = "Результат проверки файла " + ro.fileInfo.fileName() + ": ";
        if(ro.kasperDetect) report += "Kaspersky ";
        if(ro.drwebDetect)  report += "DrWeb ";
        if(ro.kasperDetect || ro.drwebDetect) log(report);
    }
}
