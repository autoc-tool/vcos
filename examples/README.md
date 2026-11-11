# vcos_examples代码仓库说明

## 1. 代码仓库简介

vcos_examples是一个综合性的示例代码仓库，包含了VCOS在不同应用场景下的完整演示项目。本仓库旨在帮助开发者快速理解和掌握 VCOS 的核心功能特性，包括车载通信、实时任务调度、网络管理、安全隔离、多核通信、诊断服务等关键技术领域。每个示例项目都提供了完整的源代码、配置文件和详细的使用说明，支持多种硬件平台（E3650、TC397等），为车载软件开发提供了丰富的参考实现和最佳实践

## 2. 代码目录结构

vcos_examples源代码位于 `haloosspace/vcos/examples` 目录下，整体目录结构如下：

```text
examples/
├── README.md                   # 本仓库说明文档
├── CMakeLists.txt              # 主构建脚本，定义全局编译选项
├── vbslite_demo                # VBSLite的演示Demo
├── rt_demo/                    # 实时任务确定性优化演示Demo
├── isolate_demo/               # 应用安全隔离演示Demo
├── multicore_com_demo/         # 多核通信演示Demo
├── nm_demo/                    # 网络管理演示Demo
├── mempool_demo/               # 内存池共享演示Demo
├── diag_demo/                  # 诊断功能演示Demo
└── ...                         # 其他示例Demo
```

## 3. 仓库使用入门

使用手册详见 [VCOS内核开发者手册](https://gitee.com/haloos/vcos/blob/master/user_manual/kernel/kernel_user_manual.md)