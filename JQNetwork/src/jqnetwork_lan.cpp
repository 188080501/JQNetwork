/*
    This file is part of JQNetwork
    
    Library introduce: https://github.com/188080501/JQNetwork

    Copyright: Jason

    Contact email: Jason@JasonServer.com
    
    GitHub: https://github.com/188080501/
*/

#include "jqnetwork_lan.h"

// Qt lib import
#include <QDebug>
#include <QUdpSocket>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QNetworkInterface>

QWeakPointer< JQNetworkThreadPool > JQNetworkLan::globalProcessorThreadPool_;

JQNetworkLan::JQNetworkLan(const JQNetworkLanSettingsSharedPointer &lanSettings):
    lanSettings_( lanSettings )
{ }

JQNetworkLan::~JQNetworkLan()
{
    processorThreadPool_->waitRun(
                [
                    this
                ]()
                {
                    mutex_.lock();

                    this->sendOffline();

                    timerForCheckLoop_.clear();

                    udpSocketForMulticastGroupAddress_.clear();
                    udpSocketForBroadcastAddress_.clear();

                    for ( const auto &lanNode: lanNodes_ )
                    {
                        if ( lanSettings_->lanNodeOfflineCallback )
                        {
                            lanSettings_->lanNodeOfflineCallback( lanNode );
                        }
                    }

                    lanNodes_.clear();

                    mutex_.unlock();
                }
    );
}

JQNetworkLanSharedPointer JQNetworkLan::createLan(
        const QHostAddress &multicastGroupAddress,
        const quint16 &multicastGroupAddressBindPort,
        const quint16 &broadcastAddressBindPort,
        const QString &dutyMark
    )
{
    JQNetworkLanSettingsSharedPointer lanSettings( new JQNetworkLanSettings );

    lanSettings->dutyMark = dutyMark;

    lanSettings->multicastGroupAddress = multicastGroupAddress;
    lanSettings->multicastGroupAddressBindPort = multicastGroupAddressBindPort;
    lanSettings->broadcastAddressBindPort = broadcastAddressBindPort;

    return JQNetworkLanSharedPointer( new JQNetworkLan( lanSettings ) );
}

QList< JQNetworkLanAddressEntries > JQNetworkLan::getLanAddressEntries()
{
    QList< JQNetworkLanAddressEntries > result;

    for ( const auto &interface: QNetworkInterface::allInterfaces() )
    {
        if ( interface.flags() != (
                 QNetworkInterface::IsUp |
                 QNetworkInterface::IsRunning |
                 QNetworkInterface::CanBroadcast |
                 QNetworkInterface::CanMulticast
                 )
             )
        { continue; }

        bool isVmAddress = interface.humanReadableName().toLower().startsWith("vm");

        for (const auto &addressEntry: interface.addressEntries() )
        {
            if ( !addressEntry.ip().toIPv4Address() ) { continue; }

            result.push_back( {
                                  addressEntry.ip(),
                                  addressEntry.netmask(),
                                  QHostAddress( addressEntry.ip().toIPv4Address() & addressEntry.netmask().toIPv4Address() ),
                                  isVmAddress
                              } );
        }
    }

    if ( result.isEmpty() )
    {
        result.push_back( {
                              QHostAddress::LocalHost,
                              QHostAddress::Broadcast,
                              QHostAddress::Broadcast,
                              false
                          } );
    }

    return result;
}

bool JQNetworkLan::begin()
{
    nodeMarkSummary_ = JQNetworkNodeMark::calculateNodeMarkSummary( lanSettings_->dutyMark );

    if ( globalProcessorThreadPool_ )
    {
        processorThreadPool_ = globalProcessorThreadPool_.toStrongRef();
    }
    else
    {
        processorThreadPool_ = QSharedPointer< JQNetworkThreadPool >( new JQNetworkThreadPool( lanSettings_->globalProcessorThreadCount ) );
        globalProcessorThreadPool_ = processorThreadPool_.toWeakRef();
    }

    bool bindSucceed = false;

    processorThreadPool_->waitRun(
                [
                    this,
                    &bindSucceed
                ]()
                {
                    timerForCheckLoop_.reset( new QTimer );
                    timerForCheckLoop_->setSingleShot( true );
                    timerForCheckLoop_->setInterval(  lanSettings_->checkLoopInterval);

                    QObject::connect( timerForCheckLoop_.data(), &QTimer::timeout, this, &JQNetworkLan::checkLoop, Qt::DirectConnection );

                    bindSucceed = this->refreshUdp();
                    if ( !bindSucceed ) { return; }

                    this->checkLoop();
                }
    );

    return bindSucceed;
}

