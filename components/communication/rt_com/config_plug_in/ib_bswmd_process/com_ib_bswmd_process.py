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
    com_ib = internal_behavior_dict.get("Com", None)
    com_autosar = ecuc_autosar_dict.get("Com", None)
    commainfunction_base = com_autosar.find("/ActiveEcuC/Com/ComConfig")

    if com_ib is None:
        return
    if com_autosar is None:
        return
    if commainfunction_base.SUB_CONTAINERS is None:
        return
    commainfunction_list = commainfunction_base.SUB_CONTAINERS.ECUC_CONTAINER_VALUE

    # 删除
    bsw_module_entry_ref_list = com_ib.find_nodes("BSW-MODULE-ENTRY-REF-CONDITIONAL")
    for bsw_module_entry_ref in bsw_module_entry_ref_list:
        entry_ref_name = getattr(bsw_module_entry_ref.BSW_MODULE_ENTRY_REF, "valueOf_", "").split("/")[-1]
        ib_bswmd_utils.delete_ib_node(com_ib, "BSW-MODULE-ENTRY-REF-CONDITIONAL", entry_ref_name)

    timing_event_list = com_ib.find_nodes("BSW-TIMING-EVENT")
    for timing_event in timing_event_list:
        timing_event_name = get_short_name(timing_event)
        ib_bswmd_utils.delete_ib_container(com_ib, "BSW-TIMING-EVENT", timing_event_name)

    bsw_schedulable_entity_list = com_ib.find_nodes("BSW-SCHEDULABLE-ENTITY")
    for bsw_schedulable_entity in bsw_schedulable_entity_list:
        entity_name = get_short_name(bsw_schedulable_entity)
        ib_bswmd_utils.delete_ib_container(com_ib, "BSW-SCHEDULABLE-ENTITY", entity_name)

    bsw_module_entry_list = com_ib.find_nodes("BSW-MODULE-ENTRY")
    for bsw_module_entry in bsw_module_entry_list:
        entry_name = get_short_name(bsw_module_entry)
        ib_bswmd_utils.delete_ib_container(com_ib, "BSW-MODULE-ENTRY", entry_name)
    
    # 判断是否有COM_EXCLUSIVE_AREA_1 如果没有COM_EXCLUSIVE_AREA_1 需要增加COM_EXCLUSIVE_AREA_1
    # COM_EXCLUSIVE_AREA_1用于给Com_MainFunction_Tx使用  用户可单独配置关闭
    exclusive_name_list = []
    bsw_module_exclusive_list = com_ib.find_nodes("EXCLUSIVE-AREA")
    for bsw_module_exclusive in bsw_module_exclusive_list:
        exclusive_name = get_short_name(bsw_module_exclusive)
        exclusive_name_list.append(exclusive_name)
    
    if "COM_EXCLUSIVE_AREA_1" not in exclusive_name_list:
        ib_bswmd_utils.create_bsw_exclusive_area(com_ib, "Com", "COM_EXCLUSIVE_AREA_1")
    

    # 新增并写入配置的周期时间
    for commainfunction in commainfunction_list:
        if get_definition_ref(commainfunction).split("/")[-1] == "ComMainFunctionTx" or get_definition_ref(commainfunction).split("/")[-1] == "ComMainFunctionRx" or get_definition_ref(commainfunction).split("/")[-1] == "ComMainFunctionGw":
            mainfunction_name = get_short_name(commainfunction)
            value_list = ib_bswmd_utils.get_parameter_values(commainfunction)
            period = get_value(value_list[0])
            ib_bswmd_utils.create_bsw_module_entry_ref_conditional(com_ib, "Com", mainfunction_name)
            ib_bswmd_utils.create_bsw_timing_event(com_ib, "Com", mainfunction_name + "TimingEvent0",
                                                   mainfunction_name, float(period))
            ib_bswmd_utils.create_bsw_schedulable_entity(com_ib, "Com", mainfunction_name, mainfunction_name)
            ib_bswmd_utils.create_bsw_module_entry(com_ib, "Com", mainfunction_name)
    

        if get_definition_ref(commainfunction).split("/")[-1] == "ComTxSignalFifo":
            mainfunction_name = "Com_SendSignalProxyPeriodic_Runable"
            value_list = ib_bswmd_utils.get_parameter_values(commainfunction)
            for value in value_list:
                if get_definition_ref(value).split("/")[-1] == "ProxyPeriodicTime":
                    break
            period = get_value(value)
            ib_bswmd_utils.create_bsw_module_entry_ref_conditional(com_ib, "Com", mainfunction_name)
            ib_bswmd_utils.create_bsw_timing_event(com_ib, "Com", mainfunction_name + "TimingEvent0",
                                                   mainfunction_name, float(period))
            ib_bswmd_utils.create_bsw_schedulable_entity(com_ib, "Com", mainfunction_name, mainfunction_name)
            ib_bswmd_utils.create_bsw_module_entry(com_ib, "Com", mainfunction_name)


