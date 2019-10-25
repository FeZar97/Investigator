#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent): QWidget(parent), ui(new Ui::Widget), settings("FeZar97", "Investigator") {

    ui->setupUi(this);
    setLayout(ui->mainLayout);
    setWindowTitle("Investigator " + VERSION);

    log(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") + "\tПрограмма запущена");
    setWindowIcon(QIcon(":/investigator.ico"));

    restoreGeometry(settings.value("geometry").toByteArray());

    connect(&distributor, &Distributor::updateUi, this, &Widget::updateUi);
    connect(&distributor, &Distributor::log, this, &Widget::log);

    distributor.setWatchDir(settings.value("watchDir", "C:/").toString());
    distributor.setInvestigatorDir(settings.value("investigatorDir", "C:/").toString());
    distributor.setCleanDir(settings.value("cleanDir", "C:/").toString());
    distributor.setDangerDir(settings.value("dangerDir", "C:/").toString());

    distributor.setUseKasper(settings.value("useKasper", true).toBool());
    distributor.setKasperFile(settings.value("kasperFilePath", "C:/Program Files (x86)/Kaspersky Lab/Kaspersky Endpoint Security for Windows/avp.com").toString());

    distributor.setUseDrweb(settings.value("useDrweb", true).toBool());
    distributor.setDrwebFile(settings.value("drwebFilePath", "C:/Program Files/DrWeb/dwscancl.exe").toString());

    connect(&workThread, &QThread::started, &distributor, &Distributor::startTempDirEye);
    connect(&workThread, &QThread::started, &distributor, &Distributor::startWatchDirEye);

    updateUi();

    distributor.moveToThread(&workThread);
    workThread.start();
}

Widget::~Widget() {
    settings.setValue("geometry", saveGeometry());

    settings.setValue("watchDir", distributor.getWatchDir());
    settings.setValue("investigatorDir", distributor.getInvestigatorDir());
    settings.setValue("cleanDir", distributor.getCleanDir());
    settings.setValue("dangerDir", distributor.getDangerDir());

    settings.setValue("kasperFilePath", distributor.getKasperFile());
    settings.setValue("useKasper", distributor.isKasperUse());

    settings.setValue("drwebFilePath", distributor.getDrwebFile());
    settings.setValue("useDrweb", distributor.isDrwebUse());

    workThread.quit();
    workThread.wait();

    delete ui;
}

void Widget::on_watchDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Директория для слежения"), distributor.getWatchDir());
    distributor.setWatchDir(dir);
}

void Widget::on_tempDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Выбор директории для временных файлов программы"), distributor.getInvestigatorDir());
    distributor.setInvestigatorDir(dir);
}

void Widget::on_cleanDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Выбор директории для чистых файлов"), distributor.getCleanDir());
    distributor.setCleanDir(dir);
}

void Widget::on_dangerousDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Выбор директории для зараженных файлов"), distributor.getDangerDir());
    distributor.setDangerDir(dir);
}

void Widget::on_kasperFileButton_clicked() {
    QString filePath = QFileDialog::getOpenFileName(this, tr("Выбор файла Kaspersky"), distributor.getKasperFile(), tr("*.com"));
    distributor.setKasperFile(filePath);
}

void Widget::on_drwebFileButton_clicked() {
    QString filePath = QFileDialog::getOpenFileName(this, tr("Выбор файла Drweb"), distributor.getDrwebFile(), tr("*.exe"));
    distributor.setDrwebFile(filePath);
}

void Widget::on_clearButton_clicked() {
    ui->logPTE->clear();
}

void Widget::log(const QString &s) {
    //QString out = QTime::currentTime().toString() + QString(4, ' ') + s;
    ui->logPTE->appendPlainText(s);
}

void Widget::updateUi() {
    ui->watchDirLE->setText(distributor.getWatchDir());
    ui->investigatorDirLE->setText(distributor.getInvestigatorDir());
    ui->cleanDirLE->setText(distributor.getCleanDir());
    ui->dangerousDirLE->setText(distributor.getDangerDir());

    ui->kasperFileLE->setText(distributor.getKasperFile());
    ui->kasperCB->setChecked(distributor.isKasperUse());
    ui->kasperFileLE->setEnabled(distributor.isKasperUse());
    ui->kasperFileButton->setEnabled(distributor.isKasperUse());

    ui->drwebFileLE->setText(distributor.getDrwebFile());
    ui->drwebCB->setChecked(distributor.isDrwebUse());
    ui->drwebFileLE->setEnabled(distributor.isDrwebUse());
    ui->drwebFileButton->setEnabled(distributor.isDrwebUse());

    ui->scanFilesNbLabel->setText("Kasper: " + QString::number(distributor.getKasperProcessedFilesNb()) +
                                  " (" +
                                  ((distributor.getKasperProcessedFilesSize() > 1023.) ? QString(QString::number(distributor.getKasperProcessedFilesSize() / 1024., 'f', 4) + " Гб") :
                                                                                         QString(QString::number(distributor.getKasperProcessedFilesSize(), 'f', 4) + " Мб")) +
                                  "),   " +
                                  "DrWeb: " + QString::number(distributor.getDrwebProcessedFilesNb()) +
                                              " (" +
                                              ((distributor.getDrwebProcessedFilesSize() > 1023.) ? QString(QString::number(distributor.getDrwebProcessedFilesSize() / 1024., 'f', 4) + " Гб") :
                                                                                                    QString(QString::number(distributor.getDrwebProcessedFilesSize(), 'f', 4) + " Мб")) +
                                              ")");
    ui->queueSizeLabel->setText(QString::number(distributor.getQueueSize()));
}

void Widget::on_kasperCB_clicked(bool isUsed) {
    distributor.setUseKasper(isUsed);
}

void Widget::on_drwebCB_clicked(bool isUsed) {
    distributor.setUseDrweb(isUsed);
}