QHostAddress JQNetworkLan::matchLanAddressEntries(const QList< QHostAddress > &ipList)
{
    for ( const auto &currentAddress: ipList )
    {
        for ( const auto &lanAddressEntries: lanAddressEntries_ )
        {
            if ( ( ( currentAddress.toIPv4Address() & lanAddressEntries.netmask.toIPv4Address() ) == lanAddressEntries.ipSegment.toIPv4Address() ) ||
                 ( currentAddress == QHostAddress::LocalHost))
            {
                return currentAddress;
            }
        }
    }

    return { };
}

QList< JQNetworkLanNode > JQNetworkLan::availableLanNodes()
{
    QList< JQNetworkLanNode > result;

    mutex_.lock();

    for ( const auto &lanAddressEntries: lanNodes_ )
    {
        result.push_back( lanAddressEntries );
    }

    mutex_.unlock();

    return result;
}

void JQNetworkLan::refreshLanAddressEntries()
{
    lanAddressEntries_ = this->getLanAddressEntries();
}

bool JQNetworkLan::refreshUdp()
{
    udpSocketForMulticastGroupAddress_.reset( new QUdpSocket );
    udpSocketForBroadcastAddress_.reset( new QUdpSocket );

    QObject::connect(
                udpSocketForMulticastGroupAddress_.data(),
                &QUdpSocket::readyRead,
                [
                    this,
                    udpSocketForMulticastGroupAddress = udpSocketForMulticastGroupAddress_.data()
                ]()
                {
                    this->onUdpSocketReadyRead( udpSocketForMulticastGroupAddress );
                }
    );
    QObject::connect(
                udpSocketForBroadcastAddress_.data(),
                &QUdpSocket::readyRead,
                [
                    this,
                    udpSocketForBroadcastAddress = udpSocketForBroadcastAddress_.data()
                ]()
                {
                    this->onUdpSocketReadyRead( udpSocketForBroadcastAddress );
                }
    );

#ifdef Q_OS_WIN
    if ( !udpSocketForMulticastGroupAddress_->bind( QHostAddress::AnyIPv4, lanSettings_->multicastGroupAddressBindPort, QUdpSocket::ReuseAddressHint ) ) { return false; }
    if ( !udpSocketForBroadcastAddress_->bind( QHostAddress::AnyIPv4, lanSettings_->broadcastAddressBindPort, QUdpSocket::ReuseAddressHint ) ) { return false; }
#else
    if ( !udpSocketForMulticastGroupAddress_->bind( QHostAddress::AnyIPv4, lanSettings_->multicastGroupAddressBindPort, QUdpSocket::ShareAddress ) ) { return false; }
    if ( !udpSocketForBroadcastAddress_->bind( QHostAddress::AnyIPv4, lanSettings_->broadcastAddressBindPort, QUdpSocket::ShareAddress ) ) { return false; }
#endif

    if ( !udpSocketForMulticastGroupAddress_->joinMulticastGroup( lanSettings_->multicastGroupAddress ) ) { return false; }

    return true;
}

void JQNetworkLan::checkLoop()
{
    ++checkLoopCounting_;

    if ( !( checkLoopCounting_ % 9 ) )
    {
        this->refreshLanAddressEntries();
    }
    if ( checkLoopCounting_ && !( checkLoopCounting_ % 3 ) )
    {
        this->refreshUdp();
    }

    mutex_.lock();

    bool lanListModified = false;
    const auto &&currentTime = QDateTime::currentMSecsSinceEpoch();

    for ( auto it = lanNodes_.begin(); it != lanNodes_.end(); )
    {
        if ( ( currentTime - it->lastActiveTime ) >= lanSettings_->lanNodeTimeoutInterval )
        {
            const auto lanNode = *it;
            lanNodes_.erase( it );
            lanListModified = true;

            mutex_.unlock();

            this->onLanNodeStateOffline( lanNode );

            mutex_.lock();

            it = lanNodes_.begin();
        }
        else
        {
            ++it;
        }
    }

    mutex_.unlock();

    if ( lanListModified )
    {
        this->onLanNodeListChanged();
    }

    this->sendOnline();

    timerForCheckLoop_->start();
}

QByteArray JQNetworkLan::makeData(const bool &requestOffline)
{
    QVariantMap data;
    QVariantList ipList;
    for ( const auto &lanAddressEntries: lanAddressEntries_ )
    {
        ipList.push_back( lanAddressEntries.ip.toString() );
    }

    data[ "nodeMarkSummary" ] = nodeMarkSummary_;
    data[ "lastActiveTime" ] = QDateTime::currentMSecsSinceEpoch();
    data[ "ipList" ] = ipList;
    data[ "requestOffline" ] = requestOffline;
    data[ "appendData" ] = QVariant();

    return QJsonDocument( QJsonObject::fromVariantMap( data ) ).toJson( QJsonDocument::Compact );
}

