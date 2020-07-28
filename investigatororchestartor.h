#ifndef CHECKORCHESTARTOR_H
#define CHECKORCHESTARTOR_H

#include <QObject>
#include <QVector>
#include <QThread>
#include <QTimer>
#include <QDebug>
#include <QDateTime>
#include <QNetworkInterface>
#include <QUdpSocket>
#include <QHostAddress>

#include "investigatorworker.h"

const int MaxThreadNb{16}; // максимальное количество потоков, на которое можно разделить входную задачу
const int SleepIntervalAfterScanMs = 1000;
const int UpdatePeriodMs = 500;

class InvestigatorOrchestartor : public QObject {
    Q_OBJECT

public:
    explicit InvestigatorOrchestartor(QObject *parent = nullptr);
    ~InvestigatorOrchestartor();

private:
    // статистика общая
    quint64 m_totalProcessedFilesNb{0}; // всего обработано файлов
    quint64 m_totalProcessedFilesSize{0}; // общий обработанный объем
    quint64 m_totalInfectedFilesNb{0}; // всего зараженных файлов
    quint64 m_totalPwdFilesNb{0}; // всего запароленных файлов

    quint64 m_currentSpeed; // суммарная скорость воркеров

    QProcess *m_initialAvsProcess{nullptr}; // процесс для инициализирующего запуска АВС
    bool m_successInitialScan{false}; // флаг успешного инициализирующего запуска АВС
    void createInitialAvsProcess(bool needLaterStart); // создать процесс

    bool m_isInWork{false}; // флаг работы

    QUdpSocket *m_udpSocket{nullptr}; // сокет для syslog
    QString m_syslogAddress; // адрес сислога

    QString m_sourceDir; // входная директория
    QString m_processDir; // временная директория для программы
    QString m_logsDir; // программа для сохранения логов
    QString m_cleanDir; // директория для чистых файлов
    QString m_infectedDir; // директория для зараженных файлов
    QString m_avsExecFileName{QDir().toNativeSeparators("C:/Program Files/Primetech/M-52/AVSFileConsoleScan.exe")}; // путь до исполняемого файла АВС
    QString m_avVersion; // версия АВС
    quint64 m_thresholdFilesNb; // порог по кол-ву файлов
    quint64 m_thresholdFilesSize; // порог по объему файлов
    quint64 m_thresholdFilesSizeUnit; // единицы измерения порога по объему файлов

    QFile m_logFile; // текущий файл логов

    quint8 m_tempCurrentWorkersNb; // временное значение кол-ва воркеров, сохраняемое до очередного цикла обработки
    quint8 m_currentWorkersNb; // используемое кол-во воркеров на данном цикле обработки

    QThread *m_workThreads; // потоки для воркеров
    InvestigatorWorker **m_workers; // воркеры
    void createWorkers(); // создание воркеров, помещение их в собственные потоки и запуск воркеров

    QFileInfoList m_totalFileList; // список файлов на обработку

    QVector<int> m_workTimeV; // [dd/hh/mm/ss]
    QTimer *m_workTimer; // таймер для подсчета времени работы
    void createWorkTimer();

    QTimer *m_updateStatisticTimer; // таймер для обновления статистики
    void createUpdateStatisticTimer();

    QTimer *m_processTimer; // таймер для запуска обработки
    void createProcessTimer();

    void stopWorkers(); // остановка воркеров
    void stopThreads(); // остановка потоков

    bool updateTotalFileList(); // обновление списка файлов на обработку
    void tryCheckFiles(); // запуск проверки

    // получение статистики от воркера
    void onWorkerFinished(int workerId);

    // получение списк ана проверку для воркера
    QFileInfoList getFilesToCheckList();

public:
    // source, process, infectedDir, cleanDir, avsPath, thresholdNb, thresholdSize
    void startWork(); // запуск
    void stopWork(); // полная остановка
    bool reconfigureWorkers(); // обновление настроек воркеров

    bool isInWork() {
        return m_isInWork;
    }
    bool readyToNextProcess(); // флаг готовности к запуску новой обработки

    bool resultOfInitialScan() {
        return m_successInitialScan;    // результат инициализирующего запуска
    }

    void fileLog(QString message); // логирование процесса работы в файлы

    // логгирование
    // ctx == "UI" - вывод сообщения в главное окно программы
    // ctx == "SYSLOG" - вывод сообщения в syslog
    // ctx == "FILE" - вывод сообщения в лог файл программы
    void log(QString message, int logCtx);

// настройки
    QString sourceDir() {
        return m_sourceDir;
    }
    void setSourceDir(QString sourceDir) {
        m_sourceDir = sourceDir;
    }

