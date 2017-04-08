#ifndef __CPP_JQNETWORK_OVERALLTEST_H__
#define __CPP_JQNETWORK_OVERALLTEST_H__

// Qt lib import
#include <QObject>

//#define PRIVATEMACRO public
#define PRIVATEMACRO private

class JQNetworkOverallTest: public QObject
{
     Q_OBJECT
    Q_DISABLE_COPY( JQNetworkOverallTest )

public:
    JQNetworkOverallTest() = default;

    ~JQNetworkOverallTest() = default;

PRIVATEMACRO slots:
    void jqNetworkThreadPoolTest();

PRIVATEMACRO slots:
    void jqNetworkThreadPoolBenchmark1();

PRIVATEMACRO slots:
    void jqNetworkThreadPoolBenchmark2();

PRIVATEMACRO slots:
    void jqNetworkNodeMarkTest();

PRIVATEMACRO slots:
    void jqNetworkConnectTest();

PRIVATEMACRO slots:
    void jeNetworkPackageTest();

PRIVATEMACRO slots:
    void jqNetworkServerTest();

PRIVATEMACRO slots:
    void jqNetworkClientTest();

PRIVATEMACRO slots:
    void jqNetworkServerAndClientTest1();

PRIVATEMACRO slots:
    void jqNetworkServerAndClientTest2();

PRIVATEMACRO slots:
    void jqNetworkLanTest();

PRIVATEMACRO slots:
    void jqNetworkProcessorTest1();

PRIVATEMACRO slots:
    void jqNetworkProcessorTest2();

PRIVATEMACRO slots:
    void jqNetworkSendFile();

PRIVATEMACRO slots:
//    private slots:
    void fusionTest1();
};

#endif//__CPP_JQNETWORK_OVERALLTEST_H__
