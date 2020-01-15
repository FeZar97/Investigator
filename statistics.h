#ifndef STATISTICS_H
#define STATISTICS_H

#include <QDialog>
#include <QStandardItemModel>
#include <QTimer>
#include <QtMath>

#include "distributor.h"

namespace Ui {
    class Statistics;
}

class Statistics : public QDialog {

    Q_OBJECT

public:
    explicit Statistics(QWidget *parent = nullptr, Distributor* distributor = nullptr, QByteArray geometry = nullptr);
    ~Statistics();

    void updateUi();

private slots:
    void on_clearButton_clicked();

private:
    Ui::Statistics *ui;
    QStandardItemModel m_model;
    Distributor* m_distributor;
    QTimer m_workTimer;
};

#endif // STATISTICS_H
