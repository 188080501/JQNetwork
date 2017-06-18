#ifndef CPP_PONG_HPP_
#define CPP_PONG_HPP_

// JQNetwork lib improt
#include <JQNetwork>

// Pong 为服务端
class Pong: public JQNetworkProcessor
{
    Q_OBJECT

public:
    Pong() = default;

    ~Pong() = default;

    bool begin()
    {
        // 创建一个服务端
        const quint16 &&listenPort = 34543; // 监听端口
        server_ = JQNetworkServer::createServer( listenPort );

        // 注册当前类
        server_->registerProcessor( this );

        // 初始化服务端
        if ( !server_->begin() )
        {
            qDebug() << "Pong: begin fail";
            return false;
        }

        qDebug() << "Pong: begin succeed";

        return true;
    }

    // 用于处理的函数必须是槽函数
public slots:
    void pong(const QVariantMap &received, QVariantMap &send)
    {
        // 回调会发生在一个专用的线程，请注意线程安全

        // 打印客户端发送的数据
        qDebug() << "Pong: received data:" << received;

        // 返回一个数据，需要指定 randomFlag 以告知客户端
        send[ "pong" ] = "pong";
    }

private:
    JQNetworkServerSharedPointer server_;
};

#endif//CPP_PONG_HPP_
