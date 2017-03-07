#include "jqnetwork_persistenetest.h"

// Qt lib import
#include <QtTest>
#include <QtConcurrent>
#include <QTcpSocket>

// JQNetwork lib import
#include <JQNetwork>

void JQNetworkPersisteneTest::test()
{
    QMutex mutex;
    qint64 sendCount = 0;
    qint64 succeedCount = 0;
    QByteArray testData;
    for ( auto count = 0; count < 4 * 1024; ++count )
    {
        testData.append( char( rand() % 256 ) );
    }

    auto server = JQNetworkServer::createServer( 56789 );

    server->serverSettings()->packageReceivedCallback = [ & ](const auto &, const auto &package)
    {
        if ( package->payloadData() != testData )
        {
            qDebug() << "test error3";
            return;
        }

        mutex.lock();
        ++succeedCount;
        mutex.unlock();
    };

    if ( !server->begin() )
    {
        qDebug() << "test error1";
        return;
    }

    auto client = JQNetworkClient::createClient();

    if ( !client->begin() )
    {
        qDebug() << "test error2";
        return;
    }

    const auto &&waitForCreateConnectReply = client->waitForCreateConnect( "127.0.0.1", 56789 );
    qDebug() << "waitForCreateConnect:" << waitForCreateConnectReply;

    if ( !waitForCreateConnectReply ) { return; }

    const auto &&startTime = QDateTime::currentMSecsSinceEpoch();
    QTimer timerForSend;
    QTimer timerForPrint;
    QEventLoop eventLoop;

    auto sendTestData = [ & ]()
    {
        client->sendPayloadData(
                "127.0.0.1",
                56789,
                testData
            );

        mutex.lock();
        ++sendCount;
        mutex.unlock();

        timerForSend.start();
    };

    auto printData = [ & ]()
    {
        qDebug() << "run time:" << ( ( QDateTime::currentMSecsSinceEpoch() - startTime ) / 1000 / 3600 ) << "hour"
                 << ( ( QDateTime::currentMSecsSinceEpoch() - startTime ) / 1000 % 3600 / 60 ) << "minute,"
                 << "send count:" << sendCount << ","
                 << "succeed count:" << succeedCount;

        if ( ( sendCount - succeedCount ) > 1000 )
        {
            qDebug() << "test error3";
            eventLoop.quit();
        }

        timerForPrint.start();
    };

    timerForSend.setInterval( 100 );
    timerForPrint.setInterval( 30 * 1000 );

    connect( &timerForSend, &QTimer::timeout, sendTestData );
    connect( &timerForPrint, &QTimer::timeout, printData );

    timerForSend.start();
    timerForPrint.start();

    eventLoop.exec();
}
