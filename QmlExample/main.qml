import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import EasyAudioTool 1.0

Window {
    id: root
    width: 960
    height: 640
    visible: true

    EasyTest {
        id: easy_test
    }

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

        header: Rectangle {
            width: ListView.view.width
            height: 70
            radius: 4
            border.color: "gray"

            //测试转码
            Row {
                anchors.centerIn: parent
                spacing: 10
                TextField {
                    id: path_edit
                    selectByMouse: true
                }
                Button {
                    text: "translate"
                    onClicked: {
                        easy_test.transcodeRun(path_edit.text);
                    }
                }
                Button {
                    text: "cancel"
                    onClicked: {
                        easy_test.transcodeCancel();
                    }
                }
            }
        }

        delegate: Rectangle {
            width: ListView.view.width
            height: 70
            radius: 4
            border.color: "gray"

            EasyAudioPlayer {
                id: player
                filepath: modelData
                property real duration: 0
                Component.onCompleted: {
                    player.duration = player.getDuration();
                }
                onPositionChanged: {
                    if(!slider.pressed){
                        slider.value = position;
                    }
                }
            }

            //测试流程
            Row {
                x: 20
                y: 10
                spacing: 10
                Text {
                    width: 200
                    elide: Text.ElideRight
                    text: "path:"+player.filepath
                }
                Text {
                    width: 120
                    elide: Text.ElideRight
                    text: "pos:"+player.position
                }
                Text {
                    width: 120
                    elide: Text.ElideRight
                    text: "len:"+player.duration
                }
                Button {
                    text: switch(player.playerState)
                          {
                          case EasyAudio.Playing: return "pause";
                          case EasyAudio.Paused: return "resume";
                          default: return "play";
                          }
                    onClicked: {
                        switch(player.playerState)
                        {
                        case EasyAudio.Playing:
                            //播放或者暂停后继续
                            player.pause();
                            break;
                        case EasyAudio.Paused:
                            //暂停播放
                            player.play();
                            break;
                        default:
                            //播放或者暂停后继续
                            player.play();
                            break;
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
                ComboBox {
                    id: speed_box
                    model: ['0.5','0.75','1.0','1.25','1.5','2.0','3.0']
                    property var speedArray: [50,75,100,125,150,200,300]
                    currentIndex: 2
                    onCurrentIndexChanged: {
                        player.playSpeed = speedArray[currentIndex];
                    }
                }
            }

            Slider {
                id: slider
                x: 20
                width: 400
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 10
                from: 0
                to: player.duration
                //value: player.position
                onPressedChanged: {
                    if(!pressed){
                        //console.log('seek',value)
                        player.seek(value)
                    }
                }
            }
        }
    }
}
