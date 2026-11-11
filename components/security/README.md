# vcos_components_security代码仓库说明

## 1. 代码仓库简介

vcos_components_security仓库包含Crc库和E2E库内容

## 2. 代码目录

vcos_components_security源代码在haloosspace/vcos/components/security目录下，目录结构如下图所示：

```text
security
├── CMakeLists.txt          # security仓库的编译脚本，用于控制编译相关资源
├── DISCLAIMER.md           # 免责声明文档
├── Kconfig                 # security模块的Kconfig文件
├── LICENSE                 # 项目许可证文件
├── README.md               # security仓库的README，用于介绍该代码仓库
├── crc                     # CRC循环冗余校验库
├── e2e                     # E2E端到端校验库
```

## 3. 仓库使用入门

当前库中Crc库需要使用UI配置，E2E库无UI界面，需要用户自行定义接口使用的结构体。此处可以参考security_user_manual。
UI界面中配置了Crc库模块时，Crc内容即纳入编译范围。E2E库是否纳入编译无法使用UI配置，需要在user.config文件中配置。
