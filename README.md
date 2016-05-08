## 库介绍

JQNetwork，为 Jason Qt Netowrk 的简称。

JQNetwork是基于Qt开发，并且未使用除了 C++标准库 和 Qt 之外的第三方库，保证了库的可移植性以及库的纯粹性。

用到的Qt库有：

* core
* network
* concurrent	
* testlib（测试部分）

理论上本库可以部署到任何Qt支持的平台上，包括Linux、Windows、OS X

推荐使用Linux或者Unix系统，因为Qt在5.6后，更换了Unix相关系统的底层模型，从select更换为乐poll，这样子网络库的并发，就脱离了1024个的限制。

使用本库，需要 Qt5.6.0或者更高版本 以及支持 C++14 的编译器。

本库的授权协议是：随便用
可以的话，帮我点个星星，也是对我莫大的帮助。

## 功能介绍

## 模块介绍