// Qt lib import
#include <QCoreApplication>
#include <QtTest>

// Project import
#include "jqnetwork_overalltest.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    JQNetworkOverallTest jqNetworkTest;

    return QTest::qExec( &jqNetworkTest, argc, argv );
}
