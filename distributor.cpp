#include "distributor.h"
#include "widget.h"

Distributor::Distributor(QObject *parent, Investigator* investigator) : QObject(parent) {
    m_investigator = investigator;
    connect(&m_watchDirEye, &QFileSystemWatcher::directoryChanged,  this, &Distributor::onWatchDirChange);
}

Distributor::~Distributor() {}

void Distributor::startWatchDirEye() {
    if(m_investigator->m_isWorking)
        stopWatchDirEye();

    if(m_investigator->beginWork()) {
        m_watchDirEye.addPath(m_investigator->m_watchDir);
        onWatchDirChange("");
    }

    emit updateUi();
}

void Distributor::stopWatchDirEye() {
    m_investigator->stopWork();

    if(!m_watchDirEye.directories().isEmpty()) {
        m_watchDirEye.removePaths(m_watchDirEye.directories());
    }

    emit updateUi();
}

void Distributor::onWatchDirChange(const QString &path) {
    Q_UNUSED(path)

    if(QDir(m_investigator->m_watchDir).entryList(usingFilters).size()) {
        QStringList filesInDir = QDir(m_investigator->m_watchDir).entryList(usingFilters);
        log(QString("Transferring %1 files from watchDir(%2) into inputDir(%3): %4").arg(QDir(m_investigator->m_watchDir).entryList(usingFilters).size())
                                                                                    .arg(m_investigator->m_watchDir)
                                                                                    .arg(m_investigator->m_inputDir)
                                                                                    .arg(entryListToString(filesInDir)), MSG_CATEGORY(DEBUG));
        moveFiles(m_investigator->m_watchDir, m_investigator->m_inputDir, ALL_FILES);
    }

    m_investigator->collectStatistics();

    if(m_investigator->m_isWorking)
        m_investigator->onProcessFinished();
}

void Distributor::clearDir(QString dirPath) {
    foreach(QFileInfo fileInfo, QDir(dirPath).entryInfoList(usingFilters)) {
        QFile::remove(fileInfo.absoluteFilePath());
    }
    m_investigator->collectStatistics();
}
