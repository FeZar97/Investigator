#ifndef STATISTICS_H
#define STATISTICS_H

#include <QStandardItemModel>
#include <QTimer>
#include <QtMath>

#include "investigator.h"

namespace Ui {
    class Statistics;
}

class Statistics : public QDialog {

    Q_OBJECT

public:
    explicit Statistics(QWidget *parent = nullptr, Investigator* investigator = nullptr, QByteArray geometry = nullptr, bool* lockUi = nullptr);
    ~Statistics();

    void updateUi();

private slots:
    void on_clearButton_clicked();

private:
    Ui::Statistics *ui;
    bool* m_lockUi;
    Investigator* m_investigator;
    QStandardItemModel m_model;
    QTimer m_workTimer;
};

#endif // STATISTICS_H
