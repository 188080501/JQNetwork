/*
    This file is part of JQNetwork
    
    Library introduce: https://github.com/188080501/JQNetwork

    Copyright: Jason

    Contact email: Jason@JasonServer.com
    
    GitHub: https://github.com/188080501/
*/

#ifndef JQNETWORK_INCLUDE_JQNETWORK_FOUNDATION_INC
#define JQNETWORK_INCLUDE_JQNETWORK_FOUNDATION_INC

// JQNetwork lib import
#include "jqnetwork_foundation.h"

// JQNetworkThreadPoolHelper
inline void JQNetworkThreadPoolHelper::run(const std::function< void() > &callback)
{
    callback();
}

#endif//JQNETWORK_INCLUDE_JQNETWORK_FOUNDATION_INC
