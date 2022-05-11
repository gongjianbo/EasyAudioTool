# EasyAudioTool （by 龚建波）
QT audio tool, including audio transcoding, playback and other functions, currently only supports Qt5. Qt音频工具，包含音频转码、播放等功能，目前仅支持Qt5。

# Environment （开发环境）
（2022-05-05）Win10 64bit + Qt5.15.2 + MSVC2019 64bit

# 3rd-info （第三方库信息）
下列为此项目携带的第三方库的信息，如有需要可替换为其他版本

### 1.silk：SILK_SDK_SRC_v1.0.9 
应用：silk音频解码

协议：（未知）

使用 MSVC2017-64bit 编译 release dll

官网被墙：https://developer.skype.com/silk/SILK_SDK_SRC_v1.0.9.zip 

可以在 github 或者 gitee 搜别人 fork 的版本，如：https://github.com/Arnie97/silk

### 2.ffmpeg：ffmpeg-n4.4.2-1-g8e98dfc57f-win64-lgpl-shared-4.4
应用：音视频编解码

协议：LGPL V3

使用编译好的 lgpl shared 版本（Auto-Build 2022-04-28 12:31）

老的下载地址没法用了：http://ffmpeg.zeranoe.com/builds/ 

新的下载地址（注意区分 GPL 和 LGPL）：https://github.com/BtbN/FFmpeg-Builds/releases

### 3.sonic
应用：音频变速

协议：Apache License V2.0

使用 MSVC2017-64bit 编译 release dll

项目地址：https://github.com/waywardgeek/sonic

commit id：e06dbb9243024dc32ee911271fabedb02a2d2f8a（2021-10-16 08：28：02）