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

using namespace JQNetwork;

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
    if ( head->versionFlag_ != JQNETWORKPACKAGE_VERSION ) { return -1; }
    if ( head->randomFlag_ == 0 ) { return -1; }

    if ( head->metaDataFlag_ == 0 ) { return -1; }
    if ( head->metaDataTotalSize_ < -1 ) { return -1; }
    if ( head->metaDataCurrentSize_ < -1 ) { return -1; }
    if ( head->metaDataTotalSize_ < head->metaDataCurrentSize_ ) { return -1; }

    if ( head->payloadDataFlag_ == 0 ) { return -1; }
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

JQNetworkPackageSharedPointer JQNetworkPackage::createPackageFromRawData(QByteArray &rawData)
{
    auto newPackage = JQNetworkPackageSharedPointer( new JQNetworkPackage );
    auto head = ( Head * )rawData.data();
    auto data = rawData.data() + JQNetworkPackage::headSize();

    newPackage->head_ = *head;

    if ( newPackage->metaDataCurrentSize() > 0 )
    {
        newPackage->metaData_.append( data, newPackage->metaDataCurrentSize() );
        data += newPackage->metaDataCurrentSize();
    }
    if ( newPackage->payloadDataCurrentSize() > 0 )
    {
        newPackage->payloadData_.append( data, newPackage->payloadDataCurrentSize() );
        data += newPackage->payloadDataCurrentSize();
    }

    rawData.remove( 0, data - rawData.data() );

    newPackage->refreshPackage();

    return newPackage;
}

JQNetworkPackageSharedPointer JQNetworkPackage::createPackageFromPayloadData(const QByteArray &payloadData, const qint32 &randomFlag)
{
    auto newPackage = JQNetworkPackageSharedPointer( new JQNetworkPackage );

    newPackage->head_.bootFlag_ = JQNETWORKPACKAGE_BOOTFLAG;
    newPackage->head_.versionFlag_ = JQNETWORKPACKAGE_VERSION;
    newPackage->head_.randomFlag_ = randomFlag;

    newPackage->head_.payloadDataFlag_ = 0x1;
    newPackage->head_.payloadDataTotalSize_ = payloadData.size();
    newPackage->head_.payloadDataCurrentSize_ = payloadData.size();

    newPackage->payloadData_ = payloadData;

    newPackage->isCompletePackage_ = true;

    return newPackage;
}

bool JQNetworkPackage::mixPackage(const JQNetworkPackageSharedPointer &mixPackage)
{
    BOOL_CHECK( !this->isCompletePackage(), ": current package is complete" );
    BOOL_CHECK( !mixPackage->isCompletePackage(), ": mix package is complete" );
    BOOL_CHECK( !this->isAbandonPackage(), ": current package is abandon package" );
    BOOL_CHECK( !mixPackage->isAbandonPackage(), ": mix package is abandon package" );
    BOOL_CHECK( this->randomFlag() == mixPackage->randomFlag(), ": randomFlag not same" );

    BOOL_CHECK( this->metaDataTotalSize() == mixPackage->metaDataTotalSize(), ": metaDataTotalSize not same" );
    BOOL_CHECK( ( this->metaDataCurrentSize() + mixPackage->metaDataCurrentSize() ) <= this->metaDataTotalSize(), ": metaDataCurrentSize overmuch" );

    BOOL_CHECK( this->payloadDataTotalSize() == mixPackage->payloadDataTotalSize(), ": payloadDataTotalSize not same" );
    BOOL_CHECK( ( this->payloadDataCurrentSize() + mixPackage->payloadDataCurrentSize() ) <= this->payloadDataTotalSize(), ": payloadDataCurrentSize overmuch" );

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
