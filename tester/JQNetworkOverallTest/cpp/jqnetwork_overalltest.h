#ifndef __CPP_JQNETWORK_OVERALLTEST_H__
#define __CPP_JQNETWORK_OVERALLTEST_H__

// Qt lib import
#include <QObject>

class JQNetworkOverallTest: public QObject
{
     Q_OBJECT

public:
    JQNetworkOverallTest() = default;

    ~JQNetworkOverallTest() = default;

private slots:
    void jqNetworkThreadPoolTest();

private slots:
    void jqNetworkThreadPoolBenchmark();

    void jqNetworkThreadPoolBenchmark2();

private slots:
    void jqNetworkNodeMarkTest();

private slots:
    void jqNetworkConnectTest();

private slots:
    void jeNetworkPackageTest();

private slots:
    void jqNetworkServerTest();

private slots:
    void jqNetworkClientTest();

private slots:
    void jqNetworkServerAndClientTest();

private slots:
    void jqNetworkServerAndClientTest2();

private slots:
    void jqNetworkLanTest();
};

#endif//__CPP_JQNETWORK_OVERALLTEST_H__
