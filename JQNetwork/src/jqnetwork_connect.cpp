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

    JQNETWORK_NULLPTR_CHECK( onConnectCreatedCallback );
    onConnectCreatedCallback( newConnect );

    newConnect->startTimerForConnectToHostTimeOut();

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

    JQNETWORK_NULLPTR_CHECK( onConnectCreatedCallback );
    onConnectCreatedCallback( newConnect );

    newConnect->startTimerForConnectToHostTimeOut();

    newConnect->tcpSocket_->setSocketDescriptor( socketDescriptor );
}

void JQNetworkConnect::close()
{
    if ( isAbandonTcpSocket_ ) { return; }
    JQNETWORK_NULLPTR_CHECK( tcpSocket_ );

    this->onReadyToDelete();
}

qint32 JQNetworkConnect::sendPayloadData(
        const QByteArray &payloadData,
        const std::function< void(const JQNetworkConnectPointer &connect, const JQNetworkPackageSharedPointer &) > &succeedCallback,
        const std::function< void(const JQNetworkConnectPointer &connect) > &failCallback
    )
{
    JQNETWORK_NULLPTR_CHECK( runOnConnectThreadCallback_, 0 );

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
        this->reaySendPayloadData( currentRandomFlag, payloadData, succeedCallback, failCallback );
    }
    else
    {
        runOnConnectThreadCallback_(
                    [
                        this,
                        currentRandomFlag,
                        payloadData,
                        succeedCallback,
                        failCallback
                    ]()
            {
                this->reaySendPayloadData( currentRandomFlag, payloadData, succeedCallback, failCallback );
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
    JQNETWORK_NULLPTR_CHECK( runOnConnectThreadCallback_, 0 );

    if ( this->thread() == QThread::currentThread() )
    {
        this->reaySendPayloadData( randomFlag, payloadData, nullptr, nullptr );
    }
    else
    {
        runOnConnectThreadCallback_(
                    [
                        this,
                        randomFlag,
                        payloadData
                    ]()
            {
                this->reaySendPayloadData( randomFlag, payloadData, nullptr, nullptr );
            }
        );
    }

    return randomFlag;
}

void JQNetworkConnect::onTcpSocketStateChanged()
{
    if ( isAbandonTcpSocket_ ) { return; }
    JQNETWORK_NULLPTR_CHECK( tcpSocket_ );

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

            JQNETWORK_NULLPTR_CHECK( connectSettings_->connectToHostSucceedCallback );
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

                    JQNETWORK_NULLPTR_CHECK( connectSettings_->connectToHostErrorCallback );
                    connectSettings_->connectToHostErrorCallback( this );

                    break;
                }
                case QAbstractSocket::RemoteHostClosedError:
                {
                    JQNETWORK_NULLPTR_CHECK( connectSettings_->remoteHostClosedCallback );
                    connectSettings_->remoteHostClosedCallback( this );
                    break;
                }
                case QAbstractSocket::HostNotFoundError:
                case QAbstractSocket::ConnectionRefusedError:
                {
                    JQNETWORK_NULLPTR_CHECK( connectSettings_->connectToHostErrorCallback );
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

void JQNetworkConnect::onTcpSocketReadyRead()
{
    if ( isAbandonTcpSocket_ ) { return; }
    JQNETWORK_NULLPTR_CHECK( tcpSocket_ );

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
                JQNETWORK_NULLPTR_CHECK( connectSettings_->packageReceivingCallback );
                connectSettings_->packageReceivingCallback( this, package->randomFlag(), 0, package->payloadDataCurrentSize(), package->payloadDataTotalSize() );

                this->onPackageReceived( package );
            }
            else
            {
                const auto &&itForPackage = receivePackagePool_.find( package->randomFlag() );

                if ( itForPackage != receivePackagePool_.end() )
                {
                    auto payloadCurrentIndex = ( *itForPackage )->payloadDataCurrentSize();
                    JQNETWORK_NULLPTR_CHECK( connectSettings_->packageReceivingCallback );
                    connectSettings_->packageReceivingCallback( this, package->randomFlag(), payloadCurrentIndex, package->payloadDataCurrentSize(), package->payloadDataTotalSize() );

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
                    JQNETWORK_NULLPTR_CHECK( connectSettings_->packageReceivingCallback );
                    connectSettings_->packageReceivingCallback( this, package->randomFlag(), 0, package->payloadDataCurrentSize(), package->payloadDataTotalSize() );

                    receivePackagePool_[ package->randomFlag() ] = package;
                }
            }
        }
    }
}

void JQNetworkConnect::onTcpSocketConnectToHostTimeOut()
{
    if ( isAbandonTcpSocket_ ) { return; }
    JQNETWORK_NULLPTR_CHECK( timerForConnectToHostTimeOut_ );
    JQNETWORK_NULLPTR_CHECK( tcpSocket_ );

    JQNETWORK_NULLPTR_CHECK( connectSettings_->connectToHostTimeoutCallback );
    connectSettings_->connectToHostTimeoutCallback( this );

    this->onReadyToDelete();
}

