#
#   This file is part of JQNetwork
#
#   Library introduce: https://github.com/188080501/JQNetwork
#
#   Copyright: Jason
#
#   Contact email: Jason@JasonServer.com
#
#   GitHub: https://github.com/188080501/
#

QT       += core

TEMPLATE = lib

CONFIG += staticlib

CONFIG( debug, debug | release ) {
    TARGET = JQNetworkd
}

CONFIG( release, debug | release ) {
    TARGET = JQNetwork
}

JQNETWORK_COMPILE_MODE = SRC
include( $$PWD/../../JQNetwork/JQNetwork.pri )

DESTDIR = $$JQNETWORK_BIN_DIR
