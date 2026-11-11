# vcos_components_maintaining代码仓库说明

## 1. 代码仓库简介

vcos_components_maintaining 为系统提供了追踪、默认错误追踪器与日志功能

- **追踪（Tracing）支持如下功能：**
  - 支持后台周期性的数据采集与传输
  - 支持通过UDP协议进行数据传输
- **日志（Logging）支持如下功能：**
  - 支持日志分级与开关控制：提供OFF、DEBUG、INFO、WARNING、ERROR、FATAL 6级日志颗粒度控制
  - 支持日志路由控制：将自定义日志接口映射到syslog系统，实现跨平台日志统一采集
- **默认错误追踪器（Default Error Tracer）支持如下功能：**
  - 提供了在软件组件和其他基础软件模块开发和运行期间支持错误检测和错误跟踪的功能
  - 支持用户异常Hook动态代码的自动生成
  - 支持基于令牌捅的流控算法，以防止错误码刷屏

## 2. 代码目录

vcos_components_maintaining 源代码在 haloosspace/vcos/components/maintaining 目录下，目录结构如下图所示：

```text
maintaining                     # 维测子系统代码
├── CMakeLists.txt              # 维测子系统构建配置，定义模块编译规则
├── DISCLAIMER.md               # 维测子系统免责声明
├── LICENSE                     # 开源协议
├── README.md                   # vcos_components_maintaining 仓库介绍
├── traced                      # 跟踪模块后台服务（数据采集与传输）
│   ├── CMakeLists.txt          # 构建规则
│   ├── inc                     # 头文件
│   ├── Kconfig                 # 配置文件
│   └── src                     # 源文件
└── det                         # 日志接口层
│   └── inc                     # 日志分级控制API头文件
│   └── src                     # 源文件
│   └── config_plug_in          # 配置项生成文件
│   └── CMakeLists.txt          # 构建规则
│   └── Kconfig                 # 配置文件
└── vcos_log                    # 日志接口层
    └── inc                     # 日志分级控制API头文件
```

## 3. 仓库使用入门

使用手册详见 [VCOS维测栈开发者手册](https://gitee.com/haloos/vcos/blob/master/user_manual/components/maintaining_user_manual.md)

Trace数据可使用智能车控OS（VCOS）提供的实时性分析工具进行分析，实时性分析工具的使用手册详见 [VCOS实时性分析工具开发者手册](https://gitee.com/haloos/vcos/blob/master/user_manual/studio/real_time_analysis_tool_user_manual.md)
