#include "statistics.h"
#include "ui_statistics.h"

Statistics::Statistics(QWidget *parent, Investigator* investigator, QByteArray geometry): QDialog(parent), ui(new Ui::Statistics) {

    ui->setupUi(this);

    setLayout(ui->mainLayout);
    setWindowTitle("Статистика работы");
    restoreGeometry(geometry);
    setVisible(true);
    resize(this->minimumSize());

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

    ui->InfoLabel->setText(m_investigator->m_processInfo);

    m_model.setHorizontalHeaderLabels(QStringList() << "M-52");
    m_model.setVerticalHeaderLabels(QStringList() << "Обнаружено зараженных файлов"          << "Просканировано файлов"
                                                  << "Объем\nпросканированных файлов (МБ)"   << "Средняя скорость\nсканирования (МБ/с)"
                                                  << "Текущая скорость\nсканирования (МБ/с)" << "Файлов в обработке"
                                                  << "Файлов в очереди"                      << "Версия АВС");

    ui->tableView->setModel(&m_model);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
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

    m_investigator->m_workTime = QString("%1 дней ").arg( days ) +
                                 QTime(0,0,0,0).addSecs(QTime(m_investigator->m_startTime.time()).secsTo(m_investigator->getEndTime().time())).toString("hh ч. mm мин. ss сек.");
    m_investigator->m_workTimeEn = QString("%1 days ").arg( days ) +
                                 QTime(0,0,0,0).addSecs(QTime(m_investigator->m_startTime.time()).secsTo(m_investigator->getEndTime().time())).toString("hh 'h.' mm 'min.' ss 'sec.'");

    ui->workTimeInfoLabel->setText(m_investigator->m_workTime);

    // обнаружено зараженных файлов
    m_model.setData(m_model.index(0,0), m_investigator->m_infectedFilesNb, Qt::DisplayRole);

    // просканировано файлов
    m_model.setData(m_model.index(1,0), m_investigator->m_processedFilesNb, Qt::DisplayRole);

    // объем просканированных файлов
    m_model.setData(m_model.index(2,0), m_investigator->m_processedFilesSizeMb, Qt::DisplayRole);

    // средняя скорость сканирования
    m_model.setData(m_model.index(3,0), m_investigator->m_averageProcessSpeed, Qt::DisplayRole);

    // текущая скорость сканирования
    m_model.setData(m_model.index(4,0), m_investigator->m_currentProcessSpeed, Qt::DisplayRole);

    // файлов в обработке
    m_model.setData(m_model.index(5,0), m_investigator->m_inProgressFilesNb, Qt::DisplayRole);

    // файлов в очереди
    m_model.setData(m_model.index(6,0), m_investigator->m_inQueueFilesNb, Qt::DisplayRole);

    // версии АВС
    m_model.setData(m_model.index(7,0), m_investigator->m_avVersion, Qt::DisplayRole);

    m_model.item(6,0)->setBackground(
                (
                     (
                        (m_investigator->m_inQueueFilesNb >= m_investigator->m_maxQueueSize)
                      ||
                        (m_investigator->m_inQueueFileSizeMb >= m_investigator->m_maxQueueVolMb)
                     )
                 ) ? QBrush(Qt::red) : QBrush(Qt::transparent)
                );
}

void Statistics::on_clearButton_clicked() {
    m_investigator->clearStatistic();
}
