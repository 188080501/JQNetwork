﻿/*
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

using namespace std;
using namespace std::placeholders;

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
{ }

JQNetworkClient::~JQNetworkClient()
{
    socketThreadPool_->waitRunEach(
                [ & ]()
                {
                    connectPools_[ QThread::currentThread() ].clear();
                }
    );
}

JQNetworkClientSharedPointer JQNetworkClient::createClient(
        const bool &fileTransferEnabled
    )
{
    JQNetworkClientSettingsSharedPointer clientSettings( new JQNetworkClientSettings );
    JQNetworkConnectPoolSettingsSharedPointer connectPoolSettings( new JQNetworkConnectPoolSettings );
    JQNetworkConnectSettingsSharedPointer connectSettings( new JQNetworkConnectSettings );

    if ( fileTransferEnabled )
    {
        connectSettings->fileTransferEnabled = true;
        connectSettings->setFilePathProviderToDefaultDir();
    }

    return JQNetworkClientSharedPointer( new JQNetworkClient( clientSettings, connectPoolSettings, connectSettings ) );
}

bool JQNetworkClient::begin()
{
    nodeMarkSummary_ = JQNetworkNodeMark::calculateNodeMarkSummary( clientSettings_->dutyMark );

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
        processorThreadPool_ = QSharedPointer< JQNetworkThreadPool >( new JQNetworkThreadPool( clientSettings_->globalProcessorThreadCount ) );
        globalProcessorThreadPool_ = processorThreadPool_.toWeakRef();
    }

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

                    connectPoolSettings->connectToHostErrorCallback         = bind( &JQNetworkClient::onConnectToHostError, this, _1, _2 );
                    connectPoolSettings->connectToHostTimeoutCallback       = bind( &JQNetworkClient::onConnectToHostTimeout, this, _1, _2 );
                    connectPoolSettings->connectToHostSucceedCallback       = bind( &JQNetworkClient::onConnectToHostSucceed, this, _1, _2 );
                    connectPoolSettings->remoteHostClosedCallback           = bind( &JQNetworkClient::onRemoteHostClosed, this, _1, _2 );
                    connectPoolSettings->readyToDeleteCallback              = bind( &JQNetworkClient::onReadyToDelete, this, _1, _2 );
                    connectPoolSettings->packageSendingCallback             = bind( &JQNetworkClient::onPackageSending, this, _1, _2, _3, _4, _5, _6 );
                    connectPoolSettings->packageReceivingCallback           = bind( &JQNetworkClient::onPackageReceiving, this, _1, _2, _3, _4, _5, _6 );
                    connectPoolSettings->packageReceivedCallback            = bind( &JQNetworkClient::onPackageReceived, this, _1, _2, _3 );
                    connectPoolSettings->waitReplyPackageSucceedCallback    = bind( &JQNetworkClient::onWaitReplySucceedPackage, this, _1, _2, _3, _4 );
                    connectPoolSettings->waitReplyPackageFailCallback       = bind( &JQNetworkClient::onWaitReplyPackageFail, this, _1, _2, _3 );

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

    auto runOnConnectThreadCallback =
            [
                this,
                rotaryIndex
            ](const std::function< void() > &callback)
            {
                this->socketThreadPool_->run( callback, rotaryIndex );
            };

    socketThreadPool_->run(
                [
                    this,
                    runOnConnectThreadCallback,
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

qint32 JQNetworkClient::sendPayloadData(
        const QString &hostName,
        const quint16 &port,
        const QByteArray &payloadData,
        const std::function< void(const JQNetworkConnectPointer &connect, const JQNetworkPackageSharedPointer &) > &succeedCallback,
        const std::function< void(const JQNetworkConnectPointer &connect) > &failCallback
    )
{
    auto connect = this->getConnect( hostName, port );

    if ( !connect ) { return 0; }

    auto randomFlag = connect->sendPayloadData(
                payloadData,
                succeedCallback,
                failCallback
            );

    if ( !randomFlag ) { return randomFlag; }

    return randomFlag;
}

JQNetworkConnectPointer JQNetworkClient::getConnect(const QString &hostName, const quint16 &port)
{
    for ( const auto &connectPool: this->connectPools_ )
    {
        auto connect = connectPool->getConnectByHostAndPort( hostName, port );

        if ( !connect ) { continue; }

        return connect;
    }

    return { };
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

void JQNetworkClient::onPackageSending(
        const JQNetworkConnectPointer &connect,
        const JQNetworkConnectPoolPointer &connectPool,
        const qint32 &randomFlag,
        const qint64 &payloadCurrentIndex,
        const qint64 &payloadCurrentSize,
        const qint64 &payloadTotalSize
    )
{
    if ( !clientSettings_->packageSendingCallback ) { return; }

    const auto &&reply = connectPool->getHostAndPortByConnect( connect );

    if ( reply.first.isEmpty() || !reply.second )
    {
        qDebug() << "JQNetworkClient::onPackageSending: error";
        return;
    }

    processorThreadPool_->run(
                [
                    this,
                    connect,
                    hostName = reply.first,
                    port = reply.second,
                    randomFlag,
                    payloadCurrentIndex,
                    payloadCurrentSize,
                    payloadTotalSize
                ]()
                {
                    this->clientSettings_->packageSendingCallback(
                        connect,
                        hostName,
                        port,
                        randomFlag,
                        payloadCurrentIndex,
                        payloadCurrentSize,
                        payloadTotalSize
                    );
                }
    );
}

void JQNetworkClient::onPackageReceiving(
        const JQNetworkConnectPointer &connect,
        const JQNetworkConnectPoolPointer &connectPool,
        const qint32 &randomFlag,
        const qint64 &payloadCurrentIndex,
        const qint64 &payloadCurrentSize,
        const qint64 &payloadTotalSize
    )
{
    if ( !clientSettings_->packageReceivingCallback ) { return; }

    const auto &&reply = connectPool->getHostAndPortByConnect( connect );

    if ( reply.first.isEmpty() || !reply.second )
    {
        qDebug() << "JQNetworkClient::onPackageReceiving: error";
        return;
    }

    processorThreadPool_->run(
                [
                    this,
                    connect,
                    hostName = reply.first,
                    port = reply.second,
                    randomFlag,
                    payloadCurrentIndex,
                    payloadCurrentSize,
                    payloadTotalSize
                ]()
                {
                    this->clientSettings_->packageReceivingCallback(
                        connect,
                        hostName,
                        port,
                        randomFlag,
                        payloadCurrentIndex,
                        payloadCurrentSize,
                        payloadTotalSize
                    );
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
                    hostName = reply.first,
                    port = reply.second,
                    package
                ]()
                {
                    this->clientSettings_->packageReceivedCallback( connect, hostName, port, package );
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
