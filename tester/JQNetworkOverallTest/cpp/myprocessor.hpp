#ifndef __CPP_MYPROCESSOR_HPP__
#define __CPP_MYPROCESSOR_HPP__

// Qt lib import
#include <QJsonObject>
#include <QThread>

// JQNetwork lib import
#include <JQNetworkProcessor>

class MyProcessor: public JQNetworkProcessor
{
    Q_OBJECT

public slots:
    void actionFlag(const QJsonObject &received, QJsonObject &send)
    {
        testData_ = received;
        testData2_ = QThread::currentThread();
        send = { };
    }

public:
    QJsonObject testData_;
    QThread *testData2_;
};

#endif//__CPP_MYPROCESSOR_HPP__
