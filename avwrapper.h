#ifndef AVWRAPPER_H
#define AVWRAPPER_H

#include <QObject>
#include <QDateTime>
#include <QTimeZone>
#include <QFileSystemWatcher>
#include <QDir>
#include <QProcess>
#include <QThread>
#include <QList>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include <QDebug>

#define     VERSION               "v1.3.27:5"

#define     KASPER_DIR_NAME       "kasper"
#define     DRWEB_DIR_NAME        "drweb"
#define     INPUT_DIR_NAME        "input"
#define     OUTPUT_DIR_NAME       "output"
#define     REPORT_DIR_NAME       "reports"
#define     PROCESSED_DIR_NAME    "processed"

enum class AV {
    NONE,
    KASPER,
    DRWEB
};

const static QString dateTimePattern = "yyyy/MM/dd hh:mm:ss";
const static QDir::Filters usingFilters = QDir::Files | QDir::Hidden;

struct AVRecord {
    QDateTime m_timeMark;
    AV m_av;
    QString m_fileName{""};
    QString m_description;
    QString m_reportName;

    AVRecord();
    AVRecord(QDateTime dt, AV av, QString fileName, QString description, QString reportName);
    AVRecord(const AVRecord &record);

    QString toString();

    AVRecord& operator=(const AVRecord &record);
};

class AVBase {
    QList<QPair<AVRecord, AVRecord>> m_base;

public:
    int findFileName(QString fileName);

    void add(AVRecord record);
    void add(QPair<AVRecord, AVRecord>& record);

    QPair<AVRecord, AVRecord>& operator[](int idx);

    int size();
};

void moveFiles(QString sourceDir, QString destinationDir);
QString getName(AV type);
QString currentDateTime();

class AVWrapper : public QObject
{
    Q_OBJECT

    bool m_readyToProcess{true};
    QFileSystemWatcher m_watcher;

    // AV params
    AV m_type;
    bool m_isUsed{true};
    QString m_avPath{"C:/"};
    QString m_reportExtension{"unknown"};
    QString m_reportName;
    QFile m_reportFile;
    int m_maxQueueSize{0};
    double m_maxQueueVolMb{0.};
    int m_maxQueueVolUnit{0};

    // indicators
    QString m_reportReadyIndicator;
    QString m_startRecordsIndicator;
    QString m_endRecordsIndicator;
    QStringList m_denyStrings;

    // folders
    QString m_investigatorDir;
    QString m_inputFolder;
    QString m_processFolder;
    QString m_outputFolder;
    QString m_reportFolder;
    QString m_dangerFolder;

    // statistics
    QDateTime m_startProcessTime;
    qint64 m_totalWorkTimeInMsec{0};
    int m_dangerFileNb{0};
    int m_reportIdx{0};
    int m_inProgressFilesNb{0};
    int m_processedFilesNb{0};
    double m_processedFilesSizeMb{0.};
    double m_processedLastFilesSizeMb{0.};
    double m_currentProcessSpeed{0.};

    // execution arguments
    QStringList m_execArgs;

    // temp
    QTextStream m_stream;
    QString m_report;
    QString m_reportLine;

public:
    explicit AVWrapper(QObject *parent = nullptr);

    void setType(AV type);

    void setUsage(bool newState);
    bool getUsage();

    void setAVPath(QString avPath);
    QString getAVPath();

    void setReportExtension(QString extension);
    QString getReportExtension();

    void setReportFolder(QString reportFolder);
    QString getReportFolder();

    void setInputFolder(QString inputFolder);
    QString getInputFolder();

    void setProcessFolder(QString processFolder);
    QString getProcessFolder();

    void setOutputFolder(QString outputFolder);
    QString getOutputFolder();

    void setDangerFolder(QString dangerFolder);
    QString getDangerFolder();

    void setInvestigatorFolder(QString investigatorDir);

    void setFolders(QString investigatorDir, QString inputFolder, QString processFolder, QString outputFolder, QString reportFolder);

    void setMaxQueueSize(int size);
    int getMaxQueueSize();

    void setMaxQueueVolMb(double volMb);
    double getMaxQueueVolMb();

    void setMaxQueueVolUnit(int maxQueueVolUnit);
    int getMaxQueueVolUnit();

    int getDangerFilesNb();
    int getCurrentReportIdx();
    int getProcessedFilesNb();
    int getInProgressFilesNb();
    double getProcessedFilesSize();
    double getAverageSpeed();
    double getCurrentSpeed();
    void clearStatistic();

    void setExecArgs(QStringList execArgs);
    void addExecArgs(QStringList additionExecArgs);
    QStringList getExecArgs();
    void deleteExecArgs(QStringList delExecArgs);

    int checkParams();

    bool isReadyToProcess();

    void setIndicators(QString readyIndicator, QString startRecordsIndicator, QString endRecordsIndicator, QStringList permitStrings);

    bool isPayload(QString line);
    QString extractInfectedFileName(QString reportLine);
    QString extractDescription(QString reportLine, QString fileName);

// CORE
    void process();

signals:
    void log(QString message);
    void updateBase(AVBase* singleAVBase);
    void finishProcess();
    void setProcessInfo(QString info);
};

#endif // AVWRAPPER_H
