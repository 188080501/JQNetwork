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

public slots:
    void jqNetworkThreadPoolTest();

public slots:
    void jqNetworkThreadPoolBenchmark();

    void jqNetworkThreadPoolBenchmark2();

public slots:
    void jqNetworkConnectTest();

public slots:
    void jeNetworkPackageTest();

public slots:
    void jqNetworkServerTest();

private slots:
    void jqNetworkClientTest();
};

#endif//__CPP_JQNETWORK_TEST_H__
