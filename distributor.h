#ifndef DISTRIBUTOR_H
#define DISTRIBUTOR_H

#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QDebug>
#include <QDir>
#include <QSet>
#include <QTimer>
#include <QProcess>

#include <QThread>
#include <QThreadPool>
#include <QFuture>
#include <QtConcurrent>
#include <QList>

using namespace QtConcurrent;

#define VERSION tr("#19.10.15/16:51")

class ProcessObject {
public:
    ProcessObject();
    ProcessObject(QFileInfo _fileInfo,
                  bool _useKasper, bool _useDrweb,
                  QString _kasperPath, QString _drwebPath,
                  QString _tempDir, QString _reportDir, QString _cleanDir, QString _dangerDir) {
        fileInfo = _fileInfo;
        useKasper = _useKasper;
        useDrweb = _useDrweb;
        kasperPath = _kasperPath;
        drwebPath = _drwebPath;

        tempDir = _tempDir;
        reportDir = _reportDir;
        cleanDir = _cleanDir;
        dangerDir = _dangerDir;

        kasperDetect = false;
        kasperResult = "";
        drwebDetect = false;
    }

    QFileInfo fileInfo;

    bool useKasper;
    bool useDrweb;
    QString kasperPath;
    QString drwebPath;

    QString tempDir;
    QString reportDir;
    QString cleanDir;
    QString dangerDir;

    bool kasperDetect;
    QString kasperResult;
    bool drwebDetect;
};

class Distributor : public QObject
{
    Q_OBJECT

    QString watchDir;
    QString tempDir;
    QString reportDir;
    QString cleanDir;
    QString dangerDir;

    QString kasperFilePath;
    bool useKasper;

    QString drwebFilePath;
    bool useDrweb;

    QFileSystemWatcher watchDirEye;
    QFileSystemWatcher tempDirEye;
    QFileSystemWatcher reportDirEye;

    QFileInfoList filesInWatchDir;
    QFileInfoList filesInTempDir;

    int processedFilesNb;

public:
    explicit Distributor(QObject *parent = nullptr);

    QList<ProcessObject> createWorkObjects(QFileInfoList filesToProcess);

// FOLDERS
    void setWatchDir(QString _watchDir);
    QString getWatchDir();

    void setTempDir(QString _tempDir);
    QString getTempDir();

    void setCleanDir(QString _cleanDir);
    QString getCleanDir();

    void setDangerDir(QString _dangerDir);
    QString getDangerDir();

// ANTIVIRUS FILES
    void setKasperFile(QString _kasperFilePath);
    QString getKasperFile();
    void setUseKasper(bool _useKasper);
    bool isKasperUse();

    void setDrwebFile(QString _drwebFilePath);
    QString getDrwebFile();
    void setUseDrweb(bool _useDrweb);
    bool isDrwebUse();

// EVENTS
    void startWatchDirEye();
    void stopWatchDirEye();
    void onWatchDirChange(const QString &path);

    void startTempDirEye();
    void stopTempDirEye();
    void onTempDirChange(const QString &path);

// RUN INFO
    int getProcessedFilesNb();
    int getQueueSize();

// CORE
    static ProcessObject processFile(ProcessObject obj);
    void processDangerFiles(QList<ProcessObject> resultObjects);

signals:
    void checkFile();
    void updateUi();
    void log(QString _message);
};

#endif // DISTRIBUTOR_H
