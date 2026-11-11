# apps_rt_demo代码仓库说明

## 1. 代码仓库简介

apps_rt_demo仓库是周期任务确定性优化demo的相关文件，包括应用文件及配置工程，配置工程支持E3650平台和TC397平台。周期任务是通过调度表进行激活的，但在实际运行过程中，由于多任务之间的相互影响，周期会有一定的波动，在硬实时系统中，这种任务周期波动需要严格控制在确定的范围之内。rt_demo中展示了如何通过vcos工具中的抖动自动优化功能提升周期任务确定性。抖动优化的原理是根据实际的性能测试数据，利用遗传算法自动寻找最优的任务激活时间偏移量，使多个任务激活时间离散化，避免了在某个时刻需要激活多个任务时，高优先级任务对低优先级的影响，从而使任务的周期更稳定

## 2. 代码目录

apps_rt_demo 源代码在 haloosspace/apps/rt_demo 目录下，目录结构如下图所示：

```text
rt_demo
├── README.md                      # 项目说明文档，包含入门指南和使用说明
├── CMakeLists.txt                 # 项目主构建脚本，定义编译选项和目标生成规则
├── apps                           # 应用程序源代码目录
│   ├── test_stubs.c               # Demo应用程序相关文件
|   └── ...
└── system_cfg                     # 平台配置目录
    ├── e3650_dev_kit              # e3650平台特定配置
    │   ├── config                 # 配置文件目录
    │   ├── gen_data               # 生成的代码文件目录
    │   ├── rt_demo.vcosproject    # e3650演示项目文件
    │   └── vcos_project.xml       # VCOS项目配置文件
    └── a2g_tc397_5v_tft           # tc397平台特定配置
        ├── config                 # 配置文件目录
        ├── gen_data               # 生成的代码文件目录
        ├── rt_demo.vcosproject    # tc397演示项目文件
        └── vcos_project.xml       # VCOS项目配置文件
```

## 3. 仓库使用入门

使用手册详见 [关键周期任务确定性方案实践](https://gitee.com/haloos/docs/blob/master/tech_cases/key_periodic_task_real_time_practice.md)

## 4. 备注

rt_demo仅支持真实硬件板子上运行（E3650和TC397的Demo板），暂不支持SIM仿真环境下运行