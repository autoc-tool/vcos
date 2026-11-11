# vcos_apps代码仓库说明

## 1. 代码仓库简介

vcos_apps仓库是NuttX开源应用开发包仓库，提供丰富的工具、shell、网络实用程序、库和解释器等。本仓库可与NuttX RTOS/VCOS RTOS内核配合使用，扩展内核功能，支撑项目需求

智能车控OS（VCOS）根据车载，智驾等项目需求在本仓库按需扩展应用开发包

## 2. 代码目录

vcos_apps源代码在haloosspace/vcos/apps目录下，目录结构如下图所示：

```text
apps
├── Application.mk
├── audioutils      # Audio 工具
├── benchmarks      # 性能测试工具
├── boot            # MCU bootloader
├── builtin         # 内置应用脚本框架
├── canutils        # CAN 工具
├── cmake
├── CMakeLists.txt
├── config.nims     # Nim 配置文件
├── crypto          # 加解密库 e.g mbedtls
├── database        # 数据库
├── Directory.mk
├── examples        # 示例集合
├── fsutils         # 文件系统工具
├── games           # 游戏库
├── graphics        # GUI图形库
├── import          # Kernel/Protect mode 编译脚本
├── include         # 应用开发包的头文件
├── industry        # 工业级驱动/应用
├── inertial        # 飞控算法
├── interpreters    # 解释器 wasm/js/etc
├── LICENSE
├── logging         # log封装
├── lte             # LTE/NB 上层协议
├── Make.defs
├── Makefile
├── math            # 算法库
├── mlearning       # 机器学习
├── modbus          # 总线 / 软总线通信协议
├── netutils        # 网络上层工具 DHCP/telnet/curl
├── NOTICE          # 通知文件
├── nshlib          # NuttX Shell 库
├── platform        # 平台相关实现
├── README.md       # NuttX-app项目自述文件
├── README_VCOS.md  # VCOS-app项目自述文件
├── sdr             # DSP 库
├── system          # 系统能力
├── testing         # 测试框架
├── tools           # 工具
├── videoutils      # 视频工具
└── wireless        # 无线协议用户态库实现 如BLE等
```

## 3. 仓库使用入门

有关vcos_apps仓库使用入门，请参考[NuttX-app项目自述文档](./README.md)
