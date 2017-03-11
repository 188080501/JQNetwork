#ifndef __CPP_PING_HPP__
#define __CPP_PING_HPP__

// Qt lib import
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>

// JQNetwork lib improt
#include <JQNetwork>

// Ping 为客户端
class Ping
{
public:
    Ping() = default;

    ~Ping() = default;

    bool begin()
    {
        timer_.setInterval( 500 );
        timer_.setSingleShot( true );
        QObject::connect( &timer_, &QTimer::timeout, std::bind( &Ping::ping, this ) );

        // 创建一个客户端
        client_ = JQNetworkClient::createClient();

        client_->clientSettings()->connectToHostSucceedCallback = std::bind( &Ping::onConnectSucceed, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 );

        if ( !client_->begin() )
        {
            qDebug() << "Ping: begin fail";
            return false;
        }

        // 创建一个连接，指向某个服务端，并保持长连接
        // 创建只需一次，若没连接成功内部会自动维护重连，断开后也会自动重连
        client_->createConnect( "127.0.0.1", 34543 );

        qDebug() << "Ping: begin succeed";

        return true;
    }

    // 连接到服务端后的回调
    void onConnectSucceed(const JQNetworkConnectPointer & /*connect*/, const QString &hostName, const quint16 &port)
    {
        // 回调会发生在一个专用的线程，请注意线程安全

        // 打印谁被连接成功了
        qDebug() << "Client: connect to server succeed:" << hostName << port;

        // 开启 PingPong 测试的定时器，循环发送
        // 因为这里在一个新的线程，所以通过 invokeMethod 去调用 start
        QMetaObject::invokeMethod( &timer_, "start" );
    }

    // Ping 测试
    void ping()
    {
        const auto &&randomFlag = client_->sendPayloadData( // 发送数据，返回0表示失败，其余数表示发送成功
                    "127.0.0.1", // 服务端IP
                    34543, // 服务端端口
                    "pong",
                    QJsonDocument( QJsonObject::fromVariantMap( QVariantMap( { { "ping", "ping" } } ) ) ).toJson( QJsonDocument::Compact ),
                    { }, // empty appendData
                    std::bind( &Ping::onPingSucceed, this, std::placeholders::_1, std::placeholders::_2 ),
                    std::bind( &Ping::onPingFail, this, std::placeholders::_1  )
                );

        if ( !randomFlag )
        {
            qDebug() << "Ping: send fail";
        }
        else
        {
            qDebug() << "Ping: send succeed, randomFlag:" << randomFlag;
            ++sendSucceedCount_;
        }
    }

     // 成功接收回复的数据时的回调
    void onPingSucceed(const JQNetworkConnectPointer & /*connect*/, const JQNetworkPackageSharedPointer &package)
    {
        // 回调会发生在一个专用的线程，请注意线程安全

        // 打印服务端回复的数据
        qDebug() << "Ping: received reply:" << package->payloadData() << ", randomFlag:" << package->randomFlag();

        ++replySucceedCount_;

        qDebug() << "TotalCount: sendSucceedCount:" << this->sendSucceedCount_ << ", replySucceedCount:" << replySucceedCount_ << "\n";

        if ( !( replySucceedCount_ % 100000 ) )
        {
            // 到达特定数量后，停止 PingPong 测试的循环发送
            // 因为这里在一个新的线程，所以通过 invokeMethod 去调用 stop
            QMetaObject::invokeMethod( &timer_, "stop" );
        }
        else
        {
            // 成功接收回复后，再发出Ping
            // 因为这里在一个新的线程，所以通过 invokeMethod 去调用 stop
            QMetaObject::invokeMethod( &timer_, "start" );
        }
    }

    // 未能成功接收回复的数据时的回调
    void onPingFail(const JQNetworkConnectPointer & /*connect*/)
    {
        // 回调会发生在一个专用的线程，请注意线程安全

        qDebug() << "Ping: received fail";
        qApp->quit();
    }

private:
    JQNetworkClientSharedPointer client_;

    // 计数器
    int sendSucceedCount_ = 0;
    int replySucceedCount_ = 0;

    // 定时器，循环发送用
    QTimer timer_;
};

#endif//__CPP_PING_HPP__
