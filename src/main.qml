import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.5

import com.ypp 1.0

Item {
    width: 640
    height: 480
    visible: true

    Text {
        id: name
        text: qsTr("text")
    }

    Render {
        x: 100
        width: 100
        height: 100
    }

    Button {
        anchors.centerIn: parent
        onClicked: {
            name.text = "hahaha"
        }
    }
}
