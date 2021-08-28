#include "EasyFFmpegDecoder.h"

#include <QDebug>

EasyFFmpegDecoder::EasyFFmpegDecoder()
{

}

EasyFFmpegDecoder::~EasyFFmpegDecoder()
{
    close();
}

void EasyFFmpegDecoder::setContext(const QSharedPointer<EasyAbstractContext> &contextPtr)
{
    close();
    //If the reference count of the old shared data object becomes 0,
    //the old shared data object is deleted.
    theContextPtr = contextPtr.dynamicCast<EasyFFmpegContext>();
}

bool EasyFFmpegDecoder::isValid() const
{
    return ((theContextPtr) && theContextPtr->isValid());
}

bool EasyFFmpegDecoder::open(const QAudioFormat &format)
{
    //无效的文件或者已开启则返回false
    if(!isValid() || isOpen())
        return false;

    //解析前重置变量及相关标志位
    setOpen(false);
    setEnd(false);
    dataTemp.clear();

    //初始化缓冲区
    resetOutBuffer(out_bufsize);

    packet = av_packet_alloc();
    //av_init_packet(packet); 接口已废弃
    frame = av_frame_alloc();

    //原数据格式参数
    inFormat = theContextPtr->audioFormat();
    //目标格式的参数
    outFormat = EasyFFmpegContext::getFormat(format);
    //qDebug()<<"in format"<<inFormat.channels<<inFormat.sampleRate<<inFormat.sampleByte;
    //qDebug()<<"out format"<<outFormat.channels<<outFormat.sampleRate<<outFormat.sampleByte;

    //区分planar和packed??
    //const bool out_is_planar=(out_sample_fmt>AV_SAMPLE_FMT_DBL&&out_sample_fmt!=AV_SAMPLE_FMT_S64);
    //返回每个sample的字节数，S16=2 bytes
    //int sample_bytes = av_get_bytes_per_sample(out_sample_fmt);

    //分配SwrContext并设置/重置公共参数
    //返回NULL为失败，否则分配上下文
    //（目前的需求默认是转为单声道 16bit 的，只有采样率会设置）
    swr_ctx = swr_alloc_set_opts(NULL, //现有的swr上下文，不可用则为NULL
                                 av_get_default_channel_layout(outFormat.channels), //输出通道布局 (AV_CH_LAYOUT_*)
                                 outFormat.sampleFmt, //输出采样格式 (AV_SAMPLE_FMT_*).
                                 outFormat.sampleRate, //输出采样频率 (frequency in Hz)
                                 av_get_default_channel_layout(inFormat.channels), //输入通道布局 (AV_CH_LAYOUT_*)
                                 inFormat.sampleFmt, //输入采样格式 (AV_SAMPLE_FMT_*).
                                 inFormat.sampleRate, //输入采样频率 (frequency in Hz)
                                 0, NULL); //日志相关略
    if(swr_ctx == NULL)
        return false;

    //初始化
    //如果要修改转换的参数，调用参数设置后再次init
    if(swr_init(swr_ctx) < 0)
        return false;

    //因为av_read_frame后就到了下一帧，为了重入先seek到起始处
    //参数一: 上下文;
    //参数二: 流索引, 如果stream_index是-1，会选择一个默认流，时间戳会从以AV_TIME_BASE为单位向具体流的时间基自动转换。
    //参数三: 将要定位处的时间戳，time_base单位或者如果没有流是指定的就用av_time_base单位。
    //参数四: seek功能flag；
    //AVSEEK_FLAG_BACKWARD  是seek到请求的timestamp之前最近的关键帧
    //AVSEEK_FLAG_BYTE 是基于字节位置的查找
    //AVSEEK_FLAG_ANY 是可以seek到任意帧，注意不一定是关键帧，因此使用时可能会导致花屏
    //AVSEEK_FLAG_FRAME 是基于帧数量快进
    //返回值：成功返回>=0
    if(av_seek_frame(theContextPtr->formatCtx,-1,0,AVSEEK_FLAG_ANY) < 0)
        return false;

    setOpen(true);
    return true;
}

