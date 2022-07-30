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
    //tagretPath:目标路径
    //tagretFormat:目标格式
    //runflag:=false则停止转码，返回false
    static bool transcodeToWavFile(const QString &srcPath, const QString &tagretPath,
                                   const QAudioFormat &tagretFormat, const std::atomic_bool &runflag);

    //文件转码为wav格式
    //目前仅测试wav-16K-单声道-16bit
    //sourcePath:源文件路径
    //targetDir:目标路径，文件名在路径下以"{uuid}.wav"命名
    //targetFormat:目标格式
    //limitSize:每个文件分片大小，超过分片大小则生成新文件
    //runflag:=false则停止转码，返回false
    //返回文件信息列表list<uuid,filepath>
    static auto transcodeToWavFile(const QString &sourcePath, const QString &targetDir,
                                   const QAudioFormat &targetFormat, const qint64 &limitSize,
                                   const std::atomic_bool &runflag)
    ->QList<QPair<QString, QString>>;

    //文件拼接为wav格式
    //目前仅测试wav-16K-单声道-16bit
    //sourcePaths:源文件路径列表
    //targetDir:目标路径，文件名在路径下以"{uuid}.wav"命名
    //targetFormat:目标格式
    //limitSize:每个文件分片大小，超过分片大小则生成新文件
    //runflag:=false则停止转码，返回false
    //返回文件信息列表list<uuid,filepath>
    static auto stitchToWavFile(const QList<QString> &sourcePaths, const QString &targetDir,
                                const QAudioFormat &targetFormat, const qint64 &limitSize,
                                const std::atomic_bool &runflag)
    ->QList<QPair<QString, QString>>;

    //默认导出格式
    static QAudioFormat defaultFormat();
};
