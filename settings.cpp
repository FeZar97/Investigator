#include "settings.h"
#include "ui_settings.h"

Settings::Settings(QWidget *parent, Investigator* investigator, QByteArray geometry, bool *lockUi): QDialog(parent), ui(new Ui::Settings), m_lockUi(lockUi) {
    ui->setupUi(this);

    setLayout(ui->mainLayout);
    setWindowTitle("Настройки программы");
    restoreGeometry(geometry);
    setVisible(true);
    resize(this->minimumSize());

    m_investigator = investigator;
    updateUi();
}

Settings::~Settings() {
    delete ui;
}

void Settings::on_watchDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, QString("Выбор каталога для слежения"), m_investigator->m_watchDir);
    if(dir.isEmpty()) {
        log("Не найден каталог для слежения.", MSG_CATEGORY(INFO + LOG_GUI));
    } else {
        m_investigator->m_watchDir = dir;
        log(QString("Выбран новый каталог для слежения: %1").arg(dir), MSG_CATEGORY(INFO));
    }

    updateUi();
}

void Settings::on_tempDirButton_clicked() {

    QString dir = QFileDialog::getExistingDirectory(this, QString("Выбор каталога для временных файлов программы"), m_investigator->m_investigatorDir);

    if(dir.isEmpty()) {
        log("Не найден каталог для временных файлов.", MSG_CATEGORY(INFO + LOG_GUI));
    } else {
        m_investigator->m_investigatorDir = dir;
        m_investigator->configureDirs();
        log(QString("Выбран новый каталог для временных файлов программы: %1").arg(dir), MSG_CATEGORY(INFO));
    }

    updateUi();
}

void Settings::on_cleanDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, QString("Выбор каталога для чистых файлов"), m_investigator->m_cleanDir);

    if(dir.isEmpty()) {
        log("Не найден каталог для чистых файлов.", MSG_CATEGORY(INFO + LOG_GUI));
    } else {
        m_investigator->m_cleanDir = dir;
        log(QString("Выбран новый каталог для чистых файлов: %1").arg(dir), MSG_CATEGORY(INFO));
    }

    updateUi();
}

void Settings::on_dangerousDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, QString("Выбор каталога для зараженных файлов"), m_investigator->m_dangerDir);
    if(m_investigator->m_dangerDir.isEmpty()) {
        log("Не найден каталог для зараженных файлов.", MSG_CATEGORY(INFO + LOG_GUI));
    } else {
        m_investigator->m_dangerDir = dir;
        log(QString("Выбран новый каталог для зараженных файлов: %1").arg(dir), MSG_CATEGORY(INFO));
    }

    updateUi();
}

void Settings::on_avFileButton_clicked() {
    QString filePath = QFileDialog::getOpenFileName(this, QString("Выбор исполняемого файла антивируса"), m_investigator->m_avPath, tr("*.exe"));

    if(!QFile(filePath).exists()) {
        log("Не найден исполняемый файл АВС.", MSG_CATEGORY(INFO + LOG_GUI));
    } else {
        m_investigator->m_avPath = filePath;
        log(QString("Выбран новый исполняемый файл АВС: %1").arg(filePath), MSG_CATEGORY(INFO));
    }

    updateUi();
}

