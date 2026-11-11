# vcos_components_memory代码仓库说明

## 1. 代码仓库简介

存储栈是一个基于块（Block）的数据存储管理系统，支持MRAM、Flash、EEPROM等多种硬件存储介质。通过分层设计，实现了硬件无关的数据管理，满足了车载系统高可靠、高性能数据读写的需求。存储栈包括两大块，分别是数据存储服务层和存储驱动适配层

* **数据存储服务层**：数据存储服务层为应用提供了基础读写接口，此外还提供了数据一致性校验、冗余管理、容错管理和优先级队列等功能
* **存储驱动适配层**：存储驱动适配层用于适配不同驱动，屏蔽硬件差异，为数据存储服务层提供了统一的驱动操作接口

## 2. 代码目录

vcos_components_memory 源代码在 haloosspace/vcos/components/memory 目录下，目录结构如下图所示：

```plaintext
memory                      # 存储栈代码
├── CMakeLists.txt          # 存储子系统顶层构建配置，定义模块编译规则
├── DISCLAIMER.md           # 存储组件免责声明
├── LICENSE                 # 开源协议
├── memif                   # 存储驱动适配层，适配不同驱动
│   ├── CMakeLists.txt      # 构建规则
│   ├── config_plug_in      # 配置项生成文件
│   ├── inc                 # 头文件
│   ├── Kconfig             # memif模块使能配置
│   └── src                 # 源文件
├── nvm                     # 数据存储服务层，非易失性数据管理
│   ├── CMakeLists.txt      # 构建规则
│   ├── config_plug_in      # 配置项生成文件
│   ├── inc                 # 头文件
│   ├── Kconfig             # nvm模块使能配置
│   └── src                 # 源文件
└── README.md               # memory仓库介绍
```

## 3. 仓库使用入门

使用手册详见 [VCOS存储栈开发者手册](https://gitee.com/haloos/vcos/blob/master/user_manual/components/non_volatile_storage_user_manual.md)
