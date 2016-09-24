// Qt lib import
#include <QCoreApplication>
#include <QtTest>

// Project import
#include "jqnetwork_test.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    JQNetworkBenchmark jqNetworkTest;

//    jqNetworkTest.jqNetworkThreadPoolTest();
//    return app.exec();

    return QTest::qExec( &jqNetworkTest, argc, argv );
}