QByteArray EasyFFmpegDecoder::readAll(std::function<bool (const char *, int)> callBack)
{
    QByteArray pcm_data;
    if(!isOpen())
        return pcm_data;

    const bool has_callback = bool(callBack);
    int ret = 0;
    //av_read_frame取流的下一帧，这里循环读取
    //返回0表示成功，小于0表示错误或者文件尾
    while (av_read_frame(theContextPtr->formatCtx, packet) >= 0)
    {
        //取音频
        if (packet->stream_index == theContextPtr->audioStreamIndex)
        {
            //提供原始数据作为解码器的输入(将packet写入到解码队列当中去)
            //返回0表示成功
            ret = avcodec_send_packet(theContextPtr->codecCtx, packet);
            if(ret != 0)
                continue;

            //从解码器循环取数据帧
            //返回0表示成功
            while (avcodec_receive_frame(theContextPtr->codecCtx, frame) == 0)
            {
                //下一个采样数的上限
                //swr_get_out_samples貌似算出来的比av_rescale_rnd多一丢丢
                //但是比最终导出的采样数多一点
                const int out_samples = swr_get_out_samples(swr_ctx,frame->nb_samples);
                //const int out_nb_samples=av_rescale_rnd(swr_get_delay(swr_ctx, in_sample_rate)+
                //                                        frame->nb_samples,
                //                                        out_sample_rate,
                //                                        contextPtr->codecCtx->sample_rate,
                //                                        AV_ROUND_ZERO);
                //qDebug()<<out_samples<<out_nb_samples<<out_bufsize;
                //缓冲区大小是否足够，不够就根据计算值扩充
                if(out_bufsize < outFormat.sampleByte*outFormat.channels*out_samples){
                    resetOutBuffer(outFormat.sampleByte*outFormat.channels*out_samples);
                }

                //重采样转换
                //如果传入的nb_samles大于了传出的nb_samplse则SwrContext中会有缓存
                //如果有缓存，swr_get_out_samples第二个参数填零取大小，swr_convert最后两个也填0来获取数据
                //通过使用swr_get_out_samples来获取下一次调用swr_convert在给定输入样本数量下输出样本数量的上限，来提供足够的空间。
                //如果是planar类型比如AV_SAMPLE_FMT_S16P,每个data[0]就是左声道，data[1]就是右声道。
                //如果是packed类型，这种类型左右声道的数据都是一个一维数据连续存放的。LRLRLRLR...，就只有data[0]有数据。
                //return每个通道输出的样本数，出错时为负值
                ret = swr_convert(swr_ctx, out_buffer_arr, out_samples,
                                  (const uint8_t **)frame->data,
                                  frame->nb_samples);

                if (ret <= 0) {
                    av_frame_unref(frame);
                    continue;
                }

                //获取给定音频参数所需的缓冲区大小=通道数 * 采样点数* 采样位数/8
                const int out_bufuse = av_samples_get_buffer_size(NULL,
                                                                  outFormat.channels,
                                                                  ret,
                                                                  outFormat.sampleFmt,
                                                                  1);
                //qDebug()<<"out"<<out_bufuse<<"sample"<<ret<<"channel"<<out_channels<<sample_bytes*out_samples;
                if(out_bufuse > 0){
                    //如果设置了回调，就调用回调函数把buf地址和数据字节长度传递出去
                    if(has_callback){
                        if(!callBack((const char *)out_buffer, out_bufuse))
                            break;
                    }else{
                        //拼接pcm数据，如果每个声道单独一个文件这里要区别设计
                        pcm_data.append((const char*)out_buffer, out_bufuse);
                    }
                }
#if 0
                //保留多声道分别处理的代码，双声道转为两个单声道文件时使用
                if(out_channels==2)
                {
                    //双声道时提取左声道数据
                    //双声道区分planaer和packed
                    if(out_is_planar){
                        //planaer左右声道单独放的
                        if(!callBack((const char*)out_buffer_arr[0], out_bufuse/2)){
                            return false;
                        }
                    }else{
                        //packed都在[0]，一左一右存放
                        for(int i = 0; i < out_bufuse; i += sample_bytes*2)
                        {
                            //回调false则整体失败返回false
                            if(!callBack((const char*)out_buffer_arr[0] + i, sample_bytes)){
                                return false;
                            }
                        }
                    }
                }else if(out_channels==1){
                    //单声道数据
                    //回调false则整体失败返回false
                    if(!callBack((const char*)out_buffer_arr[0], out_bufuse)){
                        return false;
                    }
                }
#endif
                av_frame_unref(frame);
            }
        }
        av_packet_unref(packet);
    }

    setEnd(true);
    return pcm_data;
}

