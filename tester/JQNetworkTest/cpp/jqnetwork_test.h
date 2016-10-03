#ifndef __CPP_JQNETWORK_TEST_H__
#define __CPP_JQNETWORK_TEST_H__

// Qt lib import
#include <QObject>

class JQNetworkTest: public QObject
{
     Q_OBJECT

public:
    JQNetworkTest() = default;

    ~JQNetworkTest() = default;

private slots:
    void jqNetworkThreadPoolTest();

private slots:
    void jqNetworkThreadPoolBenchmark();

    void jqNetworkThreadPoolBenchmark2();

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
};

#endif//__CPP_JQNETWORK_TEST_H__
