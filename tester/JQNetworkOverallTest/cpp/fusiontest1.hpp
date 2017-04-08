#ifndef __CPP_FUSIONTEST1_HPP__
#define __CPP_FUSIONTEST1_HPP__

// Qt lib import
#include <QThread>

// JQNetwork lib import
#include <JQNetworkProcessor>

namespace FusionTest1
{

class UserProcessor: public JQNetworkProcessor
{
    Q_OBJECT
    Q_DISABLE_COPY( UserProcessor )

public:
    UserProcessor() = default;

    ~UserProcessor() = default;

public slots:
    bool accountLogin(const QVariantMap &received, QVariantMap &send)
    {
        qDebug() << "accountLogin:" << received;

        JQNP_CHECKRECEIVEDATACONTAINSANDNOTEMPTY( "handle", "password" );

        const auto &&handle = received[ "handle" ].toString();
        const auto &&password = received[ "password" ].toString();

        if ( ( handle != "test" ) ||
             ( password != "123456" ) )
        {
            JQNP_FAIL( "handle or password error" );
        }

        if ( received.contains( "userName" ) )
        {
            send[ "userName" ] = "Jason";
        }
        if ( received.contains( "userPhoneNumber" ) )
        {
            send[ "userPhoneNumber" ] = "18600000000";
        }

        JQNP_SUCCEED();
    }
};

class DataProcessor: public JQNetworkProcessor
{
    Q_OBJECT
    Q_DISABLE_COPY( DataProcessor )

public:
    DataProcessor() = default;

    ~DataProcessor() = default;

public slots:
    bool someRecords(const QVariantMap &received, QVariantMap &send)
    {
        qDebug() << "someRecords:" << received;

        JQNP_CHECKDATACONTAINSEXPECTEDCONTENT( "action", { "upload", "download" } );

        if ( received[ "action" ] == "upload" )
        {
            JQNP_CHECKRECEIVEDATACONTAINS( "someRecords" );

            someRecords_ = received[ "someRecords" ].toList();
        }
        else if ( received[ "action" ] == "download" )
        {
            send[ "someRecords" ] = someRecords_;
        }

        JQNP_SUCCEED();
    }

private:
    QList< QVariant > someRecords_;
};

}

#endif//__CPP_FUSIONTEST1_HPP__
