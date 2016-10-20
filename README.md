## 介绍

JQNetwork，为 Jason Qt Netowrk 的简称

JQNetwork是基于Qt开发，并且未使用除了 C++标准库 和 Qt 之外的第三方库，保证了库的可移植性以及库的纯粹性

所有的传输数据的地方，都使用了TCP长连接，保证了传输数据的可靠性

#### 用到的Qt库有：

* core
* network
* concurrent	
* testlib（测试用，运行不需要）

理论上可以部署到任何Qt支持的平台上。

推荐使用Linux系统或者Unix系统，因为在5.7后，Qt更换了Unix相关系统的底层模型，从select更换为了poll，这样改进后，并发就脱离了1024个的限制。

使用本库，推荐 Qt5.7.0 或者更高版本，以及支持 C++14 的编译器，对操作系统无要求。

本库源码均已开源在了GitHub上。

GitHub地址：https://github.com/188080501/JQNetwork

方便的话，帮我点个星星，或者反馈一下使用意见，这是对我莫大的帮助。

若你已经有了更好的建议，或者想要一些新功能，可以直接邮件我，我的邮箱是：Jason@JasonServer.com

或者直接在GitHub上提交问题：
https://github.com/188080501/JQNetwork/issues

## 功能

功能|状态|加入版本
---|---|---
TCP长连接|已实现|V0.1
全双工|已实现|V0.1
全异步|已实现|V0.1
多线程|已实现|V0.1
大数据量支持|已实现|V0.2
小数据包插队传输|已实现|V0.2
数据压缩|已实现|V0.3
局域网自组网|已实现|V0.4
大文件传输支持|开发中|V0.5
数据中转|等待开发|V0.6
SSL支持|等待开发|V1.0

## 性能介绍

本库性能只能说一般般，底层是poll注定了性能不是强项，以下是我在我电脑（ iMac(i5-4670) + 127.0.0.1 ）上，测出的性能。

* 1个TCP长连接（127.0.0.1）向服务端连续发送 4Byte 数据，服务端连续返回 4Byte 数据，每秒可以达 36569 次

* 1个TCP长连接（127.0.0.1）向服务端连续发送 32KB 数据，服务端连续返回 32KB 数据，每秒可以达 3430 次，单向载荷传输速度可以达 856 Mbit/s

* 1个TCP长连接（127.0.0.1）与服务端来回发送(PingPong) 4Byte 数据，每秒可以达 13559 次

* 1个TCP长连接（127.0.0.1）与服务端来回发送(PingPong) 32KB 数据，每秒可以达 9723 次，单向载荷传输速度可以达 2424 Mbit/s

* 1个TCP长连接（127.0.0.1）发送 512MB 数据到服务端，耗时 1111 毫秒，单向载荷传输速度可以达 3686 Mbit/s

## 模块介绍

* JQNetworkFoundation

	基础库

* JQNetworkEncrypt

	加密库（私有加密，可选）

* JQNetworkPackage

	数据包

* JQNetworkConnect

	连接对象

* JQNetworkConnectPool

	连接池

* JQNetworkServer

	服务端

* JQNetworkProcessor

	服务端处理器（处理数据）

* JQNetworkClient

	客户端

* JQNetworkForwarf

	转发器

* JQNetworkLan

	局域网功能