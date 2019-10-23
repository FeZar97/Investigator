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

#define VERSION tr("#19.10.23/09:27#1")

#define REPORT_DIR_NAME  "reports"
#define KASPER_DIR_NAME  "kasper"
#define DRWEB_DIR_NAME   "drweb"

class ProcessObject {

public:
    ProcessObject();
    ProcessObject(QFileInfo _fileInfo,
                  bool _useKasper, bool _useDrweb,
                  QString _kasperPath, QString _drwebPath,
                  QString _tempDir, QString _cleanDir, QString _dangerDir) {
        fileInfo = _fileInfo;
        fileSize = QFileInfo(_fileInfo.absoluteFilePath()).size() / (1024. * 1024.);

        useKasper = _useKasper;
        kasperPath = _kasperPath;
        useDrweb = _useDrweb;
        drwebPath = _drwebPath;

        tempDir = _tempDir;

        reportDir = tempDir + "/" + REPORT_DIR_NAME;
        kasperDir = tempDir + "/" + KASPER_DIR_NAME;
        drwebDir =  tempDir + "/" + DRWEB_DIR_NAME;

        cleanDir = _cleanDir;
        dangerDir = _dangerDir;

        kasperDetect = false;
        drwebDetect = false;
    }

    QFileInfo fileInfo;
    double fileSize;

    bool useKasper;
    QString kasperPath;
    bool useDrweb;
    QString drwebPath;

    QString tempDir;

    QString reportDir;
    QString kasperDir;
    QString drwebDir;

    QString cleanDir;
    QString dangerDir;

    bool kasperDetect;
    bool drwebDetect;
};

class Distributor : public QObject
{
    Q_OBJECT

    bool readyFlag = true;

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
    QFileSystemWatcher kasperDirEye;
    QFileSystemWatcher drwebDirEye;

    QFileInfoList filesInWatchDir;
    QFileInfoList filesInTempDir;
    QFileInfoList filesInKasperDir;
    QFileInfoList filesInDrwebDir;

    int processedFilesNb;
    double processedFilesSizeMb;

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

    void startKasperDirEye();
    void stopKasperDirEye();
    void onKasperDirChange(const QString &path);

    void startDrwebDirEye();
    void stopDrwebDirEye();
    void onDrwebDirChange(const QString &path);

// RUN INFO
    int getProcessedFilesNb();
    double getProcessedFilesSize();
    int getQueueSize();

// CORE
    static ProcessObject processByKasper(ProcessObject obj);
    static ProcessObject processByDrweb(ProcessObject obj);
    static ProcessObject processResults(ProcessObject obj);
    void processDangerFiles(QList<ProcessObject> resultObjects);

signals:
    void checkFile();
    void updateUi();
    void log(QString _message);
};

#endif // DISTRIBUTOR_H
