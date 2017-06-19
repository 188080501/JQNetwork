#ifndef CPP_FUSIONTEST2_HPP_
#define CPP_FUSIONTEST2_HPP_

// Qt lib import
#include <QThread>
#include <QMutex>

// JQNetwork lib import
#include <JQNetworkProcessor>
#include <JQNetworkConnect>

namespace FusionTest2
{

class ServerProcessor: public JQNetworkProcessor
{
    Q_OBJECT

public:
    ServerProcessor() = default;

    ~ServerProcessor() = default;

public slots:
    bool registerClient(const QVariantMap &received, QVariantMap &send)
    {
        JQNP_CHECKRECEIVEDDATACONTAINSANDNOT0( "clientId" );

        mutex_.lock();

        clients_[ received[ "clientId" ].toInt() ] = this->currentThreadConnect();

        mutex_.unlock();

        JQNP_SUCCEED();
    }

    bool broadcastToAll(const QVariantMap &received, QVariantMap &send)
    {
        mutex_.lock();

        for( const auto &connect: clients_ )
        {
            connect->putVariantMapData( "receivedMessage", received );
        }

        mutex_.unlock();

        JQNP_SUCCEED();
    }

    bool broadcastToOne(const QVariantMap &received, QVariantMap &send)
    {
        JQNP_CHECKRECEIVEDDATACONTAINSANDNOT0( "clientId" );

        mutex_.lock();

        clients_[ received[ "clientId" ].toInt() ]->putVariantMapData( "receivedMessage", received );

        mutex_.unlock();

        JQNP_SUCCEED();
    }

private:
    QMutex mutex_;
    QMap< int, JQNetworkConnectPointer > clients_;
};

class ClientProcessor: public JQNetworkProcessor
{
    Q_OBJECT

public:
    ClientProcessor() = default;

    ~ClientProcessor() = default;

    inline int receivedMessageCount() const { return receivedMessageCount_; }

    inline QVariantMap lastReceived() const { return lastReceived_; }

public slots:
    void receivedMessage(const QVariantMap &received)
    {
        ++receivedMessageCount_;
        lastReceived_ = received;
    }

private:
    int receivedMessageCount_ = 0;
    QVariantMap lastReceived_;
};

}

#endif//CPP_FUSIONTEST2_HPP_
