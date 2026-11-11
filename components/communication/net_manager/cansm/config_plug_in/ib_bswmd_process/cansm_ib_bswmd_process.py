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
cansm_ib_bswm更新处理
"""

from basic_func_module.autosar_utils.utils import get_definition_ref, get_value, get_short_name
from func_module.module_management.arxml_handler.ib_bswmd_process import ib_bswmd_utils


def update_ib_bswmd(internal_behavior_dict, ecuc_autosar_dict, asw_autosar_dict, system_autosar_dict, other_input_info):
    cansm_ib = internal_behavior_dict.get("CanSM", None)
    if cansm_ib is None:
        return

    cansm_autosar = ecuc_autosar_dict.get("CanSM", None)
    if cansm_autosar is None:
        return


    cansmmainfunction_base = cansm_autosar.find("/ActiveEcuC/CanSM/CanSMConfiguration")
    if cansmmainfunction_base.SUB_CONTAINERS is None:
        return
    cansmmainfunction_list = cansmmainfunction_base.SUB_CONTAINERS.ECUC_CONTAINER_VALUE
    
    cansmmainfunction_num = 0
    for cansmmainfunction in cansmmainfunction_list:
        if get_definition_ref(cansmmainfunction).split("/")[-1] == "CanSMMainFunction":
            cansmmainfunction_num = cansmmainfunction_num + 1

    # 删除
    bsw_module_entry_ref_list = cansm_ib.find_nodes("BSW-MODULE-ENTRY-REF-CONDITIONAL")
    for bsw_module_entry_ref in bsw_module_entry_ref_list:
        entry_ref_name = getattr(bsw_module_entry_ref.BSW_MODULE_ENTRY_REF, "valueOf_", "").split("/")[-1]
        ib_bswmd_utils.delete_ib_node(cansm_ib, "BSW-MODULE-ENTRY-REF-CONDITIONAL", entry_ref_name)

    timing_event_list = cansm_ib.find_nodes("BSW-TIMING-EVENT")
    for timing_event in timing_event_list:
        timing_event_name = get_short_name(timing_event)
        ib_bswmd_utils.delete_ib_container(cansm_ib, "BSW-TIMING-EVENT", timing_event_name)

    bsw_schedulable_entity_list = cansm_ib.find_nodes("BSW-SCHEDULABLE-ENTITY")
    for bsw_schedulable_entity in bsw_schedulable_entity_list:
        entity_name = get_short_name(bsw_schedulable_entity)
        ib_bswmd_utils.delete_ib_container(cansm_ib, "BSW-SCHEDULABLE-ENTITY", entity_name)

    bsw_module_entry_list = cansm_ib.find_nodes("BSW-MODULE-ENTRY")
    for bsw_module_entry in bsw_module_entry_list:
        entry_name = get_short_name(bsw_module_entry)
        ib_bswmd_utils.delete_ib_container(cansm_ib, "BSW-MODULE-ENTRY", entry_name)


    # 没有CanSMMainFunction容器，按照默认的CanSMMainFunction
    if cansmmainfunction_num == 0:
        period = 0.01
        ib_bswmd_utils.create_bsw_module_entry_ref_conditional(cansm_ib, "CanSM", "CanSM_MainFunction")
        ib_bswmd_utils.create_bsw_timing_event(cansm_ib, "CanSM", "CanSM_MainFunction" + "TimingEvent0",
                                            "CanSM_MainFunction", float(period))
        ib_bswmd_utils.create_bsw_schedulable_entity(cansm_ib, "CanSM", "CanSM_MainFunction", "CanSM_MainFunction")
        ib_bswmd_utils.create_bsw_module_entry(cansm_ib, "CanSM", "CanSM_MainFunction")

    # 存在CanSMMainFunction容器，按照CanSMMainFunction容器下主函数taskmapping
    else:
        # 新增并写入配置的周期时间
        mainfunc_num = 0
        for cansmmainfunction in cansmmainfunction_list:
            if get_definition_ref(cansmmainfunction).split("/")[-1] == "CanSMMainFunction":
                mainfunction_name = get_short_name(cansmmainfunction)
                value_list = ib_bswmd_utils.get_parameter_values(cansmmainfunction)
                period = get_value(value_list[0])
                ib_bswmd_utils.create_bsw_module_entry_ref_conditional(cansm_ib, "CanSM", mainfunction_name)
                ib_bswmd_utils.create_bsw_timing_event(cansm_ib, "CanSM", mainfunction_name + "TimingEvent" + str(mainfunc_num),
                                                    mainfunction_name, float(period))
                ib_bswmd_utils.create_bsw_schedulable_entity(cansm_ib, "CanSM", mainfunction_name, mainfunction_name)
                ib_bswmd_utils.create_bsw_module_entry(cansm_ib, "CanSM", mainfunction_name)
