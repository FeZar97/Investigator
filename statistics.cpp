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
    m_model.setVerticalHeaderLabels(QStringList() << "Найдено вирусов" << "Просканировано файлов" << "Общий размер\nпросканированных файлов (Мб)"
                                                  << "Средняя скорость\nсканирования (Мб/с)" << "Текущая скорость\nсканирования (Мб/с)" << "Файлов в очереди"
                                                  << "Количество сформированных\nотчетов");

    ui->tableView->setModel(&m_model);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

Statistics::~Statistics() {
    delete ui;
}

void Statistics::updateUi() {
    // qDebug() << "QTime: " << QTime(0, 0, 0, 0).addSecs(int(m_distributor->getWorkTimeInSecs()).toString("hh:mm:ss");
    // qDebug() << "getWorkTimeInSecs: " << m_distributor->getWorkTimeInSecs();

    ui->workTimeInfoLabel->setText(QTime(0, 0, 0, 0).addSecs(int(m_distributor->getWorkTimeInSecs())).toString("hh:mm:ss"));

    m_model.setData(m_model.index(0,0), m_distributor->getAVDangerFilesNb(AV::KASPER), Qt::DisplayRole);
    m_model.setData(m_model.index(0,1), m_distributor->getAVDangerFilesNb(AV::DRWEB), Qt::DisplayRole);

    m_model.setData(m_model.index(1,0), m_distributor->getAVProcessedFilesNb(AV::KASPER), Qt::DisplayRole);
    m_model.setData(m_model.index(1,1), m_distributor->getAVProcessedFilesNb(AV::DRWEB), Qt::DisplayRole);

    m_model.setData(m_model.index(2,0), m_distributor->getAVProcessedFilesSize(AV::KASPER), Qt::DisplayRole);
    m_model.setData(m_model.index(2,1), m_distributor->getAVProcessedFilesSize(AV::DRWEB), Qt::DisplayRole);

    m_model.setData(m_model.index(3,0), m_distributor->getAVAverageSpeed(AV::KASPER), Qt::DisplayRole);
    m_model.setData(m_model.index(3,1), m_distributor->getAVAverageSpeed(AV::DRWEB), Qt::DisplayRole);

    m_model.setData(m_model.index(4,0), m_distributor->getAVCurrentSpeed(AV::KASPER), Qt::DisplayRole);
    m_model.setData(m_model.index(4,1), m_distributor->getAVCurrentSpeed(AV::DRWEB), Qt::DisplayRole);

    m_model.setData(m_model.index(5,0), m_distributor->getAVQueueFilesNb(AV::KASPER), Qt::DisplayRole);
    m_model.setData(m_model.index(5,1), m_distributor->getAVQueueFilesNb(AV::DRWEB), Qt::DisplayRole);

    m_model.setData(m_model.index(6,0), m_distributor->getAVCurrentReportIdx(AV::KASPER), Qt::DisplayRole);
    m_model.setData(m_model.index(6,1), m_distributor->getAVCurrentReportIdx(AV::DRWEB), Qt::DisplayRole);
}
