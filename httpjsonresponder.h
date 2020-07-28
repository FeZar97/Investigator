#ifndef HTTPJSONRESPONDER_H
#define HTTPJSONRESPONDER_H

#include <QJsonObject>
#include <QJsonDocument>

#include "httprequesthandler.h"

#include "investigatororchestartor.h"

using namespace stefanfrings;

class HttpJsonResponder : public HttpRequestHandler {
    Q_OBJECT
    InvestigatorOrchestartor *m_investigator;

public:
    HttpJsonResponder(QObject *parent = nullptr, InvestigatorOrchestartor *investigatorPtr = nullptr);
    void service(HttpRequest &request, HttpResponse &response);
};

#endif // HTTPJSONRESPONDER_H
