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
    //...
};

class JQNetworkServer: public QObject
{
    Q_OBJECT

public:
    JQNetworkServer(const QSharedPointer< JQNetworkServerSettings > serverSettings);

    ~JQNetworkServer();

    JQNetworkServer(const JQNetworkServer &) = delete;

    JQNetworkServer &operator =(const JQNetworkServer &) = delete;

private:
    // Settings
    QSharedPointer< JQNetworkServerSettings > serverSettings_;

    // Server
    QSharedPointer< QTcpServer > tcpServer_;
};

#include "jqnetwork_server.inc"

#endif//JQNETWORK_INCLUDE_JQNETWORK_SERVER_H
