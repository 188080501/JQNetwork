import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Window 2.2
import JQNetwork 1.0

Window {
    visible: true
    width: 320
    height: 480
    title: "LoginDemo"

    JQNetworkClientForQml {
        id: networkClient

        onConnectToHostError: {
            print( "onConnectToHostError:", hostName, port );

            text.text = "ConnectToHostError";
        }

        onConnectToHostTimeout: {
            print( "onConnectToHostTimeout:", hostName, port );

            text.text = "ConnectToHostTimeout";
        }

        onConnectToHostSucceed: {
            print( "onConnectToHostSucceed:", hostName, port );

            text.text = "ConnectToHostSucceed";
        }

        Component.onCompleted: {
            var beginClientSucceed = networkClient.beginClient();
            print( "JQNetworkClientForQml::beginClient:", beginClientSucceed );
            if ( !beginClientSucceed ) { return; }

            networkClient.createConnect( "127.0.0.1", 23456 );
        }
    }

    TextEdit {
        id: textEditForUsername
        text: "Username"
        verticalAlignment: Text.AlignVCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -75

        Rectangle {
            anchors.fill: parent
            anchors.margins: -10
            color: "transparent"
            border.width: 1
        }

        Text {
            anchors.right: parent.left
            anchors.rightMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            text: "Username:"
        }
    }

    TextEdit {
        id: textEditForPassword
        text: "123456"
        verticalAlignment: Text.AlignVCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 0

        Rectangle {
            anchors.fill: parent
            anchors.margins: -10
            color: "transparent"
            border.width: 1
        }

        Text {
            anchors.right: parent.left
            anchors.rightMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            text: "Password:"
        }
    }

    Button {
        text: "Login"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 75

        onClicked: {
            networkClient.sendPayloadData(
                        "127.0.0.1",
                        23456,
                        "userLogin",
                        { },
                        onLoginSucceed,
                        onLoginFail
                    );
        }

        function onLoginSucceed() {
            print( "onLoginSucceed" );
        }

        function onLoginFail() {
            print( "onLoginFail" );
        }
    }

    Text {
        id: text
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 50
    }
}
