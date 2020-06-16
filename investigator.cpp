#include "investigator.h"

Investigator::Investigator(QObject *parent) : QObject(parent),
    m_win1251Codec(QTextCodec::codecForName("Windows-1251")) {
    qRegisterMetaType<LOG_CATEGORY>("LOG_CATEGORY");
    m_syslogSocket = new QUdpSocket(this);
}

bool Investigator::checkSyslogAddress() {

    QStringList parts = m_syslogAddress.split(":");

    if(parts.size() != 2) return false;

    m_syslogIpAddress = QHostAddress(parts[0]);
    m_syslogPort = quint16(parts[1].toInt());

    if(parts[0].isNull())
        return false;
    if(m_syslogIpAddress.toString() + ":" + QString::number(m_syslogPort) == m_syslogAddress)
        return true;
    else
        return false;
}

bool Investigator::checkHttpAddress() {
    QStringList parts = m_httpServerAddress.split(":");

    if(parts.size() != 2) return false;

    m_httpServerIp = parts[0].isEmpty() ? QHostAddress::Any : QHostAddress(parts[0]);
    m_httpServerPort = quint16(parts[1].toInt());

    if(parts[0].isNull())
        return false;

    if(m_httpServerIp.toString() + ":" + QString::number(m_httpServerPort) == m_httpServerAddress)
        return true;
    else
        return false;
}

bool Investigator::checkAvParams() {

    bool problem = false;

    if(m_watchDir.isEmpty() || !QDir(m_watchDir).exists()) {
        log(LOG_CATEGORY(DEBUG + GUI), "Не выбрана директория для наблюдения!");
        problem = true;
    }

    if(m_investigatorDir.isEmpty() || !QDir(m_investigatorDir).exists()) {
        log(LOG_CATEGORY(DEBUG + GUI), "Не выбрана директория для временных файлов!");
        problem = true;
    }

    if(m_cleanDir.isEmpty() || !QDir(m_cleanDir).exists()) {
        log(LOG_CATEGORY(DEBUG + GUI), "Не выбрана директория для чистых файлов!");
        problem = true;
    }

    if(m_dangerDir.isEmpty() || !QDir(m_dangerDir).exists()) {
        log(LOG_CATEGORY(DEBUG + GUI), "Не выбрана директория для зараженных файлов!");
        problem = true;
    }

    if(m_avPath.isEmpty() || !QFile(m_avPath).exists()) {
        log(LOG_CATEGORY(DEBUG + GUI), "Не выбран исполняемый файл АВС!");
        problem = true;
    }

    return !problem;
}

QDateTime Investigator::getEndTime() {
    if(m_isWorking) {
        m_endTime = QDateTime::currentDateTime();
    }
    return m_endTime;
}

void Investigator::clearStatistic(bool force) {
    m_endTime = QDateTime::currentDateTime();

    if(force) {
        m_startTime = QDateTime::currentDateTime();

        m_infectedFilesNb = 0;
        m_inProgressFilesNb = 0;
        m_processedFilesNb = 0;
        m_averageProcessSpeed = 0;
        m_processedFilesSizeMb = 0;
        m_processedLastFilesSizeMb = 0;
        m_currentProcessSpeed = 0;
        m_scanningErrorFilesNb = 0;
        m_infectionsMapVector[0].clear();
        m_infectionsMapVector[1].clear();
    }
}

void Investigator::configureDirs() {
    if(m_investigatorDir.isEmpty() || !QDir(m_investigatorDir).exists()) {
        log(LOG_CATEGORY(DEBUG + GUI),
            QString("Не удалось создать временные подкаталоги, выберите существующую директорию для времнных файлов!."));
    } else {
        if(m_inputDir.isEmpty()) {
            m_inputDir   = m_investigatorDir + "/" + INPUT_DIR_NAME;
            QDir().mkpath(m_inputDir);
        }

        if(m_processDir.isEmpty()) {
            m_processDir = m_investigatorDir + "/" + OUTPUT_DIR_NAME;
            QDir().mkpath(m_processDir);
        }

        if(m_cleanDir.isEmpty()) {
            m_cleanDir   = m_investigatorDir + "/" + CLEAN_DIR_NAME;
            QDir().mkpath(m_cleanDir);
        }

        if(m_dangerDir.isEmpty()) {
            m_dangerDir  = m_investigatorDir + "/" + DANGER_DIR_NAME;
            QDir().mkpath(m_dangerDir);
        }

        if(m_logsDir.isEmpty()) {
            m_logsDir    = m_investigatorDir + "/" + LOGS_DIR_NAME;
            QDir().mkpath(m_logsDir);
        }

        if(m_reportsDir.isEmpty()) {
            m_reportsDir = m_investigatorDir + "/" + REPORTS_DIR_NAME;
            QDir().mkpath(m_reportsDir);
        }
    }
}

