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
cannm_ib_bswm更新处理
"""

from basic_func_module.autosar_utils.utils import get_definition_ref, get_value, get_short_name
from func_module.module_management.arxml_handler.ib_bswmd_process import ib_bswmd_utils


def update_ib_bswmd(internal_behavior_dict, ecuc_autosar_dict, asw_autosar_dict, system_autosar_dict, other_input_info):
    cannm_ib = internal_behavior_dict.get("CanNm", None)
    if cannm_ib is None:
        return

    cannm_autosar = ecuc_autosar_dict.get("CanNm", None)
    if cannm_autosar is None:
        return

    cannmmainfunction_base = cannm_autosar.find("/ActiveEcuC/CanNm/CanNmGlobalConfig")
    if cannmmainfunction_base.SUB_CONTAINERS is None:
        return
    cannmmainfunction_list = cannmmainfunction_base.SUB_CONTAINERS.ECUC_CONTAINER_VALUE
    
    cannmmainfunction_num = 0
    for cannmmainfunction in cannmmainfunction_list:
        if get_definition_ref(cannmmainfunction).split("/")[-1] == "CanNmMainFunction":
            cannmmainfunction_num = cannmmainfunction_num + 1

    # 删除默认的CanNm_MainFunction的taskmapping
    bsw_module_entry_ref_list = cannm_ib.find_nodes("BSW-MODULE-ENTRY-REF-CONDITIONAL")
    for bsw_module_entry_ref in bsw_module_entry_ref_list:
        entry_ref_name = getattr(bsw_module_entry_ref.BSW_MODULE_ENTRY_REF, "valueOf_", "").split("/")[-1]
        ib_bswmd_utils.delete_ib_node(cannm_ib, "BSW-MODULE-ENTRY-REF-CONDITIONAL", entry_ref_name)

    timing_event_list = cannm_ib.find_nodes("BSW-TIMING-EVENT")
    for timing_event in timing_event_list:
        timing_event_name = get_short_name(timing_event)
        ib_bswmd_utils.delete_ib_container(cannm_ib, "BSW-TIMING-EVENT", timing_event_name)

    bsw_schedulable_entity_list = cannm_ib.find_nodes("BSW-SCHEDULABLE-ENTITY")
    for bsw_schedulable_entity in bsw_schedulable_entity_list:
        entity_name = get_short_name(bsw_schedulable_entity)
        ib_bswmd_utils.delete_ib_container(cannm_ib, "BSW-SCHEDULABLE-ENTITY", entity_name)

    bsw_module_entry_list = cannm_ib.find_nodes("BSW-MODULE-ENTRY")
    for bsw_module_entry in bsw_module_entry_list:
        entry_name = get_short_name(bsw_module_entry)
        ib_bswmd_utils.delete_ib_container(cannm_ib, "BSW-MODULE-ENTRY", entry_name)

    # 没有CanNmMainFunction容器，按照默认的CanNm_MainFunction
    if cannmmainfunction_num == 0:
        # 新增并写入配置的周期时间
        mainfunction_name = "CanNm_MainFunction"
        period = 0.005
        ib_bswmd_utils.create_bsw_module_entry_ref_conditional(cannm_ib, "CanNm", mainfunction_name)
        ib_bswmd_utils.create_bsw_timing_event(cannm_ib, "CanNm", mainfunction_name + "TimingEvent0",
                                            mainfunction_name, float(period))
        ib_bswmd_utils.create_bsw_schedulable_entity(cannm_ib, "CanNm", mainfunction_name, mainfunction_name)
        ib_bswmd_utils.create_bsw_module_entry(cannm_ib, "CanNm", mainfunction_name)

    # 存在CanNmMainFunction容器，按照CanNmMainFunction容器下主函数的进行taskmapping
    else:
        # 新增并写入配置的周期时间
        for cannmmainfunction in cannmmainfunction_list:
            if get_definition_ref(cannmmainfunction).split("/")[-1] == "CanNmMainFunction":
                mainfunction_name = get_short_name(cannmmainfunction)
                value_list = ib_bswmd_utils.get_parameter_values(cannmmainfunction)
                period = get_value(value_list[0])
                ib_bswmd_utils.create_bsw_module_entry_ref_conditional(cannm_ib, "CanNm", mainfunction_name)
                ib_bswmd_utils.create_bsw_timing_event(cannm_ib, "CanNm", mainfunction_name + "TimingEvent0",
                                                    mainfunction_name, float(period))
                ib_bswmd_utils.create_bsw_schedulable_entity(cannm_ib, "CanNm", mainfunction_name, mainfunction_name)
                ib_bswmd_utils.create_bsw_module_entry(cannm_ib, "CanNm", mainfunction_name)

