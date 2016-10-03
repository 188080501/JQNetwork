/*
    This file is part of JQNetwork
    
    Library introduce: https://github.com/188080501/JQNetwork

    Copyright: Jason

    Contact email: Jason@JasonServer.com
    
    GitHub: https://github.com/188080501/
*/

#include "jqnetwork_package.h"

// Qt lib import
#include <QDebug>

#define BOOL_CHECK( actual, message )           \
    if ( !( actual ) )                          \
    {                                           \
        qDebug() << __func__ << message;        \
        this->isAbandonPackage_ = true;         \
        mixPackage->isAbandonPackage_ = true;   \
        return false;                           \
    }

qint32 JQNetworkPackage::checkDataIsReadyReceive(const QByteArray &rawData)
{
    /*
     * Return value:
     * > 0: Wait for more byte
     * < 0: Error data, need to abandon
     * = 0: Data is ready for receive
     */

    if ( rawData.size() < JQNetworkPackage::headSize() ) { return JQNetworkPackage::headSize() - rawData.size(); }

    auto head = ( Head * )rawData.data();
    auto dataSize = rawData.size() - JQNetworkPackage::headSize();

    if ( head->bootFlag_ != JQNETWORKPACKAGE_BOOTFLAG ) { return -1; }
    switch ( head->packageFlag_ )
    {
        case JQNETWORKPACKAGE_DATATRANSPORTPACKGEFLAG:
        case JQNETWORKPACKAGE_DATAREQUESTPACKGEFLAG: { break; }
        default: { return -1; }
    }
    if ( head->randomFlag_ == 0 ) { return -1; }

    switch ( head->metaDataFlag_ )
    {
        case JQNETWORKPACKAGE_UNCOMPRESSED:
        case JQNETWORKPACKAGE_COMPRESSED: { break; }
        default: { return -1; }
    }
    if ( head->metaDataTotalSize_ < -1 ) { return -1; }
    if ( head->metaDataCurrentSize_ < -1 ) { return -1; }
    if ( head->metaDataTotalSize_ < head->metaDataCurrentSize_ ) { return -1; }

    switch ( head->payloadDataFlag_ )
    {
        case JQNETWORKPACKAGE_UNCOMPRESSED:
        case JQNETWORKPACKAGE_COMPRESSED: { break; }
        default: { return -1; }
    }
    if ( head->payloadDataTotalSize_ < -1 ) { return -1; }
    if ( head->payloadDataCurrentSize_ < -1 ) { return -1; }
    if ( head->payloadDataTotalSize_ < head->payloadDataCurrentSize_ ) { return -1; }

    auto expectDataSize = 0;
    if ( head->metaDataCurrentSize_ > 0 )
    {
        expectDataSize += head->metaDataCurrentSize_;
    }
    if ( head->payloadDataCurrentSize_ > 0 )
    {
        expectDataSize += head->payloadDataCurrentSize_;
    }

    if ( dataSize < expectDataSize ) { return expectDataSize - dataSize; }

    return 0;
}

JQNetworkPackageSharedPointer JQNetworkPackage::createPackage(QByteArray &rawData)
{
    auto package = JQNetworkPackageSharedPointer( new JQNetworkPackage );
    auto head = ( Head * )rawData.data();
    auto data = rawData.data() + JQNetworkPackage::headSize();

    package->head_ = *head;

    if ( package->metaDataCurrentSize() > 0 )
    {
        package->metaData_.append( data, package->metaDataCurrentSize() );
        data += package->metaDataCurrentSize();
    }
    if ( package->payloadDataCurrentSize() > 0 )
    {
        package->payloadData_.append( data, package->payloadDataCurrentSize() );
        data += package->payloadDataCurrentSize();
    }

    rawData.remove( 0, data - rawData.data() );

    package->refreshPackage();

    return package;
}

