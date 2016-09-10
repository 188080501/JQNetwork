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

inline int JQNetworkThreadPool::nextRotaryIndex()
{
    rotaryIndex_ = ( rotaryIndex_ + 1 ) % helpers_->size();
    return rotaryIndex_;
}

inline void JQNetworkThreadPool::runEach(const std::function<void ()> &callback)
{
    for ( auto index = 0; index < helpers_->size(); ++index )
    {
        ( *helpers_ )[ index ]->run( callback );
    }
}

inline void JQNetworkThreadPool::waitRunEach(const std::function<void ()> &callback)
{
    for ( auto index = 0; index < helpers_->size(); ++index )
    {
        this->waitRun( callback, index );
    }
}

#endif//JQNETWORK_INCLUDE_JQNETWORK_FOUNDATION_INC
