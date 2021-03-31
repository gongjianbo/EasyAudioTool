#include "EasyAudioDefine.h"

#include <QDebug>

EasyWavHead EasyWavHead::createHead(const QAudioFormat &format, unsigned int dataSize)
{
    //采样精度位数
    const int bits = format.sampleSize();
    //通道数
    const int channels = format.channelCount();
    //采样率
    const int sample_rate = format.sampleRate();
    //格式头字节数
    const int head_size = sizeof(EasyWavHead);
    EasyWavHead wav_head;
    memset(&wav_head, 0, head_size);

    memcpy(wav_head.riffFlag, "RIFF", 4);
    memcpy(wav_head.waveFlag, "WAVE", 4);
    memcpy(wav_head.fmtFlag, "fmt ", 4);
    memcpy(wav_head.dataFlag, "data", 4);

    //出去头部前8个字节的长度，用的44字节的格式头，所以+44-8=36
    wav_head.riffSize = dataSize + 36;
    //fmt块不含id-flag和size的长度
    wav_head.fmtSize = 16;
    //1为pcm
    wav_head.compressionCode = 0x01;
    wav_head.numChannels = channels;
    wav_head.sampleRate = sample_rate;
    wav_head.bytesPerSecond = (bits / 8) * channels * sample_rate;
    wav_head.blockAlign = (bits / 8) * channels;
    wav_head.bitsPerSample = bits;
    //除去头的数据长度
    wav_head.dataSize = dataSize;

    //copy elision
    return wav_head;
}


void EasyAudioInfo::dumpAudioInfo()
{
    qDebug()<<"Dump Audio Info:"
           <<"\n\tfilepath:"<<filepath
          <<"\n\tfilename:"<<filename
         <<"\n\tfilesize:"<<filesize
        <<"\n\tformat:"<<format
       <<"\n\tencode:"<<encode
      <<"\n\tchannels:"<<channels
     <<"\n\tsampleRate:"<<sampleRate
    <<"\n\tsampleBit:"<<sampleBit
    <<"\n\tbitRate:"<<bitRate
    <<"\n\tduration:"<<duration;
}

