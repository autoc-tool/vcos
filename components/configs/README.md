# vcos_components_configs代码仓库说明

## 1. 代码仓库简介

configurator 配置工具为 vcos_components 系统功能组件提供了插件化且灵活可扩展的配置支持。该工具支持功能栈的 UI 可视化配置、参数校验及动态代码生成。
每个功能栈都可以基于 configurator 所提供的插件模板开发自有插件，并将插件信息配置在 vcos_components_configs/configurator_plug_in 目录下。
configurator 工具会根据上述目录下的配置信息，自动加载相关插件，实现对各功能栈统一的配置、校验和动态代码生成

此外，configurator工具还提供了二级菜单界面配置功能，以及配置项联动功能，为各功能模块提供丰富的易用性配置功能支持
主要提供以下功能：

- components版本管理
- 各芯片平台属性配置
- 各功能模块插件信息配置
- 各芯片平台驱动UI配置支持
- 二级菜单界面配置
- 配置项联动规则配置

## 2. 代码目录

vcos_components_configs在haloosspace/vcos/components/configs目录下，目录结构如下图所示：

```text
configs                                 # vcos_components系统功能组件配置
├── README.md                           # vcos_components系统功能组件配置说明文档（即本文）
└── configurator_plug_in                # configurator工具提供的插件配置目录
    ├── compatibility.json              # 兼容性配置文件，包含components的版本及兼容的版本信息
    ├── components_plug_in_cfg.json     # components各功能模块插件信息配置文件
    ├── components_version.py           # components版本管理模块，支持components版本更新和读取
    ├── linkage_cfg.json                # 配置项联动规则配置文件
    ├── platform_cfg                    # 各芯片平台相关的属性配置目录，包含芯片驱动配置选项和代码生成支持，以及芯片平台特殊的功能模块属性配置
    ├── plugin_template                 # 插件模板目录
    └── system_secondary                # 二级菜单界面配置
```

## 3. 仓库使用入门

configurator架构支持vcos_components功能栈各模块UI配置、校验和代码生成插件化，并在plugin_template 目录中定义了标准接口和目录结构，供各功能模块插件开发参考和使用

各功能模块需要在各自config_plug_in中添加如下内容：

- UI 配置所需的 ARXML 文件
- 校验规则配置和实现代码
- 代码生成功能接口
- jinja2 模板

并在 configs/configurator_plug_in/components_plug_in_cfg.json 中配置模块属性，包括：

- 所属 group（功能分组）
- 依赖的模块列表（用于添加模块、校验和代码生成时的自动关联）
- 是否支持 UI 配置
- 是否支持校验功能
- 是否支持动态代码生成
- 是否支持 internal behavior 及 ib arxml 的动态更新
- 该模块的 config_plug_in 路径
