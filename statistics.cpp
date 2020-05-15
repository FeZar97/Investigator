#include "statistics.h"
#include "ui_statistics.h"

Statistics::Statistics(QWidget *parent, Investigator* investigator, QByteArray geometry, bool *lockUi): QDialog(parent), ui(new Ui::Statistics), m_lockUi(lockUi) {

    ui->setupUi(this);

    setWindowTitle("Статистика работы");
    restoreGeometry(geometry);

    setLayout(ui->mainLayout);

    m_investigator = investigator;
    m_workTimer.setInterval(500);

    connect(&m_workTimer, &QTimer::timeout, this, &Statistics::updateUi);
    m_workTimer.start();

    updateUi();
}

Statistics::~Statistics() {
    delete ui;
}

void Statistics::updateUi() {

    ui->clearButton->setEnabled(!*m_lockUi);

    ui->InfoLabel->setText(m_investigator->m_processInfo);

    m_model.setHorizontalHeaderLabels(QStringList() << "M-52");
    m_model.setVerticalHeaderLabels(QStringList() << "Обнаружено зараженных файлов"          << "Просканировано файлов"
                                                  << "Объем\nпросканированных файлов"        << "Средняя скорость\nсканирования (МБ/с)"
                                                  << "Текущая скорость\nсканирования (МБ/с)" << "Файлов в обработке"
                                                  << "Файлов в очереди"                      << "Версия АВС");

    ui->tableView->setModel(&m_model);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setSelectionMode(QAbstractItemView::NoSelection);

    qint64 days;
    if( QDate(m_investigator->m_startTime.date()).daysTo(m_investigator->getEndTime().date()) == 0 ) {
        days = 0;
    } else {
        if(m_investigator->getEndTime().time() >= m_investigator->m_startTime.time()) {
            days = QDate(m_investigator->m_startTime.date()).daysTo(m_investigator->getEndTime().date());
        } else {
            days = QDate(m_investigator->m_startTime.date()).daysTo(m_investigator->getEndTime().date()) - 1;
        }
    }

    m_investigator->m_workTimeInSec = QDateTime(m_investigator->m_startTime).secsTo(m_investigator->getEndTime());
    m_investigator->m_workTime = QString("%1 дней ").arg( days ) +
                                 QTime(0,0,0,0).addSecs(m_investigator->m_workTimeInSec).toString("hh ч. mm мин. ss сек.");
    m_investigator->m_workTimeEn = QString("%1 days ").arg( days ) +
                                 QTime(0,0,0,0).addSecs(m_investigator->m_workTimeInSec).toString("hh 'h.' mm 'min.' ss 'sec.'");

    ui->workTimeInfoLabel->setText(m_investigator->m_workTime);

    // обнаружено зараженных файлов
    m_model.setData(m_model.index(0,0), m_investigator->m_infectedFilesNb, Qt::DisplayRole);

    // просканировано файлов
    m_model.setData(m_model.index(1,0), m_investigator->m_processedFilesNb, Qt::DisplayRole);

    // объем просканированных файлов
    m_model.setData(m_model.index(2,0), volumeToString(m_investigator->m_processedFilesSizeMb), Qt::DisplayRole);

    // средняя скорость сканирования
    m_model.setData(m_model.index(3,0), QString::number(m_investigator->m_averageProcessSpeed, 'f', 2), Qt::DisplayRole);

    // текущая скорость сканирования
    m_model.setData(m_model.index(4,0), QString::number(m_investigator->m_currentProcessSpeed, 'f', 2), Qt::DisplayRole);

    // файлов в обработке
    m_model.setData(m_model.index(5,0), m_investigator->m_inProgressFilesNb, Qt::DisplayRole);

    // файлов в очереди
    m_model.setData(m_model.index(6,0), m_investigator->m_inQueueFilesNb, Qt::DisplayRole);

    // версии АВС
    m_model.setData(m_model.index(7,0), QString("Версия баз: %1;\n"
                                                "Ядро М-52: %2;\n"
                                                "Ядро DrWeb: %3;\n"
                                                "Ядро Kaspersky: %4;")
                                                .arg(m_investigator->m_baseVersion)
                                                .arg(m_investigator->m_m52coreVersion)
                                                .arg(m_investigator->m_drwebCoreVersion)
                                                .arg(m_investigator->m_kasperCoreVersion)
                                                , Qt::DisplayRole);

    m_model.item(6,0)->setBackground(
                (
                     (
                        (m_investigator->m_inQueueFilesNb >= m_investigator->m_maxQueueSize)
                      ||
                        (m_investigator->m_inQueueFileSizeMb > m_investigator->m_maxQueueVolMb * (m_investigator->m_maxQueueVolUnit ? 1024 : 1) )
                     )
                 ) ? QBrush(Qt::red) : QBrush(Qt::transparent)
                );
}

void Statistics::on_clearButton_clicked() {
    if(QMessageBox::warning(this,
                            QString("Подтвердите действие"),
                            QString("Вы действительно хотите сбросить накопленную статистику?"),
                            QString("Да"), QString("Нет"), QString(),
                            1) == 0) {
        m_investigator->clearStatistic();
    }
}
