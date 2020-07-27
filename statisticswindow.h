#ifndef STATISTICSWINDOW_H
#define STATISTICSWINDOW_H

#include <QDialog>
#include <QStandardItemModel>
#include <QMessageBox>

#include "investigatororchestartor.h"

namespace Ui {
    class StatisticsWindow;
}

class StatisticsWindow : public QDialog {

    Q_OBJECT

public:
    explicit StatisticsWindow(QWidget *parent = nullptr, InvestigatorOrchestartor* investigator = nullptr);
    ~StatisticsWindow();

    void updateUi();

private:
    Ui::StatisticsWindow *ui;
    InvestigatorOrchestartor* m_investigator;
    QStandardItemModel m_model;

    void createUi();
};

#endif // STATISTICSWINDOW_H
