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

    // 创建服务端
    auto server = JQNetworkServer::createServer( 26432, QHostAddress::Any, true );

    // 设置接收的回调
    // 保存路径提供回调，用于提供接收文件的保存路径
    server->connectSettings()->filePathProvider = [ ](const auto &, const auto &, const auto &fileName)->QString
    {
        // 保存到桌面
        return QString( "%1/%2" ).arg( QStandardPaths::writableLocation( QStandardPaths::DesktopLocation ), fileName );
    };
    server->serverSettings()->packageReceivedCallback = [ ](const auto &, const JQNetworkPackageSharedPointer &package)
    {
        if ( package->fileName().isEmpty() ) { return; }

        qDebug() << "packageReceivedCallback: received file: "
                 << "\nfileName:" << package->fileName()
                 << "\nlocalFilePath:" << package->localFilePath()
                 << "\nfileSize:" << package->fileSize();
    };

    // 初始化服务端
    if ( !server->begin() )
    {
        qDebug() << "Serer begin fail";
        return -1;
    }
    qDebug() << "Server begin succeed";

    // 创建客户端
    auto client = JQNetworkClient::createClient( true );

    // 初始化客户端
    if ( !client->begin() )
    {
        qDebug() << "Client begin fail";
        return -1;
    }
    qDebug() << "Client begin succeed";

    qDebug() << "waitForCreateConnect:" << client->waitForCreateConnect( "127.0.0.1", 26432 );

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
    qDebug() << "sendFileData reply:" << client->sendFileData(
                "127.0.0.1",
                26432,
                QFileInfo( sourceFilePath ),
                [ ](const auto &, const auto &)
                {
                    qDebug( "Send file succeed" );
                },
                [ ](const auto &)
                {
                    qDebug( "Send file fail" );
                }
    );

    return a.exec();
}
