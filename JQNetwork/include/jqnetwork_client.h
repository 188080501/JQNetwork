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
{ }

struct JQNetworkClientSettings
{
    int globalSocketThreadCount = 2;
};

class JQNetworkClient: public QObject
{
    Q_OBJECT

public:
    JQNetworkClient(
            const JQNetworkClientSettingsSharedPointer &clientSettings,
            const JQNetworkConnectPoolSettingsSharedPointer connectPoolSettings,
            const JQNetworkConnectSettingsSharedPointer connectSettings
        );

    ~JQNetworkClient() = default;

    JQNetworkClient(const JQNetworkClient &) = delete;

    JQNetworkClient &operator =(const JQNetworkClient &) = delete;

private:
    // Thread pool
    static QWeakPointer< JQNetworkThreadPool > globalSocketThreadPool_;
    QSharedPointer< JQNetworkThreadPool > socketThreadPool_;

    // Settings
    JQNetworkClientSettingsSharedPointer clientSettings_;
    JQNetworkConnectPoolSettingsSharedPointer connectPoolSettings_;
    JQNetworkConnectSettingsSharedPointer connectSettings_;

    // Client
    QMap< QThread *, JQNetworkConnectPoolSharedPointer > connectPools_;
};

#include "jqnetwork_client.inc"

#endif//JQNETWORK_INCLUDE_JQNETWORK_CLIENG_H
