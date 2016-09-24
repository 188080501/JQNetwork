#ifndef __CPP_JQNETWORK_BENCHMARK_H__
#define __CPP_JQNETWORK_BENCHMARK_H__

// Qt lib import
#include <QObject>

class JQNetworkBenchmark: public QObject
{
     Q_OBJECT

public:
    JQNetworkBenchmark() = default;

    ~JQNetworkBenchmark() = default;

    void test1();

    void test2();

    void test3();

    void test4();

    void test5();
};

#endif//__CPP_JQNETWORK_BENCHMARK_H__