void Investigator::onProcessFinished() {

    int inputDirFilesNumber = QDir(m_inputDir).entryInfoList(usingFilters).size();

    // если слежение запущено, процесс не занят и есть что переносить
    if(m_isWorking &&
       !m_isInProcess &&
       inputDirFilesNumber) {

        // надо закрыть возможность переноса из inputDir в processDir на время проверки
        m_isInProcess = true;

        // запоминаем файлы переданные на проверку чтобы потом корректно обработать результат парсинга
        // log(LOG_CATEGORY(DEBUG + DEBUG_ROW), QString("Сбор файлов для проверки..."));
        m_inProcessFileList.clear();
        QStringList currentQueue = QDir(m_inputDir).entryList(usingFilters);
        QString currentFile;
        // счетчик увеличивается только в том лучае, если файл действительно перемещен в директорию для проверки
        for(int i = 0; i < qMin(m_maxQueueSize, currentQueue.size()); ) {
            currentFile = currentQueue[i];

            if(moveFile(QFile(currentFile).fileName(), m_inputDir, m_processDir)) {
                i++;
                m_inProcessFileList.append(currentFile);
            }
        }
        log(LOG_CATEGORY(DEBUG + DEBUG_ROW), QString("Файлы подготовлены..."));

        // если каталог с файлами существует
        emit startProcess(QDir::toNativeSeparators(m_avPath),
                          QStringList() << QDir::toNativeSeparators(m_processDir));
    }
}

/* обновление статистики */
void Investigator::collectStatistics() {

    // количество файлов в директории мониторинга
    m_inWatchFilesNb = QDir(m_watchDir).entryList(usingFilters).size();

    // количество файлов в временной директории
    m_inQueueFilesNb = QDir(m_inputDir).entryList(usingFilters).size();

    // количество файлов на проверке
    m_inProgressFilesNb = QDir(m_processDir).entryList(usingFilters).size();

    // объем файлов в временной директории проверки
    m_inQueueFileSizeMb = 0;
    /// ***
    // foreach(QFileInfo fi, QDir(m_processDir).entryInfoList(usingFilters)) {
    //     m_inQueueFileSizeMb += double(fi.size()) / 1024. / 1024.;
    // }
}

