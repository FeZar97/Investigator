#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent): QWidget(parent), ui(new Ui::Widget), settings("FeZar97", "Investigator") {

    ui->setupUi(this);
    setLayout(ui->mainLayout);
    setWindowTitle("Investigator " + VERSION);

    log(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss") + " " + "Программа запущена");
    setWindowIcon(QIcon(":/investigator.ico"));
    restoreGeometry(settings.value("geometry").toByteArray());

    connect(&distributor, &Distributor::updateUi, this, &Widget::updateUi);
    connect(&distributor, &Distributor::log, this, &Widget::log);

    distributor.setWatchDir(settings.value("watchDir", "C:/").toString());
    distributor.setInvestigatorDir(settings.value("investigatorDir", "C:/").toString());
    distributor.setCleanDir(settings.value("cleanDir", "C:/").toString());
    distributor.setDangerDir(settings.value("dangerDir", "C:/").toString());

    distributor.setAVUse(AV::KASPER, settings.value("useKasper", true).toBool());
    distributor.setAVFile(AV::KASPER, settings.value("kasperFilePath", "C:/Program Files (x86)/Kaspersky Lab/Kaspersky Endpoint Security for Windows/avp.com").toString());

    distributor.setAVUse(AV::DRWEB, settings.value("useDrweb", true).toBool());
    distributor.setAVFile(AV::DRWEB, settings.value("drwebFilePath", "C:/Program Files/DrWeb/dwscancl.exe").toString());

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

    settings.setValue("kasperFilePath", distributor.getAVFile(AV::KASPER));
    settings.setValue("useKasper", distributor.getAVUse(AV::KASPER));

    settings.setValue("drwebFilePath", distributor.getAVFile(AV::DRWEB));
    settings.setValue("useDrweb", distributor.getAVUse(AV::DRWEB));

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
    QString filePath = QFileDialog::getOpenFileName(this, tr("Выбор исполняемого файла Kaspersky"), distributor.getAVFile(AV::KASPER), tr("*.com"));
    distributor.setAVFile(AV::KASPER, filePath);
}

void Widget::on_drwebFileButton_clicked() {
    QString filePath = QFileDialog::getOpenFileName(this, tr("Выбор исполняемого файла Drweb"), distributor.getAVFile(AV::DRWEB), tr("*.exe"));
    distributor.setAVFile(AV::DRWEB, filePath);
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

    ui->kasperFileLE->setText(distributor.getAVFile(AV::KASPER));
    ui->kasperCB->setChecked(distributor.getAVUse(AV::KASPER));
    ui->kasperFileLE->setEnabled(distributor.getAVUse(AV::KASPER));
    ui->kasperFileButton->setEnabled(distributor.getAVUse(AV::KASPER));

    ui->drwebFileLE->setText(distributor.getAVFile(AV::DRWEB));
    ui->drwebCB->setChecked(distributor.getAVUse(AV::DRWEB));
    ui->drwebFileLE->setEnabled(distributor.getAVUse(AV::DRWEB));
    ui->drwebFileButton->setEnabled(distributor.getAVUse(AV::DRWEB));

    ui->scanFilesNbLabel->setText("Kasper: " + QString::number(distributor.getAVProcessedFilesNb(AV::KASPER)) +
                                  " (" +
                                  ((distributor.getAVProcessedFilesSize(AV::KASPER) > 1023.) ? QString(QString::number(distributor.getAVProcessedFilesSize(AV::KASPER) / 1024., 'f', 4) + " Гб") :
                                                                                         QString(QString::number(distributor.getAVProcessedFilesSize(AV::KASPER), 'f', 4) + " Мб")) +
                                  "),   " +

                                  "DrWeb: " + QString::number(distributor.getAVProcessedFilesNb(AV::DRWEB)) +
                                  " (" +
                                  ((distributor.getAVProcessedFilesSize(AV::DRWEB) > 1023.) ? QString(QString::number(distributor.getAVProcessedFilesSize(AV::DRWEB) / 1024., 'f', 4) + " Гб") :
                                                                                        QString(QString::number(distributor.getAVProcessedFilesSize(AV::DRWEB), 'f', 4) + " Мб")) +
                                  ")");

    ui->queueSizeLabel->setText("Kasper: " + QString::number(distributor.getAVInprogressFilesNb(AV::KASPER)) + ", " +
                                "DrWeb: " + QString::number(distributor.getAVInprogressFilesNb(AV::DRWEB)));
}

void Widget::on_kasperCB_clicked(bool isUsed) {
    distributor.setAVUse(AV::KASPER, isUsed);
}

void Widget::on_drwebCB_clicked(bool isUsed) {
    distributor.setAVUse(AV::DRWEB, isUsed);
}
