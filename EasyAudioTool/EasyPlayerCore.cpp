#include "EasyPlayerCore.h"

#include "EasyAudioFactory.h"
#include <QDebug>

EasyPlayerCore::EasyPlayerCore(QObject *parent)
    : QObject(parent)
{
    //参数暂时默认
    audioFormat.setSampleRate(16000);
    audioFormat.setChannelCount(1);
    audioFormat.setSampleSize(16);
    audioFormat.setCodec("audio/pcm");
    audioFormat.setByteOrder(QAudioFormat::LittleEndian);
    audioFormat.setSampleType(QAudioFormat::SignedInt);

    audioDevice = QAudioDeviceInfo::defaultOutputDevice();

    //buffer读取完之后延迟结束
    //TODO 同时会造成播放时间结束还有一段延时才结束播放
    stopTimer.setSingleShot(true);
    connect(&stopTimer,&QTimer::timeout,this,&EasyPlayerCore::stop);
    connect(&audioBuffer,&EasyPlayerBuffer::readEnded,this,[this]{
        stopTimer.start(800); //1s左右
    });
    audioBuffer.open(QIODevice::ReadWrite);
}

EasyPlayerCore::~EasyPlayerCore()
{
    stopTimer.stop();
    if(audioOutput){
        audioOutput->stop();
        audioOutput->deleteLater();
        audioOutput = nullptr;
    }
    audioBuffer.close();
}

void EasyPlayerCore::setPlayerState(EasyAudio::PlayerState state)
{
    if(playerState != state){
        playerState = state;
        emit playerStateChanged(state);
    }
}

void EasyPlayerCore::setPosition(qint64 pos)
{
    if(position != pos){
        position = pos;
        emit positionChanged(pos);
    }
}

qint64 EasyPlayerCore::calcDuration(const QString &filepath)
{
    QSharedPointer<EasyAbstractContext> context = EasyAudioFactory::createContext(filepath);
    if(context && context->isValid()){
        EasyAudioInfo info = context->audioInfo();
        if(info.valid){
            return info.duration;
        }
    }
    return 0;
}

void EasyPlayerCore::play(const QString &filepath, qint64 ms)
{
    //qDebug()<<__FUNCTION__<<ms<<filepath;
    //跳转这部分时间和播放的时间加起来作为当前播放位置
    playOffset = ms;
    setPosition(ms);
    stopTimer.stop();
    audioBuffer.resetBuffer();
    audioDecoder.clear();

    //根据路径构建解码对象
    QSharedPointer<EasyAbstractDecoder> audio_decoder = EasyAudioFactory::createDecoder(filepath);
    if(audio_decoder && audio_decoder->isValid() && audio_decoder->open(audioFormat)){
        audioDecoder = audio_decoder;
    }

    if(!audioDecoder){
        //TODO 给予错误提示的信号
        qDebug()<<"无法解析"<<filepath;
        return;
    }
    doPlay();
}

void EasyPlayerCore::suspend()
{
    if(!audioOutput){
        return;
    }
    if(getPlayerState()==EasyAudio::Playing){
        audioOutput->suspend();
        setPlayerState(EasyAudio::Paused);
    }
}

void EasyPlayerCore::resume()
{
    if(!audioOutput){
        return;
    }
    if(getPlayerState()==EasyAudio::Paused){
        audioOutput->resume();
        setPlayerState(EasyAudio::Playing);
    }
}

void EasyPlayerCore::stop()
{
    if(!audioOutput){
        return;
    }
    audioOutput->stop();
    if(audioDecoder&&audioDecoder->isOpen()){
        audioDecoder->close();
        audioDecoder.clear();
    }
    setPosition(0);
    setPlayerState(EasyAudio::Stopped);
}

void EasyPlayerCore::doPlay()
{
    //先判断设备和参数修改没，变更了就重新new
    //因为目前只用到了defaultOutputDevice，所以只需要判断defaultOutputDevice信息
    const QAudioDeviceInfo cur_default = QAudioDeviceInfo::defaultOutputDevice();
    if(audioDevice != cur_default){
        audioDevice = cur_default;
        if(audioOutput){
            audioOutput->stop();
            audioOutput->deleteLater();
            audioOutput = nullptr;
        }
    }
    if(!audioOutput){
        audioOutput = new QAudioOutput(audioDevice,audioFormat,this);
        //connect(audioOutput,&QAudioOutput::stateChanged,this,[this](QAudio::State state){
        //    qDebug()<<state;
        //});
        connect(audioOutput,&QAudioOutput::notify,this,[this]{
            //const int channels=audioFormat.channelCount();
            //const int sample_rate=audioFormat.sampleRate();
            //const int sample_byte=audioFormat.sampleSize()/8;
            //const int read_ms=audioBuffer.getReadCount()/(channels*sample_rate*sample_byte/1000); //N/(0+1)
            const int play_ms = playOffset+audioOutput->processedUSecs()/1000;
            setPosition(play_ms);
            //qDebug()<<play_ms<<audioBuffer.getReadCount()<<audioBuffer.getWriteCount()
            //       <<audioBuffer.isWriteEnd()<<audioDecoder->atEnd();
            if(audioBuffer.isWaitWrite() && audioDecoder && !audioDecoder->atEnd()){
                //每次读大约一分钟的数据
                audioBuffer.appendData(audioDecoder->readData(EasyPlayerBuffer::getOnceLength()));
                if(audioDecoder->atEnd()){
                    audioBuffer.setWriteEnd(true);
                }
            }
        });
        audioOutput->setNotifyInterval(350);
    }

    //跳转
    if(getPosition()>0){
        audioDecoder->seek(getPosition());
    }

    //每次读大约一分钟的数据
    audioBuffer.appendData(audioDecoder->readData(EasyPlayerBuffer::getOnceLength()));
    if(audioDecoder->atEnd()){
        audioBuffer.setWriteEnd(true);
    }
    audioOutput->start(&audioBuffer);
    setPlayerState(EasyAudio::Playing);
}
