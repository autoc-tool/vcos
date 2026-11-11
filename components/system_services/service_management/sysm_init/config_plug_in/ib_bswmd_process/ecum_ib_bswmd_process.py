#
# Copyright (C) 2015-2022 Lixiang Group Holding Limited
#
"""
ecum_ib_bswm更新处理
"""

from basic_func_module.autosar_utils.utils import get_short_name, get_definition_ref, get_value
from func_module.module_management.arxml_handler.ib_bswmd_process import ib_bswmd_utils

def update_ib_bswmd(internal_behavior_dict, ecuc_autosar_dict, asw_autosar_dict, system_autosar_dict, other_input_info):
    ecum_ib = internal_behavior_dict.get("EcuM", None)
    ecuc_autosar = ecuc_autosar_dict.get("EcuC", None)
    if ecum_ib is None:
        return
    if ecuc_autosar is None:
        return
    coresbase = ecuc_autosar.find("/ActiveEcuC/EcuC/EcucHardware")
    core_num = len(coresbase.SUB_CONTAINERS.ECUC_CONTAINER_VALUE)



    timing_event_list = ecum_ib.find_nodes("BSW-TIMING-EVENT")
    if len(timing_event_list) < core_num:  # 核心数增加
        core_id = 0
        while core_id < core_num:
            for timing_event in timing_event_list:
                short_name = get_short_name(timing_event)
                if short_name == "EcuM_MainFunctionTimingEvent" + str(core_id):
                    break
            else:
                ib_bswmd_utils.create_bsw_timing_event(ecum_ib, "EcuM", "EcuM_MainFunctionTimingEvent" + str(core_id),
                                                       "ecum_mainfunction", 0.01)
            core_id += 1

    elif len(timing_event_list) > core_num:  # 核心数减少
        core_id = 0
        for timing_event in timing_event_list:
            short_name = get_short_name(timing_event)
            if core_id < core_num:
                core_id += 1
        else:
            ib_bswmd_utils.delete_ib_container(ecum_ib, "BSW-TIMING-EVENT", short_name)
            core_id += 1

    # 修改 ecum_mainfunction 周期
    period = 0.01
    ecum_autosar = ecuc_autosar_dict.get("EcuM", None)
    if ecum_autosar is None:
        return

    ecumperiodbase = ecum_autosar.find("/ActiveEcuC/EcuM/EcuMGeneral")
    if ecumperiodbase is None:
        return

    value_list = ib_bswmd_utils.get_parameter_values(ecumperiodbase)
    for value_node in value_list:
        if get_definition_ref(value_node).split("/")[-1] != "EcuMMainFunctionPeriod":
            continue
        period = get_value(value_node)
        break
    timing_event_list_update = ecum_ib.find_nodes("BSW-TIMING-EVENT")
    for timing_event in timing_event_list_update:
        timing_event.PERIOD.valueOf_ = float(period)
