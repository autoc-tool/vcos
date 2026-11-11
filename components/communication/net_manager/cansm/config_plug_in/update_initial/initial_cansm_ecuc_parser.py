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


class InitialCanSMEcucParser():
    """
    All CanNm module page jump, display, save logic code.
    """

    def __init__(self, mod_autosar_dict):
        self.mod_autosar_dict = mod_autosar_dict

    def update_initial_ecuc_from_dbc_ldf(self, initial_autosar, dict):
        """
        Update the tree structure of the CanNm module based on the imported DBC file and update the data.
        Arg:
            dict : All config dictionary
        """
        self.update_the_cansm_configuration(initial_autosar, dict)

    def update_the_cansm_configuration(self, initial_autosar, dict):
        """
        update the cansm tree by DBC.
        Arg:
            config_dict : All config dictionary
        """
        dbc_info_dict = self.process_dict_extract_dbc_info(dict)
        dbcCoreList = self.process_dict_core_info(dict)
        for _, info in dbc_info_dict.items():
            network_idx = info["idx"]
            ecuc_def = self.mod_autosar_dict["CanSM"]
            configuration = initial_autosar.find(f"/InitialEcuC/CanSM/CanSMConfiguration")
            network_def = ecuc_def.find("/AUTOSAR/EcucDefs/CanSM/CanSMConfiguration/CanSMManagerNetwork")
            network_name = "CanSMManagerNetwork" if network_idx == 0 else f"CanSMManagerNetwork_{network_idx}"
            container_value = AutosarUtil.create_sub_container_value(network_name, network_def, configuration)
            self.fill_dbc_info_into_new_container(container_value, info, initial_autosar)
        # CanSMMainFunction
        if len(dbcCoreList) == 1 and dbcCoreList[0] == 0:
            container_def = "/AUTOSAR/EcucDefs/CanSM/CanSMConfiguration/CanSMMainFunction"
            cur_conf_def = ecuc_def.find(container_def)
            container_name = "CanSM_MainFunction"
            container_value = AutosarUtil.create_sub_container_value(container_name, cur_conf_def, configuration)
        else:
            for coreid in dbcCoreList:
                container_def = "/AUTOSAR/EcucDefs/CanSM/CanSMConfiguration/CanSMMainFunction"
                cur_conf_def = ecuc_def.find(container_def)
                container_name = "CanSM_MainFunction_Core" + str(coreid)
                container_value = AutosarUtil.create_sub_container_value(container_name, cur_conf_def, configuration)

    def process_dict_extract_dbc_info(self, dict):
        ret_dict = {}
        dbcs_dict = dict['dbc']
        idx = 0
        for itemValue in dbcs_dict:
            networkName = itemValue["attributes"]["network"]["define_value"]["DBName"]

            dbc_dict = {
                'idx': idx,
                'CanSMComMNetworkHandleRef': '/ActiveEcuC/ComM/ComMConfigSet/' + networkName,
                'CanSMControllerId': '/ActiveEcuC/CanIf/CanIfCtrlDrvCfg/' + networkName
            }
            idx = idx + 1
            ret_dict[itemValue['dbc_name']] = dbc_dict
        return ret_dict

    def process_dict_core_info(self, dict):
        dbcList = dict['dbc']
        coreidList = list()
        for dbc in dbcList:
            if "coreId" not in dbc:
                coreid = 0
            elif dbc['coreId'] == None:
                coreid = 0
            else:
                coreid = dbc['coreId']
            coreidList.append(coreid)
        outputList = list(set(coreidList))
        return outputList

    def fill_dbc_info_into_new_container(self, parent_container, info, initial_autosar):
        # CanSMComMNetworkHandleRef
        container_def = "/AUTOSAR/EcucDefs/CanSM/CanSMConfiguration/CanSMManagerNetwork/CanSMComMNetworkHandleRef"
        for para in parent_container.REFERENCE_VALUES.ECUC_REFERENCE_VALUE:
            def_ref = get_definition_ref(para)
            if def_ref == container_def:
                set_value_ref(para, info['CanSMComMNetworkHandleRef'])
                AutosarUtil.add_annotation_value(para, "AutoDerived")

        # CanSMController
        ecuc_def = self.mod_autosar_dict["CanSM"]
        container_def = "/AUTOSAR/EcucDefs/CanSM/CanSMConfiguration/CanSMManagerNetwork/CanSMController"
        CanSMControllerId_def = "/AUTOSAR/EcucDefs/CanSM/CanSMConfiguration/CanSMManagerNetwork/CanSMController/CanSMControllerId"
        cur_conf_def = ecuc_def.find(container_def)
        container_name = "CanSMController"
        container_value = AutosarUtil.create_sub_container_value(container_name, cur_conf_def, parent_container)
        if hasattr(container_value.REFERENCE_VALUES, "ECUC_REFERENCE_VALUE"):
            for para in container_value.REFERENCE_VALUES.ECUC_REFERENCE_VALUE:
                def_ref = get_definition_ref(para)
                if def_ref == CanSMControllerId_def:
                    set_value_ref(para, info['CanSMControllerId'])
                    AutosarUtil.add_annotation_value(para, "AutoDerived")