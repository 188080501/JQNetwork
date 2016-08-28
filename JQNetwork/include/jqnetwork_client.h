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

namespace JQNetwork
{



}

class JQNetworkClient: public QObject
{
    Q_OBJECT

public:
    JQNetworkClient() = default;

    ~JQNetworkClient() = default;

    JQNetworkClient(const JQNetworkClient &) = delete;

    JQNetworkClient &operator =(const JQNetworkClient &) = delete;
};

#include "jqnetwork_client.inc"

#endif//JQNETWORK_INCLUDE_JQNETWORK_CLIENG_H
