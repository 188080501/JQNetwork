#include "jqnetwork_benchmark.h"

// Qt lib import
#include <QtTest>
#include <QtConcurrent>
#include <QTcpSocket>

// JQNetwork lib import
#include <JQNetworkFoundation>
#include <JQNetworkPackage>
#include <JQNetworkConnect>
#include <JQNetworkConnectPool>
#include <JQNetworkServer>
#include <JQNetworkProcessor>
#include <JQNetworkClient>

void JQNetworkPersisteneTest::test1()
{
    auto server = JQNetworkServer::createServer( 12345 );

    server->serverSettings()->packageReceivedCallback = [](const auto &connect, const auto &package)
    {
        connect->replyPayloadData( package->randomFlag(), "Test" );
    };

    if ( !server->begin() )
    {
        qDebug() << "test1 error1";
        return;
    }

    auto client = JQNetworkClient::createClient();

    if ( !client->begin() )
    {
        qDebug() << "test1 error2";
        return;
    }

    const auto &&waitForCreateConnectReply = client->waitForCreateConnect( "127.0.0.1", 12345 );
    qDebug() << "waitForCreateConnect:" << waitForCreateConnectReply;

    if ( !waitForCreateConnectReply ) { return; }

    const auto &&startTime = QDateTime::currentMSecsSinceEpoch();
    auto testCount = 1000000;
    auto succeedCount = 0;
    QSemaphore semaphore;

    for ( auto count = 0; count < testCount; ++count )
    {
        client->sendPayloadData(
                    "127.0.0.1",
                    12345,
                    "Test",
                    [ &testCount, &succeedCount, &semaphore ](const auto &, const auto &)
                    {
                        if ( ++succeedCount >= testCount )
                        {
                            semaphore.release( 1 );
                        }
                    },
                    nullptr
                );
    }

    semaphore.acquire( 1 );

    const auto &&finishTime = QDateTime::currentMSecsSinceEpoch();

    qDebug() << QString( "test1 finish: total: %1 ms, %2 count/s" ).
                arg( finishTime - startTime ).
                arg( int( double(testCount) / ( ( finishTime - startTime ) / 1000.0 ) ) );
}

void JQNetworkPersisteneTest::test2()
{
    QByteArray testData;
    for ( auto count = 0; count < 32 * 1024; ++count )
    {
        testData.append( char( rand() % 256 ) );
    }

    auto server = JQNetworkServer::createServer( 23456 );

    server->serverSettings()->packageReceivedCallback = [ testData ](const auto &connect, const auto &package)
    {
        connect->replyPayloadData( package->randomFlag(), testData );
    };

    if ( !server->begin() )
    {
        qDebug() << "test2 error1";
        return;
    }

    auto client = JQNetworkClient::createClient();

    if ( !client->begin() )
    {
        qDebug() << "test2 error2";
        return;
    }

    const auto &&waitForCreateConnectReply = client->waitForCreateConnect( "127.0.0.1", 23456 );
    qDebug() << "waitForCreateConnect:" << waitForCreateConnectReply;

    if ( !waitForCreateConnectReply ) { return; }

    const auto &&startTime = QDateTime::currentMSecsSinceEpoch();
    auto testCount = 80000;
    auto succeedCount = 0;
    QSemaphore semaphore;

    for ( auto count = 0; count < testCount; ++count )
    {
        client->sendPayloadData(
                    "127.0.0.1",
                    23456,
                    testData,
                    [ &testCount, &succeedCount, &semaphore ](const auto &, const auto &)
                    {
                        if ( ++succeedCount >= testCount )
                        {
                            semaphore.release( 1 );
                        }
                    },
                    nullptr
                );
    }

    semaphore.acquire( 1 );

    const auto &&finishTime = QDateTime::currentMSecsSinceEpoch();
    const auto &&result = int( double(testCount) / ( ( finishTime - startTime ) / 1000.0 ) );

    qDebug() << QString( "test2 finish: total: %1 ms, %2 count/s, payload transfer speed: %3 Mbit/s" ).
                arg( finishTime - startTime ).
                arg( result ).
                arg( result * 32 / 1024 * 8 );
}

