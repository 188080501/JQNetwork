/*
    This file is part of JQNetwork
    
    Library introduce: https://github.com/188080501/JQNetwork

    Copyright: Jason

    Contact email: Jason@JasonServer.com
    
    GitHub: https://github.com/188080501/
*/

#ifndef JQNETWORK_INCLUDE_JQNETWORK_LAN_H
#define JQNETWORK_INCLUDE_JQNETWORK_LAN_H

// JQNetwork lib import
#include <JQNetworkFoundation>

class JQNetworkLan: public QObject
{
    Q_OBJECT

public:
    JQNetworkLan() = default;

    ~JQNetworkLan() = default;

    JQNetworkLan(const JQNetworkLan &) = delete;

    JQNetworkLan &operator =(const JQNetworkLan &) = delete;
};

#include "jqnetwork_lan.inc"

#endif//JQNETWORK_INCLUDE_JQNETWORK_LAN_H
