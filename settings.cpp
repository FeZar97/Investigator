#include "settings.h"
#include "ui_settings.h"

Settings::Settings(QWidget *parent, Investigator* investigator, QByteArray geometry, bool *lockUi, int currentTabIdx): QDialog(parent), ui(new Ui::Settings), m_lockUi(lockUi) {
    ui->setupUi(this);

    setWindowTitle("Настройки программы");
    restoreGeometry(geometry);

    ui->directoryTab->setLayout(ui->directorySettingsLayout);
    ui->avsTab->setLayout(ui->avsSettingsLayout);
    ui->reactionTab->setLayout(ui->reactionSettingsLayout);

    setMinimumSize(ui->settingsTabWidget->size());
    setMaximumSize(ui->settingsTabWidget->size());

    ui->settingsTabWidget->setCurrentIndex(currentTabIdx);

    m_investigator = investigator;
    updateUi();
}

Settings::~Settings() {
    delete ui;
}

void Settings::on_watchDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, QString("Выбор каталога для слежения"), m_investigator->m_watchDir);
    if(dir.isEmpty()) {
        log("Can't find watching directory.", MSG_CATEGORY(LOG_GUI + DEBUG));
    } else {
        m_investigator->m_watchDir = dir;
        log(QString("Changed watch dir: %1").arg(dir), MSG_CATEGORY(DEBUG));
        emit restartWatching();
    }

    updateUi();
}

void Settings::on_tempDirButton_clicked() {

    QString dir = QFileDialog::getExistingDirectory(this, QString("Выбор каталога для временных файлов программы"), m_investigator->m_investigatorDir);

    if(dir.isEmpty()) {
        log("Can't find temp directory.", MSG_CATEGORY(DEBUG + LOG_GUI));
    } else {
        m_investigator->m_investigatorDir = dir;
        m_investigator->configureDirs();
        log(QString("Changed temp directory: %1").arg(dir), MSG_CATEGORY(DEBUG));
    }

    updateUi();
}

void Settings::on_cleanDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, QString("Выбор каталога для чистых файлов"), m_investigator->m_cleanDir);

    if(dir.isEmpty()) {
        log("Can't find directory for clean files.", MSG_CATEGORY(DEBUG + LOG_GUI));
    } else {
        m_investigator->m_cleanDir = dir;
        log(QString("Changed directory for clean files: %1").arg(dir), MSG_CATEGORY(DEBUG));
    }

    updateUi();
}

void Settings::on_dangerousDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, QString("Выбор каталога для зараженных файлов"), m_investigator->m_dangerDir);
    if(dir.isEmpty()) {
        log("Can't find directory for infected files.", MSG_CATEGORY(DEBUG + LOG_GUI));
    } else {
        m_investigator->m_dangerDir = dir;
        log(QString("Changed directory for infected files: %1").arg(dir), MSG_CATEGORY(DEBUG));
    }

    updateUi();
}

void Settings::on_logsDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, QString("Выбор каталога для сохранения логов"), m_investigator->m_logsDir);
    if(dir.isEmpty()) {
        log("Can't find directory for log files.", MSG_CATEGORY(DEBUG + LOG_GUI));
    } else {
        m_investigator->m_logsDir = dir;
        log(QString("Changed directory for log files: %1").arg(dir), MSG_CATEGORY(DEBUG));
    }

    updateUi();
}

void Settings::on_avFileButton_clicked() {
    QString filePath = QFileDialog::getOpenFileName(this, QString("Выбор исполняемого файла антивируса"), m_investigator->m_avPath, tr("*.exe"));

    if(!QFile(filePath).exists()) {
        log("Can't find AVS executable file.", MSG_CATEGORY(DEBUG + LOG_GUI));
    } else {
        m_investigator->m_avPath = filePath;
        log(QString("Changed executable AVS file: %1").arg(filePath), MSG_CATEGORY(DEBUG));
    }

    updateUi();
}

