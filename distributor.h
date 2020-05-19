#ifndef DISTRIBUTOR_H
#define DISTRIBUTOR_H

#include <QFileSystemWatcher>

#include "investigator.h"

class Distributor : public QObject
{
    Q_OBJECT

    Investigator* m_investigator;
    QFileSystemWatcher m_watchDirEye;

public:
    explicit Distributor(QObject *parent = nullptr, Investigator* investigator = nullptr);
    ~Distributor();

    void startWatchDirEye();
    void stopWatchDirEye();

    void onWatchDirChange(const QString &path);

    void clearDir(QString dirPath);

signals:
    void updateUi();
    void log(QString s, LOG_CATEGORY cat);
};

#endif // DISTRIBUTOR_H
