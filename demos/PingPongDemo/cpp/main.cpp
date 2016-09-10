// Qt lib import
#include <QCoreApplication>
#include <QtTest>

// JQNetwork lib improt
#include <JQNetworkServer>
#include <JQNetworkConnect>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    const quint16 &&listenPort = 34543;
    auto server = JQNetworkServer::createServerByListenPort( listenPort );

    server->setOnPackageReceivedCallback( [](const auto &, const JQNetworkPackageSharedPointer &)
    {
        qDebug() << "onPackageReceived";

        //...
    } );

    if ( !server->begin() )
    {
        qDebug() << "begin fail";
        return -1;
    }

    qDebug() << "begin succeed, listen on:" << listenPort;

    return a.exec();
}
