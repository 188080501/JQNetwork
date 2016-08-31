#include "jqnetwork_test.h"

// Qt lib import
#include <QtTest>

// JQNetwork lib import
#include <JQNetworkFoundation>
#include <JQNetworkEncrypt>
#include <JQNetworkPackage>
#include <JQNetworkConnect>
#include <JQNetworkConnectPool>
#include <JQNetworkServer>
#include <JQNetworkProcessor>
#include <JQNetworkClient>
#include <JQNetworkForwarf>

void JQNetworkTest::jqnetworkConnectTest()
{
//    QBENCHMARK
//    QCOMPARE

    auto connectSettings = QSharedPointer< JQNetworkConnectSettings >( new JQNetworkConnectSettings );

    connectSettings->connectToHostErrorCallback = [ ]( auto ){ qDebug( "connectToHostErrorCallback" ); };
    connectSettings->connectToHostTimeoutCallback = [ ]( auto ){ qDebug( "connectToHostTimeoutCallback" ); };
    connectSettings->connectToHostSucceedCallback = [ ]( auto ){ qDebug( "connectToHostSucceedCallback" ); };
    connectSettings->remoteHostClosedCallback = [ ]( auto ){ qDebug( "remoteHostClosedCallback" ); };
    connectSettings->readyToDeleteCallback = [ ]( auto ){ qDebug( "readyToDeleteCallback" ); };

    static auto connect = JQNetworkConnect::createConnectByHostAndPort(
                connectSettings,
                "121.40.189.164",
//                { },
                23415
            );
}
