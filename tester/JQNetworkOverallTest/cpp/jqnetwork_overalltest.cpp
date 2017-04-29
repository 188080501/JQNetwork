#include "jqnetwork_overalltest.h"

// Qt lib import
#include <QtTest>
#include <QTime>
#include <QtConcurrent>
#include <QTcpSocket>
#include <QTcpServer>

// JQNetwork lib import
#include <JQNetwork>

// Project lib import
#include "processortest1.hpp"
#include "processortest2.hpp"
#include "fusiontest1.hpp"

void JQNetworkOverallTest::jqNetworkThreadPoolTest()
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

    {
        QMap< QThread *, int > flags;

        JQNetworkThreadPool threadPool( 3 );

        threadPool.waitRunEach( [ &flags ](){ flags[ QThread::currentThread() ] = 0; } );

        for ( auto count = 0; count < 100000; ++count )
        {
            threadPool.run( [ &flags ](){ ++flags[ QThread::currentThread() ]; } );
        }

        QThread::msleep( 1000 );

        QCOMPARE( ( *( flags.begin() + 0 ) + *( flags.begin() + 1 ) + *( flags.begin() + 2 ) ), 100000 );
    }
}

void JQNetworkOverallTest::jqNetworkThreadPoolBenchmark1()
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

//    qDebug() << number;

    if ( QThreadPool::globalInstance()->maxThreadCount() > 1 )
    {
        QCOMPARE( ( number != 10000000 ), true );
    }
    else
    {
        QCOMPARE( ( 8000000 <= number ) && ( number <= 10000000 ), true );
    }
}

void JQNetworkOverallTest::jqNetworkThreadPoolBenchmark2()
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

//    qDebug() << number;
    QCOMPARE( number, 90000 );
}

void JQNetworkOverallTest::jqNetworkNodeMarkTest()
{
    JQNetworkNodeMark nodeMark( "test" );

//    qDebug() << "applicationStartTime:" << nodeMark.applicationStartTime();
//    qDebug() << "applicationFilePath:" << nodeMark.applicationFilePath();
//    qDebug() << "localHostName:" << nodeMark.localHostName();
//    qDebug() << "nodeMarkCreatedTime:" << nodeMark.nodeMarkCreatedTime();
//    qDebug() << "nodeMarkClassAddress:" << nodeMark.nodeMarkClassAddress();
//    qDebug() << "dutyMark:" << nodeMark.dutyMark();
//    qDebug() << "nodeMarkSummary:" << nodeMark.nodeMarkSummary();

    QCOMPARE( nodeMark.applicationStartTime() > 0, true );
    QCOMPARE( nodeMark.applicationFilePath().isEmpty(), false );
    QCOMPARE( nodeMark.localHostName().isEmpty(), false );
    QCOMPARE( nodeMark.nodeMarkCreatedTime() > 0, true );
    QCOMPARE( nodeMark.nodeMarkClassAddress().isEmpty(), false );
    QCOMPARE( nodeMark.dutyMark().isEmpty(), false );
    QCOMPARE( nodeMark.nodeMarkSummary().isEmpty(), false );

    const auto &&nodeMarkSummary1 = nodeMark.nodeMarkSummary();

    QThread::msleep( 50 );

    const auto &&nodeMarkSummary2 = JQNetworkNodeMark::calculateNodeMarkSummary( "test" );

    QCOMPARE( nodeMarkSummary1 != nodeMarkSummary2, true );
}

