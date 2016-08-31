/*
    This file is part of JQNetwork
    
    Library introduce: https://github.com/188080501/JQNetwork

    Copyright: Jason

    Contact email: Jason@JasonServer.com
    
    GitHub: https://github.com/188080501/
*/

#ifndef JQNETWORK_INCLUDE_JQNETWORK_FOUNDATION_H
#define JQNETWORK_INCLUDE_JQNETWORK_FOUNDATION_H

// C++ lib import
#include <functional>

// Qt lib import
#include <QObject>
#include <QSharedPointer>
#include <QPointer>
#include <QString>
#include <QByteArray>
#include <QHostAddress>

#define NULLPTR_CHECK( ptr ) \
    if ( !ptr ) { qDebug( "%s: %s is null", __func__, # ptr ); return; }

class QJsonObject;
class QJsonArray;
class QJsonValue;
class QTimer;
class QThreadPool;
class QTcpSocket;
class QTcpServer;

template < class Key, class T > class QMap;

class JQNetworkPackage;
class JQNetworkConnect;
class JQNetworkConnectPool;
class JQNetworkServer;
class JQNetworkProcessor;
class JQNetworkClient;

namespace JQNetwork
{ }

#include "jqnetwork_foundation.inc"

#endif//JQNETWORK_INCLUDE_JQNETWORK_FOUNDATION_H
