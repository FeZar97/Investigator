#include "httpsettingsresponder.h"

HttpSettingsResponder::HttpSettingsResponder(QObject* parent, InvestigatorOrchestartor *investigatorPtr):
    HttpRequestHandler(parent),
    m_investigator(investigatorPtr) {
}

void HttpSettingsResponder::service(HttpRequest& request, HttpResponse& response) {

    Q_UNUSED(request)

    QJsonObject obj;
    if(m_investigator) {
        obj.insert("sourceDir",                 m_investigator->sourceDir());
        obj.insert("processDir",                m_investigator->processDir());
        obj.insert("cleanDir",                  m_investigator->cleanDir());
        obj.insert("infectedDir",               m_investigator->infectedDir());
        obj.insert("avsExecFileName",           m_investigator->avsExecFileName());
        obj.insert("currentWorkersNb",          m_investigator->currentWorkersNb());
        obj.insert("thresholdFilesNb",          m_investigator->thresholdFilesNb());
        obj.insert("thresholdFilesSize",        m_investigator->thresholdFilesSize());
        obj.insert("thresholdFilesSizeUnit",    m_investigator->thresholdFilesSizeUnit());
        obj.insert("syslogAddress",             m_investigator->syslogAddress());
    } else {
        obj.insert("error", "ptr to investigator has been not defined");
    }

    QJsonDocument doc(obj);
    response.setHeader("Content-Type", "application/json; charset=utf-8");
    response.write(doc.toJson(QJsonDocument::Indented), true);
}
