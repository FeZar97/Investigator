#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent): QWidget(parent), ui(new Ui::Widget), settings("FeZar97", "Investigator") {

    ui->setupUi(this);
    setLayout(ui->mainLayout);

    log("Программа запущена");
    setWindowIcon(QIcon(":/investigator.ico"));

    restoreGeometry(settings.value("geometry").toByteArray());

    connect(&distributor, &Distributor::updateUi, this, &Widget::updateUi);
    connect(&distributor, &Distributor::log, this, &Widget::log);

    distributor.mover.setSourceDir(settings.value("watchDir", "C:/").toString());
    distributor.mover.setTargetDir(settings.value("tempDir", "C:/").toString());

    distributor.checker.setSourceDir(settings.value("tempDir", "C:/").toString());
    distributor.checker.setCleanDir(settings.value("cleanDir", "C:/").toString());
    distributor.checker.setDangerDir(settings.value("dangerDir", "C:/").toString());

    distributor.checker.useKasper(settings.value("useKasper", true).toBool());
    distributor.checker.setKasperFile(settings.value("kasperFilePath", "C:/Program Files (x86)/Kaspersky Lab/Kaspersky Endpoint Security for Windows/avp.com").toString());

    distributor.checker.useDrweb(settings.value("useDrweb", true).toBool());
    distributor.checker.setDrwebFile(settings.value("drwebFilePath", "C:/Program Files/DrWeb/dwscancl.exe").toString());

    distributor.checker.setThreadsNb(settings.value("threadsNb", QThread::idealThreadCount()).toInt());

    connect(&workThread, &QThread::started, &distributor, &Distributor::startWork);

    distributor.moveToThread(&workThread);
    workThread.start();
}

Widget::~Widget() {
    settings.setValue("geometry", saveGeometry());

    settings.setValue("watchDir", distributor.mover.getSourceDir());
    settings.setValue("tempDir", distributor.mover.getTargetDir());

    settings.setValue("cleanDir", distributor.checker.getCleanDir());
    settings.setValue("dangerDir", distributor.checker.getDangerDir());

    settings.setValue("kasperFilePath", distributor.checker.getKasperFile());
    settings.setValue("useKasper", distributor.checker.isKasperUsed());

    settings.setValue("drwebFilePath", distributor.checker.getDrwebFile());
    settings.setValue("useDrweb", distributor.checker.isDrwebUsed());

    settings.setValue("threadsNb", distributor.checker.getThreadsNb());

    workThread.quit();
    workThread.wait();

    delete ui;
}

void Widget::on_watchDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Директория для слежения"), distributor.mover.getSourceDir());
    distributor.mover.setSourceDir(dir);
}

void Widget::on_tempDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Выбор директории для временных файлов"), distributor.mover.getTargetDir());
    distributor.mover.setTargetDir(dir);
}

void Widget::on_cleanDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Выбор директории для чистых файлов"), distributor.checker.getCleanDir());
    distributor.checker.setCleanDir(dir);
}

void Widget::on_dangerousDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Выбор директории для зараженных файлов"), distributor.checker.getDangerDir());
    distributor.checker.setDangerDir(dir);
}

void Widget::on_kasperFileButton_clicked() {
    QString filePath = QFileDialog::getOpenFileName(this, tr("Выбор файла Kaspersky"), "C:/Program Files (x86)/Kasper/Kaspersky Endpoint Security 10 for Windows SP1/avp.com", tr("*.*"));
    distributor.checker.setKasperFile(filePath);
}

void Widget::on_drwebFileButton_clicked() {
    QString filePath = QFileDialog::getOpenFileName(this, tr("Выбор файла Drweb"), "C:/Program Files (x86)", tr("*.*"));
    distributor.checker.setDrwebFile(filePath);
}

void Widget::on_clearButton_clicked() {
    ui->logPTE->clear();
}

void Widget::log(const QString &s) {
    QString out = QTime::currentTime().toString() + QString(4, ' ') + s;
    ui->logPTE->appendPlainText(out);
}

void Widget::updateUi() {
    ui->watchDirLE->setText(distributor.mover.getSourceDir());
    ui->tempDirLE->setText(distributor.mover.getTargetDir());
    ui->cleanDirLE->setText(distributor.checker.getCleanDir());
    ui->dangerousDirLE->setText(distributor.checker.getDangerDir());

    ui->kasperFileLE->setText(distributor.checker.getKasperFile());
    ui->kasperCB->setChecked(distributor.checker.isKasperUsed());
    ui->kasperFileLabel->setEnabled(distributor.checker.isKasperUsed());
    ui->kasperFileLE->setEnabled(distributor.checker.isKasperUsed());
    ui->kasperFileButton->setEnabled(distributor.checker.isKasperUsed());

    ui->drwebFileLE->setText(distributor.checker.getDrwebFile());
    ui->drwebCB->setChecked(distributor.checker.isDrwebUsed());
    ui->drwebFileLabel->setEnabled(distributor.checker.isDrwebUsed());
    ui->drwebFileLE->setEnabled(distributor.checker.isDrwebUsed());
    ui->drwebFileButton->setEnabled(distributor.checker.isDrwebUsed());

    ui->threadControlSB->setValue(distributor.checker.getThreadsNb());

    ui->scanFilesNbLabel->setText(QString::number(distributor.checker.getProcessedFilesNb()));
    ui->summarySizeNbLabel->setText(QString::number(distributor.checker.getProcessedFileSize(), 'f', 4) + " МБ");

    ui->queueSizeLabel->setText(QString::number(distributor.checker.getQueueSize()));
}

void Widget::on_threadControlSB_valueChanged(int _maxThreadsNb) {
    distributor.checker.setThreadsNb(_maxThreadsNb);
}

void Widget::on_kasperCB_clicked(bool isUsed) {
    distributor.checker.useKasper(isUsed);
}

void Widget::on_drwebCB_clicked(bool isUsed) {
    distributor.checker.useDrweb(isUsed);
}
