QT += core
QT += gui
QT += multimedia
QT += concurrent

CONFIG += c++11 utf8_source

HEADERS += \
    $$PWD/EasyAudioDefine.h \
    $$PWD/EasyAudioFactory.h \
    $$PWD/EasyAudioInterface.h \
    $$PWD/EasyAudioPlayer.h \
    $$PWD/EasyAudioRegister.h \
    $$PWD/EasyAudioTool.h \
    $$PWD/EasyFFmpegContext.h \
    $$PWD/EasyFFmpegDecoder.h \
    $$PWD/EasyPlayerBuffer.h \
    $$PWD/EasyPlayerCore.h \
    $$PWD/EasySilkContext.h \
    $$PWD/EasySilkDecoder.h

SOURCES += \
    $$PWD/EasyAudioDefine.cpp \
    $$PWD/EasyAudioFactory.cpp \
    $$PWD/EasyAudioInterface.cpp \
    $$PWD/EasyAudioPlayer.cpp \
    $$PWD/EasyAudioTool.cpp \
    $$PWD/EasyFFmpegContext.cpp \
    $$PWD/EasyFFmpegDecoder.cpp \
    $$PWD/EasyPlayerBuffer.cpp \
    $$PWD/EasyPlayerCore.cpp \
    $$PWD/EasySilkContext.cpp \
    $$PWD/EasySilkDecoder.cpp

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
