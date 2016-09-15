/*
    This file is part of JQNetwork
    
    Library introduce: https://github.com/188080501/JQNetwork

    Copyright: Jason

    Contact email: Jason@JasonServer.com
    
    GitHub: https://github.com/188080501/
*/

#ifndef JQNETWORK_INCLUDE_JQNETWORK_CLIENG_H
#define JQNETWORK_INCLUDE_JQNETWORK_CLIENG_H

// JQNetwork lib import
#include <JQNetworkFoundation>

namespace JQNetwork
{ }

struct JQNetworkClientSettings
{
    int globalSocketThreadCount = 2;
    int globalProcessorThreadCount = 1;

    std::function< void( JQNetworkConnectPointer, const QString &hostName, const quint16 &port ) > connectToHostSucceedCallback = nullptr;
};

struct JQNetworkClientSendCallbackPackage
{
    std::function< void(const JQNetworkConnectPointer &connect) > succeedCallback;
    std::function< void(const JQNetworkConnectPointer &connect) > failCallback;
};

class JQNetworkClient: public QObject
{
    Q_OBJECT

public:
    JQNetworkClient(
            const JQNetworkClientSettingsSharedPointer &clientSettings,
            const JQNetworkConnectPoolSettingsSharedPointer connectPoolSettings,
            const JQNetworkConnectSettingsSharedPointer connectSettings
        );

    ~JQNetworkClient() = default;

    JQNetworkClient(const JQNetworkClient &) = delete;

    JQNetworkClient &operator =(const JQNetworkClient &) = delete;

    static JQNetworkClientSharedPointer createClient();

    inline void setOnConnectToHostSucceedCallback(const std::function< void( JQNetworkConnectPointer, const QString &hostName, const quint16 &port ) > &callback);

    bool begin();

    void createConnect(const QString &hostName, const quint16 &port);

    int sendPayloadData(const QString &hostName, const quint16 &port, const QByteArray &payloadData);

private:
    inline void onConnectToHostError(const JQNetworkConnectPointer &connect);

    inline void onConnectToHostTimeout(const JQNetworkConnectPointer &connect);

    void onConnectToHostSucceed(const JQNetworkConnectPointer &connect);

    inline void onRemoteHostClosed(const JQNetworkConnectPointer &connect);

    inline void onReadyToDelete(const JQNetworkConnectPointer &connect);

    void onPackageReceived(const JQNetworkConnectPointer &connect, const JQNetworkPackageSharedPointer &package);

private:
    // Thread pool
    static QWeakPointer< JQNetworkThreadPool > globalSocketThreadPool_;
    QSharedPointer< JQNetworkThreadPool > socketThreadPool_;
    static QWeakPointer< JQNetworkThreadPool > globalProcessorThreadPool_;
    QSharedPointer< JQNetworkThreadPool > processorThreadPool_;

    // Settings
    JQNetworkClientSettingsSharedPointer clientSettings_;
    JQNetworkConnectPoolSettingsSharedPointer connectPoolSettings_;
    JQNetworkConnectSettingsSharedPointer connectSettings_;

    // Callbacks
    QMap< int, JQNetworkClientSendCallbackPackage > sendCallbackPackages_;

    // Client
    QMap< QThread *, JQNetworkConnectPoolSharedPointer > connectPools_;
};

#include "jqnetwork_client.inc"

#endif//JQNETWORK_INCLUDE_JQNETWORK_CLIENG_H
