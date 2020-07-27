QT += core gui widgets network

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../FeZarSource/ClickableLabel.cpp \
    ../FeZarSource/aboutprogramwidget.cpp \
    httpjsonresponder.cpp \
    httprequestmapper.cpp \
    httpsettingsresponder.cpp \
    investigatororchestartor.cpp \
    investigatorworker.cpp \
    main.cpp \
    settingswindow.cpp \
    statisticswindow.cpp \
    widget.cpp

HEADERS += \
    ../FeZarSource/ClickableLabel.h \
    ../FeZarSource/FeZar97.h \
    ../FeZarSource/aboutprogramwidget.h \
    httpjsonresponder.h \
    httprequestmapper.h \
    httpsettingsresponder.h \
    investigatororchestartor.h \
    investigatorworker.h \
    settingswindow.h \
    statisticswindow.h \
    stylehelper.h \
    widget.h

FORMS += \
    ../FeZarSource/aboutprogramwidget.ui \
    settingswindow.ui \
    statisticswindow.ui \
    widget.ui

include(../QtWebApp/QtWebApp/httpserver/httpserver.pri)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

win32: RC_ICONS = $$PWD/../ICONS/INVESTIGATOR.ico

DISTFILES +=
