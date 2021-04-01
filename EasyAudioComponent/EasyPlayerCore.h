#pragma once
#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QSharedPointer>
#include <QTimer>
#include "EasyAudioDefine.h"
#include "EasyAudioInterface.h"
#include "EasyPlayerBuffer.h"

/**
 * @brief 实际播放音频部分
 * @author 龚建波
 * @date 2021-04-01
 * @details
 * 1.播放组件可能需要多次实例化，但是单个加载的时间较长，故拆分为两部分
 * EasyPlayerCore进行播放，EasyAudioPlayer实例化后指向Core单例
 * 2.目前没有多线程方面的设计
 */
class EasyPlayerCore : public QObject
{
    Q_OBJECT
public:
    explicit EasyPlayerCore(QObject *parent = nullptr);
    ~EasyPlayerCore();

    //播放状态
    EasyAudio::PlayerState getPlayerState() const { return playerState; }
    void setPlayerState(EasyAudio::PlayerState state);

    //播放进度
    qint64 getPosition() const { return position; }
    void setPosition(qint64 pos);

    //播放
    void play(const QString &filepath);
    //暂停/恢复
    void pause();
    //停止
    void stop();

protected:
    //play初始化后调用
    void doPlay();

signals:
    void playerStateChanged(EasyAudio::PlayerState state);
    void positionChanged(qint64 position);

private:
    //播放设备
    QAudioDeviceInfo audioDevice;
    //播放参数
    QAudioFormat audioFormat;
    //播放
    QAudioOutput *audioOutput = nullptr;
    //存放音频数据
    EasyPlayerBuffer audioBuffer;
    //解码器
    QSharedPointer<EasyAbstractDecoder> audioDecoder;

    //播放状态
    EasyAudio::PlayerState playerState = EasyAudio::Stopped;
    //播放时间进度 ms
    qint64 position = 0;
    //延时关闭
    QTimer stopTimer;

    friend class EasyAudioPlayer;
};

