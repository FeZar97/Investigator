#include "httprequestmapper.h"

void HttpRequestMapper::setInvestigator(Investigator* investigatorPtr) {
    m_investigator = investigatorPtr;
}

HttpRequestMapper::HttpRequestMapper(QObject* parent, Investigator* investigatorPtr):
    HttpRequestHandler(parent) {
    setInvestigator(investigatorPtr);
}

void HttpRequestMapper::service(HttpRequest& request, HttpResponse& response) {
    QByteArray path = request.getPath();

    if(path == "/stat" || path == "/statistics" || path == "/stat.json") {
        HttpJsonResponder(this, m_investigator).service(request, response);
    } else if (path=="/turnoff") {
        emit turnOff(0);
        response.setStatus(200, "OK");
        response.write("Program is disabled.",true);
    } else {
        response.setStatus(404,"Not found");
        response.write("Incorrect URL.",true);
    }
}
