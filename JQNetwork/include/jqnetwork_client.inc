/*
    This file is part of JQNetwork
    
    Library introduce: https://github.com/188080501/JQNetwork

    Copyright: Jason

    Contact email: Jason@JasonServer.com
    
    GitHub: https://github.com/188080501/
*/

#ifndef JQNETWORK_INCLUDE_JQNETWORK_CLIENG_INC
#define JQNETWORK_INCLUDE_JQNETWORK_CLIENG_INC

// JQNetwork lib import
#include "jqnetwork_client.h"

inline JQNetworkClientSettingsSharedPointer JQNetworkClient::clientSettings()
{
    return clientSettings_;
}

inline JQNetworkConnectPoolSettingsSharedPointer JQNetworkClient::connectPoolSettings()
{
    return connectPoolSettings_;
}

inline JQNetworkConnectSettingsSharedPointer JQNetworkClient::connectSettings()
{
    return connectSettings_;
}

inline void JQNetworkClient::setOnConnectToHostSucceedCallback(
        const std::function< void( JQNetworkConnectPointer, const QString &hostName, const quint16 &port ) > &callback
    )
{
    if ( !connectPools_.isEmpty() )
    {
        qDebug() << __func__ << "set error: already begin";
        return;
    }

    clientSettings_->connectToHostSucceedCallback = callback;
}

#endif//JQNETWORK_INCLUDE_JQNETWORK_CLIENG_INC
