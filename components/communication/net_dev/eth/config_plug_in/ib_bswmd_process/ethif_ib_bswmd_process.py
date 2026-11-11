# /*
#  * Copyright (c) 2025 Li Auto Inc. and its affiliates
#  * Licensed under the Apache License, Version 2.0(the "License");
#  * you may not use this file except in compliance with the License.
#  * You may obtain a copy of the License at
#  *
#  *     http://www.apache.org/licenses/LICENSE-2.0
#  *
#  * Unless required by applicable law or agreed to in writing, software
#  * distributed under the License is distributed on an "AS IS" BASIS,
#  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  * See the License for the specific language governing permissions and
#  * limitations under the License.
#  */
"""
ethif_ib_bswm更新处理
"""

from basic_func_module.autosar_utils.utils import get_definition_ref, get_value, get_short_name
from func_module.module_management.arxml_handler.ib_bswmd_process import ib_bswmd_utils

def _period_legal(perido):
    return str(perido).replace(".", "").isdigit()

def update_ib_bswmd(internal_behavior_dict, ecuc_autosar_dict, asw_autosar_dict, system_autosar_dict, other_input_info):
    ethif_ib = internal_behavior_dict.get("EthIf", None)
    if ethif_ib is None:
        return

    ethif_autosar = ecuc_autosar_dict.get("EthIf", None)
    if ethif_autosar is None:
        return

    period_rx = 0.01
    period_state = 0.01
    ethif_general = ethif_autosar.find("/ActiveEcuC/EthIf/EthIfGeneral")
    value_list = ib_bswmd_utils.get_parameter_values(ethif_general)
    for value_node in value_list:
        if get_definition_ref(value_node).split("/")[-1] == "EthIfMainFunctionPeriod":
            period_rx = get_value(value_node)
        elif get_definition_ref(value_node).split("/")[-1] == "EthIfMainFunctionStatePeriod":
            period_state = get_value(value_node)
        else :
            continue

    if not _period_legal(period_rx):
        return
    if not _period_legal(period_state):
        return

    timing_event_list = ethif_ib.find_nodes("BSW-TIMING-EVENT")
    for timing_event in timing_event_list:
        short_name = get_short_name(timing_event)
        if short_name == "EthIf_MainFunctionRxTimingEvent0":
            timing_event.PERIOD.valueOf_ = float(period_rx)
        elif short_name == "EthIf_MainFunctionStateTimingEvent0":
            timing_event.PERIOD.valueOf_ = float(period_state)
        else :
            continue