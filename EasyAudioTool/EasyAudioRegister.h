#pragma once
#include <QQmlEngine>
#include <QQmlContext>
#include <QMetaType>

#include "EasyAudioCommon.h"
#include "EasyAudioInterface.h"
#include "EasyAudioPlayer.h"

/**
 * @brief 组件类型注册
 * @author 龚建波
 * @date 2022-05-06
 */
namespace EasyAudioTool
{

//注册为QML类型
static void registerQmlType(QQmlEngine *engine)
{
    QQmlContext *context = engine->rootContext();context;
    qmlRegisterType<EasyAudioPlayer>("EasyAudioTool",1,0,"EasyAudioPlayer");
}

//元对象注册
static void registerMetaType()
{
    qRegisterMetaType<EasyWavHead>("EasyWavHead");
    qRegisterMetaType<EasyAudioInfo>("EasyAudioInfo");
}

}