void JQNetworkLan::sendOnline()
{
//    qDebug() << "JQNetworkLan::sendOnline";

    const auto &&data = this->makeData( false );

    udpSocketForMulticastGroupAddress_->writeDatagram( data, lanSettings_->multicastGroupAddress, lanSettings_->multicastGroupAddressBindPort );
    udpSocketForBroadcastAddress_->writeDatagram( data, QHostAddress( QHostAddress::Broadcast ), lanSettings_->broadcastAddressBindPort );
}

void JQNetworkLan::sendOffline()
{
//    qDebug() << "JQNetworkLan::sendOffline";

    const auto &&data = this->makeData( true );

    udpSocketForMulticastGroupAddress_->writeDatagram( data, lanSettings_->multicastGroupAddress, lanSettings_->multicastGroupAddressBindPort );
    udpSocketForBroadcastAddress_->writeDatagram( data, QHostAddress( QHostAddress::Broadcast ), lanSettings_->broadcastAddressBindPort );
}

void JQNetworkLan::onUdpSocketReadyRead(QUdpSocket *udpSocket)
{
    while ( udpSocket->hasPendingDatagrams() )
    {
        QByteArray datagram;

        datagram.resize( udpSocket->pendingDatagramSize() );
        udpSocket->readDatagram( datagram.data(), datagram.size() );

        qDebug() << "JQNetworkLan::onUdpSocketReadyRead:" << datagram;

        const auto &&data = QJsonDocument::fromJson( datagram ).object().toVariantMap();

        const auto &&nodeMarkSummary = data[ "nodeMarkSummary" ].toString();
        const auto &&lastActiveTime = data[ "lastActiveTime" ].toLongLong();
        const auto &&requestOffline = data[ "requestOffline" ].toBool();
        const auto &&appendData = data[ "appendData" ].toMap();

        if ( nodeMarkSummary.isEmpty() )
        {
            qDebug() << "JQNetworkLan::onUdpSocketReadyRead: error data1:" << datagram;
            continue;
        }

        if ( !lastActiveTime )
        {
            qDebug() << "JQNetworkLan::onUdpSocketReadyRead: error data2:" << datagram;
            continue;
        }

        QList< QHostAddress > ipList;
        for ( const auto &ip: data[ "ipList" ].toList() )
        {
            ipList.push_back( QHostAddress( ip.toString() ) );
        }

        if ( ipList.isEmpty() )
        {
            qDebug() << "JQNetworkLan::onUdpSocketReadyRead: error data3:" << datagram;
            continue;
        }

        if ( !requestOffline )
        {
            mutex_.lock();

            if ( !lanNodes_.contains( nodeMarkSummary ) )
            {
                JQNetworkLanNode lanNode;

                lanNode.nodeMarkSummary = nodeMarkSummary;
                lanNode.lastActiveTime = lastActiveTime;
                lanNode.ipList = ipList;
                lanNode.appendData = appendData;
                lanNode.matchAddress = this->matchLanAddressEntries( ipList );
                lanNode.isSelf = nodeMarkSummary == nodeMarkSummary_;

                lanNodes_[ nodeMarkSummary ] = lanNode;

                mutex_.unlock();

                this->onLanNodeStateOnline( lanNode );
                this->onLanNodeListChanged();
            }
            else
            {
                auto lanNode = lanNodes_[ nodeMarkSummary ];

                if ( lanNode.lastActiveTime < lastActiveTime )
                {
                    lanNode.lastActiveTime = lastActiveTime;
                    lanNode.ipList = ipList;
                    lanNode.appendData = appendData;
                    lanNode.matchAddress = this->matchLanAddressEntries( ipList );

                    lanNodes_[ nodeMarkSummary ] = lanNode;

                    mutex_.unlock();

                    this->onLanNodeStateActive( lanNode );
                }
                else
                {
                    mutex_.unlock();
                }
            }
        }
        else
        {
            mutex_.lock();

            if ( lanNodes_.contains( nodeMarkSummary ) )
            {
                const auto lanNode = lanNodes_[ nodeMarkSummary ];

                lanNodes_.remove( nodeMarkSummary );

                mutex_.unlock();

                this->onLanNodeStateOffline( lanNode );
                this->onLanNodeListChanged();
            }
            else
            {
                mutex_.unlock();
            }
        }
    }
}
