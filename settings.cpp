#include "settings.h"
#include "ui_settings.h"

Settings::Settings(QWidget *parent, Distributor* distributor, QByteArray geometry, bool visible): QDialog(parent), ui(new Ui::Settings) {
    ui->setupUi(this);

    setLayout(ui->mainLayout);
    setWindowTitle("Настройки программы");
    restoreGeometry(geometry);
    setVisible(visible);

    m_distributor = distributor;
    updateUi();
}

Settings::~Settings() {
    delete ui;
}

void Settings::on_watchDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, QString("Выбор директории для слежения"), m_distributor->getWatchDir());
    m_distributor->setWatchDir(dir);
    updateUi();
}

void Settings::on_tempDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, QString("Выбор директории для временных файлов программы"), m_distributor->getInvestigatorDir());
    m_distributor->setInvestigatorDir(dir);
    updateUi();
}

void Settings::on_cleanDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, QString("Выбор директории для чистых файлов"), m_distributor->getCleanDir());
    m_distributor->setCleanDir(dir);
    updateUi();
}

void Settings::on_dangerousDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, QString("Выбор директории для зараженных файлов"), m_distributor->getDangerDir());
    m_distributor->setDangerDir(dir);
    updateUi();
}

void Settings::on_kasperFileButton_clicked() {
    QString filePath = QFileDialog::getOpenFileName(this, QString("Выбор исполняемого файла антивируса " + getName(AV::KASPER)), m_distributor->getAVFile(AV::KASPER), tr("*.com"));
    m_distributor->setAVFile(AV::KASPER, filePath);
    updateUi();
}

void Settings::on_drwebFileButton_clicked() {
    QString filePath = QFileDialog::getOpenFileName(this, QString("Выбор исполняемого файла антивируса " + getName(AV::DRWEB)), m_distributor->getAVFile(AV::DRWEB), tr("*.exe"));
    m_distributor->setAVFile(AV::DRWEB, filePath);
    updateUi();
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
    ui->kasperMaxQueueSizeSB->setValue(m_distributor->getMaxQueueSize(AV::KASPER));
    ui->kasperMaxQueueVolSB->setValue(m_distributor->getMaxQueueVolMb(AV::KASPER));
    ui->kasperMaxQueueVolUnitCB->setCurrentIndex(m_distributor->getMaxQueueVolUnit(AV::KASPER));

    ui->drwebFileLE->setText(m_distributor->getAVFile(AV::DRWEB));
    ui->drwebCB->setChecked(m_distributor->getAVUse(AV::DRWEB));
    ui->drwebMaxQueueSizeSB->setValue(m_distributor->getMaxQueueSize(AV::DRWEB));
    ui->drwebMaxQueueVolSB->setValue(m_distributor->getMaxQueueVolMb(AV::DRWEB));
    ui->drwebMaxQueueVolUnitCB->setCurrentIndex(m_distributor->getMaxQueueVolUnit(AV::DRWEB));

// enabled
    ui->kasperFileLE->setEnabled(m_distributor->getAVUse(AV::KASPER));
    ui->kasperFileButton->setEnabled(m_distributor->getAVUse(AV::KASPER));
    ui->kasperMaxQueueSizeSB->setEnabled(m_distributor->getAVUse(AV::KASPER));
    ui->kasperMaxQueueVolSB->setEnabled(m_distributor->getAVUse(AV::KASPER));
    ui->kasperMaxQueueVolUnitCB->setEnabled(m_distributor->getAVUse(AV::KASPER));

    ui->drwebFileLE->setEnabled(m_distributor->getAVUse(AV::DRWEB));
    ui->drwebFileButton->setEnabled(m_distributor->getAVUse(AV::DRWEB));
    ui->drwebMaxQueueSizeSB->setEnabled(m_distributor->getAVUse(AV::DRWEB));
    ui->drwebMaxQueueVolSB->setEnabled(m_distributor->getAVUse(AV::DRWEB));
    ui->drwebMaxQueueVolUnitCB->setEnabled(m_distributor->getAVUse(AV::DRWEB));
}

