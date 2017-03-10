// Qt lib import
#include <QCoreApplication>

// Project lib import
#include "pong.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    JQNetwork::printVersionInformation();

    Pong pong;
    if ( !pong.begin() ) { return -1; }

    return a.exec();
}
