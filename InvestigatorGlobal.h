#ifndef INVESTIGATORGLOBAL_H
#define INVESTIGATORGLOBAL_H

#include <QtGlobal>
#include <QString>

/*! \class ProcessStatistic

    \brief Класс для хранения статистики работы программы или отдельного воркера. */
class ProcessStatistic {
    quint64 m_processedFilesNb; // обработано кол-во
    quint64 m_processedFilesSize; // обработано объем
    quint64 m_pwdFilesNb; // кол-во запароленных
    quint64 m_infFilesNb; // кол-во инфицированных
    quint64 m_currentSpeed; // скорость
    QString m_avsVersion; // версия АВС

public:
    ProcessStatistic(quint64 processedFilesNb = 0, quint64 processedFilesSize = 0,
                     quint64 pwdFilesNb = 0, quint64 infFilesNb = 0,
                     quint64 currentSpeed = 0, QString avsVersion = ""):
        m_processedFilesNb{processedFilesNb},
        m_processedFilesSize{processedFilesSize},
        m_pwdFilesNb{pwdFilesNb},
        m_infFilesNb{infFilesNb},
        m_currentSpeed{currentSpeed},
        m_avsVersion{avsVersion} {}

    ProcessStatistic &operator=(const ProcessStatistic &obj) {

        if (this == &obj) {
            return *this;
        }

        m_processedFilesNb = obj.m_processedFilesNb;
        m_processedFilesSize = obj.m_processedFilesSize;
        m_pwdFilesNb = obj.m_pwdFilesNb;
        m_infFilesNb = obj.m_infFilesNb;
        m_currentSpeed = obj.m_currentSpeed;
        m_avsVersion = obj.m_avsVersion;

        return *this;
    }
};


/*! \class Directories

    \brief Класс, хранящий информацию об используемых директориях.
    Экземпляры класса используются как для хранения информации о настройках директорий для воркеров,
    так и для хранения информации о глобальных настройках путей программы. */
class Directories {
    QString m_inputDir;  // директория, из которой забираются файлы
    QString m_tempDir;   // временная директория для промежуточных этапов
    QString m_workDir;   // директория, в которой осуществляется проверка
    QString m_dangerDir; // директория для зараженных файлов
    QString m_cleanDir;  // директория для чистых файлов
    QString m_reportDir; // директория для отчетов АВС

public:
    Directories(QString inputDir = "", QString tempDir   = "",
                QString workDir  = "", QString dangerDir = "",
                QString cleanDir = "", QString reportDir = ""):
        m_inputDir{inputDir},
        m_tempDir{tempDir},
        m_workDir{workDir},
        m_dangerDir{dangerDir},
        m_cleanDir{cleanDir},
        m_reportDir{reportDir} {}

    Directories &operator=(const Directories &obj) {

        if (this == &obj) {
            return *this;
        }

        m_inputDir = obj.m_inputDir;
        m_tempDir = obj.m_tempDir;
        m_workDir = obj.m_workDir;
        m_dangerDir = obj.m_dangerDir;
        m_cleanDir = obj.m_cleanDir;
        m_reportDir = obj.m_reportDir;

        return *this;
    }
};

#endif // INVESTIGATORGLOBAL_H