void Investigator::parseReport() {

    log(LOG_CATEGORY(DEBUG + DEBUG_ROW), QString("Парсинг отчета АВС..."));

    // если включено сохранение отчетов АВС
    if(m_saveAvsReports)
        emit saveReport(QString(m_lastReport), "autosave");

    // время сканирования
    m_lastProcessWorkTimeInMsec = m_lastProcessStartTime.msecsTo(QDateTime::currentDateTime());

    // разделение отчета по строкам
    m_reportLines = m_lastReport.split("\n");

// --------- AVS VERSIONS ---------
    extractAVSVersions();
    // log(LOG_CATEGORY(DEBUG), QString("Парсинг сведений об АВС выполнен"));

// --------- STATISTIC ---------
    collectStatisticAboutLastScan();
    // log(LOG_CATEGORY(DEBUG), QString("Статистика обновлена"));

// --------- INFECTED ---------
    processingInfectedFiles();
    // log(LOG_CATEGORY(DEBUG), QString("Инфицированные файлы обработаны"));

// --------- PASSWORD PROTECTED ---------
    processingPasswordProtected();
    // log(LOG_CATEGORY(DEBUG), QString("Запароленные файлы обработаны"));

// --------- SCANNING ERROR ---------
    processingScanError();
    // log(LOG_CATEGORY(DEBUG), QString("Ошибки сканирования обработаны"));

// --------- PROCESSING CLEAN ---------
    processingCleanFiles();
    // log(LOG_CATEGORY(DEBUG), QString("Чистые файлы обработаны"));

// --------- PROCESSING OTHER FILES ---------
    processingOtherFiles();
    // log(LOG_CATEGORY(DEBUG), QString("Прочие файлы обработаны"));

    /* ЕСЛИ ОТЧЕТ ПОВРЕЖДЕН

    // если на сканировании был 1 файл и не удалось его проверить
    if(m_maxQueueSize == 1) {
        QStringList problemFile = QDir(m_processDir).entryList(usingFilters);

        if(problemFile.size() != 1) {
            log(LOG_CATEGORY(DEBUG + GUI), "Ошибка в блоке обработки поврежденных файлов");
        } else {
            log(LOG_CATEGORY(DEBUG + GUI),
                QString("Ошибка обработки файла %1. Перенос в каталог для инфицированных файлов...")
                        .arg(entryListToString(problemFile)));

            emit saveReport(m_lastReport, "scanError(Inf)");

            m_scanningErrorFilesNb++;

            moveFile(QFile(problemFile[0]).fileName(), m_processDir, m_dangerDir);
        }
    } else {
    // если на сканировании было > 2 файлов
        log(LOG_CATEGORY(DEBUG + GUI),
            QString("Ошибка парсинга, отчет АВС не сформирован корректно. Файлы перепроверятся. Размер очереди изменен с %1 до %2 файлов.")
                    .arg(m_maxQueueSize)
                    .arg(m_maxQueueSize/2));
        emit saveReport(m_lastReport, "parseError");

        m_maxQueueSize /= 2;
        moveFiles(m_processDir, m_inputDir, ALL_FILES);
    }
    */

    log(LOG_CATEGORY(DEBUG_ROW + DEBUG), "Парсинг завершен");

    m_isInProcess = false;
    onProcessFinished();
}

void Investigator::sendSyslogMessage(QString msg, int pri) {
    if(m_useSyslog && m_syslogSocket && checkSyslogAddress()) {
        msg.prepend(QString("<%1>%2 %3 ").arg(pri).arg(QNetworkInterface::allAddresses().first().toString()).arg(formattedCurrentDateTime()));
        QByteArray m_msg = msg.toUtf8();
        m_syslogSocket->writeDatagram(m_msg, m_msg.size(), QHostAddress(m_syslogIpAddress), 514);
    }
}

void Investigator::clearTemps() {

    m_lastReport.clear();

    m_tempSplitList1.clear();
    m_tempSplitList2.clear();
    m_reportLines.clear();

    m_tempInfectedFileName.clear();
    m_tempVirusInfo.clear();

    m_tempScanningErrorsNb = 0;
    m_tempTotalScanningErrorsNb = 0;

    m_lastProcessedFilesSizeMb = 0.;

    m_infectedFiles.clear();
}

QString Investigator::getReportFileName(QString baseName) {
    return QString("%1report_%2_%3_(%4).txt")
            .arg(m_reportsDir + "/")
            .arg(baseName)
            .arg(QString::number(m_reportCnt++))
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss"));
}

QString Investigator::getCurrentStatistic() {
    return QString("Время работы: %1, проверено %2 объемом: %3, обнаружено %4 зараженных, очередь: %5.\n")
                                              .arg(m_workTimeEn)
                                              .arg(m_processedFilesNb)
                                              .arg(volumeToString(m_processedFilesSizeMb))
                                              .arg(m_infectedFilesNb)
                                              .arg(m_inQueueFilesNb);
}

QString Investigator::getWorkTime() {
    return m_workTimeEn;
}

int Investigator::getInfectedFilesNb() {
    return m_infectedFilesNb;
}

int Investigator::getProcessedFilesNb() {
    return m_processedFilesNb;
}

long long Investigator::getProcessedFilesSizeMb() {
    return m_processedFilesSizeMb;
}

void Investigator::investigatorMoveFiles(QString sourceDir, QString destinationDir, int limit) {
    moveFiles(sourceDir, destinationDir, limit);
}

void Investigator::restartLastProcess() {
    emit startProcess(QDir::toNativeSeparators(m_avPath),
                 QStringList() << QDir::toNativeSeparators(m_processDir));
}

