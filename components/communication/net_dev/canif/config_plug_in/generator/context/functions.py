# This Python file uses the following encoding: utf-8
# ##############################################################################
# Copyright (c) 2025 Li Auto Inc. and its affiliates
# Licensed under the Apache License, Version 2.0(the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ##############################################################################
"""
CanIf模板内函数定义模块.
定义jinja2模板中所使用的函数.
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
    def get_canif_json_config(context):
        canif_json_config = context["canif_json"].copy()
        if "max_rx_number" in canif_json_config.keys():
            del canif_json_config["max_rx_number"]
        if "max_tx_number" in canif_json_config.keys():
            del canif_json_config["max_tx_number"]

        return canif_json_config

    @staticmethod
    @pass_context
    def get_canif_max_rx_num(context):
        canif_max_rx_num = context["canif_json"]["max_rx_number"]

        return canif_max_rx_num

    @staticmethod
    @pass_context
    def get_canif_max_tx_num(context):
        canif_max_tx_num = context["canif_json"]["max_tx_number"]

        return canif_max_tx_num

    @staticmethod
    @pass_context
    def get_canif_can_controller(context):
        can_controller_list = context["canif_json"]["can_controller"]

        return can_controller_list

    @staticmethod
    @pass_context
    def get_canif_can_controller_num(context):
        can_controller_num = len(context["canif_json"]["can_controller"])
        del context["canif_json"]["can_controller"]

        return can_controller_num

    @staticmethod
    @pass_context
    def get_cdd_name(self, cddheader):
        cdd_name_list = []
        if isinstance(cddheader, list):
            return cddheader
        else:
            cdd_name_list.append(cddheader)
        return cdd_name_list

    @staticmethod
    @pass_context
    def to_str(self, num, prefix="(uint8)"):
        num_str = prefix + str(num)
        return num_str

    @staticmethod
    @pass_context
    def to_str_noprefix(self, num):
        return str(num)
