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

using namespace JQNetwork;

JQNetworkConnect::JQNetworkConnect():
    tcpSocket_( new QTcpSocket )
{
    connect( tcpSocket_.data(), &QAbstractSocket::stateChanged, this, &JQNetworkConnect::onTcpSocketStateChanged );
    connect( tcpSocket_.data(), &QAbstractSocket::bytesWritten, this, &JQNetworkConnect::onTcpSocketBytesWritten );
    connect( tcpSocket_.data(), &QTcpSocket::readyRead, this, &JQNetworkConnect::onTcpSocketReadyRead );

    tcpSocketLastState_ = tcpSocket_->state();
}

QSharedPointer< JQNetworkConnect > JQNetworkConnect::createConnectByHostAndPort(
        const QSharedPointer< JQNetworkConnectSettings > &connectSettings,
        const QString &hostName,
        const quint16 &port
    )
{
    QSharedPointer< JQNetworkConnect > newConnect( new JQNetworkConnect );
    newConnect->connectSettings_ = connectSettings;

    newConnect->tcpSocket_->connectToHost( hostName, port );

    if ( newConnect->connectSettings_->maximumConnectToHostWaitTime != -1 )
    {
        newConnect->timerForConnectToHostTimeOut_ = QSharedPointer< QTimer >( new QTimer );
        QObject::connect( newConnect->timerForConnectToHostTimeOut_.data(), &QTimer::timeout, std::bind( &JQNetworkConnect::onTcpSocketConnectToHostTimeOut, newConnect.data() ) );
        newConnect->timerForConnectToHostTimeOut_->setSingleShot( true );
        newConnect->timerForConnectToHostTimeOut_->start( newConnect->connectSettings_->maximumConnectToHostWaitTime );
    }

    return newConnect;
}

void JQNetworkConnect::onTcpSocketStateChanged()
{
    if ( abandonTcpSocket ) { return; }
    NULLPTR_CHECK( tcpSocket_ );

    const auto &&lastState = (QTcpSocket::SocketState)tcpSocketLastState_;
    const auto &&currentState = tcpSocket_->state();

    qDebug() << __func__ << ": lastState:" << lastState << ", currentState:" << currentState;

    switch ( currentState )
    {
        case QAbstractSocket::ConnectedState:
        {
            if ( !timerForConnectToHostTimeOut_.isNull() )
            {
                timerForConnectToHostTimeOut_.clear();
            }

            NULLPTR_CHECK( connectSettings_->connectToHostSucceedCallback );
            connectSettings_->connectToHostSucceedCallback( this );

            break;
        }
        case QAbstractSocket::UnconnectedState:
        {
            switch ( tcpSocket_->error() )
            {
                case QAbstractSocket::UnknownSocketError: { break; }
                case QAbstractSocket::RemoteHostClosedError:
                {
                    NULLPTR_CHECK( connectSettings_->remoteHostClosedCallback );
                    connectSettings_->remoteHostClosedCallback( this );
                    break;
                }
                case QAbstractSocket::HostNotFoundError:
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

    tcpSocketLastState_ = currentState;
}

void JQNetworkConnect::onTcpSocketConnectToHostTimeOut()
{
    if ( abandonTcpSocket ) { return; }
    NULLPTR_CHECK( timerForConnectToHostTimeOut_ );
    NULLPTR_CHECK( tcpSocket_ );

    qDebug() << __func__;

    NULLPTR_CHECK( connectSettings_->connectToHostTimeoutCallback );
    connectSettings_->connectToHostTimeoutCallback( this );

    this->onReadyToDelete();
}

void JQNetworkConnect::onTcpSocketReadyRead()
{
    if ( abandonTcpSocket ) { return; }
    NULLPTR_CHECK( tcpSocket_ );

    const auto &&data = tcpSocket_->readAll();

    qDebug() << __func__ << ": size:" << data.size();
}

void JQNetworkConnect::onTcpSocketBytesWritten(const qint64 &bytes)
{
    if ( abandonTcpSocket ) { return; }
    NULLPTR_CHECK( tcpSocket_ );

    static qint64 total = 0;
    total += bytes;

    qDebug() << __func__ << ":" << bytes << total;
}

void JQNetworkConnect::onReadyToDelete()
{
    if ( abandonTcpSocket ) { return; }
    abandonTcpSocket = true;

    if ( !timerForConnectToHostTimeOut_ )
    {
        timerForConnectToHostTimeOut_.clear();
    }

    NULLPTR_CHECK( tcpSocket_ );
    tcpSocket_->close();

    NULLPTR_CHECK( connectSettings_->readyToDeleteCallback );
    connectSettings_->readyToDeleteCallback( this );
}
