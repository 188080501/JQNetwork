/*
    This file is part of JQNetwork
    
    Library introduce: https://github.com/188080501/JQNetwork

    Copyright: Jason

    Contact email: Jason@JasonServer.com
    
    GitHub: https://github.com/188080501/
*/

#ifndef JQNETWORK_INCLUDE_JQNETWORK_FOUNDATION_H
#define JQNETWORK_INCLUDE_JQNETWORK_FOUNDATION_H

// C++ lib import
#include <functional>

// Qt lib import
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QSharedPointer>
#include <QPointer>
#include <QHostAddress>

#define NULLPTR_CHECK( ptr ) \
    if ( !ptr ) { qDebug( "%s: %s is null", __func__, # ptr ); return; }

class QJsonObject;
class QJsonArray;
class QJsonValue;
class QTimer;
class QThreadPool;
class QEventLoop;
class QTcpSocket;
class QTcpServer;

template < typename T > class QVector;
template < class Key, class T > class QMap;

class JQNetworkPackage;
class JQNetworkConnect;
class JQNetworkConnectPool;
class JQNetworkServer;
class JQNetworkProcessor;
class JQNetworkClient;

namespace JQNetwork
{ }

class JQNetworkThreadPoolHelper: public QObject
{
    Q_OBJECT

public slots:
    inline void run(const std::function< void() > &callback);
};

class JQNetworkThreadPool: public QObject
{
    Q_OBJECT

public:
    JQNetworkThreadPool(const int &threadCount);

    ~JQNetworkThreadPool();

    JQNetworkThreadPool(const JQNetworkThreadPool &) = delete;

    JQNetworkThreadPool &operator =(const JQNetworkThreadPool &) = delete;

public:
    void run(const std::function< void() > &callback);

private:
    QSharedPointer< QThreadPool > threadPool_;
    QSharedPointer< QVector< QPointer< QEventLoop > > > eventLoops_;
    QSharedPointer< QVector< QPointer< JQNetworkThreadPoolHelper > > > helpers_;
    int rotaryIndex_ = -1;
};

#include "jqnetwork_foundation.inc"

#endif//JQNETWORK_INCLUDE_JQNETWORK_FOUNDATION_H
