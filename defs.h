#ifndef DEFS_H
#define DEFS_H

#include <QDateTime>
#include <QDialog>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <qmath.h>
#include <QMenu>
#include <QMessageBox>
#include <QObject>
#include <QTextCodec>
#include <QTextStream>
#include <QTimeZone>

#define     MAJOR_VERSION         "1"
#define     MINOR_VERSION         "6"
#define     PATCH_VERSION         "5.20"
#define     PATCH_IDENTIFICATOR   "15"
#define     VERSION               QString("v%1.%2.%3").arg(MAJOR_VERSION).arg(MINOR_VERSION).arg(PATCH_VERSION)

#define     INPUT_DIR_NAME        "input"
#define     OUTPUT_DIR_NAME       "output"
#define     CLEAN_DIR_NAME        "clean"
#define     DANGER_DIR_NAME       "danger"
#define     LOGS_DIR_NAME         "logs"
#define     REPORTS_DIR_NAME      "reports"

#define     LOG_FILE_REOPEN_TIME  5

#define     HTTP_PORT             8898

#define     ALL_FILES             -1
#define     MAX_FILES_TO_MOVE     20

const static QString dateTimePattern = "yyyy-MM-dd hh:mm:ss";
const static QDir::Filters usingFilters = QDir::Files | QDir::Hidden;

enum LOG_CATEGORY {
    NONE      = 0x00, // пустышка
    DEBUG     = 0x01, // вывод в файл логов, для сислога означает отправку полного лога на сервер
    GUI       = 0x02, // вывод в лог главного окна, для сислога означает отправку только случаев обнаружения вирусов
    DEBUG_ROW = 0x04  // вывод в строку состояния окна настроек
};

enum ACTION_TYPE {
    MOVE_TO_DIR,
    DELETE
};

/* перенос файлов из директории sourceDir в директорию destinationDir
 * с ограничением в limit файлов на итерацию
 * в случае, если время переноса файлов превышет время в [1], то выполнение функции прерывается */
inline void moveFiles(QString sourceDir, QString destinationDir, int limit) {

    if(!QDir(sourceDir).exists() || !QDir(destinationDir).exists())
        return;

    QFileInfoList filesInSourceDir = QDir(sourceDir + "/").entryInfoList(usingFilters),
                  filesInDestinationDir = QDir(destinationDir + "/").entryInfoList(usingFilters);

    QDateTime startMoveTime = QDateTime::currentDateTime();

    limit = qMin(limit, filesInSourceDir.size());
    if(limit > 0) {
        filesInSourceDir = filesInSourceDir.mid(0, limit);
    }

    while(!filesInSourceDir.isEmpty()) {
        foreach(QFileInfo fileInfo, filesInSourceDir) {
            QFileInfo oldFileInfo(fileInfo);
            if(QFile(fileInfo.absoluteFilePath()).exists()) {
                if(QFile::rename(fileInfo.absoluteFilePath(), destinationDir + "/" + fileInfo.fileName())) {
                    filesInSourceDir.removeAll(oldFileInfo);
                }
            }
        }

        /// [1]
        if(startMoveTime.msecsTo(QDateTime::currentDateTime()) > qMin(filesInSourceDir.size() * 3000, 60 * 1000) && filesInSourceDir.size()) {
            break;
        }
    }
}

/* перенос файла fileName из директории sourceDir в директорию destinationDir
 * если время переноса превышает [1], выполнение функции прерывается */
inline void moveFile(QString fileName, QString sourceDir, QString destinationDir) {
    if(!QFile(sourceDir + "/" + fileName).exists()
            || !QDir(sourceDir).exists()
            || !QDir(destinationDir).exists())
        return;

    QDateTime startMoveTime = QDateTime::currentDateTime();

    while((QDir(sourceDir).entryList()).contains(fileName)) {
        QFile::rename(sourceDir      + "/" + fileName,
                      destinationDir + "/" + fileName);

        /// [1]
        if(startMoveTime.msecsTo(QDateTime::currentDateTime()) >  5 * 1000 ||
           QFile(destinationDir + fileName).exists()) {
            break;
        }
    }
}

/* текущее время и дата в формате dateTimePattern */
inline QString formattedCurrentDateTime(){
    return QDateTime::currentDateTime().toString(dateTimePattern);
}

/* возвращает объем папки в МБ */
inline double dirSizeMb(QString dirName) {
    double volMb{0};
    foreach(QFileInfo fileInfo, QDir(dirName).entryInfoList(usingFilters)) {
        volMb += fileInfo.size();
    }
    volMb = (volMb * 8 / 1024) / 1024;
    return volMb;
}

/* конвертация QStringList -> QString */
inline QString entryListToString(QStringList &list) {
    QString res = "";
    if(list.size()) {
        for(int i = 0; i < list.size() - 1; i++) {
            res += list[i] + ", ";
        }
        res += list[list.size() - 1] + ".";
    }
    return res;
}

/* проверка на наличие файла fileName в списке fileList*/
inline bool isContainedFile(QList<QPair<QString, QString>> &fileList, QString fileName) {
    foreach(auto pair, fileList)
        if(pair.first == fileName)
            return true;
    return false;
}

/* конвертация объемов */
inline QString volumeToString(double volumeInMb) {
    if(volumeInMb < (1 << 10)) {
        return QString("%1 MB").arg(QString::number(volumeInMb, 'f', 2));
    } else if (volumeInMb/(1 << 10) < (1 << 10)) {
        return QString("%1 GB").arg(QString::number(volumeInMb/(1 << 10), 'f', 2));
    } else {
        return QString("%1 TB").arg(QString::number(volumeInMb/(1 << 20), 'f', 2));
    }
}

#endif // DEFS_H
