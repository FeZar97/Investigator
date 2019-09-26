#ifndef DISTRIBUTOR_H
#define DISTRIBUTOR_H

#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QDebug>
#include <QDir>
#include <QTimer>
#include <QProcess>

#include "mover.h"
#include "checker.h"

#define TEMP_DIR_NAME "temp"

class Distributor : public QObject
{
    Q_OBJECT

public:
    explicit Distributor(QObject *parent = nullptr);

    Mover mover;
    Checker checker;

    void startWork();

signals:
    void updateUi();
    void log(QString _message);
};

#endif // DISTRIBUTOR_H
