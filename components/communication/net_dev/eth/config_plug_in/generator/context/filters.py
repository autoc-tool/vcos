#
# Copyright (C) 2015-2022 Lixiang Group Holding Limited
#
"""
bsw/os模板内过滤器定义模块.

定义jinja2模板中所使用的过滤器函数.

用法示例:
{{"#define {}_{}"|format_align("PI", "VALUE", length=10)}} 3.14

输出: "PI_VALUE  3.14"
"""




class Filter:
  """
  Class containing all filters. Add new static functions here
  to be included in the jinja2 context
  """
