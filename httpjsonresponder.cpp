#include "httpjsonresponder.h"

HttpJsonResponder::HttpJsonResponder(QObject *parent, InvestigatorOrchestartor *investigatorPtr):
    HttpRequestHandler(parent),
    m_investigator(investigatorPtr) {
}

void HttpJsonResponder::service(HttpRequest &request, HttpResponse &response) {

    Q_UNUSED(request)

    QJsonObject obj;
    if (m_investigator) {
        obj.insert("Uptime",                ((__int64)m_investigator->getWorkTimeInSec()));
        obj.insert("QueueCount",            ((__int64)m_investigator->queueFilesNb()));
        obj.insert("QueueSize",             ((__int64)m_investigator->queueFilesSize()));
        obj.insert("ScannedCount",          ((__int64)m_investigator->totalProcessedFilesNb()));
        obj.insert("ScannedSize",           ((__int64)m_investigator->totalProcessedFilesSize()));
        obj.insert("PasswordCount",         ((__int64)m_investigator->totalPwdFilesNb()));
        obj.insert("BadCount",              ((__int64)m_investigator->totalInfectedFilesNb()));
    } else {
        obj.insert("error", "ptr to investigator has been not defined");
    }

    QJsonDocument doc(obj);
    response.setHeader("Content-Type", "application/json; charset=utf-8");
    response.write(doc.toJson(QJsonDocument::Indented), true);
}
