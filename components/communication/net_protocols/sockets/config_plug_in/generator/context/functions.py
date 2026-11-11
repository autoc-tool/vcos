#
# Copyright (C) 2015-2022 Lixiang Group Holding Limited
#
"""
os模板内函数定义模块.

定义jinja2模板中所使用的函数.

用法示例:
{{get_core_id_list()}}
"""

from jinja2 import pass_context
from func_module.generator.common.common import FunctionBase


class Function(FunctionBase):
    """
    Class containing all functions. Add new static functions here
    to be included in the jinja2 context
    """

    @staticmethod
    @pass_context
    def get_json_config(context):
        json_config = context["json"]
        return json_config

    @staticmethod
    @pass_context
    def get_TcpIpConfig(context):
        config = context["json"]["TcpIpConfig"]
        return config

    @staticmethod
    @pass_context
    def get_TcpIpGeneral(context):
        config = context["json"]["TcpIpGeneral"]
        return config

    @staticmethod
    @pass_context
    def get_TcpIpCoreIdList(context):
        config = context["json"]["TcpIpCoreidList"]
        return config

    @staticmethod
    @pass_context
    def convert_to_zero_and_one(self, context):
        if (context):
            return 1
        else:
            return 0

    @staticmethod
    @pass_context
    def convert_to_on_or_off(self, context):
        if (context):
            return 'STD_ON'
        else:
            return 'STD_OFF'

    @staticmethod
    @pass_context
    def convert_to_ms_from_s(self, context: int):
        return context * 1000

    @staticmethod
    @pass_context
    def list_or_null(self, list_context, name):
        if (len(list_context)):
            return name
        else:
            return 'NULL'

    @staticmethod
    @pass_context
    def dict_or_null(self, dict_context, key_name, name):
        if key_name in dict_context:
            return name
        else:
            return 'NULL'

    @staticmethod
    @pass_context
    def add_index(self, name, index):
        return name + str(index)
