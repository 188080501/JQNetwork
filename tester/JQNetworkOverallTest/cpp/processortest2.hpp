#ifndef __CPP_PROCESSORTEST2_HPP__
#define __CPP_PROCESSORTEST2_HPP__

// Qt lib import
#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>
#include <QDir>

// JQNetwork lib import
#include <JQNetworkProcessor>

namespace ProcessorTest2
{

class TestProcessor: public JQNetworkProcessor
{
    Q_OBJECT
    Q_DISABLE_COPY( TestProcessor )

public:
    TestProcessor() = default;

    ~TestProcessor() = default;

    static QFileInfo testFileInfo(const int &index)
    {
        return QString( "%1/JQNetworkOverallTest/myprocessor2_testfile%2" ).arg( QStandardPaths::writableLocation( QStandardPaths::TempLocation ), QString::number( index ) );
    }

    static bool createTestFile(const int &index)
    {
        QDir().mkpath( QString( "%1/JQNetworkOverallTest" ).arg( QStandardPaths::writableLocation( QStandardPaths::TempLocation ) ) );

        QFile file( testFileInfo( index ).filePath() );
        if ( !file.open( QIODevice::WriteOnly ) ) { return false; }

        file.resize( 4 );
        file.write( "test" );
        file.waitForBytesWritten( 5000 );

        return true;
    }

public slots:
    void receivedByteArray(const QByteArray &received)
    {
        if ( received == "test" )
        {
            ++counter_[ "receivedByteArray" ];
        }
    }

    void receivedByteArraySendByteArray(const QByteArray &received, QByteArray &send)
    {
        if ( received == "test" )
        {
            ++counter_[ "receivedByteArraySendByteArray" ];
        }
        send = "test";
    }

    void receivedByteArraySendVariantMap(const QByteArray &received, QVariantMap &send)
    {
        if ( received == "test" )
        {
            ++counter_[ "receivedByteArraySendVariantMap" ];
        }
        send = QVariantMap( { { "key", "value" } } );
    }

    void receivedByteArraySendFile(const QByteArray &received, QFileInfo &send)
    {
        if ( received == "test" )
        {
            ++counter_[ "receivedByteArraySendFile" ];
        }
        send = QFileInfo( this->testFileInfo( 3 ) );
    }

    void receivedVariantMap(const QVariantMap &received)
    {
        if ( received == QVariantMap( { { "key", "value" } } ) )
        {
            ++counter_[ "receivedVariantMap" ];
        }
    }

    void receivedVariantMapSendByteArray(const QVariantMap &received, QByteArray &send)
    {
        if ( received == QVariantMap( { { "key", "value" } } ) )
        {
            ++counter_[ "receivedVariantMapSendByteArray" ];
        }
        send = "test";
    }

    void receivedVariantMapSendVariantMap(const QVariantMap &received, QVariantMap &send)
    {
        if ( received == QVariantMap( { { "key", "value" } } ) )
        {
            ++counter_[ "receivedVariantMapSendVariantMap" ];
        }
        send = QVariantMap( { { "key", "value" } } );
    }

    void receivedVariantMapSendFile(const QVariantMap &received, QFileInfo &send)
    {
        if ( received == QVariantMap( { { "key", "value" } } ) )
        {
            ++counter_[ "receivedVariantMapSendFile" ];
        }
        send = QFileInfo( this->testFileInfo( 3 ) );
    }

    void receiveFile(const QFileInfo &received)
    {
        if ( received == this->testFileInfo( 2 ) )
        {
            ++counter_[ "receiveFile" ];
        }
    }

    void receiveFileSendByteArray(const QFileInfo &received, QByteArray &send)
    {
        if ( received == this->testFileInfo( 2 ) )
        {
            ++counter_[ "receiveFileSendByteArray" ];
        }
        send = "test";
    }

    void receiveFileSendVariantMap(const QFileInfo &received, QVariantMap &send)
    {
        if ( received == this->testFileInfo( 2 ) )
        {
            ++counter_[ "receiveFileSendVariantMap" ];
        }
        send = QVariantMap( { { "key", "value" } } );
    }

    void receiveFileSendFile(const QFileInfo &received, QFileInfo &send)
    {
        if ( received == this->testFileInfo( 2 ) )
        {
            ++counter_[ "receiveFileSendFile" ];
        }
        send = QFileInfo( this->testFileInfo( 3 ) );
    }

public:
    QMap< QString, int > counter_; // methodName -> count
};

}

#endif//__CPP_PROCESSORTEST2_HPP__
