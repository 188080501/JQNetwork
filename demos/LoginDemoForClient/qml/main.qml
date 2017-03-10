import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Window 2.2
import JQNetwork 1.0

Window {
    visible: true
    width: 320
    height: 480
    title: "LoginDemo"

    JQNetworkClient {
        id: networkClient

        onConnectToHostError: {
            print( "onConnectToHostError:", hostName, port );

            textForMessage.text = "ConnectToHostError";
        }

        onConnectToHostTimeout: {
            print( "onConnectToHostTimeout:", hostName, port );

            textForMessage.text = "ConnectToHostTimeout";
        }

        onConnectToHostSucceed: {
            print( "onConnectToHostSucceed:", hostName, port );

            textForMessage.text = "ConnectToHostSucceed";
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
                        "127.0.0.1",                                // 服务端的IP地址
                        23456,                                      // 服务端的端口
                        "userLogin",                                // 需要调用的方法名称
                        {                                           // \
                            username: textEditForUsername.text,     //  |--> 传给服务端的参数
                            password: textEditForPassword.text      //  |
                        },                                          // /
                        onLoginSucceed,                             // 当请求执行成功时的回调，这里可以接受匿名回调作为参数
                        onLoginFail                                 // 当请求执行失败时的回调，这里可以接受匿名回调作为参数
                    );
        }

        // 请求成功，也就是传输的数据已经到达服务端，会调用这个回调
        function onLoginSucceed(received) {
            if ( received[ "succeed" ] )
            {
                textForMessage.text = "LoginSucceed";
            }
            else
            {
                textForMessage.text = "LoginFail: Message" + received[ "message" ];
            }
        }

        // 请求失败，也就是传输的数据未到达幅度按，会调用这个回调
        function onLoginFail() {
            textForMessage.text = "LoginFail";
        }
    }

    Text {
        id: textForMessage
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 50
    }
}
