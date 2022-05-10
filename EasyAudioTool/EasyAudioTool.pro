QT += quick
QT += qml
QT += core
QT += gui
QT += widgets
QT += multimedia
QT += concurrent

TEMPLATE = lib
DEFINES += EASYAUDIOTOOL_LIBRARY
#CONFIG += staticlib
include($$PWD/../ProCommon.pri)
#output dir
#CONFIG(debug, debug|release) { }
DESTDIR = $$PWD/../bin

greaterThan(QT_MAJOR_VERSION, 4) {
    TARGET_ARCH=$${QT_ARCH}
} else {
    TARGET_ARCH=$${QMAKE_HOST.arch}
}

contains(TARGET_ARCH, x86_64) {
#only x64 msvc
win32{
LIBS += $$PWD/../3rd/ffmpeg/lib/*.lib
INCLUDEPATH += $$PWD/../3rd/ffmpeg/include
DEPENDPATH += $$PWD/../3rd/ffmpeg/include

LIBS += $$PWD/../3rd/silksdk/lib/*.lib
INCLUDEPATH += $$PWD/../3rd/silksdk/include
DEPENDPATH += $$PWD/../3rd/silksdk/include
}
}

HEADERS += \
    EasyAudioCommon.h \
    EasyAudioFactory.h \
    EasyAudioInterface.h \
    EasyAudioPlayer.h \
    EasyAudioTool.h \
    EasyFFmpegContext.h \
    EasyFFmpegDecoder.h \
    EasyPlayerBuffer.h \
    EasyPlayerCore.h \
    EasySilkContext.h \
    EasySilkDecoder.h

SOURCES += \
    EasyAudioCommon.cpp \
    EasyAudioFactory.cpp \
    EasyAudioInterface.cpp \
    EasyAudioPlayer.cpp \
    EasyAudioTool.cpp \
    EasyFFmpegContext.cpp \
    EasyFFmpegDecoder.cpp \
    EasyPlayerBuffer.cpp \
    EasyPlayerCore.cpp \
    EasySilkContext.cpp \
    EasySilkDecoder.cpp
