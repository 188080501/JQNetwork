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

推荐使用Linux系统或者Unix系统，因为在5.7后，Qt更换了Unix相关系统的底层模型，从select更换为了poll，这样子网络库的并发就脱离了1024个的限制。

使用本库，推荐 Qt5.7.0 或者更高版本，以及支持 C++14 的编译器，对操作系统无要求。

本库源码均已开源在了GitHub上。

GitHub地址：https://github.com/188080501/JQNetwork

本库的授权协议是：随便用

方便的话，帮我点个星星，或者反馈一下使用意见，这是对我莫大的帮助。

若你已经有了更好的建议，或者想要一些新功能，可以直接邮件我，我的邮箱是：Jason@JasonServer.com

或者直接在GitHub上提交问题：
https://github.com/188080501/JQNetwork/issues

## 功能介绍

JQNetwork，从TCP开始封装的网络库，目的是为了让Qt开发者更加方便的开发出更加高效、稳定的C/S架构网络程序。

#### 本网络库特性：

* TCP长连接
* 前后端分离（支持服务器前后端分离）
* 全双工
* 全异步
* 多线程（收发、处理分线程）
* 数据中转
* 数据压缩
* 大数据量支持
* 局域网自组网
* 小数据包插队传输
* SSL支持

## 开发计划

阶段|计划完成日期
---|---
功能确定|2016-08
架构确定|2016-08
开始开发|2016-08
V0.1|2016-09

## 性能介绍

本库性能只能说一般般，底层是poll注定了性能不是强项，以下是我在我电脑（ iMac(i5-4670) + 127.0.0.1 ）上，测出的性能。

* TCP短连接（127.0.0.1）每次发送 128B 的数据并且返回 128B 的数据，每秒可以达 x 次

* TCP长连接（127.0.0.1）可以达到 x 个

* 1个TCP长连接（127.0.0.1）与服务器来回发送（PingPong） 128B 数据，每秒可以达 x 次；以 32KB 计，每秒可以达 x 次。

* 1个TCP长连接（127.0.0.1）发送 512MB 数据到服务器，耗时 x 毫秒

* 5000个TCP长连接（127.0.0.1），每个连接每 500ms 向服务器发送 128B 数据，CPU 使用率在 x%

## 模块介绍

* JQNetworkFoundation

* JQNetworkEncrypt

* JQNetworkPackage

* JQNetworkConnect

* JQNetworkConnectPool

* JQNetworkServer

* JQNetworkProcessor

* JQNetworkClient

* JQNetworkForwarf