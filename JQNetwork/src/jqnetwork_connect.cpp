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

// JQNetworkConnect
JQNetworkConnect::JQNetworkConnect():
    tcpSocket_( new QTcpSocket )
{
    connect( tcpSocket_.data(), &QAbstractSocket::stateChanged, this, &JQNetworkConnect::onTcpSocketStateChanged, Qt::DirectConnection );
    connect( tcpSocket_.data(), &QAbstractSocket::bytesWritten, this, &JQNetworkConnect::onTcpSocketBytesWritten, Qt::DirectConnection );
    connect( tcpSocket_.data(), &QTcpSocket::readyRead, this, &JQNetworkConnect::onTcpSocketReadyRead, Qt::DirectConnection );
}

void JQNetworkConnect::createConnectByHostAndPort(
        const std::function< void(const QSharedPointer< JQNetworkConnect > &) > &onConnectCreatedCallback,
        const QSharedPointer< JQNetworkConnectSettings > &connectSettings,
        const QString &hostName,
        const quint16 &port
    )
{
    QSharedPointer< JQNetworkConnect > newConnect( new JQNetworkConnect );
    newConnect->connectSettings_ = connectSettings;

    NULLPTR_CHECK( onConnectCreatedCallback );
    onConnectCreatedCallback( newConnect );

    newConnect->tcpSocket_->connectToHost( hostName, port );

    if ( newConnect->connectSettings_->maximumConnectToHostWaitTime != -1 )
    {
        newConnect->timerForConnectToHostTimeOut_ = QSharedPointer< QTimer >( new QTimer );
        connect( newConnect->timerForConnectToHostTimeOut_.data(), &QTimer::timeout, newConnect.data(), &JQNetworkConnect::onTcpSocketConnectToHostTimeOut, Qt::DirectConnection );
        newConnect->timerForConnectToHostTimeOut_->setSingleShot( true );
        newConnect->timerForConnectToHostTimeOut_->start( newConnect->connectSettings_->maximumConnectToHostWaitTime );
    }
}

void JQNetworkConnect::createConnectBySocketDescriptor(
        const std::function< void(const QSharedPointer< JQNetworkConnect > &) > &onConnectCreatedCallback,
        const QSharedPointer< JQNetworkConnectSettings > &connectSettings,
        const qintptr &socketDescriptor
    )
{
    QSharedPointer< JQNetworkConnect > newConnect( new JQNetworkConnect );
    newConnect->connectSettings_ = connectSettings;

    NULLPTR_CHECK( onConnectCreatedCallback );
    onConnectCreatedCallback( newConnect );

    newConnect->tcpSocket_->setSocketDescriptor( socketDescriptor );

    if ( newConnect->connectSettings_->maximumConnectToHostWaitTime != -1 )
    {
        newConnect->timerForConnectToHostTimeOut_ = QSharedPointer< QTimer >( new QTimer );
        connect( newConnect->timerForConnectToHostTimeOut_.data(), &QTimer::timeout, newConnect.data(), &JQNetworkConnect::onTcpSocketConnectToHostTimeOut, Qt::DirectConnection );
        newConnect->timerForConnectToHostTimeOut_->setSingleShot( true );
        newConnect->timerForConnectToHostTimeOut_->start( newConnect->connectSettings_->maximumConnectToHostWaitTime );
    }
}

void JQNetworkConnect::onTcpSocketStateChanged()
{
    if ( abandonTcpSocket ) { return; }
    NULLPTR_CHECK( tcpSocket_ );

    const auto &&state = tcpSocket_->state();

    qDebug() << __func__ << ": state:" << state;

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
