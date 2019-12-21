#include "distributor.h"
#include "widget.h"

Distributor::Distributor(QObject *parent) : QObject(parent) {
    
    qRegisterMetaType<LOG_DST>("LOG_DST&");

    m_logFile.setFileName("global log " + currentDateTime() + ".txt");
    if(m_logFile.open(QFile::WriteOnly)) {
        m_logStream.setDevice(&m_logFile);
    }

// initial settings
    kasperWrapper.setType(AV::KASPER);
    drwebWrapper.setType(AV::DRWEB);

    kasperWrapper.setReportExtension("kres");
    drwebWrapper.setReportExtension("dres");

    kasperWrapper.setIndicators("; Completion:       	100%",
                                "; ------------------",
                                ";  --- Statistics ---",
                                QStringList() << "detected");

    drwebWrapper.setIndicators("Scan session completed",
                               "The mask was translated to \"\" filter",
                               "WARNING! Restore points directories have not been scanned",
                               QStringList() << "infected" << "read error");

    kasperWrapper.connectProcessingFlag(&m_isProcessing);
    drwebWrapper.connectProcessingFlag(&m_isProcessing);

    connect(&kasperWrapper, &AVWrapper::logWrapper,                 this,           &Distributor::log);
    connect(&drwebWrapper,  &AVWrapper::logWrapper,                 this,           &Distributor::log);

    connect(&watchDirEye,   &QFileSystemWatcher::directoryChanged,  this,           &Distributor::onWatchDirChange);

    configureChain();

    kasperWrapper.moveToThread(&kasperThread);
    drwebWrapper.moveToThread(&drwebThread);

    kasperThread.start();
    drwebThread.start();
}

Distributor::~Distributor() {
    if(m_logFile.isOpen())
        m_logFile.close();

    kasperThread.quit();
    kasperThread.wait();

    drwebThread.quit();
    drwebThread.wait();
}

void Distributor::setWatchDir(QString _watchDir) {

    if(_watchDir.isEmpty()) {
        log("Не найдена директория для слежения.", LOG_GUI);
    } else {
        m_watchDir = _watchDir;
    }
}

QString Distributor::getWatchDir() {
    return m_watchDir;
}

void Distributor::setInvestigatorDir(QString _investigatorDir) {

    if(_investigatorDir.isEmpty()) {
        log("Не найдена директория для временных файлов.", LOG_GUI);
    } else {
        m_investigatorDir = _investigatorDir;

        QDir().mkpath(m_investigatorDir + "/" + KASPER_DIR_NAME + "/" + INPUT_DIR_NAME);
        QDir().mkpath(m_investigatorDir + "/" + KASPER_DIR_NAME + "/" + OUTPUT_DIR_NAME);

        QDir().mkpath(m_investigatorDir + "/" + DRWEB_DIR_NAME + "/" + INPUT_DIR_NAME);
        QDir().mkpath(m_investigatorDir + "/" + DRWEB_DIR_NAME + "/" + OUTPUT_DIR_NAME);

        m_outputDir = m_investigatorDir + "/" + PROCESSED_DIR_NAME;
        QDir().mkpath(m_outputDir);

        m_reportDir = m_investigatorDir + "/" + REPORT_DIR_NAME;
        QDir().mkpath(m_reportDir);

        kasperWrapper.setFolders(m_investigatorDir,
                                 m_investigatorDir + "/" + KASPER_DIR_NAME + "/" + INPUT_DIR_NAME,
                                 m_investigatorDir + "/" + KASPER_DIR_NAME + "/" + OUTPUT_DIR_NAME,
                                 m_reportDir);

        drwebWrapper.setFolders(m_investigatorDir,
                                m_investigatorDir + "/" + DRWEB_DIR_NAME + "/" + INPUT_DIR_NAME,
                                m_investigatorDir + "/" + DRWEB_DIR_NAME + "/" + OUTPUT_DIR_NAME,
                                m_reportDir);

        configureChain();
    }
}

