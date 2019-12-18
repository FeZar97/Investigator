#ifndef DISTRIBUTOR_H
#define DISTRIBUTOR_H

#include "avwrapper.h"

Q_DECLARE_METATYPE(AVBase)

class Distributor : public QObject
{
    Q_OBJECT

    bool m_isProcessing{false};

    AVWrapper kasperWrapper;
    QThread kasperThread;

    AVWrapper drwebWrapper;
    QThread drwebThread;

    QDateTime m_startTime;
    QDateTime m_endTime;

    QFileSystemWatcher watchDirEye;

    QString m_watchDir;
    QString m_investigatorDir;
    QString m_inputDir;
    QString m_outputDir;
    QString m_reportDir;

    QString m_cleanDir;
    QString m_dangerDir;

    AVBase mainBase;

    QString m_processInfo;
    QFile m_logFile;
    QTextStream m_logStream;

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
    void setMaxQueueSize(AV AVName, int size);
    int getMaxQueueSize(AV AVName);
    void setMaxQueueVol(AV AVName, double vol);
    double getMaxQueueVolMb(AV AVName);
    void setMaxQueueVolUnit(AV AVName, int unitIdx);
    int getMaxQueueVolUnit(AV AVName);
    double calcMaxQueueVol(AV AVName);
    int getAVDangerFilesNb(AV AVName);
    int getAVCurrentReportIdx(AV AVName);
    int getAVQueueFilesNb(AV AVName);
    double getAVQueueFilesVolMb(AV AVName);
    int getAVProcessedFilesNb(AV AVName);
    int getAVInProgressFilesNb(AV AVName);
    double getAVProcessedFilesSize(AV AVName);
    double getAVAverageSpeed(AV AVName);
    double getAVCurrentSpeed(AV AVName);

    void configureAV();

// CONTROL
    void startWatchDirEye();
    void stopWatchDirEye();

// EVENTS
    void onWatchDirChange(const QString &path);

// OTHER
    QDateTime getStartTime() const;
    QDateTime getEndTime();
    bool isInProcessing();
    void clearDir(QString dirName);
    double dirSizeMb(QString dirName);
    void clearStatistic();
    void moveCleanFiles();
    void setProcessInfo(QString info);
    QString getProcessInfo() const;
    void log(QString text, LOG_DST flags);

signals:
    void updateUi();
    void logGui(QString text);
};

#endif // DISTRIBUTOR_H