void Investigator::extractAVSVersions() {
    if(m_reportLines.size() > 5) {

        QString kasperNewVersion = m_reportLines[5].remove("Версия ядра Касперский: "),
                drwebNewVersion  = m_reportLines[4].remove("Версия ядра Dr.Web: ");

        m_baseVersion       = m_reportLines[2].remove("Версия баз: ");
        m_m52coreVersion    = m_reportLines[3].remove("Версия ядра M-52: ");

        m_baseVersion.chop(1);
        m_m52coreVersion.chop(1);

        drwebNewVersion.truncate(drwebNewVersion.lastIndexOf(" количество записей"));
        drwebNewVersion.replace(drwebNewVersion.lastIndexOf(","), 1, ")");
        drwebNewVersion.replace("база ", "");

        kasperNewVersion.truncate(kasperNewVersion.lastIndexOf(" количество записей"));
        kasperNewVersion.replace(kasperNewVersion.lastIndexOf(","), 1, ")");
        kasperNewVersion.replace("база ", "");

        if(m_drwebCoreVersion != drwebNewVersion) {
            log(LOG_CATEGORY(GUI), QString("Найденная версия баз Dr.Web: %1.").arg(drwebNewVersion));
            m_drwebCoreVersion = drwebNewVersion;
        }

        if(m_kasperCoreVersion != kasperNewVersion) {
            log(LOG_CATEGORY(GUI), QString("Найденная версия баз Kaspersky: %1.").arg(kasperNewVersion));
            m_kasperCoreVersion = kasperNewVersion;
        }

        QString newVersion = QString("Версия баз: %1\nЯдро M-52: %2\nЯдро Dr.Web: %3\nЯдро Kaspersky: %4").arg(m_baseVersion).arg(m_m52coreVersion).arg(m_drwebCoreVersion).arg(m_kasperCoreVersion);

        if(m_avVersion != newVersion) {
            m_avVersion = newVersion;
        }
    }
}

void Investigator::collectStatisticAboutLastScan() {
    if(m_isWorking) {
        m_processedFilesNb += QDir(m_processDir).entryList(usingFilters).size();
        foreach(QFileInfo fi, QDir(m_processDir).entryInfoList(usingFilters)) {
            m_lastProcessedFilesSizeMb += double(fi.size()) / 1024. / 1024.;
        }
        m_processedFilesSizeMb += m_lastProcessedFilesSizeMb;

        m_currentProcessSpeed = m_lastProcessWorkTimeInMsec ? (m_lastProcessedFilesSizeMb * 1000 / (double(m_lastProcessWorkTimeInMsec))) : 0;
        if(m_currentProcessSpeeds.size() < SPEEDS_VECTOR_SIZE) {
            m_currentProcessSpeeds.push_back(m_currentProcessSpeed);
        } else {
            m_currentProcessSpeeds[m_lastCurrentProcessSpeedIdx++] = m_currentProcessSpeed;
            m_lastCurrentProcessSpeedIdx %= SPEEDS_VECTOR_SIZE;
        }

        m_averageProcessSpeed = [=](){
            double tmpSum = 0;
            for(auto cps: m_currentProcessSpeeds) {
                tmpSum += cps;
            }
            return tmpSum / double(m_currentProcessSpeeds.size());
        }();

        emit updateUi();
    }
}

