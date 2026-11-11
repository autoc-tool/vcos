# vcos_components_include代码仓库说明

## 1. 代码仓库简介

vcos_components_include是VCOS功能组件的统一接口头文件代码仓库

> 提示：VCOS并非AUTOSAR标准的完全复现，开发过程中在功能栈内部实现做了大量深度优化，因此无法做到模块级别的兼容，而是功能栈维度的兼容。功能栈接口默认通过服务映射统一为RTE（Runtime Environment）接口，相关生成代码可查看用户工程目录下的 gen_data/Rte 路径（例如：haloosspace/apps/rt_demo/system_cfg/a2g_tc397_5v_tft/gen_data/Rte）。RTE接口通过抽象隔离实现细节，降低模块耦合度，是推荐的首选调用方式。由此：本仓库接口不保证兼容AUTOSAR，仅用于开发者快速上手适配使用

## 2. 代码目录

vcos_components_include源代码在haloosspace/vcos/components/include目录下，目录结构如下图所示：

``` text
include
├── autosar                 # AUTOSAR兼容接口（有限兼容，推荐使用RTE接口）
│   ├── Compiler.h          # 编译宏相关头文件
│   ├── Os.h                # Os模块接口头文件
│   ├── Os_Types.h          # Os模块接口数据类型头文件
│   ├── Std_Types.h         # 标准数据类型头文件
│   └── ...                 # 其他AUTOSAR模块接口兼容头文件
├── nvm.h                   # VCOS存储功能栈接口头文件
├── tcpip.h                 # VCOS以太功能栈接口头文件
└── ...                     # 其他VCOS功能栈接口头文件
```

## 3. 仓库使用入门

该仓库仅用于被包含使用，API详细参考[API参考文档合集](https://gitee.com/haloos/vcos/tree/master/api_reference)

需要注意的一点是：该仓库下包含两类接口（两类接口实现功能可能会有交集，但两类接口都能分别完成全量功能）

- 类AUTOSAR接口：命名类似AUTOSAR模块（如：Os.h/Std_Types.h等）
- VCOS功能栈接口：命名与AUTOSAR模块不同，以功能栈为粒度（VCOS自建的一套标准接口，后续逐步开放。例如：nvm.h/tcpip.h等）
