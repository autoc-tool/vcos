#
# Copyright (C) 2015-2022 Lixiang Group Holding Limited
#
"""
ComM 模板内函数定义模块.
定义jinja2模板中所使用的函数.
"""

import uuid


class Function:
    """
    Class containing all functions. Add new static functions here
    to be included in the jinja2 context
    """

    @staticmethod
    def gen_uuid() -> str:
        return str(uuid.uuid4())

    @staticmethod
    def get_key_name(key_name: str) -> str:
        key_name = key_name.split("_")[-1]
        return key_name

    @staticmethod
    def get_peirod(dict) -> str:
        temp =  dict["componenttypes_pkg"]["main_function"]["period"]
        return temp