void Settings::updateUi() {

    // ---
    ui->watchDirLE->setEnabled(!*m_lockUi);
    ui->watchDirButton->setEnabled(!*m_lockUi);
    ui->clearWatchDirButton->setEnabled(!*m_lockUi);

    ui->tempDirLE->setEnabled(!*m_lockUi);
    ui->tempDirButton->setEnabled(!*m_lockUi);
    ui->clearTempDirButton->setEnabled(!*m_lockUi);

    ui->cleanDirLE->setEnabled(!*m_lockUi);
    ui->cleanDirButton->setEnabled(!*m_lockUi);
    ui->clearCleanDirButton->setEnabled(!*m_lockUi);

    ui->dangerousDirLE->setEnabled(!*m_lockUi);
    ui->dangerousDirButton->setEnabled(!*m_lockUi);
    ui->clearDangerDirButton->setEnabled(!*m_lockUi);

    ui->avFileLE->setEnabled(!*m_lockUi);
    ui->avFileButton->setEnabled(!*m_lockUi);
    ui->infectActionCB->setEnabled(!*m_lockUi);
    ui->avMaxQueueSizeSB->setEnabled(!*m_lockUi);
    ui->avMaxQueueVolSB->setEnabled(!*m_lockUi);
    ui->avMaxQueueVolUnitCB->setEnabled(!*m_lockUi);
    ui->externalHandlerFileCB->setEnabled(!*m_lockUi);
    ui->externalHandlerFileLE->setEnabled(!*m_lockUi);
    ui->externalHandlerFileButton->setEnabled(!*m_lockUi);
    ui->syslogCB->setEnabled(!*m_lockUi);
    ui->syslogAddressLE->setEnabled(!*m_lockUi);

    // ---
    ui->watchDirLE->setText(m_investigator->m_watchDir);
    ui->tempDirLE->setText(m_investigator->m_investigatorDir);
    ui->cleanDirLE->setText(m_investigator->m_cleanDir);
    ui->dangerousDirLE->setText(m_investigator->m_dangerDir);

    ui->avFileLE->setText(m_investigator->m_avPath);
    ui->avMaxQueueSizeSB->setValue(m_investigator->m_maxQueueSize);
    ui->avMaxQueueVolSB->setValue(m_investigator->m_maxQueueVolMb);
    ui->avMaxQueueVolUnitCB->setCurrentIndex(m_investigator->m_maxQueueVolUnit);

    ui->infectActionCB->setCurrentIndex(m_investigator->m_infectedFileAction);

    ui->externalHandlerFileCB->setChecked(m_investigator->m_useExternalHandler);
    ui->externalHandlerFileLE->setText(m_investigator->m_externalHandlerPath);
    ui->externalHandlerFileLE->setEnabled(m_investigator->m_useExternalHandler);
    ui->externalHandlerFileButton->setEnabled(m_investigator->m_useExternalHandler);

    ui->syslogCB->setChecked(m_investigator->m_useSyslog);
    ui->syslogAddressLE->setEnabled(m_investigator->m_useSyslog);
    ui->syslogAddressLE->setText(m_investigator->m_syslogAddress);

    // --- STYLESHEETS ---
    ui->watchDirLE->setStyleSheet(QDir(m_investigator->m_watchDir).exists()                        ? Stylehelper::defaultLEStylesheet() : Stylehelper::incorrectLEStylesheet());
    ui->tempDirLE->setStyleSheet(QDir(m_investigator->m_investigatorDir).exists()                  ? Stylehelper::defaultLEStylesheet() : Stylehelper::incorrectLEStylesheet());
    ui->cleanDirLE->setStyleSheet(QDir(m_investigator->m_cleanDir).exists()                        ? Stylehelper::defaultLEStylesheet() : Stylehelper::incorrectLEStylesheet());
    ui->dangerousDirLE->setStyleSheet(QDir(m_investigator->m_dangerDir).exists()                   ? Stylehelper::defaultLEStylesheet() : Stylehelper::incorrectLEStylesheet());

    ui->avFileLE->setStyleSheet(QFile(m_investigator->m_avPath).exists()                           ? Stylehelper::defaultLEStylesheet() : Stylehelper::incorrectLEStylesheet());

    if(m_investigator->m_useExternalHandler)
        ui->externalHandlerFileLE->setStyleSheet(QFile(m_investigator->m_externalHandlerPath).exists() ? Stylehelper::defaultLEStylesheet() : Stylehelper::incorrectLEStylesheet());
    else
        ui->externalHandlerFileLE->setStyleSheet("");

    if(m_investigator->m_useSyslog)
        ui->syslogAddressLE->setStyleSheet(m_investigator->checkSyslogAddress() ? Stylehelper::defaultLEStylesheet() : Stylehelper::incorrectLEStylesheet());
    else
        ui->syslogAddressLE->setStyleSheet("");

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

void Settings::on_clearWatchDirButton_clicked() {
    if(QMessageBox::warning(this,
                            tr("Подтвердите действие"),
                            QString("Вы действительно хотите очистить каталог %1 ?").arg(m_investigator->m_watchDir),
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::Yes) == QMessageBox::Yes) {
        log(QString("Очистка каталога для слежения: %1.").arg(m_investigator->m_watchDir), MSG_CATEGORY(INFO + LOG_ROW));
        emit clearDir(m_investigator->m_watchDir);
    }
}

void Settings::on_clearTempDirButton_clicked() {
    if( QMessageBox::warning(this,
                             tr("Подтвердите действие"),
                             QString("Вы действительно хотите очистить все временные каталоги программы?"),
                             QMessageBox::Yes | QMessageBox::No,
                             QMessageBox::Yes) == QMessageBox::Yes) {

        log(QString("Очистка временных каталогов программы."), MSG_CATEGORY(INFO + LOG_ROW));
        emit clearDir(m_investigator->m_inputDir);
        emit clearDir(m_investigator->m_processDir);
    }
}

void Settings::on_clearCleanDirButton_clicked() {
    if( QMessageBox::warning(this,
                             tr("Подтвердите действие"),
                             QString("Вы действительно хотите очистить каталог %1 ?").arg(m_investigator->m_cleanDir),
                             QMessageBox::Yes | QMessageBox::No,
                             QMessageBox::Yes) == QMessageBox::Yes) {
        log(QString("Очистка каталога для чистых файлов: %1.").arg(m_investigator->m_cleanDir), MSG_CATEGORY(INFO + LOG_ROW));
        emit clearDir(m_investigator->m_cleanDir);
    }
}

void Settings::on_clearDangerDirButton_clicked() {
    if(QMessageBox::warning(this,
                             tr("Подтвердите действие"),
                             QString("Вы действительно хотите очистить каталог %1 ?").arg(m_investigator->m_dangerDir),
                             QMessageBox::Yes | QMessageBox::No,
                             QMessageBox::Yes) == QMessageBox::Yes) {
        log(QString("Очистка каталога для зараженных файлов: %1.").arg(m_investigator->m_cleanDir), MSG_CATEGORY(INFO + LOG_ROW));
        emit clearDir(m_investigator->m_dangerDir);
    }
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
    log(QString("Изменена обработка зараженных файлов: %1.").arg(m_investigator->m_infectedFileAction), MSG_CATEGORY(INFO));
    updateUi();
}

void Settings::on_externalHandlerFileButton_clicked() {
    QString filePath = QFileDialog::getOpenFileName(this, QString("Выбор внешнего обработчика"), m_investigator->m_externalHandlerPath, tr("*.*"));

    if(!QFile(filePath).exists()) {
        log("Не найден файл внешнего обработчика.", MSG_CATEGORY(INFO + LOG_GUI));
    } else {
        m_investigator->m_externalHandlerPath = filePath;
        log(QString("Изменен путь к внешнему обработчику зараженных файлов: %1.").arg(filePath), MSG_CATEGORY(INFO));
    }

    updateUi();
}

void Settings::on_externalHandlerFileCB_clicked(bool checked) {
    m_investigator->m_useExternalHandler = checked;
    log(QString("Смена состояния флага использования внешнего обработчика: %1.").arg(checked), MSG_CATEGORY(INFO));
    updateUi();
}

void Settings::on_syslogCB_clicked(bool checked) {
    m_investigator->m_useSyslog = checked;
    m_investigator->m_syslogAddress = ui->syslogAddressLE->text();
    log(QString("Смена состояния флага использования syslog: %1.").arg(checked), MSG_CATEGORY(INFO));
    updateUi();
}

void Settings::on_syslogAddressLE_textChanged(const QString &newAddr) {
    m_investigator->m_syslogAddress = newAddr;
    log(QString("Изменен URL демона syslog: %1.").arg(newAddr), MSG_CATEGORY(INFO));
    updateUi();
}
