#include "mover.h"

Mover::Mover(QObject *parent) : QObject(parent) {
    QObject::connect(&watcher, &QFileSystemWatcher::directoryChanged, this, &Mover::onSourceDirChange);
    clear();
}

void Mover::setSourceDir(QString _sourceDir) {
    if(_sourceDir.isEmpty()) {
        qDebug() << "sourceDir не выбрана! Изменения не произведены.";
    } else {
        sourceDir = _sourceDir;
        clear();
        emit updateUi();
    }
}

QString Mover::getSourceDir() {
    return sourceDir;
}

void Mover::setTargetDir(QString _targetDir) {
    if(_targetDir.isEmpty()) {
        qDebug() << "targetDir не выбрана! Изменения не произведены.";
    } else {
        targetDir = _targetDir;
        clear();
        emit targetDirPathChange(targetDir);
        emit updateUi();
    }
}

QString Mover::getTargetDir() {
    return targetDir;
}

int Mover::getMovedFilesNb() {
    return movedFilesNb;
}

double Mover::getMovedFilesSize() {
    return movedFilesSizeMB;
}

void Mover::onSourceDirChange(const QString &path = "") {
    Q_UNUSED(path)

    // QSet<QString> sl = QDir(sourceDir + "/").entryList(QDir::Files | QDir::Hidden | QDir::NoSymLinks).toSet();
    // sl.unite(filesToMove);
    //
    // QString fileName;
    //
    // if(sl != filesToMove) {
    //     foreach(fileName, sl) {
    //         if(!filesToMove.contains(fileName)) {
    //             QFile::rename(sourceDir + "/" + fileName, targetDir + "/" + fileName);
    //
    //             movedFilesNb++;
    //             movedFilesSizeMB += QFileInfo(targetDir + "/" + fileName).size() / (1024. * 1024.);
    //
    //             sl.remove(fileName);
    //         }
    //     }
    // } else {
    //     filesToMove = QDir(sourceDir + "/").entryList(QDir::Files | QDir::Hidden | QDir::NoSymLinks).toSet();
    // }

    QString fileName;
    filesToMove = QDir(sourceDir + "/").entryList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    foreach(fileName, filesToMove) {
        QFile::rename(sourceDir + "/" + fileName, targetDir + "/" + fileName);

        movedFilesNb++;
        movedFilesSizeMB += QFileInfo(targetDir + "/" + fileName).size() / (1024. * 1024.);

        filesToMove.removeAll(fileName);
    }
}

void Mover::startWork() {
    stopWork();

    if(!sourceDir.isEmpty()) {
        if(watcher.addPath(sourceDir)) {
            onSourceDirChange();
        } else {
            qDebug() << "Ошибка в пути " + sourceDir;
        }
    }
}

void Mover::stopWork() {
    if(!watcher.directories().isEmpty()) {
        watcher.removePaths(watcher.directories());
        clear();
    }
}

void Mover::clear() {
    movedFilesNb = 0;
    movedFilesSizeMB = 0;
    filesToMove.clear();
}

void Mover::resetCounters() {
    movedFilesNb = 0;
    movedFilesSizeMB = 0;
}
