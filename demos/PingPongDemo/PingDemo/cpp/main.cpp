// Qt lib import
#include <QCoreApplication>

// Project lib import
#include "ping.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    JQNetwork::printVersionInformation();

    Ping ping;
    if ( !ping.begin() ) { return -1; }

    return a.exec();
}
