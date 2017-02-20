// Qt lib import
#include <QCoreApplication>

// Project lib import
#include "myserver.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    JQNetwork::printVersionInformation();

    MyServer myServer;
    if ( !myServer.begin() ) { return -1; }

    return a.exec();
}
