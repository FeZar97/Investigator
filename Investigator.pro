# Project created by QtCreator 2019-09-17T09:15:03

QT += core gui widgets network

TARGET = Investigator
TEMPLATE = app

VERSION = 1.6

QMAKE_TARGET_COMPANY     = FeZar97
QMAKE_TARGET_PRODUCT     = Investigator
QMAKE_TARGET_DESCRIPTION = Investigator
QMAKE_TARGET_COPYRIGHT   = FeZar97

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11

HEADERS +=  distributor.h \
            defs.h \
            httpjsonresponder.h \
            investigator.h \
            settings.h \
            statistics.h \
            stylehelper.h \
            widget.h

SOURCES += distributor.cpp \
           httpjsonresponder.cpp \
           investigator.cpp \
           main.cpp \
           settings.cpp \
           statistics.cpp \
           widget.cpp

FORMS += settings.ui \
         statistics.ui \
         widget.ui

include(../QtWebApp/QtWebApp/httpserver/httpserver.pri)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += img.qrc

win32: RC_ICONS = $$PWD/img/INVESTIGATOR.ico
