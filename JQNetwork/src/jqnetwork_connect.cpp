/*
    This file is part of JQNetwork
    
    Library introduce: https://github.com/188080501/JQNetwork

    Copyright: Jason

    Contact email: Jason@JasonServer.com
    
    GitHub: https://github.com/188080501/
*/

#include "jqnetwork_connect.h"

// Qt lib import
#include <QDebug>
#include <QTcpSocket>
#include <QTimer>
#include <QThread>
#include <QDateTime>

// JQNetwork lib import
#include <JQNetworkPackage>

// JQNetworkConnect
JQNetworkConnect::JQNetworkConnect():
    tcpSocket_( new QTcpSocket )
{
    connect( tcpSocket_.data(), &QAbstractSocket::stateChanged, this, &JQNetworkConnect::onTcpSocketStateChanged, Qt::DirectConnection );
    connect( tcpSocket_.data(), &QAbstractSocket::bytesWritten, this, &JQNetworkConnect::onTcpSocketBytesWritten, Qt::DirectConnection );
    connect( tcpSocket_.data(), &QTcpSocket::readyRead, this, &JQNetworkConnect::onTcpSocketReadyRead, Qt::DirectConnection );
}

void JQNetworkConnect::createConnect(
        const std::function< void(const JQNetworkConnectSharedPointer &) > &onConnectCreatedCallback,
        const std::function< void( std::function< void() > ) > &runOnConnectThreadCallback,
        const JQNetworkConnectSettingsSharedPointer &connectSettings,
        const QString &hostName,
        const quint16 &port
    )
{
    JQNetworkConnectSharedPointer newConnect( new JQNetworkConnect );
    newConnect->connectSettings_ = connectSettings;
    newConnect->runOnConnectThreadCallback_ = runOnConnectThreadCallback;
    newConnect->sendRandomFlagRotaryIndex_ = connectSettings->randomFlagRangeStart - 1;

    NULLPTR_CHECK( onConnectCreatedCallback );
    onConnectCreatedCallback( newConnect );

    if ( newConnect->connectSettings_->maximumConnectToHostWaitTime != -1 )
    {
        newConnect->timerForConnectToHostTimeOut_ = QSharedPointer< QTimer >( new QTimer );
        connect( newConnect->timerForConnectToHostTimeOut_.data(), &QTimer::timeout, newConnect.data(), &JQNetworkConnect::onTcpSocketConnectToHostTimeOut, Qt::DirectConnection );
        newConnect->timerForConnectToHostTimeOut_->setSingleShot( true );
        newConnect->timerForConnectToHostTimeOut_->start( newConnect->connectSettings_->maximumConnectToHostWaitTime );
    }

    newConnect->tcpSocket_->connectToHost( hostName, port );
}

void JQNetworkConnect::createConnect(
        const std::function< void(const JQNetworkConnectSharedPointer &) > &onConnectCreatedCallback,
        const std::function< void( std::function< void() > ) > &runOnConnectThreadCallback,
        const JQNetworkConnectSettingsSharedPointer &connectSettings,
        const qintptr &socketDescriptor
    )
{
    JQNetworkConnectSharedPointer newConnect( new JQNetworkConnect );
    newConnect->connectSettings_ = connectSettings;
    newConnect->runOnConnectThreadCallback_ = runOnConnectThreadCallback;
    newConnect->sendRandomFlagRotaryIndex_ = connectSettings->randomFlagRangeStart - 1;

    NULLPTR_CHECK( onConnectCreatedCallback );
    onConnectCreatedCallback( newConnect );

    if ( newConnect->connectSettings_->maximumConnectToHostWaitTime != -1 )
    {
        newConnect->timerForConnectToHostTimeOut_ = QSharedPointer< QTimer >( new QTimer );
        connect( newConnect->timerForConnectToHostTimeOut_.data(), &QTimer::timeout, newConnect.data(), &JQNetworkConnect::onTcpSocketConnectToHostTimeOut, Qt::DirectConnection );
        newConnect->timerForConnectToHostTimeOut_->setSingleShot( true );
        newConnect->timerForConnectToHostTimeOut_->start( newConnect->connectSettings_->maximumConnectToHostWaitTime );
    }

    newConnect->tcpSocket_->setSocketDescriptor( socketDescriptor );
}

void JQNetworkConnect::close()
{
    if ( isAbandonTcpSocket_ ) { return; }
    NULLPTR_CHECK( tcpSocket_ );

    this->onReadyToDelete();
}

