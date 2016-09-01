/*
    This file is part of JQNetwork
    
    Library introduce: https://github.com/188080501/JQNetwork

    Copyright: Jason

    Contact email: Jason@JasonServer.com
    
    GitHub: https://github.com/188080501/
*/

#include "jqnetwork_foundation.h"

// Qt lib import
#include <QDebug>
#include <QThreadPool>
#include <QSemaphore>

#include <QtConcurrent>

using namespace JQNetwork;

// JQNetworkThreadPool
JQNetworkThreadPool::JQNetworkThreadPool(const int &threadCount):
    threadPool_( new QThreadPool ),
    eventLoops_( new QVector< QPointer< QEventLoop > > ),
    helpers_( new QVector< QPointer< JQNetworkThreadPoolHelper > > )
{
    threadPool_->setMaxThreadCount( threadCount );
    eventLoops_->resize( threadCount );
    helpers_->resize( threadCount );

    static bool flag = true;
    if ( flag )
    {
        flag = false;
        qRegisterMetaType< std::function< void() > >( "std::function<void()>" );
    }

    QSemaphore semaphore;

    for ( auto index = 0; index < threadCount; ++index )
    {
        QtConcurrent::run(
                    threadPool_.data(),
                    [ this, index, &semaphore ](){
                        QEventLoop eventLoop;
                        JQNetworkThreadPoolHelper helper;

                        ( *this->eventLoops_ )[ index ] = &eventLoop;
                        ( *this->helpers_ )[ index ] = &helper;

                        semaphore.release( 1 );
                        eventLoop.exec();
                    }
                );
    }

    semaphore.acquire( threadCount );
}

JQNetworkThreadPool::~JQNetworkThreadPool()
{
    for ( const auto &eventLoop: *eventLoops_ )
    {
        QMetaObject::invokeMethod( eventLoop.data(), "quit" );
    }

    threadPool_->waitForDone();
}

void JQNetworkThreadPool::run(const std::function< void() > &callback)
{
    rotaryIndex_ = ( rotaryIndex_ + 1 ) % helpers_->size();

    QMetaObject::invokeMethod(
                ( *helpers_ )[ rotaryIndex_ ].data(),
                "run",
                Qt::QueuedConnection,
                Q_ARG( std::function< void() >, callback )
            );
}