QString Distributor::getInvestigatorDir() {
    return m_investigatorDir;
}

void Distributor::setCleanDir(QString _cleanDir) {
    if(_cleanDir.isEmpty()) {
        log("Не найдена директория для чистых файлов.", LOG_GUI);
    } else {
        m_cleanDir = _cleanDir;
    }
}

QString Distributor::getCleanDir() {
    return m_cleanDir;
}

void Distributor::setDangerDir(QString _dangerDir) {
    if(_dangerDir.isEmpty()) {
        log("Не найдена директория для зараженных файлов.", LOG_GUI);
    } else {
        m_dangerDir = _dangerDir;
        kasperWrapper.setDangerFolder(m_dangerDir);
        drwebWrapper.setDangerFolder(m_dangerDir);
    }
}

QString Distributor::getDangerDir() {
    return m_dangerDir;
}

void Distributor::setAVFile(AV AVName, QString AVFilePath) {

    if(AVFilePath.isEmpty()) {
        log(QString("Не найдена исполняемый файл антивируса %1.").arg(getName(AVName)), LOG_GUI);
        return;
    }

    switch(AVName) {
        case AV::KASPER:
            kasperWrapper.setAVPath(AVFilePath);
            break;

        case AV::DRWEB:
            drwebWrapper.setAVPath(AVFilePath);
            break;

        default:
            break;
    }
}

QString Distributor::getAVFile(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getAVPath();

        case AV::DRWEB:
            return drwebWrapper.getAVPath();

        default:
            return "";
    }
}

void Distributor::setAVUse(AV AVName, bool use) {
    switch(AVName) {
        case AV::KASPER:
            kasperWrapper.setUsage(use);
            break;

        case AV::DRWEB:
            drwebWrapper.setUsage(use);
            break;

        default:
            break;
    }
    configureChain();
}

bool Distributor::getAVUse(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getUsage();

        case AV::DRWEB:
            return drwebWrapper.getUsage();

        default:
            return true;
    }
}

void Distributor::setMaxQueueSize(AV AVName, int size) {
    switch(AVName) {
        case AV::KASPER:
            kasperWrapper.setMaxQueueSize(size);
            break;

        case AV::DRWEB:
            drwebWrapper.setMaxQueueSize(size);
            break;

        default:
            break;
    }
}

int Distributor::getMaxQueueSize(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getMaxQueueSize();

        case AV::DRWEB:
            return drwebWrapper.getMaxQueueSize();

        default:
            return 0;
    }
}

void Distributor::setMaxQueueVol(AV AVName, double vol) {
    switch(AVName) {
        case AV::KASPER:
            kasperWrapper.setMaxQueueVolMb(vol);
            break;

        case AV::DRWEB:
            drwebWrapper.setMaxQueueVolMb(vol);
            break;

        default:
            break;
    }
}

double Distributor::getMaxQueueVolMb(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getMaxQueueVolMb();

        case AV::DRWEB:
            return drwebWrapper.getMaxQueueVolMb();

        default:
            return 0;
    }
}

void Distributor::setMaxQueueVolUnit(AV AVName, int unitIdx) {
    switch(AVName) {
        case AV::KASPER:
            kasperWrapper.setMaxQueueVolUnit(unitIdx);
            break;

        case AV::DRWEB:
            drwebWrapper.setMaxQueueVolUnit(unitIdx);
            break;

        default:
            break;
    }
}

int Distributor::getMaxQueueVolUnit(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getMaxQueueVolUnit();

        case AV::DRWEB:
            return drwebWrapper.getMaxQueueVolUnit();

        default:
            return 0;
    }
}

double Distributor::calcMaxQueueVol(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getMaxQueueVolMb() * (kasperWrapper.getMaxQueueVolUnit() ? 1024 : 1);

        case AV::DRWEB:
            return drwebWrapper.getMaxQueueVolMb() * (drwebWrapper.getMaxQueueVolUnit() ? 1024 : 1);

        default:
            return 0;
    }
}

