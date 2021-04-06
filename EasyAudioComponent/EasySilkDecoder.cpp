#include "EasySilkDecoder.h"

#include <QDebug>

EasySilkDecoder::EasySilkDecoder()
{

}

EasySilkDecoder::~EasySilkDecoder()
{
    close();
}

void EasySilkDecoder::setContext(const QSharedPointer<EasyAbstractContext> &contextPtr)
{
    close();
    //If the reference count of the old shared data object becomes 0,
    //the old shared data object is deleted.
    theContextPtr = contextPtr.dynamicCast<EasySilkContext>();
}

bool EasySilkDecoder::isValid() const
{
    return ((theContextPtr) && theContextPtr->isValid());
}

bool EasySilkDecoder::open(const QAudioFormat &format)
{
    //无效的文件或者已开启则返回false
    if(!isValid()||isOpen())
        return false;

    decodeSampleRate = format.sampleRate();
    setOpen(false);
    setEnd(false);
    dataTemp.clear();

    //读取文件头信息，判断是否为silk格式头
    decodeFile.setFileName(theContextPtr->audiopath);
    if(decodeFile.size()<10 || !decodeFile.open(QIODevice::ReadOnly))
        return false;

    //判断silk头（#!SILK_V3）
    QByteArray read_temp = decodeFile.read(1);
    if(read_temp == "#"){
        read_temp += decodeFile.read(8);
    }else{
        //微信的silk前面加个一个字节的0x02
        read_temp = decodeFile.read(9);
    }
    if(read_temp != "#!SILK_V3"){
        decodeFile.close();
        return false;
    }

    //SKP_SILK_SDK_DecControlStruct dec_ctrl;
    //采样率
    //Valid values are 8000,12000, 16000, 24000, 32000, 44100, and 48000.
    dec_ctrl.API_sampleRate = format.sampleRate();
    dec_ctrl.framesPerPacket = 1;
    //默认就单声道16K
    if(format.channelCount()!=1 || format.sampleSize()!=16){
        qDebug()<<"silk decode format ignore."<<format;
    }

    //创建解码器
    SKP_int32 dec_size;
    int ret = SKP_Silk_SDK_Get_Decoder_Size(&dec_size);
    if(ret != 0){
        qDebug()<<"SKP_Silk_SDK_Get_Decoder_Size return"<<ret<<"size"<<dec_size;
        decodeFile.close();
        return false;
    }
    dec_state = QVector<SKP_uint8>(dec_size);
    //初始化解码器
    ret = SKP_Silk_SDK_InitDecoder(dec_state.data());
    if(ret != 0){
        qDebug()<<"SKP_Silk_SDK_InitDecoder return"<<ret;
        decodeFile.close();
        return false;
    }

    setOpen(true);
    return true;
}

QByteArray EasySilkDecoder::readAll(std::function<bool (const char *, int)> callBack)
{
    QByteArray pcm_data;
    if(!isOpen())
        return pcm_data;

    const bool has_callback = bool(callBack);
    SKP_int16 n_bytes = 0;
    SKP_int32 read_counter = 0;
    SKP_int16 len = 0;
    SKP_int16 total_len = 0;
    SKP_int32 frames = 0;
    int ret = 0;

    //循环读取并解码
    while (true) {
        //读取有效数据大小
        read_counter = decodeFile.read((char*)&n_bytes,sizeof(SKP_int16));
        if(n_bytes<1 || read_counter<1){
            setEnd(true);
            break;
        }
        //读取有效数据
        read_counter = decodeFile.read((char*)payload,n_bytes);
        //qDebug()<<"read_counter"<<read_counter<<n_bytes<<dec_state.toHex();
        if((SKP_int16)read_counter < n_bytes) {
            setEnd(true);
            break;
        }
        payload_ptr = payload;
        out_ptr = out;
        total_len = 0;
        frames = 0;
        do {
            //解码
            ret = SKP_Silk_SDK_Decode(dec_state.data(), &dec_ctrl, 0,
                                      payload_ptr, n_bytes, out_ptr, &len);
            if (ret) {
                qDebug()<<"SKP_Silk_SDK_Decode returned"<<ret;
            }

            frames++;
            out_ptr += len;
            total_len += len;

            //qDebug()<<"decode"<<len<<total_len;
            if (frames > MAX_INPUT_FRAMES) {
                qDebug()<<"frames > MAX_INPUT_FRAMES"<<frames<<total_len;
                out_ptr = out;
                total_len = 0;
                frames = 0;
            }
        } while (dec_ctrl.moreInternalDecoderFrames);

        //如果设置了回调，就调用回调函数把buf地址和数据字节长度传递出去
        if(has_callback){
            if(!callBack((const char *)out, total_len*2))
                break;
        }else{
            //拼接pcm数据
            pcm_data.append((const char *)out, total_len*2);
        }
    }

    return pcm_data;
}

QByteArray EasySilkDecoder::read(qint64 maxSize)
{
    QByteArray pcm_data;
    if(!isOpen() || isEnd() || maxSize<1)
        return pcm_data;

    //先判断缓存里的数据是否够了
    if(dataTemp.size() >= maxSize){
        pcm_data = dataTemp.left(maxSize);
        dataTemp.remove(0,maxSize);
        return pcm_data;
    }else{
        pcm_data = dataTemp;
    }

    SKP_int16 n_bytes = 0;
    SKP_int32 read_counter = 0;
    SKP_int16 len = 0;
    SKP_int16 total_len = 0;
    SKP_int32 frames = 0;
    int ret = 0;

    //循环读取并解码
    while (true) {
        //读取有效数据大小
        read_counter = decodeFile.read((char*)&n_bytes,sizeof(SKP_int16));
        if(n_bytes<1 || read_counter<1){
            setEnd(true);
            break;
        }
        //读取有效数据
        read_counter = decodeFile.read((char*)payload,n_bytes);
        //qDebug()<<"read_counter"<<read_counter<<n_bytes<<dec_state.toHex();
        if((SKP_int16)read_counter < n_bytes) {
            setEnd(true);
            break;
        }
        payload_ptr = payload;
        out_ptr = out;
        total_len = 0;
        frames = 0;
        do {
            //解码
            ret = SKP_Silk_SDK_Decode(dec_state.data(), &dec_ctrl, 0,
                                      payload_ptr, n_bytes, out_ptr, &len);
            if (ret) {
                qDebug()<<"SKP_Silk_SDK_Decode returned"<<ret;
            }

            frames++;
            out_ptr += len;
            total_len += len;

            //qDebug()<<"decode"<<len<<total_len;
            if (frames > MAX_INPUT_FRAMES) {
                qDebug()<<"frames > MAX_INPUT_FRAMES"<<frames<<total_len;
                out_ptr = out;
                total_len = 0;
                frames = 0;
            }
        } while (dec_ctrl.moreInternalDecoderFrames);

        //拼接pcm数据
        pcm_data.append((const char *)out,total_len*2);
        //数据达到maxSize就返回
        if(pcm_data.size() >= maxSize){
            dataTemp = pcm_data.right(pcm_data.size()-maxSize);
            pcm_data = pcm_data.left(maxSize);
            break;
        }
    }

    return pcm_data;
}

void EasySilkDecoder::close()
{
    setOpen(false);
    setEnd(true);
    decodeFile.close();
    dataTemp.clear();
    dec_state.clear();
}
