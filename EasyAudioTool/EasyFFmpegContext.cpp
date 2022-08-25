#include "EasyFFmpegContext.h"

#include <QFileInfo>
#include <QFile>
#include <QCryptographicHash>
#include <QDebug>

EasyFFmpegContext::EasyFFmpegContext(const QString &filepath)
    : EasyAbstractContext(EasyAudio::FFmpeg)
{
    initContext(filepath);
}

EasyFFmpegContext::~EasyFFmpegContext()
{
    freeContext();
}

EasyAudioInfo EasyFFmpegContext::audioInfo() const
{
    if(hasInfoTemp)
        return infoTemp;

    EasyAudioInfo info;

    //把需要的格式信息copy过来
    info.filepath = audiopath;
    QFileInfo f_info(audiopath);
    info.filename = f_info.fileName();
    info.filesize = f_info.size();
    QFile f_file(audiopath);
    if(f_file.open(QIODevice::ReadOnly)){
        QCryptographicHash q_hash(QCryptographicHash::Md5);
        q_hash.addData(&f_file);
        info.md5 = q_hash.result().toHex().toUpper();
        f_file.close();
    }
    info.codec = "FFmpeg";
    if(!isValid())
        return info;

    //mov格式单独处理，默认返回的是一个列表
    info.format = formatCtx->iformat->name; //如mp3 wav
    if(info.format.startsWith("mov")){
        info.format = getMovFormat();
    }
    info.encode = codec->name; //如pcm_s16
    info.channels = codecParam->channels;
    info.sampleRate = codecParam->sample_rate; //hz
    //2020-08-25 之前取的容器的位宽，不是数据的
    //info.sampleBit = (av_get_bytes_per_sample(codecCtx->sample_fmt)<<3);  //bit
    info.sampleBit = av_get_bits_per_sample(codecParam->codec_id);
    //if (codecCtx && codecCtx->bits_per_raw_sample > 0) {
    //    info.sampleBit = codecCtx->bits_per_raw_sample;
    //}
    //2020-12-31 测试一个ape文件时发现音频信息比特率为0，现判断无效则使用容器比特率
    info.bitRate = codecCtx->bit_rate<1?formatCtx->bit_rate:codecCtx->bit_rate; //bps
    info.duration = formatCtx->duration/(AV_TIME_BASE/1000.0);  //ms
    info.valid = true;

    //信息暂存起来
    hasInfoTemp = true;
    infoTemp = info;
    return info;
}

EasyFFmpegContext::ArgFormat EasyFFmpegContext::audioFormat() const
{
    ArgFormat format;
    if(isValid()){
        format.channels = codecCtx->channels;
        format.sampleRate = codecCtx->sample_rate;
        format.sampleFmt = codecCtx->sample_fmt;
        format.sampleByte = av_get_bytes_per_sample(format.sampleFmt);
    }
    return format;
}

EasyFFmpegContext::ArgFormat EasyFFmpegContext::getFormat(const QAudioFormat &qtformat)
{
    ArgFormat format;
    //valid会判断设置的四个参数，以及codec
    if(qtformat.isValid()){
        format.channels = qtformat.channelCount();
        format.sampleRate = qtformat.sampleRate();
        //8 16 32
        const int sample_size = qtformat.sampleSize();
        //sint usint float
        const QAudioFormat::SampleType sample_type = qtformat.sampleType();
        AVSampleFormat format_fmt = AV_SAMPLE_FMT_S16;
        switch (sample_type) {
        case QAudioFormat::SignedInt:
            if(sample_size == 16){
                format_fmt = AV_SAMPLE_FMT_S16;
            }else if(sample_size == 32){
                format_fmt = AV_SAMPLE_FMT_S32;
            }else if(sample_size == 64){
                format_fmt = AV_SAMPLE_FMT_S64;
            }
            break;
        case QAudioFormat::UnSignedInt:
            if(sample_size == 8){
                format_fmt = AV_SAMPLE_FMT_U8;
            }
            break;
        case QAudioFormat::Float:
            if(sample_size == 32){
                format_fmt = AV_SAMPLE_FMT_FLT;
            }else if(sample_size == 64){
                format_fmt = AV_SAMPLE_FMT_DBL;
            }
            break;
        }
        format.sampleFmt = format_fmt;
        format.sampleByte = av_get_bytes_per_sample(format.sampleFmt);
    }else{
        qDebug()<<"invalid format"<<qtformat;
    }
    return format;
}

