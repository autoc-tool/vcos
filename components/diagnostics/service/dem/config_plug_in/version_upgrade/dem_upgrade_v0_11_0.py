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

from basic_func_module.autosar_utils.utils import *
from basic_func_module.autosar_utils.autosar_util import AutosarUtil
from func_module.health_monitor.logger import logger


class DemUpgradeV0110():

    def __init__(self):
        pass

    def set_textual_param_value(self, parameter_def_name, container_obj, value):
        textual_param_list = container_obj.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE
        for textual_parameter in textual_param_list:
            if get_definition_ref(textual_parameter).split("/")[-1] == parameter_def_name:
                set_value(textual_parameter, value)

    def upgrade_module(self, bsw_ecuc_autosar_class, autosar_ib_class, mod_autosar_dict, sys_info_dict) -> bool:
        print("upgrade Dem module")
        DemClass = bsw_ecuc_autosar_class["Dem"]
        dem_general_container_val = DemClass.find("/ActiveEcuC/Dem/DemGeneral")
        dem_obd_support_type = "DemAvailabilitySupport"

        # Set "DemAvailabilitySupport" value
        parameter_def_name = "DemAvailabilitySupport"
        dem_availability_support_type = "DEM_NO_AVAILABILITY"
        self.set_textual_param_value(parameter_def_name, dem_general_container_val, dem_availability_support_type)

        # Set "DemOBDSupport" value
        parameter_def_name = "DemOBDSupport"
        dem_obd_support_type = "DEM_OBD_NO_OBD_SUPPORT"
        self.set_textual_param_value(parameter_def_name, dem_general_container_val, dem_obd_support_type)

        dem_nvram_block_container_var = DemClass.find("/ActiveEcuC/Dem/DemGeneral").SUB_CONTAINERS.ECUC_CONTAINER_VALUE
        for nvram_block in dem_nvram_block_container_var:
            if get_short_name(nvram_block) == "NvMConf_NvMBlockDescriptor_Dem_EventAvailableData":
                AutosarUtil.remove_container_value(dem_nvram_block_container_var, nvram_block)
                break

        return True
