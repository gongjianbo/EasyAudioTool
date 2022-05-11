#pragma once
#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QSharedPointer>
#include <QTimer>
#include "EasyAudioCommon.h"
#include "EasyAudioInterface.h"
#include "EasyPlayerBuffer.h"
#include "sonic.h"

/**
 * @brief 实际播放音频部分
 * @author 龚建波
 * @date 2021-04-01
 * @details
 * 1.播放组件可能需要多次实例化，但是单个加载的时间较长，故拆分为两部分
 * EasyPlayerCore进行播放，EasyAudioPlayer实例化后指向Core单例
 * 如果需要多个音频同时播放，可以用不同的core实例
 * 2.目前没有多线程方面的设计
 */
class EASYAUDIOTOOL_EXPORT EasyPlayerCore : public QObject
{
    Q_OBJECT
public:
    explicit EasyPlayerCore(QObject *parent = nullptr);
    ~EasyPlayerCore();

    //播放状态
    EasyAudio::PlayerState getPlayerState() const { return playerState; }
    void setPlayerState(EasyAudio::PlayerState state);

    //speed倍速播放，speed/100.0f为实际倍速，即100为原速度
    //core的speed只在stop时设置有效
    int getPlaySpeed() const;
    void setPlaySpeed(int speed);

    //播放进度 ms
    qint64 getPosition() const { return position; }
    void setPosition(qint64 pos);

    //音频时长 ms
    static qint64 calcDuration(const QString &filepath);

    //播放
    //filepath:音频文件路径
    //offset:跳转到指定ms时间播放，<=0则不跳转
    //checkDevice:=true时检测输出设备是否变更
    void play(const QString &filepath, qint64 offset = 0, bool checkDevice = true);
    //暂停
    void suspend();
    //暂停后恢复播放
    void resume();
    //停止
    void stop();

protected:
    //play初始化后调用
    //checkDevice:=true时检测输出设备是否变更
    //因为获取设备信息比较耗时，调节seek和speed时不用校验
    void doPlay(bool checkDevice);
    //play时每次读取数据
    void playReadData();
    //使用sonic库变速
    void initSonic();
    void freeSonic();

signals:
    //通知别的player断开信号槽链接
    void playerItemChanged();
    void playerStateChanged(EasyAudio::PlayerState state);
    void positionChanged(qint64 position);

private:
    //播放设备
    QAudioDeviceInfo audioDevice;
    //播放参数
    QAudioFormat audioFormat;
    //播放
    QAudioOutput *audioOutput{ nullptr };
    //存放音频数据
    EasyPlayerBuffer audioBuffer;
    //解码器
    QSharedPointer<EasyAbstractDecoder> audioDecoder;

    //播放状态
    EasyAudio::PlayerState playerState{ EasyAudio::Stopped };
    //播放时间进度 ms
    qint64 position{ 0 };
    //播放起始时间 ms
    qint64 playOffset{ 0 };
    //播放倍速，使用时除以100.0f，也就是100时为原速
    int playSpeed{ 100 };
    //sonic变速操作的对象
    sonicStream sonicInstance{ nullptr };
    //延时关闭
    QTimer stopTimer;

    friend class EasyAudioPlayer;
};
