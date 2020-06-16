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
    if(!QDir(dir).exists()) {
        log(LOG_CATEGORY(GUI + DEBUG), "Не удалось найти директорию для слежения.");
    } else {
        m_investigator->m_watchDir = dir;
        log(LOG_CATEGORY(DEBUG), QString("Изменена директория для наблюдения: %1").arg(dir));
        if(m_investigator->m_isWorking)
            emit restartWatching();
    }

    updateUi();
}

void Settings::on_tempDirButton_clicked() {

    QString dir = QFileDialog::getExistingDirectory(this, QString("Выбор каталога для временных файлов программы"), m_investigator->m_investigatorDir);

    if(dir.isEmpty()) {
        log(LOG_CATEGORY(DEBUG + GUI), "Не удалось найти временную директорию.");
    } else {
        m_investigator->m_investigatorDir = dir;
        m_investigator->configureDirs();
        log(LOG_CATEGORY(DEBUG), QString("Изменена временная директория программы: %1").arg(dir));
    }

    updateUi();
}

void Settings::on_cleanDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    QString("Выбор каталога для чистых файлов"),
                                                    m_investigator->m_cleanDir);

    if(dir.isEmpty()) {
        log(LOG_CATEGORY(DEBUG + GUI), "Не удалось найти директорию для чистых файлов.");
    } else {
        m_investigator->m_cleanDir = dir;
        log(LOG_CATEGORY(DEBUG), QString("Изменена директория для чистых файлов: %1").arg(dir));
    }

    updateUi();
}

void Settings::on_dangerousDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    QString("Выбор каталога для зараженных файлов"),
                                                    m_investigator->m_dangerDir);
    if(dir.isEmpty()) {
        log(LOG_CATEGORY(DEBUG + GUI), "Не удалось найти директорию для зараженных файлов.");
    } else {
        m_investigator->m_dangerDir = dir;
        log(LOG_CATEGORY(DEBUG), QString("Изменена директория для зараженных файлов: %1").arg(dir));
    }

    updateUi();
}

void Settings::on_logsDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    QString("Выбор директории для сохранения логов"),
                                                    m_investigator->m_logsDir);
    if(dir.isEmpty()) {
        log(LOG_CATEGORY(DEBUG + GUI), "Не удалось найти директорию для логов.");
    } else {
        m_investigator->m_logsDir = dir;
        log(LOG_CATEGORY(DEBUG), QString("Изменена директория хранения логов: %1").arg(dir));
    }

    updateUi();
}

void Settings::on_avFileButton_clicked() {
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    QString("Выбор исполняемого файла антивируса"),
                                                    m_investigator->m_avPath, tr("*.exe"));

    if(!QFile(filePath).exists()) {
        log(LOG_CATEGORY(DEBUG + GUI), "Не удалось найти исполняемый файл АВС.");
    } else {
        m_investigator->m_avPath = filePath;
        log(LOG_CATEGORY(DEBUG), QString("Выбран новый исполняемый файл АВС: %1").arg(filePath));
    }

    updateUi();
}

void Settings::updateUi() {

    ui->syslogCB->blockSignals(true);
    ui->syslogLevelCB->blockSignals(true);
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
    ui->avMaxQueueVolSB->setValue(m_investigator->m_maxQueueVol);
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

    ui->syslogCB->blockSignals(false);
    ui->syslogLevelCB->blockSignals(false);
}

int Settings::getVolUnits() {
    return ui->avMaxQueueVolUnitCB->currentIndex();
}

void Settings::on_avMaxQueueSizeSB_valueChanged(int size) {
    m_investigator->m_maxQueueSize = size;
}

void Settings::on_avMaxQueueVolSB_valueChanged(double maxQueueVol) {
    m_investigator->m_maxQueueVol = maxQueueVol;
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
    log(LOG_CATEGORY(DEBUG), QString("Изменен порядок работы с зараженными файлами: %1.")
                                     .arg(m_investigator->m_infectedFileAction));
    updateUi();
}

void Settings::on_externalHandlerFileButton_clicked() {
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    QString("Выбор внешнего обработчика"),
                                                    m_investigator->m_externalHandlerPath, tr("*.*"));

    if(!QFile(filePath).exists()) {
        log(LOG_CATEGORY(DEBUG + GUI), "Внешний обработчик не найден.");
    } else {
        m_investigator->m_externalHandlerPath = filePath;
        log(LOG_CATEGORY(DEBUG), QString("Изменен внешний обработчик: %1.").arg(filePath));
    }

    updateUi();
}

void Settings::on_externalHandlerFileCB_clicked(bool checked) {
    m_investigator->m_useExternalHandler = checked;
    log(LOG_CATEGORY(DEBUG), QString("Изменен флаг использования внешнего обработчика: %1.").arg(checked));
    updateUi();
}

void Settings::on_saveAVSReportsCB_clicked(bool saveState) {
    m_investigator->m_saveAvsReports = saveState;
    log(LOG_CATEGORY(DEBUG), QString("Изменен флаг сохранения отчетов АВС: %1.").arg(saveState));
    updateUi();
}

void Settings::on_reportsDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    QString("Выбор каталога для сохранения отчетов АВС"),
                                                    m_investigator->m_reportsDir);

        if(dir.isEmpty()) {
            log(LOG_CATEGORY(DEBUG + GUI),
                "Не удалось обнаружить директорию для отчетов АВС.");
        } else {
            m_investigator->m_reportsDir = dir;
            log(LOG_CATEGORY(DEBUG), QString("Изменена директория хранения отчетов АВС: %1").arg(dir));
        }

        updateUi();
}

// --- SYSLOG ---

void Settings::on_syslogCB_clicked(bool checked) {
    m_investigator->m_useSyslog = checked;
    m_investigator->m_syslogAddress = ui->syslogAddressLE->text();
    log(LOG_CATEGORY(DEBUG), QString("Изменен флаг использования syslog: %1.").arg(checked));
    updateUi();
}

void Settings::on_syslogLevelCB_currentIndexChanged(int level) {
    m_investigator->m_syslogPriority = ( (m_investigator->m_syslogPriority == DEBUG) ? GUI : DEBUG );
    log(LOG_CATEGORY(DEBUG), QString("Изменен уровень логирования syslog: %1.").arg(level));
    updateUi();
}

void Settings::on_syslogAddressLE_editingFinished() {
    m_investigator->m_syslogAddress = ui->syslogAddressLE->text();
    log(LOG_CATEGORY(DEBUG), QString("Changed address of syslog demon: %1.").arg(m_investigator->m_syslogAddress));
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
    log(LOG_CATEGORY(DEBUG), QString("Изменен флаг использования http сервера: %1.").arg(checked));
    emit startHttpServer();
    updateUi();
}

void Settings::on_httpServerAddressLE_editingFinished() {
    m_investigator->m_httpServerAddress = ui->httpServerAddressLE->text();
    log(LOG_CATEGORY(DEBUG), QString("Изменен адрес http сервера: %1.").arg(m_investigator->m_httpServerAddress));
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
