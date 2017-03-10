// Qt lib import
#include <QGuiApplication>
#include <QQmlApplicationEngine>

// JQNetwork lib improt
#include <JQNetwork>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    JQNetwork::printVersionInformation();

    QQmlApplicationEngine engine;

    JQNETWORKCLIENTFORQML_REGISTERTYPE( engine );

    engine.load( QUrl( QStringLiteral( "qrc:/main.qml" ) ) );

    return app.exec();
}
