# 系统管理栈开发者手册

## 1. 简介

系统管理栈是一组轻量级的系统管理程序，为系统各项服务提供灵活、高效的多级启动能力。系统管理支持对系统中各服务状态的管理与同步，有效协调各服务间的状态依赖，能够显著提升整个系统的确定性与可用性，为复杂系统的稳定运行奠定坚实基础

## 2. 代码目录

service_management源代码在haloosspace/vcos/components/system_services/service_management目录下，目录结构如下图所示：

```plaintext
service_management          # 系统管理栈代码
├── CMakeLists.txt          # 系统管理子系统顶层构建配置，定义模块编译规则
├── DISCLAIMER.md           # 系统管理组件免责声明
├── LICENSE                 # 开源协议
├── sysm_init               # 系统管理-启动管理
│   ├── CMakeLists.txt      # 构建规则
│   ├── config_plug_in      # 配置项生成文件
│   ├── inc                 # 头文件
│   ├── Kconfig             # sysm_init模块使能配置
│   └── src                 # 源文件
├── sysm_core               # 系统管理-核心功能
│   ├── CMakeLists.txt      # 构建规则
│   ├── config_plug_in      # 配置项生成文件
│   ├── inc                 # 头文件
│   ├── Kconfig             # sysm_core模块使能配置
│   └── src                 # 源文件
└── README.md               # service_management仓库介绍

```

## 3.仓库使用入门

使用手册详见 [VCOS系统管理栈开发者手册](https://gitee.com/haloos/vcos/blob/master/user_manual/components/system_manager_user_manual.md)
