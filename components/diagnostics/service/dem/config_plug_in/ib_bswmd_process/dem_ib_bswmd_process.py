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
dem_ib_bswm更新处理
"""

from basic_func_module.autosar_utils.utils import get_short_name, get_definition_ref, get_value, get_value_ref
from func_module.module_management.arxml_handler.ib_bswmd_process import ib_bswmd_utils

def update_ib_bswmd(internal_behavior_dict, ecuc_autosar_dict, asw_autosar_dict, system_autosar_dict, other_input_info):
    dem_enable_satellite = False
    dem_ib = internal_behavior_dict.get("Dem", None)
    if dem_ib is None:
        return

    dem_autosar = ecuc_autosar_dict.get("Dem", None)
    if dem_autosar is None:
        return
    dem_general = dem_autosar.find("/ActiveEcuC/Dem/DemGeneral")
    if dem_general is None:
        return
    # 不启用Satellite机制时DemMasterOsApplicationRef为空
    if dem_general.REFERENCE_VALUES is None:
        return
    for value_ref_cantainer in dem_general.REFERENCE_VALUES.ECUC_REFERENCE_VALUE:
        if get_definition_ref(value_ref_cantainer).split("/")[-1] != "DemMasterOsApplicationRef":
            continue
        if get_value_ref(value_ref_cantainer) is not None and get_value_ref(value_ref_cantainer) != '':
            dem_enable_satellite = True

    dem_config_set = dem_autosar.find("/ActiveEcuC/Dem/DemConfigSet")
    if dem_config_set.SUB_CONTAINERS is None:
        return
    dem_config_set_sub_containers = dem_config_set.SUB_CONTAINERS.ECUC_CONTAINER_VALUE
    if dem_config_set_sub_containers is None:
        return
    dem_event_parameter_container_list = []
    for sub_container in dem_config_set_sub_containers:
        if get_definition_ref(sub_container).split("/")[-1] == "DemEventParameter":
            dem_event_parameter_container_list.append(sub_container)
        else:
            continue
    if dem_event_parameter_container_list == []:
        return

    # 删除
    bsw_module_entry_ref_list = dem_ib.find_nodes("BSW-MODULE-ENTRY-REF-CONDITIONAL")
    for bsw_module_entry_ref in bsw_module_entry_ref_list:
        entry_ref_name = getattr(bsw_module_entry_ref.BSW_MODULE_ENTRY_REF, "valueOf_", "").split("/")[-1]
        ib_bswmd_utils.delete_ib_node(dem_ib, "BSW-MODULE-ENTRY-REF-CONDITIONAL", entry_ref_name)

    timing_event_list = dem_ib.find_nodes("BSW-TIMING-EVENT")
    for timing_event in timing_event_list:
        timing_event_name = get_short_name(timing_event)
        ib_bswmd_utils.delete_ib_container(dem_ib, "BSW-TIMING-EVENT", timing_event_name)

    bsw_schedulable_entity_list = dem_ib.find_nodes("BSW-SCHEDULABLE-ENTITY")
    for bsw_schedulable_entity in bsw_schedulable_entity_list:
        entity_name = get_short_name(bsw_schedulable_entity)
        ib_bswmd_utils.delete_ib_container(dem_ib, "BSW-SCHEDULABLE-ENTITY", entity_name)

    bsw_module_entry_list = dem_ib.find_nodes("BSW-MODULE-ENTRY")
    for bsw_module_entry in bsw_module_entry_list:
        entry_name = get_short_name(bsw_module_entry)
        ib_bswmd_utils.delete_ib_container(dem_ib, "BSW-MODULE-ENTRY", entry_name)

    if dem_enable_satellite is False:
        mainfunction_name = "Dem_MainFunction"

        ib_bswmd_utils.create_bsw_module_entry_ref_conditional(dem_ib, "Dem", mainfunction_name)
        ib_bswmd_utils.create_bsw_timing_event(dem_ib, "Dem", mainfunction_name + "TimingEvent0",
                                                mainfunction_name, 0.01)
        ib_bswmd_utils.create_bsw_schedulable_entity(dem_ib, "Dem", mainfunction_name, mainfunction_name)
        ib_bswmd_utils.create_bsw_module_entry(dem_ib, "Dem", mainfunction_name)
    else:
        add_master_and_satellite_mainfunction(dem_ib, dem_general, dem_event_parameter_container_list)

    period = 0.01
    dem_general = dem_autosar.find("/ActiveEcuC/Dem/DemGeneral")
    value_list = ib_bswmd_utils.get_parameter_values(dem_general)
    for value_node in value_list:
        if get_definition_ref(value_node).split("/")[-1] != "DemTaskTime":
            continue
        period = get_value(value_node)
        break

    timing_event_list = dem_ib.find_nodes("BSW-TIMING-EVENT")
    for timing_event in timing_event_list:
        short_name = get_short_name(timing_event)
        if short_name != "Dem_MainFunctionTimingEvent0":
            continue
        timing_event.PERIOD.valueOf_ = float(period)
        break

def add_master_and_satellite_mainfunction(dem_ib, dem_general, dem_event_parameter_container_list):
    # 新增Dem_MasterMainFunction
    mainfunction_name = "Dem_MasterMainFunction"

    ib_bswmd_utils.create_bsw_module_entry_ref_conditional(dem_ib, "Dem", mainfunction_name)
    ib_bswmd_utils.create_bsw_timing_event(dem_ib, "Dem", mainfunction_name + "TimingEvent0",
                                            mainfunction_name, 0.01)
    ib_bswmd_utils.create_bsw_schedulable_entity(dem_ib, "Dem", mainfunction_name, mainfunction_name)
    ib_bswmd_utils.create_bsw_module_entry(dem_ib, "Dem", mainfunction_name)

    # 根据satellite配置生成对应数量的Dem_SatelliteMainFunction
    satellite_name_list = []

    if dem_general.REFERENCE_VALUES is None:
        return
    for value_ref_cantainer in dem_general.REFERENCE_VALUES.ECUC_REFERENCE_VALUE:
        if get_definition_ref(value_ref_cantainer).split("/")[-1] != "DemMasterOsApplicationRef":
            continue
        if get_value_ref(value_ref_cantainer) is None or get_value_ref(value_ref_cantainer) == '':
            # master_name = "Application0_Core0"
            master_name = "SystemApplication_Core0"
        else:
            master_name = get_value_ref(value_ref_cantainer).split("/")[-1]

    for dem_event_parameter_container in dem_event_parameter_container_list:
        if dem_event_parameter_container.REFERENCE_VALUES is None:
            continue
        for value_ref_cantainer in dem_event_parameter_container.REFERENCE_VALUES.ECUC_REFERENCE_VALUE:
            if get_definition_ref(value_ref_cantainer).split("/")[-1] != "DemOsApplicationRef":
                continue
            if get_value_ref(value_ref_cantainer) is None or get_value_ref(value_ref_cantainer) == '':
                satellite_name = master_name
            else:
                satellite_name = get_value_ref(value_ref_cantainer).split("/")[-1]

            if satellite_name not in satellite_name_list:
                mainfunction_name = "Dem_SatelliteMainFunction"

                ib_bswmd_utils.create_bsw_module_entry_ref_conditional(dem_ib, "Dem", mainfunction_name)
                ib_bswmd_utils.create_bsw_timing_event(dem_ib, "Dem", mainfunction_name + "TimingEvent" + str(len(satellite_name_list) + 1),
                                                    mainfunction_name, 0.01)
                ib_bswmd_utils.create_bsw_schedulable_entity(dem_ib, "Dem", mainfunction_name, mainfunction_name)
                ib_bswmd_utils.create_bsw_module_entry(dem_ib, "Dem", mainfunction_name)
                satellite_name_list.append(satellite_name)



