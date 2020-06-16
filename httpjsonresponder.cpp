#include "httpjsonresponder.h"

HttpJsonResponder::HttpJsonResponder(QObject* parent, Investigator* investigatorPtr):
    HttpRequestHandler(parent),
    m_investigator(investigatorPtr) {
}

void HttpJsonResponder::service(HttpRequest& request, HttpResponse& response) {

    Q_UNUSED(request)

    QJsonObject obj;
    if(m_investigator) {

        /*
        obj.insert("avsBaseVersion",      m_investigator->m_baseVersion);
        obj.insert("m52coreVersion",      m_investigator->m_m52coreVersion);
        obj.insert("drwebCoreVersion",    m_investigator->m_drwebCoreVersion);
        obj.insert("kasperCoreVersion",   m_investigator->m_kasperCoreVersion);
        obj.insert("totalVolInByte",      ((__int64)(m_investigator->m_processedFilesSizeMb * 1024 * 1024)));
        obj.insert("infectedObjNb",       m_investigator->getInfectedFilesNb());
        obj.insert("totalObjNb",          m_investigator->getProcessedFilesNb());
        obj.insert("workTimeInSec",       m_investigator->m_workTimeInSec);
        obj.insert("spoVersion",          VERSION);
        obj.insert("queueFilesNb",        m_investigator->m_inQueueFilesNb);
        obj.insert("queueVolInBytes",     ((__int64)(m_investigator->m_inQueueFileSizeMb * 1024 * 1024)));
        obj.insert("errorScanning",       m_investigator->m_scanningErrorFilesNb);
        obj.insert("inactionTimeInSec",   m_investigator->m_lastProcessStartTime.secsTo(QDateTime::currentDateTime()));
        obj.insert("avgSpeedInBytes",     m_investigator->m_averageProcessSpeed * 1024 * 1024);
        obj.insert("watchFilesNb",        m_investigator->m_inWatchFilesNb);
        obj.insert("passwordProtectedNb", m_investigator->m_passwordProtectedFilesNb);
        */

        obj.insert("avsBaseVersion",        m_investigator->m_baseVersion);
        obj.insert("m52coreVersion",        m_investigator->m_m52coreVersion);
        obj.insert("drwebCoreVersion",      m_investigator->m_drwebCoreVersion);
        obj.insert("kasperCoreVersion",     m_investigator->m_kasperCoreVersion);
        obj.insert("ScannedSize",           ((__int64)(m_investigator->m_processedFilesSizeMb * 1024 * 1024)));
        obj.insert("BadCount",              m_investigator->getInfectedFilesNb());
        obj.insert("ScannedCount",          m_investigator->getProcessedFilesNb());
        obj.insert("Uptime",                m_investigator->m_workTimeInSec);
        obj.insert("spoVersion",            VERSION);
        obj.insert("QueueCount",            m_investigator->m_inQueueFilesNb + m_investigator->m_inWatchFilesNb);
        obj.insert("QueueSize",             ((__int64)(m_investigator->m_inQueueFileSizeMb * 1024 * 1024)));
        obj.insert("ErrorCount",            m_investigator->m_scanningErrorFilesNb);
        obj.insert("inactionTimeInSec",     m_investigator->m_lastProcessStartTime.secsTo(QDateTime::currentDateTime()));
        obj.insert("avgSpeedInBytes",       m_investigator->m_averageProcessSpeed * 1024 * 1024);
        obj.insert("watchFilesNb",          m_investigator->m_inWatchFilesNb);
        obj.insert("PasswordCount",         m_investigator->m_passwordProtectedFilesNb);
    } else {
        obj.insert("error", "ptr to investigator has been not defined");
    }

    QJsonDocument doc(obj);
    response.setHeader("Content-Type", "application/json; charset=utf-8");
    response.write(doc.toJson(QJsonDocument::Indented), true);
}