void EasyFFmpegContext::initContext(const QString &filepath)
{
    audiopath = filepath;
    if(!QFileInfo::exists(filepath)){
        setLastError("audio file does not exist.");
        return;
    }

    //ffmpeg默认用的utf8编码，这里转换下
    QByteArray temp = filepath.toUtf8();
    const char *c_path = temp.constData();

    //打开输入流并读取头
    //流要使用avformat_close_input关闭
    //成功时返回=0
    const int result = avformat_open_input(&formatCtx, c_path, NULL, NULL);
    if (result != 0 || formatCtx == NULL){
        setLastError("avformat_open_input error.");
        return;
    }

    //读取文件获取流信息，把它存入AVFormatContext中
    //正常时返回>=0
    if (avformat_find_stream_info(formatCtx, NULL) < 0) {
        setLastError("avformat_find_stream_info error.");
        return;
    }

    //测试用的打印信息，暂时保留
    //qDebug()<<"filepath"<<filepath;
    //duration/AV_TIME_BASE单位为秒
    //qDebug()<<"duration"<<formatCtx->duration/(AV_TIME_BASE/1000.0)<<"ms";
    //qDebug()<<"format"<<formatCtx->iformat->name<<":"<<formatCtx->iformat->long_name;
    //有些格式codecCtx->bit_rate没有码率，这时候再使用formatCtx->bit_rate
    //qDebug()<<"bitrate"<<formatCtx->bit_rate<<"bps";
    //qDebug()<<"n stream"<<formatCtx->nb_streams;
    for (unsigned int i = 0; i < formatCtx->nb_streams; i++)
    {
        //AVStream是存储每一个视频/音频流信息的结构体
        AVStream *in_stream = formatCtx->streams[i];

        //类型为音频
        if(in_stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            codecParam = in_stream->codecpar;
            streamIndex = i;
            //查找具有匹配编解码器ID的已注册解码器
            //失败返回NULL
            codec = avcodec_find_decoder(in_stream->codecpar->codec_id);
            if(codec == NULL){
                setLastError("avcodec_find_decoder error.");
                return;
            }

            //分配AVCodecContext并将其字段设置为默认值
            //需要使用avcodec_free_context释放生成的对象
            //如果失败，则返回默认填充或者 NULL
            codecCtx = avcodec_alloc_context3(codec);
            if(codecCtx == NULL){
                setLastError("avcodec_alloc_context3 error.");
                return;
            }

            //根据编码器填充上下文参数
            //事实上codecpar包含了大部分解码器相关的信息，这里是直接从AVCodecParameters复制到AVCodecContext
            //成功时返回值>=0
            if(avcodec_parameters_to_context(codecCtx, in_stream->codecpar) < 0){
                setLastError("avcodec_parameters_to_context error.");
                return;
            }

            //某些AVCodecContext字段的访问器，已弃用
            //av_codec_set_pkt_timebase(codec_ctx, in_stream->time_base);

            //没有此句会出现：Could not update timestamps for skipped samples
            codecCtx->pkt_timebase = formatCtx->streams[i]->time_base;

            //打开解码器（将解码器和解码器上下文进行关联）
            //使用给定的AVCodec初始化AVCodecContext
            //在之前必须使用avcodec_alloc_context3()分配上下文
            //成功时返回值=0
            if(avcodec_open2(codecCtx, codec, nullptr) != 0){
                setLastError("avcodec_open2 error.");
                return;
            }

            //采样率
            //qDebug()<<"sample rate"<<codecCtx->sample_rate;
            //比特率
            //qDebug()<<"bit rate"<<codecCtx->bit_rate<<formatCtx->bit_rate;
            //通道数
            //qDebug()<<"channels"<<codecCtx->channels;
            //采样深度
            //in_stream->codec->sample_fmt枚举AVSampleFormat表示数据存储格式，如16位无符号
            //av_get_bytes_per_sample返回AVSampleFormat对应的字节大小
            //qDebug()<<"sample bit"<<codecCtx->sample_fmt<<":"<<(av_get_bytes_per_sample(codecCtx->sample_fmt)<<3);
            //编码，如pcm
            //qDebug()<<"codec name"<<codec->name<<":"<<codec->long_name;

            //no error
            setValid(true);
            return;
        }
    }
    setLastError("no audio stream.");
}

void EasyFFmpegContext::freeContext()
{
    if(codecCtx){
        //不要直接使用avcodec_close，而是用avcodec_free_context
        //把codec相关的其他东西一并释放
        avcodec_free_context(&codecCtx);
    }
    if(formatCtx){
        //avformat_close_input内部其实已经调用了avformat_free_context
        avformat_close_input(&formatCtx);
        //avformat_free_context(formatCtx);
    }
    codec = NULL;
    codecCtx = NULL;
    codecParam = NULL;
    formatCtx = NULL;
}

QString EasyFFmpegContext::getMovFormat() const
{
    const QStringList mov_list={"mov","mp4","m4a","3gp","3g2","mj2"};
    //MOVMuxContext没包含，暂时用brand信息遍历
    AVDictionaryEntry *t = NULL;
    t = av_dict_get(formatCtx->metadata, "major_brand", NULL, AV_DICT_IGNORE_SUFFIX);
    if(t){
        const QString value=QString(t->value).toLower();
        for(auto &item : mov_list)
        {
            if(value.contains(item))
                return item;
        }
    }
    t = av_dict_get(formatCtx->metadata, "compatible_brands", NULL, AV_DICT_IGNORE_SUFFIX);
    if(t){
        const QString value=QString(t->value).toLower();
        for(auto &item : mov_list)
        {
            if(value.contains(item))
                return item;
        }
    }
    return QString("mov");
}
