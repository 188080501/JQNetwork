// Qt lib import
#include <QCoreApplication>
#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>

// JQNetwork lib improt
#include <JQNetwork>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    JQNetwork::printVersionInformation();

    // 创建客户端
    auto client = JQNetworkClient::createClient( true );

    // 初始化客户端
    if ( !client->begin() )
    {
        qDebug() << "Client: begin fail";
        return -1;
    }
    qDebug() << "Client: begin succeed";

    // 以阻塞方式创建连接
    qDebug() << "Client: waitForCreateConnect:" << client->waitForCreateConnect( "127.0.0.1", 26432 );

    // 创建一个测试文件
    const auto &&sourceFilePath = QString( "%1/%2" ).arg(
                QStandardPaths::writableLocation( QStandardPaths::TempLocation ),
                "jqnetwork_filetransferdemo"
            );
    {
        QFile file( sourceFilePath );
        file.open( QIODevice::WriteOnly );
        file.write( QByteArray( "FileTransferDemo" ) );
        file.waitForBytesWritten( 30 * 1000 );
    }

    // 发送文件
    qDebug() << "Client: sendFileData reply:" << client->sendFileData(
                "127.0.0.1",                                            // 服务端的IP地址
                26432,                                                  // 服务端的端口
                "fileTransfer",                                         // 需要调用的服务端方法名称
                QFileInfo( sourceFilePath ),                            // 需要发送的文件
                { },                                                    // empty appendData
                [ ](const auto &, const auto &)
                {
                    qDebug() << "Client: send file succeed";
                },
                [ ](const auto &)
                {
                    qDebug() << "Client: send file fail";
                }
    );

    return a.exec();
}
