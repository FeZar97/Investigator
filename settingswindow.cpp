#include "settingswindow.h"
#include "ui_settingswindow.h"

SettingsWindow::SettingsWindow(QWidget *parent, InvestigatorOrchestartor *investigator, bool *isUiLocked):
    QDialog(parent),
    ui(new Ui::SettingsWindow) {
    ui->setupUi(this);

    m_isUiLocked = isUiLocked;

    setWindowTitle("Настройки программы");

    ui->threadsNumberSB->setMaximum(MaxThreadNb);

    ui->directoryTab->setLayout(ui->directorySettingsLayout);
    ui->avsTab->setLayout(ui->avsSettingsLayout);
    ui->reactionTab->setLayout(ui->reactionSettingsLayout);

    setMinimumSize(ui->settingsTabWidget->size());
    setMaximumSize(ui->settingsTabWidget->size());

    m_investigator = investigator;

    updateUi();
}

SettingsWindow::~SettingsWindow() {
    delete ui;
}

void SettingsWindow::on_watchDirButton_clicked() {
    m_investigator->setSourceDir(QFileDialog::getExistingDirectory(this, QString("Выбор директории для слежения"), m_investigator->sourceDir()));
    updateUi();
}

void SettingsWindow::on_tempDirButton_clicked() {
    m_investigator->setProcessDir(QFileDialog::getExistingDirectory(this, QString("Директория для временных файлов"), m_investigator->processDir()));
    updateUi();
}

void SettingsWindow::on_cleanDirButton_clicked() {
    m_investigator->setCleanDir(QFileDialog::getExistingDirectory(this, QString("Директория для чистых файлов"), m_investigator->cleanDir()));
    updateUi();
}

void SettingsWindow::on_dangerousDirButton_clicked() {
    m_investigator->setInfectedDir(QFileDialog::getExistingDirectory(this, QString("Директория для зараженных файлов"), m_investigator->infectedDir()));
    updateUi();
}

void SettingsWindow::on_avsExecFileButton_clicked() {
    m_investigator->setAvsExecFileName(QFileDialog::getOpenFileName(this,
                                                                    QString("Выбор исполняемого файла антивируса"),
                                                                    m_investigator->avsExecFileName(), tr("*.exe")));
    updateUi();
}

void SettingsWindow::on_thresholdFilesNbSB_valueChanged(int thresholdFilesNb) {
    m_investigator->setThresholdFilesNb(thresholdFilesNb);
}

void SettingsWindow::on_thresholdFilesSizeSB_valueChanged(int thresholdFilesSize) {
    m_investigator->setThresholdFilesSize(thresholdFilesSize);
}

void SettingsWindow::on_thresholdFilesSizeUnitCB_currentIndexChanged(int thresholdFilesUnitIdx) {
    m_investigator->setThresholdFilesSizeUnit(SizeConverter::sizeIdxToUnit(thresholdFilesUnitIdx));
}

void SettingsWindow::on_threadsNumberSB_valueChanged(int workersNb) {
    m_investigator->setWorkersNb(workersNb);
    updateUi();
}

