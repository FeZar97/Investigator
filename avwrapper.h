#ifndef AVWRAPPER_H
#define AVWRAPPER_H

#include <QObject>
#include <QDateTime>
#include <QFileSystemWatcher>
#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QThread>
#include <QList>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QTextStream>

enum class AV {
    KASPER,
    DRWEB
};

struct AVRecord {
    QDateTime m_timeMark;
    AV m_av;
    QString m_fileName;
    QString m_description;
    QString m_reportName;

    AVRecord(QDateTime dt, AV av, QString fileName, QString description, QString reportName);
    AVRecord(AVRecord &record);

    QString toString();

    AVRecord& operator=(AVRecord &record);
};

class AVBase {
    QList<QPair<AVRecord, AVRecord>> base;
public:

    int findFileName(QString fileName);
    void add(AVRecord record);
    void add(QPair<AVRecord, AVRecord> record);
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

    // statistics
    int m_reportIdx{0};
    int m_processedFilesNb{0};
    int m_inprogressFilesNb{0};
    double m_processedFilesSizeMb{0.};

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
    static QString getName(AV type);

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

    int getProcessedFilesNb();
    int getInprogressFilesNb();
    double getProcessedFilesSize();

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
    void updateList(QList<AVRecord> list);
    void finalProcessing();
    void updateUi();
};

#endif // AVWRAPPER_H
