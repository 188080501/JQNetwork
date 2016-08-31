// Qt lib import
#include <QCoreApplication>
#include <QtTest>

// Project import
#include "jqnetwork_test.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    JQNetworkTest jqNetworkTest;

    jqNetworkTest.jqnetworkConnectTest();

    return a.exec();

    return QTest::qExec( &jqNetworkTest, argc, argv );
}
