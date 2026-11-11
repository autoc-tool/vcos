# vcos_tests代码仓库说明

## 1. 代码仓库简介

vcos_tests是VCOS自动化测试框架及测试代码的仓库，涵盖测试框架（test_frameworks）和测试用例（testcases）两部分：

- 测试框架（test_frameworks）：包括vctest和vptest测试框架，支持上板的单元测试和上下位机端到端测试

- 测试用例（testcases）：包含C语言和Python语言的测试用例，覆盖单元测试、功能测试、集成测试等多种类型

## 2. 代码目录

vcos_tests源代码在haloosspace/vcos/tests目录下，结构如下：

```plaintext
tests
├── CMakeLists.txt
├── DISCLAIMER.md
├── LICENSE
├── README.md
├── testcases                       # 测试用例
│   ├── CMakeLists.txt
│   ├── commands                    # 测试命令行
│   ├── pytests                     # pytest测试用例
│   └── units                       # unity测试用例
└── test_frameworks                 # 自动化测试框架
    ├── CMakeLists.txt
    ├── vctest                      # vctest测试框架
    └── vptest                      # vptest测试框架
```

## 3. 仓库使用入门

### 3.1. 约束

在开发测试用例前，请确保已了解VCOS系统的开发和调试流程。参考文档如下：

