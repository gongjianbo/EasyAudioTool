#pragma once
#include "EasyAudioCommon.h"
#include "EasyAudioInterface.h"

/**
 * @brief Context及Decoder/Encoder等的创建
 * @author 龚建波
 * @date 2021-03-31
 */
class EASYAUDIOTOOL_EXPORT EasyAudioFactory
{
public:
    //根据文件路径判断格式并创建对应context
    static QSharedPointer<EasyAbstractContext>
    createContext(const QString &filepath);

    //根据文件路径判断格式并创建对应decoder
    static QSharedPointer<EasyAbstractDecoder>
    createDecoder(const QString &filepath);
};
