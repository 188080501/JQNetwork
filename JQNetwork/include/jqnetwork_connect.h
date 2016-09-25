/*
    This file is part of JQNetwork
    
    Library introduce: https://github.com/188080501/JQNetwork

    Copyright: Jason

    Contact email: Jason@JasonServer.com
    
    GitHub: https://github.com/188080501/
*/

#ifndef JQNETWORK_INCLUDE_JQNETWORK_CONNECT_H
#define JQNETWORK_INCLUDE_JQNETWORK_CONNECT_H

// JQNetwork lib import
#include <JQNetworkFoundation>

struct JQNetworkConnectSettings
{
    bool longConnection = true;
    bool autoMaintainLongConnection = false;

    int streamFormat = -1;
    qint64 cutPackageSize = 2 * 1024 * 1024;
    qint64 packageCompressionThresholdForBytes = -1;
    int packageCompressionThresholdForFirstCommunicationElapsed = -1;
    qint64 maximumSendForTotalByteCount = -1;
    qint64 maximumSendPackageByteCount = -1;
    int maximumSendSpeed = -1; // Byte/s
    qint64 maximumReceiveForTotalByteCount = -1;
    qint64 maximumReceivePackageByteCount = -1;
    int maximumReceiveSpeed = -1; // Byte/s

    qint32 randomFlagRangeStart = -1;
    qint32 randomFlagRangeEnd = -1;

    int maximumConnectToHostWaitTime = 15 * 1000;
    int maximumSendPackageWaitTime = 30 * 1000;
    int maximumReceivePackageWaitTime = 30 * 1000;
    int maximumNoCommunicationTime = 30 * 1000;
    int maximumConnectionTime = -1;

    std::function< void( const JQNetworkConnectPointer & ) > connectToHostErrorCallback = nullptr;
    std::function< void( const JQNetworkConnectPointer & ) > connectToHostTimeoutCallback = nullptr;
    std::function< void( const JQNetworkConnectPointer & ) > connectToHostSucceedCallback = nullptr;
    std::function< void( const JQNetworkConnectPointer & ) > remoteHostClosedCallback = nullptr;
    std::function< void( const JQNetworkConnectPointer & ) > readyToDeleteCallback = nullptr;
    std::function< void( const JQNetworkConnectPointer &, const JQNetworkPackageSharedPointer & ) > packageReceivedCallback = nullptr;
    std::function< void( const JQNetworkConnectPointer &, const JQNetworkPackageSharedPointer &, const std::function< void(const JQNetworkConnectPointer &connect, const JQNetworkPackageSharedPointer &) > & ) > waitReplyPackageSucceedCallback = nullptr;
    std::function< void( const JQNetworkConnectPointer &, const std::function< void(const JQNetworkConnectPointer &connect) > & ) > waitReplyPackageFailCallback = nullptr;
};

class JQNetworkConnect: public QObject
{
    Q_OBJECT

private:
    struct ReceivedCallbackPackage
    {
        qint64 sendTime;
        std::function< void(const JQNetworkConnectPointer &connect, const JQNetworkPackageSharedPointer &) > succeedCallback;
        std::function< void(const JQNetworkConnectPointer &connect) > failCallback;
    };

private:
    JQNetworkConnect();

    JQNetworkConnect(const JQNetworkConnect &) = delete;

    JQNetworkConnect &operator =(const JQNetworkConnect &) = delete;

public:
    ~JQNetworkConnect() = default;

    static void createConnect(
            const std::function< void(const JQNetworkConnectSharedPointer &) > &onConnectCreatedCallback,
            const std::function< void( std::function< void() > ) > &runOnConnectThreadCallback,
            const JQNetworkConnectSettingsSharedPointer &connectSettings,
            const QString &hostName,
            const quint16 &port
        );

    static void createConnect(
            const std::function< void(const JQNetworkConnectSharedPointer &) > &onConnectCreatedCallback,
            const std::function< void( std::function< void() > ) > &runOnConnectThreadCallback,
            const JQNetworkConnectSettingsSharedPointer &connectSettings,
            const qintptr &socketDescriptor
        );

    inline bool isAbandonTcpSocket() const;

    void close();

    qint32 sendPayloadData(
            const QByteArray &payloadData,
            const std::function< void(const JQNetworkConnectPointer &connect, const JQNetworkPackageSharedPointer &) > &succeedCallback = nullptr,
            const std::function< void(const JQNetworkConnectPointer &connect) > &failCallback = nullptr
        );

    qint32 replyPayloadData(
            const QByteArray &payloadData,
            const qint32 &randomFlag
        );

private Q_SLOTS:
    void onTcpSocketStateChanged();

    void onTcpSocketBytesWritten(const qint64 &bytes);

    void onTcpSocketReadyRead();

    void onTcpSocketConnectToHostTimeOut();

    void onSendPackageCheck();

private:
    void startTimerForConnectToHostTimeOut();

    void startTimerForSendPackageCheck();

    void onPackageReceived(const JQNetworkPackageSharedPointer &package);

    void onReadyToDelete();

    void reaySendPayloadData(
            const QByteArray &payloadData,
            const qint32 &randomFlag,
            const std::function< void(const JQNetworkConnectPointer &connect, const JQNetworkPackageSharedPointer &) > &succeedCallback,
            const std::function< void(const JQNetworkConnectPointer &connect) > &failCallback
        );

private:
    // Settings
    JQNetworkConnectSettingsSharedPointer connectSettings_;
    std::function< void( std::function< void() > ) > runOnConnectThreadCallback_;

    // Socket
    QSharedPointer< QTcpSocket > tcpSocket_;
    bool onceConnectSucceed_ = false;
    bool isAbandonTcpSocket_ = false;
    QByteArray tcpSocketBuffer_;

    // Timer
    QSharedPointer< QTimer > timerForConnectToHostTimeOut_;
    QSharedPointer< QTimer > timerForSendPackageCheck_;

    // Package
    QMutex mutexForSend_;
    qint32 sendRandomFlagRotaryIndex_ = 0;
    QMap< qint32, JQNetworkPackageSharedPointer > sendPackagePool_; // randomFlag -> package
    QMap< qint32, JQNetworkPackageSharedPointer > receivePackagePool_; // randomFlag -> package
    QMap< qint32, ReceivedCallbackPackage > onReceivedCallbacks_; // randomFlag -> package

    // Statistics
    qint64 waitForSendBytes_ = 0;
    qint64 alreadyWrittenBytes_ = 0;
};

#include "jqnetwork_connect.inc"

#endif//JQNETWORK_INCLUDE_JQNETWORK_CONNECT_H
