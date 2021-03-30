HEADERS += \
    $$PWD/EasyAudioDefine.h \
    $$PWD/EasyAudioInterface.h \
    $$PWD/EasyAudioRegister.h \
    $$PWD/EasyFFmpegContext.h \
    $$PWD/EasyFFmpegDecoder.h

SOURCES += \
    $$PWD/EasyAudioDefine.cpp \
    $$PWD/EasyAudioInterface.cpp \
    $$PWD/EasyFFmpegContext.cpp \
    $$PWD/EasyFFmpegDecoder.cpp

greaterThan(QT_MAJOR_VERSION, 4) {
    TARGET_ARCH=$${QT_ARCH}
} else {
    TARGET_ARCH=$${QMAKE_HOST.arch}
}

contains(TARGET_ARCH, x86_64) {
#only x64 msvc
win32{
LIBS += $$PWD/../ffmpeg/lib/*.lib
INCLUDEPATH += $$PWD/../ffmpeg/include
DEPENDPATH += $$PWD/../ffmpeg/include

LIBS += $$PWD/../silksdk/lib/*.lib
INCLUDEPATH += $$PWD/../silksdk/include
DEPENDPATH += $$PWD/../silksdk/include
}
}
