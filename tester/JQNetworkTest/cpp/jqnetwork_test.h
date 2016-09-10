#ifndef JQNetworkTest_H
#define JQNetworkTest_H

// Qt lib import
#include <QObject>

class JQNetworkTest: public QObject
{
     Q_OBJECT

public:
    JQNetworkTest() = default;

    ~JQNetworkTest() = default;

public slots:
    void jqNetworkThreadPoolTest();

public slots:
    void jqNetworkThreadPoolBenchmark();

    void jqNetworkThreadPoolBenchmark2();

public slots:
    void jqNetworkServerTest();

public slots:
    void jqNetworkConnectTest();

public slots:
    void jeNetworkPackageTest();
};

#endif // JQNetworkTest_H