qint32 JQNetworkConnect::sendPayloadData(
        const QByteArray &payloadData,
        const std::function< void(const JQNetworkConnectPointer &connect, const JQNetworkPackageSharedPointer &) > &succeedCallback,
        const std::function< void(const JQNetworkConnectPointer &connect) > &failCallback
    )
{
    NULLPTR_CHECK( runOnConnectThreadCallback_, 0 );

    mutexForSend_.lock();

    if ( sendRandomFlagRotaryIndex_ >= connectSettings_->randomFlagRangeEnd )
    {
        sendRandomFlagRotaryIndex_ = connectSettings_->randomFlagRangeStart;
    }
    else
    {
        ++sendRandomFlagRotaryIndex_;
    }

    const auto currentRandomFlag = sendRandomFlagRotaryIndex_;

    mutexForSend_.unlock();

    if ( this->thread() == QThread::currentThread() )
    {
        this->reaySendPayloadData( payloadData, currentRandomFlag, succeedCallback, failCallback );
    }
    else
    {
        runOnConnectThreadCallback_(
                    [
                        this,
                        payloadData,
                        currentRandomFlag,
                        succeedCallback,
                        failCallback
                    ]()
            {
                this->reaySendPayloadData( payloadData, currentRandomFlag, succeedCallback, failCallback );
            }
        );
    }

    return currentRandomFlag;
}

qint32 JQNetworkConnect::replyPayloadData(
        const QByteArray &payloadData,
        const qint32 &randomFlag
    )
{
    NULLPTR_CHECK( runOnConnectThreadCallback_, 0 );

    if ( this->thread() == QThread::currentThread() )
    {
        this->reaySendPayloadData( payloadData, randomFlag, nullptr, nullptr );
    }
    else
    {
        runOnConnectThreadCallback_(
                    [
                        this,
                        payloadData,
                        randomFlag
                    ]()
            {
                this->reaySendPayloadData( payloadData, randomFlag, nullptr, nullptr );
            }
        );
    }

    return randomFlag;
}

void JQNetworkConnect::onTcpSocketStateChanged()
{
    if ( isAbandonTcpSocket_ ) { return; }
    NULLPTR_CHECK( tcpSocket_ );

    const auto &&state = tcpSocket_->state();

//    qDebug() << "onTcpSocketStateChanged:" << this << ": state:" << state;

    switch ( state )
    {
        case QAbstractSocket::ConnectedState:
        {
            if ( !timerForConnectToHostTimeOut_.isNull() )
            {
                timerForConnectToHostTimeOut_.clear();
            }

            NULLPTR_CHECK( connectSettings_->connectToHostSucceedCallback );
            connectSettings_->connectToHostSucceedCallback( this );

            onceConnectSucceed_ = true;

            break;
        }
        case QAbstractSocket::UnconnectedState:
        {
            switch ( tcpSocket_->error() )
            {
                case QAbstractSocket::UnknownSocketError:
                {
                    if ( onceConnectSucceed_ ) { break; }

                    NULLPTR_CHECK( connectSettings_->connectToHostErrorCallback );
                    connectSettings_->connectToHostErrorCallback( this );

                    break;
                }
                case QAbstractSocket::RemoteHostClosedError:
                {
                    NULLPTR_CHECK( connectSettings_->remoteHostClosedCallback );
                    connectSettings_->remoteHostClosedCallback( this );
                    break;
                }
                case QAbstractSocket::HostNotFoundError:
                case QAbstractSocket::ConnectionRefusedError:
                {
                    NULLPTR_CHECK( connectSettings_->connectToHostErrorCallback );
                    connectSettings_->connectToHostErrorCallback( this );
                    break;
                }
                default:
                {
                    qDebug() << "onTcpSocketStateChanged: unknow error:" << tcpSocket_->error();
                    break;
                }
            }

            this->onReadyToDelete();
            break;
        }
        default: { break; }
    }
}

void JQNetworkConnect::onTcpSocketConnectToHostTimeOut()
{
    if ( isAbandonTcpSocket_ ) { return; }
    NULLPTR_CHECK( timerForConnectToHostTimeOut_ );
    NULLPTR_CHECK( tcpSocket_ );

//    qDebug() << __func__;

    NULLPTR_CHECK( connectSettings_->connectToHostTimeoutCallback );
    connectSettings_->connectToHostTimeoutCallback( this );

    this->onReadyToDelete();
}

