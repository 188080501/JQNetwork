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

    std::function< void( QPointer< JQNetworkConnect > ) > connectToHostErrorCallback = nullptr;
    std::function< void( QPointer< JQNetworkConnect > ) > connectToHostTimeoutCallback = nullptr;
    std::function< void( QPointer< JQNetworkConnect > ) > connectToHostSucceedCallback = nullptr;
    std::function< void( QPointer< JQNetworkConnect > ) > remoteHostClosedCallback = nullptr;
    std::function< void( QPointer< JQNetworkConnect > ) > readyToDeleteCallback = nullptr;
};

class JQNetworkServer: public QObject
{
    Q_OBJECT

public:
    JQNetworkServer(
            const QSharedPointer< JQNetworkServerSettings > serverSettings,
            const QSharedPointer< JQNetworkConnectPoolSettings > connectPoolSettings,
            const QSharedPointer< JQNetworkConnectSettings > connectSettings
        );

    ~JQNetworkServer();

    JQNetworkServer(const JQNetworkServer &) = delete;

    JQNetworkServer &operator =(const JQNetworkServer &) = delete;

    bool begin();

private:
    void incomingConnection(const qintptr &socketDescriptor);

    void onConnectToHostError(const QPointer< JQNetworkConnect > &connect);

    void onConnectToHostTimeout(const QPointer< JQNetworkConnect > &connect);

    void onConnectToHostSucceed(const QPointer< JQNetworkConnect > &connect);

    void onRemoteHostClosed(const QPointer< JQNetworkConnect > &connect);

    void onReadyToDelete(const QPointer< JQNetworkConnect > &connect);

private:
    // Server thread
    static QWeakPointer< JQNetworkThreadPool > globalServerThreadPool_;
    QSharedPointer< JQNetworkThreadPool > serverThreadPool_;
    static QWeakPointer< JQNetworkThreadPool > globalSocketThreadPool_;
    QSharedPointer< JQNetworkThreadPool > socketThreadPool_;
    static QWeakPointer< JQNetworkThreadPool > globalProcessorThreadPool_;
    QSharedPointer< JQNetworkThreadPool > processorThreadPool_;

    // Settings
    QSharedPointer< JQNetworkServerSettings > serverSettings_;
    QSharedPointer< JQNetworkConnectPoolSettings > connectPoolSettings_;
    QSharedPointer< JQNetworkConnectSettings > connectSettings_;

    // Server
    QSharedPointer< QTcpServer > tcpServer_;
    QMap< QThread *, QSharedPointer< JQNetworkConnectPool > > connectPools_;
};

#include "jqnetwork_server.inc"

#endif//JQNETWORK_INCLUDE_JQNETWORK_SERVER_H
