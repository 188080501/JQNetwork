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
    void onTcpSocketReadyRead();

    void onTcpSocketError();

public:
    static QSharedPointer< JQNetworkConnect > createConnect();

signals:
    void connectToHostSucceed();

private:
    QSharedPointer< QTcpSocket > tcpSocket_;
};

#include "jqnetwork_connect.inc"

#endif//JQNETWORK_INCLUDE_JQNETWORK_CONNECT_H
