#pragma once
#include "EasyAudioDefine.h"
#include "EasyAudioInterface.h"
#include <QObject>
#include <QFutureWatcher>
#include <QThreadPool>
#include <QUrl>
#include <QDir>
#include <atomic>

/**
 * @brief 音频数据处理工具
 * @author 龚建波
 * @date 2021-04-02
 * @details
 * 1.主要是对音频文件处理，如转码、拼接等
 * 2.处理逻辑在线程中进行，并反馈进度，提供终止操作接口
 * 线程处理的结果以信号槽返回
 */
class EasyAudioTool : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList supportedSuffixs READ getSupportedSuffixs WRITE setSupportedSuffixs NOTIFY supportedSuffixsChanged)
    Q_PROPERTY(QString cacheDir READ getCacheDir WRITE setCacheDir NOTIFY cacheDirChanged)
    //处理状态
    Q_PROPERTY(bool processing READ getProcessing NOTIFY processStatusChanged)
    Q_PROPERTY(int processCount READ getProcessCount NOTIFY processProgressChanged)
    Q_PROPERTY(int processSuccess READ getProcessSuccess NOTIFY processProgressChanged)
    Q_PROPERTY(int processFail READ getProcessFail NOTIFY processProgressChanged)
public:
    explicit EasyAudioTool(QObject *parent = nullptr);
    ~EasyAudioTool();

    //支持的后缀列表，默认为空，可以把常用的放到这里面
    QStringList getSupportedSuffixs() { return supportedSuffixs; }
    void setSupportedSuffixs(const QStringList &filter);

    //缓存目录，在使用前设置
    QString getCacheDir() const { return cacheDir; }
    void setCacheDir(const QString &dir);

    //【】处理状态
    bool getProcessing() const { return onProcess; }
    int getProcessCount() const { return processCount; }
    int getProcessSuccess() const { return processSuccess; }
    int getProcessFail() const { return processFail; }

private:
    //QList<QUrl>转为QList<QString>
    //QUrl.toLocalFile()获取路径
    QList<QString> urlsToStrs(const QList<QUrl> &urls);
    //设置处理状态
    //on为处理启停，start时count为处理数，finished时count>0=true成功
    void setProcessing(bool on, int count);
    //设置计数器值
    void setProcessProgress(int count, int success, int fail);
    //更新处理进度，result=true则增加success，否则增加fail计数
    void updateProcessProgress(bool result);

signals:
    void supportedSuffixsChanged();
    void cacheDirChanged(const QString &dir);
    //【】处理状态
    //启停
    void processStatusChanged();
    //处理开始，count为总数
    void processStarted(int count);
    //处理进度，count总数，success当前成功数，fail当前失败数
    void processProgressChanged(int count, int success, int fail);
    //处理结果，result=true则成功
    void processFinished(bool result);
    //【】解析
    //每解析完一个音频文件的信息后，信号槽传递出来
    void parseFinished(const EasyAudioInfo &info);
    //解析信息失败的文件列表
    void parseFailedChanged(const QStringList &files);
    //【】转码
    //类似解析，转码完一个音频文件后，信息信号槽传递出来
    void transcodeFinished(const EasyAudioInfo &info);
    //转码失败的文件列表
    void transcodeFailedChanged(const QStringList &files);
    //【】拼接
    //返回拼接结果，只要一个处理失败则无效
    //根据info.valid判断结果有效性
    void stitchFinished(const EasyAudioInfo &info);

public slots:
    //【】解析
    //获取文件列表的文件信息
    //files: 文件列表
    //useFilter: 是否过滤后缀
    //filter: 过滤列表，如["*.wav","*.mp3"]
    void parsePathList(const QList<QString> &files,
                       bool useFilter = false,
                       const QStringList &filter = QStringList());
    //QtQuick.Dialogs使用的url存储路径
    void parseUrlList(const QList<QUrl> &files,
                      bool useFilter = false,
                      const QStringList &filter = QStringList());
    //获取目录下的音频文件信息
    //dir: 目录
    //useFilter: 是否过滤后缀
    //filter: 过滤列表，如["*.wav","*.mp3"]
    void parseDirPath(const QDir &dir,
                      bool useFilter = false,
                      const QStringList &filter = QStringList());
    //QtQuick.Dialogs使用的url存储路径
    void parseDirUrl(const QUrl &dir,
                     bool useFilter = false,
                     const QStringList &filter = QStringList());
    //【】转码
    //转码多个文件，转码后存放到缓存文件夹然后emit路径出去
    //（转码一般用提取完信息的文件，所以这列没加过滤参数）
    //files: 文件列表
    //dstdir: 生成后的保存目录，empty则放到cacheDir
    void transcodePathList(const QList<QString> &files,
                           const QString &dstdir = QString());
    //QtQuick.Dialogs使用的url存储路径
    void transcodeUrlList(const QList<QUrl> &files,
                          const QString &dstdir = QString());
    //单个文件转换编码，非线程
    EasyAudioInfo transcodeFile(const QString &srcpath,
                                const QString &dstpath,
                                const QAudioFormat &format);
    //【】拼接
    //将多个文件转为指定格式并按顺序拼接
    //files: 文件列表
    //dstpath: 生成的路径
    void stitchPathList(const QList<QString> &files,
                        const QString &dstpath = QString());
    //QtQuick.Dialogs使用的url存储路径
    void stitchUrlList(const QList<QUrl> &files,
                       const QString &dstpath = QString());
    //【】停止操作
    //目前只是把标志位=false，处理逻辑去判断
    void stop();
    //【】设置转码后的目标格式
    //channels: 目标格式通道数
    //sampleRate: 目标格式采样率
    //sampleBit: 目标格式采样存储类型枚举，暂未定义
    //codec: 目标格式编码，暂固定为-wav
    void setTargetFormat(int channels, int sampleRate, int sampleBit, const QString &codec);

private:
    //后缀过滤
    QStringList supportedSuffixs;
    //数据缓存目录，默认为exe同级目录下easyCache文件夹
    QString cacheDir;
    //线程池
    QThreadPool taskPool;
    //该类为单任务版本，不会同时处理多个任务
    //而不是每次任务都构建一个新的watcher+thread
    QFutureWatcher<void> taskWatcher;
    //处理状态，正在执行任务则为true
    //用于判断终止操作
    std::atomic_bool onProcess = false;
    //待处理文件数
    std::atomic_int processCount = 0;
    //处理成功数
    std::atomic_int processSuccess = 0;
    //处理失败数
    std::atomic_int processFail = 0;

    //转码后的目标格式
    //主要设置channels、sampleRate、sampleBit、codec
    QAudioFormat targetFormat;
};

