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
cantp_ib_bswm更新处理
"""

from basic_func_module.autosar_utils.utils import get_short_name, get_definition_ref, get_value
from func_module.module_management.arxml_handler.ib_bswmd_process import ib_bswmd_utils


def update_ib_bswmd(internal_behavior_dict, ecuc_autosar_dict, asw_autosar_dict, system_autosar_dict, other_input_info):
    cantp_ib = internal_behavior_dict.get("CanTp", None)
    if cantp_ib is None:
        return

    cantp_autosar = ecuc_autosar_dict.get("CanTp", None)
    if cantp_autosar is None:
        return

    cantpmainfunction_base = cantp_autosar.find("/ActiveEcuC/CanTp/CanTpConfig")
    if cantpmainfunction_base.SUB_CONTAINERS is None:
        return
    cantpmainfunction_list = cantpmainfunction_base.SUB_CONTAINERS.ECUC_CONTAINER_VALUE

    cantpmainfunction_num = 0
    for cantpmainfunction in cantpmainfunction_list:
        if get_definition_ref(cantpmainfunction).split("/")[-1] == "CanTpMainFunction":
            cantpmainfunction_num = cantpmainfunction_num + 1
# 查找/AUTOSAR/EcucDefs/CanTp/CanTpConfiguration/CanTpMainFunction  计数


    # 删除
    bsw_module_entry_ref_list = cantp_ib.find_nodes("BSW-MODULE-ENTRY-REF-CONDITIONAL")
    for bsw_module_entry_ref in bsw_module_entry_ref_list:
        entry_ref_name = getattr(bsw_module_entry_ref.BSW_MODULE_ENTRY_REF, "valueOf_", "").split("/")[-1]
        ib_bswmd_utils.delete_ib_node(cantp_ib, "BSW-MODULE-ENTRY-REF-CONDITIONAL", entry_ref_name)
# 为什么只删除entry_ref_name = Mainfunction_Core3?

    timing_event_list = cantp_ib.find_nodes("BSW-TIMING-EVENT")
    for timing_event in timing_event_list:
        timing_event_name = get_short_name(timing_event)
        ib_bswmd_utils.delete_ib_container(cantp_ib, "BSW-TIMING-EVENT", timing_event_name)
# 只删除timing_event_name = CanTp_MainFunction_Core3_TimingEvent0？

    bsw_schedulable_entity_list = cantp_ib.find_nodes("BSW-SCHEDULABLE-ENTITY")
    for bsw_schedulable_entity in bsw_schedulable_entity_list:
        entity_name = get_short_name(bsw_schedulable_entity)
        ib_bswmd_utils.delete_ib_container(cantp_ib, "BSW-SCHEDULABLE-ENTITY", entity_name)
# 删除entity_name = 'CanTp_MainFunction_Core3'CanTp_MainFunction_Core0 CanTp_MainFunction_Core1 CanTp_MainFunction_Core2

    bsw_module_entry_list = cantp_ib.find_nodes("BSW-MODULE-ENTRY")
    for bsw_module_entry in bsw_module_entry_list:
        entry_name = get_short_name(bsw_module_entry)
        ib_bswmd_utils.delete_ib_container(cantp_ib, "BSW-MODULE-ENTRY", entry_name)
# 走不到

    # 没有CanTpMainFunction容器，按照默认的CanTpMainFunction
    if cantpmainfunction_num == 0:
        period = 0.005
        ib_bswmd_utils.create_bsw_module_entry_ref_conditional(cantp_ib, "CanTp", "CanTp_MainFunction")
        ib_bswmd_utils.create_bsw_timing_event(cantp_ib, "CanTp", "CanTp_MainFunction" + "TimingEvent0",
                                            "CanTp_MainFunction", float(period))
        ib_bswmd_utils.create_bsw_schedulable_entity(cantp_ib, "CanTp", "CanTp_MainFunction", "CanTp_MainFunction")
        ib_bswmd_utils.create_bsw_module_entry(cantp_ib, "CanTp", "CanTp_MainFunction")

    # 存在CanTpMainFunction容器，按照CanTpMainFunction容器下主函数taskmapping
    else:
        # 新增并写入配置的周期时间
        mainfunc_num = 0
        for cantpmainfunction in cantpmainfunction_list:
            if get_definition_ref(cantpmainfunction).split("/")[-1] == "CanTpMainFunction":
                mainfunction_name = get_short_name(cantpmainfunction)
                value_list = ib_bswmd_utils.get_parameter_values(cantpmainfunction) # MainFunctionPeriod的值 0.005 ？
                period = get_value(value_list[0])
                ib_bswmd_utils.create_bsw_module_entry_ref_conditional(cantp_ib, "CanTp", mainfunction_name)
                ib_bswmd_utils.create_bsw_timing_event(cantp_ib, "CanTp", mainfunction_name + "TimingEvent" + str(mainfunc_num),
                                                    mainfunction_name, float(period))
                ib_bswmd_utils.create_bsw_schedulable_entity(cantp_ib, "CanTp", mainfunction_name, mainfunction_name)
                ib_bswmd_utils.create_bsw_module_entry(cantp_ib, "CanTp", mainfunction_name)
