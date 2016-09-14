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

namespace JQNetwork
{ }

struct JQNetworkConnectSettings
{
    bool longConnection = true;
    bool autoMaintainLongConnection = false;

    qint64 cutPackageSize = -1;
    int streamFormat = -1;
    qint64 packageCompressionThresholdForBytes = -1;
    int packageCompressionThresholdForFirstCommunicationElapsed = -1;
    qint64 maximumSendForTotalByteCount = -1;
    qint64 maximumSendPackageByteCount = -1;
    qint64 maximumSendSpeed = -1;
    qint64 maximumReceiveForTotalByteCount = -1;
    qint64 maximumReceivePackageByteCount = -1;
    qint64 maximumReceiveSpeed = -1;

    int maximumConnectToHostWaitTime = 15 * 1000;
    int maximumSendPackageWaitTime = 30 * 1000;
    int maximumReceivePackageWaitTime = 30 * 1000;
    int maximumNoCommunicationTime = 30 * 1000;
    int maximumConnectionTime = -1;

    std::function< void( JQNetworkConnectPointer ) > connectToHostErrorCallback = nullptr;
    std::function< void( JQNetworkConnectPointer ) > connectToHostTimeoutCallback = nullptr;
    std::function< void( JQNetworkConnectPointer ) > connectToHostSucceedCallback = nullptr;
    std::function< void( JQNetworkConnectPointer ) > remoteHostClosedCallback = nullptr;
    std::function< void( JQNetworkConnectPointer ) > readyToDeleteCallback = nullptr;
    std::function< void( JQNetworkConnectPointer, JQNetworkPackageSharedPointer ) > packageReceivedCallback = nullptr;
};

class JQNetworkConnect: public QObject
{
    Q_OBJECT

private:
    JQNetworkConnect();

    JQNetworkConnect(const JQNetworkConnect &) = delete;

    JQNetworkConnect &operator =(const JQNetworkConnect &) = delete;

public:
    ~JQNetworkConnect() = default;

    static void createConnect(
            const std::function< void(const JQNetworkConnectSharedPointer &) > &onConnectCreatedCallback,
            const std::function< void( std::function< void() > ) > runOnConnectThreadCallback,
            const JQNetworkConnectSettingsSharedPointer &connectSettings,
            const QString &hostName,
            const quint16 &port
        );

    static void createConnect(
            const std::function< void(const JQNetworkConnectSharedPointer &) > &onConnectCreatedCallback,
            const std::function< void( std::function< void() > ) > runOnConnectThreadCallback,
            const JQNetworkConnectSettingsSharedPointer &connectSettings,
            const qintptr &socketDescriptor
        );

    inline bool isAbandonTcpSocket() const;

    qint32 sendPayloadData(const QByteArray &payloadData);

private Q_SLOTS:
    void onTcpSocketStateChanged();

    void onTcpSocketConnectToHostTimeOut();

    void onTcpSocketBytesWritten(const qint64 &bytes);

    void onTcpSocketReadyRead();

    void onReadyToDelete();

    void sendPayloadData(const QByteArray &payloadData, const qint32 &randomFlag);

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

    // Package
    QMutex mutexForSend_;
    qint32 sendRotaryIndex_ = 0;
    QMap< qint32, JQNetworkPackageSharedPointer > sendPackagePool_; // randomFlag -> package
    QMap< qint32, JQNetworkPackageSharedPointer > receivePackagePool_; // randomFlag -> package
};

#include "jqnetwork_connect.inc"

#endif//JQNETWORK_INCLUDE_JQNETWORK_CONNECT_H
