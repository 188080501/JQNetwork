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

void JQNetworkTest::jqNetworkThreadPoolTest()
{
    QMap< QThread *, int > flag;
    flag[ QThread::currentThread() ] =  2;

    {
        JQNetworkThreadPool threadPool( 3 );

        threadPool.run( [ & ](){ ++flag[ QThread::currentThread() ]; } );
        threadPool.run( [ & ](){ ++flag[ QThread::currentThread() ]; } );
        threadPool.run( [ & ](){ ++flag[ QThread::currentThread() ]; } );
        threadPool.run( [ & ](){ ++flag[ QThread::currentThread() ]; } );
        threadPool.run( [ & ](){ ++flag[ QThread::currentThread() ]; } );
        threadPool.run( [ & ](){ ++flag[ QThread::currentThread() ]; } );
    }

    QCOMPARE( flag.size(), 4 );
    for ( auto value: flag )
    {
        QCOMPARE( value, 2 );
    }
}

void JQNetworkTest::jqNetworkConnectTest()
{
    auto connectSettings = QSharedPointer< JQNetworkConnectSettings >( new JQNetworkConnectSettings );

    bool flag1 = false;
    bool flag2 = false;
    bool flag3 = false;
    bool flag4 = false;
    bool flag5 = false;

    connectSettings->connectToHostErrorCallback   = [ & ]( auto ){ flag1 = true; qDebug( "connectToHostErrorCallback" ); };
    connectSettings->connectToHostTimeoutCallback = [ & ]( auto ){ flag2 = true; qDebug( "connectToHostTimeoutCallback" ); };
    connectSettings->connectToHostSucceedCallback = [ & ]( auto ){ flag3 = true; qDebug( "connectToHostSucceedCallback" ); };
    connectSettings->remoteHostClosedCallback     = [ & ]( auto ){ flag4 = true; qDebug( "remoteHostClosedCallback" ); };
    connectSettings->readyToDeleteCallback        = [ & ]( auto ){ flag5 = true; qDebug( "readyToDeleteCallback" ); };

    {
        JQNetworkConnect::createConnectByHostAndPort(
                    connectSettings,
                    "127.0.0.1",
                    0
                );
    }

    QCOMPARE( flag1, true );
    QCOMPARE( flag2, false );
    QCOMPARE( flag3, false );
    QCOMPARE( flag4, false );
    QCOMPARE( flag5, true );

    flag1 = false;
    flag2 = false;
    flag3 = false;
    flag4 = false;
    flag5 = false;

    {
        auto connect = JQNetworkConnect::createConnectByHostAndPort(
                    connectSettings,
                    "www.baidu.com",
                    80
                );
        QEventLoop eventLoop;
        QTimer::singleShot( 3000, &eventLoop, &QEventLoop::quit );
        eventLoop.exec();
    }

    QCOMPARE( flag1, false );
    QCOMPARE( flag2, false );
    QCOMPARE( flag3, true );
    QCOMPARE( flag4, false );
    QCOMPARE( flag5, true );

    flag1 = false;
    flag2 = false;
    flag3 = false;
    flag4 = false;
    flag5 = false;

    {
        connectSettings->maximumConnectToHostWaitTime = 1;
        auto connect = JQNetworkConnect::createConnectByHostAndPort(
                    connectSettings,
                    "www.baidu.com",
                    80
                );
        QEventLoop eventLoop;
        QTimer::singleShot( 3000, &eventLoop, &QEventLoop::quit );
        eventLoop.exec();
        connectSettings->maximumConnectToHostWaitTime = 15 * 1000;
    }

    QCOMPARE( flag1, false );
    QCOMPARE( flag2, true );
    QCOMPARE( flag3, false );
    QCOMPARE( flag4, false );
    QCOMPARE( flag5, true );
}
