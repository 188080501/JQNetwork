#include "jqnetwork_test.h"

// Qt lib import
#include <QtTest>
#include <QtConcurrent>
#include <QTcpSocket>

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
    QMutex mutex;
    QMap< QThread *, int > flag;
    flag[ QThread::currentThread() ] =  2;

    {
        JQNetworkThreadPool threadPool( 3 );

        threadPool.runEach( [ & ](){ mutex.lock(); ++flag[ QThread::currentThread() ]; mutex.unlock(); } );
        threadPool.runEach( [ & ](){ mutex.lock(); ++flag[ QThread::currentThread() ]; mutex.unlock(); } );
    }

    QCOMPARE( flag.size(), 4 );
    for ( auto value: flag )
    {
        QCOMPARE( value, 2 );
    }

    {
        JQNetworkThreadPool threadPool( 1 );
        int flag = 0;

        threadPool.waitRun( [ & ](){ ++flag; } );
        QCOMPARE( flag, 1 );
    }
}

void JQNetworkTest::jqNetworkThreadPoolBenchmark()
{
    int number = 0;

    QBENCHMARK_ONCE
    {
        JQNetworkThreadPool threadPool( 3 );

        for ( auto count = 0; count < 10000000; ++count )
        {
            threadPool.run( [ & ](){ ++number; } );
        }
    }

    qDebug() << number;
    QCOMPARE( ( number != 10000000 ), true );
}

void JQNetworkTest::jqNetworkThreadPoolBenchmark2()
{
    int number = 0;

    QBENCHMARK_ONCE
    {
        JQNetworkThreadPool threadPool( 3 );

        for ( auto count = 0; count < 30000; ++count )
        {
            threadPool.waitRunEach( [ & ](){ ++number; } );
        }
    }

    qDebug() << number;
    QCOMPARE( number, 90000 );
}

void JQNetworkTest::jqNetworkServerTest()
{
    auto serverSettings = QSharedPointer< JQNetworkServerSettings >( new JQNetworkServerSettings );
    auto connectPoolSettings = QSharedPointer< JQNetworkConnectPoolSettings >( new JQNetworkConnectPoolSettings );
    auto connectSettings = QSharedPointer< JQNetworkConnectSettings >( new JQNetworkConnectSettings );

    serverSettings->listenPort = 42821;

    JQNetworkServer server( serverSettings, connectPoolSettings, connectSettings );

    QCOMPARE( server.begin(), true );

    int succeedCount = 0;

    QtConcurrent::run(
                [
                    &succeedCount
                ]()
                {
                    QTcpSocket socket;
                    socket.connectToHost( "127.0.0.1", 42821 );
                    succeedCount += socket.waitForConnected();

                    for ( auto count = 0; count < 3; ++count )
                    {
                        QThread::msleep( 100 );
                        socket.write( "Hello,JQNetwork!" );
                        socket.waitForBytesWritten();
                    }
                }
    );

    QThread::sleep( 1 );
    QCOMPARE( succeedCount, 1 );
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
                    [](const auto &){},
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
        QSharedPointer< JQNetworkConnect > connect;

        JQNetworkConnect::createConnectByHostAndPort(
                    [ &connect ](const auto &connect_){ connect = connect_; },
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
        QSharedPointer< JQNetworkConnect > connect;

        connectSettings->maximumConnectToHostWaitTime = 1;

        JQNetworkConnect::createConnectByHostAndPort(
                    [ &connect ](const auto &connect_){ connect = connect_; },
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

void JQNetworkTest::jeNetworkPackageTest()
{
    //...
}
