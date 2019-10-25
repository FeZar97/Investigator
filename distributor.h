#ifndef DISTRIBUTOR_H
#define DISTRIBUTOR_H

#include <QObject>
#include <QDateTime>
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

#define     VERSION             tr("#19.10.24/17:16#5")

#define     TEMP_DIR_NAME       "temp"
#define     REPORT_DIR_NAME     "reports"
#define     KASPER_DIR_NAME     "kasper"
#define     DRWEB_DIR_NAME      "drweb"

class ProcessObject {

public:
    ProcessObject();
    ProcessObject(QFileInfo _fileInfo,
                  bool _useKasper, bool _useDrweb,
                  QString _kasperPath, QString _drwebPath,
                  QString _investigatorDir, QString _cleanDir, QString _dangerDir) {
        fileInfo = _fileInfo;
        fileSize = QFileInfo(_fileInfo.absoluteFilePath()).size() / (1024. * 1024.);

        useKasper = _useKasper;
        kasperPath = _kasperPath;
        useDrweb = _useDrweb;
        drwebPath = _drwebPath;

        investigatorDir = _investigatorDir;

        tempDir   = investigatorDir + "/" + TEMP_DIR_NAME;
        reportDir = investigatorDir + "/" + REPORT_DIR_NAME;
        kasperDir = investigatorDir + "/" + KASPER_DIR_NAME;
        drwebDir  = investigatorDir + "/" + DRWEB_DIR_NAME;

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

    QString investigatorDir;

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

    QString investigatorDir;

    QString watchDir;
    QFileSystemWatcher watchDirEye;
    QFileInfoList filesInWatchDir;

    QString tempDir;
    QFileSystemWatcher tempDirEye;
    QFileInfoList filesInTempDir;

    QString reportDir;

    QString kasperDir;
    int kasperReportIdx{0};
    int kasperProcessedFilesNb{0};
    double kasperProcessedFilesSizeMb{0};
    QString kasperFilePath;
    bool useKasper;

    QString drwebDir;
    int drwebReportIdx{0};
    int drwebProcessedFilesNb{0};
    double drwebProcessedFilesSizeMb{0};
    QString drwebFilePath;
    bool useDrweb;

    QString cleanDir;
    QString dangerDir;

public:
    explicit Distributor(QObject *parent = nullptr);

    QList<ProcessObject> createWorkObjects(QFileInfoList filesToProcess);

// FOLDERS
    void setWatchDir(QString _watchDir);
    QString getWatchDir();

    void setInvestigatorDir(QString _investigatorDir);
    QString getInvestigatorDir();

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
    int getKasperProcessedFilesNb();
    double getKasperProcessedFilesSize();
    int getDrwebProcessedFilesNb();
    double getDrwebProcessedFilesSize();
    int getQueueSize();

// CORE
    void processDirByKasper(QString dirToProcess);
    void processDirByDrweb(QString dirToProcess);

// oth
    void setReadyFlag(bool state);

signals:
    void updateUi();
    void log(QString _message);
};

#endif // DISTRIBUTOR_H