| 材料名称 | 材料链接 |
| -------- | -------- |
| VCOS系统源码下载 | [源码下载流程](https://gitee.com/haloos/docs/blob/master/quick_start/01_downloading.md) |
| VCOS系统工程编译 | [工程编译流程](https://gitee.com/haloos/docs/blob/master/quick_start/02_compiling.md) |
| VCOS系统部署运行 | [部署运行流程](https://gitee.com/haloos/docs/blob/master/quick_start/03_running.md) |

### 3.2. 测试用例范围说明

| 用例范围 | 测试范围 | 测试含义 |
| -------- | -------- | ------- |
| Radius0 | 针对单个功能模块或组件的单元测试，验证最小代码单元的正确性 | 基础测试，保证模块功能可运行 |
| Radius1 | 对多个功能模块的集成测试，验证组件间的交互逻辑，覆盖端到端流程 | 综合测试，保证整体功能基本可正常使用 |
| Radius2 | 模拟用户实际使用场景，验证系统在复杂交互下的功能和性能表现 | 复杂测试，测试各种正常及异常组合场景 |
| Radius3 | 验证极端条件下的系统健壮性，如长时间运行、高负载、故障恢复等场景 | 极端测试，探索难以触及的异常组合或极限边界场景 |

### 3.3. units测试用例开发与运行

#### 3.3.1. 基本结构

units测试用例基于vctest测试框架运行，采用group->suite->testcase的层次结构进行注册与管理：

| 层次结构 | 层次含义 |
| -------- | -------- |
| group | 测试组，同一组件（component）对应的测试用例应属同一group |
| suite | 测试套，同一功能模块（module）对应的测试用例应属同一 suite |
| testcase | 测试用例，针对函数或模块功能进行测试 |

#### 3.3.2. 添加units测试用例流程

**运行约束**
units测试用例依赖配置项UNITY_FRAMEWORK。需在Kconfig的menuconfig界面勾选模块使能开关，并保存配置文件，再执行cmake编译命令生效

**注册测试group**
在units目录下，新增group对应的目录及文件：

```plaintext
units
├── CMakeLists.txt
├── group_demo                  # 新增测试group目录
│   ├── CMakeLists.txt
│   ├── Kconfig                 # 测试group对应的使能配置
│   ├── register_group_demo.c   # 测试group对应的测试suite在此文件内注册
│   ├── register_group_demo.h
│   └── suite_demo              # 测试suite对应的测试用例在此目录实现
└── register_group.c            # 测试group在此文件内注册
```

在register_group.c文件中，注册新增的测试group：

```c
void register_groups(void)
{
#if defined(CONFIG_UNIT_GROUP_YOUR_GROUP) && (CONFIG_UNIT_GROUP_YOUR_GROUP == 1)
    REGISTER_GROUP("group_demo", register_group_demo); // 注册测试group
#endif
}
```

在Kconfig中添加测试group的开关配置：

```kconfig
config UNIT_GROUP_YOUR_GROUP
    bool "unit group your group"
    default n
    ---help---
        Enable the UNIT_GROUP_YOUR_GROUP.
```

如需生效，需要在menuconfig界面勾选UNIT_GROUP_YOUR_GROUP使能开关，并保存配置文件，再执行cmake编译命令

**添加测试suite**
在register_group_demo.c中，注册suite：

```c
void register_group_demo(char *group_name) {
    add_group_to_registry(group_name, setup_group, teardown_group);
    REGISTER_SUITE(group_name, "suite_demo", register_suite_demo); // 注册测试suite
}
```

在suite_demo目录下，增加测试用例注册文件及测试用例实现文件：

```plaintext
suite_demo
├── CMakeLists.txt
├── inc
│   ├── register_suite_demo.h
│   └── test_demo.h
└── src
    ├── register_suite_demo.c   # 测试suite对应的测试用例在此文件注册
    └── test_demo.c             # 测试suite对应的测试用例在此文件实现
```

**添加测试testcase**
在register_suite_demo.c中，注册testcase：

```c
void register_suite_demo(const char *group_name, const char *suite_name) {
    test_suite_s *this_suite = add_suite_to_group(group_name, suite_name, suite_setup, suite_teardown);
    REGISTER_TESTCASE(this_suite, "test_demo_1", test_demo_01, UNIT_CASE_R0, 0); // 新增测试用例1，运行在core 0
    REGISTER_TESTCASE(this_suite, "test_demo_2", test_demo_02, UNIT_CASE_R0, 0); // 新增测试用例2，运行在core 0
}
```

在test_demo.c中，实现测试用例：

```c
void test_demo_01(void)
{
    /* 添加测试代码 */
    TEST_ASSERT_TRUE(...); // 测试断言
}
```

**常用断言函数**
全部断言函数均在unity.h中定义，常见如：

| 断言类型 | 示例 |
| -------- | ---  |
| 布尔条件 | TEST_ASSERT_TRUE(condition) |
| 指针验证 | TEST_ASSERT_NULL(pointer) |
| 值相等 | TEST_ASSERT_EQUAL_INT(expected, actual) |
| 字符串相等 | TEST_ASSERT_EQUAL_STRING(expected, actual) |

#### 3.3.3. 运行units测试用例

将镜像烧录进开发板，使用串口工具登录开发板后，通过ut命令运行units测试用例：

```shell
Usage: ut show [verbose]
       ut run [-g group_num] [-s suite_num] [-t testcase_num] [-r testcase_radius]
```

运行示例：

```shell
nsh> ut run -g 1 -s 1 -t 1

--- Running Tests ---

Group 1: group_demo
    Suite 1: suite_demo
        Test: test_demo_1 ... PASSED

Summary
-----------------------
1 Tests 0 Failures 0 Ignored
OK
```

### 3.4. pytest测试用例开发与运行

#### 3.4.1. 目录结构

```plaintext
tests
└── testcases/
    └── pytests/
        ├── pytest.ini                       # pytest设置文件
        ├── conftest.py                      # 全局前后置文件
        ├── env/                             # 测试用例依赖的环境设置
        |   ├── environment.json5            # 环境设置
        |   └── requirements.txt             # 依赖的python库
        ├── interaction/                     # 需要下位机的交互测试用例
        ├── studio/                          # 纯上位机测试用例，不依赖下位机交互
        └── vptest/                          # vptest测试框架和基础库
```

#### 3.4.2. 添加pytest测试用例

在需要测试的模块目录下添加对应的pytest测试用例文件，如：

```python
import pytest

from vptest.shell import shell

@pytest.mark.R0
class TestBasicFunctions:
    def test_hello(self):
        shell.write_shell("hello", clear_history=True)
        assert "Hello, World" in shell.read_shell(), "no reply from command 'hello'"
```

#### 3.4.3. 配置pytest运行环境

在pytest测试用例运行前，需要进行以下环境配置：

- 安装Python依赖（在pytests/env/目录下执行）：

    ```shell
    python -m pip install -r requirements.txt
    ```

- 编辑environment.json5，配置开发板串口等：

    ```json5
    {
        /* 当前的测试环境 - HW: 硬件开发板，SIM: 模拟器 */
        "RUNNING_ENV": "HW",
        /* 测试的芯片架构 */
        "TARGET": "",
        /* 测试的example工程 */
        "EXAMPLE_NAME": "tc397_demo",
        /* shell设置，PORT为串口设备名，BAUDRATE为波特率，PROMPT为shell提示符 */
        "SHELL": {
            "PORT": "COM6",
            "BAUDRATE": 115200,
            "PROMPT": "nsh> "
        }
    }
    ```

#### 3.4.4. 运行pytest测试用例

在pytests/目录下，通过pytest命令运行测试用例。例如：

```shell
pytest -m "your_marker" -k "your_keyword" <测试用例文件或路径>
```

运行示例：

```shell
> pytest -m "R0" -k "test_hello" interaction/
============================================================================== test session starts ===============================================================================
interaction/basic_tests/test_startup.py::test_hello
--------------------------------------------------------------------------------- live log setup ---------------------------------------------------------------------------------
[INFO - 17:56:06] open shell successful: COM8 115200
[INFO - 17:56:06] Shell Input:

[INFO - 17:56:06] System is up!
--------------------------------------------------------------------------------- live log call ----------------------------------------------------------------------------------
[INFO - 17:56:06]
[INFO - 17:56:06] 开始测试
[INFO - 17:56:07] Shell Input: hello
PASSED                                                                                                                                                                      [100%]
======================================================================== 1 passed, 53 deselected in 7.44s ========================================================================
```

更多pytest用法请查询：

```shell
 pytest -h
```
