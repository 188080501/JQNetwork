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

//    for ( auto count = 2; count; --count )
//    {
//        if ( QTest::qExec( &jqNetworkTest, argc, argv ) )
//        {
//            return -1;
//        }
//    }
//    return 0;

    return QTest::qExec( &jqNetworkTest, argc, argv );
}