void JQNetworkPersisteneTest::test3()
{
    auto server = JQNetworkServer::createServer( 34567 );

    server->serverSettings()->packageReceivedCallback = [](const auto &connect, const auto &package)
    {
        connect->replyPayloadData( package->randomFlag(), "Test" );
    };

    if ( !server->begin() )
    {
        qDebug() << "test3 error1";
        return;
    }

    auto client = JQNetworkClient::createClient();

    if ( !client->begin() )
    {
        qDebug() << "test3 error2";
        return;
    }

    const auto &&waitForCreateConnectReply = client->waitForCreateConnect( "127.0.0.1", 34567 );
    qDebug() << "waitForCreateConnect:" << waitForCreateConnectReply;

    if ( !waitForCreateConnectReply ) { return; }

    const auto &&startTime = QDateTime::currentMSecsSinceEpoch();
    auto testCount = 400000;
    auto succeedCount = 0;
    QSemaphore semaphore;

    std::function< void() > ping;
    ping = [ &testCount, &succeedCount, &semaphore, &ping, &client ]()
    {
        client->sendPayloadData(
                    "127.0.0.1",
                    34567,
                    "Test",
                    [ &testCount, &succeedCount, &semaphore, &ping ](const auto &, const auto &)
                    {
                        if ( ++succeedCount >= testCount )
                        {
                            semaphore.release( 1 );
                        }
                        else
                        {
                            ping();
                        }
                    },
                    nullptr
                );
    };

    ping();

    semaphore.acquire( 1 );

    const auto &&finishTime = QDateTime::currentMSecsSinceEpoch();

    qDebug() << QString( "test3 finish: total: %1 ms, %2 PingPong/s" ).
                arg( finishTime - startTime ).
                arg( int( double(testCount) / ( ( finishTime - startTime ) / 1000.0 ) ) );
}

void JQNetworkPersisteneTest::test4()
{
    QByteArray testData;
    for ( auto count = 0; count < 32 * 1024; ++count )
    {
        testData.append( char( rand() % 256 ) );
    }

    auto server = JQNetworkServer::createServer( 45678 );

    server->serverSettings()->packageReceivedCallback = [ testData ](const auto &connect, const auto &package)
    {
        connect->replyPayloadData( package->randomFlag(), testData );
    };

    if ( !server->begin() )
    {
        qDebug() << "test4 error1";
        return;
    }

    auto client = JQNetworkClient::createClient();

    if ( !client->begin() )
    {
        qDebug() << "test4 error2";
        return;
    }

    const auto &&waitForCreateConnectReply = client->waitForCreateConnect( "127.0.0.1", 45678 );
    qDebug() << "waitForCreateConnect:" << waitForCreateConnectReply;

    if ( !waitForCreateConnectReply ) { return; }

    const auto &&startTime = QDateTime::currentMSecsSinceEpoch();
    auto testCount = 250000;
    auto succeedCount = 0;
    QSemaphore semaphore;

    std::function< void() > ping;
    ping = [ &testCount, &succeedCount, &semaphore, &ping, &client, testData ]()
    {
        client->sendPayloadData(
                    "127.0.0.1",
                    45678,
                    testData,
                    [ &testCount, &succeedCount, &semaphore, &ping ](const auto &, const auto &)
                    {
                        if ( ++succeedCount >= testCount )
                        {
                            semaphore.release( 1 );
                        }
                        else
                        {
                            ping();
                        }
                    },
                    nullptr
                );
    };

    ping();

    semaphore.acquire( 1 );

    const auto &&finishTime = QDateTime::currentMSecsSinceEpoch();
    const auto &&result = int( double(testCount) / ( ( finishTime - startTime ) / 1000.0 ) );

    qDebug() << QString( "test4 finish: total: %1 ms, %2 PingPong/s, payload transfer speed: %3 Mbit/s" ).
                arg( finishTime - startTime ).
                arg( result ).
                arg( result * 32 / 1024 * 8 );
}

void JQNetworkPersisteneTest::test5()
{
    static QByteArray testData;
    for ( auto count = 0; count < 512 * 1024 * 1024; ++count )
    {
        testData.append( char( rand() % 256 ) );
    }

    auto server = JQNetworkServer::createServer( 56789 );
    QSemaphore semaphore;

    server->serverSettings()->packageReceivedCallback = [ &semaphore ](const auto &, const auto &)
    {
        semaphore.release( 1 );
    };

    if ( !server->begin() )
    {
        qDebug() << "test5 error1";
        return;
    }

    auto client = JQNetworkClient::createClient();

    if ( !client->begin() )
    {
        qDebug() << "test5 error2";
        return;
    }

    const auto &&waitForCreateConnectReply = client->waitForCreateConnect( "127.0.0.1", 56789 );
    qDebug() << "waitForCreateConnect:" << waitForCreateConnectReply;

    if ( !waitForCreateConnectReply ) { return; }

    const auto &&startTime = QDateTime::currentMSecsSinceEpoch();

    client->sendPayloadData(
                "127.0.0.1",
                56789,
                testData
            );

    semaphore.acquire( 1 );

    const auto &&finishTime = QDateTime::currentMSecsSinceEpoch();

    qDebug() << QString( "test5 finish: total: %1 ms, payload transfer speed: %2 Mbit/s" ).
                arg( finishTime - startTime ).
                arg( int( 512.0 / ( ( finishTime - startTime ) / 1000.0 ) * 8 ) );

}