void JQNetworkConnect::onSendPackageCheck()
{
//    qDebug() << "onSendPackageCheck:" << QThread::currentThread() << this->thread();

    if ( onReceivedCallbacks_.isEmpty() )
    {
        timerForSendPackageCheck_.clear();
    }
    else
    {
        const auto &&currentTime = QDateTime::currentMSecsSinceEpoch();

        auto it = onReceivedCallbacks_.begin();

        while ( ( it != onReceivedCallbacks_.end() ) &&
                ( ( currentTime - it->sendTime ) > connectSettings_->maximumReceivePackageWaitTime ) )
        {
            if ( it->failCallback )
            {
                JQNETWORK_NULLPTR_CHECK( connectSettings_->waitReplyPackageFailCallback );
                connectSettings_->waitReplyPackageFailCallback( this, it->failCallback );
            }

            onReceivedCallbacks_.erase( it );
            it = onReceivedCallbacks_.begin();
        }

        if ( !onReceivedCallbacks_.isEmpty() )
        {
            timerForSendPackageCheck_->start();
        }
    }
}

void JQNetworkConnect::startTimerForConnectToHostTimeOut()
{
    if ( timerForConnectToHostTimeOut_ )
    {
        qDebug() << "startTimerForConnectToHostTimeOut: error, timer already started";
        return;
    }

    if ( connectSettings_->maximumConnectToHostWaitTime == -1 ) { return; }

    timerForConnectToHostTimeOut_.reset( new QTimer );
    connect( timerForConnectToHostTimeOut_.data(), &QTimer::timeout,
             this, &JQNetworkConnect::onTcpSocketConnectToHostTimeOut,
             Qt::DirectConnection );

    timerForConnectToHostTimeOut_->setSingleShot( true );
    timerForConnectToHostTimeOut_->start( connectSettings_->maximumConnectToHostWaitTime );
}

void JQNetworkConnect::startTimerForSendPackageCheck()
{
    if ( timerForSendPackageCheck_ )
    {
        qDebug() << "startTimerForSendPackageCheck: error, timer already started";
        return;
    }

    if ( connectSettings_->maximumSendPackageWaitTime == -1 ) { return; }

    timerForSendPackageCheck_.reset( new QTimer );
    connect( timerForSendPackageCheck_.data(), &QTimer::timeout,
             this, &JQNetworkConnect::onSendPackageCheck,
             Qt::DirectConnection );

    timerForSendPackageCheck_->setSingleShot( true );
    timerForSendPackageCheck_->start( 1000 );
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
            JQNETWORK_NULLPTR_CHECK( connectSettings_->waitReplyPackageSucceedCallback );
            connectSettings_->waitReplyPackageSucceedCallback( this, package, it->succeedCallback );
        }

        onReceivedCallbacks_.erase( it );
    }
    else
    {
        JQNETWORK_NULLPTR_CHECK( connectSettings_->packageReceivedCallback );
        connectSettings_->packageReceivedCallback( this, package );
    }
}

void JQNetworkConnect::onTcpSocketBytesWritten(const qint64 &bytes)
{
    if ( isAbandonTcpSocket_ ) { return; }
    JQNETWORK_NULLPTR_CHECK( tcpSocket_ );

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

    JQNETWORK_NULLPTR_CHECK( tcpSocket_ );
    tcpSocket_->close();

    JQNETWORK_NULLPTR_CHECK( connectSettings_->readyToDeleteCallback );
    connectSettings_->readyToDeleteCallback( this );
}

void JQNetworkConnect::reaySendPayloadData(
        const qint32 &randomFlag,
        const QByteArray &payloadData,
        const std::function< void(const JQNetworkConnectPointer &connect, const JQNetworkPackageSharedPointer &) > &succeedCallback,
        const std::function< void(const JQNetworkConnectPointer &connect) > &failCallback
    )
{
    if ( isAbandonTcpSocket_ ) { return; }
    JQNETWORK_NULLPTR_CHECK( tcpSocket_ );

    auto packages = JQNetworkPackage::createPackagesFromPayloadData(
                payloadData,
                randomFlag,
                connectSettings_->cutPackageSize
            );
    if ( packages.isEmpty() )
    {
        qDebug() << "JQNetworkConnect::reaySendPayloadData: createPackagesFromPayloadData error";
        return;
    }

    auto firstPackage = packages.first();
    packages.pop_front();

    const auto &&buffer = firstPackage->toByteArray();

    waitForSendBytes_ += buffer.size();
    tcpSocket_->write( buffer );

    if ( succeedCallback || failCallback )
    {
        onReceivedCallbacks_[ randomFlag ] =
        {
            QDateTime::currentMSecsSinceEpoch(),
            succeedCallback,
            failCallback
        };

        if ( !timerForSendPackageCheck_ )
        {
            this->startTimerForSendPackageCheck();
        }
    }

    if ( !packages.isEmpty() )
    {
        sendPackagePool_[ randomFlag ] = packages;
    }

    JQNETWORK_NULLPTR_CHECK( connectSettings_->packageSendingCallback );
    connectSettings_->packageSendingCallback( this, randomFlag, 0, firstPackage->payloadDataCurrentSize(), firstPackage->payloadDataTotalSize() );
}
