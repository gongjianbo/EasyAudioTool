#include "EasyTest.h"
#include "EasyAudioTool.h"
#include <thread>
#include <QDebug>

EasyTest::EasyTest(QObject *parent)
    : QObject(parent)
{

}

void EasyTest::transcodeRun(const QString &filepath)
{
    QString outpath = filepath + "_trans.wav";
    QAudioFormat format = EasyAudioTool::defaultFormat();
    runFlag = true;
    qDebug()<<__FUNCTION__;
    std::thread th([=]{
        qDebug()<<"start trans";
        bool ret = EasyAudioTool::transcodeToWavFile(filepath, outpath, format, runFlag);
        qDebug()<<"end trans"<<ret<<runFlag;
    });
    th.detach();
}

void EasyTest::transcodeCancel()
{
    runFlag = false;
    qDebug()<<__FUNCTION__;
}
