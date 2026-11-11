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

class DcmRuleUtils():

    def __init__(self) -> None:
        pass

    def get_dcm_general(self, dcm_dict):
        parent_path = '/ActiveEcuC/Dcm'
        dcm_general = self.dcm_rule_get_dict_by_definition_ref(dcm_dict, "DcmGeneral")
        return parent_path, dcm_general

    def get_dcm_config_set(self, dcm_dict):
        parent_path = '/ActiveEcuC/Dcm'
        dcm_config_set = self.dcm_rule_get_dict_by_definition_ref(dcm_dict, "DcmConfigSet")
        return parent_path, dcm_config_set

    # Dsd
    # DcmDsdServiceRequestManufacturerNotification = dcm_tools_get_dict_rows_by_definition_ref(DcmDsd, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsd/DcmDsdServiceRequestManufacturerNotification")
    # DcmDsdServiceRequestSupplierNotification = dcm_tools_get_dict_rows_by_definition_ref(DcmDsd, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsd/DcmDsdServiceRequestSupplierNotification")

    def get_dcm_dsd(self, dcm_dict):
        parent_path, dcm_config_set = self.get_dcm_config_set(dcm_dict)
        dcm_dsd = self.dcm_rule_get_dict_by_definition_ref(dcm_config_set, "DcmDsd")
        return parent_path + '/' + 'DcmConfigSet', dcm_dsd

    def get_dcm_dsd_service_table(self, dcm_dict):
        parent_path, dcm_dsd = self.get_dcm_dsd(dcm_dict)
        dcm_dsd_service_table_list = self.dcm_rule_get_dict_rows_by_definition_ref(dcm_dsd, 'DcmDsdServiceTable')
        return parent_path + '/' + 'DcmDsd', dcm_dsd_service_table_list

    # # Dsl
    # DcmDslConnections = dcm_tools_get_dict_by_definition_ref_from_rows(DcmDslProtocolRows, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsl/DcmDslProtocol/DcmDslProtocolRow/DcmDslConnection")
    # DcmDslMainConnections = dcm_tools_get_dict_by_definition_ref_from_rows(DcmDslConnections, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsl/DcmDslProtocol/DcmDslProtocolRow/DcmDslConnection/DcmDslMainConnection")
    # DcmDslProtocolTxs = dcm_tools_get_dict_by_definition_ref_from_rows(DcmDslMainConnections, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsl/DcmDslProtocol/DcmDslProtocolRow/DcmDslConnection/DcmDslMainConnection/DcmDslProtocolTx")
    # DcmDslPeriodicTransmissions = dcm_tools_get_dict_by_definition_ref_from_rows(DcmDslConnections, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsl/DcmDslProtocol/DcmDslProtocolRow/DcmDslConnection/DcmDslPeriodicTransmission")
    # DcmDslDiagResp = dcm_tools_get_dict_by_key(DcmDsl, "DcmDslDiagResp")
    # DcmDslBuffers = dcm_tools_get_dict_rows_by_definition_ref(DcmDsl, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsl/DcmDslBuffer")
    # DcmDslCallbackDCMRequestServices = dcm_tools_get_dict_rows_by_definition_ref(DcmDsl, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsl/DcmDslCallbackDCMRequestService")
    def get_dcm_dsl(self, dcm_dict):
        parent_path, dcm_config_set = self.get_dcm_config_set(dcm_dict)
        dcm_dsl = self.dcm_rule_get_dict_by_definition_ref(dcm_config_set, "DcmDsl")
        return parent_path + '/' + 'DcmConfigSet', dcm_dsl

    def get_dcm_dsl_protocol_row_list(self, dcm_dict):
        parent_path, dcm_dsl = self.get_dcm_dsl(dcm_dict)
        dsl_protocol = self.dcm_rule_get_dict_by_definition_ref(dcm_dsl, "DcmDslProtocol")
        dsl_protocol_row_list = self.dcm_rule_get_dict_rows_by_definition_ref(dsl_protocol, 'DcmDslProtocolRow')
        parent_path = parent_path + '/' + 'DcmDsl' + '/' + 'DcmDslProtocol'
        return parent_path, dsl_protocol_row_list

    # # Dsp
    # DcmDspRequestControls = dcm_tools_get_dict_rows_by_definition_ref(DcmDsp, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspRequestControl")
    # DcmDspDids = dcm_tools_get_dict_rows_by_definition_ref(DcmDsp, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspDid")
    # DcmDspDidRanges = dcm_tools_get_dict_rows_by_definition_ref(DcmDsp, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspDidRange")
    # DcmDspDatas = dcm_tools_get_dict_rows_by_definition_ref(DcmDsp, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspData")
    # DcmDspDidInfos = dcm_tools_get_dict_rows_by_definition_ref(DcmDsp, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspDidInfo")
    # DcmDspDataInfos = dcm_tools_get_dict_rows_by_definition_ref(DcmDsp, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspDataInfo")
    # DcmDspRoutines = dcm_tools_get_dict_rows_by_definition_ref(DcmDsp, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspRoutine")
    # DcmDspCommonAuthorizations = dcm_tools_get_dict_rows_by_definition_ref(DcmDsp, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspCommonAuthorization")
    # DcmDspComControl = dcm_tools_get_dict_by_key(DcmDsp, "DcmDspComControl")
    # DcmDspComControl_allChannels = dcm_tools_get_dict_rows_by_definition_ref(DcmDspComControl, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspComControl/DcmDspComControlAllChannel")
    # DcmDspComControl_specificChannels = dcm_tools_get_dict_rows_by_definition_ref(DcmDspComControl, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspComControl/DcmDspComControlSpecificChannel")
    # DcmDslProtocolRxs = dcm_tools_get_channelindex(DcmDslMainConnections,DcmDspComControl_allChannels,DcmDspComControl_specificChannels ,"/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsl/DcmDslProtocol/DcmDslProtocolRow/DcmDslConnection/DcmDslMainConnection/DcmDslProtocolRx")
    # DcmDspMemory = dcm_tools_get_dict_by_key(DcmDsp, "DcmDspMemory")
    # DcmDspMemoryIdInfoList = dcm_tools_get_dict_rows_by_definition_ref(DcmDspMemory, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspMemory/DcmDspMemoryIdInfo")
    # DcmDspMemoryTransfer = dcm_tools_get_dict_rows_by_definition_ref(DcmDsp, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspMemoryTransfer")
    # DcmDspRequestFileTransfer = dcm_tools_get_dict_rows_by_definition_ref(DcmDsp, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspRequestFileTransfer")

    def get_dcm_dsp(self, dcm_dict):
        parent_path, dcm_config_set = self.get_dcm_config_set(dcm_dict)
        dcm_dsp = self.dcm_rule_get_dict_by_definition_ref(dcm_config_set, "DcmDsp")
        return parent_path + '/' + 'DcmConfigSet', dcm_dsp

    def get_dcm_dsp_security_row_list(self, dcm_dict):
        parent_path, dcm_dsp = self.get_dcm_dsp(dcm_dict)
        dsp_security = self.dcm_rule_get_dict_by_definition_ref(dcm_dsp, "DcmDspSecurity")
        dsp_security_row_list = self.dcm_rule_get_dict_rows_by_definition_ref(dsp_security, 'DcmDspSecurityRow')
        parent_path = parent_path + '/' + 'DcmDsp' + '/' + 'DcmDspSecurity'
        return parent_path, dsp_security_row_list

    def get_dcm_dsp_session_row_list(self, dcm_dict):
        parent_path, dcm_dsp = self.get_dcm_dsp(dcm_dict)
        dsp_session = self.dcm_rule_get_dict_by_definition_ref(dcm_dsp, 'DcmDspSession')
        dsp_session_row_list = self.dcm_rule_get_dict_rows_by_definition_ref(dsp_session, 'DcmDspSessionRow')
        parent_path = parent_path + '/' + 'DcmDsp' + '/' + 'DcmDspSession'
        return parent_path, dsp_session_row_list

    def get_dcm_dsp_did_list(self, dcm_dict):
        parent_path, dcm_dsp = self.get_dcm_dsp(dcm_dict)
        dsp_did_list = self.dcm_rule_get_dict_rows_by_definition_ref(dcm_dsp, 'DcmDspDid')
        parent_path = parent_path + '/' + 'DcmDsp'
        return parent_path, dsp_did_list

    def get_dcm_dsp_did_info_list(self, dcm_dict):
        parent_path, dcm_dsp = self.get_dcm_dsp(dcm_dict)
        dsp_did_info_list = self.dcm_rule_get_dict_rows_by_definition_ref(dcm_dsp, 'DcmDspDidInfo')
        parent_path = parent_path + '/' + 'DcmDsp'
        return parent_path, dsp_did_info_list

    def get_dcm_dsp_data_list(self, dcm_dict):
        parent_path, dcm_dsp = self.get_dcm_dsp(dcm_dict)
        dsp_data_list = self.dcm_rule_get_dict_rows_by_definition_ref(dcm_dsp, 'DcmDspData')
        parent_path = parent_path + '/' + 'DcmDsp'
        return parent_path, dsp_data_list

    def get_dcm_dsp_data_info_list(self, dcm_dict):
        parent_path, dcm_dsp = self.get_dcm_dsp(dcm_dict)
        dsp_data_info_list = self.dcm_rule_get_dict_rows_by_definition_ref(dcm_dsp, 'DcmDspDataInfo')
        parent_path = parent_path + '/' + 'DcmDsp'
        return parent_path, dsp_data_info_list

    # # ProcessingConditions
    def get_dcm_processing_conditions(self, dcm_dict):
        parent_path, dcm_config_set = self.get_dcm_config_set(dcm_dict)
        dcm_processing_conditions = self.dcm_rule_get_dict_by_definition_ref(dcm_config_set, "DcmProcessingConditions")
        return parent_path + '/' + 'DcmConfigSet', dcm_processing_conditions

    def get_dcm_mode_condition_list(self, dcm_dict):
        parent_path, dcm_processing_conditions = self.get_dcm_processing_conditions(dcm_dict)
        dcm_mode_condition_list = self.dcm_rule_get_dict_rows_by_definition_ref(dcm_processing_conditions,
                                                                                'DcmModeCondition')
        parent_path = parent_path + '/' + 'DcmProcessingConditions'
        return parent_path, dcm_mode_condition_list

    def get_dcm_mode_rule_list(self, dcm_dict):
        parent_path, dcm_processing_conditions = self.get_dcm_processing_conditions(dcm_dict)
        dcm_mode_rule_list = self.dcm_rule_get_dict_rows_by_definition_ref(dcm_processing_conditions, 'DcmModeRule')
        parent_path = parent_path + '/' + 'DcmProcessingConditions'
        return parent_path, dcm_mode_rule_list

    # # Common Function
    def dcm_rule_get_dict_by_definition_ref(self, input_dict: dict, definition_ref: str) -> dict:
        for k, v in input_dict.items():
            if isinstance(v, dict):
                if "DefinitionRef" in v.keys():
                    if v["DefinitionRef"].split('/')[-1] == definition_ref:
                        return input_dict[k]
        return {}

    def dcm_rule_get_dict_rows_by_definition_ref(self, input_dict: dict, definition_ref: str) -> dict:
        out_dict = {}
        for k, v in input_dict.items():
            if isinstance(v, dict):
                if "DefinitionRef" in v.keys():
                    if v["DefinitionRef"].split('/')[-1] == definition_ref:
                        out_dict.update({k: v})
        return out_dict

    def dcm_rule_get_dict_by_definition_ref_from_rows(self, input_dict: dict, definition_ref: str) -> dict:
        out_dict = {}
        for k, v in input_dict.items():
            if isinstance(v, dict):
                for k1, v1 in v.items():
                    if isinstance(v1, dict):
                        if "DefinitionRef" in v1.keys():
                            if v1["DefinitionRef"].split('/')[-1] == definition_ref:
                                out_dict.update({k1: v1})
        return out_dict

    def dcm_rule_get_value_from_dict_by_key(self, input_dict: dict, input_key: str) -> dict:
        output_dict = {}
        for key, value in input_dict.items():
            if key == input_key:
                output_dict = value
        return output_dict

    def dcm_rule_check_did_exist(self, input_dict: dict, check_type: str) -> bool:
        result = False
        read_dict = self.dcm_rule_get_dict_by_definition_ref(input_dict, "DcmDspDidRead")
        write_dict = self.dcm_rule_get_dict_by_definition_ref(input_dict, "DcmDspDidWrite")
        control_dict = self.dcm_rule_get_dict_by_definition_ref(input_dict, "DcmDspDidControl")

        if check_type == 'Read' and len(read_dict) > 0:
            result = True
        elif check_type == 'Write' and len(write_dict) > 0:
            result = True
        elif len(control_dict) > 0 and control_dict['DcmDspDidControlMask']['Value'] != 'DCM_CONTROLMASK_EXTERNAL':
            if check_type == 'FreezeCurrentState' and control_dict['DcmDspDidFreezeCurrentState']['Value']:
                result = True
            elif check_type == 'ResetToDefault' and control_dict['DcmDspDidResetToDefault']['Value']:
                result = True
            elif check_type == 'ShortTermAdjustment' and control_dict['DcmDspDidShortTermAdjustment']['Value']:
                result = True
            elif check_type == 'ReturnControlToEcu':
                if control_dict['DcmDspDidFreezeCurrentState']['Value'] or control_dict['DcmDspDidResetToDefault'][
                  'Value'] or control_dict['DcmDspDidShortTermAdjustment']['Value']:
                    result = True
        return result
