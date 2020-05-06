#ifndef HTTPJSONRESPONDER_H
#define HTTPJSONRESPONDER_H

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>

#include "investigator.h"
#include "httprequesthandler.h"

using namespace stefanfrings;

class HttpJsonResponder : public HttpRequestHandler
{
    Q_OBJECT
    Investigator* m_investigator;

public:
    HttpJsonResponder(QObject* parent = nullptr);
    void setInvestigatorPtr(Investigator* investigatorPtr = nullptr);
    void service(HttpRequest& request, HttpResponse& response);
};

#endif // HTTPJSONRESPONDER_H
