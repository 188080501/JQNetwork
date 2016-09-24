// Qt lib import
#include <QCoreApplication>
#include <QtTest>

// Project import
#include "jqnetwork_benchmark.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    JQNetworkBenchmark benchmark;

    qDebug() << "----- test1 start -----";
    benchmark.test1();
    qDebug() << "----- test1 end -----";

    qDebug() << "----- test2 start -----";
    benchmark.test2();
    qDebug() << "----- test2 end -----";

    qDebug() << "----- test3 start -----";
    benchmark.test3();
    qDebug() << "----- test3 end -----";

    qDebug() << "----- test4 start -----";
    benchmark.test4();
    qDebug() << "----- test4 end -----";

    qDebug() << "----- test5 start -----";
    benchmark.test5();
    qDebug() << "----- test5 end -----";
}

