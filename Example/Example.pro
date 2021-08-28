#project: Qml Audio Component
#author: GongJianBo 1992
#date: 2021-03-30
QT += core
QT += gui
QT += widgets
QT += quick

CONFIG += c++11 utf8_source
DESTDIR += $$PWD/bin

QMAKE_CFLAGS_WARN_ON = /W3
QMAKE_CXXFLAGS_WARN_ON = $$QMAKE_CFLAGS_WARN_ON
QMAKE_CXXFLAGS += /sdl
DEFINES += QT_DEPRECATED_WARNINGS

HEADERS += \
    EasyModel.h \
    EasyTest.h

SOURCES += \
        EasyModel.cpp \
        EasyTest.cpp \
        main.cpp

RESOURCES += qml.qrc

INCLUDEPATH += $$PWD/../EasyAudioComponent
include($$PWD/../EasyAudioComponent/EasyAudioComponent.pri)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#DEFINES += VLD_Check
contains(DEFINES,VLD_Check){
message(enable VLD_Check)
win32{
INCLUDEPATH += "D:/Program Files (x86)/Visual Leak Detector/include"
contains(QMAKE_HOST.arch, x86_64) {
    LIBS += "D:/Program Files (x86)/Visual Leak Detector/lib/Win64/vld.lib"
} else {
    LIBS += "D:/Program Files (x86)/Visual Leak Detector/lib/Win32/vld.lib"
} #end host.arch
} #end win32
} else {
message(disable VLD_Check)
} #end defines
