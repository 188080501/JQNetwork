// Qt lib import
#include <QCoreApplication>
#include <QTimer>

// JQNetwork lib improt
#include <JQNetworkPackage>
#include <JQNetworkConnect>
#include <JQNetworkServer>
#include <JQNetworkClient>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // 创建一个服务端
    const quint16 &&listenPort = 34543; // 监听端口
    auto server = JQNetworkServer::createServer( listenPort );

    // 设置接收到数据包后的回调
    server->serverSettings()->packageReceivedCallback = [ ](const auto &connect, const auto &package)
    {
        // 回调会发生在一个专用的线程，请注意线程安全

        // 打印客户端发送的数据
        qDebug() << "Server: received data:" << package->payloadData() << ", randomFlag:" << package->randomFlag();

        // 返回一个数据，需要指定 randomFlag 以告知客户端
        connect->replyPayloadData( package->randomFlag(), "Pong" );
    };

    // 初始化服务端
    if ( !server->begin() )
    {
        qDebug() << "Server: begin fail";
        return -1;
    }

    // 计数
    int sendSucceedCount = 0, replySucceedCount = 0;

    // 创建一个客户端
    auto client = JQNetworkClient::createClient();

    // 创建定时器，循环发送
    QTimer timer;

    // 创建PingPong测试
    std::function< void() > ping;
    ping = [ &ping, &timer, client, &sendSucceedCount, &replySucceedCount ]()
    {
        const auto &&randomFlag = client->sendPayloadData( // 发送数据，返回0表示失败，其余数表示发送成功
                    "127.0.0.1",
                    34543,
                    "Ping",
                    [ &ping, &timer, &sendSucceedCount, &replySucceedCount ](const auto & /*connect*/, const auto &package) // 成功接收回复的数据时的回调
                    {
                        // 回调会发生在一个专用的线程，请注意线程安全

                        // 打印服务端回复的数据
                        qDebug() << "Client: received reply:" << package->payloadData() << ", randomFlag:" << package->randomFlag();

                        ++replySucceedCount;

                        qDebug() << "TotalCount: sendSucceedCount:" << sendSucceedCount << ", replySucceedCount:" << replySucceedCount;

                        if ( !( replySucceedCount % 100000 ) )
                        {
                            // 到达特定数量后，停止 PingPong 测试的循环发送
                            // 因为这里在一个新的线程，所以通过 invokeMethod 去调用 stop
                            QMetaObject::invokeMethod( &timer, "stop" );
                        }
                        else
                        {
                            // 成功接收回复后，再发出Ping
                            QMetaObject::invokeMethod( &timer, "start" );
                        }
                    },
                    [](const auto & /*connect*/) // 未能成功接收回复的数据时的回调
                    {
                        // 回调会发生在一个专用的线程，请注意线程安全

                        qDebug() << "Client: received fail";
                        qApp->quit();
                    }
                );

        if ( !randomFlag )
        {
            qDebug() << "Client: send fail";
        }
        else
        {
            qDebug() << "Client: send succeed, randomFlag:" << randomFlag;
            ++sendSucceedCount;
        }
    };

    timer.setInterval( 500 );
    timer.setSingleShot( true );

    QObject::connect( &timer, &QTimer::timeout, ping );

    // 设置连接到服务端后的回调
    client->clientSettings()->connectToHostSucceedCallback = [ &timer ](JQNetworkConnectPointer, const QString &hostName, const quint16 &port)
    {
        // 回调会发生在一个专用的线程，请注意线程安全

        // 打印谁被连接成功了
        qDebug() << "Client: connect to server succeed:" << hostName << port;

        // 开启 PingPong 测试的定时器，循环发送
        // 因为这里在一个新的线程，所以通过 invokeMethod 去调用 start
        QMetaObject::invokeMethod( &timer, "start" );
    };

    // 初始化客户端
    if ( !client->begin() )
    {
        qDebug() << "Client: begin fail";
        return -1;
    }

    // 创建一个连接，指向某个服务端，并保持长连接
    // 创建只需一次，若没连接成功内部会自动维护重连，断开后也会自动重连
    client->createConnect( "127.0.0.1", 34543 );

    return a.exec();
}
