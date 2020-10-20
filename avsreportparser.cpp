#include "avsreportparser.h"

void AvsReportParser::extractAvsVersions() {
    QString kasperNewVersion = m_reportLines[5].remove("Версия ядра Касперский: "),
            drwebNewVersion  = m_reportLines[4].remove("Версия ядра Dr.Web: ");

    parseResult.avsVersions.m_baseAVSVersion = m_reportLines[2].remove("Версия баз: ");
    parseResult.avsVersions.m_m52coreVersion = m_reportLines[3].remove("Версия ядра M-52: ");

    parseResult.avsVersions.m_baseAVSVersion.chop(1);
    parseResult.avsVersions.m_m52coreVersion.chop(1);

    drwebNewVersion.truncate(drwebNewVersion.lastIndexOf(" количество записей"));
    drwebNewVersion.replace(drwebNewVersion.lastIndexOf(","), 1, ")");
    drwebNewVersion.replace("база ", "");

    kasperNewVersion.truncate(kasperNewVersion.lastIndexOf(" количество записей"));
    kasperNewVersion.replace(kasperNewVersion.lastIndexOf(","), 1, ")");
    kasperNewVersion.replace("база ", "");

    parseResult.avsVersions.m_drwebCoreVersion = drwebNewVersion;
    parseResult.avsVersions.m_kasperCoreVersion = kasperNewVersion;

    parseResult.avsVersions.m_version =
        QString("Версия баз: %1\nЯдро M-52: %2\nЯдро Dr.Web: %3\nЯдро Kaspersky: %4")
        .arg(parseResult.avsVersions.m_baseAVSVersion)
        .arg(parseResult.avsVersions.m_m52coreVersion)
        .arg(parseResult.avsVersions.m_m52coreVersion)
        .arg(parseResult.avsVersions.m_kasperCoreVersion);
}

void AvsReportParser::findInfectedFiles() {

    // поиск зараженных файлов в отчете АВС
    for (int i = 0; i < m_reportLines.size(); i++) {

        // если часть строки репорта содержит путь к папке проверки, то в этой строке инфа о зараженном файле
        if (m_reportLines[i].contains("инфицирован ") && m_reportLines[i].contains("M-52:")) {

            QStringList m_tempSplitList1, m_tempSplitList2;
            QString m_tempInfectedFileName, m_tempVirusInfo;

            // разделение на подстроки; разделитель - фраза "файл '"
            m_tempSplitList1 = m_reportLines[i].split("файл '");

            // если после деления есть подстроки, то подстрока [1] начинается с имени зараженного файла
            // для извлечения этого имени необходимо убрать все символы после второй одинарной кавычки
            if (m_tempSplitList1.size() > 1) {

                m_tempSplitList1 = m_tempSplitList1[1].split("'");

                // извлечение имени инфицированного файла
                m_tempInfectedFileName = m_tempSplitList1[0];

                // извлечение информации о вирусе
                m_tempSplitList1 = m_reportLines[i].split("инфицирован (");

                // если после деления есть подстроки, то в подстроке [1] есть информация о вирусе
                if (m_tempSplitList1.size() > 1) {

                    m_tempVirusInfo = m_tempSplitList1[1];
                    m_tempVirusInfo.remove(" ) - Файл пропущен");

                    // разделение на информацию по касперскому и по вебу
                    // в [0] строка для DrWeb, в [1] для Kaspersky
                    QStringList twoAVSinformation = m_tempVirusInfo.split(";");
                    QStringList drwebDetectedVirusesList, kasperDetectedVirusesList, tempList;

                    // если в начале DrWeb
                    if (twoAVSinformation[0].contains("DrWeb: ")) {

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

                    parseResult.infectedFiles.push_back({QFileInfo(m_tempInfectedFileName), QString(m_tempVirusInfo)});
                }
            }
        }
    }
}

AvsReportParser::AvsReportParser():
    m_win1251Codec{QTextCodec::codecForName("Windows-1251")}
{}

ParseResult AvsReportParser::parse(QByteArray data) {
    m_avsReport = m_win1251Codec->toUnicode(data);

    parseResult.clear();

    if (m_avsReport.contains("Время сканирования") &&
            m_avsReport.contains("Сканирование объектов: ")) {

        m_reportLines = m_avsReport.split("\n");

        extractAvsVersions();
        findInfectedFiles();
    } else {
        parseResult.lastErrorDescription = QString("Отчет АВС поврежден.");
    }

    return parseResult;
}
