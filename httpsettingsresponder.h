#ifndef HTTPSETTINGSRESPONDER_H
#define HTTPSETTINGSRESPONDER_H

#include <QJsonObject>
#include <QJsonDocument>

#include "httprequesthandler.h"

#include "investigatororchestartor.h"

using namespace stefanfrings;

class HttpSettingsResponder: public HttpRequestHandler
{
    Q_OBJECT
    InvestigatorOrchestartor *m_investigator;

public:
    HttpSettingsResponder(QObject* parent = nullptr, InvestigatorOrchestartor *investigatorPtr = nullptr);
    void service(HttpRequest& request, HttpResponse& response);
};

#endif // HTTPSETTINGSRESPONDER_H
