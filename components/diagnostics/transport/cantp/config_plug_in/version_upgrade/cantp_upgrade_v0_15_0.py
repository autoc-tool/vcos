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

from basic_func_module.autosar_utils.utils import get_definition_ref, set_value, get_value
from basic_func_module.autosar_utils.autosar_util import AutosarUtil
from func_module.health_monitor.logger import logger

class CanTpUpgradeV0150():

    def __init__(self):
        pass

    def set_numerical_param_value(self, parameter_def_name, container_obj, value):
        numerical_param_list = container_obj.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE
        for numerical_parameter in numerical_param_list:
            if get_definition_ref(numerical_parameter).split("/")[-1] == parameter_def_name:
                set_value(numerical_parameter, value)
                return

    def upgrade_module(self, bsw_ecuc_autosar_class, autosar_ib_class, mod_autosar_dict, sys_info_dict) -> bool:
        # 获取CanTp模块的源数据
        print("upgrade CanTp module")
        CanTpClass = bsw_ecuc_autosar_class["CanTp"]

        # 查找并存储CanTpMainFunctionPeriod的值
        if CanTpClass.find("/ActiveEcuC/CanTp/CanTpConfig").PARAMETER_VALUES == None:
            return
        CanTpConfigParam = CanTpClass.find("/ActiveEcuC/CanTp/CanTpConfig").PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE
        CanTpConfigContainer = CanTpClass.find("/ActiveEcuC/CanTp/CanTpConfig").SUB_CONTAINERS.ECUC_CONTAINER_VALUE
        UpdateCanTpMainfunction = True
        main_function_period = None

        for param in CanTpConfigParam:
            if get_definition_ref(param) == "/AUTOSAR/EcucDefs/CanTp/CanTpConfig/CanTpMainFunctionPeriod":
                main_function_period = get_value(param)
                # AutosarUtil.remove_value_from_list(CanTpConfigParam, param)
                if main_function_period == None or main_function_period == "":
                    main_function_period = "0.005"

        for container in CanTpConfigContainer:
            if get_definition_ref(container) == "/AUTOSAR/EcucDefs/CanTp/CanTpConfig/CanTpMainFunction":
                UpdateCanTpMainfunction = False

        # 如果没有找到CanTpMainFunction容器，则创建新的容器
        if UpdateCanTpMainfunction == True:
            CanTpConfigDict = mod_autosar_dict["CanTp"].find("/AUTOSAR/EcucDefs/CanTp/CanTpConfig/CanTpMainFunction")
            CanTpMainFunction_Containner = AutosarUtil.create_sub_container_value("CanTp_MainFunction", CanTpConfigDict, CanTpClass.find("/ActiveEcuC/CanTp/CanTpConfig"))
            self.set_numerical_param_value('CanTpMainFunctionPeriod', CanTpMainFunction_Containner, main_function_period)

