import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Easy Audio Component (by GongJianBo 1992)")

    TabBar {
        id: bar
        width: parent.width
        padding: 1
        TabButton {
            text: qsTr("Audio")
        }
        TabButton {
            text: qsTr("Test")
        }
        background: Rectangle{
            border.color: "black"
        }
    }

    StackLayout {
        anchors.fill: parent
        anchors.topMargin: bar.height+1
        currentIndex: bar.currentIndex
        AudioList{
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 10
        }
        AudioTest{
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 10
        }
    }
}
