#pragma once
#include <QQmlEngine>
#include <QQmlContext>
#include <QMetaType>

#include "EasyAudioDefine.h"
#include "EasyAudioInterface.h"
#include "EasyFFmpegContext.h"
#include "EasyFFmpegDecoder.h"
#include "EasySilkContext.h"
#include "EasySilkDecoder.h"

/**
 * @brief 组件类型注册
 * @author 龚建波
 * @date 2021-03-30
 */
struct EasyAudioRegister
{
    //注册为QML类型
    static void registerQmlType(QQmlEngine *engine)
    {
        QQmlContext *context = engine->rootContext();context;
    }

    //元对象注册
    static void registerMetaType()
    {
        qRegisterMetaType<EasyWavHead>("EasyWavHead");
        qRegisterMetaType<EasyAudioInfo>("EasyAudioInfo");
    }
};