// поиск и обработка зараженных
void Investigator::processingInfectedFiles() {

    // поиск зараженных файлов в отчете АВС
    for(int i = 0; i < m_reportLines.size(); i++) {

        // если часть строки репорта содержит путь к папке проверки, то в этой строке инфа о зараженном файле
        if(m_reportLines[i].contains(QDir::toNativeSeparators(m_processDir)) &&
                m_reportLines[i].contains("M-52:")) {

            // разделение на подстроки по директории для сканирования
            m_tempSplitList1 = m_reportLines[i].split(QDir::toNativeSeparators(m_processDir) + "\\"); // разделитель - путь к папке с файлами

            // если после деления есть подстроки
            if(m_tempSplitList1.size() > 1) {
                m_tempSplitList1 = m_tempSplitList1[1].split("'");

                // выделение имени инфицированного файла и избавление от вложенных архивов
                if(m_tempSplitList1[0].contains("//")) {
                    m_tempSplitList2 = m_tempSplitList1[0].split("//");
                    if(m_tempSplitList2.size()) {
                        m_tempInfectedFileName = m_tempSplitList2[0];
                    }
                } else {
                    m_tempInfectedFileName = m_tempSplitList1[0];
                }

                // извлечение информации о вирусе
                m_tempSplitList1 = m_reportLines[i].split("инфицирован ");
                if(m_tempSplitList1.size() > 1) {
                    m_tempVirusInfo = m_tempSplitList1[1];

                    m_tempVirusInfo.remove(" - Файл пропущен");
                    m_tempVirusInfo.truncate(m_tempVirusInfo.lastIndexOf(")") + 1);

                    m_tempVirusInfo.remove(0,1); // remove first '('
                    m_tempVirusInfo.chop(1); // remove last ')'

                    // в 0 строка для DrWeb, в 1 для Kaspersky
                    QStringList twoAVSinformation = m_tempVirusInfo.split(";");

                    QStringList drwebDetectedVirusesList, kasperDetectedVirusesList, tempList;

                    log(GUI, m_tempVirusInfo);

                    // если в начале DrWeb
                    if(twoAVSinformation[0].startsWith("DrWeb: ")) {

                        // вирусы, обнаруженные DrWeb
                        QString drwebDetectedViruses = twoAVSinformation[0].remove("DrWeb: ");
                        drwebDetectedViruses.remove(";");
                        // все вирусы с повторениями
                        tempList = drwebDetectedViruses.split(", ");// образование нового списка без повторений

                        for(auto virusName: tempList) {
                            if(!drwebDetectedVirusesList.contains(virusName) && virusName != " ") {
                                drwebDetectedVirusesList.append(virusName);
                            }
                        }

                        // вирусы, обнаруженные Kaspersky
                        QString kasperDetectedViruses = twoAVSinformation[1].remove(" Kaspersky: ");
                        kasperDetectedViruses.remove("; ");
                        // все вирусы с повторениями
                        tempList = kasperDetectedViruses.split(", ");
                        // образование нового списка без повторений
                        for(auto virusName: tempList) {
                            if(!kasperDetectedVirusesList.contains(virusName) && virusName != " ") {
                                kasperDetectedVirusesList.append(virusName);
                            }
                        }
                    } else {
                        // вирусы, обнаруженные Kaspersky
                        QString kasperDetectedViruses = twoAVSinformation[0].remove("Kaspersky: ");
                        kasperDetectedViruses.remove("; ");
                        // все вирусы с повторениями
                        tempList = kasperDetectedViruses.split(", ");
                        // образование нового списка без повторений
                        for(auto virusName: tempList) {
                            if(!kasperDetectedVirusesList.contains(virusName) && virusName != " ") {
                                kasperDetectedVirusesList.append(virusName);
                            }
                        }
                    }

                    // отправка информации в сислог
                    for(auto virusName: kasperDetectedVirusesList) {
                        sendSyslogMessage("(K)" + virusName);
                    }
                    for(auto virusName: drwebDetectedVirusesList) {
                        sendSyslogMessage("(D)" + virusName);
                    }

                    /*
                    // обновление словарика вирусов
                    // обновление Kaspersky
                    for(auto virusName: kasperDetectedVirusesList) {
                        m_infectionsMapVector[0][virusName] = m_infectionsMapVector[0][virusName] + 1;
                    }
                    // обновление DrWeb
                    for(auto virusName: drwebDetectedVirusesList) {
                        m_infectionsMapVector[1][virusName] = m_infectionsMapVector[1][virusName] + 1;
                    }
                    */

                    m_tempVirusInfo = "(";
                    if(drwebDetectedVirusesList.size()) {
                        m_tempVirusInfo += "DrWeb: " + entryListToString(drwebDetectedVirusesList);
                        m_tempVirusInfo.remove(m_tempVirusInfo.size() - 1, 1);
                        m_tempVirusInfo += "; ";
                    }
                    if(kasperDetectedVirusesList.size()) {
                        m_tempVirusInfo += "Kaspersky: " + entryListToString(kasperDetectedVirusesList);
                        m_tempVirusInfo.remove(m_tempVirusInfo.size() - 1, 1);
                        m_tempVirusInfo += "; ";
                    }
                    m_tempVirusInfo.remove(m_tempVirusInfo.size() - 1, 1);
                    m_tempVirusInfo += ")";

                    // в список зараженных файлов файл добавляется только в том случае, если его там еще нет
                    if(!isContainedFile(m_infectedFiles, m_tempInfectedFileName) &&
                       m_tempVirusInfo.length() > 3) { /// *** это зачем хз
                        m_infectedFiles.push_back(QPair<QString,QString>{m_tempInfectedFileName, m_tempVirusInfo});
                    }
                }
            }
        }
    }

    // сохранение отчета об инфицированных файлах
    if(m_infectedFiles.size()) {
        emit saveReport(QString(m_lastReport), "infected");
    }

    // обработка зараженных
    foreach(auto infectedFile, m_infectedFiles) {

        log(LOG_CATEGORY(GUI + DEBUG),
            QString("Зараженный файл: %1 %2.").arg(infectedFile.first).arg(infectedFile.second));

        switch(m_infectedFileAction) {

            case MOVE_TO_DIR:

                if(moveFile(QFile(infectedFile.first).fileName(), m_processDir, m_dangerDir)) {
                    m_infectedFilesNb++;
                }

                if(m_useExternalHandler) {
                    emit startExternalHandler(m_externalHandlerPath,
                                              QStringList()
                                                << QString("'%1'").arg(m_dangerDir + "/" + infectedFile.first)
                                                << QString("'%1'").arg(infectedFile.second)
                                                << QString("'%1'").arg(m_baseVersion));

                    m_infectedFilesNb++;
                }
                break;

            case DELETE:
                if(!QFile(m_processDir + "/" + infectedFile.first).remove()) {
                    log(LOG_CATEGORY(GUI + DEBUG), QString("Не удалось удалить инфицированный файл %1.").arg(infectedFile.first));
                } else {
                    log(LOG_CATEGORY(DEBUG), QString("Файл %1 удален.").arg(infectedFile.first));
                    m_infectedFilesNb++;
                }
                break;
        }
    }
}

