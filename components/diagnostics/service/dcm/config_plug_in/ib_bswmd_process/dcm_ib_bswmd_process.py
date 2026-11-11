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
dcm_ib_bswm更新处理
"""

from basic_func_module.autosar_utils.utils import get_short_name, get_definition_ref, get_value
from func_module.module_management.arxml_handler.ib_bswmd_process import ib_bswmd_utils


def update_ib_bswmd(internal_behavior_dict, ecuc_autosar_dict, asw_autosar_dict, system_autosar_dict, other_input_info):
    dcm_ib = internal_behavior_dict.get("Dcm", None)
    if dcm_ib is None:
        return

    dcm_autosar = ecuc_autosar_dict.get("Dcm", None)
    if dcm_autosar is None:
        return

    period = 0.01
    dcm_general = dcm_autosar.find("/ActiveEcuC/Dcm/DcmGeneral")
    value_list = ib_bswmd_utils.get_parameter_values(dcm_general)
    for value_node in value_list:
        if get_definition_ref(value_node).split("/")[-1] != "DcmTaskTime":
            continue
        period = get_value(value_node)
        break

    timing_event_list = dcm_ib.find_nodes("BSW-TIMING-EVENT")
    for timing_event in timing_event_list:
        short_name = get_short_name(timing_event)
        if short_name != "Dcm_MainFunctionTimingEvent0":
            continue
        timing_event.PERIOD.valueOf_ = float(period)
        break
