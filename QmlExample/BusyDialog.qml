import QtQuick 2.12
import QtQuick.Controls 2.12

Dialog{
    id: control
    width: 200
    height: 200

    modal: true
    closePolicy: Popup.NoAutoClose
    anchors.centerIn: parent
    parent: Overlay.overlay
    padding: 0

    contentItem: BusyIndicator{
        width: 200
        height: 200
    }
}
