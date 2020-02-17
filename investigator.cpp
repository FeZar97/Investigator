#include "investigator.h"

void moveFiles(QString sourceDir, QString destinationDir, int limit) {

    if(!QDir(sourceDir).exists() || !QDir(destinationDir).exists())
        return;

    QFileInfoList filesInSourceDir = QDir(sourceDir + "/").entryInfoList(usingFilters),
                  filesInDestinationDir = QDir(destinationDir + "/").entryInfoList(usingFilters);

    QDateTime startMoveTime = QDateTime::currentDateTime();

    limit = qMin(limit, filesInSourceDir.size());
    if(limit > 0) {
        filesInSourceDir = filesInSourceDir.mid(0, limit);
    }

    while(!filesInSourceDir.isEmpty()) {
        foreach(QFileInfo fileInfo, filesInSourceDir) {
            QFileInfo oldFileInfo(fileInfo);
            if(QFile(fileInfo.absoluteFilePath()).exists()) {
                if(QFile::rename(fileInfo.absoluteFilePath(), destinationDir + "/" + fileInfo.fileName())) {
                    filesInSourceDir.removeAll(oldFileInfo);
                }
            }
        }

        if(startMoveTime.msecsTo(QDateTime::currentDateTime()) > qMin(filesInSourceDir.size() * 3000, 60 * 1000) && filesInSourceDir.size()) {
            break;
        }
    }
}

void moveFile(QString fileName, QString sourceDir, QString destinationDir) {
    if(!QFile(sourceDir + "/" + fileName).exists()
            || !QDir(sourceDir).exists()
            || !QDir(destinationDir).exists())
        return;

    QDateTime startMoveTime = QDateTime::currentDateTime();

    while((QDir(sourceDir).entryList()).contains(fileName)) {
        QFile::rename(sourceDir + "/" + fileName, destinationDir + "/" + fileName);

        if(startMoveTime.msecsTo(QDateTime::currentDateTime()) >  10 * 1000 || QFile(destinationDir + fileName).exists()) {
            break;
        }
    }
}

QString currentDateTime() {
    return QDateTime::currentDateTime().toString(dateTimePattern);
}

double dirSizeMb(QString dirName) {
    double volMb{0};
    foreach(QFileInfo fileInfo, QDir(dirName).entryInfoList(usingFilters)) {
        volMb += fileInfo.size();
    }
    volMb = (volMb * 8 / 1024) / 1024;
    return volMb;
}

Investigator::Investigator(QObject *parent) : QObject(parent){
    qRegisterMetaType<MSG_CATEGORY>("MSG_CATEGORY");
    syslogSocket = new QUdpSocket(this);
}

bool Investigator::checkSyslogAddress() {

    QStringList parts = m_syslogAddress.split(":");

    if(parts.size() != 2) return false;

    syslogAddress = QHostAddress(parts[0]);
    syslogPort = quint16(parts[1].toInt());

    if(parts[0].isNull())
        return false;
    if(syslogAddress.toString() + ":" + QString::number(syslogPort) == m_syslogAddress)
        return true;
    else
        return false;
}

