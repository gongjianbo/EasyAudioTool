import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import EasyAudioTool 1.0

Window {
    id: root
    width: 800
    height: 500
    visible: true

    EasyAudioPlayer {
        id: player
    }

    //测试流程
    Column {
        anchors.centerIn: parent
        spacing: 10
        Text {
            text: "path:"+player.filepath
        }
        Text {
            text: "pos:"+player.position
        }
        Button {
            text: "play"
            onClicked: {
                player.play('./audio/audio.mp3')
            }
        }
    }
}
