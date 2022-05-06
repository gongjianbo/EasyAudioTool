#include "EasyAudioPlayer.h"

#include <QDebug>

EasyPlayerCore* getPlayerCore(){
    static EasyPlayerCore core;
    return &core;
}

EasyAudioPlayer::EasyAudioPlayer(QObject *parent)
    : QObject(parent)
{
    //以前一个ListView多个音频共用了一个Player
    //导致更新进度和状态时每个Item都要判断是否是当前音频在更新
    //现在改为每个Item单独一个Player，切换Item播放时重新关联信号槽
    //core = getPlayerCore();
    //connect(core,&EasyPlayerCore::playerStateChanged,this,&EasyAudioPlayer::setPlayerState);
    //connect(core,&EasyPlayerCore::positionChanged,this,&EasyAudioPlayer::setPosition);
}

EasyAudioPlayer::~EasyAudioPlayer()
{
    stop();
}

QString EasyAudioPlayer::getFilepath() const
{
    return audioPath;
}

void EasyAudioPlayer::setFilepath(const QString &filepath)
{
    if(audioPath != filepath){
        //如果正在播放则停止
        if(!getIsStopped()){
            stop();
        }

        audioPath = filepath;
        emit filepathChanged();
    }
}

EasyAudio::PlayerState EasyAudioPlayer::getPlayerState() const
{
    return playerState;
}

void EasyAudioPlayer::setPlayerState(EasyAudio::PlayerState state)
{
    if(playerState != state){
        playerState = state;
        emit playerStateChanged();
    }
}

bool EasyAudioPlayer::getOnPlaying() const
{
    return (playerState == EasyAudio::Playing);
}

bool EasyAudioPlayer::getIsStopped() const
{
    return (playerState == EasyAudio::Stopped);
}

bool EasyAudioPlayer::getIsPaused() const
{
    return (playerState == EasyAudio::Paused);
}

qint64 EasyAudioPlayer::getPosition() const
{
    return position;
}

void EasyAudioPlayer::setPosition(qint64 pos)
{
    if(position != pos){
        position = pos;
        emit positionChanged();
    }
}

void EasyAudioPlayer::prepare()
{
    //已经加载过了就返回
    if(core){
        return;
    }

    core = getPlayerCore();
    emit core->playerItemChanged();

    connect(core,&EasyPlayerCore::playerStateChanged,this,&EasyAudioPlayer::setPlayerState);
    connect(core,&EasyPlayerCore::positionChanged,this,&EasyAudioPlayer::setPosition);
    connect(core,&EasyPlayerCore::playerItemChanged,this,[this]{
        core->stop();
        core->disconnect(this);
        core = nullptr;
        setPosition(0);
        setPlayerState(EasyAudio::Stopped);
    });
}

void EasyAudioPlayer::play()
{
    if(!core){
        prepare();
    }
    if(getFilepath().isEmpty()){
        return;
    }
    //暂停就恢复，否则重新开始
    if(getIsPaused()){
        core->resume();
    }else{
        core->play(getFilepath());
    }
}

void EasyAudioPlayer::pause()
{
    if(!core){
        return;
    }
    core->suspend();
}

void EasyAudioPlayer::stop()
{
    if(!core){
        return;
    }
    core->stop();
}

void EasyAudioPlayer::forward(qint64 ms)
{
    ms;
}

void EasyAudioPlayer::backward(qint64 ms)
{
    ms;
}

void EasyAudioPlayer::seek(qint64 ms)
{
    ms;
}
