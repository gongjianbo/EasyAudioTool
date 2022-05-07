#pragma once
#include <QObject>
#include "EasyAudioCommon.h"
#include "EasyPlayerCore.h"

/**
 * @brief 音频播放
 * @author 龚建波
 * @date 2021-04-01
 * @details
 * 1.使用Decoder解码为pcm数据后进行播放
 * 2.播放组件可能需要多次实例化，但是单个加载的时间较长，故拆分为两部分
 * EasyPlayerCore进行播放，EasyAudioPlayer实例化后指向Core单例
 * @note
 * 1.之前继承自QQuickItem获取visibleChanged来自动关闭
 *   为了QML与Widgets共用，继承自QObject去掉UI关联性
 *   QML可以封装一个QML Item来获取visibleChanged
 * 2.目前需要保存当前播放列表的一些状态，暂时不作为QML单例
 * 3.以前一个ListView多个音频共用了一个Player，导致更新进度和状态时每个Item都要判断是否是当前音频在更新
 *   现在改为每个Item单独一个Player，切换Item播放时重新关联信号槽
 *   但这时要考虑ListView Item离开范围被释放的问题
 * @history
 * 2022-05-07 ListView多个Item共享Player变更为独立的Player
 * 2022-05-08 增加跳转播放
 */
class EASYAUDIOTOOL_EXPORT EasyAudioPlayer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString filepath READ getFilepath WRITE setFilepath NOTIFY filepathChanged)
    Q_PROPERTY(EasyAudio::PlayerState playerState READ getPlayerState NOTIFY playerStateChanged)
    Q_PROPERTY(bool onPlaying READ getOnPlaying NOTIFY playerStateChanged)
    Q_PROPERTY(qint64 position READ getPosition NOTIFY positionChanged)
public:
    explicit EasyAudioPlayer(QObject *parent = nullptr);
    ~EasyAudioPlayer();

    //文件路径
    QString getFilepath() const;
    void setFilepath(const QString &filepath);

    //播放状态
    EasyAudio::PlayerState getPlayerState() const;
    void setPlayerState(EasyAudio::PlayerState state);
    bool getOnPlaying() const;
    bool getIsStopped() const;
    bool getIsPaused() const;

    //播放进度 ms
    qint64 getPosition() const;
    void setPosition(qint64 pos);

    //获取音频时长
    Q_INVOKABLE qint64 getDuration() const;

public slots:
    //切换core关联的player对象
    void prepare();
    //播放或者暂停后继续
    void play();
    //暂停
    void pause();
    //停止
    void stop();
    //快进N ms，N>0
    void forward(qint64 ms);
    //快退N ms，N>0
    void backward(qint64 ms);
    //跳转到指定ms时间，<0则为0
    //TODO 大于总时长的情况未处理，只是继续播放会直接结束
    void seek(qint64 ms);

signals:
    void filepathChanged();
    void playerStateChanged();
    void positionChanged();

private:
    //音频文件路径
    QString audioPath;
    //播放状态
    EasyAudio::PlayerState playerState{ EasyAudio::Stopped };
    //播放时间进度 ms
    qint64 position{ 0 };

    //实际播放加载内容作为单例
    EasyPlayerCore *core{ nullptr };
};
