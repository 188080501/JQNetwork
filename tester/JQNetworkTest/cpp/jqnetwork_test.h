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

private slots:
    void jqNetworkThreadPoolTest();

private slots:
    void jqNetworkThreadPoolBenchmark();

public slots:
    void jqNetworkConnectTest();
};

#endif // JQNetworkTest_H