    QString processDir() {
        return m_processDir;
    }
    void setProcessDir(QString processDir) {
        m_processDir = processDir;
    }

    QString infectedDir() {
        return m_infectedDir;
    }
    void setInfectedDir(QString infectedDir) {
        m_infectedDir = infectedDir;
    }

    QString cleanDir() {
        return m_cleanDir;
    }
    void setCleanDir(QString cleanDir) {
        m_cleanDir = cleanDir;
    }

    int tempCurrentWorkersNb() {
        return m_tempCurrentWorkersNb;
    }
    int currentWorkersNb() {
        return m_currentWorkersNb;
    }
    void setWorkersNb(int workersNb) {
        m_tempCurrentWorkersNb = workersNb;
    }

    qint64 thresholdFilesNb() {
        return m_thresholdFilesNb;
    }
    void setThresholdFilesNb(qint64 thresholdFilesNb) {
        m_thresholdFilesNb = thresholdFilesNb;
    }

    qint64 thresholdFilesSize() {
        return m_thresholdFilesSize;
    }
    void setThresholdFilesSize(qint64 thresholdFilesSize) {
        m_thresholdFilesSize = thresholdFilesSize;
    }

    qint64 thresholdFilesSizeUnit() {
        return m_thresholdFilesSizeUnit;
    }
    void setThresholdFilesSizeUnit(qint64 thresholdFilesSizeUnit) {
        m_thresholdFilesSizeUnit = thresholdFilesSizeUnit;
    }

    QString avsExecFileName() {
        return m_avsExecFileName;
    }
    void setAvsExecFileName(QString avsFileName);

    QString syslogAddress() {
        return m_syslogAddress;
    }
    void setSyslogAddress(QString syslogAddress) {
        m_syslogAddress = syslogAddress;
    }

// статистика
    QString avsVersion() {
        return m_avVersion;
    }

    qint64 currentQueueFilesNb() {
        return m_totalFileList.size();
    }

    // количество файлов, находящихся на проверке в данный момент
    qint64 inProcessFilesNb();

    // статистика воркера с id == workerId
    WorkerStatistic getWorkerStatistic(int workerId);

    // начальное сканирование для определения версии баз
    void getInitialAvsScan(bool needLaterStart = false);

    // установка глобальных счетчиков
    void setTotalProcessedFilesNb(quint64 totalProcessedFilesNb) {
        m_totalProcessedFilesNb = totalProcessedFilesNb;
    }
    void setTotalProcessedFilesSize(quint64 totalProcessedFilesSize) {
        m_totalProcessedFilesSize = totalProcessedFilesSize;
    }
    void setTotalInfectedFilesNb(quint64 totalInfectedFilesNb) {
        m_totalInfectedFilesNb = totalInfectedFilesNb;
    }
    void setTotalPwdFilesNb(quint64 totalPwdFilesNb) {
        m_totalPwdFilesNb = totalPwdFilesNb;
    }

    // сброс таймера работы
    void dumpWorkTimer() {
        m_workTimeV = QVector<int>(4, 0);
    }

    // статистика по очереди
    quint64 queueFilesNb();
    quint64 queueFilesSize();

    // статистика общая
    quint64 totalProcessedFilesNb() {
        return m_totalProcessedFilesNb;
    }
    quint64 totalProcessedFilesSize() {
        return m_totalProcessedFilesSize;
    }
    quint64 totalInfectedFilesNb() {
        return m_totalInfectedFilesNb;
    }
    quint64 totalPwdFilesNb() {
        return m_totalPwdFilesNb;
    }
    quint64 getWorkTimeInSec(); // время работы в секундах

    quint64 currentSpeed(); // скорость работы ОБЩАЯ
    QVector<int> getWorkTimeVector() {
        return m_workTimeV;    // [dd, hh, mm, ss]
    }

    QString workTimeToString(); // время работы в виде строки
    QString workStatisticToString(); // статистика работы в виде строки

    void sendSyslogMessage(QString message); // отправка сообщения в сислог

signals:
    void tryWorkerCheckFiles(int id, QFileInfoList filesList);
    void updateProgress(); // сигнал готовности состояния процесса
    void updateUi(); // обновление интерфейса
    void uiLog(QString msg); // вывод сообщения в главное окно
};

#endif // CHECKORCHESTARTOR_H
