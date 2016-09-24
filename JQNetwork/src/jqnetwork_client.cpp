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
                    JQNetworkConnectPoolSettingsSharedPointer connectPoolSettings( new JQNetworkConnectPoolSettings( *this->connectPoolSettings_ ) );
                    JQNetworkConnectSettingsSharedPointer connectSettings( new JQNetworkConnectSettings( *this->connectSettings_ ) );

                    connectPoolSettings->connectToHostErrorCallback         = [ this ](const auto &connect){ this->onConnectToHostError( connect ); };
                    connectPoolSettings->connectToHostTimeoutCallback       = [ this ](const auto &connect){ this->onConnectToHostTimeout( connect ); };
                    connectPoolSettings->connectToHostSucceedCallback       = [ this ](const auto &connect){ this->onConnectToHostSucceed( connect ); };
                    connectPoolSettings->remoteHostClosedCallback           = [ this ](const auto &connect){ this->onRemoteHostClosed( connect ); };
                    connectPoolSettings->readyToDeleteCallback              = [ this ](const auto &connect){ this->onReadyToDelete( connect ); };
                    connectPoolSettings->packageReceivedCallback            = [ this ](const auto &connect, const auto &package){ this->onPackageReceived( connect, package ); };
                    connectPoolSettings->waitReplyPackageSucceedCallback    = [ this ](const auto &connect, const auto &package, const auto &callback){ this->onWaitReplySucceedPackage( connect, package, callback ); };
                    connectPoolSettings->waitReplyPackageFailCallback       = [ this ](const auto &connect, const auto &callback){ this->onWaitReplyPackageFail( connect, callback ); };

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
        const JQNetworkOnReceivedCallbackPackage &callbackPackage
    )
{
    for ( const auto &connectPool: this->connectPools_ )
    {
        auto connect = connectPool->getConnectByHostAndPort( hostName, port );

        if ( !connect ) { continue; }

        auto randomFlag = connect->sendPayloadData(
                    payloadData,
                    callbackPackage
                );

        if ( !randomFlag ) { return randomFlag; }

        return randomFlag;
    }

    return 0;
}

void JQNetworkClient::onConnectToHostError(const JQNetworkConnectPointer &)
{
    if ( !clientSettings_->connectToHostErrorCallback ) { return; }

    // TODO
}

void JQNetworkClient::onConnectToHostTimeout(const JQNetworkConnectPointer &)
{
    if ( !clientSettings_->connectToHostTimeoutCallback ) { return; }

    // TODO
}

void JQNetworkClient::onConnectToHostSucceed(const JQNetworkConnectPointer &connect)
{
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

                        const auto &&hostKey = QString( "%1:%2" ).arg( reply.first, QString::number( reply.second ) );

                        mutex_.lock();

                        auto it = waitConnectSucceedSemaphore_.find( hostKey );
                        if ( it != waitConnectSucceedSemaphore_.end() )
                        {
                            ( *it )->release( 1 );
                        }

                        mutex_.unlock();

                        if ( !clientSettings_->connectToHostSucceedCallback ) { break; }

                        this->clientSettings_->connectToHostSucceedCallback( connect, reply.first, reply.second );

                        break;
                    }
                }
    );
}

void JQNetworkClient::onRemoteHostClosed(const JQNetworkConnectPointer &)
{
    if ( !clientSettings_->remoteHostClosedCallback ) { return; }

    // TODO
}

void JQNetworkClient::onReadyToDelete(const JQNetworkConnectPointer &)
{
    if ( !clientSettings_->readyToDeleteCallback ) { return; }

    // TODO
}

void JQNetworkClient::onPackageReceived(const JQNetworkConnectPointer &, const JQNetworkPackageSharedPointer &)
{
    if ( !clientSettings_->packageReceivedCallback ) { return; }

    // TODO
}

void JQNetworkClient::onWaitReplySucceedPackage(
        const JQNetworkConnectPointer &connect,
        const JQNetworkPackageSharedPointer &package,
        const std::function< void(const JQNetworkConnectPointer &connect, const JQNetworkPackageSharedPointer &) > &callback
    )
{
//    qDebug() << "onWaitReplySucceedPackage: 1:" << QThread::currentThread() << this->thread() << connect->thread();

    processorThreadPool_->run(
                [
                    connect,
                    package,
                    callback
                ]()
                {
//                    qDebug() << "onWaitReplySucceedPackage: 2:" << QThread::currentThread();

                    callback( connect, package );
                }
    );
}

void JQNetworkClient::onWaitReplyPackageFail(
        const JQNetworkConnectPointer &connect,
        const std::function< void(const JQNetworkConnectPointer &connect) > &callback
    )
{
//    qDebug() << "onWaitReplyPackageFail: 1:" << QThread::currentThread();

    processorThreadPool_->run(
                [
                    connect,
                    callback
                ]()
                {
//                    qDebug() << "onWaitReplyPackageFail: 2:" << QThread::currentThread();

                    callback( connect );
                }
    );
}