int Distributor::getAVDangerFilesNb(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getDangerFilesNb();

        case AV::DRWEB:
            return drwebWrapper.getDangerFilesNb();

        default:
            return 0;
    }
}

int Distributor::getAVCurrentReportIdx(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getCurrentReportIdx();

        case AV::DRWEB:
            return drwebWrapper.getCurrentReportIdx();

        default:
            return 0;
    }
}

int Distributor::getAVQueueFilesNb(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return QDir(m_investigatorDir + "/" + KASPER_DIR_NAME + "/" + INPUT_DIR_NAME).entryList(usingFilters).size();

        case AV::DRWEB:
            return QDir(m_investigatorDir + "/" + DRWEB_DIR_NAME + "/" + INPUT_DIR_NAME).entryList(usingFilters).size();

        default:
            return 0;
    }
}

double Distributor::getAVQueueFilesVolMb(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return dirSizeMb(m_investigatorDir + "/" + KASPER_DIR_NAME + "/" + INPUT_DIR_NAME + "/");

        case AV::DRWEB:
            return dirSizeMb(m_investigatorDir + "/" + DRWEB_DIR_NAME + "/" + INPUT_DIR_NAME + "/");

        default:
            return 0;
    }
}

int Distributor::getAVProcessedFilesNb(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getProcessedFilesNb();

        case AV::DRWEB:
            return drwebWrapper.getProcessedFilesNb();

        default:
            return 0;
    }
}

int Distributor::getAVInProgressFilesNb(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getInProgressFilesNb();

        case AV::DRWEB:
            return drwebWrapper.getInProgressFilesNb();

        default:
            return 0;
    }
}

double Distributor::getAVProcessedFilesSize(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getProcessedFilesSize();

        case AV::DRWEB:
            return drwebWrapper.getProcessedFilesSize();

        default:
            return 0;
    }
}

void Distributor::startWatchDirEye() {
    stopWatchDirEye();

    if(!m_watchDir.isEmpty()) {
        if(watchDirEye.addPath(m_watchDir)) {
            if(!kasperWrapper.isReadyToProcess() && kasperWrapper.getUsage()) {
                log("Процесс проверки антивируса Kaspersky уже запущен!", LOG_GUI);
                return;
            }

            if(!drwebWrapper.isReadyToProcess() && drwebWrapper.getUsage()) {
                log("Процесс проверки антивируса DrWeb уже запущен!", LOG_GUI);
                return;
            }

            m_startTime = QDateTime::currentDateTime();
            m_isProcessing = true;

            log(QString("Запущено слежение за директорией %1.").arg(m_watchDir), LOG_GUI);

            onWatchDirChange("");

        } else {
            log(QString("Не удалось начать слежение за папкой %1.").arg(m_watchDir), LOG_GUI);
        }
    }
}

void Distributor::stopWatchDirEye() {

    m_isProcessing = false;

    m_endTime = QDateTime::currentDateTime();

    log(QString("Слежение за директорией %1 остановлено.").arg(m_watchDir), LOG_GUI);

    if(!watchDirEye.directories().isEmpty()) {
        watchDirEye.removePaths(watchDirEye.directories());
    }
}

void Distributor::onWatchDirChange(const QString &path) {
    Q_UNUSED(path)

    if(m_isProcessing) {
        log(QString("Перенос файлов из %1 в %2").arg(m_watchDir).arg(m_inputDir), LOG_DST(LOG_FILE | LOG_ROW));
        moveFiles(m_watchDir, m_inputDir, &m_isProcessing);

        emit startProcess();
    }
}

QDateTime Distributor::getStartTime() const {
    return m_startTime;
}

QDateTime Distributor::getEndTime() {
    if(m_isProcessing)
        m_endTime = QDateTime::currentDateTime();
    return m_endTime;
}

bool Distributor::isInProcessing(){
    return m_isProcessing;
}