QList< JQNetworkPackageSharedPointer > JQNetworkPackage::createTransportPackages(
        const QByteArray &payloadData,
        const qint32 &randomFlag,
        const qint64 cutPackageSize
    )
{
    QList< JQNetworkPackageSharedPointer > result;

    for ( auto index = 0; index < payloadData.size(); )
    {
        auto package = JQNetworkPackageSharedPointer( new JQNetworkPackage );

        package->head_.bootFlag_ = JQNETWORKPACKAGE_BOOTFLAG;
        package->head_.packageFlag_ = JQNETWORKPACKAGE_DATATRANSPORTPACKGEFLAG;
        package->head_.randomFlag_ = randomFlag;

        package->head_.metaDataFlag_ = JQNETWORKPACKAGE_UNCOMPRESSED;

        package->head_.payloadDataFlag_ = JQNETWORKPACKAGE_UNCOMPRESSED;
        package->head_.payloadDataTotalSize_ = payloadData.size();

        if ( cutPackageSize == -1 )
        {
            package->head_.payloadDataCurrentSize_ = payloadData.size();
            package->payloadData_ = payloadData;
            package->isCompletePackage_ = true;

            index = payloadData.size();
        }
        else
        {
            if ( ( index + cutPackageSize ) > payloadData.size() )
            {
                package->payloadData_ = payloadData.mid( index );
                package->head_.payloadDataCurrentSize_ = package->payloadData_.size();
                package->isCompletePackage_ = result.isEmpty();

                index = payloadData.size();
            }
            else
            {
                package->head_.payloadDataCurrentSize_ = cutPackageSize;
                package->payloadData_ = payloadData.mid( index, cutPackageSize );
                package->isCompletePackage_ = !index && ( ( index + cutPackageSize ) == payloadData.size() );

                index += cutPackageSize;
            }
        }

        result.push_back( package );
    }

    return result;
}

JQNetworkPackageSharedPointer JQNetworkPackage::createRequestPackage(const qint32 &randomFlag)
{
    auto package = JQNetworkPackageSharedPointer( new JQNetworkPackage );

    package->head_.bootFlag_ = JQNETWORKPACKAGE_BOOTFLAG;
    package->head_.packageFlag_ = JQNETWORKPACKAGE_DATAREQUESTPACKGEFLAG;
    package->head_.randomFlag_ = randomFlag;

    package->head_.metaDataFlag_ = JQNETWORKPACKAGE_UNCOMPRESSED;

    package->head_.payloadDataFlag_ = JQNETWORKPACKAGE_UNCOMPRESSED;

    return package;
}

bool JQNetworkPackage::mixPackage(const JQNetworkPackageSharedPointer &mixPackage)
{
    BOOL_CHECK( !this->isCompletePackage(), ": current package is complete" );
    BOOL_CHECK( !mixPackage->isCompletePackage(), ": mix package is complete" );
    BOOL_CHECK( !this->isAbandonPackage(), ": current package is abandon package" );
    BOOL_CHECK( !mixPackage->isAbandonPackage(), ": mix package is abandon package" );
    BOOL_CHECK( this->randomFlag() == mixPackage->randomFlag(), ": randomFlag not same" );

    BOOL_CHECK( this->metaDataTotalSize() == mixPackage->metaDataTotalSize(), ": metaDataTotalSize not same" );
    BOOL_CHECK( ( this->metaDataCurrentSize() + mixPackage->metaDataCurrentSize() ) <= this->metaDataTotalSize(),
                ": metaDataCurrentSize overmuch" );

    BOOL_CHECK( this->payloadDataTotalSize() == mixPackage->payloadDataTotalSize(), ": payloadDataTotalSize not same" );
    BOOL_CHECK( ( this->payloadDataCurrentSize() + mixPackage->payloadDataCurrentSize() ) <= this->payloadDataTotalSize(),
                ": payloadDataCurrentSize overmuch" );

    BOOL_CHECK( ( ( this->metaDataTotalSize() > 0 ) || ( this->payloadDataTotalSize() > 0 ) ), ": data error" );

    if ( this->metaDataTotalSize() > 0 )
    {
        this->metaData_.append( mixPackage->metaData() );
        this->head_.metaDataCurrentSize_ += mixPackage->metaDataCurrentSize();
    }
    if ( this->payloadDataTotalSize() > 0 )
    {
        this->payloadData_.append( mixPackage->payloadData() );
        this->head_.payloadDataCurrentSize_ += mixPackage->payloadDataCurrentSize();
    }

    this->refreshPackage();

    return true;
}
