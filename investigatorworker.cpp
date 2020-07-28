#include "investigatorworker.h"

InvestigatorWorker::InvestigatorWorker(QObject *parent): QObject{parent}, m_win1251Codec{QTextCodec::codecForName("Windows-1251")} {
    stopWork();
}

// передача воркеру внешних параметров
// id, (input, process, danger, clean, report), avsExec
void InvestigatorWorker::configure(int id, QStringList dirList, QString avsExecFileName) {
    Q_ASSERT(dirList.size() == 5);

    m_id = id;

    m_inputDir = dirList[0];
    m_processDir = dirList[1];
    m_dangerDir = dirList[2];
    m_cleanDir = dirList[3];
    m_reportDir = dirList[4];

    m_avsExecFileName = avsExecFileName;

    checkProcessDirExists();

    moveFilesDD(m_workerProcessDir, m_inputDir);
}

void InvestigatorWorker::startWork() {
    flushStatistic();
    m_isInWork = true;
}

void InvestigatorWorker::stopWork() {
    flushStatistic();
    m_isInWork = false;
}

// проверка возможности запуска проверки
bool InvestigatorWorker::canAcceptWork(int id) {
    return !m_isInProcess && m_isInWork && m_id == id;
}

// проверка наличия директории для работы
void InvestigatorWorker::checkProcessDirExists() {
    m_workerProcessDir = QDir::toNativeSeparators(m_processDir + QString("/worker%1").arg(m_id));
    if (!QDir(m_workerProcessDir).exists()) {
        QDir().mkpath(m_workerProcessDir);
    }
}

// запуск процесса проверки для воркера с id
void InvestigatorWorker::tryCheckFiles(int id, QFileInfoList filesToProcess) {

    // если воркер не занят, и совпал id
    if (canAcceptWork(id)) {

        // перенос старых файлов из processDir в inputDir
        moveFilesDD(m_workerProcessDir, m_inputDir);
        // попытка перенести файлы отданные на перенос в свою папку workera
        moveFilesLD(filesToProcess, m_workerProcessDir);

        // сохранение списка файлов, которые удалось перенести
        m_filesInProcess = QDir(m_workerProcessDir).entryInfoList(usingFilters);

        // если список не пустой
        if (m_filesInProcess.size() != 0) {
            m_isInProcess = true;

            flushStatistic();

            createAvsProcess();
            m_avsProcess->start(m_avsExecFileName, QStringList() << m_workerProcessDir);
        }
    }
}

// статистики последней обработки
WorkerStatistic InvestigatorWorker::getLastStatistics() {
    return WorkerStatistic(m_id, m_filesInProcess.size(), m_filesToProcessSize,
                           m_pwdFilesNb, m_infList.size(), m_speed, m_avVersion);
}

// сброс времянок
void InvestigatorWorker::flushStatistic() {
    m_avsReport.clear();
    m_lastProcStartTime = QDateTime::currentDateTime();

    m_lastProcWorkTimeInMsec = 0;
    m_filesToProcessSize = 0;

    m_reportLines.clear();
    m_baseAVSVersion.clear();
    m_m52coreVersion.clear();
    m_drwebCoreVersion.clear();
    m_kasperCoreVersion.clear();
    m_avVersion.clear();
    m_tempSplitList1.clear();
    m_tempSplitList2.clear();
    m_pwdFilesNb = 0;
    m_infList.clear();
}

// парсинг отчета АВС
void InvestigatorWorker::parseReport() {

    // время проверки
    m_lastProcWorkTimeInMsec = m_lastProcStartTime.msecsTo(QDateTime::currentDateTime());

    m_reportLines = m_avsReport.split("\n");

    // сохранение версии баз и статистики
    extractAVSVersions();
    collectStatisticAboutLastScan();

    // обработка файлов
    processingInfectedFiles();
    processingOtherFiles();

    m_isInProcess = false;

    emit finish(m_id);
}

void InvestigatorWorker::createAvsProcess() {
    if (m_avsProcess) {
        m_avsProcess->close();
        delete m_avsProcess;
    }
    m_avsProcess = new QProcess(nullptr);

    connect(m_avsProcess,  QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
    [ = ](int exitCode, QProcess::ExitStatus exitStatus) {
        Q_UNUSED(exitCode)

        if (exitStatus == QProcess::NormalExit) {

            m_avsReport = m_win1251Codec->toUnicode(m_avsProcess->readAll());

            if (m_avsReport.contains("Время сканирования") &&
                    m_avsReport.contains("Сканирование объектов: ")) {
                parseReport();
            } else {
                log(QString("Отчет АВС поврежден."), Logger::UI + Logger::SYSLOG + Logger::FILE);
            }
        } else {
            log(QString("Процесс проверки воркера %1 был аварийно завершен.").arg(
                    m_id), Logger::UI + Logger::SYSLOG + Logger::FILE);
        }
    }
           );
}