void Distributor::clearDir(QString dirName) {

    setProcessInfo("Очистка директории " + dirName);

    foreach(QFileInfo fileInfo, QDir(dirName).entryInfoList(usingFilters)) {
        QFile::remove(fileInfo.absoluteFilePath());
    }
}

double Distributor::dirSizeMb(QString dirName) {
    double volMb{0};
    foreach(QFileInfo fileInfo, QDir(dirName).entryInfoList(usingFilters)) {
        volMb += fileInfo.size();
    }
    volMb = (volMb * 8 / 1024) / 1024;
    return volMb;
}

void Distributor::clearStatistic() {
    m_startTime = QDateTime::currentDateTime();
    m_endTime = QDateTime::currentDateTime();
    kasperWrapper.clearStatistic();
    drwebWrapper.clearStatistic();
}

void Distributor::moveCleanFiles() {
    if(!QDir(m_dangerDir).exists()) QDir().mkpath(m_dangerDir);
    if(!QDir(m_cleanDir).exists())  QDir().mkpath(m_cleanDir);

    foreach(QFileInfo avRecord, QDir(m_outputDir).entryInfoList(usingFilters)) {
        QFile::rename(avRecord.absoluteFilePath(), m_cleanDir + "/" + avRecord.fileName());
    }
}

void Distributor::setProcessInfo(QString info) {
    m_processInfo = info;
}

QString Distributor::getProcessInfo() const {
    return m_processInfo;
}

void Distributor::log(QString text, LOG_DST flags) {

    if(flags & LOG_FILE) {
        m_logStream << currentDateTime() + " " + text + "\r\n";
    }

    if(flags & LOG_ROW) {
        m_processInfo = text;
    }

    if(flags & LOG_GUI) {
        emit logGui(currentDateTime() + " " + text);
    }
}

double Distributor::getAVAverageSpeed(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getAverageSpeed();

        case AV::DRWEB:
            return drwebWrapper.getAverageSpeed();

        default:
            return 0;
    }
}

double Distributor::getAVCurrentSpeed(AV AVName) {
    switch(AVName) {
        case AV::KASPER:
            return kasperWrapper.getCurrentSpeed();

        case AV::DRWEB:
            return drwebWrapper.getCurrentSpeed();

        default:
            return 0;
    }
}

void Distributor::disconnectAll() {
    disconnect(this,           &Distributor::startProcess, nullptr,  nullptr);
    disconnect(&kasperWrapper, &AVWrapper::finishProcess,  nullptr,  nullptr);
    disconnect(&drwebWrapper,  &AVWrapper::finishProcess,  nullptr,  nullptr);
}

void Distributor::configureChain() {

    disconnectAll();

    if(kasperWrapper.getUsage()) {
        if(!kasperWrapper.hasStarvation()) {
        // if kasper hasn`t starvation

            m_inputDir = kasperWrapper.getInputFolder();

            kasperWrapper.setOutputFolder(drwebWrapper.getInputFolder());
            drwebWrapper.setOutputFolder(m_outputDir);

            connect(this,           &Distributor::startProcess,             &kasperWrapper, &AVWrapper::process);

            // when kasper finish, drweb can start
            connect(&kasperWrapper, &AVWrapper::finishProcess,              &drwebWrapper,  &AVWrapper::process);

            // when last AV finished works, need move remaind files to clear folder
            connect(&drwebWrapper,  &AVWrapper::finishProcess,              this,           &Distributor::moveCleanFiles);
        } else {
        // if kasper has starvation

            m_inputDir = drwebWrapper.getInputFolder();

            drwebWrapper.setOutputFolder(m_outputDir);

            connect(this,           &Distributor::startProcess,             &drwebWrapper,  &AVWrapper::process);

            // when last AV finished works, need move remaind files to clear folder
            connect(&drwebWrapper,  &AVWrapper::finishProcess,              this,           &Distributor::moveCleanFiles);
        }
    }
}
