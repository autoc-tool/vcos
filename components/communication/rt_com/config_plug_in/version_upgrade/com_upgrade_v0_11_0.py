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
# ##############################################################################from basic_func_module.autosar_utils.utils import *
from basic_func_module.autosar_utils.autosar_util import AutosarUtil
from func_module.health_monitor.logger import logger


class ComUpgradeV0110():

    def __init__(self):
        pass

    def set_numerical_param_value(self, parameter_def_name, container_obj, value):
        numerical_param_list = container_obj.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE
        for numerical_parameter in numerical_param_list:
            if get_definition_ref(numerical_parameter).split("/")[-1] == parameter_def_name:
                set_value(numerical_parameter, value)
                return
            
    def upgrade_module(self, bsw_ecuc_autosar_class, autosar_ib_class, mod_autosar_dict, sys_info_dict) -> bool:
        print("upgrade Com module")
        ComClass = bsw_ecuc_autosar_class["Com"]
        ComConfigSet = ComClass.find("/ActiveEcuC/Com/ComConfig").SUB_CONTAINERS.ECUC_CONTAINER_VALUE
        UpdateRxMainfunction = True
        UpdateTxMainfunction = True
        UpdateGwMainfunction = True
        # IPDU IPduMainFunctionRef
        for ele in ComConfigSet:
            if get_definition_ref(ele) == "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu":
                comipduref = ele.REFERENCE_VALUES.ECUC_REFERENCE_VALUE + ele.REFERENCE_VALUES.ECUC_INSTANCE_REFERENCE_VALUE + ele.PARAMETER_VALUES.ECUC_ADD_INFO_PARAM_VALUE + ele.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE + ele.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE
                IPduMainFunctionRefValue = ""
                for comipduref_sub in comipduref:
                    if get_definition_ref(comipduref_sub) == "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComIPduDirection":
                        if get_value(comipduref_sub) == "RECEIVE":
                            IPduMainFunctionRefValue = "/ActiveEcuC/Com/ComConfig/ComMainFunctionRx"
                        if get_value(comipduref_sub) == "SEND":
                            IPduMainFunctionRefValue = "/ActiveEcuC/Com/ComConfig/ComMainFunctionTx" 
                
                for comipduref_sub in comipduref:
                    if get_definition_ref(comipduref_sub) == "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/IPduMainFunctionRef" and get_value_ref(comipduref_sub) == None:
                        if IPduMainFunctionRefValue != "":
                            set_value_ref(comipduref_sub, IPduMainFunctionRefValue)

            if get_definition_ref(ele) == "/AUTOSAR/EcucDefs/Com/ComConfig/ComMainFunctionRx":
                UpdateRxMainfunction = False
            if get_definition_ref(ele) == "/AUTOSAR/EcucDefs/Com/ComConfig/ComMainFunctionTx":
                UpdateTxMainfunction = False
            if get_definition_ref(ele) == "/AUTOSAR/EcucDefs/Com/ComConfig/ComMainFunctionGw":
                UpdateGwMainfunction = False

        # 创建mainfunction容器
        if UpdateRxMainfunction == True:
            ComConfigDict = mod_autosar_dict["Com"].find("/AUTOSAR/EcucDefs/Com/ComConfig/ComMainFunctionRx")
            ComMainFunctionRx_Containner = AutosarUtil.create_sub_container_value("ComMainFunctionRx", ComConfigDict, ComClass.find("/ActiveEcuC/Com/ComConfig"))
            self.set_numerical_param_value('MainRxTimeBase', ComMainFunctionRx_Containner, "0.005")

        if UpdateTxMainfunction == True:
            ComConfigDict = mod_autosar_dict["Com"].find("/AUTOSAR/EcucDefs/Com/ComConfig/ComMainFunctionTx")
            ComMainFunctionTx_Containner = AutosarUtil.create_sub_container_value("ComMainFunctionTx", ComConfigDict, ComClass.find("/ActiveEcuC/Com/ComConfig"))
            self.set_numerical_param_value('MainTxTimeBase', ComMainFunctionTx_Containner, "0.005")

        if UpdateGwMainfunction == True:
            ComConfigDict = mod_autosar_dict["Com"].find("/AUTOSAR/EcucDefs/Com/ComConfig/ComMainFunctionGw")
            ComMainFunctionGw_Containner = AutosarUtil.create_sub_container_value("ComMainFunctionGw", ComConfigDict, ComClass.find("/ActiveEcuC/Com/ComConfig"))
            self.set_numerical_param_value('MainGwTimeBase', ComMainFunctionGw_Containner, "0.005")