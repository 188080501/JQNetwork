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
{



}

class JQNetworkServer: public QObject
{
    Q_OBJECT

public:
    JQNetworkServer() = default;

    ~JQNetworkServer() = default;

    JQNetworkServer(const JQNetworkServer &) = delete;

    JQNetworkServer &operator =(const JQNetworkServer &) = delete;
};

#include "jqnetwork_server.inc"

#endif//JQNETWORK_INCLUDE_JQNETWORK_SERVER_H
