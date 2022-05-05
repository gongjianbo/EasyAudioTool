QT += core
QT += gui
QT += multimedia
QT += concurrent

CONFIG += c++11
CONFIG += utf8_source

TEMPLATE = lib
CONFIG += staticlib
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
    EasyAuidoTool.h

SOURCES += \
    EasyAuidoTool.cpp