// извлечение информации о версих баз
void InvestigatorWorker::extractAVSVersions() {
    if (m_reportLines.size() > 5) {

        QString kasperNewVersion = m_reportLines[5].remove("Версия ядра Касперский: "),
                drwebNewVersion  = m_reportLines[4].remove("Версия ядра Dr.Web: ");

        m_baseAVSVersion = m_reportLines[2].remove("Версия баз: ");
        m_m52coreVersion = m_reportLines[3].remove("Версия ядра M-52: ");

        m_baseAVSVersion.chop(1);
        m_m52coreVersion.chop(1);

        drwebNewVersion.truncate(drwebNewVersion.lastIndexOf(" количество записей"));
        drwebNewVersion.replace(drwebNewVersion.lastIndexOf(","), 1, ")");
        drwebNewVersion.replace("база ", "");

        kasperNewVersion.truncate(kasperNewVersion.lastIndexOf(" количество записей"));
        kasperNewVersion.replace(kasperNewVersion.lastIndexOf(","), 1, ")");
        kasperNewVersion.replace("база ", "");

        m_drwebCoreVersion = drwebNewVersion;
        m_kasperCoreVersion = kasperNewVersion;

        m_avVersion =
            QString("Версия баз: %1\nЯдро M-52: %2\nЯдро Dr.Web: %3\nЯдро Kaspersky: %4")
            .arg(m_baseAVSVersion)
            .arg(m_m52coreVersion)
            .arg(m_m52coreVersion)
            .arg(m_kasperCoreVersion);
    }
}

// сохранение объема проверенных файлов и скорости проверки
void InvestigatorWorker::collectStatisticAboutLastScan() {

    // подсчет объема проверенных файлов
    m_filesToProcessSize = [ = ]() {
        quint64 listSize = 0;
        foreach (QFileInfo info, m_filesInProcess) {
            listSize += info.size();
        }
        return listSize;
    }
    ();

    // вычисление скорости
    m_speed = m_lastProcWorkTimeInMsec ? (m_filesToProcessSize * 1000 / (double(
                                                                             m_lastProcWorkTimeInMsec))) : 0;

    // подсчет кол-ва запароленных
    for (int i = 0; i < m_reportLines.size(); i++) {
        if (m_reportLines[i].contains("M-52: файл") &&
                m_reportLines[i].contains("- Защищен паролем")) {
            m_pwdFilesNb++;
        }
    }
}

