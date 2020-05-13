#include "httpjsonresponder.h"

HttpJsonResponder::HttpJsonResponder(QObject* parent, Investigator* investigatorPtr):
    HttpRequestHandler(parent),
    m_investigator(investigatorPtr) {
}

void HttpJsonResponder::service(HttpRequest& request, HttpResponse& response) {

    Q_UNUSED(request)

    QJsonObject obj;
    if(m_investigator) {
        obj.insert("avsBaseVersion",    m_investigator->m_baseVersion);
        obj.insert("m52coreVersion",    m_investigator->m_m52coreVersion);
        obj.insert("drwebCoreVersion",  m_investigator->m_drwebCoreVersion);
        obj.insert("kasperCoreVersion", m_investigator->m_kasperCoreVersion);
        obj.insert("totalVolInByte",    (long long)(m_investigator->m_processedFilesSizeMb * 8));
        obj.insert("infectedObjNb",     m_investigator->getInfectedFilesNb());
        obj.insert("totalObjNb",        m_investigator->getProcessedFilesNb());
        obj.insert("workTimeInSec",     m_investigator->m_workTimeInSec);
        obj.insert("spoVersion",        VERSION);
    } else {
        obj.insert("error", "ptr to investigator has been not defined");
    }

    QJsonDocument doc(obj);
    response.setHeader("Content-Type", "application/json; charset=utf-8");
    response.write(doc.toJson(QJsonDocument::Compact), true);
}