void JQNetworkOverallTest::jqNetworkConnectTest()
{
    auto connectSettings = JQNetworkConnectSettingsSharedPointer( new JQNetworkConnectSettings );

    bool flag1 = false;
    bool flag2 = false;
    bool flag3 = false;
    bool flag4 = false;
    bool flag5 = false;

    connectSettings->connectToHostErrorCallback   = [ & ](const auto &)
    {
        flag1 = true;
//        qDebug( "connectToHostErrorCallback" );
    };
    connectSettings->connectToHostTimeoutCallback = [ & ](const auto &)
    {
        flag2 = true;
//        qDebug( "connectToHostTimeoutCallback" );
    };
    connectSettings->connectToHostSucceedCallback = [ & ](const auto &)
    {
        flag3 = true;
//        qDebug( "connectToHostSucceedCallback" );
    };
    connectSettings->remoteHostClosedCallback     = [ & ](const auto &)
    {
        flag4 = true;
//        qDebug( "remoteHostClosedCallback" );
    };
    connectSettings->readyToDeleteCallback        = [ & ](const auto &)
    {
        flag5 = true;
//        qDebug( "readyToDeleteCallback" );
    };

    {
        JQNetworkConnect::createConnect(
                    [](const auto &){ },
                    { },
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

    const auto &&lanAddressEntries = JQNetworkLan::lanAddressEntries();
    if ( lanAddressEntries.isEmpty() || ( lanAddressEntries[ 0 ].ip == QHostAddress::LocalHost ) )
    {
        QSKIP( "lanAddressEntries is empty" );
    }

    flag1 = false;
    flag2 = false;
    flag3 = false;
    flag4 = false;
    flag5 = false;

    {
        JQNetworkConnectSharedPointer connect;

        JQNetworkConnect::createConnect(
                    [ &connect ](const auto &connect_){ connect = connect_; },
                    { },
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

        JQNetworkConnect::createConnect(
                    [ &connect ](const auto &connect_){ connect = connect_; },
                    { },
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

void JQNetworkOverallTest::jeNetworkPackageTest()
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
            const auto &&package = JQNetworkPackage::readPackage( rawData );

            QCOMPARE( rawData.size(), 0 );
            QCOMPARE( package->isCompletePackage(), true );
            QCOMPARE( package->isAbandonPackage(), false );
            QCOMPARE( (int)package->bootFlag(), 0x7d );
            QCOMPARE( (int)package->packageFlag(), 0x1 );
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
            const auto &&package1 = JQNetworkPackage::readPackage( rawData1 );
            const auto &&package2 = JQNetworkPackage::readPackage( rawData2 );
            const auto &&package3 = JQNetworkPackage::readPackage( rawData3 );

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

        {
            auto packages = JQNetworkPackage::createPayloadTransportPackages(
                        { }, // empty targetActionFlag
                        "12345",
                        { }, // empty appendData
                        1,
                        2
                    );
            QCOMPARE( packages.size(), 3 );

            auto package1 = packages.at( 0 );
            auto package2 = packages.at( 1 );
            auto package3 = packages.at( 2 );

            QCOMPARE( package1->randomFlag(), 1 );
            QCOMPARE( package2->randomFlag(), 1 );
            QCOMPARE( package3->randomFlag(), 1 );

            QCOMPARE( package1->payloadDataTotalSize(), 5 );
            QCOMPARE( package2->payloadDataTotalSize(), 5 );
            QCOMPARE( package3->payloadDataTotalSize(), 5 );

            QCOMPARE( package1->payloadDataCurrentSize(), 2 );
            QCOMPARE( package2->payloadDataCurrentSize(), 2 );
            QCOMPARE( package3->payloadDataCurrentSize(), 1 );

            QCOMPARE( package1->payloadData(), QByteArray( "12" ) );
            QCOMPARE( package2->payloadData(), QByteArray( "34" ) );
            QCOMPARE( package3->payloadData(), QByteArray( "5" ) );

            QCOMPARE( package1->payloadDataSize(), 2 );
            QCOMPARE( package2->payloadDataSize(), 2 );
            QCOMPARE( package3->payloadDataSize(), 1 );

            QCOMPARE( package2->isAbandonPackage(), false );
            QCOMPARE( package3->isAbandonPackage(), false );

            QCOMPARE( package1->isCompletePackage(), false );
            QCOMPARE( package1->mixPackage( package2 ), true );
            QCOMPARE( package1->isCompletePackage(), false );
            QCOMPARE( package1->mixPackage( package3 ), true );
            QCOMPARE( package1->isCompletePackage(), true );

            QCOMPARE( package2->isCompletePackage(), false );
            QCOMPARE( package3->isCompletePackage(), false );

            QCOMPARE( package1->payloadDataSize(), 5 );
            QCOMPARE( package1->payloadData(), QByteArray( "12345" ) );
        }

        {
            auto packages = JQNetworkPackage::createPayloadTransportPackages(
                        { }, // empty targetActionFlag
                        "12345",
                        { }, // empty appendData
                        2,
                        5
                   );
            QCOMPARE( packages.size(), 1 );

            auto package = packages.first();

            QCOMPARE( package->isAbandonPackage(), false );
            QCOMPARE( package->isCompletePackage(), true );
            QCOMPARE( package->payloadDataSize(), 5 );
            QCOMPARE( package->payloadData(), QByteArray( "12345" ) );
        }

        {
            auto packages = JQNetworkPackage::createPayloadTransportPackages(
                        { }, // empty targetActionFlag
                        "12345",
                        { }, // empty appendData
                        2,
                        5
                   );
            QCOMPARE( packages.size(), 1 );

            auto package = packages.first();

            QCOMPARE( package->isAbandonPackage(), false );
            QCOMPARE( package->isCompletePackage(), true );
            QCOMPARE( package->payloadDataSize(), 5 );
            QCOMPARE( package->payloadData(), QByteArray( "12345" ) );
        }

        {
            auto packages = JQNetworkPackage::createPayloadTransportPackages(
                        { }, // empty targetActionFlag
                        { }, // empty payloadData
                        { }, // empty appendData
                        2
                    );
            QCOMPARE( packages.size(), 1 );

            auto package = packages.first();

            QCOMPARE( package->isAbandonPackage(), false );
            QCOMPARE( package->isCompletePackage(), true );
            QCOMPARE( package->payloadDataSize(), 0 );
            QCOMPARE( package->payloadData(), QByteArray() );
        }
    }

    {
        auto package1 = JQNetworkPackage::createPayloadTransportPackages( { }, "Jason", { }, 1, -1, false ).first();
        auto package2 = JQNetworkPackage::createPayloadTransportPackages( { }, "Jason", { }, 1, -1, true ).first();

        QCOMPARE( package1->payloadDataFlag(), JQNETWORKPACKAGE_UNCOMPRESSEDFLAG );
        QCOMPARE( package2->payloadDataFlag(), JQNETWORKPACKAGE_COMPRESSEDFLAG );

        QCOMPARE( package1->payloadData() == "Jason", true );
        QCOMPARE( package2->payloadData() != "Jason", true );

        QCOMPARE( package1->payloadDataSize() == 5, true );
        QCOMPARE( package2->payloadDataSize() > 5, true );

        package2->refreshPackage();

        QCOMPARE( package1->payloadDataFlag(), package2->payloadDataFlag() );
        QCOMPARE( package1->payloadData(), package2->payloadData() );
        QCOMPARE( package1->payloadDataSize(), package2->payloadDataSize() );
    }

    {
        auto packagesForSource = JQNetworkPackage::createPayloadTransportPackages( { }, "12345", { }, 1, 1, true );

        QCOMPARE( packagesForSource.size(), 5 );

        auto packageForTarget = packagesForSource.first();
        packagesForSource.pop_front();

        packageForTarget->refreshPackage();

        for ( auto package: packagesForSource )
        {
            QCOMPARE( package->payloadDataSize() > 1, true );

            package->refreshPackage();

            QCOMPARE( packageForTarget->mixPackage( package ), true );
        }

        QCOMPARE( packageForTarget->isAbandonPackage(), false );
        QCOMPARE( packageForTarget->isCompletePackage(), true );
        QCOMPARE( packageForTarget->payloadData(), QByteArray( "12345" ) );
    }

    {
        auto packagesForSource = JQNetworkPackage::createPayloadTransportPackages( { }, "12345", QVariantMap( { { "key", "value" } } ), 1, 2, true );

        QCOMPARE( packagesForSource.size(), 3 );

        for ( auto index = 0; index < packagesForSource.size(); ++index )
        {
            if ( index )
            {
                QCOMPARE( packagesForSource[ index ]->metaDataCurrentSize(), 0 );
                QCOMPARE( packagesForSource[ index ]->metaData(), QByteArray() );
            }
            else
            {
                QCOMPARE( packagesForSource[ index ]->metaDataCurrentSize(), 52 );
                QCOMPARE( packagesForSource[ index ]->metaData(), QByteArray( "{\"appendData\":{\"key\":\"value\"},\"targetActionFlag\":\"\"}" ) );
            }
        }
    }
}

void JQNetworkOverallTest::jqNetworkServerTest()
{
    auto serverSettings = JQNetworkServerSettingsSharedPointer( new JQNetworkServerSettings );
    auto connectPoolSettings = JQNetworkConnectPoolSettingsSharedPointer( new JQNetworkConnectPoolSettings );
    auto connectSettings = JQNetworkConnectSettingsSharedPointer( new JQNetworkConnectSettings );

    {
        JQNetworkServer server( serverSettings, connectPoolSettings, connectSettings );
    }

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

void JQNetworkOverallTest::jqNetworkClientTest()
{
    bool flag1 = false;
    bool flag2 = false;

    int count1 = 0;

    QTcpServer tcpServer;
    QCOMPARE( tcpServer.listen( QHostAddress::Any, 12345 ), true );

    JQNetworkClientSettingsSharedPointer clientSettings( new JQNetworkClientSettings );
    JQNetworkConnectPoolSettingsSharedPointer connectPoolSettings( new JQNetworkConnectPoolSettings );
    JQNetworkConnectSettingsSharedPointer connectSettings( new JQNetworkConnectSettings );

    clientSettings->readyToDeleteCallback = [ &flag1, &count1 ](const auto &, const auto &, const auto &){ flag1 = true; ++count1; };

    {
        JQNetworkClient client( clientSettings, connectPoolSettings, connectSettings );
        client.begin();

        QCOMPARE( client.sendPayloadData( "127.0.0.1", 23456, { } ), 0 );
        QCOMPARE( client.waitForCreateConnect( "127.0.0.1", 23456 ), false );

        count1 = 0;

        for ( auto count = 0; count < 500; ++count )
        {
            client.createConnect( "127.0.0.1", 34567 + count );
        }

        QThread::msleep( 15 * 1000 );

        QCOMPARE( count1, 500 );
    }

    connectSettings->maximumReceivePackageWaitTime = 800;

    {
        JQNetworkClient client( clientSettings, connectPoolSettings, connectSettings );
        client.begin();

        flag1 = false;
        QCOMPARE( client.waitForCreateConnect( "Hello,world!", 12345 ), false );
        QThread::msleep( 200 );
        QCOMPARE( flag1, true );

        flag1 = false;
        QCOMPARE( client.waitForCreateConnect( "127.0.0.1", 12345 ), true );
        QCOMPARE( client.waitForCreateConnect( "127.0.0.1", 12345 ), true );
        QCOMPARE( client.waitForCreateConnect( "127.0.0.1", 12345 ), true );

        QThread::msleep( 200 );
        QCOMPARE( flag1, false );

        client.sendPayloadData( "127.0.0.1", 12345, "Test", nullptr, [ &flag2 ](const auto &){ flag2 = true; } );

        QThread::msleep( 100 );
        QCOMPARE( flag2, false );

        QThread::msleep( 1500 );
        QCOMPARE( flag2, true );
    }

    QCOMPARE( flag1, true );

    connectSettings->maximumReceivePackageWaitTime = 30 * 1000;
}

void JQNetworkOverallTest::jqNetworkServerAndClientTest1()
{
    QString serverFlag;
    QString clientFlag;
    QThread *serverProcessThread = nullptr;
    QThread *clientProcessThread = nullptr;

    auto server = JQNetworkServer::createServer( 23456 );
    server->connectSettings()->cutPackageSize = 2;
    server->serverSettings()->globalProcessorThreadCount = 1;
    server->serverSettings()->packageSendingCallback = [ &serverFlag, &serverProcessThread ](
            const auto &,
            const auto &randomFlag,
            const auto &payloadCurrentIndex,
            const auto &payloadCurrentSize,
            const auto &payloadTotalSize
        )
    {
//        qDebug() << "server packageSendingCallback:" << randomFlag << payloadCurrentIndex << payloadCurrentSize << payloadTotalSize;

        serverFlag += QString( "server packageSendingCallback: %1 %2 %3 %4\n" ).arg( randomFlag ).arg( payloadCurrentIndex ).arg( payloadCurrentSize ).arg( payloadTotalSize );

        QCOMPARE( QThread::currentThread(), serverProcessThread );
    };
    server->serverSettings()->packageReceivingCallback = [ &serverFlag, &serverProcessThread ](
            const auto &,
            const auto &randomFlag,
            const auto &payloadCurrentIndex,
            const auto &payloadCurrentSize,
            const auto &payloadTotalSize
        )
    {
//        qDebug() << "server packageReceivingCallback:" << randomFlag << payloadCurrentIndex << payloadCurrentSize << payloadTotalSize;

        serverFlag += QString( "server packageReceivingCallback: %1 %2 %3 %4\n" ).arg( randomFlag ).arg( payloadCurrentIndex ).arg( payloadCurrentSize ).arg( payloadTotalSize );

        if ( !serverProcessThread )
        {
            serverProcessThread = QThread::currentThread();
        }
        QCOMPARE( QThread::currentThread(), serverProcessThread );
    };
    server->serverSettings()->packageReceivedCallback = [ &serverFlag, &serverProcessThread ](
            const auto &connect,
            const auto &package
        )
    {
//        qDebug() << "server packageReceivedCallback:" << package->payloadData();

        serverFlag += QString( "server packageReceivedCallback: %1\n" ).arg( QString::fromLatin1( package->payloadData() ) );

        connect->replyPayloadData( package->randomFlag(), "67890" );
        connect->sendPayloadData( "abcd" );

        QCOMPARE( QThread::currentThread(), serverProcessThread );
    };
    QCOMPARE( server->begin(), true );

    auto client = JQNetworkClient::createClient();
    client->connectSettings()->cutPackageSize = 2;
    client->clientSettings()->globalProcessorThreadCount = 1;
    client->clientSettings()->packageSendingCallback = [ &clientFlag, &clientProcessThread ](
            const auto &,
            const auto &hostName,
            const auto &port,
            const auto &randomFlag,
            const auto &payloadCurrentIndex,
            const auto &payloadCurrentSize,
            const auto &payloadTotalSize
        )
    {
//        qDebug() << "client packageSendingCallback:" << hostName << port << randomFlag << payloadCurrentIndex << payloadCurrentSize << payloadTotalSize;

        clientFlag += QString( "client packageSendingCallback: %1 %2 %3 %4 %5 %6\n" ).arg( hostName ).arg( port ).arg( randomFlag ).arg( payloadCurrentIndex ).arg( payloadCurrentSize ).arg( payloadTotalSize );

        if ( !clientProcessThread )
        {
            clientProcessThread = QThread::currentThread();
        }
        QCOMPARE( QThread::currentThread(), clientProcessThread );
    };
    client->clientSettings()->packageReceivingCallback = [ &clientFlag, &clientProcessThread ](
            const auto &,
            const auto &hostName,
            const auto &port,
            const auto &randomFlag,
            const auto &payloadCurrentIndex,
            const auto &payloadCurrentSize,
            const auto &payloadTotalSize
        )
    {
//        qDebug() << "client packageReceivingCallback:" << hostName << port << randomFlag << payloadCurrentIndex << payloadCurrentSize << payloadTotalSize;

        clientFlag += QString( "client packageReceivingCallback: %1 %2 %3 %4 %5 %6\n" ).arg( hostName ).arg( port ).arg( randomFlag ).arg( payloadCurrentIndex ).arg( payloadCurrentSize ).arg( payloadTotalSize );

        QCOMPARE( QThread::currentThread(), clientProcessThread );
    };
    client->clientSettings()->packageReceivedCallback = [ &clientFlag, &clientProcessThread ](
            const auto &,
            const auto &hostName,
            const auto &port,
            const auto &package
        )
    {
//        qDebug() << "client packageReceivedCallback:" << hostName << port << package->payloadData();

        clientFlag += QString( "client packageReceivedCallback: %1 %2 %3\n" ).arg( hostName ).arg( port ).arg( QString::fromLatin1( package->payloadData() ) );

        QCOMPARE( QThread::currentThread(), clientProcessThread );
    };
    QCOMPARE( client->begin(), true );

    QCOMPARE( client->waitForCreateConnect( "127.0.0.1", 23456 ), true );

    auto succeedCallback = [ &clientFlag, &clientProcessThread ]
            (
                const auto &,
                const auto &package
            )
    {
//        qDebug() << "client succeedCallback:" << package->payloadData();

        clientFlag += QString( "client succeedCallback: %1\n" ).arg( QString::fromLatin1( package->payloadData() ) );

        QCOMPARE( QThread::currentThread(), clientProcessThread );
    };

    QCOMPARE( client->sendPayloadData( "127.0.0.1", 23456, "12345", succeedCallback ), 1 );

    QThread::msleep( 1000 );

    QCOMPARE( serverFlag.toLatin1().data(),
              "server packageReceivingCallback: 1 0 2 5\n"
              "server packageReceivingCallback: 1 2 2 5\n"
              "server packageReceivingCallback: 1 4 1 5\n"
              "server packageReceivedCallback: 12345\n"
              "server packageSendingCallback: 1 0 2 5\n"
              "server packageSendingCallback: 1000000000 0 2 4\n"
              "server packageSendingCallback: 1 2 2 5\n"
              "server packageSendingCallback: 1000000000 2 2 4\n"
              "server packageSendingCallback: 1 4 1 5\n"
          );

    QCOMPARE( clientFlag.toLatin1().data(),
              "client packageSendingCallback: 127.0.0.1 23456 1 0 2 5\n"
              "client packageSendingCallback: 127.0.0.1 23456 1 2 2 5\n"
              "client packageSendingCallback: 127.0.0.1 23456 1 4 1 5\n"
              "client packageReceivingCallback: 127.0.0.1 23456 1 0 2 5\n"
              "client packageReceivingCallback: 127.0.0.1 23456 1000000000 0 2 4\n"
              "client packageReceivingCallback: 127.0.0.1 23456 1 2 2 5\n"
              "client packageReceivingCallback: 127.0.0.1 23456 1000000000 2 2 4\n"
              "client packageReceivedCallback: 127.0.0.1 23456 abcd\n"
              "client packageReceivingCallback: 127.0.0.1 23456 1 4 1 5\n"
              "client succeedCallback: 67890\n"
          );
}

void JQNetworkOverallTest::jqNetworkServerAndClientTest2()
{
    QString serverFlag;
    QString clientFlag;

    QByteArray testData;
    for ( auto count = 0; count < 32 * 1024 * 1024; ++count )
    {
        testData.append( rand() % 16 );
    }

    auto server = JQNetworkServer::createServer( 34567 );
    server->serverSettings()->globalProcessorThreadCount = 1;
    server->serverSettings()->packageReceivingCallback = [ &serverFlag ](
            const auto &,
            const auto &randomFlag,
            const auto &payloadCurrentIndex,
            const auto &payloadCurrentSize,
            const auto &payloadTotalSize
        )
    {
//        qDebug() << "server packageReceivingCallback:" << randomFlag << payloadCurrentIndex << payloadCurrentSize << payloadTotalSize;

        serverFlag += QString( "server packageReceivingCallback: %1 %2 %3 %4\n" ).arg( randomFlag ).arg( payloadCurrentIndex ).arg( payloadCurrentSize ).arg( payloadTotalSize );
    };
    server->serverSettings()->packageReceivedCallback = [ &serverFlag, testData ](
            const auto &,
            const auto &package
        )
    {
//        qDebug() << "server packageReceivedCallback:" << package->payloadDataSize();

        QCOMPARE( package->payloadData(), testData );

        serverFlag += QString( "server packageReceivedCallback: %1\n" ).arg( package->payloadDataSize() );
    };
    QCOMPARE( server->begin(), true );

    auto client = JQNetworkClient::createClient();
    client->connectSettings()->cutPackageSize = 8 * 1024 * 1024;
    client->connectSettings()->packageCompressionThresholdForConnectSucceedElapsed = 0;
    client->clientSettings()->globalProcessorThreadCount = 1;
    client->clientSettings()->packageSendingCallback = [ &clientFlag ](
            const auto &,
            const auto &hostName,
            const auto &port,
            const auto &randomFlag,
            const auto &payloadCurrentIndex,
            const auto &payloadCurrentSize,
            const auto &payloadTotalSize
        )
    {
//        qDebug() << "client packageSendingCallback:" << hostName << port << randomFlag << payloadCurrentIndex << payloadCurrentSize << payloadTotalSize;

        clientFlag += QString( "client packageSendingCallback: %1 %2 %3 %4 %5 %6\n" ).arg( hostName ).arg( port ).arg( randomFlag ).arg( payloadCurrentIndex ).arg( payloadCurrentSize ).arg( payloadTotalSize );
    };
    QCOMPARE( client->begin(), true );

    QCOMPARE( client->waitForCreateConnect( "127.0.0.1", 34567 ), true );

    QTime time;
    time.start();

    QCOMPARE( client->sendPayloadData( "127.0.0.1", 34567, testData ), 1 );

    const auto &&elapsed = time.elapsed();
//    qDebug() << "elapsed:" << elapsed;
    QCOMPARE( elapsed > 200, true );

    QThread::msleep( 8000 );

    const auto &&alreadyWrittenBytes = client->getConnect( "127.0.0.1", 34567 )->alreadyWrittenBytes();
//    qDebug() << "alreadyWrittenBytes:" << alreadyWrittenBytes;
    QCOMPARE( alreadyWrittenBytes < 30554432, true );

    QCOMPARE( serverFlag.toLatin1().data(),
              "server packageReceivingCallback: 1 0 8388608 33554432\n"
              "server packageReceivingCallback: 1 8388608 8388608 33554432\n"
              "server packageReceivingCallback: 1 16777216 8388608 33554432\n"
              "server packageReceivingCallback: 1 25165824 8388608 33554432\n"
              "server packageReceivedCallback: 33554432\n"
          );

    QCOMPARE( clientFlag.toLatin1().data(),
              "client packageSendingCallback: 127.0.0.1 34567 1 0 8388608 33554432\n"
              "client packageSendingCallback: 127.0.0.1 34567 1 8388608 8388608 33554432\n"
              "client packageSendingCallback: 127.0.0.1 34567 1 16777216 8388608 33554432\n"
              "client packageSendingCallback: 127.0.0.1 34567 1 25165824 8388608 33554432\n"
          );
}

void JQNetworkOverallTest::jqNetworkServerAndClientTest3()
{
    auto server = JQNetworkServer::createServer( 12569 );

    QMutex mutex;
    auto succeedCount = 0;

    QByteArray testData;
    testData.resize( 4 * 1024 );
    memset( testData.data(), 0, testData.size() );

    server->serverSettings()->packageReceivedCallback = [ testData, &mutex, &succeedCount ](const auto &connect, const auto &package)
    {
        if ( package->payloadData() == testData )
        {
            mutex.lock();
            ++succeedCount;
            mutex.unlock();
        }

        connect->replyPayloadData( package->randomFlag(), "OK" );
    };

    const auto &&lanAddressEntries = JQNetworkLan::lanAddressEntries();
    if ( lanAddressEntries.isEmpty() || ( lanAddressEntries[ 0 ].ip == QHostAddress::LocalHost ) )
    {
        QSKIP( "lanAddressEntries is empty" );
    }

    auto test = [ targetIp = lanAddressEntries[ 0 ].ip.toString(), testData ]()
    {
        auto client = JQNetworkClient::createClient();

        QCOMPARE( client->begin(), true );
        QCOMPARE( client->waitForCreateConnect( "127.0.0.1", 12569 ), true );
        QCOMPARE( client->waitForCreateConnect( targetIp, 12569 ), true );

        for ( auto count = 100; count; --count )
        {
            client->waitForSendPayloadData( "127.0.0.1", 12569, testData );
            client->waitForSendPayloadData( targetIp, 12569, testData );
        }
    };

    QCOMPARE( server->begin(), true );

    test();
    test();
    test();
    test();
    test();

    for ( auto count = 500; count; --count )
    {
        QtConcurrent::run( test );
    }

    QThreadPool::globalInstance()->waitForDone();

    QCOMPARE( succeedCount, 101000 );
}

void JQNetworkOverallTest::jqNetworkLanTest()
{
    bool flag1 = false;
    bool flag2 = false;
    bool flag3 = false;

    {
        auto lan = JQNetworkLan::createLan( QHostAddress( "228.12.23.34" ), 12345 );

        lan->lanSettings()->lanNodeOnlineCallback = [ &flag1 ](const auto & /*lanNode*/)
        {
//            qDebug() << "lanNodeOnlineCallback" << lanNode.nodeMarkSummary;
            flag1 = true;
        };
        lan->lanSettings()->lanNodeActiveCallback = [ &flag2 ](const auto & /*lanNode*/)
        {
//            qDebug() << "lanNodeActiveCallback" << lanNode.nodeMarkSummary;
            flag2 = true;
        };
        lan->lanSettings()->lanNodeOfflineCallback = [ &flag3 ](const auto & /*lanNode*/)
        {
//            qDebug() << "lanNodeOfflineCallback" << lanNode.nodeMarkSummary;
            flag3 = true;
        };

        QCOMPARE( lan->begin(), true );

        QThread::sleep( 20 );

        auto availableLanNodes = lan->availableLanNodes();
        QCOMPARE( availableLanNodes.size() >= 1, true );

        bool flag4 = false;

        for ( const auto &availableLanNode: availableLanNodes )
        {
            if ( availableLanNode.isSelf )
            {
               flag4 = true;
            }
        }

        QCOMPARE( flag4, true );
    }

    QCOMPARE( flag1, true );
    QCOMPARE( flag2, true );
    QCOMPARE( flag3, true );

    const auto &&lanAddressEntries = JQNetworkLan::lanAddressEntries();

    QCOMPARE( lanAddressEntries.size() >= 1, true );

    {
        QVector< QSharedPointer< JQNetworkLan > > lans;

        for ( auto count = 0; count < 5; ++count )
        {
            auto lan = JQNetworkLan::createLan( QHostAddress( "228.12.23.34" ), 12345 );

            lan->setAppendData( count );

            QCOMPARE( lan->begin(), true );
            QThread::sleep( 1 );

            lans.push_back( lan );
        }

        QThread::sleep( 1 );

        for ( const auto &lan: lans )
        {
            const auto &&availableLanNodes = lan->availableLanNodes();

            QCOMPARE( availableLanNodes.size(), 5 );

            QSet< int > flag;

            for ( const auto &lanNode: availableLanNodes )
            {
                flag.insert( lanNode.appendData.toInt() );
            }

            QCOMPARE( flag.size(), 5 );
        }
    }
}

void JQNetworkOverallTest::jqNetworkProcessorTest1()
{
    ProcessorTest1::TestProcessor myProcessor;

    QCOMPARE( myProcessor.availableSlots(), QSet< QString >( { "actionFlag" } ) );

    auto test = [ &myProcessor ]()
    {
        auto package = JQNetworkPackage::createPayloadTransportPackages(
                    "actionFlag",
                    "{\"key\":\"value\"}",
                    { }, // Empty appendData
                    0x1234
                ).first();
        package->refreshPackage();

        return myProcessor.handlePackage(
                    nullptr,
                    package
                );
    };

    QCOMPARE( test(), false );

    myProcessor.setReceivedPossibleThreads( { QThread::currentThread() } );

    QCOMPARE( test(), true );

    QCOMPARE( myProcessor.testData_, QVariantMap( { { "key", "value" } } ) );
    QCOMPARE( myProcessor.testData2_, QThread::currentThread() );
}

void JQNetworkOverallTest::jqNetworkProcessorTest2()
{
    QFile::remove( ProcessorTest2::TestProcessor::testFileInfo( 1 ).filePath() );
    QFile::remove( ProcessorTest2::TestProcessor::testFileInfo( 2 ).filePath() );
    QFile::remove( ProcessorTest2::TestProcessor::testFileInfo( 3 ).filePath() );
    QFile::remove( ProcessorTest2::TestProcessor::testFileInfo( 4 ).filePath() );

    QThread::sleep( 1 );

    ProcessorTest2::TestProcessor::createTestFile( 1 );
    ProcessorTest2::TestProcessor::createTestFile( 3 );

    auto server = JQNetworkServer::createServer( 33445, QHostAddress::LocalHost, true );
    auto processor = QSharedPointer< ProcessorTest2::TestProcessor >( new ProcessorTest2::TestProcessor );

    server->registerProcessor( processor.data() );

    auto filePathProvider = [ ](const auto &, const auto &, const auto &fileName)
    {
        return QString( "%1/JQNetworkOverallTest/%2" ).arg(
                        QStandardPaths::writableLocation( QStandardPaths::TempLocation ),
                        ( fileName == "myprocessor2_testfile1" ) ? ( "myprocessor2_testfile2" ) : ( "myprocessor2_testfile4" )
                    );
    };

    server->connectSettings()->filePathProvider = filePathProvider;

    QCOMPARE( server->availableProcessorMethodNames().size(), 13 );
    QCOMPARE( server->begin(), true );

    auto client = JQNetworkClient::createClient( true );

    client->connectSettings()->filePathProvider = filePathProvider;

    QCOMPARE( client->begin(), true );

    QCOMPARE( client->waitForCreateConnect( "127.0.0.1", 33445 ), true );

    int succeedCounter = 0;
    int keySucceedCounter = 0;
    int appendKey2SucceedCounter = 0;
    auto succeedCallback = [ & ](const JQNetworkConnectPointer &, const JQNetworkPackageSharedPointer &package)
    {
        ++succeedCounter;

        const auto &&receivedData = QJsonDocument::fromJson( package->payloadData() ).object().toVariantMap();
        const auto &&receivedAppendData = package->appendData();

        if ( receivedData[ "key" ] == "value" )
        {
            ++keySucceedCounter;
        }

        if ( receivedAppendData[ "key2" ] == "value2" )
        {
            ++appendKey2SucceedCounter;
        }
    };

    client->sendPayloadData( "127.0.0.1", 33445, "receivedByteArray", "test", succeedCallback );
    client->sendPayloadData( "127.0.0.1", 33445, "receivedByteArraySendByteArray", "test", succeedCallback );
    client->sendPayloadData( "127.0.0.1", 33445, "receivedByteArraySendVariantMap", "test", succeedCallback );
    client->sendPayloadData( "127.0.0.1", 33445, "receivedByteArraySendFile", "test", succeedCallback );

    client->sendVariantMapData( "127.0.0.1", 33445, "receivedVariantMap", { { "key", "value" } }, succeedCallback );
    client->sendVariantMapData( "127.0.0.1", 33445, "receivedVariantMapSendByteArray", { { "key", "value" } }, succeedCallback );
    client->sendVariantMapData( "127.0.0.1", 33445, "receivedVariantMapSendVariantMap", { { "key", "value" } }, succeedCallback );
    client->sendVariantMapData( "127.0.0.1", 33445, "receivedVariantMapSendFile", { { "key", "value" } }, succeedCallback );

    client->sendFileData( "127.0.0.1", 33445, "receiveFile", ProcessorTest2::TestProcessor::testFileInfo( 1 ), succeedCallback );
    client->sendFileData( "127.0.0.1", 33445, "receiveFileSendByteArray", ProcessorTest2::TestProcessor::testFileInfo( 1 ), succeedCallback );
    client->sendFileData( "127.0.0.1", 33445, "receiveFileSendVariantMap", ProcessorTest2::TestProcessor::testFileInfo( 1 ), succeedCallback );
    client->sendFileData( "127.0.0.1", 33445, "receiveFileSendFile", ProcessorTest2::TestProcessor::testFileInfo( 1 ), succeedCallback );

    client->sendVariantMapData( "127.0.0.1", 33445, "receivedVariantMapAndAppendSendVariantMapAndAppend", { { "key2", "value2" } }, { { "key3", "value3" } }, succeedCallback );

    QThread::sleep( 2 );

    QCOMPARE( processor->counter_.size(), 13 );
    QCOMPARE( succeedCounter, 10 );
    QCOMPARE( keySucceedCounter, 4 );
    QCOMPARE( appendKey2SucceedCounter, 1 );

    QCOMPARE( ProcessorTest2::TestProcessor::testFileInfo( 1 ).exists(), true );
    QCOMPARE( ProcessorTest2::TestProcessor::testFileInfo( 2 ).exists(), true );
    QCOMPARE( ProcessorTest2::TestProcessor::testFileInfo( 3 ).exists(), true );
    QCOMPARE( ProcessorTest2::TestProcessor::testFileInfo( 4 ).exists(), true );
}

void JQNetworkOverallTest::jqNetworkSendFile()
{
    QEventLoop eventLoop;

    auto flag1 = false;

    auto server = JQNetworkServer::createServer( 12457, QHostAddress::Any, true );

    server->serverSettings()->packageReceivedCallback = [ &flag1, &eventLoop ](const JQNetworkConnectPointer &, const JQNetworkPackageSharedPointer &package)
    {
        eventLoop.quit();
        flag1 = true;

        QCOMPARE( package->containsFile(), true );
        QCOMPARE( package->metaData().isEmpty(), false );
        QCOMPARE( package->payloadData().isEmpty(), true );
    };

    QCOMPARE( server->begin(), true );

    auto client = JQNetworkClient::createClient( true );
    QCOMPARE( client->begin(), true );

    QCOMPARE( client->waitForCreateConnect( "127.0.0.1", 12457 ), true );

    const auto &&testFileDir = QString( "%1/JQNetworkTestFile" ).arg( QStandardPaths::writableLocation( QStandardPaths::TempLocation ) );
    if ( !QDir( testFileDir ).exists() )
    {
        QCOMPARE( QDir().mkdir( testFileDir ), true );
    }

    const auto &&testSourceFilePath = QString( "%1/testfile" ).arg( testFileDir );
    const auto &&testSourceFileInfo = QFileInfo( testSourceFilePath );

    if ( !testSourceFileInfo.exists() || ( testSourceFileInfo.size() != ( 64 * 1024 * 1024 ) ) )
    {
        QFile testSourceFile( testSourceFilePath );
        QCOMPARE( testSourceFile.open( QIODevice::WriteOnly ), true );
        QCOMPARE( testSourceFile.resize( 64 * 1024 * 1024 ), true );
    }

    QCOMPARE( client->sendFileData( "127.0.0.1", 12457, testSourceFilePath ) > 0, true );

    QTimer::singleShot( 10 * 1000, &eventLoop, &QEventLoop::quit );
    eventLoop.exec();

    QCOMPARE( flag1, true );
}

void JQNetworkOverallTest::fusionTest1()
{
    auto server = JQNetworkServer::createServer( 24680 );
    auto userProcessor = QSharedPointer< FusionTest1::UserProcessor >( new FusionTest1::UserProcessor );
    auto dataProcessor = QSharedPointer< FusionTest1::DataProcessor >( new FusionTest1::DataProcessor );

    server->registerProcessor( userProcessor.data() );
    server->registerProcessor( dataProcessor.data() );

    QCOMPARE( server->availableProcessorMethodNames().size(), 3 );
    QCOMPARE( server->begin(), true );

    auto client = JQNetworkClient::createClient();

    QCOMPARE( client->begin(), true );

    {
        QTime time;
        time.start();

        {
            const auto &&sendReply = client->waitForSendPayloadData(
                            "127.0.0.1",
                            24680,
                            "accountLogin",
                            QByteArray()
                        );
            QCOMPARE( sendReply, 1 );
        }

        {
            bool flag2 = false;
            const auto &&sendReply = client->waitForSendPayloadData(
                            "127.0.0.1",
                            24680,
                            "accountLogin",
                            { },
                            nullptr,
                            [ &flag2 ](const auto &){ flag2 = true; }
                        );
            QCOMPARE( sendReply, 2 );
            QCOMPARE( flag2, false );
        }

        {
            bool flag1 = false;
            const auto &&sendReply = client->waitForSendPayloadData(
                            "127.0.0.1",
                            24680,
                            "accountLogin",
                            { },
                            [ &flag1 ](const auto &, const auto &){ flag1 = true; },
                            nullptr
                        );
            QCOMPARE( sendReply, 3 );
            QCOMPARE( flag1, true );
        }

        {
            bool flag1 = false;
            bool flag2 = false;
            const auto &&sendReply = client->waitForSendPayloadData(
                            "127.0.0.1",
                            24680,
                            "accountLogin",
                            { },
                            [ &flag1 ](const auto &, const auto &){ flag1 = true; },
                            [ &flag2 ](const auto &){ flag2 = true; }
                        );
            QCOMPARE( sendReply, 4 );
            QCOMPARE( flag1, true );
            QCOMPARE( flag2, false );
        }

        QCOMPARE( ( time.elapsed() < 100 ), true );

        {
            const auto &&sendReply = client->waitForSendVariantMapData(
                        "127.0.0.1",
                        24680,
                        "msleep",
                        { { "msleep", 500 } }
                    );
            QCOMPARE( sendReply, 5 );
        }

        QCOMPARE( ( time.elapsed() > 500 ), true );
    }

    {
        client->waitForSendVariantMapData(
                        "127.0.0.1",
                        24680,
                        "accountLogin",
                        { },
                        [ ](const auto &, const auto &package)
                        {
                            const auto &&received = QJsonDocument::fromJson( package->payloadData() ).object().toVariantMap();
                            QCOMPARE( received.isEmpty(), false );
                            QCOMPARE( received[ "succeed" ].toBool(), false );
                            QCOMPARE( received[ "message" ].toString(), QString( "error: handle is empty" ) );
                            QCOMPARE( received[ "userName" ].toString(), QString( "" ) );
                            QCOMPARE( received[ "userPhoneNumber" ].toString(), QString( "" ) );
                        },
                        [ ](const auto &){ }
                    );

        client->waitForSendVariantMapData(
                        "127.0.0.1",
                        24680,
                        "accountLogin",
                        { { "handle", "test" } },
                        [ ](const auto &, const auto &package)
                        {
                            const auto &&received = QJsonDocument::fromJson( package->payloadData() ).object().toVariantMap();
                            QCOMPARE( received.isEmpty(), false );
                            QCOMPARE( received[ "succeed" ].toBool(), false );
                            QCOMPARE( received[ "message" ].toString(), QString( "error: password is empty" ) );
                            QCOMPARE( received[ "userName" ].toString(), QString( "" ) );
                            QCOMPARE( received[ "userPhoneNumber" ].toString(), QString( "" ) );
                        },
                        [ ](const auto &){ }
                    );

        client->waitForSendVariantMapData(
                        "127.0.0.1",
                        24680,
                        "accountLogin",
                        { { "handle", "test" }, { "password", "123456" } },
                        [ ](const auto &, const auto &package)
                        {
                            const auto &&received = QJsonDocument::fromJson( package->payloadData() ).object().toVariantMap();
                            QCOMPARE( received.isEmpty(), false );
                            QCOMPARE( received[ "succeed" ].toBool(), true );
                            QCOMPARE( received[ "message" ].toString(), QString( "" ) );
                            QCOMPARE( received[ "userName" ].toString(), QString( "" ) );
                            QCOMPARE( received[ "userPhoneNumber" ].toString(), QString( "" ) );
                        },
                        [ ](const auto &){ }
                    );

        client->waitForSendVariantMapData(
                        "127.0.0.1",
                        24680,
                        "accountLogin",
                        { { "handle", "test" }, { "password", "123456" }, { "userName", "" }, { "userPhoneNumber", "" } },
                        [ ](const auto &, const auto &package)
                        {
                            const auto &&received = QJsonDocument::fromJson( package->payloadData() ).object().toVariantMap();
                            QCOMPARE( received.isEmpty(), false );
                            QCOMPARE( received[ "succeed" ].toBool(), true );
                            QCOMPARE( received[ "message" ].toString(), QString( "" ) );
                            QCOMPARE( received[ "userName" ].toString(), QString( "Jason" ) );
                            QCOMPARE( received[ "userPhoneNumber" ].toString(), QString( "18600000000" ) );
                        },
                        [ ](const auto &){ }
                    );
    }

    {
        client->waitForSendVariantMapData(
                        "127.0.0.1",
                        24680,
                        "someRecords",
                        { { "action", "download" } },
                        [ ](const auto &, const auto &package)
                        {
                            const auto &&received = QJsonDocument::fromJson( package->payloadData() ).object().toVariantMap();
                            QCOMPARE( received[ "someRecords" ].toList(), QVariantList() );
                        },
                        [ ](const auto &){ }
                    );

        client->waitForSendVariantMapData(
                        "127.0.0.1",
                        24680,
                        "someRecords",
                        { { "action", "upload" }, { "someRecords", QVariantList( { QVariant( "tests" ) } ) } },
                        [ ](const auto &, const auto &package)
                        {
                            const auto &&received = QJsonDocument::fromJson( package->payloadData() ).object().toVariantMap();
                            QCOMPARE( received[ "someRecords" ].toList(), QVariantList() );
                        },
                        [ ](const auto &){ }
                    );

        client->waitForSendVariantMapData(
                        "127.0.0.1",
                        24680,
                        "someRecords",
                        { { "action", "download" } },
                        [ ](const auto &, const auto &package)
                        {
                            const auto &&received = QJsonDocument::fromJson( package->payloadData() ).object().toVariantMap();
                            QCOMPARE( received[ "someRecords" ].toList(), QVariantList( { QVariant( "tests" ) } ) );
                        },
                        [ ](const auto &){ }
                    );
    }
}
