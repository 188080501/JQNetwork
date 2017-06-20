#ifndef CPP_PROCESSORTEST2_HPP_
#define CPP_PROCESSORTEST2_HPP_

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
            mutex_.lock();
            ++counter_[ "receivedByteArray" ];
            mutex_.unlock();
        }
    }

    void receivedByteArraySendByteArray(const QByteArray &received, QByteArray &send)
    {
        if ( received == "test" )
        {
            mutex_.lock();
            ++counter_[ "receivedByteArraySendByteArray" ];
            mutex_.unlock();
        }
        send = "test";
    }

    void receivedByteArraySendVariantMap(const QByteArray &received, QVariantMap &send)
    {
        if ( received == "test" )
        {
            mutex_.lock();
            ++counter_[ "receivedByteArraySendVariantMap" ];
            mutex_.unlock();
        }
        send = QVariantMap( { { "key", "value" } } );
    }

    void receivedByteArraySendFile(const QByteArray &received, QFileInfo &send)
    {
        if ( received == "test" )
        {
            mutex_.lock();
            ++counter_[ "receivedByteArraySendFile" ];
            mutex_.unlock();
        }
        send = QFileInfo( this->testFileInfo( 3 ) );
    }

    void receivedVariantMap(const QVariantMap &received)
    {
        if ( received == QVariantMap( { { "key", "value" } } ) )
        {
            mutex_.lock();
            ++counter_[ "receivedVariantMap" ];
            mutex_.unlock();
        }
    }

    void receivedVariantMapSendByteArray(const QVariantMap &received, QByteArray &send)
    {
        if ( received == QVariantMap( { { "key", "value" } } ) )
        {
            mutex_.lock();
            ++counter_[ "receivedVariantMapSendByteArray" ];
            mutex_.unlock();
        }
        send = "test";
    }

    void receivedVariantMapSendVariantMap(const QVariantMap &received, QVariantMap &send)
    {
        if ( received == QVariantMap( { { "key", "value" } } ) )
        {
            mutex_.lock();
            ++counter_[ "receivedVariantMapSendVariantMap" ];
            mutex_.unlock();
        }
        send = QVariantMap( { { "key", "value" } } );
    }

    void receivedVariantMapSendFile(const QVariantMap &received, QFileInfo &send)
    {
        if ( received == QVariantMap( { { "key", "value" } } ) )
        {
            mutex_.lock();
            ++counter_[ "receivedVariantMapSendFile" ];
            mutex_.unlock();
        }
        send = QFileInfo( this->testFileInfo( 3 ) );
    }

    void receiveFile(const QFileInfo &received)
    {
        if ( received == this->testFileInfo( 2 ) )
        {
            mutex_.lock();
            ++counter_[ "receiveFile" ];
            mutex_.unlock();
        }
    }

    void receiveFileSendByteArray(const QFileInfo &received, QByteArray &send)
    {
        if ( received == this->testFileInfo( 2 ) )
        {
            mutex_.lock();
            ++counter_[ "receiveFileSendByteArray" ];
            mutex_.unlock();
        }
        send = "test";
    }

    void receiveFileSendVariantMap(const QFileInfo &received, QVariantMap &send)
    {
        if ( received == this->testFileInfo( 2 ) )
        {
            mutex_.lock();
            ++counter_[ "receiveFileSendVariantMap" ];
            mutex_.unlock();
        }
        send = QVariantMap( { { "key", "value" } } );
    }

    void receiveFileSendFile(const QFileInfo &received, QFileInfo &send)
    {
        if ( received == this->testFileInfo( 2 ) )
        {
            mutex_.lock();
            ++counter_[ "receiveFileSendFile" ];
            mutex_.unlock();
        }
        send = QFileInfo( this->testFileInfo( 3 ) );
    }

    void receivedVariantMapAndAppendSendVariantMapAndAppend(
            const QVariantMap &received,
            QVariantMap &send,
            const QVariantMap &receivedAppend,
            QVariantMap &sendAppend
        )
    {
        if ( ( received == QVariantMap( { { "key2", "value2" } } ) ) &&
             ( receivedAppend == QVariantMap( { { "key3", "value3" } } ) ) )
        {
            mutex_.lock();
            ++counter_[ "receivedVariantMapAndAppendSendVariantMapAndAppend" ];
            mutex_.unlock();
        }

        send = QVariantMap( { { "key", "value" } } );
        sendAppend = QVariantMap( { { "key2", "value2" } } );
    }

public:
    QMutex mutex_;
    QMap< QString, int > counter_; // methodName -> count
};

}

#endif//CPP_PROCESSORTEST2_HPP_
