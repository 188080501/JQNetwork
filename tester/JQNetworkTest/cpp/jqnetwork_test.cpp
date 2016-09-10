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
    auto serverSettings = JQNetworkServerSettingsSharedPointer( new JQNetworkServerSettings );
    auto connectPoolSettings = JQNetworkConnectPoolSettingsSharedPointer( new JQNetworkConnectPoolSettings );
    auto connectSettings = JQNetworkConnectSettingsSharedPointer( new JQNetworkConnectSettings );

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
    auto connectSettings = JQNetworkConnectSettingsSharedPointer( new JQNetworkConnectSettings );

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
                    {},
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
        JQNetworkConnectSharedPointer connect;

        JQNetworkConnect::createConnectByHostAndPort(
                    [ &connect ](const auto &connect_){ connect = connect_; },
                    {},
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
        JQNetworkConnectSharedPointer connect;

        connectSettings->maximumConnectToHostWaitTime = 1;

        JQNetworkConnect::createConnectByHostAndPort(
                    [ &connect ](const auto &connect_){ connect = connect_; },
                    {},
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
    {
        QCOMPARE( JQNetworkPackage::headSize(), 24 );
        QCOMPARE( JQNetworkPackage::checkDataIsReadyReceive( QByteArray::fromHex( "" ) ), 24 );
        QCOMPARE( JQNetworkPackage::checkDataIsReadyReceive( QByteArray::fromHex( "7e 01 04030201 01 02000000 02000000 01 03000000 03000000" ) ), -1 );
        QCOMPARE( JQNetworkPackage::checkDataIsReadyReceive( QByteArray::fromHex( "7d 00 04030201 00 02000000 02000000 00 03000000 03000000" ) ), -1 );
        QCOMPARE( JQNetworkPackage::checkDataIsReadyReceive( QByteArray::fromHex( "7d 01 04030201 01 02000000 02000000 01 03000000 03000000" ) ), 5 );
        QCOMPARE( JQNetworkPackage::checkDataIsReadyReceive( QByteArray::fromHex( "7d 01 04030201 01 02000000 02000000 01 03000000 03000000 112233" ) ), 2 );
        QCOMPARE( JQNetworkPackage::checkDataIsReadyReceive( QByteArray::fromHex( "7d 01 04030201 01 02000000 02000000 01 03000000 03000000 1122334455" ) ), 0 );

        {
            auto rawData = QByteArray::fromHex( "7d 01 04030201 01 02000000 02000000 01 03000000 03000000 1122334455" );
            const auto &&package = JQNetworkPackage::createPackageFromRawData( rawData );

            QCOMPARE( rawData.size(), 0 );
            QCOMPARE( package->isCompletePackage(), true );
            QCOMPARE( package->isAbandonPackage(), false );
            QCOMPARE( (int)package->bootFlag(), 0x7d );
            QCOMPARE( (int)package->versionFlag(), 0x1 );
            QCOMPARE( package->randomFlag(), 0x01020304 );
            QCOMPARE( (int)package->metaDataFlag(), 0x1 );
            QCOMPARE( package->metaDataTotalSize(), 0x2 );
            QCOMPARE( package->metaDataCurrentSize(), 2 );
            QCOMPARE( (int)package->payloadDataFlag(), 0x1 );
            QCOMPARE( package->payloadDataTotalSize(), 3 );
            QCOMPARE( package->payloadDataCurrentSize(), 3 );
            QCOMPARE( package->metaData(), QByteArray::fromHex( "1122" ) );
            QCOMPARE( package->payloadData(), QByteArray::fromHex( "334455" ) );
        }

        {
            auto rawData1 = QByteArray::fromHex( "7d 01 04030201 01 05000000 02000000 01 05000000 03000000 1122334455" );
            auto rawData2 = QByteArray::fromHex( "7d 01 04030201 01 05000000 03000000 01 05000000 02000000 1122334455" );
            auto rawData3 = QByteArray::fromHex( "7d 01 04030201 01 05000000 03000000 01 05000000 02000000 1122334455" );
            const auto &&package1 = JQNetworkPackage::createPackageFromRawData( rawData1 );
            const auto &&package2 = JQNetworkPackage::createPackageFromRawData( rawData2 );
            const auto &&package3 = JQNetworkPackage::createPackageFromRawData( rawData3 );

            QCOMPARE( package1->isCompletePackage(), false );
            QCOMPARE( package1->isAbandonPackage(), false );
            QCOMPARE( package2->isCompletePackage(), false );
            QCOMPARE( package2->isAbandonPackage(), false );

            QCOMPARE( package1->mixPackage( package2 ), true );
            QCOMPARE( package1->isCompletePackage(), true );
            QCOMPARE( package1->isAbandonPackage(), false );
            QCOMPARE( package1->metaData(), QByteArray::fromHex( "1122112233" ) );
            QCOMPARE( package1->payloadData(), QByteArray::fromHex( "3344554455" ) );

            QCOMPARE( package1->mixPackage( package3 ), false );
            QCOMPARE( package1->isAbandonPackage(), true );
            QCOMPARE( package3->isAbandonPackage(), true );
        }
    }
}
