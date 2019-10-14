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

class ProcessObject {
public:
    ProcessObject();
    ProcessObject(QString _fileName,
                  bool _useKasper, bool _useDrweb,
                  QString _kasperPath, QString _drwebPath) {
        fileName = _fileName;
        useKasper = _useKasper;
        useDrweb = _useDrweb;
        kasperPath = _kasperPath;
        drwebPath = _drwebPath;
    }

    QString fileName;

    bool useKasper;
    bool useDrweb;
    QString kasperPath;
    QString drwebPath;
};

class ResultObject {
public:
    ResultObject();

    ResultObject(QFileInfo _fileInfo) {
        fileInfo = _fileInfo;
    }

    ResultObject(QFileInfo _fileInfo,
                 QString _tempDir, QString _reportDir, QString _cleanDir, QString _dangerDir) {
        fileInfo = _fileInfo;

        tempDir = _tempDir;
        reportDir = _reportDir;
        cleanDir = _cleanDir;
        dangerDir = _dangerDir;

        kasperDetect = false;
        drwebDetect = false;
    }

    QFileInfo fileInfo;

    QString tempDir;
    QString reportDir;
    QString cleanDir;
    QString dangerDir;

    bool kasperDetect;
    bool drwebDetect;

    bool operator==(const ResultObject &otherObj) {
        return fileInfo.baseName() == otherObj.fileInfo.baseName();
    }
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
    QFileInfoList filesInReportDir;

    int processedFilesNb;

public:
    explicit Distributor(QObject *parent = nullptr);

    QList<ProcessObject> createWorkObjects();
    QList<ResultObject> createResultObjects();

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

    void startReportDirEye();
    void stopReportDirEye();
    void onReportDirChange(const QString &path);

// RUN INFO
    int getProcessedFilesNb();
    int getQueueSize();

// CORE
    static ProcessObject processFile(ProcessObject obj);
    static ResultObject processResult(ResultObject resultObject);
    void processDangerFiles(QList<ResultObject> resultObjects);

signals:
    void checkFile();
    void updateUi();
    void log(QString _message);
};

#endif // DISTRIBUTOR_H
