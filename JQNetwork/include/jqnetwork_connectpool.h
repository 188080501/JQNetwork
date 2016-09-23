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

struct JQNetworkConnectPoolSettings
{
    bool autoReconnect = true;
    bool reconnectIntervalTime = 15 * 1000;

    std::function< void( JQNetworkConnectPointer ) > connectToHostErrorCallback = nullptr;
    std::function< void( JQNetworkConnectPointer ) > connectToHostTimeoutCallback = nullptr;
    std::function< void( JQNetworkConnectPointer ) > connectToHostSucceedCallback = nullptr;
    std::function< void( JQNetworkConnectPointer ) > remoteHostClosedCallback = nullptr;
    std::function< void( JQNetworkConnectPointer ) > readyToDeleteCallback = nullptr;
    std::function< void( JQNetworkConnectPointer, JQNetworkPackageSharedPointer ) > packageReceivedCallback = nullptr;
};

class JQNetworkConnectPool: public QObject
{
    Q_OBJECT

public:
    JQNetworkConnectPool(
            JQNetworkConnectPoolSettingsSharedPointer connectPoolSettings,
            JQNetworkConnectSettingsSharedPointer connectSettings
        );

    ~JQNetworkConnectPool() = default;

    JQNetworkConnectPool(const JQNetworkConnectPool &) = delete;

    JQNetworkConnectPool &operator =(const JQNetworkConnectPool &) = delete;

    void createConnect(const std::function< void( std::function< void() > ) > runOnConnectThreadCallback, const QString &hostName, const quint16 &port);

    void createConnect(const std::function< void( std::function< void() > ) > runOnConnectThreadCallback, const qintptr &socketDescriptor);

    inline bool containsConnect(const QString &hostName, const quint16 &port);

    inline bool containsConnect(const qintptr &socketDescriptor);

    QPair< QString, quint16 > getHostAndPortByConnect(const JQNetworkConnectPointer &connect);

    qintptr getSocketDescriptorByConnect(const JQNetworkConnectPointer &connect);

    JQNetworkConnectPointer getConnectByHostAndPort(const QString &hostName, const quint16 &port);

    JQNetworkConnectPointer getConnectBySocketDescriptor(const qintptr &socketDescriptor);

private:
    inline void onConnectToHostError(const JQNetworkConnectPointer &connect);

    inline void onConnectToHostTimeout(const JQNetworkConnectPointer &connect);

    void onConnectToHostSucceed(const JQNetworkConnectPointer &connect);

    inline void onRemoteHostClosed(const JQNetworkConnectPointer &connect);

    void onReadyToDelete(const JQNetworkConnectPointer &connect);

    inline void onPackageReceived(const JQNetworkConnectPointer &connect, const JQNetworkPackageSharedPointer &package);

private:
    // Settings
    JQNetworkConnectPoolSettingsSharedPointer connectPoolSettings_;
    JQNetworkConnectSettingsSharedPointer connectSettings_;

    // Connect
    QMap< JQNetworkConnect *, JQNetworkConnectSharedPointer > connectForConnecting_;
    QMap< JQNetworkConnect *, JQNetworkConnectSharedPointer > connectForConnected_;

    QMap< QString, JQNetworkConnectPointer > bimapForHostAndPort1; // "127.0.0.1:34543" -> Connect
    QMap< JQNetworkConnect *, QString > bimapForHostAndPort2; // Connect -> "127.0.0.1:34543"
    QMap< qintptr, JQNetworkConnectPointer > bimapForSocketDescriptor1; // socketDescriptor -> Connect
    QMap< JQNetworkConnect *, qintptr > bimapForSocketDescriptor2; // Connect -> socketDescriptor

    // Other
    QMutex mutex_;
};

#include "jqnetwork_connectpool.inc"

#endif//JQNETWORK_INCLUDE_JQNETWORK_CONNECTPOOL_H
