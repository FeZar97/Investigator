#include "widget.h"
#include <QApplication>
#include <QSplashScreen>
#include <QPixmap>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QPixmap pix(":/img/SPLASHSCREEN.png");
    QSplashScreen *splashScreen = new QSplashScreen(pix);
    splashScreen->show();
    splashScreen->showMessage("Loaded modules");
    app.processEvents();

    QTime time;
    time.start();

    while(time.elapsed() < 1500){
        splashScreen->setPixmap(pix);
        app.processEvents();
    }

    Widget w;
    w.show();
    splashScreen->finish(&w);
    return app.exec();
}
