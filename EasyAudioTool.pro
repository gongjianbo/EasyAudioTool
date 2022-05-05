TEMPLATE = subdirs

SUBDIRS += \
    EasyAudioTool \
    QmlExample 

QmlExample.depends += EasyAudioTool
