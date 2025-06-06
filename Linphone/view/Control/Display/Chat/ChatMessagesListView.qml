import QtQuick
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Controls.Basic as Control
import Linphone
import UtilsCpp
import SettingsCpp
import "qrc:/qt/qml/Linphone/view/Style/buttonStyle.js" as ButtonStyle
import "qrc:/qt/qml/Linphone/view/Control/Tool/Helper/utils.js" as Utils

ListView {
    id: mainItem
    property ChatGui chat
    property color backgroundColor
    spacing: Math.round(4 * DefaultStyle.dp)

    Component.onCompleted: {
        var index = chatMessageProxy.findFirstUnreadIndex()
        positionViewAtIndex(index, ListView.End)
        var chatMessage = chatMessageProxy.getChatMessageAtIndex(index)
        if (chatMessage && !chatMessage.core.isRead) chatMessage.core.lMarkAsRead()
    }

    onCountChanged: if (atYEnd) {
        var index = chatMessageProxy.findFirstUnreadIndex()
        mainItem.positionViewAtIndex(index, ListView.End)
        var chatMessage = chatMessageProxy.getChatMessageAtIndex(index)
        if (chatMessage && !chatMessage.core.isRead) chatMessage.core.lMarkAsRead()
    }
    
    Button {
        visible: !mainItem.atYEnd
        icon.source: AppIcons.downArrow
        leftPadding: Math.round(16 * DefaultStyle.dp)
        rightPadding: Math.round(16 * DefaultStyle.dp)
        topPadding: Math.round(16 * DefaultStyle.dp)
        bottomPadding: Math.round(16 * DefaultStyle.dp)
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.bottomMargin: Math.round(18 * DefaultStyle.dp)
        anchors.rightMargin: Math.round(18 * DefaultStyle.dp)
        onClicked: {
            var index = chatMessageProxy.findFirstUnreadIndex()
            mainItem.positionViewAtIndex(index, ListView.End)
            var chatMessage = chatMessageProxy.getChatMessageAtIndex(index)
            if (chatMessage && !chatMessage.core.isRead) chatMessage.core.lMarkAsRead()
        }
    }

    model: ChatMessageProxy {
        id: chatMessageProxy
        chatGui: mainItem.chat
    }

    header: Item {
        height: Math.round(18 * DefaultStyle.dp)
    }

    delegate: ChatMessage {
        chatMessage: modelData
        property real maxWidth: Math.round(mainItem.width * (3/4))
        // height: childrenRect.height
        width: mainItem.width
        property var previousIndex: index - 1
        property var previousFromAddress: chatMessageProxy.getChatMessageAtIndex(index-1)?.core.fromAddress
        backgroundColor: isRemoteMessage ? DefaultStyle.main2_100 : DefaultStyle.main1_100
        isFirstMessage: !previousFromAddress || previousFromAddress !== modelData.core.fromAddress
        anchors.right: !isRemoteMessage && parent
            ? parent.right
            : undefined

        onMessageDeletionRequested: modelData.core.lDelete()
    }

    footerPositioning: ListView.OverlayFooter
    footer: Control.Control {
        visible: composeLayout.composingName !== ""
        width: mainItem.width
        z: mainItem.z + 2
        topPadding: Math.round(5 * DefaultStyle.dp)
        bottomPadding: Math.round(5 * DefaultStyle.dp)
        background: Rectangle {
            anchors.fill: parent
            color: mainItem.backgroundColor
        }
        contentItem: RowLayout {
            id: composeLayout
            property string composingName: mainItem.chat.core.composingName
            Avatar {
                Layout.preferredWidth: Math.round(20 * DefaultStyle.dp)
                Layout.preferredHeight: Math.round(20 * DefaultStyle.dp)
                _address: mainItem.chat.core.composingAddress
            }
            Text {
                Layout.fillWidth: true
                font {
                    pixelSize: Typography.p3.pixelSize
                    weight: Typography.p3.weight
                }
                //: %1 is writing…
                text: qsTr("%1 est en train d'écrire…").arg(composeLayout.composingName)
            }
        }
    }
}