#ifndef AVSREPORTPARSER_H
#define AVSREPORTPARSER_H

#include <QFileInfoList>
#include <QTextCodec>

#include "../FeZarSource/FeZar97.h"

typedef QList<QPair<QFileInfo, QString>> InfectList;

/*! \brief Версии баз */
class AvsBaseVersions {
public:
    QString m_version; // полная версия баз
    QString m_baseAVSVersion; // версии баз
    QString m_m52coreVersion; // версии баз м52
    QString m_drwebCoreVersion; // версии баз веба
    QString m_kasperCoreVersion; // версии баз касперского

    void clear() {
        m_version.clear();
        m_baseAVSVersion.clear();
        m_m52coreVersion.clear();
        m_drwebCoreVersion.clear();
        m_kasperCoreVersion.clear();
    }
};

/*! \brief Результат парсинга */
class ParseResult {
public:
    InfectList infectedFiles;
    AvsBaseVersions avsVersions;
    QString lastErrorDescription; // ошибка в работе парсера

    ParseResult(InfectList list = {}, AvsBaseVersions versions = {}):
        infectedFiles{list},
        avsVersions{versions} {};

    void clear() {
        infectedFiles.clear();
        avsVersions.clear();
        lastErrorDescription.clear();
    }
};

/*! \brief Парсер отчета АВС */
class AvsReportParser {

    QTextCodec *m_win1251Codec; // кодек для конвертации отчета АВС
    QString m_avsReport; // отчет АВС
    QStringList m_reportLines;

    ParseResult parseResult;

    void extractAvsVersions();
    void findInfectedFiles();

public:
    AvsReportParser();

    ParseResult parse(QByteArray data);
};

#endif // AVSREPORTPARSER_H
