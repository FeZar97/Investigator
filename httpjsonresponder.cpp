#include "httpjsonresponder.h"

HttpJsonResponder::HttpJsonResponder(QObject* parent):
    HttpRequestHandler(parent),
    m_investigator(nullptr) {
}

void HttpJsonResponder::setInvestigatorPtr(Investigator* investigatorPtr) {
    m_investigator = investigatorPtr;
}

void HttpJsonResponder::service(HttpRequest& request, HttpResponse& response) {

    QJsonObject obj;
    if(m_investigator) {
        obj.insert("workTime", m_investigator->getWorkTime());
        obj.insert("infectedObj", m_investigator->getInfectedFilesNb());
        obj.insert("totalObj", m_investigator->getProcessedFilesNb());
        obj.insert("totalVolMb", volumeToString(m_investigator->getProcessedFilesSizeMb()));
    } else {
        obj.insert("error", "ptr to investigator has been not defined");
    }

    QJsonDocument doc(obj);
    response.setHeader("Content-Type", "application/json; charset=utf-8");
    response.write(doc.toJson(QJsonDocument::Compact), true);
}