void SettingsWindow::updateUi() {

    ui->settingsTabWidget->setCurrentIndex(m_currentOpenTab);

    if(m_investigator) {

        // --- DIRS PAGE ---
        ui->watchDirButton->setEnabled(!*m_isUiLocked);
        ui->watchDirLE->setText(m_investigator->sourceDir());
        ui->watchDirLE->setStyleSheet((QDir(m_investigator->sourceDir()).exists() && !m_investigator->sourceDir().isEmpty()) ? Stylehelper::LineEditDefaultStylesheet() : Stylehelper::LineEditIncorrectStylesheet());
        ui->watchDirLE->setEnabled(!*m_isUiLocked);

        ui->tempDirButton->setEnabled(!*m_isUiLocked);
        ui->tempDirLE->setText(m_investigator->processDir());
        ui->tempDirLE->setStyleSheet((QDir(m_investigator->processDir()).exists() && !m_investigator->processDir().isEmpty()) ? Stylehelper::LineEditDefaultStylesheet() : Stylehelper::LineEditIncorrectStylesheet());
        ui->tempDirLE->setEnabled(!*m_isUiLocked);

        ui->cleanDirButton->setEnabled(!*m_isUiLocked);
        ui->cleanDirLE->setText(m_investigator->cleanDir());
        ui->cleanDirLE->setStyleSheet((QDir(m_investigator->cleanDir()).exists() && !m_investigator->cleanDir().isEmpty()) ? Stylehelper::LineEditDefaultStylesheet() : Stylehelper::LineEditIncorrectStylesheet());
        ui->cleanDirLE->setEnabled(!*m_isUiLocked);

        ui->dangerousDirButton->setEnabled(!*m_isUiLocked);
        ui->dangerousDirLE->setText(m_investigator->infectedDir());
        ui->dangerousDirLE->setStyleSheet((QDir(m_investigator->infectedDir()).exists() && !m_investigator->infectedDir().isEmpty()) ? Stylehelper::LineEditDefaultStylesheet() : Stylehelper::LineEditIncorrectStylesheet());
        ui->dangerousDirLE->setEnabled(!*m_isUiLocked);

        // --- AVS PAGE ---
        ui->avsExecFileLE->setEnabled(!*m_isUiLocked && !m_investigator->isInWork());
        ui->avsExecFileLE->setText(m_investigator->avsExecFileName());
        ui->avsExecFileLE->setStyleSheet(QFile(m_investigator->avsExecFileName()).exists() ? Stylehelper::LineEditDefaultStylesheet() : Stylehelper::LineEditIncorrectStylesheet());
        ui->avsExecFileButton->setEnabled(!*m_isUiLocked && !m_investigator->isInWork());

        ui->threadsNumberSB->setEnabled(!*m_isUiLocked && !m_investigator->isInWork());
        ui->threadsNumberSB->setValue(m_investigator->tempCurrentWorkersNb());

        ui->thresholdFilesNbSB->setEnabled(!*m_isUiLocked);
        ui->thresholdFilesNbSB->setValue(m_investigator->thresholdFilesNb());
        ui->thresholdFilesSizeSB->setEnabled(!*m_isUiLocked);
        ui->thresholdFilesSizeSB->setValue(m_investigator->thresholdFilesSize());
        ui->thresholdFilesSizeUnitCB->setEnabled(!*m_isUiLocked);
        ui->thresholdFilesSizeUnitCB->setCurrentIndex(SizeConverter::sizeUnitIdx(m_investigator->thresholdFilesSizeUnit()));

        ui->externalHandlerFileCB->setEnabled(!*m_isUiLocked);
        // ui->externalHandlerFileCB->setChecked(m_investigator->m_useExternalHandler);
        ui->externalHandlerFileLE->setEnabled(!*m_isUiLocked);
        // ui->externalHandlerFileLE->setText(m_investigator->m_externalHandlerPath);
        ui->externalHandlerFileButton->setEnabled(!*m_isUiLocked);

        // --- EVENTS PAGE ---
        ui->syslogAddressLE->setEnabled(!*m_isUiLocked);
        ui->syslogAddressLE->setText(m_investigator->syslogAddress());
        ui->syslogAddressLE->setStyleSheet(!QHostAddress(m_investigator->syslogAddress()).isNull() ? Stylehelper::LineEditDefaultStylesheet() : Stylehelper::LineEditIncorrectStylesheet());

        ui->restartHttpServerButton->setEnabled(!*m_isUiLocked);
    }
}

void SettingsWindow::on_externalHandlerFileButton_clicked() {
    /*
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    QString("Выбор внешнего обработчика"),
                                                    m_investigator->m_externalHandlerPath, tr("*.*"));

    m_investigator->m_externalHandlerPath = filePath;

    updateUi();
    */
}

void SettingsWindow::on_externalHandlerFileCB_clicked(bool checked) {
    Q_UNUSED(checked)
    /*
    m_investigator->m_useExternalHandler = checked;
    updateUi();
    */
}

// --- SYSLOG ---
void SettingsWindow::on_syslogAddressLE_editingFinished() {
    m_investigator->setSyslogAddress(ui->syslogAddressLE->text());
    updateUi();
}

void SettingsWindow::on_settingsTabWidget_currentChanged(int index) {
    m_currentOpenTab = index;
}

// перезапуск http сервера
void SettingsWindow::on_restartHttpServerButton_clicked() {
    emit restartHttpServer();
}
