#include "widget.h"
#include <QApplication>
#include <QSettings>

#include "httplistener.h"
#include "httpjsonresponder.h"

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

   QApplication app(argc, argv);
   Widget w;

   QSettings* listenerSettings = new QSettings("C:/settings.ini", QSettings::IniFormat, &app);
   listenerSettings->beginGroup("listener");

   HttpJsonResponder* responder = new HttpJsonResponder(&app);
   responder->setInvestigatorPtr(w.getInvestigatorPtr());
   new HttpListener(listenerSettings, responder, &app); // Start the HTTP server

   w.show();

   return app.exec();
}
