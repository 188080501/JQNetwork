/*
    This file is part of JQNetwork
    
    Library introduce: https://github.com/188080501/JQNetwork

    Copyright: Jason

    Contact email: Jason@JasonServer.com
    
    GitHub: https://github.com/188080501/
*/

#include "jqnetwork_connectpool.h"

// Qt lib import
#include <QDebug>
#include <QTimer>

// JQNetwork lib import
#include <JQNetworkConnect>

using namespace JQNetwork;

JQNetworkConnectPool::JQNetworkConnectPool(
        QSharedPointer< JQNetworkConnectPoolSettings > connectPoolSettings,
        QSharedPointer< JQNetworkConnectSettings > connectSettings
    ):
    connectPoolSettings_( connectPoolSettings ),
    connectSettings_( connectSettings )
{
    connectSettings_->connectToHostErrorCallback   = [ this ](const auto &connect){ this->onConnectToHostError( connect ); };
    connectSettings_->connectToHostTimeoutCallback = [ this ](const auto &connect){ this->onConnectToHostTimeout( connect ); };
    connectSettings_->connectToHostSucceedCallback = [ this ](const auto &connect){ this->onConnectToHostSucceed( connect ); };
    connectSettings_->remoteHostClosedCallback     = [ this ](const auto &connect){ this->onRemoteHostClosed( connect ); };
    connectSettings_->readyToDeleteCallback        = [ this ](const auto &connect){ this->onReadyToDelete( connect ); };
}

void JQNetworkConnectPool::createConnectByHostAndPort(const QString &hostName, const quint16 &port)
{
    JQNetworkConnect::createConnectByHostAndPort(
                [ this ](const auto &connect){ this->connectForConnecting_[ connect.data() ] = connect; },
                connectSettings_,
                hostName,
                port
            );
}

void JQNetworkConnectPool::createConnectBySocketDescriptor(const qintptr &socketDescriptor)
{
    JQNetworkConnect::createConnectBySocketDescriptor(
                [ this ](const auto &connect){ this->connectForConnecting_[ connect.data() ] = connect; },
                connectSettings_,
                socketDescriptor
            );
}

void JQNetworkConnectPool::onConnectToHostError(const QPointer< JQNetworkConnect > &connect)
{
    qDebug() << __func__ << connect.data();

    NULLPTR_CHECK( connectPoolSettings_->connectToHostErrorCallback );
    connectPoolSettings_->connectToHostErrorCallback( connect );
}

void JQNetworkConnectPool::onConnectToHostTimeout(const QPointer< JQNetworkConnect > &connect)
{
    qDebug() << __func__ << connect.data();

    NULLPTR_CHECK( connectPoolSettings_->connectToHostTimeoutCallback );
    connectPoolSettings_->connectToHostTimeoutCallback( connect );
}

void JQNetworkConnectPool::onConnectToHostSucceed(const QPointer< JQNetworkConnect > &connect)
{
    qDebug() << __func__ << connect.data();

    auto containsInConnecting = connectForConnecting_.contains( connect.data() );

    if ( !containsInConnecting )
    {
        qDebug() << __func__ << ": error: connect not contains" << connect.data();
        return;
    }

    connectForConnected_[ connect.data() ] = connectForConnecting_[ connect.data() ];
    connectForConnecting_.remove( connect.data() );

    NULLPTR_CHECK( connectPoolSettings_->connectToHostSucceedCallback );
    connectPoolSettings_->connectToHostSucceedCallback( connect );
}

void JQNetworkConnectPool::onRemoteHostClosed(const QPointer< JQNetworkConnect > &connect)
{
    qDebug() << __func__ << connect.data();

    NULLPTR_CHECK( connectPoolSettings_->remoteHostClosedCallback );
    connectPoolSettings_->remoteHostClosedCallback( connect );
}

void JQNetworkConnectPool::onReadyToDelete(const QPointer< JQNetworkConnect > &connect)
{
    qDebug() << __func__ << connect.data();

    auto containsInConnecting = connectForConnecting_.contains( connect.data() );
    auto containsInConnected = connectForConnected_.contains( connect.data() );

    if ( !containsInConnecting && !containsInConnected )
    {
        qDebug() << __func__ << ": error: connect not contains" << connect.data();
        return;
    }

    if ( containsInConnecting )
    {
        QTimer::singleShot( 0, [ connect = connectForConnecting_[ connect.data() ] ](){} );
        connectForConnecting_.remove( connect.data() );
    }

    if ( containsInConnected )
    {
        QTimer::singleShot( 0, [ connect = connectForConnected_[ connect.data() ] ](){} );
        connectForConnected_.remove( connect.data() );
    }

    NULLPTR_CHECK( connectPoolSettings_->readyToDeleteCallback );
    connectPoolSettings_->readyToDeleteCallback( connect );
}
