#ifndef __CPP_JQNETWORK_TEST_H__
#define __CPP_JQNETWORK_TEST_H__

// Qt lib import
#include <QObject>

class JQNetworkBenchmark: public QObject
{
     Q_OBJECT

public:
    JQNetworkBenchmark() = default;

    ~JQNetworkBenchmark() = default;

private slots:
    void jqNetworkThreadPoolTest();

private slots:
    void jqNetworkThreadPoolBenchmark();

    void jqNetworkThreadPoolBenchmark2();

private slots:
    void jqNetworkServerTest();

private slots:
    void jqNetworkClientTest();

private slots:
    void jqNetworkConnectTest();

private slots:
    void jeNetworkPackageTest();
};

#endif//__CPP_JQNETWORK_TEST_H__
