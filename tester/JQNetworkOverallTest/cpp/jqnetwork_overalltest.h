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

public slots:
    void jqNetworkThreadPoolTest();

public slots:
    void jqNetworkThreadPoolBenchmark();

    void jqNetworkThreadPoolBenchmark2();

public slots:
    void jqNetworkNodeMarkTest();

public slots:
    void jqNetworkConnectTest();

public slots:
    void jeNetworkPackageTest();

public slots:
    void jqNetworkServerTest();

public slots:
    void jqNetworkClientTest();

public slots:
    void jqNetworkServerAndClientTest();

public slots:
    void jqNetworkServerAndClientTest2();

public slots:
    void jqNetworkLanTest();

private slots:
    void jqNetworkProcessorTest();
};

#endif//__CPP_JQNETWORK_OVERALLTEST_H__
