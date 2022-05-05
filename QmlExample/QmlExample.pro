QT += quick
QT += qml
QT += core
QT += gui
QT += widgets
QT += multimedia
QT += sql
QT += network
QT += concurrent

CONFIG += c++11
CONFIG += utf8_source

INCLUDEPATH += $$PWD/../EasyAudioTool
DEPENDPATH += $$PWD/../EasyAudioTool

DESTDIR = $$PWD/../bin
LIBS += $$PWD/../bin/EasyAudioTool.lib
PRE_TARGETDEPS += $$PWD/../bin/EasyAudioTool.lib

SOURCES += \
        main.cpp

RESOURCES += \
    qml.qrc
