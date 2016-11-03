// Qt lib import
#include <QCoreApplication>
#include <QtTest>

// Project import
#include "jqnetwork_persistenetest.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    JQNetworkPersisteneTest persisteneTest;

    persisteneTest.test();
}

