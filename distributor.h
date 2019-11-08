#ifndef DISTRIBUTOR_H
#define DISTRIBUTOR_H

#include "avwrapper.h"

Q_DECLARE_METATYPE(QList<AVRecord>)

#define     VERSION               "#7.11/1740"

#define     KASPER_DIR_NAME       "kasper"
#define     DRWEB_DIR_NAME        "drweb"
#define     INPUT_DIR_NAME        "input"
#define     OUTPUT_DIR_NAME       "output"
#define     REPORT_DIR_NAME       "reports"
#define     PROCESSED_DIR_NAME    "processed"

class Distributor : public QObject
{
    Q_OBJECT

    bool m_isProcessing;

    AVWrapper kasperWrapper;
    QThread kasperThread;

    AVWrapper drwebWrapper;
    QThread drwebThread;

    QDateTime startTime;
    QDateTime endTime;

    QFileSystemWatcher watchDirEye;

    QString watchDir;
    QString investigatorDir;
    QString inputDir;
    QString outputDir;
    QString reportDir;

    QString cleanDir;
    QString dangerDir;

    AVBase mainBase;

public:
    explicit Distributor(QObject *parent = nullptr);
    ~Distributor();

// SETTINGS
    void setWatchDir(QString _watchDir);
    QString getWatchDir();

    void setInvestigatorDir(QString _investigatorDir);
    QString getInvestigatorDir();

    void setCleanDir(QString _cleanDir);
    QString getCleanDir();

    void setDangerDir(QString _dangerDir);
    QString getDangerDir();

// AV wrappers
    void setAVFile(AV AVName, QString AVFilePath);
    QString getAVFile(AV AVName);
    void setAVUse(AV AVName, bool use);
    bool getAVUse(AV AVName);
    int getAVDangerFilesNb(AV AVName);
    int getAVCurrentReportIdx(AV AVName);
    int getAVQueueFilesNb(AV AVName);
    int getAVProcessedFilesNb(AV AVName);
    int getAVInprogressFilesNb(AV AVName);
    double getAVProcessedFilesSize(AV AVName);
    double getAVAverageSpeed(AV AVName);
    double getAVCurrentSpeed(AV AVName);

    void configureAV();

// CONTROL
    void startWatchDirEye();
    void stopWatchDirEye();

// EVENTS
    void onWatchDirChange(const QString &path);

// CORE
    void sortingProcessedFiles();
    void updateBase(AVBase& singleAVBase);

// OTHER
    qint64 getWorkTimeInSecs();
    bool isInProcessing();

signals:
    void updateUi();
    void log(QString _message);
};

#endif // DISTRIBUTOR_H
