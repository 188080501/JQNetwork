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
#include <vector>

// Qt lib import
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QPointer>
#include <QHostAddress>
#include <QMutex>

#define JQNETWORKPACKAGE_BOOTFLAG 0x7d
#define JQNETWORKPACKAGE_VERSION 1

#define NULLPTR_CHECK( ptr, ... ) \
    if ( !ptr ) { qDebug( "%s: %s is null", __func__, # ptr ); return __VA_ARGS__; }

class QJsonObject;
class QJsonArray;
class QSemaphore;
class QJsonValue;
class QMutex;
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

struct JQNetworkConnectSettings;
struct JQNetworkConnectPoolSettings;
struct JQNetworkServerSettings;
struct JQNetworkClientSettings;

typedef QSharedPointer< JQNetworkPackage > JQNetworkPackageSharedPointer;
typedef QSharedPointer< JQNetworkConnect > JQNetworkConnectSharedPointer;
typedef QPointer< JQNetworkConnect > JQNetworkConnectPointer;
typedef QSharedPointer< JQNetworkConnectPool > JQNetworkConnectPoolSharedPointer;
typedef QSharedPointer< JQNetworkServer > JQNetworkServerSharedPointer;
typedef QSharedPointer< JQNetworkProcessor > JQNetworkProcessorSharedPointer;
typedef QSharedPointer< JQNetworkClient > JQNetworkClientSharedPointer;

typedef QSharedPointer< JQNetworkConnectSettings > JQNetworkConnectSettingsSharedPointer;
typedef QSharedPointer< JQNetworkConnectPoolSettings > JQNetworkConnectPoolSettingsSharedPointer;
typedef QSharedPointer< JQNetworkServerSettings > JQNetworkServerSettingsSharedPointer;
typedef QSharedPointer< JQNetworkClientSettings > JQNetworkClientSettingsSharedPointer;

struct JQNetworkOnReceivedCallbackPackage
{
    std::function< void(const JQNetworkConnectPointer &connect, const JQNetworkPackageSharedPointer &) > succeedCallback = nullptr;
    std::function< void(const JQNetworkConnectPointer &connect) > failCallback = nullptr;
};

class JQNetworkThreadPoolHelper: public QObject
{
    Q_OBJECT

public:
    JQNetworkThreadPoolHelper();

    ~JQNetworkThreadPoolHelper() = default;

    void run(const std::function< void() > &callback);

public Q_SLOTS:
    void onRun();

private:
    QMutex mutex_;
    QSharedPointer< std::vector< std::function< void() > > > waitForRunCallbacks_;
    bool alreadyCall_ = false;
    qint64 lastRunTime_ = 0;
    int lastRunCallbackCount_ = 0;
};

class JQNetworkThreadPool: public QObject
{
    Q_OBJECT

public:
    JQNetworkThreadPool(const int &threadCount);

    ~JQNetworkThreadPool();

    JQNetworkThreadPool(const JQNetworkThreadPool &) = delete;

    JQNetworkThreadPool &operator =(const JQNetworkThreadPool &) = delete;

    inline int nextRotaryIndex();

    int run(const std::function< void() > &callback, const int &threadIndex = -1);

    inline void runEach(const std::function< void() > &callback);

    int waitRun(const std::function< void() > &callback, const int &threadIndex = -1);

    inline void waitRunEach(const std::function< void() > &callback);

private:
    QSharedPointer< QThreadPool > threadPool_;
    QSharedPointer< QVector< QPointer< QEventLoop > > > eventLoops_;
    QSharedPointer< QVector< QPointer< JQNetworkThreadPoolHelper > > > helpers_;
    int rotaryIndex_ = -1;
};

#include "jqnetwork_foundation.inc"

#endif//JQNETWORK_INCLUDE_JQNETWORK_FOUNDATION_H
