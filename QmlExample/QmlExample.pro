QT += quick
QT += qml
QT += core
QT += gui
QT += widgets
QT += multimedia
QT += sql
QT += network
QT += concurrent

include($$PWD/../ProCommon.pri)
#output dir
#CONFIG(debug, debug|release) { }
DESTDIR = $$PWD/../bin
LIBS += $$PWD/../bin/EasyAudioTool.lib
PRE_TARGETDEPS += $$PWD/../bin/EasyAudioTool.lib

INCLUDEPATH += $$PWD/../EasyAudioTool
DEPENDPATH += $$PWD/../EasyAudioTool

SOURCES += \
        main.cpp

RESOURCES += \
    qml.qrc
