#ifndef __CPP_MYSERVER_HPP__
#define __CPP_MYSERVER_HPP__

// Qt lib import
#include <QTimer>

// JQNetwork lib improt
#include <JQNetwork>

class MyServer: public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY( MyServer )

public:
    MyServer() = default;

    ~MyServer() = default;

    bool begin()
    {
        // 创建一个服务端
        const quint16 &&listenPort = 23456; // 监听端口
        server_ = JQNetworkServer::createServer( listenPort );

        // 设置接收到数据包后的回调
        server_->serverSettings()->packageReceivedCallback = std::bind( &MyServer::onPackageReceived, this, std::placeholders::_1, std::placeholders::_2 );

        // 初始化服务端
        if ( !server_->begin() )
        {
            qDebug() << "MyServer: begin fail";
            return false;
        }

        qDebug() << "MyServer: begin succeed";

        return true;
    }

    void onPackageReceived(const JQNetworkConnectPointer &connect, const JQNetworkPackageSharedPointer &package)
    {
        // 回调会发生在一个专用的线程，请注意线程安全

        // 打印客户端发送的数据
        qDebug() << "MyServer: received data:" << package->payloadData() << ", randomFlag:" << package->randomFlag();

        // 返回一个数据，需要指定 randomFlag 以告知客户端
        connect->replyPayloadData( package->randomFlag(), "OK" );
    }

private:
    JQNetworkServerSharedPointer server_;
};

#endif//__CPP_MYSERVER_HPP__
