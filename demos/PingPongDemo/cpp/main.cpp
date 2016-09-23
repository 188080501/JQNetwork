// Qt lib import
#include <QCoreApplication>
#include <QtTest>
#include <QtConcurrent>

// JQNetwork lib improt
#include <JQNetworkPackage>
#include <JQNetworkConnect>
#include <JQNetworkServer>
#include <JQNetworkClient>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QtConcurrent::run( [ ]()
    {
        // 创建一个服务端
        const quint16 &&listenPort = 34543;
        auto server = JQNetworkServer::createServerByListenPort( listenPort );

        // 设置接收到数据包后的回调
        server->setOnPackageReceivedCallback( [ ](const auto &connect, const auto &package)
        {
            // 回调会发生在一个专用的线程，请注意线程安全

            // 打印客户端发送的数据
            qDebug() << "Server: received data:" << package->payloadData() << ", randomFlag:" << package->randomFlag();

            // 返回一个数据，需要指定 randomFlag 以告知客户端
            connect->replyPayloadData( "Pong", package->randomFlag() );
        } );

        // 初始化服务端
        if ( !server->begin() )
        {
            qDebug() << "Server: begin fail";
            return;
        }

        // 进入事件循环
        QEventLoop eventLoop;
        eventLoop.exec();
    } );

    QtConcurrent::run( [ ]()
    {
        // 计数
        int sendSucceedCount = 0, replySucceedCount = 0;

        // 创建一个客户端
        auto client = JQNetworkClient::createClient();

        // 创建定时器，循环发送
        QTimer timer;

        // 创建PingPong测试
        auto pingPong = [ &timer, client, &sendSucceedCount, &replySucceedCount ]()
        {
            // 发送数据，返回0表示失败，其余数表示发送成功
            const auto &&randomFlag = client->sendPayloadData(
                        "127.0.0.1",
                        34543,
                        "Ping",
                        {
                            // 成功接收回复的数据时的回调
                            [ &timer, &sendSucceedCount, &replySucceedCount ](const auto & /*connect*/, const auto &package)
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
                            },
                            // 未能成功接收回复的数据时的回调
                            [](const auto & /*connect*/)
                            {
                                // 回调会发生在一个专用的线程，请注意线程安全
                                //...
                            }
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

        QObject::connect( &timer, &QTimer::timeout, pingPong );

        // 设置连接到服务端后的回调
        client->setOnConnectToHostSucceedCallback( [ &timer, &pingPong ](JQNetworkConnectPointer, const QString &hostName, const quint16 &port)
        {
//            // 回调会发生在一个专用的线程，请注意线程安全

//            // 打印谁被连接成功了
            qDebug() << "Client: connect to server succeed:" << hostName << port;

//            // 开启 PingPong 测试的定时器，循环发送
//            // 因为这里在一个新的线程，所以通过 invokeMethod 去调用 start
            QMetaObject::invokeMethod( &timer, "start", Q_ARG( int, 500 ) );
        } );

        // 初始化客户端
        if ( !client->begin() )
        {
            qDebug() << "Client: begin fail";
            return;
        }

        // 创建一个连接，指向某个服务端，并保持长连接
        // 创建只需一次，若没连接成功内部会自动维护重连，断开后也会自动重连
        client->createConnect( "127.0.0.1", 34543 );

        // 进入事件循环
        QEventLoop eventLoop;
        eventLoop.exec();
    } );

    return a.exec();
}
