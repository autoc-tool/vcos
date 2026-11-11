#
# Copyright (C) 2015-2022 Lixiang Group Holding Limited
#
"""
os模板内函数定义模块.

定义jinja2模板中所使用的函数.

用法示例:
{{get_core_id_list()}}
"""


from func_module.generator.common.common import FunctionBase
from jinja2 import pass_context

class Function(FunctionBase):
    """
    Class containing all functions. Add new static functions here
    to be included in the jinja2 context
     """
    @staticmethod
    @pass_context
    def get_bsw_callout_user_code_list(context):
        usercode_list = context["functions"]
        return usercode_list