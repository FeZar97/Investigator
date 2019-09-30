# Project created by QtCreator 2019-09-17T09:15:03

QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QASf
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11

SOURCES += \
        checker.cpp \
        distributor.cpp \
        main.cpp \
        mover.cpp \
        widget.cpp

HEADERS += \
        checker.h \
        distributor.h \
        mover.h \
        widget.h

FORMS += \
        widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=

RESOURCES += \
    qrc.qrc
