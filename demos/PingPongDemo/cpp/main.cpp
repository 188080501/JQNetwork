// Qt lib import
#include <QCoreApplication>
#include <QtTest>
#include <QTcpSocket>

// JQNetwork lib improt
#include <JQNetworkPackage>
#include <JQNetworkServer>
#include <JQNetworkConnect>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    const quint16 &&listenPort = 34543;
    auto server = JQNetworkServer::createServerByListenPort( listenPort );

    server->setOnPackageReceivedCallback( [](const auto &, const JQNetworkPackageSharedPointer &package)
    {
        qDebug() << "onPackageReceived:" << package->payloadDataSize();

        //...
    } );

    if ( !server->begin() )
    {
        qDebug() << "begin fail";
        return -1;
    }

    qDebug() << "begin succeed, listen on:" << listenPort;

    QTcpSocket socket;
    socket.connectToHost( "127.0.0.1", 34543 );

    return a.exec();
}
