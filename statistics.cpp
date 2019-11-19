#include "statistics.h"
#include "ui_statistics.h"

Statistics::Statistics(QWidget *parent, Distributor* distributor, QByteArray geometry, bool visible): QDialog(parent), ui(new Ui::Statistics) {

    ui->setupUi(this);

    setLayout(ui->mainLayout);
    setWindowTitle("Статистика работы");
    restoreGeometry(geometry);
    setVisible(visible);

    m_distributor = distributor;
    m_workTimer.setInterval(1000);

    connect(&m_workTimer, &QTimer::timeout, this, &Statistics::updateUi);
    m_workTimer.start();

    m_model.setHorizontalHeaderLabels(QStringList() << "Kaspersky" << "DrWeb");
    m_model.setVerticalHeaderLabels(QStringList() << "Обнаружено зараженных файлов" << "Просканировано файлов" << "Объем\nпросканированных файлов (Мб)"
                                                  << "Средняя скорость\nсканирования (Мб/с)" << "Текущая скорость\nсканирования (Мб/с)"
                                                  << "Файлов в обработке" << "Файлов в очереди" << "Количество сформированных\nотчетов");

    ui->tableView->setModel(&m_model);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setSelectionMode(QAbstractItemView::NoSelection);

    updateUi();
}

Statistics::~Statistics() {
    delete ui;
}

void Statistics::updateUi() {

    ui->workTimeInfoLabel->setText(
                    QString("%1 дней ").arg(QDate(m_distributor->getStartTime().date()).daysTo(m_distributor->getEndTime().date())) +
                    QTime(0,0,0,0).addSecs(QTime(m_distributor->getStartTime().time()).secsTo(m_distributor->getEndTime().time())).toString("hh ч. mm мин. ss сек."));

    // обнаружено зараженных файлов
    m_model.setData(m_model.index(0,0), m_distributor->getAVDangerFilesNb(AV::KASPER), Qt::DisplayRole);
    m_model.setData(m_model.index(0,1), m_distributor->getAVDangerFilesNb(AV::DRWEB), Qt::DisplayRole);

    // просканировано файлов
    m_model.setData(m_model.index(1,0), m_distributor->getAVProcessedFilesNb(AV::KASPER), Qt::DisplayRole);
    m_model.setData(m_model.index(1,1), m_distributor->getAVProcessedFilesNb(AV::DRWEB), Qt::DisplayRole);

    // объем просканированных файлов
    m_model.setData(m_model.index(2,0), m_distributor->getAVProcessedFilesSize(AV::KASPER), Qt::DisplayRole);
    m_model.setData(m_model.index(2,1), m_distributor->getAVProcessedFilesSize(AV::DRWEB), Qt::DisplayRole);

    // средняя скорость сканирования
    m_model.setData(m_model.index(3,0), m_distributor->getAVAverageSpeed(AV::KASPER), Qt::DisplayRole);
    m_model.setData(m_model.index(3,1), m_distributor->getAVAverageSpeed(AV::DRWEB), Qt::DisplayRole);

    // текущая скорость сканирования
    m_model.setData(m_model.index(4,0), m_distributor->getAVCurrentSpeed(AV::KASPER), Qt::DisplayRole);
    m_model.setData(m_model.index(4,1), m_distributor->getAVCurrentSpeed(AV::DRWEB), Qt::DisplayRole);

    // файлов в обработке
    m_model.setData(m_model.index(5,0), m_distributor->getAVInProgressFilesNb(AV::KASPER), Qt::DisplayRole);
    m_model.setData(m_model.index(5,1), m_distributor->getAVInProgressFilesNb(AV::DRWEB), Qt::DisplayRole);

    // файлов в очереди
    m_model.setData(m_model.index(6,0), m_distributor->getAVQueueFilesNb(AV::KASPER), Qt::DisplayRole);
    m_model.setData(m_model.index(6,1), m_distributor->getAVQueueFilesNb(AV::DRWEB), Qt::DisplayRole);

    // количество сформированных отчетов
    m_model.setData(m_model.index(7,0), m_distributor->getAVCurrentReportIdx(AV::KASPER), Qt::DisplayRole);
    m_model.setData(m_model.index(7,1), m_distributor->getAVCurrentReportIdx(AV::DRWEB), Qt::DisplayRole);

    m_model.item(6,0)->setBackground(
                (
                     (
                        (m_distributor->getAVQueueFilesNb(AV::KASPER) >= m_distributor->getMaxQueueSize(AV::KASPER))
                      ||
                        (m_distributor->getAVQueueFilesVolMb(AV::KASPER) >= m_distributor->calcMaxQueueVol(AV::KASPER))
                     )
                 &&
                     (m_distributor->getAVUse(AV::KASPER))
                 ) ? QBrush(Qt::red) : QBrush(Qt::transparent)
                );

    m_model.item(6,1)->setBackground(
                (
                     (
                        (m_distributor->getAVQueueFilesNb(AV::DRWEB) >= m_distributor->getMaxQueueSize(AV::DRWEB))
                      ||
                        (m_distributor->getAVQueueFilesVolMb(AV::DRWEB) >= m_distributor->calcMaxQueueVol(AV::DRWEB))
                     )
                 &&
                     (m_distributor->getAVUse(AV::DRWEB))
                 ) ? QBrush(Qt::red) : QBrush(Qt::transparent)
                );
}

void Statistics::on_clearButton_clicked() {
    m_distributor->clearStatistic();
}
