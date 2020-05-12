#ifndef INVESTIGATOR_H
#define INVESTIGATOR_H

#include <QObject>
#include <QDateTime>
#include <QTimeZone>
#include <QDir>
#include <QProcess>
#include <QThread>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QPalette>
#include <QDebug>
#include <QSettings>
#include <QHostAddress>
#include <QUdpSocket>
#include <QTextCodec>
#include <QNetworkInterface>

#include "stylehelper.h"

#define     MAJOR_VERSION         "1"
#define     MINOR_VERSION         "6"
#define     PATCH_VERSION         "5.6"
#define     VERSION               QString("v%1.%2.%3").arg(MAJOR_VERSION).arg(MINOR_VERSION).arg(PATCH_VERSION)

#define     INPUT_DIR_NAME        "input"
#define     OUTPUT_DIR_NAME       "output"
#define     CLEAN_DIR_NAME        "clean"
#define     DANGER_DIR_NAME       "danger"
#define     LOGS_DIR_NAME         "logs"
#define     REPORTS_DIR_NAME      "reports"

#define     ALL_FILES             -1

enum ACTION_TYPE {
    MOVE_TO_DIR,
    DELETE
};

enum MSG_CATEGORY {
    INFO     = 0x01,
    DEBUG    = 0x02,
    LOG_GUI  = 0x04,
    LOG_ROW  = 0x08
};

const static QString dateTimePattern = "yyyy-MM-dd hh:mm:ss";
const static QDir::Filters usingFilters = QDir::Files | QDir::Hidden;

void moveFiles(QString sourceDir, QString destinationDir, int limit);
void moveFile(QString fileName, QString sourceDir, QString destinationDir);
void clearDir(QString dirName);
QString entryListToString(QStringList &list);
QString currentDateTime();
double dirSizeMb(QString dirName);
bool isContainedFile(QList<QPair<QString, QString>> &fileList, QString fileName);
QString volumeToString(double volumeInMb);

class Investigator : public QObject
{
    Q_OBJECT

public:
    bool m_isWorking{false};
    bool m_isInProcess{false};

    QDateTime m_startTime{QDateTime::currentDateTime()}; // время запуска программы
    QDateTime m_endTime{QDateTime::currentDateTime()}; // время окончания работы (в процессе работы постоянно обновляется)

    QString m_avPath; // путь к исполняемому файлу
    QString m_baseVersion{""}, m_m52coreVersion, m_drwebCoreVersion, m_kasperCoreVersion; // версии баз
    QString m_avVersion{"Не удалось определить версию продукта."}; // все версии

    QString m_watchDir; // каталог за которой следим
    QString m_investigatorDir; // каталог для временных файлов программы

    QString m_inputDir; // куда копируются файлы из m_watchDir
    QString m_processDir; // где проверяются файлы
    QString m_cleanDir; // каталог для читых файлов
    QString m_dangerDir; // каталог для зараженных файлов
    QString m_logsDir; // каталог логов

    QString m_processInfo; // информация для строки в окне статистики
    QString m_lastReport; // последний репорт
    unsigned long long m_reportCnt{0}; // счетчик репортов

    QStringList m_tempSplitList, m_reportLines; // для метода parseReport
    QString m_tempFileName, m_tempVirusInfo; // для метода parseReport
    double m_lastProcessedFilesSizeMb{0}; // объем последней просканированной выборки файлов
    QDateTime m_lastProcessStartTime{QDateTime::currentDateTime()}; // время начала последней проверки
    unsigned int m_lastProcessWorkTimeInSec{0}; // время обработки последней выборки

    QStringList m_inProcessFileList; // файлы в обработке
    QList<QPair<QString,QString>> m_infectedFiles; // зараженные файлы, выявленные в процессе проверки

    ACTION_TYPE m_infectedFileAction{MOVE_TO_DIR}; // действие с зараженными файлами
    bool m_saveAvsReports; // флаг сохранения отчетов АВС
    QString m_reportsDir; // каталог сохранения отчетов АВС

    bool m_useExternalHandler{false}; // флаг использования внещнего обработчика
    QString m_externalHandlerPath{""}; // путь к внешнему обработчику

    bool m_useSyslog{false}; // флаг логгирования в сислог
    QString m_syslogAddress; // адрес syslogd
    MSG_CATEGORY m_syslogPriority; // нижняя граница приоритета сообщений, которые следует отправлять в сислог

    QString m_workTime{""}; // время работы в формате "d дней hh ч. mm мин. ss сек"
    QString m_workTimeEn{""}; // время работы в формате "d days hh h. mm min. ss sec"

    QUdpSocket *m_syslogSocket; // сокет для syslog
    QHostAddress m_syslogIpAddress; // адрес демона
    quint16 m_syslogPort; // используемый порт

    bool m_useHttpServer{true}; // флаг использования http сервера
    QString m_httpServerAddress; // адрес http
    QHostAddress m_httpServerIp; // входной адрес для сервера
    quint16 m_httpServerPort; // порт сервера

    int m_maxQueueSize{10}; // макс число файлов в очереди
    double m_maxQueueVolMb{128.}; // макс объем файлов в очереди в мегабайтах
    int m_maxQueueVolUnit{0}; // единицы измерения объема

    int m_infectedFilesNb{0}; // кол-во найденных зараженных файлов
    int m_inProgressFilesNb{0}; // кол-во файлов в обработке
    int m_processedFilesNb{0}; // кол-во обработаннызх файлов
    double m_processedFilesSizeMb{0.}; // объем уже обработанных файлов
    double m_processedLastFilesSizeMb{0.}; // объем файлов последнего сканирования
    double m_averageProcessSpeed{0.}; // средняя скорость проверки
    double m_currentProcessSpeed{0.}; // текущая скорость проверки
    int m_inQueueFilesNb{0}; // количество файлов в очереди
    double m_inQueueFileSizeMb{0}; // размер файлов в очереди

    explicit Investigator(QObject *parent = nullptr);

    /* проверка корректности url syslogd */
    bool checkSyslogAddress();

    /* проверка корректности url http */
    bool checkHttpAddress();

    /* проверка параметров программы на корректность */
    bool checkAvParams();

    /* получение времени окончания работы
     * в случае, если проверка запущена, возвращается текущее время
     * если проверка остановлена, возвращается время окончания последней проверки */
    QDateTime getEndTime();

    /* очистка статистики */
    void clearStatistic();

    /* настройка путей до временных каталогов в соответствии с выбранным каталогом программы */
    void configureDirs();

    /* слот, вызываемый при окончании очередной проверки */
    void onProcessFinished();

    /* сбор статистики */
    void collectStatistics();

    /* запуск работы */
    bool beginWork();

    /* остановка работы */
    void stopWork();

    /* разбор отчета */
    void parseReport(QString report);

    /* send syslog message */
    void sendSyslogMessage(QString msg = "", int pri = 99);

    /* сброс временных пееременных для метода parseReport */
    void clearParserTemps();

    QString getReportFileName(QString baseName = "");

    QString getWorkTime();
    int getInfectedFilesNb();
    int getProcessedFilesNb();
    long long getProcessedFilesSizeMb();

signals:
    /* эмитится каждый раз при изменении статистики */
    void updateUi();

    /* вызов АВС с аргументами args, имеющего URI programPath */
    void process(QString programPath, QStringList args);

    /* остановка процесса АВС */
    void stopProcess();

    /* вывод информации */
    void log(QString s, MSG_CATEGORY cat);

    void saveReport(QString report = "", QString baseName = "");

    /* вызов внешнего обработчика */
    void startExternalHandler(QString path, QStringList args);
};

#endif // INVESTIGATOR_H
