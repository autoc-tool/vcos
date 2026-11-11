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
comm_ib_bswm更新处理
"""

from basic_func_module.autosar_utils.utils import get_definition_ref, get_value, get_short_name
from func_module.module_management.arxml_handler.ib_bswmd_process import ib_bswmd_utils


def update_ib_bswmd(internal_behavior_dict, ecuc_autosar_dict, asw_autosar_dict, system_autosar_dict, other_input_info):
    comm_ib = internal_behavior_dict.get("ComM", None)
    comm_autosar = ecuc_autosar_dict.get("ComM", None)
    channelbase = comm_autosar.find("/ActiveEcuC/ComM/ComMConfigSet")

    if comm_ib is None:
        return
    if comm_autosar is None:
        return
    if channelbase.SUB_CONTAINERS is None:
        return

    channel_list = channelbase.SUB_CONTAINERS.ECUC_CONTAINER_VALUE

    # 删除
    bsw_module_entry_ref_list = comm_ib.find_nodes("BSW-MODULE-ENTRY-REF-CONDITIONAL")
    for bsw_module_entry_ref in bsw_module_entry_ref_list:
        entry_ref_name = getattr(bsw_module_entry_ref.BSW_MODULE_ENTRY_REF, "valueOf_", "").split("/")[-1]
        ib_bswmd_utils.delete_ib_node(comm_ib, "BSW-MODULE-ENTRY-REF-CONDITIONAL", entry_ref_name)

    timing_event_list = comm_ib.find_nodes("BSW-TIMING-EVENT")
    for timing_event in timing_event_list:
        timing_event_name = get_short_name(timing_event)
        ib_bswmd_utils.delete_ib_container(comm_ib, "BSW-TIMING-EVENT", timing_event_name)

    bsw_schedulable_entity_list = comm_ib.find_nodes("BSW-SCHEDULABLE-ENTITY")
    for bsw_schedulable_entity in bsw_schedulable_entity_list:
        entity_name = get_short_name(bsw_schedulable_entity)
        ib_bswmd_utils.delete_ib_container(comm_ib, "BSW-SCHEDULABLE-ENTITY", entity_name)

    bsw_module_entry_list = comm_ib.find_nodes("BSW-MODULE-ENTRY")
    for bsw_module_entry in bsw_module_entry_list:
        entry_name = get_short_name(bsw_module_entry)
        ib_bswmd_utils.delete_ib_container(comm_ib, "BSW-MODULE-ENTRY", entry_name)
    
    ib_bswmd_utils.create_bsw_module_entry_ref_conditional(comm_ib, "ComM", "ComM_MainFunction_All_Channels")
    ib_bswmd_utils.create_bsw_schedulable_entity(comm_ib, "ComM", "ComM_MainFunction_All_Channels", "ComM_MainFunction_All_Channels")
    ib_bswmd_utils.create_bsw_module_entry(comm_ib, "ComM", "ComM_MainFunction_All_Channels")

    # 新增
    for channel in channel_list:
        if get_definition_ref(channel).split("/")[-1] == "ComMChannel":
            channel_name = get_short_name(channel)
            mainfunction_name = "ComM_MainFunction_" + channel_name

            ib_bswmd_utils.create_bsw_module_entry_ref_conditional(comm_ib, "ComM", mainfunction_name)
            ib_bswmd_utils.create_bsw_timing_event(comm_ib, "ComM", mainfunction_name + "TimingEvent0",
                                                   mainfunction_name, 0.01)
            ib_bswmd_utils.create_bsw_schedulable_entity(comm_ib, "ComM", mainfunction_name, mainfunction_name)
            ib_bswmd_utils.create_bsw_module_entry(comm_ib, "ComM", mainfunction_name)

    # 修改 ComM_MainFunction 周期
    period = 0.01
    timing_event_list_update = comm_ib.find_nodes("BSW-TIMING-EVENT")
    for channel in channel_list:
        if get_definition_ref(channel).split("/")[-1] == "ComMChannel":
            channel_name = get_short_name(channel)
            value_list = ib_bswmd_utils.get_parameter_values(channel)

            for value_node in value_list:
                if get_definition_ref(value_node).split("/")[-1] != "ComMMainFunctionPeriod":
                    continue
                period = get_value(value_node)
                break

            for timing_event in timing_event_list_update:
                short_name = get_short_name(timing_event)
                if short_name == "ComM_MainFunction_" + channel_name + "TimingEvent0":
                    timing_event.PERIOD.valueOf_ = float(period)
                    break
