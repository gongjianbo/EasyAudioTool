#include "EasyAudioPlayer.h"

EasyPlayerCore* getPlayerCore(){
    static EasyPlayerCore core;
    return &core;
}

EasyAudioPlayer::EasyAudioPlayer(QObject *parent)
    : QObject(parent)
{
    core = getPlayerCore();
    connect(core,&EasyPlayerCore::playerStateChanged,this,&EasyAudioPlayer::setPlayerState);
    connect(core,&EasyPlayerCore::positionChanged,this,&EasyAudioPlayer::setPosition);
}

EasyAudioPlayer::~EasyAudioPlayer()
{
    core->stop();
}

void EasyAudioPlayer::setFilepath(const QString &filepath)
{
    //TODO 目前没有自动播放列表的功能，所以设置路径时不会stop/play
    //需要显示调用stop/play
    if(audiopath != filepath){
        audiopath = filepath;
        emit filepathChanged();
    }
}

void EasyAudioPlayer::setPlayerState(EasyAudio::PlayerState state)
{
    if(playerState != state){
        playerState = state;
        emit playerStateChanged();
    }
}

void EasyAudioPlayer::setPosition(qint64 pos)
{
    if(position != pos){
        position = pos;
        emit positionChanged();
    }
}

//void EasyAudioPlayer::setTarget(const QString &targetInfo)
//{
//    if(target!=targetInfo){
//        target=targetInfo;
//        emit targetChanged();
//    }
//}

void EasyAudioPlayer::play(const QString &filepath)
{
    stop();
    //如果参数带路径则使用新的路径，否则播放之前设置的文件
    if(!filepath.isEmpty())
        setFilepath(filepath);
    if(!getFilepath().isEmpty())
        core->play(getFilepath());
}

void EasyAudioPlayer::pause()
{
    core->pause();
}

void EasyAudioPlayer::stop()
{
    core->stop();
}