void Settings::updateUi() {

// --- ENABLES ---
    // --- DIRS PAGE ---
    ui->watchDirLE->setEnabled(!*m_lockUi);
    ui->watchDirButton->setEnabled(!*m_lockUi);

    ui->tempDirLE->setEnabled(!*m_lockUi);
    ui->tempDirButton->setEnabled(!*m_lockUi);

    ui->cleanDirLE->setEnabled(!*m_lockUi);
    ui->cleanDirButton->setEnabled(!*m_lockUi);

    ui->dangerousDirLE->setEnabled(!*m_lockUi);
    ui->dangerousDirButton->setEnabled(!*m_lockUi);

    ui->logsDirLE->setEnabled(!*m_lockUi);
    ui->logsDirButton->setEnabled(!*m_lockUi);

    // --- AVS PAGE ---
    ui->avFileLE->setEnabled(!*m_lockUi);
    ui->avFileButton->setEnabled(!*m_lockUi);

    ui->infectActionCB->setEnabled(!*m_lockUi);

    ui->avMaxQueueSizeSB->setEnabled(!*m_lockUi);
    ui->avMaxQueueVolSB->setEnabled(!*m_lockUi);
    ui->avMaxQueueVolUnitCB->setEnabled(!*m_lockUi);

    ui->saveAVSReportsCB->setEnabled(!*m_lockUi);
    ui->reportsDirLE->setEnabled(!*m_lockUi && m_investigator->m_saveAvsReports);
    ui->reportsDirButton->setEnabled(!*m_lockUi && m_investigator->m_saveAvsReports);

    ui->externalHandlerFileCB->setEnabled(!*m_lockUi);
    ui->externalHandlerFileLE->setEnabled(!*m_lockUi && m_investigator->m_useExternalHandler);
    ui->externalHandlerFileButton->setEnabled(!*m_lockUi && m_investigator->m_useExternalHandler);

    // --- EVENTS PAGE ---
    ui->syslogCB->setEnabled(!*m_lockUi);
    ui->syslogAddressLE->setEnabled(!*m_lockUi && m_investigator->m_useSyslog);
    ui->syslogLevelCB->setEnabled(!*m_lockUi && m_investigator->m_useSyslog);

    ui->httpServerCB->setEnabled(!*m_lockUi);
    ui->httpServerAddressLE->setEnabled(!*m_lockUi && m_investigator->m_useHttpServer);

// --- VALUES ---
    // --- DIRS PAGE ---
    ui->watchDirLE->setText(m_investigator->m_watchDir);
    ui->tempDirLE->setText(m_investigator->m_investigatorDir);
    ui->cleanDirLE->setText(m_investigator->m_cleanDir);
    ui->dangerousDirLE->setText(m_investigator->m_dangerDir);
    ui->logsDirLE->setText(m_investigator->m_logsDir);

    // --- AVS PAGE ---
    ui->avFileLE->setText(m_investigator->m_avPath);

    ui->infectActionCB->setCurrentIndex(m_investigator->m_infectedFileAction);

    ui->avMaxQueueSizeSB->setValue(m_investigator->m_maxQueueSize);
    ui->avMaxQueueVolSB->setValue(m_investigator->m_maxQueueVolMb);
    ui->avMaxQueueVolUnitCB->setCurrentIndex(m_investigator->m_maxQueueVolUnit);

    ui->saveAVSReportsCB->setChecked(m_investigator->m_saveAvsReports);
    ui->reportsDirLE->setText(m_investigator->m_reportsDir);

    ui->externalHandlerFileCB->setChecked(m_investigator->m_useExternalHandler);
    ui->externalHandlerFileLE->setText(m_investigator->m_externalHandlerPath);

    // --- EVENTS PAGE ---
    ui->syslogCB->setChecked(m_investigator->m_useSyslog);
    ui->syslogAddressLE->setText(m_investigator->m_syslogAddress);
    ui->syslogLevelCB->setCurrentIndex(m_investigator->m_syslogPriority - 1);

    ui->httpServerCB->setChecked(m_investigator->m_useHttpServer);
    ui->httpServerAddressLE->setText(m_investigator->m_httpServerAddress);

// --- STYLESHEETS ---
    // --- DIRS PAGE ---
    ui->watchDirLE->setStyleSheet(getLEStyleSheet(QDir(m_investigator->m_watchDir).exists() && !m_investigator->m_watchDir.isEmpty()));
    ui->tempDirLE->setStyleSheet(getLEStyleSheet(QDir(m_investigator->m_investigatorDir).exists() && !m_investigator->m_investigatorDir.isEmpty()));
    ui->cleanDirLE->setStyleSheet(getLEStyleSheet(QDir(m_investigator->m_cleanDir).exists() && !m_investigator->m_cleanDir.isEmpty()));
    ui->dangerousDirLE->setStyleSheet(getLEStyleSheet(QDir(m_investigator->m_dangerDir).exists() && !m_investigator->m_dangerDir.isEmpty()));
    ui->logsDirLE->setStyleSheet(getLEStyleSheet(QDir(m_investigator->m_logsDir).exists() && !m_investigator->m_logsDir.isEmpty()));

    // --- AVS PAGE ---
    ui->avFileLE->setStyleSheet(getLEStyleSheet(QFile(m_investigator->m_avPath).exists()));
    ui->reportsDirLE->setStyleSheet(m_investigator->m_saveAvsReports ? getLEStyleSheet(QDir(m_investigator->m_reportsDir).exists() && !m_investigator->m_reportsDir.isEmpty()) : "");
    ui->externalHandlerFileLE->setStyleSheet(m_investigator->m_useExternalHandler ? getLEStyleSheet(QFile(m_investigator->m_externalHandlerPath).exists()) : "");

    // --- EVENTS PAGE ---
    ui->syslogAddressLE->setStyleSheet(m_investigator->m_useSyslog ? getLEStyleSheet(m_investigator->checkSyslogAddress()) : "");
    ui->httpServerAddressLE->setStyleSheet(m_investigator->m_useHttpServer ? getLEStyleSheet(m_investigator->checkHttpAddress()) : "");
}

int Settings::getVolUnits() {
    return ui->avMaxQueueVolUnitCB->currentIndex();
}

void Settings::on_avMaxQueueSizeSB_valueChanged(int size) {
    m_investigator->m_maxQueueSize = size;
}

