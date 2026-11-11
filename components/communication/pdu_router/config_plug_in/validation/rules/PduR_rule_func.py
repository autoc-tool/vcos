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
from func_module.validation.common.basic_rule import BasicRule, timing_decorator
from durable.lang import *
from durable.engine import *
from basic_func_module.autosar_utils.autosar_util import AutosarUtil

def pdur_validate_get_routing_path(ecuc_pdur_dict: dict):
    valid = True
    PduRRoutingPaths = {}
    PduRRoutingPaths_Name = 'PduRRoutingPaths'
    for PduRRoutingPaths_Name, item in ecuc_pdur_dict.items():
        if "DefinitionRef" not in item or item["DefinitionRef"] != '/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths':
            continue
        PduRRoutingPaths = item
        break
    if len(PduRRoutingPaths) == 0:
        valid = False
    return PduRRoutingPaths, PduRRoutingPaths_Name, valid