import QtQuick 2.12
import EasyAudio 1.0

//播放器
//适配原来的接口
Item {
    id: control

    //target用于区分不同的播放按钮所属组件
    property string target: ""
    //wrap player的属性
    property alias filepath: player.filepath
    property alias playerState: player.playerState
    property alias isPlaying: player.isPlaying
    property alias position: player.position

    //播放组件
    EasyAudioPlayer{
        id: player;
    }

    //播放
    function play(path){
        player.play(path);
    }

    //暂停/恢复
    function pause(){
        player.pause();
    }

    //停止
    function stop(){
        player.stop();
    }
}
