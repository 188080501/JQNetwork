/*
    This file is part of JQNetwork
    
    Library introduce: https://github.com/188080501/JQNetwork

    Copyright: Jason

    Contact email: Jason@JasonServer.com
    
    GitHub: https://github.com/188080501/
*/

#ifndef JQNETWORK_INCLUDE_JQNETWORK_CONNECTPOOL_H
#define JQNETWORK_INCLUDE_JQNETWORK_CONNECTPOOL_H

// JQNetwork lib import
#include <JQNetworkFoundation>

namespace JQNetwork
{ }

struct JQNetworkConnectPoolSettings
{
    bool autoReconnect = true;
    bool reconnectIntervalTime = 15 * 1000;

    std::function< void( QPointer< JQNetworkConnect > ) > connectToHostErrorCallback = nullptr;
    std::function< void( QPointer< JQNetworkConnect > ) > connectToHostTimeoutCallback = nullptr;
    std::function< void( QPointer< JQNetworkConnect > ) > connectToHostSucceedCallback = nullptr;
    std::function< void( QPointer< JQNetworkConnect > ) > remoteHostClosedCallback = nullptr;
    std::function< void( QPointer< JQNetworkConnect > ) > readyToDeleteCallback = nullptr;
};

class JQNetworkConnectPool: public QObject
{
    Q_OBJECT

public:
    JQNetworkConnectPool(
            QSharedPointer< JQNetworkConnectPoolSettings > connectPoolSettings,
            QSharedPointer< JQNetworkConnectSettings > connectSettings
        );

    ~JQNetworkConnectPool() = default;

    JQNetworkConnectPool(const JQNetworkConnectPool &) = delete;

    JQNetworkConnectPool &operator =(const JQNetworkConnectPool &) = delete;

public:
    void createConnectByHostAndPort(const QString &hostName, const quint16 &port);

    void createConnectBySocketDescriptor(const qintptr &socketDescriptor);

private:
    void onConnectToHostError(const QPointer< JQNetworkConnect > &connect);

    void onConnectToHostTimeout(const QPointer< JQNetworkConnect > &connect);

    void onConnectToHostSucceed(const QPointer< JQNetworkConnect > &connect);

    void onRemoteHostClosed(const QPointer< JQNetworkConnect > &connect);

    void onReadyToDelete(const QPointer< JQNetworkConnect > &connect);

private:
    // Settings
    QSharedPointer< JQNetworkConnectPoolSettings > connectPoolSettings_;
    QSharedPointer< JQNetworkConnectSettings > connectSettings_;

    // Connect
    QMap< JQNetworkConnect *, QSharedPointer< JQNetworkConnect > > connectForConnecting_;
    QMap< JQNetworkConnect *, QSharedPointer< JQNetworkConnect > > connectForConnected_;
};

#include "jqnetwork_connectpool.inc"

#endif//JQNETWORK_INCLUDE_JQNETWORK_CONNECTPOOL_H
