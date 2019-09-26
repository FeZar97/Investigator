#include "checker.h"

Checker::Checker(QObject *parent) : QObject(parent) {
    connect(&watcher, &QFileSystemWatcher::directoryChanged, this, &Checker::onSourceDirChange);
    threadPool.setMaxThreadCount(QThread::idealThreadCount());
}

void Checker::setSourceDir(QString _sourceDir) {
    if(_sourceDir.isEmpty()) {
        qDebug() << "sourceDir не выбрана! Изменения не произведены.";
    } else {
        sourceDir = _sourceDir;
        clear();
        emit updateUi();
    }
}

QString Checker::getSourceDir() {
    return sourceDir;
}

void Checker::setCleanDir(QString _cleanDir) {
    if(_cleanDir.isEmpty()) {
        qDebug() << "cleanDir не выбрана! Изменения не произведены.";
    } else {
        cleanDir = _cleanDir;
        clear();
        emit updateUi();
    }
}

QString Checker::getCleanDir() {
    return cleanDir;
}

void Checker::setDangerDir(QString _dangerDir) {
    if(_dangerDir.isEmpty()) {
        qDebug() << "dangerDir не выбрана! Изменения не произведены.";
    } else {
        dangerDir = _dangerDir;
        clear();
        emit updateUi();
    }
}

QString Checker::getDangerDir() {
    return dangerDir;
}

void Checker::setKasperFile(QString _kasperFilePath) {
    if(_kasperFilePath.isEmpty()) {
        qDebug() << "kasperFilePath не выбрана! Изменения не произведены.";
    } else {
        kasperFilePath = _kasperFilePath;
        clear();
        emit updateUi();
    }
}

QString Checker::getKasperFile() {
    return kasperFilePath;
}

void Checker::setDrwebFile(QString _drwebFilePath) {
    if(_drwebFilePath.isEmpty()) {
        qDebug() << "drwebFilePath не выбрана! Изменения не произведены.";
    } else {
        drwebFilePath = _drwebFilePath;
        clear();
        emit updateUi();
    }
}

QString Checker::getDrwebFile() {
    return drwebFilePath;
}

int Checker::getProcessedFilesNb() {
    return processedFilesNb;
}

double Checker::getProcessedFileSize() {
    return processedFilesSizeMB;
}

void Checker::onSourceDirChange(const QString &path) {
    Q_UNUSED(path)

    filesToCheck = QDir(sourceDir + "/").entryList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    if(!filesToCheck.isEmpty())
        tryCheckFiles();
}

void Checker::setThreadsNb(int _threadsNb) {
    threadPool.setMaxThreadCount(_threadsNb);
}

int Checker::getThreadsNb() {
    return threadPool.maxThreadCount();
}

void Checker::tryCheckFiles() {
    if(!filesToCheck.isEmpty()) {

        QString fileName = "";

        for(int i = 0; i < filesToCheck.size(); i++) {
            if(!filesInProgress.contains(filesToCheck.at(i)))
                fileName = filesToCheck.at(i);
        }

        if(!fileName.isEmpty()) {

            filesToCheck.removeAll(fileName);
            filesInProgress.append(fileName);

            QtConcurrent::run(&threadPool, [=]() {
                processedFilesNb++;

                // log(sourceDir + "/" + fileName);

                processedFilesSizeMB += QFileInfo(sourceDir + "/" + fileName).size() / (1024. * 1024.);

                int kasperResult = QProcess::execute(kasperFilePath, QStringList() << "scan" << sourceDir + "/" + fileName << "/i0");
                if(kasperResult) {
                    log("Касперский :: Результат проверки файла " + fileName + ": " + QString::number(kasperResult));
                }

                int drwebResult = QProcess::execute(drwebFilePath, QStringList() << "/DR" << sourceDir + "/" + fileName);
                if(drwebResult) {
                    log("Drweb :: Результат проверки файла " + fileName + ": " + QString::number(drwebResult));
                }

                filesInProgress.removeAll(fileName);

                if(kasperResult || drwebResult) {
                    QFile::rename(sourceDir + "/" + fileName, dangerDir + "/" + fileName);
                } else {
                    QFile::rename(sourceDir + "/" + fileName, cleanDir + "/" + fileName);
                }


                emit updateUi();
            });
        }
    }
}

void Checker::startWork() {
    stopWork();



    if(!sourceDir.isEmpty()) {
        if(watcher.addPath(sourceDir)) {
            onSourceDirChange();
        } else {
            qDebug() << "Ошибка в пути " + sourceDir;
        }
    }
}

void Checker::stopWork() {
    if(!watcher.directories().isEmpty()) {
        watcher.removePaths(watcher.directories());
        clear();
    }
}

void Checker::clear() {
    processedFilesNb = 0;
    processedFilesSizeMB = 0;
    filesToCheck.clear();
}
