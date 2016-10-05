﻿/*
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
    int globalSocketThreadCount = JQNETWORK_ADVISE_THREADCOUNT;
    int globalProcessorThreadCount = 1;

    std::function< void( const JQNetworkConnectPointer &, const QString &hostName, const quint16 &port ) > connectToHostErrorCallback = nullptr;
    std::function< void( const JQNetworkConnectPointer &, const QString &hostName, const quint16 &port ) > connectToHostTimeoutCallback = nullptr;
    std::function< void( const JQNetworkConnectPointer &, const QString &hostName, const quint16 &port ) > connectToHostSucceedCallback = nullptr;
    std::function< void( const JQNetworkConnectPointer &, const QString &hostName, const quint16 &port ) > remoteHostClosedCallback = nullptr;
    std::function< void( const JQNetworkConnectPointer &, const QString &hostName, const quint16 &port ) > readyToDeleteCallback = nullptr;
    std::function< void( const JQNetworkConnectPointer &, const QString &hostName, const quint16 &port, const qint32 &, const qint64 &, const qint64 &, const qint64 & ) > packageSendingCallback = nullptr;
    std::function< void( const JQNetworkConnectPointer &, const QString &hostName, const quint16 &port, const qint32 &, const qint64 &, const qint64 &, const qint64 & ) > packageReceivingCallback = nullptr;
    std::function< void( const JQNetworkConnectPointer &, const QString &hostName, const quint16 &port, const JQNetworkPackageSharedPointer & ) > packageReceivedCallback = nullptr;
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

    inline JQNetworkClientSettingsSharedPointer clientSettings();

    inline JQNetworkConnectPoolSettingsSharedPointer connectPoolSettings();

    inline JQNetworkConnectSettingsSharedPointer connectSettings();

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

    JQNetworkConnectPointer getConnect(const QString &hostName, const quint16 &port);

private:
    void onConnectToHostError(const JQNetworkConnectPointer &connect, const JQNetworkConnectPoolPointer &connectPool);

    void onConnectToHostTimeout(const JQNetworkConnectPointer &connect, const JQNetworkConnectPoolPointer &connectPool);

    void onConnectToHostSucceed(const JQNetworkConnectPointer &connect, const JQNetworkConnectPoolPointer &connectPool);

    void onRemoteHostClosed(const JQNetworkConnectPointer &connect, const JQNetworkConnectPoolPointer &connectPool);

    void onReadyToDelete(const JQNetworkConnectPointer &connect, const JQNetworkConnectPoolPointer &connectPool);

    void onPackageSending(
            const JQNetworkConnectPointer &connect,
            const JQNetworkConnectPoolPointer &connectPool,
            const qint32 &randomFlag,
            const qint64 &payloadCurrentIndex,
            const qint64 &payloadCurrentSize,
            const qint64 &payloadTotalSize
        );

    void onPackageReceiving(
            const JQNetworkConnectPointer &connect,
            const JQNetworkConnectPoolPointer &connectPool,
            const qint32 &randomFlag,
            const qint64 &payloadCurrentIndex,
            const qint64 &payloadCurrentSize,
            const qint64 &payloadTotalSize
        );

    void onPackageReceived(
            const JQNetworkConnectPointer &connect,
            const JQNetworkConnectPoolPointer &connectPool,
            const JQNetworkPackageSharedPointer &package
        );

    void onWaitReplySucceedPackage(
            const JQNetworkConnectPointer &connect,
            const JQNetworkConnectPoolPointer &connectPool,
            const JQNetworkPackageSharedPointer &package,
            const std::function< void(const JQNetworkConnectPointer &connect, const JQNetworkPackageSharedPointer &) > &callback
        );

    void onWaitReplyPackageFail(
            const JQNetworkConnectPointer &connect,
            const JQNetworkConnectPoolPointer &connectPool,
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
