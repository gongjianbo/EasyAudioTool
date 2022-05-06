import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import EasyAudioTool 1.0

Window {
    id: root
    width: 800
    height: 500
    visible: true

    ListView {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10

        model: [
            './audio/audio.flac',
            './audio/audio.mp3',
            './audio/audio.wav',
            './audio/weixin.amr'
        ]

        delegate: Rectangle {
            width: ListView.view.width
            height: 40

            EasyAudioPlayer {
                id: player
                filepath: modelData
            }

            //测试流程
            Row {
                anchors.centerIn: parent
                spacing: 10
                Text {
                    width: 200
                    elide: Text.ElideRight
                    text: "path:"+player.filepath
                }
                Text {
                    width: 150
                    elide: Text.ElideRight
                    text: "pos:"+player.position
                }
                Button {
                    text: player.onPlaying ? "pause" : "play"
                    onClicked: {
                        if(player.onPlaying){
                            //暂停播放
                            player.pause();
                        }else{
                            //播放或者暂停后继续
                            player.play();
                        }
                    }
                }
                Button {
                    text: "stop"
                    onClicked: {
                        //停止
                        player.stop();
                    }
                }
            }
        }
    }
}