void Settings::on_avMaxQueueVolSB_valueChanged(double maxQueueVolMb) {
    m_investigator->m_maxQueueVolMb = maxQueueVolMb;
    emit s_updateUi();
}

void Settings::on_avMaxQueueVolUnitCB_currentIndexChanged(int unitIdx) {
    m_investigator->m_maxQueueVolUnit = unitIdx;
    emit s_updateUi();
}

void Settings::on_infectActionCB_currentIndexChanged(int actionIdx) {
    m_investigator->m_infectedFileAction = ACTION_TYPE(actionIdx);
    if(m_investigator->m_infectedFileAction == DELETE) {
        m_investigator->m_useExternalHandler = false;
        ui->externalHandlerFileCB->setEnabled(false);
    } else {
        m_investigator->m_useExternalHandler = ui->externalHandlerFileCB->isChecked();
        ui->externalHandlerFileCB->setEnabled(true);
    }
    log(QString("Changed action type with infected files: %1.").arg(m_investigator->m_infectedFileAction), MSG_CATEGORY(DEBUG));
    updateUi();
}

void Settings::on_externalHandlerFileButton_clicked() {
    QString filePath = QFileDialog::getOpenFileName(this, QString("Выбор внешнего обработчика"), m_investigator->m_externalHandlerPath, tr("*.*"));

    if(!QFile(filePath).exists()) {
        log("Can't find external handler for infected files.", MSG_CATEGORY(DEBUG + LOG_GUI));
    } else {
        m_investigator->m_externalHandlerPath = filePath;
        log(QString("Changed path to extrnal handler: %1.").arg(filePath), MSG_CATEGORY(DEBUG));
    }

    updateUi();
}

void Settings::on_externalHandlerFileCB_clicked(bool checked) {
    m_investigator->m_useExternalHandler = checked;
    log(QString("Change state of using external handler for infected files: %1.").arg(checked), MSG_CATEGORY(DEBUG));
    updateUi();
}

void Settings::on_saveAVSReportsCB_clicked(bool saveState) {
    m_investigator->m_saveAvsReports = saveState;
    log(QString("AVS reports saving flag has been changed: %1.").arg(saveState), MSG_CATEGORY(DEBUG));
    updateUi();
}

void Settings::on_reportsDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, QString("Выбор каталога для сохранения отчетов АВС"), m_investigator->m_reportsDir);
        if(dir.isEmpty()) {
            log("Can't find directory for report files.", MSG_CATEGORY(DEBUG + LOG_GUI));
        } else {
            m_investigator->m_reportsDir = dir;
            log(QString("Changed directory for report files: %1").arg(dir), MSG_CATEGORY(DEBUG));
        }

        updateUi();
}

// --- SYSLOG ---

void Settings::on_syslogCB_clicked(bool checked) {
    m_investigator->m_useSyslog = checked;
    m_investigator->m_syslogAddress = ui->syslogAddressLE->text();
    log(QString("Change state of syslog using: %1.").arg(checked), MSG_CATEGORY(DEBUG));
    updateUi();
}

void Settings::on_syslogLevelCB_currentIndexChanged(int level) {
    level += 1;
    m_investigator->m_syslogPriority = MSG_CATEGORY(level);
    log(QString("Syslog priority level has been changed: %1.").arg(level), MSG_CATEGORY(DEBUG));
    updateUi();
}

void Settings::on_syslogAddressLE_editingFinished() {
    m_investigator->m_syslogAddress = ui->syslogAddressLE->text();
    log(QString("Changed address of syslog demon: %1.").arg(m_investigator->m_syslogAddress), MSG_CATEGORY(DEBUG));
    updateUi();
}

void Settings::on_syslogAddressLE_textChanged(const QString &newAddress) {
    Q_UNUSED(newAddress)
    ui->syslogAddressLE->setStyleSheet(Stylehelper::changedLEStylesheet());
}

// --- HTTP SERVER ---

void Settings::on_httpServerCB_clicked(bool checked) {
    m_investigator->m_useHttpServer = checked;
    m_investigator->m_httpServerAddress = ui->httpServerAddressLE->text();
    log(QString("Change state of http using: %1.").arg(checked), MSG_CATEGORY(DEBUG));
    emit startHttpServer();
    updateUi();
}

void Settings::on_httpServerAddressLE_editingFinished() {
    m_investigator->m_httpServerAddress = ui->httpServerAddressLE->text();
    log(QString("Changed address of http address: %1.").arg(m_investigator->m_httpServerAddress), MSG_CATEGORY(DEBUG));
    emit startHttpServer();
    updateUi();
}

void Settings::on_httpServerAddressLE_textChanged(const QString &newAddress) {
    Q_UNUSED(newAddress)
    ui->httpServerAddressLE->setStyleSheet(Stylehelper::changedLEStylesheet());
}

QString Settings::getLEStyleSheet(bool isCorrect) {
    return isCorrect ? Stylehelper::defaultLEStylesheet() : Stylehelper::incorrectLEStylesheet();
}

void Settings::on_settingsTabWidget_currentChanged(int index) {
    m_currentTab = index;
}
