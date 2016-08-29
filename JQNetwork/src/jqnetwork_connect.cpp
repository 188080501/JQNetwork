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

using namespace JQNetwork;

JQNetworkConnect::JQNetworkConnect():
    tcpSocket_( new QTcpSocket )
{
    connect( tcpSocket_.data(), &QTcpSocket::readyRead, this, &JQNetworkConnect::onTcpSocketReadyRead, Qt::DirectConnection );
    connect( tcpSocket_.data(), ( QAbstractSocket::SocketError( QAbstractSocket::* )() const )&QAbstractSocket::error, this, &JQNetworkConnect::onTcpSocketError, Qt::DirectConnection );
}

QSharedPointer< JQNetworkConnect > JQNetworkConnect::createConnect()
{
    QSharedPointer< JQNetworkConnect > newConnect( new JQNetworkConnect );

    // TODO

    return newConnect;
}

void JQNetworkConnect::onTcpSocketReadyRead()
{
    const auto &&data = tcpSocket_->readAll();

    qDebug() << "onTcpSocketReadyRead: readAll:" << data;
}

void JQNetworkConnect::onTcpSocketError()
{
    switch ( tcpSocket_->error() )
    {
        case QAbstractSocket::HostNotFoundError:
        {
            break;
        }
        default: { break; }
    }
}
