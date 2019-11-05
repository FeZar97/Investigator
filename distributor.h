#ifndef DISTRIBUTOR_H
#define DISTRIBUTOR_H

#include "avwrapper.h"

Q_DECLARE_METATYPE(QList<AVRecord>)

#define     VERSION              "#19.11.05/#3"

#define     KASPER_DIR_NAME       "kasper"
#define     DRWEB_DIR_NAME        "drweb"
#define     INPUT_DIR_NAME        "input"
#define     OUTPUT_DIR_NAME       "output"
#define     REPORT_DIR_NAME       "reports"
#define     PROCESSED_DIR_NAME    "processed"

class Distributor : public QObject
{
    Q_OBJECT

    AVWrapper kasperWrapper;
    QThread kasperThread;

    AVWrapper drwebWrapper;
    QThread drwebThread;

    QFileSystemWatcher watchDirEye;

    QString watchDir;
    QString investigatorDir;
    QString inputDir;
    QString outputDir;
    QString reportDir;

    QString cleanDir;
    QString dangerDir;

    AVBase recordBase;

public:
    explicit Distributor(QObject *parent = nullptr);
    ~Distributor();

// settings
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
    int getAVProcessedFilesNb(AV AVName);
    int getAVInprogressFilesNb(AV AVName);
    double getAVProcessedFilesSize(AV AVName);

    void configureAV();

// EVENTS
    void startWatchDirEye();
    void onWatchDirChange(const QString &path);

// CORE
    void sortingProcessedFiles();
    void updateBase(QList<AVRecord> list);

signals:
    void updateUi();
    void log(QString _message);
};

#endif // DISTRIBUTOR_H
