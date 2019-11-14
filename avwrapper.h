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

enum class AV {
    NONE,
    KASPER,
    DRWEB
};

const static QString dateTimePattern = "yyyy/MM/dd hh:mm:ss";
const static QDir::Filters usingFilters = QDir::Files | QDir::Hidden | QDir::NoSymLinks;

struct AVRecord {
    QDateTime m_timeMark;
    AV m_av;
    QString m_fileName;
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

    void add(AVRecord* record);
    void add(QPair<AVRecord, AVRecord>* record);
    void add(AVBase& base);

    void remove(QString fileName);
    void remove(int idx);
    void clear();

    QPair<AVRecord, AVRecord>& operator[](int idx);

    int size();
};

void moveFiles(QString sourceDir, QString destinationDir);

class AVWrapper : public QObject
{
    Q_OBJECT

    bool m_readyToProcess{true};

    // AV params
    AV m_type;
    bool m_isUsed{true};
    QString m_avPath{"C:/"};
    QString m_reportExtension{"unknown"};
    QString m_reportName;
    QFile m_reportFile;
    int m_maxQueueSize{0};
    double m_maxQueueVol{0.};

    // indicators
    QString m_reportReadyIndicator;
    QString m_startRecordsIndicator;
    QString m_endRecordsIndicator;
    QStringList m_permitStrings;

    // folders
    QString m_inputFolder;
    QString m_processFolder;
    QString m_outputFolder;
    QString m_reportFolder;
    QString m_dangerFolder;

    // statistics
    AVBase m_avBase;
    int m_dangerFileNb{0};
    int m_reportIdx{0};
    int m_processedFilesNb{0};
    int m_inprogressFilesNb{0};
    double m_processedFilesSizeMb{0.};

    double m_processedLastFilesSizeMb{0.};
    double m_currentProcessSpeed{0.};

    // execution arguments
    QStringList m_execArgs;

    // temp
    bool m_isReportReady;
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

    void setMaxQueueSize(int size);
    int getMaxQueueSize();

    void setMaxQueueVol(double vol);
    double getMaxQueueVol();

    int getDangerFilesNb();
    int getCurrentReportIdx();
    int getProcessedFilesNb();
    int getInprogressFilesNb();
    double getProcessedFilesSize();
    double getAverageSpeed(qint64 workTime);
    double getCurrentSpeed();

    void setExecArgs(QStringList execArgs);
    void addExecArgs(QStringList additionExecArgs);
    QStringList getExecArgs();
    void deleteExecArgs(QStringList delExecArgs);

    int checkParams();

    bool isReadyToProcess();

    void setIndicators(QString readyIndicator, QString startRecordsIndicator, QString endRecordsIndicator, QStringList permitStrings);

    bool isPayload(QString line);
    QString extractFileName(QString reportLine);
    QString extractDescription(QString reportLine, QString fileName);

// CORE
    void process();

signals:
    void log(QString message);
    void updateBase(AVBase& singleAVBase);
    void finalProcessing();
    void updateUi();
};

QString getName(AV type);
QString currentDateTime();

#endif // AVWRAPPER_H
