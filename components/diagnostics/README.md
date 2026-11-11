# vcos_components_diagnostics代码仓库说明

## 1. 代码仓库简介

vcos_components_diagnostics仓库包含诊断传输和诊断服务等子模块。其中，诊断传输提供诊断报文在CAN总线传输处理，诊断服务提供诊断通信管理，UDS服务，诊断事件管理的功能，并支持UI界面进行参数配置；

## 2. 代码目录

vcos_components_diagnostics源代码在haloosspace/vcos/components/diagnostics目录下，目录结构如下图所示：

```text
diagnostics
├── CMakeLists.txt          # diagnostics仓库的编译脚本，用于控制编译相关资源
├── DISCLAIMER.md           # 免责声明文档
├── LICENSE                 # 项目许可证文件
├── README.md               # diagnostics仓库的readme，用于介绍该代码仓库(即本文)
├── service                 # 诊断服务模块代码目录
│   ├── CMakeLists.txt      # 诊断服务模块的编译脚本，用于控制编译相关资源
│   ├── dcm                 # 诊断通信管理服务模块，用于诊断通信管理和UDS服务实现
│   └── dem                 # 诊断事件管理服务模块，用于诊断事件和DTC管理
├── transport               # 诊断传输模块代码目录
│   ├── cantp               # can诊断传输模块
│   └── CMakeLists.txt      # 诊断传输模块的编译脚本，用于控制编译相关资源
```

## 3. 仓库使用入门

### 3.1. 诊断协议栈开发者手册

诊断协议栈详细功能配置说明与使用示例可参见 [诊断协议栈开发者手册](https://gitee.com/haloos/vcos/blob/master/user_manual/components/diagnostics_user_manual.md)
