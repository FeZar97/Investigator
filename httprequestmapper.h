#ifndef HTTPREQUESTMAPPER_H
#define HTTPREQUESTMAPPER_H

#include "httprequesthandler.h"

#include "httpjsonresponder.h"
#include "httpsettingsresponder.h"

#include "investigatororchestartor.h"

using namespace stefanfrings;

class HttpRequestMapper: public HttpRequestHandler {
    Q_OBJECT

    InvestigatorOrchestartor *m_investigator;
    void setInvestigator(InvestigatorOrchestartor *orchestartorPtr = nullptr);

public:
    HttpRequestMapper(QObject *parent = 0, InvestigatorOrchestartor *investigatorPtr = nullptr);
    void service(HttpRequest &request, HttpResponse &response);
};

#endif // HTTPREQUESTMAPPER_H
