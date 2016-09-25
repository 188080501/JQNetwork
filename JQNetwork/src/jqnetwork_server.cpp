﻿/*
    This file is part of JQNetwork
    
    Library introduce: https://github.com/188080501/JQNetwork

    Copyright: Jason

    Contact email: Jason@JasonServer.com
    
    GitHub: https://github.com/188080501/
*/

#include "jqnetwork_server.h"

// Qt lib import
#include <QDebug>
#include <QTcpServer>
#include <QThread>

// JQNetwork lib import
#include <JQNetworkConnectPool>
#include <JQNetworkConnect>

// JQNetworkServerHelper
class JQNetworkServerHelper: public QTcpServer
{
public:
    JQNetworkServerHelper(const std::function< void(qintptr socketDescriptor) > &onIncomingConnectionCallback):
        onIncomingConnectionCallback_( onIncomingConnectionCallback )
    { }

    ~JQNetworkServerHelper() = default;

private:
    inline void incomingConnection(qintptr socketDescriptor)
    {
        onIncomingConnectionCallback_( socketDescriptor );
    }

private:
    std::function< void(qintptr socketDescriptor) > onIncomingConnectionCallback_;
};

// JQNetworkServer
QWeakPointer< JQNetworkThreadPool > JQNetworkServer::globalServerThreadPool_;
QWeakPointer< JQNetworkThreadPool > JQNetworkServer::globalSocketThreadPool_;
QWeakPointer< JQNetworkThreadPool > JQNetworkServer::globalProcessorThreadPool_;

JQNetworkServer::JQNetworkServer(
        const JQNetworkServerSettingsSharedPointer serverSettings,
        const JQNetworkConnectPoolSettingsSharedPointer connectPoolSettings,
        const JQNetworkConnectSettingsSharedPointer connectSettings
    ):
    serverSettings_( serverSettings ),
    connectPoolSettings_( connectPoolSettings ),
    connectSettings_( connectSettings )
{
    if ( globalServerThreadPool_ )
    {
        serverThreadPool_ = globalServerThreadPool_.toStrongRef();
    }
    else
    {
        serverThreadPool_ = QSharedPointer< JQNetworkThreadPool >( new JQNetworkThreadPool( serverSettings->globalServerThreadCount ) );
        globalServerThreadPool_ = serverThreadPool_.toWeakRef();
    }

    if ( globalSocketThreadPool_ )
    {
        socketThreadPool_ = globalSocketThreadPool_.toStrongRef();
    }
    else
    {
        socketThreadPool_ = QSharedPointer< JQNetworkThreadPool >( new JQNetworkThreadPool( serverSettings->globalSocketThreadCount ) );
        globalSocketThreadPool_ = socketThreadPool_.toWeakRef();
    }

    if ( globalProcessorThreadPool_ )
    {
        processorThreadPool_ = globalProcessorThreadPool_.toStrongRef();
    }
    else
    {
        processorThreadPool_ = QSharedPointer< JQNetworkThreadPool >( new JQNetworkThreadPool( serverSettings->globalProcessorThreadCount ) );
        globalProcessorThreadPool_ = processorThreadPool_.toWeakRef();
    }
}

JQNetworkServer::~JQNetworkServer()
{
    if ( !this->tcpServer_ ) { return; }

    serverThreadPool_->waitRun(
                [
                    this
                ]()
                {
                    tcpServer_->close();
                    tcpServer_.clear();
                }
    );

    socketThreadPool_->waitRunEach(
                [ & ]()
                {
                    connectPools_[ QThread::currentThread() ].clear();
                }
    );
}

JQNetworkServerSharedPointer JQNetworkServer::createServerByListenPort(
        const quint16 &listenPort,
        const QHostAddress &listenAddress
    )
{
    JQNetworkServerSettingsSharedPointer serverSettings( new JQNetworkServerSettings );
    JQNetworkConnectPoolSettingsSharedPointer connectPoolSettings( new JQNetworkConnectPoolSettings );
    JQNetworkConnectSettingsSharedPointer connectSettings( new JQNetworkConnectSettings );

    serverSettings->listenAddress = listenAddress;
    serverSettings->listenPort = listenPort;

    return JQNetworkServerSharedPointer( new JQNetworkServer( serverSettings, connectPoolSettings, connectSettings ) );
}

bool JQNetworkServer::begin()
{
    bool listenSucceed = false;

    serverThreadPool_->waitRun(
                [
                    this,
                    &listenSucceed
                ]()
                {
                    this->tcpServer_ = QSharedPointer< QTcpServer >( new JQNetworkServerHelper( [ this ]( auto socketDescriptor )
                        { this->incomingConnection( socketDescriptor ); } ) );

                    listenSucceed = this->tcpServer_->listen(
                                this->serverSettings_->listenAddress,
                                this->serverSettings_->listenPort
                            );
                }
    );

    if ( !listenSucceed ) { return false; }

    socketThreadPool_->waitRunEach(
                [
                    this
                ]()
                {
                    JQNetworkConnectPoolSettingsSharedPointer connectPoolSettings( new JQNetworkConnectPoolSettings( *this->connectPoolSettings_ ) );
                    JQNetworkConnectSettingsSharedPointer connectSettings( new JQNetworkConnectSettings( *this->connectSettings_ ) );

                    connectPoolSettings->connectToHostErrorCallback   = [ this ](const auto &connect)
                        { this->onConnectToHostError( connect ); };
                    connectPoolSettings->connectToHostTimeoutCallback = [ this ](const auto &connect)
                        { this->onConnectToHostTimeout( connect ); };
                    connectPoolSettings->connectToHostSucceedCallback = [ this ](const auto &connect)
                        { this->onConnectToHostSucceed( connect ); };
                    connectPoolSettings->remoteHostClosedCallback     = [ this ](const auto &connect)
                        { this->onRemoteHostClosed( connect ); };
                    connectPoolSettings->readyToDeleteCallback        = [ this ](const auto &connect)
                        { this->onReadyToDelete( connect ); };
                    connectPoolSettings->packageReceivedCallback      = [ this ](const auto &connect, const auto &package)
                        { this->onPackageReceived( connect, package ); };

                    connectSettings->randomFlagRangeStart = 1000000000;
                    connectSettings->randomFlagRangeEnd = 1999999999;

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

void JQNetworkServer::incomingConnection(const qintptr &socketDescriptor)
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
                    socketDescriptor
                ]()
                {
                    this->connectPools_[ QThread::currentThread() ]->createConnect(
                        runOnConnectThreadCallback,
                        socketDescriptor
                    );
                },
                rotaryIndex
    );
}

void JQNetworkServer::onPackageReceived(const JQNetworkConnectPointer &connect, const JQNetworkPackageSharedPointer &package)
{
    NULLPTR_CHECK( serverSettings_->packageReceivedCallback );

    processorThreadPool_->run(
                [
                    connect,
                    package,
                    callback = serverSettings_->packageReceivedCallback
                ]()
                {
                    callback( connect, package );
                }
    );
}
