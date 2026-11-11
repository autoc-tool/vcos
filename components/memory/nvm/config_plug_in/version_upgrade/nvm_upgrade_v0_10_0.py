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

from basic_func_module.autosar_utils.utils import get_definition_ref, set_value, get_value, get_short_name,get_value_ref
from basic_func_module.autosar_utils.autosar_util import AutosarUtil
from func_module.logger import logger
import re

class NvMUpgradeV0100():

    def __init__(self):
        pass

    def upgrade_module(self, bsw_ecuc_autosar_class, autosar_ib_class, mod_autosar_dict, sys_info_dict) -> bool:
        print("upgrade NvM module")
        srcdata = bsw_ecuc_autosar_class["NvM"]
        nvmblockdescriptor = srcdata.find("/ActiveEcuC/NvM").CONTAINERS
        nvmblockdescriptor_block_list = nvmblockdescriptor.ECUC_CONTAINER_VALUE
        for nvmblockdescriptor_block in nvmblockdescriptor_block_list:
            if get_short_name(nvmblockdescriptor_block) == "NvMBlockDescriptor_Dem_StatusBit":
                textualparam_list = nvmblockdescriptor_block.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE
                for textualparam in textualparam_list:
                    if get_value(textualparam) == "Dem_GetStatusBitSubsetBufferAddr()":
                        set_value(textualparam, "Dem_GetStatusDataAddr()")
            elif get_short_name(nvmblockdescriptor_block) == "NvMBlockDescriptor_Dem_PriMemEventAdmin":
                textualparam_list = nvmblockdescriptor_block.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE
                for textualparam in textualparam_list:
                    if get_value(textualparam) == "Dem_GetPriMemAdminDataAddr()":
                        set_value(textualparam, "Dem_GetAdminDataAddr()")

        return True