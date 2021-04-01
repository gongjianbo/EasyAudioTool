#project: Qml Audio Component
#author: GongJianBo 1992
#date: 2021-03-30
QT += core
QT += gui
QT += widgets
QT += quick
QT += multimedia
QT += concurrent

CONFIG += c++11 utf8_source

DESTDIR += $$PWD/bin

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        EasyModel.cpp \
        EasyTest.cpp \
        main.cpp

RESOURCES += qml.qrc

INCLUDEPATH += $$PWD/EasyAudioComponent
include($$PWD/EasyAudioComponent/EasyAudioComponent.pri)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    EasyModel.h \
    EasyTest.h