bool Investigator::checkAvParams() {

    bool problem = false;

    // m_watchDir
    if(m_watchDir.isEmpty()) {
        log("Не выбран каталог для слежения!", MSG_CATEGORY(INFO + LOG_GUI));
        problem = true;
    }
    if(!QDir(m_watchDir).exists()) {
        log("Выбранный каталог для слежения не существует!", MSG_CATEGORY(INFO + LOG_GUI));
        problem = true;
    }

    // m_investigatorDir
    if(m_investigatorDir.isEmpty()) {
        log("Не выбран каталог для временных файлов программы!", MSG_CATEGORY(INFO + LOG_GUI));
        problem = true;
    }
    if(!QDir(m_investigatorDir).exists()) {
        log("Выбранный каталог для временных файлов программы не существует!", MSG_CATEGORY(INFO + LOG_GUI));
        problem = true;
    }

    // m_cleanDir
    if(m_cleanDir.isEmpty()) {
        log("Не выбран каталог для чистых файлов!", MSG_CATEGORY(INFO + LOG_GUI));
        problem = true;
    }
    if(!QDir(m_cleanDir).exists()) {
        log("Выбранный каталог для чистых файлов не существует!", MSG_CATEGORY(INFO + LOG_GUI));
        problem = true;
    }

    // m_dangerDir
    if(m_dangerDir.isEmpty()) {
        log("Не выбран каталог для зараженных файлов!", MSG_CATEGORY(INFO + LOG_GUI));
        problem = true;
    }
    if(!QDir(m_dangerDir).exists()) {
        log("Выбранный каталог для зараженных файлов не существует!", MSG_CATEGORY(INFO + LOG_GUI));
        problem = true;
    }

    // m_avPath
    if(m_avPath.isEmpty()) {
        log("Не выбран файл АВС!", MSG_CATEGORY(INFO + LOG_GUI));
        problem = true;
    }
    if(!QFile(m_avPath).exists()) {
        log("Выбранный файл АВС не существует!", MSG_CATEGORY(INFO + LOG_GUI));
        problem = true;
    }

    // m_isWorking
    if(m_isWorking) {
        log("Слежение уже начато!", MSG_CATEGORY(INFO + LOG_GUI));
        problem = true;
    }

    // m_isInProcess
    if(m_isInProcess) {
        log("Процесс АВС уже запущен!", MSG_CATEGORY(INFO + LOG_GUI));
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

void Investigator::clearStatistic() {
    m_startTime = QDateTime::currentDateTime();
    m_endTime = QDateTime::currentDateTime();

    m_infectedFilesNb = 0;
    m_inProgressFilesNb = 0;
    m_processedFilesNb = 0;
    m_averageProcessSpeed = 0;
    m_processedFilesSizeMb = 0;
    m_processedLastFilesSizeMb = 0;
    m_currentProcessSpeed = 0;
}

void Investigator::configureDirs() {
    m_inputDir   = m_investigatorDir + "/" + INPUT_DIR_NAME;
    m_processDir = m_investigatorDir + "/" + OUTPUT_DIR_NAME;
    m_reportsDir = m_investigatorDir + "/" + "reports";

    QDir().mkpath(m_inputDir);
    QDir().mkpath(m_processDir);
    QDir().mkpath(m_reportsDir);
}

void Investigator::onProcessFinished() {

    int filesNumber = QDir(m_inputDir).entryInfoList(usingFilters).size();

    // если слежение запущено, процесс не занят и есть что переносить
    if(m_isWorking && !m_isInProcess && filesNumber) {

        m_isInProcess = true;

        log(QString("Перенос %1 файлов из inputDir(%2) в processDir(%3) с ограничением %4 файлов.").arg(filesNumber)
                                                                                                  .arg(m_inputDir)
                                                                                                  .arg(m_processDir)
                                                                                                  .arg(m_maxQueueSize), MSG_CATEGORY(INFO + LOG_ROW));
        m_inProcessFileList.clear();
        m_inProcessFileList = QDir(m_inputDir).entryList(usingFilters);

        if(m_inProcessFileList.size()) {
            log(QString("Проверяемые файлы: %1").arg(entryListToString(m_inProcessFileList)), MSG_CATEGORY(INFO));
        }

        moveFiles(m_inputDir, m_processDir, m_maxQueueSize);

        // если каталог с файлами не пуст
        if(!QDir(m_processDir).isEmpty()) {
            log("Запуск проверки...", MSG_CATEGORY(INFO + LOG_ROW));
            emit process(QDir::toNativeSeparators(m_avPath), QStringList() << QDir::toNativeSeparators(m_processDir));

        }
    }

    collectStatistics();
}

void Investigator::collectStatistics() {
    m_inQueueFilesNb = QDir(m_inputDir).entryList(usingFilters).size();
    m_inProgressFilesNb = QDir(m_processDir).entryList(usingFilters).size();

    emit updateUi();
}

bool Investigator::beginWork() {

    if(checkAvParams()) {
        clearStatistic();
        log(QString("Запущено слежение за каталогом %1.").arg(m_watchDir), MSG_CATEGORY(INFO + LOG_GUI));
        m_isWorking = true;
        return true;
    } else {
        log(QString("Не удалось начать слежение за каталогом %1.").arg(m_watchDir), MSG_CATEGORY(INFO + LOG_GUI));
        return false;
    }
}

void Investigator::stopWork() {
    m_isWorking = false;
    m_endTime = QDateTime::currentDateTime();

    collectStatistics();

    log(QString("Работа программы приостановлена."), MSG_CATEGORY(INFO + LOG_GUI));

    emit stopProcess();
}

void Investigator::parseReport(QString report) {
    m_lastReport = report;

    if(m_lastReport.contains("Сканирование объектов: ") && m_lastReport.contains("Сканирование завершено")) {
        emit saveReport(QString(m_lastReport), m_reportCnt++);

        QStringList infectedFilesList, tempSplitList, reportLines;
        QString tempFileName;

        reportLines = m_lastReport.split("\n");

        if(reportLines.size() > 5) {
            QString baseVersion       = reportLines[2].remove("Версия баз: ");
            QString m52coreVersion    = reportLines[3].remove("Версия ядра M-52: ");
            QString drwebCoreVersion  = reportLines[4].remove("Версия ядра Dr.Web: ");
            QString kasperCoreVersion = reportLines[5].remove("Версия ядра Касперский: ");
            QString newVersion = QString("Ядро M-52: %1\nЯдро Dr.Web: %2\nЯдро Kaspersky: %3").arg(m52coreVersion).arg(drwebCoreVersion).arg(kasperCoreVersion);
            if(m_avVersion != newVersion) {
                m_avVersion = newVersion;
                log(QString("Определена версия АВС: %1").arg(newVersion), INFO);
            }
        }

        // accumulate statistic
        if(m_isWorking) {
            double lastProcessedFilesSizeMb = 0.;
            m_processedFilesNb += QDir(m_processDir).entryList(usingFilters).size();
            foreach(QFileInfo fi, QDir(m_processDir).entryInfoList(usingFilters)) {
                lastProcessedFilesSizeMb += double(fi.size()) / 1024. / 1024.;
            }
            m_processedFilesSizeMb += lastProcessedFilesSizeMb;
            double workTimeInSecs = double(m_startTime.msecsTo(getEndTime())) / 1000.;
            m_averageProcessSpeed = (workTimeInSecs > 0.01) ? (m_processedFilesSizeMb / workTimeInSecs) : 0;
            m_currentProcessSpeed = (workTimeInSecs > 0.01) ? (lastProcessedFilesSizeMb / workTimeInSecs) : 0;
            emit updateUi();
        }

        if(reportLines.size() > 13) {
            for(int i = 6; i < reportLines.size() - 7; i++) {
                if(reportLines[i].contains(QDir::toNativeSeparators(m_processDir))) {
                    tempSplitList = reportLines[i].split(QDir::toNativeSeparators(m_processDir) + "\\");
                    tempSplitList = tempSplitList[1].split("'");

                    if(tempSplitList[0].contains("//")) {
                        tempFileName = tempSplitList[0].split("//")[0];
                    } else {
                        tempFileName = tempSplitList[0];
                    }

                    if(!infectedFilesList.contains(tempFileName)) {
                        infectedFilesList.push_back(tempFileName);
                    }
                }
            }
        }

        m_infectedFilesNb += infectedFilesList.size();

        // обработка зараженных
        foreach(QString infectedFile, infectedFilesList) {

            log(QString("Найден зараженный файл: %1.").arg(infectedFile), MSG_CATEGORY(CRITICAL + LOG_GUI));
            sendSyslogMessage(QString("Detected infected file: %1").arg(infectedFile), SYS_CRITICAL, SYS_USER);

            switch(m_infectedFileAction) {

                case MOVE_TO_DIR:
                    log(QString("Перенос файла %1 в каталог для зараженных файлов(%2).").arg(infectedFile).arg(m_dangerDir), MSG_CATEGORY(INFO));
                    moveFile(infectedFile, m_processDir, m_dangerDir);
                    break;

                case DELETE:
                    if(!QFile(m_processDir + "/" + infectedFile).remove())
                        log(QString("Не удалось удалить зараженный файл %1.").arg(infectedFile), MSG_CATEGORY(CRITICAL + LOG_GUI));
                    else
                        log(QString("Файл %1 удален.").arg(infectedFile), MSG_CATEGORY(INFO));
                    break;
            }

            if(m_useExternalHandler) {
                log(QString("Внешняя обработка зараженного файла %1.").arg(m_dangerDir + "/" + infectedFile), MSG_CATEGORY(INFO));
            }
        }

        // перенос чистых
        QStringList clearFiles = QDir(m_processDir).entryList(usingFilters);
        log(QString("Перенос %1 проверенных файлов в каталог для чистых файлов(%2): %3").arg(QDir(m_processDir).entryList(usingFilters).size())
                                                                                        .arg(m_cleanDir)
                                                                                        .arg(entryListToString(clearFiles)), MSG_CATEGORY(INFO));
        // перенос только тех, которые отправлялись на проверку
        foreach(QString fileName, clearFiles) {
            if(m_inProcessFileList.contains(fileName)) {
                moveFile(fileName, m_processDir, m_cleanDir);
            }
        }

        // все оставшиеся файлы возвращаются обратно в m_inputDir
        moveFiles(m_processDir, m_cleanDir, ALL_FILES);

        collectStatistics();

        log(QString("Время работы: %1, проверено %2 файлов объемом %3 Мб, найдено зараженных файлов: %4").arg(m_workTime)
                                                                                                         .arg(m_processedFilesNb)
                                                                                                         .arg(m_processedFilesSizeMb)
                                                                                                         .arg(m_infectedFilesNb), MSG_CATEGORY(INFO));
        sendSyslogMessage(QString("Work time: %1, cheked %2 file with total volume: %3 Mb, detected %4 infected files.").arg(m_workTimeEn)
                                                                                                                        .arg(m_processedFilesNb)
                                                                                                                        .arg(m_processedFilesSizeMb)
                                                                                                                        .arg(m_infectedFilesNb), SYS_INFO, SYS_USER);
    } else {
        log(QString("Ошибка разбора отчета: отчет поврежден. Файлы будут перепроверены."), MSG_CATEGORY(INFO));
        moveFiles(m_processDir, m_inputDir, ALL_FILES);
    }

    m_isInProcess = false;

    onProcessFinished();
}

void Investigator::sendSyslogMessage(QString msg, SYSLOG_PRIORITIES pri, SYSLOG_FACILITIES fac) {
    if(m_useSyslog && syslogSocket && checkSyslogAddress()) {
        msg.prepend(QString("<%1>%2 %3 ").arg(fac * 8 + pri).arg(QNetworkInterface::allAddresses().first().toString()).arg(currentDateTime()));
        QByteArray m_msg = msg.toUtf8();
        syslogSocket->writeDatagram(m_msg, m_msg.size(), QHostAddress(syslogAddress), 514);
        log(QString("Отправка сообщения(%1) размером %2 байт на адрес SysLog: %3").arg(QString::fromUtf8(m_msg))
                                                                              .arg(m_msg.size())
                                                                              .arg(QHostAddress(syslogAddress).toString()), MSG_CATEGORY(INFO));
    }
}

QString entryListToString(QStringList &list) {
    QString res = "";
    if(list.size()) {
        for(int i = 0; i < list.size() - 1; i++) {
            res += list[i] + ", ";
        }
        res += list[list.size() - 1] + ".";
    }
    return res;
}
