import QtQuick 2.12
import QtQml 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2
import EasyAudioTool 1.0

//音频列表
ListView {
    id: control

    EasyModel {
        id: easy_model
    }

    EasyAudioPlayer {
        id: easy_player
    }

    clip: true
    spacing: 2

    property int columnCount: 11

    model: easy_model

    headerPositioning: ListView.OverlayHeader
    header: Rectangle{
        z: 2
        width: control.width
        height: 40
        color: "#999999"
        Row{
            anchors.verticalCenter: parent.verticalCenter
            Text{
                width: control.width/columnCount*2
                text: "filename"
            }
            Text{
                width: control.width/columnCount
                text: "filesize"
            }
            Text{
                width: control.width/columnCount
                text: "format"
            }
            Text{
                width: control.width/columnCount
                text: "encode"
            }
            Text{
                width: control.width/columnCount
                text: "channels"
            }
            Text{
                width: control.width/columnCount
                text: "sampleRate"
            }
            Text{
                width: control.width/columnCount
                text: "sampleBit"
            }
            Text{
                width: control.width/columnCount
                text: "bitRate"
            }
            Text{
                width: control.width/columnCount
                text: "duration"
            }
            Text{
                width: control.width/columnCount
                text: "operate"
            }
        }
    }

    delegate: Rectangle{
        width: control.width
        height: 40
        color: "#EEEEEE"
        Row{
            anchors.verticalCenter: parent.verticalCenter
            Text{
                width: control.width/columnCount*2
                text: model.filename
            }
            Text{
                width: control.width/columnCount
                text: model.filesize
            }
            Text{
                width: control.width/columnCount
                text: model.format
            }
            Text{
                width: control.width/columnCount
                text: model.encode
            }
            Text{
                width: control.width/columnCount
                text: model.channels
            }
            Text{
                width: control.width/columnCount
                text: model.sampleRate
            }
            Text{
                width: control.width/columnCount
                text: model.sampleBit
            }
            Text{
                width: control.width/columnCount
                text: model.bitRate
            }
            Text{
                width: control.width/columnCount
                text: model.duration
            }
            Row{
                anchors.verticalCenter: parent.verticalCenter
                width: control.width/columnCount
                height: 1
                spacing: 5
                Button{
                    anchors.verticalCenter: parent.verticalCenter
                    width: 46
                    height: 26
                    text: "play"
                    onClicked: {
                        easy_player.filepath=model.filepath;
                        easy_player.play();
                    }
                }
                Button{
                    anchors.verticalCenter: parent.verticalCenter
                    width: 46
                    height: 26
                    text: "stop"
                    onClicked: {
                        easy_player.stop();
                    }
                }
            }
        }
    }

    FileDialog{
        id: easy_dilaog
        title: "select dir"
        selectExisting: true
        selectFolder: true
        onAccepted: {
            console.log(folder);
            easy_model.parseUrl(folder,["*.wav","*.mp3"]);
        }
    }

    footerPositioning: ListView.OverlayFooter
    footer: Rectangle{
        z: 2
        width: control.width
        height: 40
        color: "#999999"

        Row{
            x: 10
            anchors.verticalCenter: parent.verticalCenter
            spacing: 10
            Button{
                text: "select dir"
                onClicked: {
                    easy_dilaog.open();
                }
            }
            Text{
                text: '["*.wav","*.mp3"]'
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    ScrollBar.vertical: ScrollBar {
        id: scroll_vertical
        z: 10
        //visible: true
        implicitWidth: 18
        contentItem: Rectangle{
            visible: (scroll_vertical.size<1.0)
            implicitWidth: 18
            radius: 9
            color: (scroll_vertical.hovered||scroll_vertical.pressed)
                   ? "#555555"
                   : "#888888"
        }
    }

    BusyDialog {
        id: busy_dialog
        visible: easy_model.taskRunning
    }
}