// обработка зараженных
void InvestigatorWorker::processingInfectedFiles() {
    // поиск зараженных файлов в отчете АВС
    for (int i = 0; i < m_reportLines.size(); i++) {

        // если часть строки репорта содержит путь к папке проверки, то в этой строке инфа о зараженном файле
        if (m_reportLines[i].contains(QDir::toNativeSeparators(m_workerProcessDir)) &&
                m_reportLines[i].contains("M-52:")) {

            // разделение на подстроки по директории для сканирования
            m_tempSplitList1 = m_reportLines[i].split(QDir::toNativeSeparators(m_workerProcessDir) +
                                                      "\\"); // разделитель - путь к папке с файлами

            // если после деления есть подстроки
            if (m_tempSplitList1.size() > 1) {
                m_tempSplitList1 = m_tempSplitList1[1].split("'");

                // выделение имени инфицированного файла и избавление от вложенных архивов
                if (m_tempSplitList1[0].contains("//")) {
                    m_tempSplitList2 = m_tempSplitList1[0].split("//");
                    if (m_tempSplitList2.size()) {
                        m_tempInfectedFileName = m_tempSplitList2[0];
                    }
                } else {
                    m_tempInfectedFileName = m_tempSplitList1[0];
                }

                // извлечение информации о вирусе
                m_tempSplitList1 = m_reportLines[i].split("инфицирован ");
                if (m_tempSplitList1.size() > 1) {
                    m_tempVirusInfo = m_tempSplitList1[1];

                    m_tempVirusInfo.remove(" - Файл пропущен");
                    m_tempVirusInfo.truncate(m_tempVirusInfo.lastIndexOf(")") + 1);

                    m_tempVirusInfo.remove(0, 1); // remove first '('
                    m_tempVirusInfo.chop(1); // remove last ')'

                    // в 0 строка для DrWeb, в 1 для Kaspersky
                    QStringList twoAVSinformation = m_tempVirusInfo.split(";");

                    QStringList drwebDetectedVirusesList, kasperDetectedVirusesList, tempList;

                    // если в начале DrWeb
                    if (twoAVSinformation[0].startsWith("DrWeb: ")) {

                        // вирусы, обнаруженные DrWeb
                        QString drwebDetectedViruses = twoAVSinformation[0].remove("DrWeb: ");
                        drwebDetectedViruses.remove(";");
                        // все вирусы с повторениями
                        tempList =
                            drwebDetectedViruses.split(", ");// образование нового списка без повторений

                        for (auto virusName : tempList) {
                            if (!drwebDetectedVirusesList.contains(virusName) && virusName != " ") {
                                drwebDetectedVirusesList.append(virusName);
                            }
                        }

                        // вирусы, обнаруженные Kaspersky
                        QString kasperDetectedViruses = twoAVSinformation[1].remove(" Kaspersky: ");
                        kasperDetectedViruses.remove("; ");
                        // все вирусы с повторениями
                        tempList = kasperDetectedViruses.split(", ");
                        // образование нового списка без повторений
                        for (auto virusName : tempList) {
                            if (!kasperDetectedVirusesList.contains(virusName) && virusName != " ") {
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
                        for (auto virusName : tempList) {
                            if (!kasperDetectedVirusesList.contains(virusName) && virusName != " ") {
                                kasperDetectedVirusesList.append(virusName);
                            }
                        }
                    }

                    // формирование новой записи о вирусе
                    m_tempVirusInfo = "(";
                    if (drwebDetectedVirusesList.size()) {
                        m_tempVirusInfo += "DrWeb: " + entryListToString(drwebDetectedVirusesList);
                        m_tempVirusInfo.remove(m_tempVirusInfo.size() - 1, 1);
                        m_tempVirusInfo += "; ";
                    }
                    if (kasperDetectedVirusesList.size()) {
                        m_tempVirusInfo += "Kaspersky: " + entryListToString(kasperDetectedVirusesList);
                        m_tempVirusInfo.remove(m_tempVirusInfo.size() - 1, 1);
                        m_tempVirusInfo += "; ";
                    }
                    m_tempVirusInfo.remove(m_tempVirusInfo.size() - 1, 1);
                    m_tempVirusInfo += ")";

                    // в список зараженных файлов файл добавляется только в том случае, если его там еще нет
                    if (!isContainedFile(m_infList, m_tempInfectedFileName) &&
                            m_tempVirusInfo.length() > 3) { /// *** ХЗ ПОЧЕМУ 3
                        m_infList.push_back(QPair<QString, QString> {m_tempInfectedFileName, m_tempVirusInfo});
                    }
                }
            }
        }
    }

    // сохранение отчета АВС если есть инфицированные файла
    // if(m_infList.size()) {
    //     saveInfectedFileReport();
    // }

    // обработка зараженных
    foreach (auto infectedFile, m_infList) {
        saveInfectedFileReport(infectedFile.first, infectedFile.second);

        QString message = QString("%1 %2").arg(infectedFile.first).arg(infectedFile.second);
        log(QString("Зараженный файл: %1").arg(message), Logger::UI + Logger::SYSLOG);
        moveFile(QString("%1\\%2").arg(m_workerProcessDir).arg(infectedFile.first), m_dangerDir);
    }
}

// перенос в cleanDir
void InvestigatorWorker::processingOtherFiles() {
    // все оставшиеся файлы в m_workerProcessDir переносятся в m_cleanDir
    moveFilesDD(m_workerProcessDir, m_cleanDir);
}

// генератор имени отчета
// формат: '<workerId>_<detectedTime>.rep'
QString InvestigatorWorker::getInfectedReportFileName() {
    return QString("inf_%1_%2.rep")
           .arg(m_id)
           .arg(QDateTime::currentDateTime().toString("dd-MM-yy hh-mm-ss"));
}

// сохранение информации о заражении в файл с именемfileName
void InvestigatorWorker::saveInfectedFileReport(QString fileName, QString report) {
    if (!QDir(m_reportDir).exists()) {
        QDir().mkdir(m_reportDir);
    }

    if (fileName.isEmpty()) {
        fileName = getInfectedReportFileName();
    }
    fileName = QString("%1\\%2.txt").arg(m_reportDir).arg(QFileInfo(fileName).baseName());

    QFile outFile(fileName);
    outFile.open(QIODevice::WriteOnly);
    QTextStream ts(&outFile);
    ts << report << endl;
    outFile.close();
}
