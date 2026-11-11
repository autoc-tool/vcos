# vcos_components_communication代码仓库说明

## 1. 代码仓库简介

vcos_components_communication仓库包含通用TCP/IP网络协议栈、网络接口抽象层等子模块。其中，TCP/IP网络协议栈提供TCP、UDP、ICMP、IPv4、ARP等基础协议，对上提供类套接字接口，并支持UI界面进行参数配置；网络接口层支持ETH、CAN等典型车载总线接口

## 2. 代码目录

vcos_components_communication源代码在haloosspace/vcos/components/communication目录下，目录结构如下图所示：

```text
communication
├── CMakeLists.txt          # communication仓库的编译脚本，用于控制编译相关资源
├── DISCLAIMER.md           # 免责声明文档
├── Kconfig                 # communication模块的Kconfig文件
├── LICENSE                 # 项目许可证文件
├── README.md               # communication仓库的readme，用于介绍该代码仓库(即本文)
├── cdd                     # cdd模块相关实现
├── external                # 第三方代码目录，用于存放其它开源项目代码
│   └── lwip                # lwip相关代码目录
├── net_dev                 # 驱动抽象层，对驱动资源及功能进行抽象封装，供上层模块使用
│   ├── CMakeLists.txt      # 驱动抽象层的编译脚本，用于控制编译相关资源
│   ├── canif               # can的驱动抽象层
│   ├── cantrcv1145         # can收发器1145
│   └── eth                 # 以太网的驱动抽象层
├── net_manager             # 网络总线管理面相关实现
│   ├── CMakeLists.txt      # 网络总线管理的编译脚本，用于控制编译相关资源
│   ├── cannm               # can网络管理
│   ├── cansm               # can状态管理
│   ├── comm                # 通信管理
│   └── nm                  # 网络管理
├── net_protocols           # 协议栈层的代码
│   ├── CMakeLists.txt      # 协议栈层的编译脚本，用于控制编译相关资源
│   ├── arp                 # arp协议实现
│   ├── icmp                # icmp协议实现
│   ├── ipv4                # ipv4协议实现
│   ├── sockets             # 网络编程接口实现
│   ├── tcp                 # tcp协议实现
│   └── udp                 # udp协议实现
├── pdu_router              # Pdu路由
└── rt_com                  # 实时通信模块
```

## 3. 仓库使用入门

### 3.1. ETH

以太协议栈详细功能配置说明与使用示例可参见 [以太协议栈开发者手册](https://gitee.com/haloos/vcos/blob/master/user_manual/components/ethernet_user_manual.md)

### 3.2. CAN

CAN多核部署详细功能配置说明与使用示例可参见 [CAN协议栈开发者手册](https://gitee.com/haloos/vcos/blob/master/user_manual/components/can_communicaiton_user_manual.md)
