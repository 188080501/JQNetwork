#ifndef __CPP_JQNETWORK_PERSISTENETEST_H__
#define __CPP_JQNETWORK_PERSISTENETEST_H__

// Qt lib import
#include <QObject>

class JQNetworkPersisteneTest: public QObject
{
     Q_OBJECT

public:
    JQNetworkPersisteneTest() = default;

    ~JQNetworkPersisteneTest() = default;

    void test();
};

#endif//__CPP_JQNETWORK_PERSISTENETEST_H__
