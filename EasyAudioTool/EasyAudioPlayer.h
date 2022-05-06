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
 * 为了QML与Widgets共用，继承自QObject去掉UI关联性
 * QML可以封装一个QML Item来获取visibleChanged
 * 2.目前需要保存当前播放列表的一些状态，暂时不作为QML单例
 */
class EASYAUDIOTOOL_EXPORT EasyAudioPlayer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString filepath READ getFilepath WRITE setFilepath NOTIFY filepathChanged)
    Q_PROPERTY(EasyAudio::PlayerState playerState MEMBER playerState NOTIFY playerStateChanged)
    Q_PROPERTY(bool onPlaying READ getOnPlaying NOTIFY playerStateChanged)
    Q_PROPERTY(qint64 position READ getPosition NOTIFY positionChanged)
    //Q_PROPERTY(QString target READ getTarget WRITE setTarget NOTIFY targetChanged)
public:
    explicit EasyAudioPlayer(QObject *parent = nullptr);
    ~EasyAudioPlayer();

    //文件路径
    QString getFilepath() const { return audiopath; }
    void setFilepath(const QString &filepath);

    //播放状态
    EasyAudio::PlayerState getPlayerState() const { return playerState; }
    bool getOnPlaying() const { return (playerState==EasyAudio::Playing); }
    void setPlayerState(EasyAudio::PlayerState state);

    //播放进度
    qint64 getPosition() const { return position; }
    void setPosition(qint64 pos);

    //用于判断当前播放组件
    //QString getTarget() const { return target; }
    //void setTarget(const QString &targetInfo);

public slots:
    //播放
    void play(const QString &filepath = QString());
    //暂停/恢复
    void pause();
    //停止
    void stop();

signals:
    void filepathChanged();
    void playerStateChanged();
    void positionChanged();
    //void targetChanged();

private:
    //音频文件路径
    QString audiopath;
    //播放状态
    EasyAudio::PlayerState playerState{ EasyAudio::Stopped };
    //播放时间进度 ms
    qint64 position{ 0 };

    //实际播放加载内容作为单例
    EasyPlayerCore *core{ nullptr };
    //用于判断当前组件唯一性，只有路径在多级列表会联动
    //QString target;
};
