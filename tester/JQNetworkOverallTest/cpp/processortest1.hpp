#ifndef __CPP_PROCESSORTEST1_HPP__
#define __CPP_PROCESSORTEST1_HPP__

// Qt lib import
#include <QThread>

// JQNetwork lib import
#include <JQNetworkProcessor>

namespace ProcessorTest1
{

class TestProcessor: public JQNetworkProcessor
{
    Q_OBJECT
    Q_DISABLE_COPY( TestProcessor )

public:
    TestProcessor() = default;

    ~TestProcessor() = default;

public slots:
    void actionFlag(const QVariantMap &received)
    {
        testData_ = received;
        testData2_ = QThread::currentThread();
    }

public:
    QVariantMap testData_;
    QThread *testData2_;
};

}

#endif//__CPP_PROCESSORTEST1_HPP__
