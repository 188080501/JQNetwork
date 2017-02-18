// Qt lib import
#include <QGuiApplication>
#include <QQmlApplicationEngine>

// JQNetwork lib improt
#include <JQNetworkClient>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    JQNetwork::printVersionInformation();

    QQmlApplicationEngine engine;
    engine.load( QUrl( QStringLiteral( "qrc:/main.qml" ) ) );

    return app.exec();
}
