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

win32 : CONFIG( debug, debug | release ) {
    QMAKE_POST_LINK += copy $$OUT_PWD/debug/$$JQNETWORK_LIB_FILENAME $$JQNETWORK_LIB_FILEPATH
}
else : win32 : CONFIG( release, debug | release ) {
    QMAKE_POST_LINK += copy $$OUT_PWD/release/$$JQNETWORK_LIB_FILENAME $$JQNETWORK_LIB_FILEPATH
}
else {
    QMAKE_POST_LINK += cp $$OUT_PWD/$$JQNETWORK_LIB_FILENAME $$JQNETWORK_LIB_FILEPATH
}
jqnetworklib.path = $$JQNETWORK_BIN_DIR
