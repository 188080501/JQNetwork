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

struct JQNetworkClientSettings
{
    int globalSocketThreadCount = 2;
    int globalProcessorThreadCount = 1;

    std::function< void( const JQNetworkConnectPointer &, const QString &hostName, const quint16 &port ) > connectToHostErrorCallback = nullptr;
    std::function< void( const JQNetworkConnectPointer &, const QString &hostName, const quint16 &port ) > connectToHostTimeoutCallback = nullptr;
    std::function< void( const JQNetworkConnectPointer &, const QString &hostName, const quint16 &port ) > connectToHostSucceedCallback = nullptr;
    std::function< void( const JQNetworkConnectPointer &, const QString &hostName, const quint16 &port ) > remoteHostClosedCallback = nullptr;
    std::function< void( const JQNetworkConnectPointer &, const QString &hostName, const quint16 &port ) > readyToDeleteCallback = nullptr;
    std::function< void( const JQNetworkConnectPointer &, const JQNetworkPackageSharedPointer &, const QString &hostName, const quint16 &port ) > packageReceivedCallback = nullptr;
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

    ~JQNetworkClient();

    JQNetworkClient(const JQNetworkClient &) = delete;

    JQNetworkClient &operator =(const JQNetworkClient &) = delete;

    static JQNetworkClientSharedPointer createClient();

    inline void setOnConnectToHostSucceedCallback(
            const std::function< void( JQNetworkConnectPointer, const QString &hostName, const quint16 &port ) > &callback
        );

    bool begin();

    void createConnect(const QString &hostName, const quint16 &port);

    bool waitForCreateConnect(const QString &hostName, const quint16 &port, const int &timeout = 30 * 1000);

    int sendPayloadData(
            const QString &hostName,
            const quint16 &port,
            const QByteArray &payloadData,
            const std::function< void(const JQNetworkConnectPointer &connect, const JQNetworkPackageSharedPointer &) > &succeedCallback = nullptr,
            const std::function< void(const JQNetworkConnectPointer &connect) > &failCallback = nullptr
        );

private:
    void onConnectToHostError(const JQNetworkConnectPointer &connect);

    void onConnectToHostTimeout(const JQNetworkConnectPointer &connect);

    void onConnectToHostSucceed(const JQNetworkConnectPointer &connect);

    void onRemoteHostClosed(const JQNetworkConnectPointer &connect);

    void onReadyToDelete(const JQNetworkConnectPointer &connect);

    void onPackageReceived(const JQNetworkConnectPointer &connect, const JQNetworkPackageSharedPointer &package);

    void onWaitReplySucceedPackage(
            const JQNetworkConnectPointer &connect,
            const JQNetworkPackageSharedPointer &package,
            const std::function< void(const JQNetworkConnectPointer &connect, const JQNetworkPackageSharedPointer &) > &callback
        );

    void onWaitReplyPackageFail(
            const JQNetworkConnectPointer &connect,
            const std::function< void(const JQNetworkConnectPointer &connect) > &callback
        );

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

    // Client
    QMap< QThread *, JQNetworkConnectPoolSharedPointer > connectPools_;

    // Other
    QMutex mutex_;
    QMap< QString, QSharedPointer< QSemaphore > > waitConnectSucceedSemaphore_; // "127.0.0.1:34543" -> Connect
};

#include "jqnetwork_client.inc"

#endif//JQNETWORK_INCLUDE_JQNETWORK_CLIENG_H
