#include "distributor.h"
#include "widget.h"

Distributor::Distributor(QObject *parent, Investigator* investigator) : QObject(parent) {
    m_investigator = investigator;
    connect(&m_watchDirEye, &QFileSystemWatcher::directoryChanged,  this, &Distributor::onWatchDirChange);
}

Distributor::~Distributor() {}

void Distributor::startWatchDirEye() {

    // удаление тех путей который уже были в watcher
    if(!m_watchDirEye.directories().isEmpty()) {
        m_watchDirEye.removePaths(m_watchDirEye.directories());
    }

    m_watchDirEye.addPath(m_investigator->m_watchDir);

    // запуск слежения
    onWatchDirChange("");
}

// остановка слежения
void Distributor::stopWatchDirEye() {
    if(!m_watchDirEye.directories().isEmpty()) {
        m_watchDirEye.removePaths(m_watchDirEye.directories());
    }
}

void Distributor::onWatchDirChange(const QString &path) {
    Q_UNUSED(path)

    // если есть что переносить
    while(QDir(m_investigator->m_watchDir).entryList(usingFilters).size()) {
        moveFiles(m_investigator->m_watchDir, m_investigator->m_inputDir, MAX_FILES_TO_MOVE);
    }

    // попытка начать проверку файлов
    emit tryProcess();
}

void Distributor::distributorMoveFiles(QString sourceDir, QString destinationDir, int limit) {
    moveFiles(sourceDir, destinationDir, limit);
}
