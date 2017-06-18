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
        qDebug() << "ServerProcessor::registerClient:" << received;

        JQNP_CHECKRECEIVEDDATACONTAINSANDNOT0( "clientId" );

        mutex_.lock();

        clients_[ received[ "clientId" ].toInt() ] = this->currentThreadConnect();

        mutex_.unlock();

        JQNP_SUCCEED();
    }

    bool broadcastToAll(const QVariantMap &received, QVariantMap &send)
    {
        qDebug() << "ServerProcessor::broadcastToAll:" << received;

        mutex_.lock();

        for( const auto &connect: clients_ )
        {
            connect->putVariantMapData( "receivedMessage", { { "message", "hello" } } );
        }

        mutex_.unlock();

        JQNP_SUCCEED();
    }

    void broadcastToOne(const QVariantMap &received)
    {
        qDebug() << "ServerProcessor::broadcastToOne:" << received;
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

public slots:
    void receivedMessage(const QVariantMap &received)
    {
        qDebug() << "ClientProcessor::receivedMessage:" << received;
    }
};

}

#endif//CPP_FUSIONTEST2_HPP_
