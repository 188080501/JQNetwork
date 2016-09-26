// Qt lib import
#include <QCoreApplication>
#include <QtTest>

// Project import
#include "jqnetwork_test.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    JQNetworkTest jqNetworkTest;

    return QTest::qExec( &jqNetworkTest, argc, argv );
}
