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

QT *= core network concurrent

CONFIG *= c++11
CONFIG *= c++14

INCLUDEPATH *= \
    $$PWD/include/

lessThan( QT_MAJOR_VERSION, 5 ) | lessThan( QT_MINOR_VERSION, 7 ) {
    error( JQNetwork request minimum Qt version is 5.7.0 )
}

JQNETWORK_BIN_DIR = $$PWD/bin/$$[QT_VERSION]/$$[QMAKE_XSPEC]/

!exists( $$JQNETWORK_BIN_DIR ) {
    mkpath( $$JQNETWORK_BIN_DIR )
}

unix | linux | mingw {
    CONFIG( debug, debug | release ) {
        JQNETWORK_LIB_FILENAME = libJQNetworkd.a
    }
    CONFIG( release, debug | release ) {
        JQNETWORK_LIB_FILENAME = libJQNetwork.a
    }
}
else: msvc {
    CONFIG( debug, debug | release ) {
        JQNETWORK_LIB_FILENAME = JQNetworkd.lib
    }
    CONFIG( release, debug | release ) {
        JQNETWORK_LIB_FILENAME = JQNetwork.lib
    }
}
else {
    error( unknow platfrom )
}

JQNETWORK_LIB_FILEPATH = $$JQNETWORK_BIN_DIR/$$JQNETWORK_LIB_FILENAME

!equals(JQNETWORK_COMPILE_MODE, SRC) {
    exists($$JQNETWORK_LIB_FILEPATH) {
        JQNETWORK_COMPILE_MODE = LIB
    }
    else {
        JQNETWORK_COMPILE_MODE = SRC
    }
}

equals(JQNETWORK_COMPILE_MODE,SRC) {

    HEADERS *= \
        $$PWD/include/*.h \
        $$PWD/include/*.inc

    SOURCES += \
        $$PWD/src/*.cpp

}
else : equals(JQNETWORK_COMPILE_MODE,LIB) {
    LIBS *= $$JQNETWORK_LIB_FILEPATH
}
else {
    error(unknow JQNETWORK_COMPILE_MODE: $$JQNETWORK_COMPILE_MODE)
}

DEFINES += JQNETWORK_COMPILE_MODE_STRING=\\\"$$JQNETWORK_COMPILE_MODE\\\"
