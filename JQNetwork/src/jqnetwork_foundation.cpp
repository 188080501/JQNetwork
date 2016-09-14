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
#include <QVector>

#include <QtConcurrent>

using namespace JQNetwork;

// JQNetworkThreadPoolHelper
void JQNetworkThreadPoolHelper::run(const std::function< void() > &callback)
{
    mutex_.lock();

    if ( !waitForRunCallbacks_ )
    {
        waitForRunCallbacks_ = QSharedPointer< std::vector< std::function< void() > > >( new std::vector< std::function< void() > > );
    }

    waitForRunCallbacks_->push_back( callback );

    if ( !alreadyCall_ )
    {
        alreadyCall_ = true;
        QMetaObject::invokeMethod(
                    this,
                    "onRun",
                    Qt::QueuedConnection
                );
    }

    mutex_.unlock();
}

void JQNetworkThreadPoolHelper::onRun()
{
    auto currentTime = QDateTime::currentMSecsSinceEpoch();

    if ( ( ( currentTime - lastRunTime_ ) < 5 ) && ( lastRunCallbackCount_ > 10 ) )
    {
        QThread::msleep( 5 );
    }

    QSharedPointer< std::vector< std::function< void() > > > buf;

    mutex_.lock();

    buf = waitForRunCallbacks_;
    waitForRunCallbacks_.clear();

    alreadyCall_ = false;

    lastRunTime_ = currentTime;
    lastRunCallbackCount_ = buf->size();

    mutex_.unlock();

    for ( const auto &callback: *buf )
    {
        callback();
    }
}

// JQNetworkThreadPool
JQNetworkThreadPool::JQNetworkThreadPool(const int &threadCount):
    threadPool_( new QThreadPool ),
    eventLoops_( new QVector< QPointer< QEventLoop > > ),
    helpers_( new QVector< QPointer< JQNetworkThreadPoolHelper > > )
{
    threadPool_->setMaxThreadCount( threadCount );
    eventLoops_->resize( threadCount );
    helpers_->resize( threadCount );

    QSemaphore semaphoreForThreadStart;

    for ( auto index = 0; index < threadCount; ++index )
    {
        QtConcurrent::run(
                    threadPool_.data(),
                    [
                        this,
                        index,
                        &semaphoreForThreadStart
                    ]()
                    {
                        QEventLoop eventLoop;
                        JQNetworkThreadPoolHelper helper;

                        ( *this->eventLoops_ )[ index ] = &eventLoop;
                        ( *this->helpers_ )[ index ] = &helper;

                        semaphoreForThreadStart.release( 1 );
                        eventLoop.exec();
                    }
                );
    }

    semaphoreForThreadStart.acquire( threadCount );
}

JQNetworkThreadPool::~JQNetworkThreadPool()
{
    for ( const auto &eventLoop: *eventLoops_ )
    {
        QMetaObject::invokeMethod( eventLoop.data(), "quit" );
    }

    threadPool_->waitForDone();
}

int JQNetworkThreadPool::run(const std::function< void() > &callback, const int &threadIndex)
{
    if ( threadIndex == -1 )
    {
        rotaryIndex_ = ( rotaryIndex_ + 1 ) % helpers_->size();
    }

    const auto index = ( threadIndex == -1 ) ? ( rotaryIndex_ ) : ( threadIndex );

    ( *helpers_ )[ index ]->run( callback );

    return index;
}

int JQNetworkThreadPool::waitRun(const std::function<void ()> &callback, const int &threadIndex)
{
    QSemaphore semaphore;

    auto index = this->run(
                [
                    &semaphore,
                    &callback
                ]()
                {
                    callback();
                    semaphore.release( 1 );
                },
                threadIndex
    );

    semaphore.acquire( 1 );

    return index;
}
