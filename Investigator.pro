# Project created by QtCreator 2019-09-17T09:15:03

QT       += core gui concurrent widgets

TARGET = Investigator
TEMPLATE = app

VERSION = 1.1

QMAKE_TARGET_COMPANY     = FeZar97
QMAKE_TARGET_PRODUCT     = Investigator
QMAKE_TARGET_DESCRIPTION = Investigator
QMAKE_TARGET_COPYRIGHT   = FeZar97

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11

SOURCES += \
        avwrapper.cpp \
        distributor.cpp \
        main.cpp \
        settings.cpp \
        statistics.cpp \
        widget.cpp

HEADERS += \
        avwrapper.h \
        distributor.h \
        settings.h \
        statistics.h \
        widget.h

FORMS += \
        settings.ui \
        statistics.ui \
        widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=

RESOURCES += \
    img.qrc

win32: RC_ICONS = $$PWD/img/INVESTIGATOR.ico