// поиск и обработка файлов защищенных паролем
void Investigator::processingPasswordProtected() {
    for(int i = 0; i < m_reportLines.size(); i++) {
        if(m_reportLines[i].contains("M-52: файл") &&
           m_reportLines[i].contains("- Защищен паролем")) {
            m_passwordProtectedFilesNb++;
        }
    }
}

// поиск ошибок сканирования
void Investigator::processingScanError() {
    for(int i = 0; i < m_reportLines.size(); i++) {
        if(m_reportLines[i].contains("M-52: файл") &&
           m_reportLines[i].contains("Ошибка сканирования")) {
            m_scanningErrorFilesNb++;
        }
    }
}

// перенос чистых
void Investigator::processingCleanFiles() {
    // переносятся только те файлы, которые отправлялись на проверку
    foreach(QString fileName, QDir(m_processDir).entryList(usingFilters)) {
        if(m_inProcessFileList.contains(fileName) && // если такой файл отдавался на проверку
          !m_lastReport.contains(QFile(fileName).fileName())) { // и имя файла не фигурирует в отчете
            moveFile(QFile(fileName).fileName(), m_processDir, m_cleanDir);
        }
    }
}

// обработка файлов, оставшхся после предыдущих операций
void Investigator::processingOtherFiles() {

    moveFiles(m_processDir, m_cleanDir, ALL_FILES);

    /*
    // перенос оставшихся в директорию для зараженных
    if(QDir(m_processDir).entryList(usingFilters).size()) {

        // log(LOG_CATEGORY(GUI), QString("В директории обработки остались неучтенные файлы"));
        // emit saveReport(m_lastReport, "processError");
        moveFiles(m_processDir, m_cleanDir, ALL_FILES);

        /// ***
        // if(m_maxQueueSize == 1) {
        //     QStringList problemFile = QDir(m_processDir).entryList(usingFilters);
        //
        //     if(problemFile.size() != 1) {
        //         log(LOG_CATEGORY(DEBUG + GUI), "Ошибка в блоке обработки зависаний АВС.");
        //     } else {
        //         log(LOG_CATEGORY(DEBUG + GUI),
        //             QString("АВС не удается просканировать файл %1. Перенос в каталог для инфицированных файлов...")
        //                     .arg(entryListToString(problemFile)));
        //
        //         emit saveReport(m_lastReport, "processError");
        //
        //         moveFile(QFile(problemFile[0]).fileName(), m_processDir, m_dangerDir);
        //     }
        // }
        // moveFiles(m_processDir, m_inputDir, ALL_FILES);
    }
    */
}
