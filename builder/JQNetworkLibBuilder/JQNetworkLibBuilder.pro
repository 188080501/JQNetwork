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

win32 {
    CONFIG( debug, debug | release ) {
        SOURCEFILEPATH = $$OUT_PWD/debug/$$JQNETWORK_LIB_FILENAME
    }
    else: CONFIG( release, debug | release ) {
        SOURCEFILEPATH = $$OUT_PWD/release/$$JQNETWORK_LIB_FILENAME
    }

    TARGETFILEPATH = $$JQNETWORK_LIB_FILEPATH

    SOURCEFILEPATH ~= s,/,\\,g
    TARGETFILEPATH ~= s,/,\\,g

    QMAKE_POST_LINK += copy \"$$SOURCEFILEPATH\" \"$$TARGETFILEPATH\"
}
else: unix | linux {
    QMAKE_POST_LINK += cp \"$$OUT_PWD/$$JQNETWORK_LIB_FILENAME\" \"$$JQNETWORK_LIB_FILEPATH\"
}
else {
    error( unknow platfrom )
}