void JQNetworkConnect::onTcpSocketReadyRead()
{
    if ( isAbandonTcpSocket_ ) { return; }
    NULLPTR_CHECK( tcpSocket_ );

    const auto &&data = tcpSocket_->readAll();

    tcpSocketBuffer_.append( data );

//    qDebug() << tcpSocketBuffer_.size() << data.size();

    forever
    {
        const auto &&checkReply = JQNetworkPackage::checkDataIsReadyReceive( tcpSocketBuffer_ );
//        qDebug() << checkReply;

        if ( checkReply > 0 )
        {
            return;
        }
        else if ( checkReply < 0 )
        {
            tcpSocketBuffer_.remove( 0, checkReply * -1 );
        }
        else
        {
            auto package = JQNetworkPackage::createPackageFromRawData( tcpSocketBuffer_ );
            if ( package->isCompletePackage() )
            {
                this->onPackageReceived( package );
            }
            else
            {
                const auto &&itForPackage = receivePackagePool_.find( package->randomFlag() );
                if ( itForPackage != receivePackagePool_.end() )
                {
                    if ( !(*itForPackage)->mixPackage( package ) )
                    {
                        receivePackagePool_.erase( itForPackage );
                        return;
                    }

                    if ( (*itForPackage)->isCompletePackage() && !(*itForPackage)->isAbandonPackage() )
                    {
                        this->onPackageReceived( *itForPackage );
                        receivePackagePool_.erase( itForPackage );
                    }
                }
                else
                {
                    receivePackagePool_[ package->randomFlag() ] = package;
                }
            }
        }
    }
}

void JQNetworkConnect::onSendPackageCheck()
{
    //...
}

void JQNetworkConnect::onPackageReceived(const JQNetworkPackageSharedPointer &package)
{
    if ( ( connectSettings_->randomFlagRangeStart <= package->randomFlag() ) &&
         ( package->randomFlag() < connectSettings_->randomFlagRangeEnd ) )
    {
        auto it = onReceivedCallbacks_.find( package->randomFlag() );
        if ( it == onReceivedCallbacks_.end() ) { return; }

        if ( it->succeedCallback )
        {
            NULLPTR_CHECK( connectSettings_->waitReplyPackageSucceedCallback );
            connectSettings_->waitReplyPackageSucceedCallback( this, package, it->succeedCallback );
        }

        onReceivedCallbacks_.erase( it );
    }
    else
    {
        NULLPTR_CHECK( connectSettings_->packageReceivedCallback );
        connectSettings_->packageReceivedCallback( this, package );
    }
}

void JQNetworkConnect::onTcpSocketBytesWritten(const qint64 &bytes)
{
    if ( isAbandonTcpSocket_ ) { return; }
    NULLPTR_CHECK( tcpSocket_ );

    waitForSendBytes_ -= bytes;
    alreadyWrittenBytes_ += bytes;

//    qDebug() << "onTcpSocketBytesWritten:" << waitForSendBytes_ << alreadyWrittenBytes_ << QThread::currentThread();
}

void JQNetworkConnect::onReadyToDelete()
{
    if ( isAbandonTcpSocket_ ) { return; }
    isAbandonTcpSocket_ = true;

    if ( !timerForConnectToHostTimeOut_ )
    {
        timerForConnectToHostTimeOut_.clear();
    }

    NULLPTR_CHECK( tcpSocket_ );
    tcpSocket_->close();

    NULLPTR_CHECK( connectSettings_->readyToDeleteCallback );
    connectSettings_->readyToDeleteCallback( this );
}

void JQNetworkConnect::reaySendPayloadData(
        const QByteArray &payloadData,
        const qint32 &randomFlag,
        const std::function< void(const JQNetworkConnectPointer &connect, const JQNetworkPackageSharedPointer &) > &succeedCallback,
        const std::function< void(const JQNetworkConnectPointer &connect) > &failCallback
    )
{
    if ( isAbandonTcpSocket_ ) { return; }
    NULLPTR_CHECK( tcpSocket_ );

    const auto &&package = JQNetworkPackage::createPackageFromPayloadData( payloadData, randomFlag );
    const auto &&buffer = package->toByteArray();

    waitForSendBytes_ += buffer.size();
    tcpSocket_->write( buffer );
//    qDebug() << "write:" << tcpSocket_->write( buffer );

    if ( succeedCallback || failCallback )
    {
        onReceivedCallbacks_[ randomFlag ] =
        {
            QDateTime::currentMSecsSinceEpoch(),
            succeedCallback,
            failCallback
        };
    }
}
