# vcos_components代码仓库说明

## 1. 代码仓库简介

vcos_components为VCOS针对车辆控制域软件需求和规范提供的系统功能组件，旨在满足智能网联汽车的端到端确定性实时可靠调度、低时延可靠通信和路由、诊断标定、功能安全、可维可测、信息安全等需求

主要提供以下功能栈：

- 网络通信栈：主要提供CAN、ETH通信、路由和网络管理功能
- 诊断标定栈：主要提供车载UDS、OBD诊断功能和XCP标定功能
- 存储服务栈：主要提供KV、Block存储等功能
- 可维可测栈：主要提供在线GDB、LOG等功能
- 实时调度框架：主要提供跨多ECU系统的确定性硬实时调度功能
- 实时运行环境：主要提供应用间及应用和系统功能组件、OS间的通信接口、应用软件的调度管理等功能
- 以及系统状态管理和OTA升级等功能

## 2. 代码目录

vcos_components源代码在haloosspace/vcos/components目录下，目录结构如下图所示：

```text
components                      # VCOS针对车辆控制域软件需求和规范提供的系统功能组件
├── CMakeLists.txt              # vcos_components系统功能组件编译脚本
├── README.md                   # vcos_components系统功能组件说明文档（即本文）
├── communication               # 通信功能栈
│   ├── CMakeLists.txt          # communication功能栈编译脚本
│   ├── README.md               # communication功能栈说明文档
│   ├── cdd                     # 复杂驱动模块，支持通信PDU通知到应用层或复杂驱动
│   ├── external                # 通信功能栈外部依赖
│   ├── net_dev                 # 网络设备驱动模块
│   ├── net_manager             # 网络管理模块
│   ├── net_protocols           # 网络协议模块
│   ├── pdu_router              # PDU路由模块
│   └── rt_com                  # communication信号处理模块
├── configs                     # vcos_components相关配置
│   ├── README.md               # 配置说明文档
│   └── configurator_plug_in    # vcos_studio/configurator插件配置
├── fusa                        # 功能安全栈
│   ├── CMakeLists.txt          # 功能安全栈编译脚本
│   └── isolate                 # 轻量级安全隔离功能模块
├── include                     # vcos_components内部公共头文件
│   ├── autosar_os              # 适配autosar os的接口
│   ├── bswm.h                  # 系统组件管理模块接口
│   ├── comstack_cfg.h          # 通信栈类型配置
│   ├── comstack_types.h        # 通信栈类型定义
│   ├── memmap.h                # 内存映射头文件
│   ├── nvm.h                   # 适配nvm模块的接口 
│   ├── sysm_init.h             # 系统管理模块接口
│   ├── tcpip.h                 # 适配tcpip模块的接口
│   └── traced.h                # 适配traced模块的接口
├── maintaining                 # 可维可测栈    
│   ├── CMakeLists.txt          # 可维可测栈编译脚本
│   ├── README.md               # 可维可测栈说明文档
│   ├── det                     # det功能模块
│   ├── traced                  # trace功能模块
│   └── vcos_log                # vcos_log功能模块
├── memory                      # 存储服务栈
│   ├── CMakeLists.txt          # 存储服务栈编译脚本
│   ├── README.md               # 存储服务栈说明文档
│   ├── memif                   # memif功能模块
│   ├── memmap                  # 内存映射模块
│   ├── mempool                 # 共享内存池模块
│   └── nvm                     # nvm功能模块
├── rt_framework                # 实时调度框架
│   ├── CMakeLists.txt          # 实时调度框架编译脚本
│   ├── Kconfig                 # 实时调度框架kconfig配置项
│   ├── README.md               # 实时调度框架说明文档
│   ├── config_plug_in          # 实时调度框架configurator配置插件
│   ├── inc                     # 实时调度框架内部头文件
│   └── src                     # 实时调度框架源码
└── runtime                     # 运行时环境
│   ├── EcuC                    # EcuC功能模块
│   ├── README.md               # 运行时环境说明文档
│   └── Rte                     # Rte功能模块
├── security                    # 信息安全功能栈
│   ├── CMakeLists.txt          # 信息安全功能模块编译脚本
│   ├── crc                     # crc功能模块
│   └── e2e                     # e2e功能模块
└── system_services             # 系统服务功能模块
    ├── CMakeLists.txt          # 系统服务功能模块编译脚本
    └── service_management      # 系统管理功能模块
```

