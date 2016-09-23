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

QT       += core testlib

TEMPLATE = app

include( $$PWD/../../JQNetwork/JQNetworkSrc.pri )
#include( $$PWD/../../JQNetwork/JQNetworkLib.pri )

SOURCES += \
    $$PWD/cpp/main.cpp \
    $$PWD/cpp/jqnetwork_test.cpp

HEADERS += \
    $$PWD/cpp/jqnetwork_test.h
