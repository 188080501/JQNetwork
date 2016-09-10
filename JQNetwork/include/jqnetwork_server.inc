/*
    This file is part of JQNetwork
    
    Library introduce: https://github.com/188080501/JQNetwork

    Copyright: Jason

    Contact email: Jason@JasonServer.com
    
    GitHub: https://github.com/188080501/
*/

#ifndef JQNETWORK_INCLUDE_JQNETWORK_SERVER_INC
#define JQNETWORK_INCLUDE_JQNETWORK_SERVER_INC

// JQNetwork lib import
#include "jqnetwork_server.h"

inline void JQNetworkServer::onConnectToHostError(const JQNetworkConnectPointer &connect)
{
    if ( !serverSettings_->connectToHostErrorCallback ) { return; }

    processorThreadPool_->run(
                [
                    connect,
                    callback = serverSettings_->connectToHostErrorCallback
                ]()
                {
                    callback( connect );
                }
    );
}

inline void JQNetworkServer::onConnectToHostTimeout(const JQNetworkConnectPointer &connect)
{
    if ( !serverSettings_->connectToHostTimeoutCallback ) { return; }

    processorThreadPool_->run(
                [
                    connect,
                    callback = serverSettings_->connectToHostTimeoutCallback
                ]()
                {
                    callback( connect );
                }
    );
}

inline void JQNetworkServer::onConnectToHostSucceed(const JQNetworkConnectPointer &connect)
{
    if ( !serverSettings_->connectToHostSucceedCallback ) { return; }

    processorThreadPool_->run(
                [
                    connect,
                    callback = serverSettings_->connectToHostSucceedCallback
                ]()
                {
                    callback( connect );
                }
    );
}

inline void JQNetworkServer::onRemoteHostClosed(const JQNetworkConnectPointer &connect)
{
    if ( !serverSettings_->remoteHostClosedCallback ) { return; }

    processorThreadPool_->run(
                [
                    connect,
                    callback = serverSettings_->remoteHostClosedCallback
                ]()
                {
                    callback( connect );
                }
    );
}

inline void JQNetworkServer::onReadyToDelete(const JQNetworkConnectPointer &connect)
{
    if ( !serverSettings_->readyToDeleteCallback ) { return; }

    processorThreadPool_->run(
                [
                    connect,
                    callback = serverSettings_->readyToDeleteCallback
                ]()
                {
                    callback( connect );
                }
    );
}

inline void JQNetworkServer::onPackageReceivedCallback(const JQNetworkConnectPointer &connect, const JQNetworkPackageSharedPointer &package)
{
    NULLPTR_CHECK( serverSettings_->onPackageReceivedCallback );

    processorThreadPool_->run(
                [
                    connect,
                    package,
                    callback = serverSettings_->onPackageReceivedCallback
                ]()
                {
                    callback( connect, package );
                }
    );
}

#endif//JQNETWORK_INCLUDE_JQNETWORK_SERVER_INC
