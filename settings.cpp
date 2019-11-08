#include "settings.h"
#include "ui_settings.h"

Settings::Settings(QWidget *parent, Distributor* distributor, QByteArray geometry, bool visible): QDialog(parent), ui(new Ui::Settings) {
    ui->setupUi(this);

    setLayout(ui->mainLayout);
    setWindowTitle("Настройки программы");
    restoreGeometry(geometry);
    setVisible(visible);

    m_distributor = distributor;
}

Settings::~Settings() {
    delete ui;
}

void Settings::on_watchDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, QString("Выбор директории для мониторинга"), m_distributor->getWatchDir());
    m_distributor->setWatchDir(dir);
}

void Settings::on_tempDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, QString("Выбор директории для временных файлов программы"), m_distributor->getInvestigatorDir());
    m_distributor->setInvestigatorDir(dir);
}

void Settings::on_cleanDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, QString("Выбор директории для чистых файлов"), m_distributor->getCleanDir());
    m_distributor->setCleanDir(dir);
}

void Settings::on_dangerousDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, QString("Выбор директории для зараженных файлов"), m_distributor->getDangerDir());
    m_distributor->setDangerDir(dir);
}

void Settings::on_kasperFileButton_clicked() {
    QString filePath = QFileDialog::getOpenFileName(this, QString("Выбор исполняемого файла антивируса " + getName(AV::KASPER)), m_distributor->getAVFile(AV::KASPER), tr("*.com"));
    m_distributor->setAVFile(AV::KASPER, filePath);
}

void Settings::on_drwebFileButton_clicked() {
    QString filePath = QFileDialog::getOpenFileName(this, QString("Выбор исполняемого файла антивируса " + getName(AV::DRWEB)), m_distributor->getAVFile(AV::DRWEB), tr("*.exe"));
    m_distributor->setAVFile(AV::DRWEB, filePath);
}

void Settings::on_kasperCB_clicked(bool isUsed) {
    m_distributor->setAVUse(AV::KASPER, isUsed);
    updateUi();
}

void Settings::on_drwebCB_clicked(bool isUsed) {
    m_distributor->setAVUse(AV::DRWEB, isUsed);
    updateUi();
}

void Settings::updateUi() {
    ui->watchDirLE->setText(m_distributor->getWatchDir());
    ui->investigatorDirLE->setText(m_distributor->getInvestigatorDir());
    ui->cleanDirLE->setText(m_distributor->getCleanDir());
    ui->dangerousDirLE->setText(m_distributor->getDangerDir());

    ui->kasperFileLE->setText(m_distributor->getAVFile(AV::KASPER));
    ui->kasperCB->setChecked(m_distributor->getAVUse(AV::KASPER));
    ui->kasperFileLE->setEnabled(m_distributor->getAVUse(AV::KASPER));
    ui->kasperFileButton->setEnabled(m_distributor->getAVUse(AV::KASPER));

    ui->drwebFileLE->setText(m_distributor->getAVFile(AV::DRWEB));
    ui->drwebCB->setChecked(m_distributor->getAVUse(AV::DRWEB));
    ui->drwebFileLE->setEnabled(m_distributor->getAVUse(AV::DRWEB));
    ui->drwebFileButton->setEnabled(m_distributor->getAVUse(AV::DRWEB));
}
