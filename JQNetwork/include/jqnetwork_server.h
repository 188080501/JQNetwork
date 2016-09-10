/*
    This file is part of JQNetwork
    
    Library introduce: https://github.com/188080501/JQNetwork

    Copyright: Jason

    Contact email: Jason@JasonServer.com
    
    GitHub: https://github.com/188080501/
*/

#ifndef JQNETWORK_INCLUDE_JQNETWORK_SERVER_H
#define JQNETWORK_INCLUDE_JQNETWORK_SERVER_H

// JQNetwork lib import
#include <JQNetworkFoundation>

namespace JQNetwork
{ }

struct JQNetworkServerSettings
{
    int globalServerThreadCount = 1;
    int globalSocketThreadCount = 2;
    int globalProcessorThreadCount = 2;

    QHostAddress listenAddress = QHostAddress::Any;
    quint16 listenPort = 0;

    std::function< void( JQNetworkConnectPointer ) > connectToHostErrorCallback = nullptr;
    std::function< void( JQNetworkConnectPointer ) > connectToHostTimeoutCallback = nullptr;
    std::function< void( JQNetworkConnectPointer ) > connectToHostSucceedCallback = nullptr;
    std::function< void( JQNetworkConnectPointer ) > remoteHostClosedCallback = nullptr;
    std::function< void( JQNetworkConnectPointer ) > readyToDeleteCallback = nullptr;
    std::function< void( JQNetworkConnectPointer, JQNetworkPackageSharedPointer ) > onPackageReceivedCallback = nullptr;
};

class JQNetworkServer: public QObject
{
    Q_OBJECT

public:
    JQNetworkServer(
            const JQNetworkServerSettingsSharedPointer serverSettings,
            const JQNetworkConnectPoolSettingsSharedPointer connectPoolSettings,
            const JQNetworkConnectSettingsSharedPointer connectSettings
        );

    ~JQNetworkServer();

    JQNetworkServer(const JQNetworkServer &) = delete;

    JQNetworkServer &operator =(const JQNetworkServer &) = delete;

    static JQNetworkServerSharedPointer createServerByListenPort(const quint16 &listenPort, const QHostAddress &listenAddress = QHostAddress::Any);

    void setOnPackageReceivedCallback(const std::function< void( JQNetworkConnectPointer, JQNetworkPackageSharedPointer ) > &callback);

    bool begin();

private:
    void incomingConnection(const qintptr &socketDescriptor);

    inline void onConnectToHostError(const JQNetworkConnectPointer &connect);

    inline void onConnectToHostTimeout(const JQNetworkConnectPointer &connect);

    inline void onConnectToHostSucceed(const JQNetworkConnectPointer &connect);

    inline void onRemoteHostClosed(const JQNetworkConnectPointer &connect);

    inline void onReadyToDelete(const JQNetworkConnectPointer &connect);

    inline void onPackageReceivedCallback(const JQNetworkConnectPointer &connect, const JQNetworkPackageSharedPointer &package);

private:
    // Thread pool
    static QWeakPointer< JQNetworkThreadPool > globalServerThreadPool_;
    QSharedPointer< JQNetworkThreadPool > serverThreadPool_;
    static QWeakPointer< JQNetworkThreadPool > globalSocketThreadPool_;
    QSharedPointer< JQNetworkThreadPool > socketThreadPool_;
    static QWeakPointer< JQNetworkThreadPool > globalProcessorThreadPool_;
    QSharedPointer< JQNetworkThreadPool > processorThreadPool_;

    // Settings
    JQNetworkServerSettingsSharedPointer serverSettings_;
    JQNetworkConnectPoolSettingsSharedPointer connectPoolSettings_;
    JQNetworkConnectSettingsSharedPointer connectSettings_;

    // Server
    QSharedPointer< QTcpServer > tcpServer_;
    QMap< QThread *, JQNetworkConnectPoolSharedPointer > connectPools_;
};

#include "jqnetwork_server.inc"

#endif//JQNETWORK_INCLUDE_JQNETWORK_SERVER_H
