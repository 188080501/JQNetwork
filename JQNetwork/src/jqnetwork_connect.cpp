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

// JQNetwork lib import
#include <JQNetworkPackage>

using namespace JQNetwork;

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
        const std::function< void( std::function< void() > ) > runOnConnectThreadCallback,
        const JQNetworkConnectSettingsSharedPointer &connectSettings,
        const QString &hostName,
        const quint16 &port
    )
{
    JQNetworkConnectSharedPointer newConnect( new JQNetworkConnect );
    newConnect->connectSettings_ = connectSettings;
    newConnect->runOnConnectThreadCallback_ = runOnConnectThreadCallback;

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
        const std::function< void( std::function< void() > ) > runOnConnectThreadCallback,
        const JQNetworkConnectSettingsSharedPointer &connectSettings,
        const qintptr &socketDescriptor
    )
{
    JQNetworkConnectSharedPointer newConnect( new JQNetworkConnect );
    newConnect->connectSettings_ = connectSettings;
    newConnect->runOnConnectThreadCallback_ = runOnConnectThreadCallback;

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

qint32 JQNetworkConnect::sendPayloadData(const QByteArray &payloadData)
{
    NULLPTR_CHECK( runOnConnectThreadCallback_, 0 );

    mutexForSend_.lock();

    ++sendRotaryIndex_;
    if ( sendRotaryIndex_ >= 1000000000 )
    {
        sendRotaryIndex_ = 1;
    }

    const auto currentRandomFlag = sendRotaryIndex_;

    mutexForSend_.unlock();

    if ( this->thread() == QThread::currentThread() )
    {
        this->sendPayloadData( payloadData, currentRandomFlag );
    }
    else
    {
        runOnConnectThreadCallback_(
                    [
                        this,
                        payloadData,
                        currentRandomFlag
                    ]()
            {
                this->sendPayloadData( payloadData, currentRandomFlag );
            }
        );
    }

    return currentRandomFlag;
}

void JQNetworkConnect::onTcpSocketStateChanged()
{
    if ( isAbandonTcpSocket_ ) { return; }
    NULLPTR_CHECK( tcpSocket_ );

    const auto &&state = tcpSocket_->state();

    qDebug() << __func__ << this << ": state:" << state;

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
                    qDebug() << __func__ << ": unknow error:" << tcpSocket_->error();
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

    qDebug() << __func__;

    NULLPTR_CHECK( connectSettings_->connectToHostTimeoutCallback );
    connectSettings_->connectToHostTimeoutCallback( this );

    this->onReadyToDelete();
}

void JQNetworkConnect::onTcpSocketReadyRead()
{
    if ( isAbandonTcpSocket_ ) { return; }
    NULLPTR_CHECK( tcpSocket_ );

    const auto &&data = tcpSocket_->readAll();

    qDebug() << __func__ << ": size:" << data.size() << this << QThread::currentThread();

    if ( tcpSocketBuffer_.isEmpty() )
    {
        tcpSocketBuffer_ = data;
    }
    else
    {
        tcpSocketBuffer_.append( data );
    }

    forever
    {
        const auto &&checkReply = JQNetworkPackage::checkDataIsReadyReceive( tcpSocketBuffer_ );
        if ( checkReply > 0 ) { return; }
        else if ( checkReply < 0 ) { tcpSocketBuffer_.remove( 0, checkReply * -1 ); }
        else { break; }
    }

    auto package = JQNetworkPackage::createPackageFromRawData( tcpSocketBuffer_ );
    if ( package->isCompletePackage() )
    {
        NULLPTR_CHECK( connectSettings_->packageReceivedCallback );

        connectSettings_->packageReceivedCallback( this, package );
    }

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
            NULLPTR_CHECK( connectSettings_->packageReceivedCallback );

            connectSettings_->packageReceivedCallback( this, *itForPackage );

            receivePackagePool_.erase( itForPackage );
        }
    }
    else
    {
        receivePackagePool_[ package->randomFlag() ] = package;
    }
}

void JQNetworkConnect::onTcpSocketBytesWritten(const qint64 &bytes)
{
    if ( isAbandonTcpSocket_ ) { return; }
    NULLPTR_CHECK( tcpSocket_ );

    waitForSendBytes_ -= bytes;
    alreadyWrittenBytes_ += bytes;

    qDebug() << __func__ << ":" << waitForSendBytes_ << alreadyWrittenBytes_;
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

void JQNetworkConnect::sendPayloadData(const QByteArray &payloadData, const qint32 &randomFlag)
{
    if ( isAbandonTcpSocket_ ) { return; }
    NULLPTR_CHECK( tcpSocket_ );

    auto package = JQNetworkPackage::createPackageFromPayloadData( payloadData, randomFlag );
    auto buffer = package->toByteArray();

    waitForSendBytes_ += buffer.size();
    tcpSocket_->write( buffer );
}
