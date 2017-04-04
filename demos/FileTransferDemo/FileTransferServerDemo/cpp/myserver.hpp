#ifndef __CPP_MYSERVER_HPP__
#define __CPP_MYSERVER_HPP__

// Qt lib import
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>

// JQNetwork lib improt
#include <JQNetwork>

class MyServer: public JQNetworkProcessor
{
    Q_OBJECT
    Q_DISABLE_COPY( MyServer )

public:
    MyServer() = default;

    ~MyServer() = default;

    bool begin()
    {
        // 创建一个服务端
        server_ = JQNetworkServer::createServer( 26432, QHostAddress::Any, true );

        // 注册当前类
        server_->registerProcessor( this );

        // 设置文件保存到桌面
        server_->connectSettings()->setFilePathProviderToDir( QStandardPaths::writableLocation( QStandardPaths::DesktopLocation ) );

        // 初始化服务端
        if ( !server_->begin() )
        {
            qDebug() << "MyServer: begin fail";
            return false;
        }

        qDebug() << "MyServer: begin succeed";

        return true;
    }

    // 用于处理的函数必须是槽函数
public slots:
    void fileTransfer(const QFileInfo &received, QVariantMap &send)
    {
        // 回调会发生在一个专用的线程，请注意线程安全

        // 打印接收到的文件
        qDebug() << "fileTransfer received:" << received.filePath();

        // 返回数据
        send[ "succeed" ] = true;
        send[ "message" ] = "";
    }

private:
    JQNetworkServerSharedPointer server_;
};

#endif//__CPP_MYSERVER_HPP__
