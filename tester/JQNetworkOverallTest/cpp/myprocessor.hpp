#ifndef __CPP_MYPROCESSOR_HPP__
#define __CPP_MYPROCESSOR_HPP__

// Qt lib import
#include <QThread>

// JQNetwork lib import
#include <JQNetworkProcessor>

class MyProcessor: public JQNetworkProcessor
{
    Q_OBJECT
    Q_DISABLE_COPY( MyProcessor )

public:
    MyProcessor() = default;

    ~MyProcessor() = default;

public slots:
    void actionFlag(const QVariantMap &received, QVariantMap &send)
    {
        testData_ = received;
        testData2_ = QThread::currentThread();
        send = { };
    }

public:
    QVariantMap testData_;
    QThread *testData2_;
};

#endif//__CPP_MYPROCESSOR_HPP__
