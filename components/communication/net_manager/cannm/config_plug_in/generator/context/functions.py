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
os模板内函数定义模块.

定义jinja2模板中所使用的函数.

用法示例:
{{get_core_id_list()}}
"""

from jinja2 import pass_context
from func_module.generator.common.common import FunctionBase
from components.communication.net_manager.cannm.config_plug_in.generator.filter import (
    CanNmCoreChannelMappingDict,
    CanNmMainFunctionCoreIdDict,
    CanNmMainFunctionPeriodDict,
    CanNmChannelIdDict,
)


class Function(FunctionBase):
    """
    Class containing all functions. Add new static functions here
    to be included in the jinja2 context
    """
    @staticmethod
    @pass_context
    def get_cannm_global_config(context):
        cannm_global_config = context["CanNmConfig"]["CanNmGlobalConfig"]
        return cannm_global_config

    @staticmethod
    @pass_context
    def get_cannm_pn_info_config(context):
        cannm_pn_info_config = context["CanNmConfig"]["CanNmPnInfo"]
        return cannm_pn_info_config

    @staticmethod
    @pass_context
    def get_cannm_pn_filter_mask_byte_config(context):
        cannm_pn_filter_mask_byte_config = context["CanNmConfig"]["CanNmPnInfo"]["CanNmPnFilterMaskByte"]
        return cannm_pn_filter_mask_byte_config

    @staticmethod
    @pass_context
    def get_cannm_channel_config(context):
        cannm_channel_config = context["CanNmConfig"]["CanNmChannelConfig"]
        return cannm_channel_config

    @staticmethod
    @pass_context
    def get_cannm_channel_rx_pdu_config(context):
        cannm_channel_config = context["CanNmConfig"]["CanNmChannelConfig"]
        cannm_channel_count = 0
        cannm_rxpdu_count = 0
        cannm_channel_rx_pdu_config = {}
        for channel_key, channel_value in cannm_channel_config.items():
            cannm_channel_rx_pdu_config[cannm_channel_count] = {}
            for rx_pdu_key, rx_pdu_value in channel_value.items():
                if isinstance(rx_pdu_value, dict):
                    for rxpdukey, rxpduvalue in rx_pdu_value.items():
                        cannm_channel_rx_pdu_config[cannm_channel_count][cannm_rxpdu_count] = {}
                        if rxpdukey == "CanNmRxPduRef":
                                cannm_channel_rx_pdu_config[cannm_channel_count][cannm_rxpdu_count]= cannm_channel_config[channel_key][rx_pdu_key][rxpdukey]
                                cannm_rxpdu_count = cannm_rxpdu_count + 1
            cannm_channel_count = cannm_channel_count + 1
            cannm_rxpdu_count = 0
        return cannm_channel_rx_pdu_config

    @staticmethod
    @pass_context
    def get_cannm_multicore_or_not(context):
        cannm_multicore = "STD_OFF"
        cannm_core_values = list(CanNmCoreChannelMappingDict.values())
        for value in cannm_core_values:
            for value1 in cannm_core_values:
                if value != value1:
                    cannm_multicore = "STD_ON"
                    break
        return cannm_multicore

    @staticmethod
    @pass_context
    def get_cannm_single_coreid(context):
        cannm_single_coreid = 0
        cannm_core_values = list(CanNmCoreChannelMappingDict.values())

        for value in cannm_core_values:
            for value1 in cannm_core_values:
                if value != value1:
                    break
                else:
                    cannm_single_coreid = value

        if {} in CanNmCoreChannelMappingDict.values():
            cannm_single_coreid = 0

        return cannm_single_coreid

    @staticmethod
    @pass_context
    def get_cannm_core_include_channel_num(context, core_index):
        CanNmCoreKey = f"CANNM_CORE{core_index}_INCLUDE_CHANNEL_NUM"
        return CanNmCoreKey

    @staticmethod
    @pass_context
    def get_cannm_core_include_channel_num_dict(context):
        CanNmCoreNum = 4
        CanNmCoreNumIndex = 0
        CanNmCoreIncludeChannelNumDict = {}
        for cannmindex in range(CanNmCoreNum):
            for key, value in CanNmCoreChannelMappingDict.items():
                if (value == cannmindex):
                    CanNmCoreNumIndex = CanNmCoreNumIndex + 1
            CanNmCoreIncludeChannelNumDict[cannmindex] = CanNmCoreNumIndex
            CanNmCoreNumIndex = 0

        return CanNmCoreIncludeChannelNumDict

    @staticmethod
    @pass_context
    def get_cannm_core_dict(context):
        CanNmCoreNum = 4
        CanNmCoreDict = {}
        for i in range(CanNmCoreNum):
            CanNmCoreDict[i] = f"CANNM_CORE{i}_ID"
        return CanNmCoreDict

    @staticmethod
    @pass_context
    def get_cannm_channel_mapping_mainfunction_period_dict(context):
        cannm_coreid_mapping_period_dict = {}
        cannm_channel_mapping_mainfunction_period_dict = {}
        for index in range(len(CanNmMainFunctionCoreIdDict)):
            core_id = CanNmMainFunctionCoreIdDict[index]
            period = CanNmMainFunctionPeriodDict[index]
            cannm_coreid_mapping_period_dict[core_id] = period
        if len(cannm_coreid_mapping_period_dict) == 0:
            cannm_coreid_mapping_period_dict[0] = 5
        for key, value in CanNmCoreChannelMappingDict.items():
            cannm_channel_mapping_mainfunction_period_dict[key] = cannm_coreid_mapping_period_dict[value]
        return cannm_channel_mapping_mainfunction_period_dict

    @staticmethod
    @pass_context
    def get_cannm_channel_id_dict(context):
        return CanNmChannelIdDict

