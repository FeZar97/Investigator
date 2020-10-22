#ifndef CHECKWORKER_H
#define CHECKWORKER_H

#include <QObject>
#include <QFileInfoList>
#include <QProcess>
#include <QDateTime>
#include <QTextCodec>
#include <QXmlStreamReader>

#include <../FeZarSource/FeZar97.h>

class WorkerStatistic {
public:
    int workerId;
    quint64 processedFilesNb; // обработано кол-во
    quint64 processedFilesSize; // обработано объем
    quint64 pwdFilesNb; // кол-во запароленных
    quint64 infFilesNb; // кол-во запароленных
    quint64 currentSpeed; // скорость
    QString avsVersion; // версия АВС

    WorkerStatistic(int wId = -1, quint64 cPFNb = 0, quint64 cPFSize = 0, quint64 cPwdFNb = 0,
                    quint64 cInfFNb = 0, quint64 cSpd = 0, QString aV = "") {
        workerId = wId;
        processedFilesNb = cPFNb;
        processedFilesSize = cPFSize;
        pwdFilesNb = cPwdFNb;
        infFilesNb = cInfFNb;
        currentSpeed = cSpd;
        avsVersion = aV;
    }
};

// вернуть набор (проверено, объем, скорость, запароленных, ошибок, список(зараженный, подробности))
class InvestigatorWorker: public QObject {
    Q_OBJECT

    int m_id; // идентификатор
    bool m_isInWork{false};
    bool m_isInProcess{false};

    QString m_inputDir; // директория, из которой забираются файлы
    QString m_processDir; // директория для воркеров
    QString m_workerProcessDir; // директория, в которой осуществляется проверка
    QString m_dangerDir; // директория для зараженных файлов
    QString m_cleanDir; // директория для чистых файлов
    QString m_reportDir; // директория для отчетов АВС

    bool m_saveXmlReports; // флаг сохранения xml отчетов

    bool m_useExternalHandler; // флаг использования внешнего обработчика
    QString m_externalHandlerPath; // путь к внешнему обработчику

    QFileInfoList m_filesInProcess; // список файлов на проверке
    QString m_avsExecFileName{"C:/Program Files/Primetech/M-52/AVSFileConsoleScan.exe"}; // путь до исполняемого файла
    QProcess *m_avsProcess{nullptr}; // процесс проверки

    QString m_avsReport; // отчет АВС
    QTextCodec *m_win1251Codec; // кодек для конвертации отчета М52

    QDateTime m_lastProcStartTime; // время начала последней проверки
    quint64 m_lastProcWorkTimeInMsec; // время обработки последней выборки

    // времянки
    QStringList m_reportLines;
    QString m_baseAVSVersion, m_m52coreVersion, m_drwebCoreVersion,
            m_kasperCoreVersion; // версии баз
    QString m_avVersion; // все версии
    QStringList m_tempSplitList1, m_tempSplitList2;
    QString m_tempInfectedFileName, m_tempVirusInfo;

    // счетчики
    quint64 m_filesToProcessSize; // объем просканированных файлов за последнюю проверку в байтах
    quint64 m_speed{0}; // скорость последнего сканирования
    quint64 m_pwdFilesNb; // кол-во запароленных файлов за последнюю проверку
    QList<QPair<QString, QString>> m_infList; // список зараженных файлов

    bool canAcceptWork(int
                       id); // проверка возможности запуска проверки
    void checkProcessDirExists(); // проверка возможности запуска проверки
    void flushStatistic(); // сброс статистики
    void parseReport(); // парсинг отчета

    void createAvsProcess();

    // методы для parseReport
    void extractAVSVersions();
    void collectStatisticAboutLastScan();// сохранение объема проверенных файлов и скорости проверки

    // обработка файлов
    void processingInfectedFiles();
    void processingOtherFiles();

    // сохранение отчета АВС об инфицированных файлах
    QString getInfectedReportFileName(); // имя отчета
    void saveXmlFileReport(QString fileName = "", QString report = "");

    /* fileName - имя json файла (совпадает с именем зараженного файла)
     * report - строка (<antivirus>: <virus info>)
     * avVersion - сводная информация об антивирусе (состав, базы, весрии, даты) */
    void createVirusXml(QString xmlFileName, QString infectedFileName, QString report,
                        QString avVersion);

    /* параметры зараженного файла */
    QStringList infectedFileInfo(QString infectedFileName, QString report, QString avVersion);

public:
    InvestigatorWorker(QObject *parent);

    bool isInProcess() {
        return m_isInProcess;
    }
    quint64 speed() {
        return m_speed;
    }

    void startWork();
    void stopWork();

    void setAvsExecFileName(QString avsExecFileName) {
        m_avsExecFileName = avsExecFileName;
    }

    // передача воркеру внешних параметров
    void configure(int id, QStringList dirList, QString m_avsExecFileName, bool useExternalHandler,
                   QString externalhandlerPath, bool saveXmlReports);

    // запуск процесса проверки для воркера с id
    void tryCheckFiles(int id, QFileInfoList filesToProcess);

    // возвращает статистику по последнему скнаированию
    WorkerStatistic getLastStatistics();

    // кол-во файлов в директории воркера
    qint64 filesInProcessNb() {

        if (QDir(m_workerProcessDir).exists()) {
            return QDir(m_workerProcessDir).entryInfoList(usingFilters).size();
        } else {
            m_speed = 0;
            return 0;
        }
    };

signals:
    void finish(int id); // сигнал о готовности воркера к работе
    void log(QString message, int ctx); // логгирование
};

#endif // CHECKWORKER_H
