/*
    This file is part of JQNetwork
    
    Library introduce: https://github.com/188080501/JQNetwork

    Copyright: Jason

    Contact email: Jason@JasonServer.com
    
    GitHub: https://github.com/188080501/
*/

#include "jqnetwork_client.h"

// Qt lib import
#include <QDebug>
#include <QThread>

// JQNetwork lib import
#include <JQNetworkConnectPool>
#include <JQNetworkConnect>
#include <JQNetworkPackage>

using namespace JQNetwork;
QWeakPointer< JQNetworkThreadPool > JQNetworkClient::globalSocketThreadPool_;
QWeakPointer< JQNetworkThreadPool > JQNetworkClient::globalProcessorThreadPool_;

JQNetworkClient::JQNetworkClient(
        const JQNetworkClientSettingsSharedPointer &clientSettings,
        const JQNetworkConnectPoolSettingsSharedPointer connectPoolSettings,
        const JQNetworkConnectSettingsSharedPointer connectSettings
    ):
    clientSettings_( clientSettings ),
    connectPoolSettings_( connectPoolSettings ),
    connectSettings_( connectSettings )
{
    if ( globalSocketThreadPool_ )
    {
        socketThreadPool_ = globalSocketThreadPool_.toStrongRef();
    }
    else
    {
        socketThreadPool_ = QSharedPointer< JQNetworkThreadPool >( new JQNetworkThreadPool( clientSettings_->globalSocketThreadCount ) );
        globalSocketThreadPool_ = socketThreadPool_.toWeakRef();
    }

    if ( globalProcessorThreadPool_ )
    {
        processorThreadPool_ = globalProcessorThreadPool_.toStrongRef();
    }
    else
    {
        processorThreadPool_ = QSharedPointer< JQNetworkThreadPool >( new JQNetworkThreadPool( clientSettings->globalProcessorThreadCount ) );
        globalProcessorThreadPool_ = processorThreadPool_.toWeakRef();
    }
}

JQNetworkClientSharedPointer JQNetworkClient::createClient()
{
    JQNetworkClientSettingsSharedPointer clientSettings( new JQNetworkClientSettings );
    JQNetworkConnectPoolSettingsSharedPointer connectPoolSettings( new JQNetworkConnectPoolSettings );
    JQNetworkConnectSettingsSharedPointer connectSettings( new JQNetworkConnectSettings );

    return JQNetworkClientSharedPointer( new JQNetworkClient( clientSettings, connectPoolSettings, connectSettings ) );
}

bool JQNetworkClient::begin()
{
    socketThreadPool_->waitRunEach(
                [
                    this
                ]()
                {
                    JQNetworkConnectPoolSettingsSharedPointer connectPoolSettings( new JQNetworkConnectPoolSettings( *this->connectPoolSettings_ ) );
                    JQNetworkConnectSettingsSharedPointer connectSettings( new JQNetworkConnectSettings( *this->connectSettings_ ) );

                    connectPoolSettings->connectToHostErrorCallback   = [ this ](const auto &connect){ this->onConnectToHostError( connect ); };
                    connectPoolSettings->connectToHostTimeoutCallback = [ this ](const auto &connect){ this->onConnectToHostTimeout( connect ); };
                    connectPoolSettings->connectToHostSucceedCallback = [ this ](const auto &connect){ this->onConnectToHostSucceed( connect ); };
                    connectPoolSettings->remoteHostClosedCallback     = [ this ](const auto &connect){ this->onRemoteHostClosed( connect ); };
                    connectPoolSettings->readyToDeleteCallback        = [ this ](const auto &connect){ this->onReadyToDelete( connect ); };
                    connectPoolSettings->packageReceivedCallback    = [ this ](const auto &connect, const auto &package){ this->onPackageReceived( connect, package ); };

                    connectPools_[ QThread::currentThread() ] = JQNetworkConnectPoolSharedPointer(
                                new JQNetworkConnectPool(
                                    connectPoolSettings,
                                    connectSettings
                                )
                            );
                }
    );

    return true;
}

void JQNetworkClient::createConnect(const QString &hostName, const quint16 &port)
{
    const auto &&rotaryIndex = socketThreadPool_->nextRotaryIndex();
    socketThreadPool_->run(
                [
                    this,
                    runOnConnectThreadCallback =
                        [
                            this,
                            rotaryIndex
                        ](const std::function< void() > &callback)
                        {
                            this->socketThreadPool_->run( callback, rotaryIndex );
                        },
                    hostName,
                    port
                ]()
                {
                    this->connectPools_[ QThread::currentThread() ]->createConnect(
                        runOnConnectThreadCallback,
                        hostName,
                        port
                    );
                },
                rotaryIndex
    );
}

int JQNetworkClient::sendPayloadData(
        const QString &hostName,
        const quint16 &port,
        const QByteArray &payloadData,
        const JQNetworkClientSendCallbackPackage &callbackPackagea
    )
{
    for ( const auto &connectPool: this->connectPools_ )
    {
        auto connect = connectPool->getConnectByHostAndPort( hostName, port );

        if ( !connect ) { continue; }

        auto randomFlag = connect->sendPayloadData( payloadData );

        if ( !randomFlag ) { return randomFlag; }

        sendCallbackPackages_[ randomFlag ] = callbackPackagea;

        return randomFlag;
    }

    return 0;
}

void JQNetworkClient::onConnectToHostSucceed(const JQNetworkConnectPointer &connect)
{
    if ( !clientSettings_->connectToHostSucceedCallback ) { return; }

    processorThreadPool_->run(
                [
                    this,
                    connect
                ]()
                {
                    for ( const auto &connectPool: this->connectPools_ )
                    {
                        auto reply = connectPool->getHostAndPortByConnect( connect );

                        if ( reply.first.isEmpty() || !reply.second ) { continue; }

                        this->clientSettings_->connectToHostSucceedCallback( connect, reply.first, reply.second );

                        break;
                    }
                }
    );
}

void JQNetworkClient::onPackageReceived(const JQNetworkConnectPointer &connect, const JQNetworkPackageSharedPointer &package)
{
    qDebug() << __func__ << package->randomFlag() << connect.data() << QThread::currentThread();
}
