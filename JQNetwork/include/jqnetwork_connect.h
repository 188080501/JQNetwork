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

    std::function< void( QPointer< JQNetworkConnect > ) > connectToHostErrorCallback = nullptr;
    std::function< void( QPointer< JQNetworkConnect > ) > connectToHostTimeoutCallback = nullptr;
    std::function< void( QPointer< JQNetworkConnect > ) > connectToHostSucceedCallback = nullptr;
    std::function< void( QPointer< JQNetworkConnect > ) > remoteHostClosedCallback = nullptr;
    std::function< void( QPointer< JQNetworkConnect > ) > readyToDeleteCallback = nullptr;
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

private Q_SLOTS:
    void onTcpSocketStateChanged();

    void onTcpSocketConnectToHostTimeOut();

    void onTcpSocketBytesWritten(const qint64 &bytes);

    void onTcpSocketReadyRead();

    void onReadyToDelete();

public:
    static QSharedPointer< JQNetworkConnect > createConnectByHostAndPort(
            const QSharedPointer< JQNetworkConnectSettings > &connectSettings,
            const QString &hostName,
            const quint16 &port
        );

private:
    // Settings
    QSharedPointer< JQNetworkConnectSettings > connectSettings_;

    // Socket
    QSharedPointer< QTcpSocket > tcpSocket_;
    bool onceConnectSucceed_ = false;
    bool abandonTcpSocket = false;

    // Timer
    QSharedPointer< QTimer > timerForConnectToHostTimeOut_;
};

#include "jqnetwork_connect.inc"

#endif//JQNETWORK_INCLUDE_JQNETWORK_CONNECT_H
