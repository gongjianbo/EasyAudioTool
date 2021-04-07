#include "EasySilkContext.h"

#include <QFileInfo>
#include <QFile>
#include <QCryptographicHash>
#include <QDebug>

EasySilkContext::EasySilkContext(const QString &filepath)
    : EasyAbstractContext(EasyAudio::SilkV3)
{
    initContext(filepath);
}

EasySilkContext::~EasySilkContext()
{
    freeContext();
}

EasyAudioInfo EasySilkContext::audioInfo() const
{
    if(hasInfoTemp)
        return infoTemp;

    EasyAudioInfo info;

    info.filepath = audiopath;
    QFileInfo f_info(audiopath);
    info.filename = f_info.fileName();
    info.filesize = f_info.size();
    QFile f_file(audiopath);
    if(f_file.open(QIODevice::ReadOnly)){
        QCryptographicHash q_hash(QCryptographicHash::Md5);
        q_hash.addData(&f_file);
        info.filemd5 = q_hash.result().toHex().toUpper();
        f_file.close();
    }
    if(!isValid())
        return info;

    //通过总的字节数来计算时长
    qint64 total_byte = 0;
    auto call_back = [&total_byte](const char *, int byte_len)->bool{
        total_byte += byte_len;
        return true;
    };
    const int sample_rate = 8000;
    if(toPcm(audiopath,sample_rate,call_back)){
        //正常解析完毕
        info.format = "silk";
        info.encode = "SILK_V3";
        //TODO silk格式采样率固定展示为16K-16bit-单声道
        info.channels = 1;
        info.sampleRate = 16000; //hz
        info.sampleBit = 16;  //bit
        info.bitRate = 16000*16*1; //bps
        //sample_rate/1000就是1ms的采样数，因为一个点两个字节所以减半
        info.duration = total_byte/(sample_rate/1000*2);  //ms
        info.valid = true;
    }
    //信息暂存起来
    hasInfoTemp = true;
    infoTemp = info;
    return info;
}

bool EasySilkContext::isSilkAudio(const QString &filepath)
{
    //读取文件头信息，判断是否为silk格式头
    QFile file(filepath);
    if(file.size()<10 || !file.open(QIODevice::ReadOnly))
        return false;

    //判断silk头（#!SILK_V3）
    QByteArray read_temp = file.read(1);
    if(read_temp == "#"){
        read_temp += file.read(8);
    }else{
        //微信的silk前面加个一个字节的0x02
        //所以这里直接赋值替换掉先读的一个字节
        read_temp = file.read(9);
    }
    file.close();

    return bool(read_temp == "#!SILK_V3");
}

bool EasySilkContext::toPcm(const QString &filepath,
                            int sampleRate,
                            std::function<bool (const char *, int)> callBack)
{
    //这些变量是从示例拿的
    const int MAX_BYTES_PER_FRAME = 1024;
    const int MAX_INPUT_FRAMES = 5;
    //static const int MAX_FRAME_LENGTH = 480;
    const int FRAME_LENGTH_MS = 20;
    const int MAX_API_FS_KHZ = 48;

    //读取文件头信息，判断是否为silk格式头
    QFile file(filepath);
    if(file.size()<10 || !file.open(QIODevice::ReadOnly))
        return false;

    //判断silk头（#!SILK_V3）
    QByteArray read_temp = file.read(1);
    if(read_temp == "#"){
        read_temp += file.read(8);
    }else{
        //微信的silk前面加个一个字节的0x02
        //所以这里直接赋值替换掉先读的一个字节
        read_temp = file.read(9);
    }

    if(read_temp != "#!SILK_V3"){
        file.close();
        return false;
    }

    SKP_SILK_SDK_DecControlStruct dec_ctrl;
    //采样率
    //Valid values are 8000,12000, 16000, 24000, 32000, 44100, and 48000.
    dec_ctrl.API_sampleRate = sampleRate;
    dec_ctrl.framesPerPacket = 1;
    if(sampleRate<8000){
        qDebug()<<"silk decode sampleRate error."<<sampleRate;
        return false;
    }

    //创建解码器
    SKP_int32 dec_size;
    int ret = SKP_Silk_SDK_Get_Decoder_Size(&dec_size);
    if(ret != 0)
        qDebug()<<"SKP_Silk_SDK_Get_Decoder_Size return"<<ret<<"size"<<dec_size;
    //SKP_uint8 *dec_state = new SKP_uint8[dec_size];
    QVector<SKP_uint8> dec_state(dec_size);
    //初始化解码器
    ret = SKP_Silk_SDK_InitDecoder(dec_state.data());
    if(ret != 0)
        qDebug()<<"SKP_Silk_SDK_InitDecoder return"<<ret;

    //参照示例
    SKP_uint8 payload[MAX_BYTES_PER_FRAME * MAX_INPUT_FRAMES];
    SKP_uint8 *payload_ptr = NULL;
    SKP_int16 out[((FRAME_LENGTH_MS * MAX_API_FS_KHZ) << 1) * MAX_INPUT_FRAMES];
    SKP_int16 *out_ptr = NULL;
    SKP_int16 n_bytes = 0;
    SKP_int32 read_counter = 0;
    SKP_int16 len = 0;
    SKP_int16 total_len = 0;
    SKP_int32 frames = 0;

    //循环读取并解码
    while(true)
    {
        //读取有效数据大小
        read_counter = file.read((char*)&n_bytes,sizeof(SKP_int16));
        if(n_bytes<1 || read_counter<1)
            break;
        //读取有效数据
        read_counter = file.read((char*)payload,n_bytes);
        //qDebug()<<"read_counter"<<read_counter<<n_bytes<<dec_state.toHex();
        if( (SKP_int16)read_counter < n_bytes ) {
            break;
        }
        payload_ptr = payload;
        out_ptr = out;
        total_len = 0;
        frames = 0;
        do {
            //解码
            ret = SKP_Silk_SDK_Decode(dec_state.data(), &dec_ctrl, 0, payload_ptr, n_bytes, out_ptr, &len);
            if (ret) {
                qDebug()<<"SKP_Silk_SDK_Decode returned"<<ret;
            }

            frames++;
            out_ptr += len;
            total_len += len;

            //qDebug()<<"decode"<<len<<total_len;
            if (frames > MAX_INPUT_FRAMES) {
                qDebug()<<"silk frames > MAX_INPUT_FRAMES"<<frames<<total_len;
                out_ptr = out;
                total_len = 0;
                frames = 0;
            }
        } while (dec_ctrl.moreInternalDecoderFrames);
        //回调处理pcm数据
        if(!callBack((const char *)out,total_len*2))
            break;
    }

    //清理
    file.close();
    //delete[] dec_state;
    return true;
}

void EasySilkContext::initContext(const QString &filepath)
{
    audiopath = filepath;
    setValid(isSilkAudio(filepath));
}

void EasySilkContext::freeContext()
{

}
