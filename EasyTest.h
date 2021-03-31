#ifndef EASYTEST_H
#define EASYTEST_H

#include <QObject>

//本地测试
struct EasyTest
{
    void TestAll();

    void FFmpegRead(const QString &filepath);
    void FFmpegReadAll(const QString &filepath);

    void SilkRead(const QString &filepath);
    void SilkReadAll(const QString &filepath);

    void FactoryContext(const QString &filepath);
    void FactoryDecoder(const QString &filepath);
};

#endif // EASYTEST_H
