#ifndef CHECKER_H
#define CHECKER_H

#include <QObject>
#include <QThread>
#include <QThreadPool>
#include <QFuture>
#include <QtConcurrent>
#include <QFileSystemWatcher>
#include <QSet>
#include <QDir>
#include <QDebug>

class Checker : public QObject
{
    Q_OBJECT

    QFileSystemWatcher watcher;

    QString sourceDir;
    QString cleanDir;
    QString dangerDir;

    QString kasperFilePath;
    bool kasperFlag;

    QString drwebFilePath;
    bool drwebFlag;

    QStringList filesToCheck;
    QStringList filesInProgress;

    QThreadPool threadPool;

    int processedFilesNb;
    double processedFilesSizeMB;

public:
    explicit Checker(QObject *parent = nullptr);

    void setSourceDir(QString _sourceDir);
    QString getSourceDir();

    void setCleanDir(QString _cleanDir);
    QString getCleanDir();

    void setDangerDir(QString _dangerDir);
    QString getDangerDir();

    void setKasperFile(QString _kasperFilePath);
    QString getKasperFile();

    void setDrwebFile(QString _drwebFilePath);
    QString getDrwebFile();

    void setThreadsNb(int _threadsNb);
    int getThreadsNb();

    int getProcessedFilesNb();
    double getProcessedFileSize();

    int getQueueSize();

    void useKasper(bool isUsed);
    bool isKasperUsed();

    void useDrweb(bool isUsed);
    bool isDrwebUsed();

    // -----------------------------------------------------------
    void onSourceDirChange(const QString &path = "");
    void tryCheckFiles();

    void startWork();
    void stopWork();

    void clear();

signals:
    void updateUi();
    void log(QString info);
};

#endif // CHECKER_H
