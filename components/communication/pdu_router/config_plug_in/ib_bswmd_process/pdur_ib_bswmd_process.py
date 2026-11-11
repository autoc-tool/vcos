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
pdur_ib_bswm更新处理
"""

from basic_func_module.autosar_utils.utils import get_definition_ref, get_value, get_short_name
from func_module.module_management.arxml_handler.ib_bswmd_process import ib_bswmd_utils


def update_ib_bswmd(internal_behavior_dict, ecuc_autosar_dict, asw_autosar_dict, system_autosar_dict, other_input_info):
    pdur_ib = internal_behavior_dict.get("PduR", None)
    if pdur_ib is None:
        return

    pdur_autosar = ecuc_autosar_dict.get("PduR", None)
    if pdur_autosar is None:
        return


    pdurmainfunction_base = pdur_autosar.find("/ActiveEcuC/PduR")
    if pdurmainfunction_base.CONTAINERS.ECUC_CONTAINER_VALUE is None:
        return
    pdurmainfunction_list = pdurmainfunction_base.CONTAINERS.ECUC_CONTAINER_VALUE
    
    pdurmainfunction_num = 0
    for pdurmainfunction in pdurmainfunction_list:
        if get_definition_ref(pdurmainfunction).split("/")[-1] == "PduRMainFunction":
            pdurmainfunction_num = pdurmainfunction_num + 1

    # 删除
    bsw_module_entry_ref_list = pdur_ib.find_nodes("BSW-MODULE-ENTRY-REF-CONDITIONAL")
    for bsw_module_entry_ref in bsw_module_entry_ref_list:
        entry_ref_name = getattr(bsw_module_entry_ref.BSW_MODULE_ENTRY_REF, "valueOf_", "").split("/")[-1]
        ib_bswmd_utils.delete_ib_node(pdur_ib, "BSW-MODULE-ENTRY-REF-CONDITIONAL", entry_ref_name)

    timing_event_list = pdur_ib.find_nodes("BSW-TIMING-EVENT")
    for timing_event in timing_event_list:
        timing_event_name = get_short_name(timing_event)
        ib_bswmd_utils.delete_ib_container(pdur_ib, "BSW-TIMING-EVENT", timing_event_name)

    bsw_schedulable_entity_list = pdur_ib.find_nodes("BSW-SCHEDULABLE-ENTITY")
    for bsw_schedulable_entity in bsw_schedulable_entity_list:
        entity_name = get_short_name(bsw_schedulable_entity)
        ib_bswmd_utils.delete_ib_container(pdur_ib, "BSW-SCHEDULABLE-ENTITY", entity_name)

    bsw_module_entry_list = pdur_ib.find_nodes("BSW-MODULE-ENTRY")
    for bsw_module_entry in bsw_module_entry_list:
        entry_name = get_short_name(bsw_module_entry)
        ib_bswmd_utils.delete_ib_container(pdur_ib, "BSW-MODULE-ENTRY", entry_name)


    # 没有PduRMainFunction容器，不生成PduRMainFunction
    # 存在PduRMainFunction容器，按照PduRMainFunction容器下主函数taskmapping
    if pdurmainfunction_num != 0:
        # 新增并写入配置的周期时间
        mainfunc_num = 0
        for pdurmainfunction in pdurmainfunction_list:
            if get_definition_ref(pdurmainfunction).split("/")[-1] == "PduRMainFunction":
                mainfunction_name = get_short_name(pdurmainfunction)
                value_list = ib_bswmd_utils.get_parameter_values(pdurmainfunction)
                period = get_value(value_list[0])
                ib_bswmd_utils.create_bsw_module_entry_ref_conditional(pdur_ib, "PduR", mainfunction_name)
                ib_bswmd_utils.create_bsw_timing_event(pdur_ib, "PduR", mainfunction_name + "TimingEvent" + str(mainfunc_num),
                                                    mainfunction_name, float(period))
                ib_bswmd_utils.create_bsw_schedulable_entity(pdur_ib, "PduR", mainfunction_name, mainfunction_name)
                ib_bswmd_utils.create_bsw_module_entry(pdur_ib, "PduR", mainfunction_name)
