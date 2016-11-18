// Qt lib import
#include <QCoreApplication>
#include <QFile>

// JQNetwork lib improt
#include <JQNetworkPackage>
#include <JQNetworkConnect>
#include <JQNetworkServer>
#include <JQNetworkClient>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    JQNetwork::printVersionInformation();

    auto server = JQNetworkServer::createServer( 26432, QHostAddress::Any, true );
    if ( !server->begin() )
    {
        qDebug() << "Serer begin fail";
        return -1;
    }
    qDebug() << "Server begin succeed";

    auto client = JQNetworkClient::createClient( true );
    if ( !client->begin() )
    {
        qDebug() << "Client begin fail";
        return -1;
    }
    qDebug() << "Client begin succeed";

    return a.exec();
}
