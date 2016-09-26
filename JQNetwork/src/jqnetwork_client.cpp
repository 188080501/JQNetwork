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
#include <QSemaphore>

// JQNetwork lib import
#include <JQNetworkConnectPool>
#include <JQNetworkConnect>
#include <JQNetworkPackage>

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

JQNetworkClient::~JQNetworkClient()
{
    socketThreadPool_->waitRunEach(
                [ & ]()
                {
                    connectPools_[ QThread::currentThread() ].clear();
                }
    );
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
                    JQNetworkConnectPoolSettingsSharedPointer connectPoolSettings(
                                new JQNetworkConnectPoolSettings( *this->connectPoolSettings_ )
                            );
                    JQNetworkConnectSettingsSharedPointer connectSettings(
                                new JQNetworkConnectSettings( *this->connectSettings_ )
                            );

                    connectPoolSettings->connectToHostErrorCallback         = [ this ](const auto &connect, const auto &connectPool)
                        { this->onConnectToHostError( connect, connectPool ); };
                    connectPoolSettings->connectToHostTimeoutCallback       = [ this ](const auto &connect, const auto &connectPool)
                        { this->onConnectToHostTimeout( connect, connectPool ); };
                    connectPoolSettings->connectToHostSucceedCallback       = [ this ](const auto &connect, const auto &connectPool)
                        { this->onConnectToHostSucceed( connect, connectPool ); };
                    connectPoolSettings->remoteHostClosedCallback           = [ this ](const auto &connect, const auto &connectPool)
                        { this->onRemoteHostClosed( connect, connectPool ); };
                    connectPoolSettings->readyToDeleteCallback              = [ this ](const auto &connect, const auto &connectPool)
                        { this->onReadyToDelete( connect, connectPool ); };
                    connectPoolSettings->packageReceivedCallback            = [ this ](const auto &connect, const auto &connectPool, const auto &package)
                        { this->onPackageReceived( connect, connectPool, package ); };
                    connectPoolSettings->waitReplyPackageSucceedCallback    = [ this ](const auto &connect, const auto &connectPool, const auto &package, const auto &callback)
                        { this->onWaitReplySucceedPackage( connect, connectPool, package, callback ); };
                    connectPoolSettings->waitReplyPackageFailCallback       = [ this ](const auto &connect, const auto &connectPool, const auto &callback)
                        { this->onWaitReplyPackageFail( connect, connectPool, callback ); };

                    connectSettings->randomFlagRangeStart = 1;
                    connectSettings->randomFlagRangeEnd = 999999999;

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

bool JQNetworkClient::waitForCreateConnect(const QString &hostName, const quint16 &port, const int &timeout)
{
    QSharedPointer< QSemaphore > semaphore( new QSemaphore );
    const auto &&hostKey = QString( "%1:%2" ).arg( hostName, QString::number( port ) );

    mutex_.lock();

    waitConnectSucceedSemaphore_[ hostKey ] = semaphore;
    this->createConnect( hostName, port );

    mutex_.unlock();

    const auto &&acquireSucceed = semaphore->tryAcquire( 1, timeout );

    mutex_.lock();

    waitConnectSucceedSemaphore_.remove( hostKey );

    mutex_.unlock();

    return acquireSucceed;
}

int JQNetworkClient::sendPayloadData(
        const QString &hostName,
        const quint16 &port,
        const QByteArray &payloadData,
        const std::function< void(const JQNetworkConnectPointer &connect, const JQNetworkPackageSharedPointer &) > &succeedCallback,
        const std::function< void(const JQNetworkConnectPointer &connect) > &failCallback
    )
{
    for ( const auto &connectPool: this->connectPools_ )
    {
        auto connect = connectPool->getConnectByHostAndPort( hostName, port );

        if ( !connect ) { continue; }

        auto randomFlag = connect->sendPayloadData(
                    payloadData,
                    succeedCallback,
                    failCallback
                );

        if ( !randomFlag ) { return randomFlag; }

        return randomFlag;
    }

    return 0;
}

void JQNetworkClient::onConnectToHostError(const JQNetworkConnectPointer &connect, const JQNetworkConnectPoolPointer &connectPool)
{
    if ( !clientSettings_->connectToHostErrorCallback ) { return; }

    const auto &&reply = connectPool->getHostAndPortByConnect( connect );

    if ( reply.first.isEmpty() || !reply.second )
    {
        qDebug() << "JQNetworkClient::onConnectToHostError: error";
        return;
    }

    processorThreadPool_->run(
                [
                    this,
                    connect,
                    hostName = reply.first,
                    port = reply.second
                ]()
                {
                    this->clientSettings_->connectToHostErrorCallback( connect, hostName, port );
                }
    );
}

void JQNetworkClient::onConnectToHostTimeout(const JQNetworkConnectPointer &connect, const JQNetworkConnectPoolPointer &connectPool)
{
    if ( !clientSettings_->connectToHostTimeoutCallback ) { return; }

    const auto &&reply = connectPool->getHostAndPortByConnect( connect );

    if ( reply.first.isEmpty() || !reply.second )
    {
        qDebug() << "JQNetworkClient::onConnectToHostTimeout: error";
        return;
    }

    processorThreadPool_->run(
                [
                    this,
                    connect,
                    hostName = reply.first,
                    port = reply.second
                ]()
                {
                    this->clientSettings_->connectToHostTimeoutCallback( connect, hostName, port );
                }
    );
}

void JQNetworkClient::onConnectToHostSucceed(const JQNetworkConnectPointer &connect, const JQNetworkConnectPoolPointer &connectPool)
{
    const auto &&reply = connectPool->getHostAndPortByConnect( connect );

    if ( reply.first.isEmpty() || !reply.second )
    {
        qDebug() << "JQNetworkClient::onConnectToHostSucceed: error";
        return;
    }

    processorThreadPool_->run(
                [
                    this,
                    connect,
                    hostName = reply.first,
                    port = reply.second
                ]()
                {
                    const auto &&hostKey = QString( "%1:%2" ).arg( hostName, QString::number( port ) );

                    this->mutex_.lock();

                    auto it = this->waitConnectSucceedSemaphore_.find( hostKey );
                    if ( it != this->waitConnectSucceedSemaphore_.end() )
                    {
                        ( *it )->release( 1 );
                    }

                    this->mutex_.unlock();

                    if ( !this->clientSettings_->connectToHostSucceedCallback ) { return; }
                    this->clientSettings_->connectToHostSucceedCallback( connect, hostName, port );
                }
            );
}

void JQNetworkClient::onRemoteHostClosed(const JQNetworkConnectPointer &connect, const JQNetworkConnectPoolPointer &connectPool)
{
    if ( !clientSettings_->remoteHostClosedCallback ) { return; }

    const auto &&reply = connectPool->getHostAndPortByConnect( connect );

    if ( reply.first.isEmpty() || !reply.second )
    {
        qDebug() << "JQNetworkClient::onRemoteHostClosed: error";
        return;
    }

    processorThreadPool_->run(
                [
                    this,
                    connect,
                    hostName = reply.first,
                    port = reply.second
                ]()
                {
                    this->clientSettings_->remoteHostClosedCallback( connect, hostName, port );
                }
    );
}

void JQNetworkClient::onReadyToDelete(const JQNetworkConnectPointer &connect, const JQNetworkConnectPoolPointer &connectPool)
{
    if ( !clientSettings_->readyToDeleteCallback ) { return; }

    const auto &&reply = connectPool->getHostAndPortByConnect( connect );

    if ( reply.first.isEmpty() || !reply.second )
    {
        qDebug() << "JQNetworkClient::onReadyToDelete: error";
        return;
    }

    processorThreadPool_->run(
                [
                    this,
                    connect,
                    hostName = reply.first,
                    port = reply.second
                ]()
                {
                    this->clientSettings_->readyToDeleteCallback( connect, hostName, port );
                }
    );
}

void JQNetworkClient::onPackageReceived(
        const JQNetworkConnectPointer &connect,
        const JQNetworkConnectPoolPointer &connectPool,
        const JQNetworkPackageSharedPointer &package
    )
{
    if ( !clientSettings_->packageReceivedCallback ) { return; }

    const auto &&reply = connectPool->getHostAndPortByConnect( connect );

    if ( reply.first.isEmpty() || !reply.second )
    {
        qDebug() << "JQNetworkClient::onPackageReceived: error";
        return;
    }

    processorThreadPool_->run(
                [
                    this,
                    connect,
                    package,
                    hostName = reply.first,
                    port = reply.second
                ]()
                {
                    this->clientSettings_->packageReceivedCallback( connect, package, hostName, port );
                }
    );
}

void JQNetworkClient::onWaitReplySucceedPackage(
        const JQNetworkConnectPointer &connect,
        const JQNetworkConnectPoolPointer &,
        const JQNetworkPackageSharedPointer &package,
        const std::function< void(const JQNetworkConnectPointer &connect, const JQNetworkPackageSharedPointer &) > &callback
    )
{
    processorThreadPool_->run(
                [
                    connect,
                    package,
                    callback
                ]()
                {
                    callback( connect, package );
                }
    );
}

void JQNetworkClient::onWaitReplyPackageFail(
        const JQNetworkConnectPointer &connect,
        const JQNetworkConnectPoolPointer &,
        const std::function< void(const JQNetworkConnectPointer &connect) > &callback
    )
{
    processorThreadPool_->run(
                [
                    connect,
                    callback
                ]()
                {
                    callback( connect );
                }
    );
}