## 3. 仓库使用入门

对于支持configurator工具配置的功能模块，需要在configurator工具添加该模块并进行配置、校验通过并生成动态代码后，才会在生成动态代码的CMakeLists.txt中打开该模块的编译开关。configurator工具的使用请参考[配置工具开发者手册](https://gitee.com/haloos/vcos/blob/master/user_manual/studio/configurator_tool_user_manual.md)

对于不支持configurator工具配置的功能模块，如components/maintaining/vcos_log模块，需要在kconfig的menuconfig界面勾选模块使能开关，并保存kconfig配置文件，再执行cmake编译命令才能打开模块的编译开关

本仓库所包含的功能栈及对应的用户手册如下：

| 功能栈 |  开发者手册 |
| :---------- | :------------- |
| **实时调度框架** | [实时调度框架开发者手册](https://gitee.com/haloos/vcos/blob/master/user_manual/kernel/kernel_user_manual.md) / [实时调度框架配置项说明](https://gitee.com/haloos/vcos/blob/master/configuration_reference/os_cfg.md) |
| **以太通信栈** | [以太通信栈开发者手册](https://gitee.com/haloos/vcos/blob/master/user_manual/components/ethernet_user_manual.md) / [EthIf模块配置项说明](https://gitee.com/haloos/vcos/blob/master/configuration_reference/ethif_cfg.md) 、[TcpIp模块配置项说明](https://gitee.com/haloos/vcos/blob/master/configuration_reference/tcpip_cfg.md) |
| **CAN通信栈** | [CAN通信栈开发者手册](https://gitee.com/haloos/vcos/blob/master/user_manual/components/can_communication_user_manual.md)  |
| **网络管理** | [网络管理开发者手册](https://gitee.com/haloos/vcos/blob/master/user_manual/components/net_manager_user_manual.md)  |
| **存储服务栈** | [存储服务栈开发者手册](https://gitee.com/haloos/vcos/blob/master/user_manual/components/non_volatile_storage_user_manual.md)、[内存池开发者手册](https://gitee.com/haloos/vcos/blob/master/user_manual/components/mempool_user_manual.md)、[内存映射用户手册](https://gitee.com/haloos/vcos/blob/master/user_manual/components/memmap_user_manual.md) / [NvM模块配置项说明](https://gitee.com/haloos/vcos/blob/master/configuration_reference/nvm_cfg.md)、[MemIf模块配置项说明](https://gitee.com/haloos/vcos/blob/master/configuration_reference/memif_cfg.md)、[mempool模块配置项说明](https://gitee.com/haloos/vcos/blob/master/configuration_reference/mempool_cfg.md)、[memmap模块配置项说明](https://gitee.com/haloos/vcos/blob/master/configuration_reference/memmap_cfg.md) |
| **实时运行环境** | [Rte模块配置项说明](https://gitee.com/haloos/vcos/blob/master/configuration_reference/rte_cfg.md)、[EcuC模块配置项说明](https://gitee.com/haloos/vcos/blob/master/configuration_reference/ecuc_cfg.md) |
| **功能安全栈** | [轻量级安全隔离功能模块开发者手册](https://gitee.com/haloos/vcos/blob/master/user_manual/components/isolate_user_manual.md) / [isolate模块配置项说明](https://gitee.com/haloos/vcos/blob/master/configuration_reference/isolate_cfg.md) |
| **可维可测** | [实时性分析工具开发者手册](https://gitee.com/haloos/vcos/blob/master/user_manual/studio/real_time_analysis_tool_user_manual.md) |
| **系统服务** | [系统管理开发者手册](https://gitee.com/haloos/vcos/blob/master/user_manual/components/system_manager_user_manual.md) / [系统组件管理模块配置项说明](https://gitee.com/haloos/vcos/blob/master/configuration_reference/sysm_core_cfg.md)、[系统管理模块配置项说明](https://gitee.com/haloos/vcos/blob/master/configuration_reference/sysm_init_cfg.md) |
