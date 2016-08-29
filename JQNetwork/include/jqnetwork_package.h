/*
    This file is part of JQNetwork
    
    Library introduce: https://github.com/188080501/JQNetwork

    Copyright: Jason

    Contact email: Jason@JasonServer.com
    
    GitHub: https://github.com/188080501/
*/

#ifndef JQNETWORK_INCLUDE_JQNETWORK_PACKAGE_H
#define JQNETWORK_INCLUDE_JQNETWORK_PACKAGE_H

// JQNetwork lib import
#include <JQNetworkFoundation>

namespace JQNetwork
{ }

class JQNetworkPackage: public QObject
{
    Q_OBJECT

public:
    JQNetworkPackage() = default;

    ~JQNetworkPackage() = default;

    JQNetworkPackage(const JQNetworkPackage &) = delete;

    JQNetworkPackage &operator =(const JQNetworkPackage &) = delete;
};

#include "jqnetwork_package.inc"

#endif//JQNETWORK_INCLUDE_JQNETWORK_PACKAGE_H
