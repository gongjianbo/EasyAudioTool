#pragma once
#include <QQmlEngine>
#include <QQmlContext>
#include <QMetaType>
#include <atomic>

#include "EasyAudioCommon.h"
#include "EasyAudioInterface.h"
#include "EasyAudioPlayer.h"

/**
 * @brief 提供音频工具函数，如文件转码等
 * @author 龚建波
 * @date 2022-05-10
 */
class EASYAUDIOTOOL_EXPORT EasyAudioTool
{
public:
    //注册QML类型，初始化该lib时调用，如main中加载qml前
    static void registerQmlType(QQmlEngine *engine);
    //元对象注册，初始化该lib时调用，如main中加载qml前
    static void registerMetaType();

    //文件转码为wav格式
    //目前仅测试wav-16K-单声道-16bit
    //srcPath:源文件路径
    //destPath:目标路径
    //destFormat:目标格式
    //runflag:=false则停止转码，返回false
    static bool transcodeToWavFile(const QString &srcPath, const QString &destPath,
                                   const QAudioFormat &destFormat, const std::atomic_bool &runflag);
    //默认导出格式
    static QAudioFormat defaultFormat();
};
