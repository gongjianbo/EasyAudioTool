import QtQuick 2.12
import QtQuick.Controls 2.12
import Test 1.0

Item {
    id: control

    EasyTest{
        id: test
    }

    Row{
        x: 10
        y: 10
        spacing: 10
        Button{
            text: "解码测试"
            onClicked: {
                test.testDecoder();
            }
        }
        Button{
            text: "播放测试"
            onClicked: {
                test.testPlayer();
            }
        }
    }
}
