#
# Copyright (C) 2015-2022 Lixiang Group Holding Limited
#
"""
tcpip_ib_bswm更新处理
"""

from basic_func_module.autosar_utils.utils import get_short_name, get_definition_ref, get_value
from func_module.module_management.arxml_handler.ib_bswmd_process import ib_bswmd_utils

def _period_legal(perido):
    return str(perido).replace(".", "").isdigit()

def update_ib_bswmd(internal_behavior_dict, ecuc_autosar_dict, asw_autosar_dict, system_autosar_dict, other_input_info):
    tcpip_ib = internal_behavior_dict.get("TcpIp", None)
    if tcpip_ib is None:
        return

    tcpip_autosar = ecuc_autosar_dict.get("TcpIp", None)
    if tcpip_autosar is None:
        return

    period = 0.01
    tcpip_general = tcpip_autosar.find("/ActiveEcuC/TcpIp/TcpIpGeneral")
    value_list = ib_bswmd_utils.get_parameter_values(tcpip_general)
    for value_node in value_list:
        if get_definition_ref(value_node).split("/")[-1] != "TcpIpMainFunctionPeriod":
            continue
        period = get_value(value_node)
        break

    if not _period_legal(period):
        return

    timing_event_list = tcpip_ib.find_nodes("BSW-TIMING-EVENT")
    for timing_event in timing_event_list:
        short_name = get_short_name(timing_event)
        if short_name != "TcpIp_MainFunctionTimingEvent0":
            continue
        timing_event.PERIOD.valueOf_ = float(period)
        break
