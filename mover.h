#ifndef MOVER_H
#define MOVER_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QSet>
#include <QDir>
#include <QDebug>

class Mover : public QObject
{
    Q_OBJECT

    QString sourceDir;
    QString targetDir;

    QFileSystemWatcher watcher;

    QStringList filesToMove;

    int movedFilesNb;
    double movedFilesSizeMB;

public:
    explicit Mover(QObject *parent = nullptr);

    void setSourceDir(QString _sourceDir);
    QString getSourceDir();

    void setTargetDir(QString _targetDir);
    QString getTargetDir();

    int getMovedFilesNb();
    double getMovedFilesSize();

    void onSourceDirChange(const QString &path);

    void startWork();
    void stopWork();

    void clear();
    void resetCounters();

signals:
    void targetDirPathChange(QString newTargetDirPath);
    void updateUi();
};

#endif // MOVER_H