QByteArray EasyFFmpegDecoder::read(qint64 maxSize)
{
    QByteArray pcm_data;
    if(!isOpen() || isEnd() || maxSize < 1)
        return pcm_data;
    //先判断缓存里的数据是否够了
    if(dataTemp.size() >= maxSize){
        pcm_data = dataTemp.left(maxSize);
        dataTemp.remove(0,maxSize);
        return pcm_data;
    }else{
        pcm_data = dataTemp;
    }

    int ret = 0;
    //av_read_frame取流的下一帧，这里循环读取
    //返回0表示成功，小于0表示错误或者文件尾
    while (av_read_frame(theContextPtr->formatCtx, packet) >= 0)
    {
        //取音频
        if(packet->stream_index == theContextPtr->audioStreamIndex)
        {
            //提供原始数据作为解码器的输入(将packet写入到解码队列当中去)
            //返回0表示成功
            ret = avcodec_send_packet(theContextPtr->codecCtx, packet);
            if(ret != 0)
                continue;

            //从解码器循环取数据帧
            //返回0表示成功
            while (avcodec_receive_frame(theContextPtr->codecCtx, frame) == 0)
            {
                //下一个采样数的上限
                //swr_get_out_samples貌似算出来的比av_rescale_rnd多一丢丢
                //但是比最终导出的采样数多一点
                const int out_samples = swr_get_out_samples(swr_ctx,frame->nb_samples);
                //const int out_nb_samples=av_rescale_rnd(swr_get_delay(swr_ctx, in_sample_rate)+
                //                                        frame->nb_samples,
                //                                        out_sample_rate,
                //                                        contextPtr->codecCtx->sample_rate,
                //                                        AV_ROUND_ZERO);
                //qDebug()<<out_samples<<out_nb_samples<<out_bufsize;
                //缓冲区大小是否足够，不够就根据计算值扩充
                if(out_bufsize < outFormat.sampleByte*outFormat.channels*out_samples){
                    resetOutBuffer(outFormat.sampleByte*outFormat.channels*out_samples);
                }

                //重采样转换
                //如果传入的nb_samles大于了传出的nb_samplse则SwrContext中会有缓存
                //如果有缓存，swr_get_out_samples第二个参数填零取大小，swr_convert最后两个也填0来获取数据
                //通过使用swr_get_out_samples来获取下一次调用swr_convert在给定输入样本数量下输出样本数量的上限，来提供足够的空间。
                //如果是planar类型比如AV_SAMPLE_FMT_S16P,每个data[0]就是左声道，data[1]就是右声道。
                //如果是packed类型，这种类型左右声道的数据都是一个一维数据连续存放的。LRLRLRLR...，就只有data[0]有数据。
                //return每个通道输出的样本数，出错时为负值
                ret = swr_convert(swr_ctx, out_buffer_arr, out_samples,
                                  (const uint8_t **)frame->data,
                                  frame->nb_samples);

                if(ret <= 0){
                    av_frame_unref(frame);
                    continue;
                }

                //获取给定音频参数所需的缓冲区大小=通道数 * 采样点数* 采样位数/8
                const int out_bufuse = av_samples_get_buffer_size(NULL,
                                                                  outFormat.channels,
                                                                  ret,
                                                                  outFormat.sampleFmt,
                                                                  1);
                //qDebug()<<"out"<<out_bufuse<<"sample"<<ret<<"channel"<<out_channels<<sample_bytes*out_samples;
                if(out_bufuse > 0){
                    //拼接pcm数据，如果每个声道单独一个文件这里要区别设计
                    pcm_data.append((const char*)out_buffer, out_bufuse);
                }
                av_frame_unref(frame);

                //数据达到maxSize就返回
                if(pcm_data.size() >= maxSize){
                    dataTemp = pcm_data.right(pcm_data.size()-maxSize);
                    pcm_data = pcm_data.left(maxSize);

                    av_packet_unref(packet);
                    return pcm_data;
                }
            }
        }
        av_packet_unref(packet);
    }

    setEnd(true);
    return pcm_data;
}

void EasyFFmpegDecoder::close()
{
    setOpen(false);
    setEnd(true);
    dataTemp.clear();

    if(frame){
        av_frame_unref(frame);
        av_frame_free(&frame);
        frame = NULL;
    }
    if(packet){
        //av_free_packet改用av_packet_unref
        av_packet_unref(packet);
        av_packet_free(&packet);
        packet = NULL;
    }
    if(swr_ctx){
        swr_close(swr_ctx);
        swr_free(&swr_ctx);
        swr_ctx = NULL;
    }
    if(out_buffer){
        delete [] out_buffer;
        out_buffer = NULL;
        out_buffer_arr[0] = NULL;
        out_buffer_arr[1] = NULL;
    }
}

void EasyFFmpegDecoder::resetOutBuffer(int bufferSize)
{
    if(out_buffer){
        delete [] out_buffer;
    }
    out_bufsize = bufferSize*1.2;
    if(out_bufsize%2 != 0)
        out_bufsize += 1;
    out_buffer = new uint8_t[out_bufsize];
    out_buffer_arr[0] = out_buffer;
    out_buffer_arr[1] = out_buffer+out_bufsize/2;
}
