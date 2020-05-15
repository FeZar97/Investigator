#include "docreportcreator.h"

DocReportCreator::DocReportCreator()
{}

void DocReportCreator::createReport(QString reportName) {
    QAxObject *wordApp = new QAxObject("Word.Application");
    QAxObject *wordDoc = wordApp->querySubObject("Documents()");

    QAxObject *newDoc = wordDoc->querySubObject("Add(QVariant)", QVariant("C:\\report.docx"));

    // заголовок таблицы
    QAxObject *rangeName1 = newDoc->querySubObject("Range()");
    rangeName1->dynamicCall("SetRange(int,int)", 101, 200);
    rangeName1->setProperty("Text", "Сводная таблица " + reportName);

    // выравнивание
    QAxObject *rangeName1Align = rangeName1->querySubObject("ParagraphFormat");
    rangeName1Align->setProperty("Alignment", "wdAlignParagraphLeft");

    // таблицы
    QAxObject *tables = newDoc->querySubObject("Tables()");
    QAxObject *rangeTable1 = newDoc->querySubObject("Range()");
    rangeTable1->dynamicCall("SetRange(int, int)", 201, 300);
    QAxObject* table1 = tables->querySubObject("Add(Range, NumRows, NumColumns, DefaultTableBehavior, AutoFitBehavior)", rangeTable1->asVariant(), 4, 3, 1, 1);

    //Заполняем таблицу
    //горизонтальные заголовки
    QAxObject *currentCell = table1->querySubObject("Cell(Row, Column)", 1, 1);
    QAxObject *rangeCurrentCell = currentCell->querySubObject("Range()");
    rangeCurrentCell->dynamicCall("InsertAfter(Text)", "Наименование");
    currentCell = table1->querySubObject("Cell(Row, Column)", 1, 2);
    rangeCurrentCell = currentCell->querySubObject("Range()");
    rangeCurrentCell->dynamicCall("InsertAfter(Text)", "Формула");
    currentCell = table1->querySubObject("Cell(Row, Column)", 1, 3);
    rangeCurrentCell = currentCell->querySubObject("Range()");
    rangeCurrentCell->dynamicCall("InsertAfter(Text)", "Значение");

    //вертикальные заголовки
    for (int i = 0; i < 3; i++) {
        currentCell = table1->querySubObject("Cell(Row, Column)", i+2, 1);
        rangeCurrentCell = currentCell->querySubObject("Range()");
        QString temp = "Переменная" + QString::number(i);
        rangeCurrentCell->dynamicCall("InsertAfter(Text)", temp);
    }

    wordApp->setProperty("Visible", true);
}
