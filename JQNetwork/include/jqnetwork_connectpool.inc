/*
    This file is part of JQNetwork
    
    Library introduce: https://github.com/188080501/JQNetwork

    Copyright: Jason

    Contact email: Jason@JasonServer.com
    
    GitHub: https://github.com/188080501/
*/

#ifndef JQNETWORK_INCLUDE_JQNETWORK_CONNECTPOOL_INC
#define JQNETWORK_INCLUDE_JQNETWORK_CONNECTPOOL_INC

// JQNetwork lib import
#include "jqnetwork_connectpool.h"

inline void JQNetworkConnectPool::onConnectToHostError(const JQNetworkConnectPointer &connect)
{
    NULLPTR_CHECK( connectPoolSettings_->connectToHostErrorCallback );
    connectPoolSettings_->connectToHostErrorCallback( connect );
}

inline void JQNetworkConnectPool::onConnectToHostTimeout(const JQNetworkConnectPointer &connect)
{
    NULLPTR_CHECK( connectPoolSettings_->connectToHostTimeoutCallback );
    connectPoolSettings_->connectToHostTimeoutCallback( connect );
}

inline void JQNetworkConnectPool::onRemoteHostClosed(const JQNetworkConnectPointer &connect)
{
    NULLPTR_CHECK( connectPoolSettings_->remoteHostClosedCallback );
    connectPoolSettings_->remoteHostClosedCallback( connect );
}

#endif//JQNETWORK_INCLUDE_JQNETWORK_CONNECTPOOL_INC
