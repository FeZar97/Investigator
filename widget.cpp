#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent): QWidget(parent), ui(new Ui::Widget), settings("FeZar97", "Investigator") {

    ui->setupUi(this);
    setLayout(ui->mainLayout);
    setWindowTitle(QString("Investigator ") + VERSION);

    restoreGeometry(settings.value("geometry").toByteArray());
    distributor.setWatchDir(settings.value("watchDir", "C:/").toString());
    distributor.setInvestigatorDir(settings.value("investigatorDir", QDir::tempPath()).toString());
    distributor.setCleanDir(settings.value("cleanDir", "C:/").toString());
    distributor.setDangerDir(settings.value("dangerDir", "C:/").toString());
    distributor.setAVUse(AV::KASPER, settings.value("useKasper", true).toBool());
    distributor.setAVFile(AV::KASPER, settings.value("kasperFilePath", "C:/Program Files (x86)/Kaspersky Lab/Kaspersky Endpoint Security for Windows/avp.com").toString());
    distributor.setAVUse(AV::DRWEB, settings.value("useDrweb", true).toBool());
    distributor.setAVFile(AV::DRWEB, settings.value("drwebFilePath", "C:/Program Files/DrWeb/dwscancl.exe").toString());

    settingsWindow = new Settings(this, &distributor, settings.value("settingsWinGeometry").toByteArray(), settings.value("settingsWinVisible").toBool());
    statisticWindow = new Statistics(this, &distributor, settings.value("statisticWinGeometry").toByteArray(), settings.value("statisticWinVisible").toBool());

    connect(&distributor,   &Distributor::updateUi,   this,             &Widget::updateUi);
    connect(&distributor,   &Distributor::log,        this,             &Widget::log);
    connect(&workThread,    &QThread::started,        &distributor,     &Distributor::startWatchDirEye);

    distributor.moveToThread(&workThread);
    workThread.start();

    log(currentDateTime() + " Программа запущена.");

    updateUi();
}

Widget::~Widget() {
    settings.setValue("geometry",               saveGeometry());

    settings.setValue("settingsWinGeometry",    settingsWindow->saveGeometry());
    settings.setValue("settingsWinVisible",     settingsWindow->isVisible());
    settings.setValue("statisticWinGeometry",   statisticWindow->saveGeometry());
    settings.setValue("statisticWinVisible",    statisticWindow->isVisible());

    settings.setValue("watchDir",               distributor.getWatchDir());
    settings.setValue("investigatorDir",        distributor.getInvestigatorDir());
    settings.setValue("cleanDir",               distributor.getCleanDir());
    settings.setValue("dangerDir",              distributor.getDangerDir());

    settings.setValue("kasperFilePath",         distributor.getAVFile(AV::KASPER));
    settings.setValue("useKasper",              distributor.getAVUse(AV::KASPER));

    settings.setValue("drwebFilePath",          distributor.getAVFile(AV::DRWEB));
    settings.setValue("useDrweb",               distributor.getAVUse(AV::DRWEB));

    workThread.quit();
    workThread.wait();

    delete settingsWindow;
    delete statisticWindow;

    delete ui;
}
void Widget::log(const QString &s) {
    ui->logPTE->appendPlainText(s);
}

void Widget::updateUi() {
    ui->startButton->setEnabled(!distributor.isInProcessing());
    ui->stopButton->setEnabled(distributor.isInProcessing());

    settingsWindow->updateUi();
    //statisticWindow->updateUi();
}

void Widget::on_startButton_clicked() {
    distributor.startWatchDirEye();
}

void Widget::on_stopButton_clicked() {
    distributor.stopWatchDirEye();
}

void Widget::on_settingsButton_clicked() {
    settingsWindow->setVisible(!settingsWindow->isVisible());
}

void Widget::on_statisticButton_clicked() {
    statisticWindow->setVisible(!statisticWindow->isVisible());
}

void Widget::on_clearButton_clicked() {
    ui->logPTE->clear();
}
