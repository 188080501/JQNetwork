/*
    This file is part of JQNetwork
    
    Library introduce: https://github.com/188080501/JQNetwork

    Copyright: Jason

    Contact email: Jason@JasonServer.com
    
    GitHub: https://github.com/188080501/
*/

#ifndef JQNETWORK_INCLUDE_JQNETWORK_CONNECTPOOL_H
#define JQNETWORK_INCLUDE_JQNETWORK_CONNECTPOOL_H

// JQNetwork lib import
#include <JQNetworkFoundation>

namespace JQNetwork
{ }

class JQNetworkConnectPool: public QObject
{
    Q_OBJECT

public:
    JQNetworkConnectPool() = default;

    ~JQNetworkConnectPool() = default;

    JQNetworkConnectPool(const JQNetworkConnectPool &) = delete;

    JQNetworkConnectPool &operator =(const JQNetworkConnectPool &) = delete;
};

#include "jqnetwork_connectpool.inc"

#endif//JQNETWORK_INCLUDE_JQNETWORK_CONNECTPOOL_H
