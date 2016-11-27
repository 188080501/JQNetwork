// Qt lib import
#include <QCoreApplication>
#include <QtTest>

// JQNetwork lib import
#include "JQNetworkFoundation"

// Project import
#include "jqnetwork_persistenetest.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    JQNetwork::printVersionInformation();

    JQNetworkPersisteneTest persisteneTest;

    persisteneTest.test();
}

