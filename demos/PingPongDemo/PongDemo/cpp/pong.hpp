#ifndef __CPP_PONG_HPP__
#define __CPP_PONG_HPP__

// JQNetwork lib improt
#include <JQNetwork>

// Pong 为服务端
class Pong
{
public:
    Pong() = default;

    ~Pong() = default;

    bool begin()
    {
        // 创建一个服务端
        const quint16 &&listenPort = 34543; // 监听端口
        server_ = JQNetworkServer::createServer( listenPort );

        // 设置接收到数据包后的回调
        server_->serverSettings()->packageReceivedCallback = std::bind( &Pong::onPackageReceived, this, std::placeholders::_1, std::placeholders::_2 );

        // 初始化服务端
        if ( !server_->begin() )
        {
            qDebug() << "Pong: begin fail";
            return false;
        }

        qDebug() << "Pong: begin succeed";

        return true;
    }

    void onPackageReceived(const JQNetworkConnectPointer &connect, const JQNetworkPackageSharedPointer &package)
    {
        // 回调会发生在一个专用的线程，请注意线程安全

        // 打印客户端发送的数据
        qDebug() << "Pong: received data:" << package->payloadData() << ", randomFlag:" << package->randomFlag();

        // 返回一个数据，需要指定 randomFlag 以告知客户端
        connect->replyPayloadData( package->randomFlag(), "Pong" );
    }

private:
    JQNetworkServerSharedPointer server_;
};

#endif//__CPP_PONG_HPP__
