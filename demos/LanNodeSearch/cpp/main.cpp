// Qt lib import
#include <QCoreApplication>

// JQNetwork lib improt
#include <JQNetworkLan>
#include <QtNetwork>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    std::function< void() > printLanNodeList;

    // 创建配置
    JQNetworkLanSettingsSharedPointer lanSettings( new JQNetworkLanSettings );
    lanSettings->multicastGroupAddress = QHostAddress( "228.12.23.34" ); // 组播地址
    lanSettings->multicastGroupAddressBindPort = 12345; // 组播端口
    lanSettings->broadcastAddressBindPort = 23456; // 全局广播端口

    lanSettings->lanNodeListChangedCallback = [ &printLanNodeList ]() // 当节点列表发生改变时的回调
    {
        printLanNodeList();
    };

    // 根据配置创建一个局域网模块
    JQNetworkLan lan( lanSettings );

    // 给列表打印回调赋值，以供使用
    printLanNodeList = [ &lan ]()
    {
        qDebug() << "\n----- Lan node list start -----";

        for ( const auto &lanNode: lan.availableLanNodes() )
        {
            qDebug() << "lanNode: matchAddress" << lanNode.matchAddress.toString();
        }

        qDebug() << "----- Lan node list end -----";
    };

    // 初始化局域网模块
    const auto &&beginSucceed = lan.begin();
    qDebug() << "lan begin succeed:" << beginSucceed;
    if ( !beginSucceed ) { return -1; }

    return a.exec();
}
