#include "statisticswindow.h"
#include "ui_statisticswindow.h"

StatisticsWindow::StatisticsWindow(QWidget *parent, InvestigatorOrchestartor *investigator):
    QDialog(parent),
    ui(new Ui::StatisticsWindow) {

    ui->setupUi(this);
    setWindowTitle("Статистика работы");
    setLayout(ui->mainLayout);

    m_investigator = investigator;

    createUi();
}

StatisticsWindow::~StatisticsWindow() {
    delete ui;
}

void StatisticsWindow::updateUi() {

    if(m_investigator) {
        // проверено файлов
        m_model.setData(m_model.index(0,0),
                        m_investigator->totalProcessedFilesNb(), Qt::DisplayRole);

        // проверено объем
        m_model.setData(m_model.index(1,0),
                        SizeConverter::sizeToString(m_investigator->totalProcessedFilesSize()), Qt::DisplayRole);

        // зараженных файлов
        m_model.setData(m_model.index(2,0),
                        m_investigator->totalInfectedFilesNb(), Qt::DisplayRole);

        // файлов с паролем
        m_model.setData(m_model.index(3,0),
                        m_investigator->totalPwdFilesNb(), Qt::DisplayRole);

        // скорость проверки
        m_model.setData(m_model.index(4,0),
                        QString("%1/с").arg(SizeConverter::sizeToString(m_investigator->currentSpeed())), Qt::DisplayRole);

        // файлов в обработке
        m_model.setData(m_model.index(5,0),
                        m_investigator->inProcessFilesNb() , Qt::DisplayRole);

        // файлов в очереди
        m_model.setData(m_model.index(6,0),
                        m_investigator->currentQueueFilesNb(), Qt::DisplayRole);

        // версии АВС
        m_model.setData(m_model.index(7,0),
                        m_investigator->avsVersion().isEmpty() ? "Не определена" : m_investigator->avsVersion(), Qt::DisplayRole);
    }
}
void StatisticsWindow::createUi() {
    m_model.setHorizontalHeaderLabels(QStringList() << "M-52");
    m_model.setVerticalHeaderLabels(QStringList() << "Проверено файлов"      << "Объем\nпроверенных файлов"
                                                  << "Зараженных файлов"     << "Файлов с паролем"
                                                  << "Скорость сканирования" << "Файлов в обработке"
                                                  << "Файлов в очереди"      << "Версия АВС");

    ui->tableView->setModel(&m_model);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setSelectionMode(QAbstractItemView::NoSelection);

    updateUi();
    ui->tableView->resizeRowsToContents();
}
