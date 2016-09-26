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

//    QFile file( "/Users/Jason/Desktop/test.psd" );
//    file.open( QIODevice::ReadOnly );

//    const auto &&sourceData = file.readAll();

//    qDebug() << sourceData.size();

//    auto test = [ & ]( const int &level )
//    {
//        const auto &&startTime = QDateTime::currentMSecsSinceEpoch();
//        const auto &&result = qCompress( sourceData, level ).size();

//        qDebug() << "level:" << level << result << ( QDateTime::currentMSecsSinceEpoch() - startTime );
//    };

//    test( -1 );
//    test( 1 );
//    test( 2 );
//    test( 3 );
//    test( 4 );
//    test( 5 );
//    test( 6 );
//    test( 7 );
//    test( 8 );
//    test( 9 );
}

