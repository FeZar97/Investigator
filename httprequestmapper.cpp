#include "httprequestmapper.h"

void HttpRequestMapper::setInvestigator(InvestigatorOrchestartor *investigatorPtr) {
    m_investigator = investigatorPtr;
}

HttpRequestMapper::HttpRequestMapper(QObject *parent, InvestigatorOrchestartor *investigatorPtr):
    HttpRequestHandler(parent) {
    setInvestigator(investigatorPtr);
}

void HttpRequestMapper::service(HttpRequest &request, HttpResponse &response) {
    QByteArray path = request.getPath();

    if (path == "/stat" || path == "/statistics" || path == "/stat.json") {
        HttpJsonResponder(this, m_investigator).service(request, response);
    }
    if (path == "/flushGlobalStatistic") {
        m_investigator->flushGlobalStatistic();
    }
    if (path == "/getSettings") {
        HttpSettingsResponder(this, m_investigator).service(request, response);
    }
    if (path == "/setSettings") {
        //HttpJsonResponder(this, m_investigator).service(request, response);
    } else {
        response.setStatus(404, "Not found");
        response.write("Incorrect URL.", true);
    }
}
