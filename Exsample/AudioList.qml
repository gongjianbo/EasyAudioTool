import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2
import EasyAudio 1.0
import Test 1.0

//音频列表
ListView {
    id: control

    EasyAudioPlayer{
        id: easy_player
    }

    EasyModel{
        id: easy_model
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
            RowLabel{
                width: control.width/columnCount*2
                text: "filename"
            }
            RowLabel{
                width: control.width/columnCount
                text: "filesize"
            }
            RowLabel{
                width: control.width/columnCount
                text: "format"
            }
            RowLabel{
                width: control.width/columnCount
                text: "encode"
            }
            RowLabel{
                width: control.width/columnCount
                text: "channels"
            }
            RowLabel{
                width: control.width/columnCount
                text: "sampleRate"
            }
            RowLabel{
                width: control.width/columnCount
                text: "sampleBit"
            }
            RowLabel{
                width: control.width/columnCount
                text: "bitRate"
            }
            RowLabel{
                width: control.width/columnCount
                text: "duration"
            }
            RowLabel{
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
            RowLabel{
                width: control.width/columnCount*2
                text: model.filename
            }
            RowLabel{
                width: control.width/columnCount
                text: model.filesize
            }
            RowLabel{
                width: control.width/columnCount
                text: model.format
            }
            RowLabel{
                width: control.width/columnCount
                text: model.encode
            }
            RowLabel{
                width: control.width/columnCount
                text: model.channels
            }
            RowLabel{
                width: control.width/columnCount
                text: model.sampleRate
            }
            RowLabel{
                width: control.width/columnCount
                text: model.sampleBit
            }
            RowLabel{
                width: control.width/columnCount
                text: model.bitRate
            }
            RowLabel{
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
                    onClicked: easy_player.play(model.filepath);
                }
                Button{
                    anchors.verticalCenter: parent.verticalCenter
                    width: 46
                    height: 26
                    text: "stop"
                    onClicked: easy_player.stop();
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
            easy_model.parseUrl(folder);
        }
    }

    footerPositioning: ListView.OverlayFooter
    footer: Rectangle{
        z: 2
        width: control.width
        height: 40
        color: "#999999"

        Button{
            text: "select dir"
            onClicked: {
                easy_dilaog.open();
            }
        }
    }
}
