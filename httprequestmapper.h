#ifndef HTTPREQUESTMAPPER_H
#define HTTPREQUESTMAPPER_H

#include "httpjsonresponder.h"
#include "httprequesthandler.h"

using namespace stefanfrings;

class HttpRequestMapper: public HttpRequestHandler {
    Q_OBJECT

    Investigator *m_investigator;
    void setInvestigator(Investigator* investigatorPtr = nullptr);

public:
    HttpRequestMapper(QObject* parent = 0, Investigator* investigatorPtr = nullptr);
    void service(HttpRequest& request, HttpResponse& response);

signals:
    void turnOff(int code);
};

#endif // HTTPREQUESTMAPPER_H
