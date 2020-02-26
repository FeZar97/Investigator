#include "widget.h"
#include <QApplication>

void messageHandler(QtMsgType type, const QMessageLogContext& ctx, const QString& msg) {
    QString txt;
    static long long uid = 0;

    QRegExp rx("([\\w-]+::[\\w-]+)");
    if(rx.indexIn(ctx.function) == -1)
        return;

    QString function = rx.cap(1);

    switch(type) {
        case QtInfoMsg:
            txt = QString("INFO_MSG %1").arg(msg);
            break;

        case QtDebugMsg:
            txt = QString("DEBUG_MSG %1").arg(msg);
            break;

        case QtWarningMsg:
            txt = QString("WARNING_MSG %1").arg(msg);
            break;

        case QtCriticalMsg:
            txt = QString("CRITICAL_MSG %1").arg(msg);
            break;

        case QtFatalMsg:
            txt = QString("FATAL_MSG %1").arg(msg);
            abort();
    }
    QDateTime dateTime = QDateTime::currentDateTime();
    uid++;
    txt = QString("%1:%2 %3").arg(dateTime.toString("yyyy-MM-dd hh:mm:ss")).arg(uid).arg(txt);

    QString dirPath = "C:/logs/" + QDate().currentDate().toString("yy-MM-dd");
    if(!QDir(dirPath).exists()) QDir().mkpath(dirPath);
    QFile outFile(QString("%1KAP-log-%2-%3.log").arg(dirPath+ "/").arg(QDateTime::currentDateTime().toString("hh")).arg(VERSION));
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << endl;
    outFile.close();
}

int main(int argc, char *argv[]) {
   qInstallMessageHandler(messageHandler);
   qSetMessagePattern("%{type} %{if-category}%{category}: %{endif}%{function}: %{message}");

   QApplication a(argc, argv);
   Widget w;
   w.show();

   return a.exec();
}
