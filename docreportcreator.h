#ifndef DOCREPORTCREATOR_H
#define DOCREPORTCREATOR_H

#include <QObject>
#include <QAxWidget>
#include <QAxObject>

class DocReportCreator
{

public:
    DocReportCreator();

    static void createReport(QString reportName);
};

#endif // DOCREPORTCREATOR_H