int Settings::getVolUnits(AV av) {
    switch(av) {
        case AV::KASPER:
            return ui->kasperMaxQueueVolUnitCB->currentIndex();
        case AV::DRWEB:
            return ui->drwebMaxQueueVolUnitCB->currentIndex();
        default:
            return 0;
    }
}

void Settings::on_kasperMaxQueueSizeSB_valueChanged(int size) {
    m_distributor->setMaxQueueSize(AV::KASPER, size);
}

void Settings::on_drwebMaxQueueSizeSB_valueChanged(int size) {
    m_distributor->setMaxQueueSize(AV::DRWEB, size);
}

void Settings::on_kasperMaxQueueVolSB_valueChanged(double kasperMaxQueueVolMb) {
    m_distributor->setMaxQueueVol(AV::KASPER, kasperMaxQueueVolMb);
}

void Settings::on_drwebMaxQueueVolSB_valueChanged(double drwebMaxQueueVolMb) {
    m_distributor->setMaxQueueVol(AV::DRWEB, drwebMaxQueueVolMb);
}

void Settings::on_kasperMaxQueueVolUnitCB_currentIndexChanged(int unitIdx) {
    m_distributor->setMaxQueueVolUnit(AV::KASPER, unitIdx);
}

void Settings::on_drwebMaxQueueVolUnitCB_currentIndexChanged(int unitIdx){
    m_distributor->setMaxQueueVolUnit(AV::DRWEB, unitIdx);
}

void Settings::on_clearWatchDirButton_clicked() {
    if( QMessageBox::warning(this,
                             tr("Подтвердите действие"),
                             QString("Вы действительно хотите очистить директорию %1 ?").arg(m_distributor->getWatchDir()),
                             QMessageBox::Yes | QMessageBox::No,
                             QMessageBox::Yes) == QMessageBox::Yes) {
        m_distributor->clearDir(m_distributor->getWatchDir());
    }
}

void Settings::on_clearTempDirButton_clicked() {
    if( QMessageBox::warning(this,
                             tr("Подтвердите действие"),
                             QString("Вы действительно хотите очистить все временные директории?"),
                             QMessageBox::Yes | QMessageBox::No,
                             QMessageBox::Yes) == QMessageBox::Yes) {
        m_distributor->clearDir(m_distributor->getInvestigatorDir() + "/" + KASPER_DIR_NAME + "/" + INPUT_DIR_NAME);
        m_distributor->clearDir(m_distributor->getInvestigatorDir() + "/" + KASPER_DIR_NAME + "/" + OUTPUT_DIR_NAME);

        m_distributor->clearDir(m_distributor->getInvestigatorDir() + "/" + DRWEB_DIR_NAME + "/" + INPUT_DIR_NAME);
        m_distributor->clearDir(m_distributor->getInvestigatorDir() + "/" + DRWEB_DIR_NAME + "/" + OUTPUT_DIR_NAME);

        m_distributor->clearDir(m_distributor->getInvestigatorDir() + "/" + PROCESSED_DIR_NAME);
        m_distributor->clearDir(m_distributor->getInvestigatorDir() + "/" + REPORT_DIR_NAME);
    }
}

void Settings::on_clearCleanDirButton_clicked() {
    if( QMessageBox::warning(this,
                             tr("Подтвердите действие"),
                             QString("Вы действительно хотите очистить директорию %1 ?").arg(m_distributor->getCleanDir()),
                             QMessageBox::Yes | QMessageBox::No,
                             QMessageBox::Yes) == QMessageBox::Yes) {
        m_distributor->clearDir(m_distributor->getCleanDir());
    }
}

void Settings::on_clearDangerDirButton_clicked() {
    if( QMessageBox::warning(this,
                             tr("Подтвердите действие"),
                             QString("Вы действительно хотите очистить директорию %1 ?").arg(m_distributor->getDangerDir()),
                             QMessageBox::Yes | QMessageBox::No,
                             QMessageBox::Yes) == QMessageBox::Yes) {
        m_distributor->clearDir(m_distributor->getDangerDir());
    }
}
