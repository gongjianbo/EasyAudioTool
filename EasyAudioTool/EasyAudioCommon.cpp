#include "EasyAudioCommon.h"

#include <QDebug>

const char *RIFF_FLAG = "RIFF";
const char *WAVE_FLAG = "WAVE";
const char *FMT_FLAG = "fmt ";
const char *DATA_FLAG = "data";

EasyWavHead::EasyWavHead()
{
    memset(this, 0, sizeof(EasyWavHead));
}

EasyWavHead::EasyWavHead(const QAudioFormat &format, unsigned int dataSize)
    : EasyWavHead(format.sampleRate(), format.sampleSize(), format.channelCount(), dataSize)
{

}

EasyWavHead::EasyWavHead(int sampleRate, int sampleSize, int channelCount, unsigned int dataSize)
{
    //先清零再赋值
    memset(this, 0, sizeof(EasyWavHead));

    memcpy(this->riffFlag, RIFF_FLAG, 4);
    memcpy(this->waveFlag, WAVE_FLAG, 4);
    memcpy(this->fmtFlag, FMT_FLAG, 4);
    memcpy(this->dataFlag, DATA_FLAG, 4);

    //除去头部前8个字节的长度，用的44字节的定长格式头，所以+44-8=36
    this->riffSize = dataSize + 36;
    //fmt块大小
    this->fmtSize = 16;
    //1为pcm
    this->compressionCode = 0x01;
    this->numChannels = channelCount;
    this->sampleRate = sampleRate;
    this->bytesPerSecond = (sampleSize / 8) * channelCount * sampleRate;
    this->blockAlign = (sampleSize / 8) * channelCount;
    this->bitsPerSample = sampleSize;
    //除去头的数据长度
    this->dataSize = dataSize;
}

bool EasyWavHead::isValid() const
{
    //简单的比较，主要用在未使用解析器时解析wav头
    if (memcmp(riffFlag, RIFF_FLAG, 4) != 0 ||
            memcmp(waveFlag, WAVE_FLAG, 4) != 0 ||
            memcmp(fmtFlag, FMT_FLAG, 4) != 0 ||
            memcmp(dataFlag, DATA_FLAG, 4) != 0 ||
            riffSize != dataSize + 36 ||
            compressionCode != 0x01)
        return false;
    return true;
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
