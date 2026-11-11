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
Cfg updater for Dcm module.

Update Dcm cfg info according to imported configuration file, e.g. *.cdd
"""
import os
from math import ceil
from typing import Dict
from functools import wraps
from func_module.health_monitor.logger import logger
from basic_func_module.autosar_utils.utils import get_definition_ref, set_value, set_value_ref, get_value_ref
from basic_func_module.autosar_utils.autosar_util import AutosarUtil
from func_module.module_management.arxml_handler.arxml_parse_save import ArxmlParseHandler
from func_module.module_management.arxml_handler.config_comparator import ConfigComparator

CUR_DIR = os.path.dirname(__file__)


def update_short_name_length(short_name) -> str:
    if short_name and len(short_name) > 90:
        short_name = short_name[:90]
    return short_name


class DcmCfgConverter():

    def __init__(self, base_arxml_path) -> None:
        self.base_arxml_path = base_arxml_path
        pass

    def set_numerical_param_value(self, parameter_def_name, container_obj, value):
        numerical_param_list = container_obj.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE
        for numerical_parameter in numerical_param_list:
            if get_definition_ref(numerical_parameter).split("/")[-1] == parameter_def_name:
                set_value(numerical_parameter, value)
                return
        # print(f"Cannot find {parameter_def_name}")

    def remove_reference_value(self, reference_def_name, container_obj, value):
        reference_list = container_obj.REFERENCE_VALUES.ECUC_REFERENCE_VALUE
        for reference_parameter in reference_list:
            if get_definition_ref(reference_parameter).split("/")[-1] == reference_def_name and get_value_ref(
                    reference_parameter) == value:
                reference_list.remove(reference_parameter)
                break

    def add_reference_value(self, reference_def, container_obj, value):
        parent_node = container_obj.REFERENCE_VALUES
        reference_val_obj = AutosarUtil.set_reference(parent_node, reference_def)
        set_value_ref(reference_val_obj, value)

    def set_reference_value(self, reference_def_name, container_obj, value):
        reference_list = container_obj.REFERENCE_VALUES.ECUC_REFERENCE_VALUE
        for reference_parameter in reference_list:
            if get_definition_ref(reference_parameter).split("/")[-1] == reference_def_name:
                set_value_ref(reference_parameter, value)
                return
        # print(f"Cannot find {reference_def_name}")

    def set_textual_param_value(self, parameter_def_name, container_obj, value):
        textual_param_list = container_obj.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE
        for textual_parameter in textual_param_list:
            if get_definition_ref(textual_parameter).split("/")[-1] == parameter_def_name:
                set_value(textual_parameter, value)
                return
        # print(f"Cannot find {parameter_def_name}")


class DcmCfgConverterWithCdd(DcmCfgConverter):

    def __init__(self,
                 base_arxml_path,
                 merge_signal_flag,
                 db_cdd_dict,
                 all_excel_dict,
                 mod_autosar_dict,
                 ecuc_autosar_dict,
                 system_autosar_dict=None) -> None:
        super().__init__(base_arxml_path)
        self.merge_signal_flag = merge_signal_flag
        self.db_cdd_dict: Dict = db_cdd_dict
        # Now only support import one cdd file
        self.cdd_info = list(self.db_cdd_dict.values())[0]
        self.mod_autosar_dict: Dict = mod_autosar_dict
        self.ecuc_autosar_dict: Dict = ecuc_autosar_dict
        self.system_autosar_dict = system_autosar_dict
        self.container_checker_enable = True

    def check_user_define(self, obj):
        for annotation in obj.ANNOTATIONS.ANNOTATION:
            if annotation.ANNOTATION_ORIGIN.valueOf_ == "UserDefined":
                return True

        return False

    def container_checker(create_func):

        @wraps(create_func)
        def checker(self: "DcmCfgConverterWithCdd", *args, **kargs):
            if self.container_checker_enable:
                parent_obj = kargs["parent_obj"]
                container_name = kargs["container_name"]
                parent_path = parent_obj.get_path()
                exist_container = parent_obj.find(f"{parent_path}/{container_name}")
                # 如果在旧ecuc中有此container
                if exist_container:
                    pass
                else:
                    pass
            create_func(self, *args, **kargs)

        return checker

    def filter_service_execute_permission(self, state_group_list, execute_permission_code):
        security_info_dict = {}
        session_info_dict = {}
        # Interate state group to reason state executable info
        for state_group in state_group_list:
            if state_group.spec == "session":
                for state in state_group.state_list:
                    executability = execute_permission_code[state.number - 1]
                    try:
                        session_info_dict[state.name] = str(executability == "1").upper() if state.used else "FALSE"
                    except KeyError:
                        logger.error(f"Unsupported session: {state.name}")
            elif state_group.spec == "security":
                for state in state_group.state_list:
                    executability = execute_permission_code[state.number - 1]
                    try:
                        # If the state used or is Locked state, set reference for the state according to actual config
                        if state.used or state.name == "Locked":
                            security_info_dict[state.name] = str(executability == "1").upper()
                        # Else set the reference for this state to false
                        else:
                            security_info_dict[state.name] = "FALSE"
                    except KeyError:
                        logger.error(f"Unsupported security level: {state.name}")
            else:
                logger.info(f"Unsporrted state group {state_group.name}")

        return session_info_dict, security_info_dict

    def create_level_reference_param(self, reference_def_name, level_info_dict, level_ref_path, container_def,
                                     container_val):
        permitted_flag = False
        for session_name, session_permission in level_info_dict.items():
            if session_permission == "TRUE":
                permitted_flag = True
                # Set f"{reference_def_name}" value
                default_session_path = f"{level_ref_path}/{session_name}"
                reference_def = self.mod_autosar_dict["Dcm"].find(container_def.get_path() + f"/{reference_def_name}")
                self.add_reference_value(reference_def, container_val, default_session_path)
        # Remove the reference item with None value if it has actually level ref
        if permitted_flag:
            self.remove_reference_value(reference_def_name, container_val, None)

    def create_dcm_dsd_subservice(self, parent_obj, subservice_info):
        container_def = self.mod_autosar_dict["Dcm"].find(
            "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsd/DcmDsdServiceTable/DcmDsdService/DcmDsdSubService")
        container_val = AutosarUtil.create_sub_container_value(subservice_info.name, container_def, parent_obj)
        # Set "DcmDsdSubServiceId" value
        self.set_numerical_param_value('DcmDsdSubServiceId', container_val, subservice_info.subservice_id)

        session_info_dict, security_info_dict = self.filter_service_execute_permission(
            self.cdd_info.state_groups, subservice_info.execute_permission)

        # Set "DcmDsdSubServiceSessionLevelRef" value
        reference_def_name = "DcmDsdSubServiceSessionLevelRef"
        level_ref_path = "/ActiveEcuC/Dcm/DcmConfigSet/DcmDsp/DcmDspSession"
        self.create_level_reference_param(reference_def_name, session_info_dict, level_ref_path, container_def,
                                          container_val)

        # Set "DcmDsdSubServiceSecurityLevelRef" value
        reference_def_name = "DcmDsdSubServiceSecurityLevelRef"
        level_ref_path = "/ActiveEcuC/Dcm/DcmConfigSet/DcmDsp/DcmDspSecurity"
        self.create_level_reference_param(reference_def_name, security_info_dict, level_ref_path, container_def,
                                          container_val)

    def create_dcm_dsd_service(self, parent_obj, service_info):
        container_def = self.mod_autosar_dict["Dcm"].find(
            "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsd/DcmDsdServiceTable/DcmDsdService")
        sercice_name = service_info.name
        if int(service_info.service_id) < 0x10:
            sercice_name = "ProcessOBDService" + '{:02X}'.format(int(service_info.service_id))
        container_val = AutosarUtil.create_sub_container_value(sercice_name, container_def, parent_obj)
        # Set "DcmDsdSidTabServiceId" value
        self.set_numerical_param_value('DcmDsdSidTabServiceId', container_val, service_info.service_id)

        # Get the execute permission of the uds service
        session_info_dict, security_info_dict = self.filter_service_execute_permission(
            self.cdd_info.state_groups, service_info.execute_permission)

        # Set "DcmDsdSidTabSessionLevelRef" value
        reference_def_name = "DcmDsdSidTabSessionLevelRef"
        level_ref_path = "/ActiveEcuC/Dcm/DcmConfigSet/DcmDsp/DcmDspSession"
        self.create_level_reference_param(reference_def_name, session_info_dict, level_ref_path, container_def,
                                          container_val)

        # Set "DcmDsdSidTabSecurityLevelRef" value
        reference_def_name = "DcmDsdSidTabSecurityLevelRef"
        level_ref_path = "/ActiveEcuC/Dcm/DcmConfigSet/DcmDsp/DcmDspSecurity"
        self.create_level_reference_param(reference_def_name, security_info_dict, level_ref_path, container_def,
                                          container_val)

        sub_func_avail = service_info.sub_func_avail
        if sub_func_avail:
            # Set "DcmDsdSidTabSubfuncAvail" value to true
            self.set_numerical_param_value('DcmDsdSidTabSubfuncAvail', container_val, "true")
            for subservice in service_info.subservices.values():
                self.create_dcm_dsd_subservice(container_val, subservice)
        else:
            # Set "DcmDsdSidTabSubfuncAvail" value to false
            self.set_numerical_param_value('DcmDsdSidTabSubfuncAvail', container_val, "false")

    def create_dcm_dsd_service_table(self, parent_obj):
        container_def = self.mod_autosar_dict["Dcm"].find(
            "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsd/DcmDsdServiceTable")
        container_val = AutosarUtil.create_sub_container_value("DcmDsdServiceTable", container_def, parent_obj)
        # Set "DcmDsdSidTabId" value
        self.set_numerical_param_value('DcmDsdSidTabId', container_val, '0')
        dsd_service_list = self.cdd_info.uds_services
        dsd_service_list.sort(key=lambda serivce: int(serivce.service_id))
        for service in dsd_service_list:
            self.create_dcm_dsd_service(container_val, service)
        pass

    def create_dcm_dsd(self, parent_obj):
        container_def = self.mod_autosar_dict["Dcm"].find("/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsd")
        container_val = AutosarUtil.create_sub_container_value("DcmDsd", container_def, parent_obj)
        self.create_dcm_dsd_service_table(container_val)

    def create_dcm_dsp_security_row(self, parent_obj, security_level_info):
        name_to_level_map = {"Locked": "0", "UnlockedL1": "1", "Unlocked_L2": "5", "Level_1": "1", "Level_2": "5"}
        if security_level_info.name in name_to_level_map.keys():
            security_level_number = name_to_level_map[security_level_info.name]
        elif "1" in security_level_info.name:
            security_level_number = "1"
        elif "2" in security_level_info.name:
            security_level_number = "5"
        else:
            security_level_number = "0"

        container_def = self.mod_autosar_dict["Dcm"].find(
            "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspSecurity/DcmDspSecurityRow")
        container_val = AutosarUtil.create_sub_container_value(security_level_info.name, container_def, parent_obj)

        # Set "DcmDspSecurityLevel" value
        self.set_numerical_param_value('DcmDspSecurityLevel', container_val, security_level_number)
        if security_level_info.name == "Locked":
            # Set "DcmDspSecurityUsePort" value
            self.set_textual_param_value('DcmDspSecurityUsePort', container_val, "USE_ASYNCH_FNC")
            AutosarUtil.add_annotation_value(container_val, "InVisible")
        else:
            key_size = str(
                int(security_level_info.request_seed_service_info['pos']['comp_list'][2]['data'][0].length / 8))
            seed_size = str(int(security_level_info.send_key_service_info['req']['comp_list'][2]['data'][0].length / 8))
            # Set "DcmDspSecurityKeySize" value
            self.set_numerical_param_value('DcmDspSecurityKeySize', container_val, key_size)
            # Set "DcmDspSecuritySeedSize" value
            self.set_numerical_param_value('DcmDspSecuritySeedSize', container_val, seed_size)  #底版修改默认值4

    def create_dcm_dsp_security(self, parent_obj):
        container_def = self.mod_autosar_dict["Dcm"].find("/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspSecurity")
        container_val = AutosarUtil.create_sub_container_value("DcmDspSecurity", container_def, parent_obj)

        state_groups = self.cdd_info.state_groups
        security_level_info_list = list(filter(lambda state_group: state_group.spec == "security",
                                               state_groups))[0].state_list
        for security_level_info in security_level_info_list:
            if security_level_info.used or security_level_info.name == "Locked":
                self.create_dcm_dsp_security_row(container_val, security_level_info)

    def create_dcm_dsp_session_row(self, parent_obj, session_level_info):
        name_to_level_map = {"Default": "1", "Programming": "2", "Extended": "3"}
        container_def = self.mod_autosar_dict["Dcm"].find(
            "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspSession/DcmDspSessionRow")
        container_val = AutosarUtil.create_sub_container_value(session_level_info.name, container_def, parent_obj)

        activated_interface = [{
            "name": key,
            "pos": val["pos"]
        } for key, val in self.cdd_info.interface_param.items() if val["v"] == "1"]
        if len(activated_interface) > 0:
            used_interface = sorted(activated_interface, key=lambda x: int(x["pos"]))[0]["name"]
            used_interface_param = self.cdd_info.interface_param[used_interface]

            # Set "DcmDspSessionP2ServerMax" value
            interface_param_name = "P2Server" if used_interface != "DoIP_ISO13400" else "CP_P2Max_Ecu"
            self.set_numerical_param_value('DcmDspSessionP2ServerMax', container_val,
                                           str(float(used_interface_param[interface_param_name]) / 1000))
            # Set "DcmDspSessionP2StarServerMax" value
            interface_param_name = "P2ExServer" if used_interface != "DoIP_ISO13400" else "CP_P2Star_Ecu"
            self.set_numerical_param_value('DcmDspSessionP2StarServerMax', container_val,
                                           str(float(used_interface_param[interface_param_name]) / 1000))
            # Set "DcmDspSessionLevel" value
            self.set_numerical_param_value('DcmDspSessionLevel', container_val,
                                           name_to_level_map[session_level_info.name])
            # Set "DcmDspSessionForBoot" value
            # self.set_textual_param_value('DcmDspSessionForBoot', container_val, "DCM_NO_BOOT")  #底版修改默认值DCM_NO_BOOT

    def create_dcm_dsp_session(self, parent_obj):
        container_def = self.mod_autosar_dict["Dcm"].find("/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspSession")
        container_val = AutosarUtil.create_sub_container_value("DcmDspSession", container_def, parent_obj)

        state_groups = self.cdd_info.state_groups
        session_level_info_list = list(filter(lambda state_group: state_group.spec == "session",
                                              state_groups))[0].state_list
        for session_level_info in session_level_info_list:
            if session_level_info.used:
                self.create_dcm_dsp_session_row(container_val, session_level_info)

    def create_dcm_dsp_did_read(self, parent_obj, read_service_info):
        container_def = self.mod_autosar_dict["Dcm"].find(
            "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspDidInfo/DcmDspDidRead")
        container_val = AutosarUtil.create_sub_container_value("DcmDspDidRead", container_def, parent_obj)

        # Get the execute permission of the read service
        session_info_dict, security_info_dict = self.filter_service_execute_permission(
            self.cdd_info.state_groups, read_service_info["executability"])

        # Set "DcmDspDidReadSecurityLevelRef" value
        reference_def_name = "DcmDspDidReadSecurityLevelRef"
        level_ref_path = "/ActiveEcuC/Dcm/DcmConfigSet/DcmDsp/DcmDspSecurity"
        self.create_level_reference_param(reference_def_name, security_info_dict, level_ref_path, container_def,
                                          container_val)

        # Set "DcmDspDidReadSessionLevelRef" value
        reference_def_name = "DcmDspDidReadSessionRef"
        level_ref_path = "/ActiveEcuC/Dcm/DcmConfigSet/DcmDsp/DcmDspSession"
        self.create_level_reference_param(reference_def_name, session_info_dict, level_ref_path, container_def,
                                          container_val)

    def create_dcm_dsp_did_write(self, parent_obj, write_service_info):
        container_def = self.mod_autosar_dict["Dcm"].find(
            "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspDidInfo/DcmDspDidWrite")
        container_val = AutosarUtil.create_sub_container_value("DcmDspDidWrite", container_def, parent_obj)

        # Get the execute permission of the read service
        session_info_dict, security_info_dict = self.filter_service_execute_permission(
            self.cdd_info.state_groups, write_service_info["executability"])

        # Set "DcmDspDidWriteSecurityLevelRef" value
        reference_def_name = "DcmDspDidWriteSecurityLevelRef"
        level_ref_path = "/ActiveEcuC/Dcm/DcmConfigSet/DcmDsp/DcmDspSecurity"
        self.create_level_reference_param(reference_def_name, security_info_dict, level_ref_path, container_def,
                                          container_val)

        # Set "DcmDspDidWriteSessionRef" value
        reference_def_name = "DcmDspDidWriteSessionRef"
        level_ref_path = "/ActiveEcuC/Dcm/DcmConfigSet/DcmDsp/DcmDspSession"
        self.create_level_reference_param(reference_def_name, session_info_dict, level_ref_path, container_def,
                                          container_val)

    def create_dcm_dsp_did_control(self, parent_obj, return_control_service_info, reset_service_info,
                                   short_term_adjustment_info, freeze_info):
        container_def = self.mod_autosar_dict["Dcm"].find(
            "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspDidInfo/DcmDspDidControl")
        container_val = AutosarUtil.create_sub_container_value("DcmDspDidControl", container_def, parent_obj)

        info_tuple = (return_control_service_info, reset_service_info, short_term_adjustment_info, freeze_info)
        executability_str = "0"
        for service_info in info_tuple:
            if service_info["executability"]:
                state_count = len(service_info["executability"])
                executability_digit = int(executability_str, 2) | int(service_info["executability"], 2)
                executability_str = bin(executability_digit)[2:].zfill(state_count)
        # Get the execute permission of the read service
        session_info_dict, security_info_dict = self.filter_service_execute_permission(
            self.cdd_info.state_groups, executability_str)

        if freeze_info["access"]:
            # Set "DcmDspDidFreezeCurrentState" value
            self.set_numerical_param_value('DcmDspDidFreezeCurrentState', container_val, "true")
        if reset_service_info["access"]:
            # Set "DcmDspDidResetToDefault" value
            self.set_numerical_param_value('DcmDspDidResetToDefault', container_val, "true")
        if short_term_adjustment_info["access"]:
            # Set "DcmDspDidShortTermAdjustment" value
            self.set_numerical_param_value('DcmDspDidShortTermAdjustment', container_val, "true")

        # Set "DcmDspDidControlSecurityLevelRef" value
        reference_def_name = "DcmDspDidControlSecurityLevelRef"
        level_ref_path = "/ActiveEcuC/Dcm/DcmConfigSet/DcmDsp/DcmDspSecurity"
        self.create_level_reference_param(reference_def_name, security_info_dict, level_ref_path, container_def,
                                          container_val)

        # Set "DcmDspDidControlSessionRef" value
        reference_def_name = "DcmDspDidControlSessionRef"
        level_ref_path = "/ActiveEcuC/Dcm/DcmConfigSet/DcmDsp/DcmDspSession"
        self.create_level_reference_param(reference_def_name, session_info_dict, level_ref_path, container_def,
                                          container_val)

    def create_dcm_dsp_did_info(self, parent_obj, did_info):
        container_def = self.mod_autosar_dict["Dcm"].find("/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspDidInfo")
        did_name = 'DidInfo_0x' + '{:04X}'.format(did_info.identifier) + '_' + did_info.name
        container_val = AutosarUtil.create_sub_container_value(did_name, container_def, parent_obj)

        read_service_info = did_info.service_info["Read"]
        write_service_info = did_info.service_info["Write"]
        return_control_service_info = did_info.service_info["ReturnControl"]
        reset_service_info = did_info.service_info["ResetDefault"]
        short_term_adjustment_info = did_info.service_info["ShortTermAdjustment"]
        freeze_info = did_info.service_info["Freeze"]
        if read_service_info["access"]:
            # DcmDspDidRead
            self.create_dcm_dsp_did_read(container_val, read_service_info)
        if write_service_info["access"]:
            # DcmDspDidWrite
            self.create_dcm_dsp_did_write(container_val, write_service_info)
        if return_control_service_info["access"] or reset_service_info["access"] or short_term_adjustment_info[
                "access"] or freeze_info["access"]:
            # DcmDspDidControl
            self.create_dcm_dsp_did_control(container_val, return_control_service_info, reset_service_info,
                                            short_term_adjustment_info, freeze_info)

    def create_dcm_dsp_did_signal(self, parent_obj, did_info, data_name, offset):
        container_def = self.mod_autosar_dict["Dcm"].find(
            "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspDid/DcmDspDidSignal")
        container_val = AutosarUtil.create_sub_container_value(data_name, container_def, parent_obj)

        # Set "DcmDspDidByteOffset" value
        self.set_numerical_param_value('DcmDspDidByteOffset', container_val, str(offset))
        # Set "DcmDspDidDataRef" value
        data_short_name = update_short_name_length('0x' + '{:04X}'.format(did_info.identifier) + '_' + did_info.name +
                                                   '_' + data_name)
        ref_did_info_path = f"/ActiveEcuC/Dcm/DcmConfigSet/DcmDsp/{data_short_name}"
        self.set_reference_value('DcmDspDidDataRef', container_val, ref_did_info_path)

    def did_data_pre_process(self, datas_obj):
        data_dict_list = []
        if self.merge_signal_flag:
            length = 0
            for data in datas_obj:
                length += ceil(data.length / 8)
            data_dict = {}
            data_dict['name'] = 'MergedData'
            data_dict['type'] = 'UINT8_N' if length > 1 else 'UINT8'
            data_dict['length'] = length
            data_dict_list.append(data_dict)
        else:
            for data in datas_obj:
                data_dict = {}
                data_dict['name'] = data.name
                data_dict['type'] = data.data_type.data_type_str
                data_dict['length'] = ceil(data.length / 8)
                data_dict_list.append(data_dict)
        return data_dict_list

    def routine_signal_pre_process(self, datas_obj, is_req_data):
        signal_dict_list = []
        if self.merge_signal_flag:
            length = 0
            for signal in datas_obj:
                length += ceil(signal.length / 8) * 8
            signal_dict = {}
            signal_dict['name'] = 'ReqData' if is_req_data else 'ResData'
            signal_dict['type'] = 'UINT8_N' if length > 8 else 'UINT8'
            signal_dict['length'] = length
            signal_dict_list.append(signal_dict)
        else:
            for signal in datas_obj:
                signal_dict = {}
                signal_dict['name'] = signal.name
                signal_dict['type'] = signal.data_type.data_type_str
                signal_dict['length'] = ceil(signal.length / 8) * 8
                signal_dict_list.append(signal_dict)

        return signal_dict_list

    def create_dcm_dsp_did(self, parent_obj, did_info, data_dict_list):
        container_def = self.mod_autosar_dict["Dcm"].find("/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspDid")
        did_name = '0x' + '{:04X}'.format(did_info.identifier) + '_' + did_info.name
        container_val = AutosarUtil.create_sub_container_value(did_name, container_def, parent_obj)

        # Set "DcmDspDidUsed" value
        self.set_numerical_param_value('DcmDspDidUsed', container_val, "true")
        # Set "DcmDspDidIdentifier" value
        self.set_numerical_param_value('DcmDspDidIdentifier', container_val, str(did_info.identifier))
        # Set "DcmDspDidSize" value
        self.set_numerical_param_value('DcmDspDidSize', container_val, str(did_info.length))
        # Set "DcmDspDidInfoRef" value
        did_info_name = 'DidInfo_0x' + '{:04X}'.format(did_info.identifier) + '_' + did_info.name
        ref_did_info_path = f"/ActiveEcuC/Dcm/DcmConfigSet/DcmDsp/{did_info_name}"
        self.set_reference_value('DcmDspDidInfoRef', container_val, ref_did_info_path)

        offset = 0
        for data in data_dict_list:
            self.create_dcm_dsp_did_signal(container_val, did_info, data['name'], offset)
            offset += data['length']

    def create_dcm_dsp_data_info(self, parent_obj, did_info, data_name):
        container_def = self.mod_autosar_dict["Dcm"].find("/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspDataInfo")
        data_info_name = update_short_name_length('DataInfo_0x' + '{:04X}'.format(did_info.identifier) + '_' +
                                                  did_info.name + '_' + data_name)
        AutosarUtil.create_sub_container_value(data_info_name, container_def, parent_obj)

    def create_dcm_dsp_data(self, parent_obj, did_info, data_info):
        container_def = self.mod_autosar_dict["Dcm"].find("/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspData")
        data_name = update_short_name_length('0x' + '{:04X}'.format(did_info.identifier) + '_' + did_info.name + '_' +
                                             data_info['name'])
        container_val = AutosarUtil.create_sub_container_value(data_name, container_def, parent_obj)

        # Set "DcmDspDataByteSize" value
        self.set_numerical_param_value('DcmDspDataByteSize', container_val, str(data_info['length']))
        # Set "DcmDspDataEndianness" value
        self.set_textual_param_value('DcmDspDataEndianness', container_val, "OPAQUE")
        # Set "DcmDspDataType" value
        self.set_textual_param_value('DcmDspDataType', container_val, data_info['type'])
        # Set "DcmDspDataUsePort" value
        self.set_textual_param_value('DcmDspDataUsePort', container_val, "USE_DATA_ASYNCH_CLIENT_SERVER")
        # Set "DcmDspDataInfoRef" value
        data_info_name = update_short_name_length('DataInfo_0x' + '{:04X}'.format(did_info.identifier) + '_' +
                                                  did_info.name + '_' + data_info['name'])
        ref_data_info_path = f"/ActiveEcuC/Dcm/DcmConfigSet/DcmDsp/{data_info_name}"
        self.set_reference_value('DcmDspDataInfoRef', container_val, ref_data_info_path)

    def create_dcm_dsp_start_routine_in_signal(self, parent_obj, signal_info, pos):
        container_def = self.mod_autosar_dict["Dcm"].find(
            "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspRoutine/DcmDspStartRoutine/DcmDspStartRoutineIn/DcmDspStartRoutineInSignal"
        )
        container_val = AutosarUtil.create_sub_container_value(signal_info['name'], container_def, parent_obj)

        # Set "DcmDspRoutineParameterSize" value
        self.set_numerical_param_value('DcmDspRoutineParameterSize', container_val, str(int(signal_info['length'] / 8)))
        # Set "DcmDspRoutineSignalPos" value
        self.set_numerical_param_value('DcmDspRoutineSignalPos', container_val, str(int(pos)))
        # Set "DcmDspRoutineSignalEndianness" value
        self.set_textual_param_value('DcmDspRoutineSignalEndianness', container_val, "OPAQUE")
        # Set "DcmDspRoutineSignalType" value
        self.set_textual_param_value('DcmDspRoutineSignalType', container_val, signal_info['type'])

    def create_dcm_dsp_start_routine_out_signal(self, parent_obj, signal_info, pos):
        container_def = self.mod_autosar_dict["Dcm"].find(
            "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspRoutine/DcmDspStartRoutine/DcmDspStartRoutineOut/DcmDspStartRoutineOutSignal"
        )
        container_val = AutosarUtil.create_sub_container_value(signal_info['name'], container_def, parent_obj)

        # Set "DcmDspRoutineParameterSize" value
        self.set_numerical_param_value('DcmDspRoutineParameterSize', container_val, str(int(signal_info['length'] / 8)))
        # Set "DcmDspRoutineSignalPos" value
        self.set_numerical_param_value('DcmDspRoutineSignalPos', container_val, str(int(pos)))
        # Set "DcmDspRoutineSignalEndianness" value
        self.set_textual_param_value('DcmDspRoutineSignalEndianness', container_val, "OPAQUE")
        # Set "DcmDspRoutineSignalType" value
        self.set_textual_param_value('DcmDspRoutineSignalType', container_val, signal_info['type'])

    def create_dcm_dsp_stop_routine_in_signal(self, parent_obj, signal_info, pos):
        container_def = self.mod_autosar_dict["Dcm"].find(
            "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspRoutine/DcmDspStopRoutine/DcmDspStopRoutineIn/DcmDspStopRoutineInSignal"
        )
        container_val = AutosarUtil.create_sub_container_value(signal_info['name'], container_def, parent_obj)

        # Set "DcmDspRoutineParameterSize" value
        self.set_numerical_param_value('DcmDspRoutineParameterSize', container_val, str(int(signal_info['length'] / 8)))
        # Set "DcmDspRoutineSignalPos" value
        self.set_numerical_param_value('DcmDspRoutineSignalPos', container_val, str(int(pos)))
        # Set "DcmDspRoutineSignalEndianness" value
        self.set_textual_param_value('DcmDspRoutineSignalEndianness', container_val, "OPAQUE")
        # Set "DcmDspRoutineSignalType" value
        self.set_textual_param_value('DcmDspRoutineSignalType', container_val, signal_info['type'])

    def create_dcm_dsp_stop_routine_out_signal(self, parent_obj, signal_info, pos):
        container_def = self.mod_autosar_dict["Dcm"].find(
            "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspRoutine/DcmDspStopRoutine/DcmDspStopRoutineOut/DcmDspStopRoutineOutSignal"
        )
        container_val = AutosarUtil.create_sub_container_value(signal_info['name'], container_def, parent_obj)

        # Set "DcmDspRoutineParameterSize" value
        self.set_numerical_param_value('DcmDspRoutineParameterSize', container_val, str(int(signal_info['length'] / 8)))
        # Set "DcmDspRoutineSignalPos" value
        self.set_numerical_param_value('DcmDspRoutineSignalPos', container_val, str(int(pos)))
        # Set "DcmDspRoutineSignalEndianness" value
        self.set_textual_param_value('DcmDspRoutineSignalEndianness', container_val, "OPAQUE")
        # Set "DcmDspRoutineSignalType" value
        self.set_textual_param_value('DcmDspRoutineSignalType', container_val, signal_info['type'])

    def create_dcm_dsp_request_routine_results_out_signal(self, parent_obj, signal_info, pos):
        container_def = self.mod_autosar_dict["Dcm"].find(
            "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspRoutine/DcmDspRequestRoutineResults/" +
            "DcmDspRequestRoutineResultsOut/DcmDspRequestRoutineResultsOutSignal")
        container_val = AutosarUtil.create_sub_container_value(signal_info['name'], container_def, parent_obj)

        # Set "DcmDspRoutineParameterSize" value
        self.set_numerical_param_value('DcmDspRoutineParameterSize', container_val, str(int(signal_info['length'] / 8)))
        # Set "DcmDspRoutineSignalPos" value
        self.set_numerical_param_value('DcmDspRoutineSignalPos', container_val, str(int(pos)))
        # Set "DcmDspRoutineSignalEndianness" value
        self.set_textual_param_value('DcmDspRoutineSignalEndianness', container_val, "OPAQUE")
        # Set "DcmDspRoutineSignalType" value
        self.set_textual_param_value('DcmDspRoutineSignalType', container_val, signal_info['type'])

    def create_dcm_dsp_start_routine_in(self, parent_obj, parameter_info):
        container_def = self.mod_autosar_dict["Dcm"].find(
            "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspRoutine/DcmDspStartRoutine/DcmDspStartRoutineIn")
        container_val = AutosarUtil.create_sub_container_value("DcmDspStartRoutineIn", container_def, parent_obj)

        pos = 0
        signal_dict_list = self.routine_signal_pre_process(parameter_info["datas"], True)
        for signal in signal_dict_list:
            self.create_dcm_dsp_start_routine_in_signal(container_val, signal, pos)
            pos += signal['length']

    def create_dcm_dsp_start_routine_out(self, parent_obj, parameter_info):
        container_def = self.mod_autosar_dict["Dcm"].find(
            "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspRoutine/DcmDspStartRoutine/DcmDspStartRoutineOut")
        container_val = AutosarUtil.create_sub_container_value("DcmDspStartRoutineOut", container_def, parent_obj)

        pos = 0
        signal_dict_list = self.routine_signal_pre_process(parameter_info["datas"], False)
        for signal in signal_dict_list:
            self.create_dcm_dsp_start_routine_out_signal(container_val, signal, pos)
            pos += signal['length']

    def create_dcm_dsp_stop_routine_in(self, parent_obj, parameter_info):
        container_def = self.mod_autosar_dict["Dcm"].find(
            "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspRoutine/DcmDspStopRoutine/DcmDspStopRoutineIn")
        container_val = AutosarUtil.create_sub_container_value("DcmDspStopRoutineIn", container_def, parent_obj)

        pos = 0
        signal_dict_list = self.routine_signal_pre_process(parameter_info["datas"], True)
        for signal in signal_dict_list:
            self.create_dcm_dsp_stop_routine_in_signal(container_val, signal, pos)
            pos += signal['length']

    def create_dcm_dsp_stop_routine_out(self, parent_obj, parameter_info):
        container_def = self.mod_autosar_dict["Dcm"].find(
            "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspRoutine/DcmDspStopRoutine/DcmDspStopRoutineOut")
        container_val = AutosarUtil.create_sub_container_value("DcmDspStopRoutineOut", container_def, parent_obj)

        pos = 0
        signal_dict_list = self.routine_signal_pre_process(parameter_info["datas"], False)
        for signal in signal_dict_list:
            self.create_dcm_dsp_stop_routine_out_signal(container_val, signal, pos)
            pos += signal['length']

    def create_dcm_dsp_request_routine_results_out(self, parent_obj, parameter_info):
        container_def = self.mod_autosar_dict["Dcm"].find(
            "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspRoutine/DcmDspRequestRoutineResults/DcmDspRequestRoutineResultsOut"
        )
        container_val = AutosarUtil.create_sub_container_value("DcmDspRequestRoutineResultsOut", container_def,
                                                               parent_obj)

        pos = 0
        signal_dict_list = self.routine_signal_pre_process(parameter_info["datas"], False)
        for signal in signal_dict_list:
            self.create_dcm_dsp_request_routine_results_out_signal(container_val, signal, pos)
            pos += signal['length']

    def create_dcm_dsp_start_routine(self, parent_obj, routine_info, service_info):
        container_def = self.mod_autosar_dict["Dcm"].find(
            "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspRoutine/DcmDspStartRoutine")
        container_val = AutosarUtil.create_sub_container_value("DcmDspStartRoutine", container_def, parent_obj)

        # Set "DcmDspStartRoutineCommonAuthorizationRef" value
        common_authorization_name = f"CommonAuthorization_{routine_info.name}"
        ref_did_info_path = f"/ActiveEcuC/Dcm/DcmConfigSet/DcmDsp/{common_authorization_name}"
        self.set_reference_value('DcmDspStartRoutineCommonAuthorizationRef', container_val, ref_did_info_path)

        req_param = service_info.parameter_dict["req_param"]
        if req_param and req_param["datas"]:
            self.create_dcm_dsp_start_routine_in(container_val, req_param)
        pos_param = service_info.parameter_dict["pos_param"]
        if pos_param and pos_param["datas"]:
            self.create_dcm_dsp_start_routine_out(container_val, pos_param)

    def create_dcm_dsp_stop_routine(self, parent_obj, routine_info, service_info):
        container_def = self.mod_autosar_dict["Dcm"].find(
            "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspRoutine/DcmDspStopRoutine")
        container_val = AutosarUtil.create_sub_container_value("DcmDspStopRoutine", container_def, parent_obj)

        # Set "DcmDspStopRoutineCommonAuthorizationRef" value
        common_authorization_name = f"CommonAuthorization_{routine_info.name}"
        ref_did_info_path = f"/ActiveEcuC/Dcm/DcmConfigSet/DcmDsp/{common_authorization_name}"
        self.set_reference_value('DcmDspStopRoutineCommonAuthorizationRef', container_val, ref_did_info_path)

        req_param = service_info.parameter_dict["req_param"]
        if req_param and req_param["datas"]:
            self.create_dcm_dsp_stop_routine_in(container_val, req_param)
        pos_param = service_info.parameter_dict["pos_param"]
        if pos_param and pos_param["datas"]:
            self.create_dcm_dsp_stop_routine_out(container_val, pos_param)

    def create_dcm_dsp_request_routine_results(self, parent_obj, routine_info, service_info):
        container_def = self.mod_autosar_dict["Dcm"].find(
            "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspRoutine/DcmDspRequestRoutineResults")
        container_val = AutosarUtil.create_sub_container_value("DcmDspRequestRoutineResults", container_def, parent_obj)

        # Set "DcmDspRequestRoutineResultsCommonAuthorizationRef" value
        common_authorization_name = f"CommonAuthorization_{routine_info.name}"
        ref_did_info_path = f"/ActiveEcuC/Dcm/DcmConfigSet/DcmDsp/{common_authorization_name}"
        self.set_reference_value('DcmDspRequestRoutineResultsCommonAuthorizationRef', container_val, ref_did_info_path)

        pos_param = service_info.parameter_dict["pos_param"]
        if pos_param and pos_param["datas"]:
            self.create_dcm_dsp_request_routine_results_out(container_val, pos_param)

    def create_dcm_dsp_routine(self, parent_obj, routine_info):
        container_def = self.mod_autosar_dict["Dcm"].find("/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspRoutine")
        routine_name = '0x' + '{:04X}'.format(int(routine_info.identifier)) + '_' + routine_info.name
        container_val = AutosarUtil.create_sub_container_value(routine_name, container_def, parent_obj)

        # Set "DcmDspRoutineUsed" value
        self.set_numerical_param_value('DcmDspRoutineUsed', container_val, "true")
        # Set "DcmDspRoutineUsePort" value
        self.set_numerical_param_value('DcmDspRoutineUsePort', container_val, "true")
        # Set "DcmDspRoutineIdentifier" value
        self.set_numerical_param_value('DcmDspRoutineIdentifier', container_val, routine_info.identifier)

        for service in routine_info.service_list:
            # Start routine service
            if service.ssid == "1":
                self.create_dcm_dsp_start_routine(container_val, routine_info, service)
            # Stop routine service
            if service.ssid == "2":
                self.create_dcm_dsp_stop_routine(container_val, routine_info, service)
            # Request routine result service
            if service.ssid == "3":
                self.create_dcm_dsp_request_routine_results(container_val, routine_info, service)

    def create_dcm_dsp_common_authorization(self, parent_obj, routine_info, service_info):
        container_def = self.mod_autosar_dict["Dcm"].find(
            "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspCommonAuthorization")
        common_authorization_name = f"CommonAuthorization_{routine_info.name}"
        container_val = AutosarUtil.create_sub_container_value(common_authorization_name, container_def, parent_obj)

        session_info_dict, security_info_dict = self.filter_service_execute_permission(
            self.cdd_info.state_groups, service_info.execute_permission)
        # Set "DcmDspCommonAuthorizationSecurityLevelRef" value
        reference_def_name = "DcmDspCommonAuthorizationSecurityLevelRef"
        level_ref_path = "/ActiveEcuC/Dcm/DcmConfigSet/DcmDsp/DcmDspSecurity"
        self.create_level_reference_param(reference_def_name, security_info_dict, level_ref_path, container_def,
                                          container_val)

        # Set "DcmDspCommonAuthorizationSessionRef" value
        reference_def_name = "DcmDspCommonAuthorizationSessionRef"
        level_ref_path = "/ActiveEcuC/Dcm/DcmConfigSet/DcmDsp/DcmDspSession"
        self.create_level_reference_param(reference_def_name, session_info_dict, level_ref_path, container_def,
                                          container_val)

    def create_dcm_dsp_pid(self, parent_obj, pid_info):
        is_service_01_exist = '1' in pid_info.service_info.keys()
        is_service_02_exist = '2' in pid_info.service_info.keys()
        pid_name = '{:02X}'.format(int(pid_info.identifier)) + '_' + pid_info.name
        if is_service_01_exist and is_service_02_exist:
            pid_name += '_Read_PID_OBD'
            pid_service = 'DCM_SERVICE_01_02'
            pid_data_list = pid_info.service_info['1']['data_list']
        elif is_service_01_exist:
            pid_name += '_Read_PID_OBD'
            pid_service = 'DCM_SERVICE_01'
            pid_data_list = pid_info.service_info['1']['data_list']
        elif is_service_02_exist:
            pid_name += '_Read_Freeze_Frame_PID_Data'
            pid_service = 'DCM_SERVICE_02'
            pid_data_list = pid_info.service_info['2']['data_list']
        else:
            return

        pid_data_size = 0  #bit
        for data in pid_data_list:
            pid_data_size += data.length

        if (pid_data_size == 0):
            return

        container_def = self.mod_autosar_dict["Dcm"].find("/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspPid")
        container_val = AutosarUtil.create_sub_container_value(pid_name, container_def, parent_obj)

        # Set "DcmDspPidService" value
        self.set_textual_param_value('DcmDspPidService', container_val, pid_service)
        # Set "DcmDspPidIdentifier" value
        self.set_numerical_param_value('DcmDspPidIdentifier', container_val, pid_info.identifier)
        # Set "DcmDspPidSize" value
        self.set_numerical_param_value('DcmDspPidSize', container_val, str(int(pid_data_size / 8)))

        # Create PidData container
        data_name = '0x' + '{:02X}'.format(int(pid_info.identifier)) + '_Pid_Data'
        container_def = self.mod_autosar_dict["Dcm"].find(
            "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspPid/DcmDspPidData")
        container_val = AutosarUtil.create_sub_container_value(data_name, container_def, container_val)
        # Set "DcmDspPidByteOffset" value
        self.set_numerical_param_value('DcmDspPidByteOffset', container_val, '0')
        # Set "DcmDspPidDataByteSize" value
        self.set_numerical_param_value('DcmDspPidDataByteSize', container_val, str(int(pid_data_size / 8)))

        if is_service_01_exist:
            # Create DcmDspPidService01 container
            container_def = self.mod_autosar_dict["Dcm"].find(
                "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspPid/DcmDspPidData/DcmDspPidService01")
            container_val = AutosarUtil.create_sub_container_value('DcmDspPidService01', container_def, container_val)

    def create_dcm_dsp_vehi_id(self, parent_obj, vehi_id_info):
        vehi_id_name = '0x' + '{:02X}'.format(int(
            vehi_id_info.identifier)) + '_' + vehi_id_info.name + '_Read_InfoType_Data_OBD'
        container_def = self.mod_autosar_dict["Dcm"].find("/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspVehInfo")
        container_val = AutosarUtil.create_sub_container_value(vehi_id_name, container_def, parent_obj)
        # Set "DcmDspVehInfoInfoType" value
        self.set_numerical_param_value('DcmDspVehInfoInfoType', container_val, vehi_id_info.identifier)

        # Create VidData container
        data_name = '0x' + '{:02X}'.format(int(vehi_id_info.identifier)) + '_InfoType_Data'
        data_size = 0
        for data in vehi_id_info.service_info['9']['data_list']:
            data_size += int(data.length / 8)  #Byte
        container_def = self.mod_autosar_dict["Dcm"].find(
            "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspVehInfo/DcmDspVehInfoData")
        container_val = AutosarUtil.create_sub_container_value(data_name, container_def, container_val)
        # Set "DcmDspVehInfoDataSize" value
        self.set_numerical_param_value('DcmDspVehInfoDataSize', container_val, str(data_size))

    def create_dcm_dsp(self, parent_obj):
        container_def = self.mod_autosar_dict["Dcm"].find("/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp")
        container_val = AutosarUtil.create_sub_container_value("DcmDsp", container_def, parent_obj)
        self.create_dcm_dsp_security(container_val)
        self.create_dcm_dsp_session(container_val)

        used_did_list = [did for did in self.cdd_info.dids if did.used]
        for did in used_did_list:
            data_dict_list = self.did_data_pre_process(did.datas)
            self.create_dcm_dsp_did_info(container_val, did)
            self.create_dcm_dsp_did(container_val, did, data_dict_list)
            for data_dict in data_dict_list:
                self.create_dcm_dsp_data_info(container_val, did, data_dict['name'])
                self.create_dcm_dsp_data(container_val, did, data_dict)

        used_routine_list = [routine for routine in self.cdd_info.routines if routine.used]
        for routine in used_routine_list:
            self.create_dcm_dsp_routine(container_val, routine)
            if len(routine.service_list) > 0:
                self.create_dcm_dsp_common_authorization(container_val, routine, routine.service_list[0])

        used_pid_list = [value for key, value in self.cdd_info.pids.items() if int(key) % 0x20 != 0 and value.used]
        for pid in used_pid_list:
            if len(pid.service_info) > 0:
                self.create_dcm_dsp_pid(container_val, pid)

        used_vehi_id_list = [value for key, value in self.cdd_info.vehi_ids.items() if value.used]
        for vehi_id in used_vehi_id_list:
            self.create_dcm_dsp_vehi_id(container_val, vehi_id)

    def create_dcm_dsl_buffer(self, parent_obj):
        container_def = self.mod_autosar_dict["Dcm"].find("/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsl/DcmDslBuffer")
        container_val = AutosarUtil.create_sub_container_value("DcmDslBuffer", container_def, parent_obj)

        # Set "DcmDslBufferSize" value
        self.set_numerical_param_value('DcmDslBufferSize', container_val, "1024")

    def create_dcm_dsl_diag_resp(self, parent_obj):
        container_def = self.mod_autosar_dict["Dcm"].find("/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsl/DcmDslDiagResp")
        container_val = AutosarUtil.create_sub_container_value("DcmDslDiagResp", container_def, parent_obj)

        # Set "DcmDslDiagRespMaxNumRespPend" value
        self.set_numerical_param_value('DcmDslDiagRespMaxNumRespPend', container_val, "5")

    def create_dcm_dsl_protocol(self, parent_obj):
        container_def = self.mod_autosar_dict["Dcm"].find("/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsl/DcmDslProtocol")
        AutosarUtil.create_sub_container_value("DcmDslProtocol", container_def, parent_obj)

    def create_dcm_dsl(self, parent_obj):
        container_def = self.mod_autosar_dict["Dcm"].find("/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsl")
        container_val = AutosarUtil.create_sub_container_value("DcmDsl", container_def, parent_obj)
        self.create_dcm_dsl_buffer(container_val)
        self.create_dcm_dsl_diag_resp(container_val)
        self.create_dcm_dsl_protocol(container_val)

    def create_dcm_processing_conditions(self, parent_obj):
        container_def = self.mod_autosar_dict["Dcm"].find("/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmProcessingConditions")
        AutosarUtil.create_sub_container_value("DcmProcessingConditions", container_def, parent_obj)

    def create_diagnostic_description_data(self):
        # Temporarily disable container_checker decorator
        self.container_checker_enable = False

        # Create new DiagnosticsDescriptionData.arxml
        new_ddd = ArxmlParseHandler.parse_diagnostics_description_data_arxml(self.base_arxml_path)
        dcm_config_set_val = new_ddd.find("/InitialEcuC/Dcm/DcmConfigSet")
        self.create_dcm_dsd(dcm_config_set_val)
        self.create_dcm_dsl(dcm_config_set_val)
        self.create_dcm_dsp(dcm_config_set_val)
        self.create_dcm_processing_conditions(dcm_config_set_val)

        # Enable container_checker decorator
        self.container_checker_enable = True

        return new_ddd

    def update_ecuc(self, new_ddd):
        dcm_ecuc_obj = self.ecuc_autosar_dict["Dcm"]
        dcm_configuration_values = dcm_ecuc_obj.find("/ActiveEcuC/Dcm")
        for container in dcm_configuration_values.CONTAINERS.ECUC_CONTAINER_VALUE:
            ecuc_container_path: str = container.get_path()
            ecuc_container_path.replace("ActiveEcuC", "InitialEcuC", 1)

    def update_dcm_ddd(self, new_dcm_ddd):
        old_ddd = self.system_autosar_dict["DiagnosticsDescriptionData"]
        old_dcm_ddd = old_ddd.find("/InitialEcuC/Dcm")
        new_dcm_ddd.parent_object_ = None

        ecuc_module_cfg_val_list: list = old_dcm_ddd.get_parent().ECUC_MODULE_CONFIGURATION_VALUES
        # Delete old dcm info
        AutosarUtil.remove_container_value(ecuc_module_cfg_val_list, old_dcm_ddd)
        ecuc_module_cfg_val_list.append(new_dcm_ddd)

        new_ddd = AutosarUtil.update_autosar(self.system_autosar_dict["DiagnosticsDescriptionData"])
        self.system_autosar_dict["DiagnosticsDescriptionData"] = new_ddd
        pass

    def update_from_cdd(self):
        """
        Update dcm cfg according to cdd file content

        1. Update dcm cfg according to cdd file content

        """
        # Create new DiagnosticsDescriptionData according to new cdd file
        new_ddd = self.create_diagnostic_description_data()
        new_dcm_ddd = new_ddd.find("/InitialEcuC/Dcm")
        old_dcm_ddd = self.system_autosar_dict["DiagnosticsDescriptionData"].find("/InitialEcuC/Dcm")

        # Compare the difference between two config
        dcm_initial_ecuc_comparator = ConfigComparator(old_dcm_ddd, new_dcm_ddd)
        dcm_initial_ecuc_comparator.config_comparator()
        new_ecuc = dcm_initial_ecuc_comparator.update_ecuc_config(self.ecuc_autosar_dict["Dcm"])

        self.ecuc_autosar_dict["Dcm"] = new_ecuc

        self.update_dcm_ddd(new_dcm_ddd)
