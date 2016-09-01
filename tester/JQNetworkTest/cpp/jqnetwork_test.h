#ifndef JQNetworkTest_H
#define JQNetworkTest_H

// Qt lib import
#include <QObject>

class JQNetworkTest: public QObject
{
     Q_OBJECT

private slots:
    void jqNetworkThreadPoolTest();

    void jqNetworkConnectTest();
};

#endif // JQNetworkTest_H
