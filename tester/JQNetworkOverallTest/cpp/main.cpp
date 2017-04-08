// Qt lib import
#include <QCoreApplication>
#include <QtTest>

// JQNetwork lib import
#include "JQNetworkFoundation"

// Project import
#include "jqnetwork_overalltest.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    JQNetwork::printVersionInformation();

    JQNetworkOverallTest jqNetworkTest;

    return QTest::qExec( &jqNetworkTest, argc, argv );
}
