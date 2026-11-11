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
os模板内函数定义模块.

定义jinja2模板中所使用的函数.

用法示例:
{{get_core_id_list()}}
"""

from jinja2 import pass_context
from func_module.generator.common.common import FunctionBase


class Function(FunctionBase):
    """
    Class containing all functions. Add new static functions here
    to be included in the jinja2 context
     """

    @staticmethod
    @pass_context
    def dcm_get_last_str(self, input_str):
        out_str = input_str.split("/")[-1]
        return out_str

    @staticmethod
    @pass_context
    def get_enable_value_from_boolean(self, value: bool) -> str:
        enable_value = "STD_OFF"
        if value:
            enable_value = "STD_ON"

        return enable_value

    @staticmethod
    @pass_context
    def check_dcm_callback_session_enabled_value(self, dcm_DcmDsp) -> str:
        enable = False
        if "DcmDspEnableSessionChangeCallback" in dcm_DcmDsp.keys():
            enable = dcm_DcmDsp["DcmDspEnableSessionChangeCallback"]["Value"]
        return enable

    @staticmethod
    @pass_context
    def check_dcm_callback_ecu_reset_enabled_value(self, dcm_DcmDsp) -> str:
        enable = False
        if "DcmDspEnableEcuResetCallback" in dcm_DcmDsp.keys():
            enable = dcm_DcmDsp["DcmDspEnableEcuResetCallback"]["Value"]
        return enable

    @staticmethod
    @pass_context
    def get_DCM_MANUFACTURER_NOTIFICATION_enable(self, input_dict):
        if input_dict:
            return "STD_ON"
        else:
            return "STD_OFF"

    @staticmethod
    @pass_context
    def get_session_mask(self, input_ref, input_session_row):
        mask = 0
        session_str_list = []
        if isinstance(input_ref, str):
            session_str_list.append(input_ref.split("/")[-1])
        elif isinstance(input_ref, list):
            for input in input_ref:
                session_str_list.append(input.split("/")[-1])

        for index, key in enumerate(input_session_row.keys()):
            if key in session_str_list:
                mask += (1 << index)
        return mask

    @staticmethod
    @pass_context
    def get_security_mask(self, input_ref, input_security_row):
        mask = 0
        security_str_list = []
        if isinstance(input_ref, str):
            security_str_list.append(input_ref.split("/")[-1])
        elif isinstance(input_ref, list):
            for input in input_ref:
                security_str_list.append(input.split("/")[-1])

        for index, key in enumerate(input_security_row.keys()):
            if key in security_str_list:
                mask += (1 << index)
        return mask

    @staticmethod
    @pass_context
    def to_binary(self, int_num):
        return format(int_num,'04b')

    @staticmethod
    @pass_context
    def get_vsg_ref_list(self, input_ref):
        vsg_str_list = []
        if isinstance(input_ref, str):
            vsg_str_list.append(input_ref)
        elif isinstance(input_ref, list):
            vsg_str_list = input_ref

        return vsg_str_list


    @staticmethod
    @pass_context
    def get_routine_authorization_name(self, routine_service):
        if "DcmDspStartRoutine" in routine_service:
            AuthorizationRef = routine_service["DcmDspStartRoutine"]["DcmDspStartRoutineCommonAuthorizationRef"]["ValueRef"]
        elif "DcmDspStopRoutine" in routine_service:
            AuthorizationRef = routine_service["DcmDspStopRoutine"]["DcmDspStopRoutineCommonAuthorizationRef"]["ValueRef"]
        elif "DcmDspRequestRoutineResults" in routine_service:
            AuthorizationRef = routine_service["DcmDspRequestRoutineResults"]["DcmDspRequestRoutineResultsCommonAuthorizationRef"]["ValueRef"]
        key_name = AuthorizationRef.split("/")[-1]
        return key_name


    @staticmethod
    @pass_context
    def get_routine_subservice_vsgref(self, input_routine_subservice, input_common_authorizations):
        Vsg_ref_list = []
        if "DcmDspStartRoutine" in input_routine_subservice:
            AuthorizationRef = input_routine_subservice["DcmDspStartRoutine"]["DcmDspStartRoutineCommonAuthorizationRef"]["ValueRef"]
            key_name = AuthorizationRef.split("/")[-1]
        elif "DcmDspStopRoutine" in input_routine_subservice:
            AuthorizationRef = input_routine_subservice["DcmDspStopRoutine"]["DcmDspStopRoutineCommonAuthorizationRef"]["ValueRef"]
            key_name = AuthorizationRef.split("/")[-1]
        elif "DcmDspRequestRoutineResults" in input_routine_subservice:
            AuthorizationRef = input_routine_subservice["DcmDspRequestRoutineResults"]["DcmDspRequestRoutineResultsCommonAuthorizationRef"]["ValueRef"]
            key_name = AuthorizationRef.split("/")[-1]
        Vsg_ref = input_common_authorizations[key_name]["DcmDspCommonAuthorizationVehicleSystemGroupRef"]["ValueRef"]
        Vsg_ref_list = Function.get_vsg_ref_list(self, Vsg_ref)
        return Vsg_ref_list

    @staticmethod
    @pass_context
    def get_dcm_sec_att_cntr_enabled_value(self, dcm_DcmDspSecurityRow):
        ret = False
        retInStr = "STD_OFF"
        for key, value in dcm_DcmDspSecurityRow.items():
            ret |= value["DcmDspSecurityAttemptCounterEnabled"]["Value"]
        if ret is True:
            retInStr = "STD_ON"
        return retInStr

    @staticmethod
    @pass_context
    def get_dcm_dsp_did_range_enabled_value(self, dcm_DcmDspDidRanges):
        retInStr = "STD_OFF"
        if dcm_DcmDspDidRanges:
            retInStr = "STD_ON"
        return retInStr

    @staticmethod
    @pass_context
    def print_mode_rule_condition_list(self, input_list):
        if len(input_list) > 0:
            sorted_list = sorted(input_list)
            output = "{" + sorted_list[0]
            for index in sorted_list[1:]:
                output += ", " + index
            output += "}"
        else:
            output = "{}"
        return output

    @staticmethod
    @pass_context
    def get_moderule_ref_list(self, input_ref, dcm_DcmModeRules, dcm_DcmModeConditions):
        moderule_list = []
        modecondition_list = []
        if isinstance(input_ref, str):
            if(input_ref.split("/")[-1] in dcm_DcmModeRules.keys()):
                moderule_list.append(str(dcm_DcmModeRules[input_ref.split("/")[-1]]["index"]))
            elif(input_ref.split("/")[-1] in dcm_DcmModeConditions.keys()):
                modecondition_list.append(str(dcm_DcmModeConditions[input_ref.split("/")[-1]]["index"]))
        elif isinstance(input_ref, list):
            for ref_str in input_ref:
                if(ref_str.split("/")[-1] in dcm_DcmModeRules.keys()):
                    moderule_list.append(str(dcm_DcmModeRules[ref_str.split("/")[-1]]["index"]))
                elif(ref_str.split("/")[-1] in dcm_DcmModeConditions.keys()):
                    modecondition_list.append(str(dcm_DcmModeConditions[ref_str.split("/")[-1]]["index"]))
        return modecondition_list, moderule_list

    @staticmethod
    @pass_context
    def get_subservice_dict(self, service_dict):
        out_dict = {}
        if service_dict["DcmDsdSidTabSubfuncAvail"]["Value"]:
            for k, v in service_dict.items():
                if isinstance(v, dict):
                    if "DefinitionRef" in v.keys():
                        if v["DefinitionRef"].split("/")[-1] == "DcmDsdSubService" and v['DcmDsdSubServiceUsed']['Value']:
                            out_dict.update({k: v})
        return out_dict

    @staticmethod
    @pass_context
    def get_subservice_ref(self, service_dict):
        out_str = 'NULL'
        subservice_dict = Function.get_subservice_dict(self, service_dict)
        if len(subservice_dict) > 0:
            out_str = "SubService_table" + "{:X}".format(service_dict["dsd_service_table_index"]) + "_0x" + "{:X}".format(service_dict["DcmDsdSidTabServiceId"]["Value"])
        return out_str

    @staticmethod
    @pass_context
    def get_function_from_str(self, func_str):
        if func_str == "" or func_str == "<empty>" or func_str == None:
            return "NULL"
        return func_str

    @staticmethod
    @pass_context
    def get_moderule_ref_str(self, intput_str, dcm_mode_rules):
        if intput_str == "" or intput_str == "<empty>" or intput_str == None:
            return len(dcm_mode_rules)
        else:
            if dcm_mode_rules:
                for key, value in dcm_mode_rules.items():
                    if intput_str.split("/")[-1] == key:
                        return value["index"]
            else:
                return len(dcm_mode_rules)

    @staticmethod
    @pass_context
    def get_didsignal_form_dspdid(self, input):
        if isinstance(input, dict):
            if "DefinitionRef" in input.keys():
                if input["DefinitionRef"] == "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspDid/DcmDspDidSignal":
                    return input
        return None

    @staticmethod
    @pass_context
    def get_didsignal_number_form_dspdid(self, input):
        number = 0
        for k, v in input.items():
            if isinstance(v, dict):
                if "DefinitionRef" in v.keys():
                    if v["DefinitionRef"] == "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspDid/DcmDspDidSignal":
                        number = number + 1

        return number

    @staticmethod
    @pass_context
    def check_key_exit(self, input_dict, input_key):
        if isinstance(input_dict, dict):
            if input_key in input_dict.keys():
                return True

        return False

    @staticmethod
    @pass_context
    def get_routine_signal_para(self, input_dict, name):
        para_dict = {}
        para_dict["DcmDspRoutineParameterSize"] = input_dict[name]["DcmDspRoutineParameterSize"]["Value"]
        para_dict["DcmDspRoutineSignalPos"] = input_dict[name]["DcmDspRoutineSignalPos"]["Value"]
        para_dict["DcmDspRoutineSignalType"] = input_dict[name]["DcmDspRoutineSignalType"]["Value"]
        return para_dict

    @staticmethod
    @pass_context
    def get_routine_is_VARIABLE_LENGTH_contained(self, routine_dict):
        ret = False
        for key, value in routine_dict.items():
            if key != "DefinitionRef":
                signal_para = Function.get_routine_signal_para(self, routine_dict, key)
                if signal_para["DcmDspRoutineSignalType"] == "VARIABLE_LENGTH":
                    ret = True
        return ret

    @staticmethod
    @pass_context
    def get_routine_parameters_size(self, input_dict):
        total_size = 0
        for key, value in input_dict.items():
            if key != "DefinitionRef":
                signal_para = Function.get_routine_signal_para(self, input_dict, key)
                total_size += int(signal_para["DcmDspRoutineParameterSize"])
        return total_size

    @staticmethod
    @pass_context
    def get_routine_parameters_signal_type(self, input_dict):
        signals_type_dict = {}
        for key, value in input_dict.items():
            if key != "DefinitionRef":
                signal_para = Function.get_routine_signal_para(self, input_dict, key)
                signals_type_dict[key] = signal_para["DcmDspRoutineSignalType"]
        return signals_type_dict

    @staticmethod
    @pass_context
    def get_routine_util_name(self, input_dict, signal_name):
        util_name = None
        for key, value in input_dict.items():
            if key == signal_name:
                signal_para = Function.get_routine_signal_para(self, input_dict, key)
                signal_type = signal_para["DcmDspRoutineSignalType"]
                signal_position_byte = int(signal_para["DcmDspRoutineSignalPos"] / 8)
                if signal_type == "BOOLEAN":
                    util_name = "((bool)(Dcm_UtiGetReqDataAsU8Rel(indata, " + str(signal_position_byte) + ")));"
                elif signal_type == "UINT8":
                    util_name = "((uint8)(Dcm_UtiGetReqDataAsU8Rel(indata," + str(signal_position_byte) + ")));"
                elif signal_type == "UINT16":
                    util_name = "((uint16)(Dcm_UtiMake16Bit(Dcm_UtiGetReqDataAsU8Rel(indata," + str(
                        signal_position_byte) + "), Dcm_UtiGetReqDataAsU8Rel(indata, " + str(signal_position_byte +
                                                                                             1) + ")))); "
                elif signal_type == "UINT32":
                    util_name = "((uint32)(Dcm_UtiMake32Bit(Dcm_UtiGetReqDataAsU8Rel(indata," + str(
                        signal_position_byte) + "), Dcm_UtiGetReqDataAsU8Rel(indata, " + str(
                            signal_position_byte + 1) + "), Dcm_UtiGetReqDataAsU8Rel(indata," + str(
                                signal_position_byte +
                                2) + " ), Dcm_UtiGetReqDataAsU8Rel(indata," + str(signal_position_byte + 3) + " )))); "
                elif signal_type == "SINT8":
                    util_name = "((sint8)(Dcm_UtiGetReqDataAsU8Rel(indata," + str(signal_position_byte) + ")));"
                elif signal_type == "SINT16":
                    util_name = "((sint16)(Dcm_UtiMake16Bit(Dcm_UtiGetReqDataAsU8Rel(indata," + str(
                        signal_position_byte) + "), Dcm_UtiGetReqDataAsU8Rel(indata, " + str(signal_position_byte +
                                                                                             1) + ")))); "
                elif signal_type == "SINT32":
                    util_name = "((sint32)(Dcm_UtiMake32Bit(Dcm_UtiGetReqDataAsU8Rel(indata," + str(
                        signal_position_byte) + "), Dcm_UtiGetReqDataAsU8Rel(indata, " + str(
                            signal_position_byte + 1) + "), Dcm_UtiGetReqDataAsU8Rel(indata," + str(
                                signal_position_byte +
                                2) + " ), Dcm_UtiGetReqDataAsU8Rel(indata," + str(signal_position_byte + 3) + " )))); "
                elif signal_type == "UINT8_N":
                    util_name = "(uint8*)(&(Dcm_UtiGetReqDataAsU8Rel(indata," + str(signal_position_byte) + ")));"
                else:
                    util_name = None
        return util_name

    @staticmethod
    @pass_context
    def get_routine_outsignal_util_name(self, input_dict, signal_name):
        util_name = None
        for key, value in input_dict.items():
            if key == signal_name:
                signal_para = Function.get_routine_signal_para(self, input_dict, key)
                signal_type = signal_para["DcmDspRoutineSignalType"]
                signal_position_byte = int(signal_para["DcmDspRoutineSignalPos"] / 8)
                if signal_type == "BOOLEAN":
                    util_name = "(Dcm_UtiGetResDataAsU8At(outdata, " + str(signal_position_byte) + "))"
                elif signal_type == "UINT8":
                    util_name = "(Dcm_UtiGetResDataAsU8At(outdata," + str(signal_position_byte) + "))"
                elif signal_type == "UINT16":
                    util_name = "(Dcm_UtiMake16Bit(Dcm_UtiGetResDataAsU8At(outdata," + str(
                        signal_position_byte) + "), Dcm_UtiGetResDataAsU8At(outdata, " + str(signal_position_byte +
                                                                                             1) + ")))"
                elif signal_type == "UINT32":
                    util_name = "(Dcm_UtiMake32Bit(Dcm_UtiGetResDataAsU8At(outdata," + str(
                        signal_position_byte) + "), Dcm_UtiGetResDataAsU8At(outdata, " + str(
                            signal_position_byte + 1) + "), Dcm_UtiGetResDataAsU8At(outdata," + str(
                                signal_position_byte +
                                2) + " ), Dcm_UtiGetResDataAsU8At(outdata," + str(signal_position_byte + 3) + " )))"
                elif signal_type == "SINT8":
                    util_name = "(Dcm_UtiGetResDataAsU8At(outdata," + str(signal_position_byte) + "))"
                elif signal_type == "SINT16":
                    util_name = "(Dcm_UtiMake16Bit(Dcm_UtiGetResDataAsU8At(outdata," + str(
                        signal_position_byte) + "), Dcm_UtiGetResDataAsU8At(outdata, " + str(signal_position_byte +
                                                                                             1) + "))) "
                elif signal_type == "SINT32":
                    util_name = "(Dcm_UtiMake32Bit(Dcm_UtiGetResDataAsU8At(outdata," + str(
                        signal_position_byte) + "), Dcm_UtiGetResDataAsU8At(outdata, " + str(
                            signal_position_byte + 1) + "), Dcm_UtiGetResDataAsU8At(outdata," + str(
                                signal_position_byte +
                                2) + " ), Dcm_UtiGetResDataAsU8At(outdata," + str(signal_position_byte + 3) + " )))"
                elif signal_type == "UINT8_N":
                    util_name = "(Dcm_UtiGetResDataAsU8At(outdata," + str(signal_position_byte) + "))"
                else:
                    util_name = None
        return util_name

    @staticmethod
    @pass_context
    def get_routine_transform_signal_type(self, signal_type, property):
        transfer_signal_name = None

        if signal_type == "BOOLEAN":
            transfer_signal_name = "boolean"
        elif signal_type == "UINT8":
            transfer_signal_name = "uint8"
        elif signal_type == "UINT16":
            transfer_signal_name = "uint16"
        elif signal_type == "UINT32":
            transfer_signal_name = "uint32"
        elif signal_type == "SINT8":
            transfer_signal_name = "sint8"
        elif signal_type == "SINT16":
            transfer_signal_name = "sint16"
        elif signal_type == "SINT32":
            transfer_signal_name = "sint32"
        elif signal_type == "UINT8_N":
            if property == "DcmDspStartRoutineIn" or property == "DcmDspStopRoutineIn" or property == "DcmDspRequestRoutineResultsIn":
                transfer_signal_name = "uint8 *"
            elif property == "DcmDspStartRoutineOut" or property == "DcmDspStopRoutineOut" or property == "DcmDspRequestRoutineResultsOut":
                transfer_signal_name = "uint8"
            else:
                transfer_signal_name = None
        else:
            transfer_signal_name = None
        return transfer_signal_name

    @staticmethod
    @pass_context
    def adapt_dsp_data_use_port(self, use_port):
        out_use_port = ""
        if use_port == "USE_BLOCK_ID":
            out_use_port = "DATA_PORT_BLOCK_ID"
        elif use_port == "USE_DATA_ASYNCH_CLIENT_SERVER" or use_port == "USE_DATA_ASYNCH_CLIENT_SERVER_ERROR" or use_port == "USE_DATA_ASYNCH_FNC" or use_port == "USE_DATA_ASYNCH_FNC_ERROR":
            out_use_port = "DATA_PORT_ASYNCH"
        elif use_port == "USE_DATA_SENDER_RECEIVER" or use_port == "USE_DATA_SENDER_RECEIVER_AS_SERVICE":
            out_use_port = "DATA_PORT_SR"
        elif use_port == "USE_DATA_SYNCH_CLIENT_SERVER" or use_port == "USE_DATA_SYNCH_FNC":
            out_use_port = "DATA_PORT_SYNCH"
        elif use_port == "USE_ECU_SIGNAL":
            out_use_port = "DATA_PORT_ECU_SIGNAL"
        return out_use_port

    @staticmethod
    @pass_context
    def adapt_dsp_data_ConditionCheckReadFnc_type(self, use_port):
        type = "asynch_condition_check_read_fnc"
        if use_port == "USE_DATA_SYNCH_CLIENT_SERVER" or use_port == "USE_DATA_SYNCH_FNC":
            type = "synch_condition_check_read_fnc"
        return type

    @staticmethod
    @pass_context
    def adapt_dsp_data_ReadDataFnc_type(self, use_port):
        type = "asynch_data_read_fnc"
        if use_port == "USE_DATA_SYNCH_CLIENT_SERVER" or use_port == "USE_DATA_SYNCH_FNC":
            type = "synch_data_read_fnc"
        elif use_port == "USE_DATA_SENDER_RECEIVER" or use_port == "USE_DATA_SENDER_RECEIVER_AS_SERVICE":
            type = "sr_data_read_fnc"
        return type

    @staticmethod
    @pass_context
    def adapt_dsp_data_WriteDataFnc_type(self, use_port):
        type = "fix_len_asynch_data_write_fnc"
        if use_port == "USE_DATA_SYNCH_CLIENT_SERVER" or use_port == "USE_DATA_SYNCH_FNC":
            type = "fix_len_synch_data_write_fnc"
        return type

    @staticmethod
    @pass_context
    def adapt_dsp_data_GetScalingInfoFnc_type(self, use_port):
        type = "asynch_get_scaling_info_fnc"
        if use_port == "USE_DATA_SYNCH_CLIENT_SERVER" or use_port == "USE_DATA_SYNCH_FNC":
            type = "synch_get_scaling_info_fnc"
        return type

    @staticmethod
    @pass_context
    def adapt_dsp_data_ResetToDefaultFnc_type(self, use_port):
        type = "func_asynch_reset_to_default_fnc"
        if use_port == "USE_DATA_SYNCH_CLIENT_SERVER" or use_port == "USE_DATA_SYNCH_FNC":
            type = "func_synch_reset_to_default_fnc"
        return type

    @staticmethod
    @pass_context
    def adapt_dsp_data_FreezeCurrentStateFnc_type(self, use_port):
        type = "func_asynch_freeze_current_state_fnc"
        if use_port == "USE_DATA_SYNCH_CLIENT_SERVER" or use_port == "USE_DATA_SYNCH_FNC":
            type = "func_synch_freeze_current_state_fnc"
        return type

    @staticmethod
    @pass_context
    def adapt_dsp_data_ShortTermAdjustmentFnc_type(self, use_port):
        type = "func_asynch_short_term_adjustment_fnc"
        if use_port == "USE_DATA_SYNCH_CLIENT_SERVER" or use_port == "USE_DATA_SYNCH_FNC":
            type = "func_synch_short_term_adjustment_fnc"
        return type

    @staticmethod
    @pass_context
    def is_nvmdid_used(self, dcm_DcmDspDatas):
        result = False
        for key,value in dcm_DcmDspDatas.items():
            if value["DcmDspDataUsePort"]["Value"] == "USE_BLOCK_ID":
                return True
        return result
    @staticmethod
    @pass_context
    def get_nvmblockid(self, blockref):
        if blockref != None and blockref != "<empty>":
            blockid = "NvMConf_"+blockref.split("/")[-1]
        else:
            blockid = 0
        return blockid

    @staticmethod
    @pass_context
    def getVehInfoTotalSize(self, dcm_DcmDspVehInfo):
        total_size = 0
        for key, value in dcm_DcmDspVehInfo.items():
            if "DcmDspVehInfoDataSize" in value:
                total_size += (value["DcmDspVehInfoDataSize"]["Value"] -1)
        return total_size

    @staticmethod
    @pass_context
    def getVehInfoNumberOfDataItems(self, dcm_DcmDspVehInfo):
        datacount = 0
        for key, value in dcm_DcmDspVehInfo.items():
            if "DcmDspVehInfoDataSize" in value:
                datacount += 1
        return datacount

    @staticmethod
    @pass_context
    def getVehInfoTypeFnc(self, data_name, data_value):
        funcname = "NULL"
        if data_value["DcmDspVehInfoDataUsePort"]["Value"] == True:
            funcname = "Rte_Call_InfotypeServices_" + data_name + "_GetInfotypeValueData"
        elif data_value["DcmDspVehInfoDataReadFnc"]["Value"] != None and data_value["DcmDspVehInfoDataReadFnc"]["Value"] != '':
            funcname = data_value["DcmDspVehInfoDataReadFnc"]["Value"]
        return funcname

    @staticmethod
    @pass_context
    def getVehInfoSize(self, dcm_DcmDspVehInfodataitem):
        datasize = dcm_DcmDspVehInfodataitem["DcmDspVehInfoDataSize"]["Value"] - 1
        return datasize

    @staticmethod
    @pass_context
    def adapt_dsp_data_endianess(self, endianess):
        out_endianess = ""
        if endianess == "BIG_ENDIAN":
            out_endianess = "DCM_BIG_ENDIAN"
        elif endianess == "LITTLE_ENDIAN":
            out_endianess = "DCM_LITTLE_ENDIAN"
        elif endianess == "OPAQUE":
            out_endianess = "DCM_ENDIAN_NOT_USED"
        return out_endianess

    @staticmethod
    @pass_context
    def adapt_dsl_protocol_addr_type(self, add_type):
        out_add_type = ""
        if add_type == "DCM_FUNCTIONAL_TYPE":
            out_add_type = "DCM_PROTOCOL_FUNCTIONAL_ADDR_TYPE"
        elif add_type == "DCM_PHYSICAL_TYPE":
            out_add_type = "DCM_PROTOCOL_PHYSICAL_ADDR_TYPE"
        return out_add_type

    @staticmethod
    @pass_context
    def is_use_data_ui_func(self, use_port):
        if use_port == "USE_DATA_ASYNCH_FNC" or use_port == "USE_DATA_ASYNCH_FNC_ERROR" or use_port == "USE_DATA_SYNCH_FNC":
            return True
        else:
            return False

    @staticmethod
    @pass_context
    def is_use_ui_func(self, use_port):
        if use_port == "USE_ASYNCH_FNC":
            return True
        else:
            return False

    @staticmethod
    @pass_context
    def is_use_data_port_client_server(self, use_port):
        if ((use_port == "USE_DATA_ASYNCH_CLIENT_SERVER") or (use_port == "USE_DATA_ASYNCH_CLIENT_SERVER_ERROR") or
            (use_port == "USE_DATA_SYNCH_CLIENT_SERVER")):
            return True
        else:
            return False

    @staticmethod
    @pass_context
    def get_data_ui_func_by_variation(self, use_port, ui_func, func_name):
        if Function.is_use_data_ui_func(self, use_port):
            return Function.get_function_from_str(self, ui_func)
        elif Function.is_use_data_port_client_server(self, use_port):
            return func_name
        else:
            return "NULL"

    @staticmethod
    @pass_context
    def get_ui_func_by_variation(self, use_port, ui_func, func_name):
        if Function.is_use_ui_func(self, use_port):
            return Function.get_function_from_str(self, ui_func)
        else:
            return func_name

    @staticmethod
    @pass_context
    def get_ui_func_by_use_port(self, use_port, ui_func, func_name):
        if use_port is False:
            return Function.get_function_from_str(self, ui_func)
        else:
            return func_name

    @staticmethod
    @pass_context
    def get_func_ConditionCheckRead(self, use_port, ui_func, did_info_dict, func_name, fun_used):
        if fun_used is True:
            if Function.get_value_from_dict_by_key(did_info_dict, "DcmDspDidRead") is None:
                return "NULL"
            else:
                return Function.get_data_ui_func_by_variation(self, use_port, ui_func, func_name)
        else:
            return "NULL"

    @staticmethod
    @pass_context
    def get_func_CheckEnabled(self, use_port, ui_func, func_name, enabled):
        if enabled is True:
            return Function.get_ui_func_by_variation(self, use_port, ui_func, func_name)
        else:
            return "NULL"

    @staticmethod
    @pass_context
    def to_hex(self, input):
        return hex(input)

    @staticmethod
    @pass_context
    def dcm_get_MemoryRange_name(self, DcmDspMemoryIdInfo):
        for key, value in DcmDspMemoryIdInfo.items():
            if "DefinitionRef" in value:
                if "DcmDspReadMemoryRangeInfo" == value["DefinitionRef"].split("/")[-1]:
                    read_memory_name = key
                if "DcmDspWriteMemoryRangeInfo" == value["DefinitionRef"].split("/")[-1]:
                    write_memory_name = key
        return (read_memory_name, write_memory_name)

    @staticmethod
    @pass_context
    def dcm_get_Read_MemoryRange_name_list(self, DcmDspMemoryIdInfo):
        read_memory_name = []
        for key, value in DcmDspMemoryIdInfo.items():
            if "DefinitionRef" in value:
                if "DcmDspReadMemoryRangeInfo" == value["DefinitionRef"].split("/")[-1]:
                    read_memory_name.append(key)
        return read_memory_name

    @staticmethod
    @pass_context
    def dcm_get_Write_MemoryRange_name_list(self, DcmDspMemoryIdInfo):
        write_memory_name = []
        for key, value in DcmDspMemoryIdInfo.items():
            if "DefinitionRef" in value:
                if "DcmDspWriteMemoryRangeInfo" == value["DefinitionRef"].split("/")[-1]:
                    write_memory_name.append(key)
        return write_memory_name

    @staticmethod
    @pass_context
    def get_value_from_dict_by_key(input_dict, input_key):
        output_dict = None
        for key, value in input_dict.items():
            if key == input_key:
                output_dict = value

        return output_dict

    @staticmethod
    @pass_context
    def get_did_info_dict(self, did_infos_dict, dids_dict, dsp_data_name):
        # 只有遍历所有的Did 才能找到每个DspData 对应的DataInfo 和DidInfo
        for key, value in dids_dict.items():
            for key1, value1 in value.items():
                if isinstance(value1, dict):
                    if "DefinitionRef" in value1.keys():
                        if value1[
                                "DefinitionRef"] == "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspDid/DcmDspDidSignal":
                            did_info_ref = value["DcmDspDidInfoRef"]["ValueRef"].split("/")[-1]
                            dsp_data_ref = value1["DcmDspDidDataRef"]["ValueRef"].split("/")[-1]
                            if dsp_data_ref == dsp_data_name:
                                did_info_ref_dict = Function.get_value_from_dict_by_key(did_infos_dict, did_info_ref)
                                # dsp_data_ref_dict = Function.get_value_from_dict_by_key(datas_dict, dsp_data_ref)
                                # data_info_ref = dsp_data_ref_dict["DcmDspDataInfoRef"]["ValueRef"].split("/")[-1]
                                # data_info_ref_dict = Function.get_value_from_dict_by_key(data_infos_dict, data_info_ref)
                                return did_info_ref_dict
        return {}

    @staticmethod
    @pass_context
    def get_func_client_server_read_write(self, use_port, ui_func, did_info_dict, func_name, read_write):
        if Function.is_use_data_port_client_server(self, use_port) is False:
            if Function.is_use_data_ui_func(self, use_port):
                return Function.get_function_from_str(self, ui_func)
            else:
                return "NULL"
        else:
            if Function.get_value_from_dict_by_key(did_info_dict, read_write) is None:
                return "NULL"
            else:
                return func_name

    @staticmethod
    @pass_context
    def get_func_client_server_readdatalength(self, use_port, ui_func, did_info_dict, func_name, read_write, data_type):
        if Function.is_use_data_port_client_server(self, use_port) is False:
            if Function.is_use_data_ui_func(self, use_port):
                return Function.get_function_from_str(self, ui_func)
            else:
                return "NULL"
        else:
            if Function.get_value_from_dict_by_key(did_info_dict, read_write) is None:
                return "NULL"
            else:
                if data_type == "UINT8_DYN":
                    return func_name
                else:
                    return "NULL"

    @staticmethod
    @pass_context
    def get_func_client_server_control_ResetToDefault(self, use_port, ui_func, did_info_dict, func_name):
        if Function.is_use_data_port_client_server(self, use_port) is False:
            if Function.is_use_data_ui_func(self, use_port):
                return Function.get_function_from_str(self, ui_func)
            else:
                return "NULL"
        else:
            if Function.get_value_from_dict_by_key(did_info_dict, "DcmDspDidControl") is None:
                return "NULL"
            else:
                if did_info_dict["DcmDspDidControl"]["DcmDspDidResetToDefault"]["Value"] == True:
                    return func_name
                else:
                    return "NULL"

    @staticmethod
    @pass_context
    def get_func_client_server_control_ShortTermAdjustment(self, use_port, ui_func, did_info_dict, func_name):
        if Function.is_use_data_port_client_server(self, use_port) is False:
            if Function.is_use_data_ui_func(self, use_port):
                return Function.get_function_from_str(self, ui_func)
            else:
                return "NULL"
        else:
            if Function.get_value_from_dict_by_key(did_info_dict, "DcmDspDidControl") is None:
                return "NULL"
            else:
                if did_info_dict["DcmDspDidControl"]["DcmDspDidShortTermAdjustment"]["Value"] == True:
                    return func_name
                else:
                    return "NULL"

    @staticmethod
    @pass_context
    def get_func_client_server_control_FreezeCurrentState(self, use_port, ui_func, did_info_dict, func_name):
        if Function.is_use_data_port_client_server(self, use_port) is False:
            if Function.is_use_data_ui_func(self, use_port):
                return Function.get_function_from_str(self, ui_func)
            else:
                return "NULL"
        else:
            if Function.get_value_from_dict_by_key(did_info_dict, "DcmDspDidControl") is None:
                return "NULL"
            else:
                if did_info_dict["DcmDspDidControl"]["DcmDspDidFreezeCurrentState"]["Value"] == True:
                    return func_name
                else:
                    return "NULL"

    @staticmethod
    @pass_context
    def get_func_client_server_control_ReturnControlToECU(self, use_port, ui_func, did_info_dict, func_name):
        if Function.is_use_data_port_client_server(self, use_port) is False:
            if Function.is_use_data_ui_func(self, use_port):
                return Function.get_function_from_str(self, ui_func)
            else:
                return "NULL"
        else:
            if Function.get_value_from_dict_by_key(did_info_dict, "DcmDspDidControl") is None:
                return "NULL"
            else:
                if ((did_info_dict["DcmDspDidControl"]["DcmDspDidFreezeCurrentState"]["Value"] == True) or
                    (did_info_dict["DcmDspDidControl"]["DcmDspDidResetToDefault"]["Value"] == True) or
                    (did_info_dict["DcmDspDidControl"]["DcmDspDidShortTermAdjustment"]["Value"] == True)):
                    return func_name
                else:
                    return "NULL"

    @staticmethod
    @pass_context
    def get_func_client_server_pid_read(self, pid_info):
        func_str = "NULL"
        if ((pid_info["DcmDspPidService"]["Value"] == "DCM_SERVICE_01") or
            (pid_info["DcmDspPidService"]["Value"] == 'DCM_SERVICE_01_02')):
            for k, v in pid_info.items():
                if isinstance(v, dict):
                    if "DefinitionRef" in v.keys():
                        if v["DefinitionRef"].split("/")[-1] == "DcmDspPidData":
                            for k1, v1 in v.items():
                                if isinstance(v1, dict):
                                    if "DefinitionRef" in v1.keys():
                                        if v1["DefinitionRef"].split("/")[-1] == "DcmDspPidService01":
                                            if v1["DcmDspPidDataUsePort"]["Value"] == "USE_DATA_SYNCH_CLIENT_SERVER":
                                                func_str = "Rte_Call_DataServices_" + k + "_ReadData"
                                            if v1["DcmDspPidDataUsePort"]["Value"] == "USE_DATA_SYNCH_FNC":
                                                func_str = Function.get_function_from_str(
                                                    self, v1["DcmDspPidDataReadFnc"]["Value"])
        return func_str

    @staticmethod
    @pass_context
    def get_ref_from_path(self, path, dict, ref_name) -> str:
        ref_indx = 0
        found = False

        if path == "" or path == "<empty>" or path == None:
            return "NULL"

        name = path.split("/")[-1]
        for key, value in dict.items():
            if key == name:
                found = True
                break
            else:
                ref_indx += 1

        if found is True:
            return ref_name + "[{}]".format(ref_indx)
        else:
            return "NULL"

    @staticmethod
    @pass_context
    def check_service_enable(self, id, list) -> bool:
        result = id in list
        return result

    @staticmethod
    @pass_context
    def use_jumptoboot(self, dcm_DcmDspSessionRow) -> bool:
        for key, value in dcm_DcmDspSessionRow.items():
            if value["DcmDspSessionForBoot"]["Value"] != "DCM_NO_BOOT":
                return True
        return False

    @staticmethod
    @pass_context
    def get_dsl_diag_resp_max_num_resp_pend(self, dcm_DcmDslDiagResp):
        result = 0
        if len(dcm_DcmDslDiagResp) > 0:
            result = dcm_DcmDslDiagResp["DcmDslDiagRespMaxNumRespPend"]["Value"]
        return result

    @staticmethod
    @pass_context
    def get_dsl_diag_response_Indication_func(self, dcm_DcmDslDiagResp):
        retInStr = "STD_OFF"
        funcName = "NULL"
        if len(dcm_DcmDslDiagResp) > 0:
            if dcm_DcmDslDiagResp["DcmDslDiagResponseIndicationFunc"]["Value"] != None and dcm_DcmDslDiagResp["DcmDslDiagResponseIndicationFunc"]["Value"] != '':
                retInStr = "STD_ON"
                funcName = dcm_DcmDslDiagResp["DcmDslDiagResponseIndicationFunc"]["Value"]
        return (retInStr, funcName)

    @staticmethod
    @pass_context
    def get_value_except_none(self, value):
        if value == '' or value == None:
            return 0
        return value

    @staticmethod
    @pass_context
    def get_fnc_value_except_none(self, value):
        if value == '' or value == None:
            return "NULL"
        return value

    @staticmethod
    @pass_context
    def get_fnc_value_enabled(self, dcm_DcmDspSessionRow) -> bool:
        for key, value in dcm_DcmDspSessionRow.items():
            if value["DcmDspSessionResponseMultiplePendingFnc"]["Value"] != '' and value["DcmDspSessionResponseMultiplePendingFnc"]["Value"] != None:
                return True
        return False

    @staticmethod
    @pass_context
    def get_protocol_rx_comm_channel_index(self, main_connection_index, main_connections, com_control_list):
        index = 0
        if main_connection_index < len(main_connections) and len(com_control_list) > 0:
            com_channel = list(
                main_connections.values())[main_connection_index]['DcmDslProtocolComMChannelRef']['ValueRef'].split(
                    "/")[-1]
            if com_channel in com_control_list:
                index = com_control_list.index(com_channel)
        return index

    @staticmethod
    @pass_context
    def get_protocol_tx_index_in_main_connection(self, main_connection_index, protocol_tx_list):
        for index, value in enumerate(list(protocol_tx_list.values())):
            if 'main_connection_index' in value.keys():
                if value['main_connection_index'] == main_connection_index:
                    return index
        return -1

    @staticmethod
    @pass_context
    def get_protocol_phy_rx_index_in_main_connection(self, main_connection_index, protocol_rx_list):
        for index, value in enumerate(list(protocol_rx_list.values())):
            if 'main_connection_index' in value.keys():
                if value['main_connection_index'] == main_connection_index:
                    if value["DcmDslProtocolRxAddrType"]["Value"] == 'DCM_PHYSICAL_TYPE':
                        return index
        return -1

    @staticmethod
    @pass_context
    def get_elememt_index_in_list(self, element, element_list):
        index = 255
        if element in element_list:
            index = element_list.index(element)
        return index

    @staticmethod
    @pass_context
    def is_comchannel_in_com_control(self, comchannel, all_channel):
        for key, value in all_channel.items():
            if comchannel == value["DcmDspAllComMChannelRef"]["ValueRef"].split("/")[-1]:
                return True
        return False

    @staticmethod
    @pass_context
    def get_connection_count_in_protocol(self, dcm_DcmDslProtocolRow):
        count = 0
        for key,value in dcm_DcmDslProtocolRow.items():
             if isinstance(value, dict):
                if "DefinitionRef" in value.keys():
                    if value['DefinitionRef'].split("/")[-1] == 'DcmDslConnection':
                        count += 1
        return count

    @staticmethod
    @pass_context
    def get_user_define_service_enabled_value(self, dcm_DcmDsdService):
        retInStr = "STD_OFF"
        for key,value in dcm_DcmDsdService.items():
            if value['DcmDsdSidTabFnc']['Value'] != None and value['DcmDsdSidTabFnc']['Value'] != '':
                retInStr = "STD_ON"
        return retInStr

    @staticmethod
    @pass_context
    def get_user_define_sub_service_enabled_value(self, dcm_DcmDsdService):
        retInStr = "STD_OFF"
        for key,value in dcm_DcmDsdService.items():
            if value['DcmDsdServiceUsed']['Value'] and value["DcmDsdSidTabSubfuncAvail"]["Value"]:
                for k, v in value.items():
                    if isinstance(v, dict):
                        if "DefinitionRef" in v.keys():
                            if v["DefinitionRef"].split("/")[-1] == "DcmDsdSubService":
                                if v['DcmDsdSubServiceFnc']['Value'] != None and v['DcmDsdSubServiceFnc']['Value'] != '':
                                    retInStr = "STD_ON"
        return retInStr

    @staticmethod
    @pass_context
    def get_nof_iocontrol_dids(self, dcm_DcmDspDidInfos):
        num = 0
        for key,value in dcm_DcmDspDidInfos.items():
            if 'DcmDspDidControl' in value.keys():
                num +=1
        return num

    @staticmethod
    @pass_context
    def isDspDataReadDataLengthFncEnable(self, dcm_DcmDspDatas, dcm_DcmDspDidInfos, dcm_DcmDspDids):
        for key, value in dcm_DcmDspDatas.items():
            did_info = Function.get_did_info_dict(self, dcm_DcmDspDidInfos, dcm_DcmDspDids, key)
            dspDataReadDataLengthFnc = Function.get_func_client_server_readdatalength(self, value["DcmDspDataUsePort"]["Value"], value["DcmDspDataReadDataLengthFnc"]["Value"], did_info, "Rte_Call_DataServices_" + key +"_ReadDataLength", "DcmDspDidRead", value["DcmDspDataType"]["Value"])
            if dspDataReadDataLengthFnc != None and dspDataReadDataLengthFnc != "NULL":
                return True
        return False

    @staticmethod
    @pass_context
    def isDspDataConditionCheckReadFncEnable(self, dcm_DcmDspDatas, dcm_DcmDspDidInfos, dcm_DcmDspDids):
        for key, value in dcm_DcmDspDatas.items():
            did_info = Function.get_did_info_dict(self, dcm_DcmDspDidInfos, dcm_DcmDspDids, key)
            dspDataConditionCheckReadFnc = Function.get_func_ConditionCheckRead(self, value["DcmDspDataUsePort"]["Value"], value["DcmDspDataConditionCheckReadFnc"]["Value"], did_info, "Rte_Call_DataServices_" + key + "_ConditionCheckRead",value["DcmDspDataConditionCheckReadFncUsed"]["Value"])
            if dspDataConditionCheckReadFnc != None and dspDataConditionCheckReadFnc != "NULL":
                return True
        return False

    @staticmethod
    @pass_context
    def isDspDataReadDataFncEnable(self, dcm_DcmDspDatas, dcm_DcmDspDidInfos, dcm_DcmDspDids):
        for key, value in dcm_DcmDspDatas.items():
            did_info = Function.get_did_info_dict(self, dcm_DcmDspDidInfos, dcm_DcmDspDids, key)
            dspDataReadDataFnc = Function.get_func_client_server_read_write(self, value["DcmDspDataUsePort"]["Value"], value["DcmDspDataReadFnc"]["Value"], did_info, "Rte_Call_DataServices_" + key +"_ReadData", "DcmDspDidRead")
            if dspDataReadDataFnc != None and dspDataReadDataFnc != "NULL":
                return True
        return False

    @staticmethod
    @pass_context
    def isDspDataWriteDataFncEnable(self, dcm_DcmDspDatas, dcm_DcmDspDidInfos, dcm_DcmDspDids):
        for key, value in dcm_DcmDspDatas.items():
            did_info = Function.get_did_info_dict(self, dcm_DcmDspDidInfos, dcm_DcmDspDids, key)
            dspDataWriteDataFnc = Function.get_func_client_server_read_write(self, value["DcmDspDataUsePort"]["Value"], value["DcmDspDataWriteFnc"]["Value"], did_info, "Rte_Call_DataServices_" + key + "_WriteData", "DcmDspDidWrite")
            if dspDataWriteDataFnc != None and dspDataWriteDataFnc != "NULL":
                return True
        return False

    @staticmethod
    @pass_context
    def isDspDataReturnControlToEcuFncEnable(self, dcm_DcmDspDatas, dcm_DcmDspDidInfos, dcm_DcmDspDids):
        for key, value in dcm_DcmDspDatas.items():
            did_info = Function.get_did_info_dict(self, dcm_DcmDspDidInfos, dcm_DcmDspDids, key)
            dspDataReturnControlToEcuFnc = Function.get_func_client_server_control_ReturnControlToECU(self, value["DcmDspDataUsePort"]["Value"], value["DcmDspDataReturnControlToEcuFnc"]["Value"], did_info, "Rte_Call_DataServices_" + key + "_ReturnControlToECU") # type: ignore
            if dspDataReturnControlToEcuFnc != None and dspDataReturnControlToEcuFnc != "NULL":
                return True
        return False

    @staticmethod
    @pass_context
    def isDspDataResetToDefaultFncEnable(self, dcm_DcmDspDatas, dcm_DcmDspDidInfos, dcm_DcmDspDids):
        for key, value in dcm_DcmDspDatas.items():
            did_info = Function.get_did_info_dict(self, dcm_DcmDspDidInfos, dcm_DcmDspDids, key)
            dspDataResetToDefaultFnc = Function.get_func_client_server_control_ResetToDefault(self, value["DcmDspDataUsePort"]["Value"], value["DcmDspDataResetToDefaultFnc"]["Value"], did_info, "Rte_Call_DataServices_" + key + "_ResetToDefault")
            if dspDataResetToDefaultFnc != None and dspDataResetToDefaultFnc != "NULL":
                return True
        return False

    @staticmethod
    @pass_context
    def isDspDataFreezeCurrentStateFncEnable(self, dcm_DcmDspDatas, dcm_DcmDspDidInfos, dcm_DcmDspDids):
        for key, value in dcm_DcmDspDatas.items():
            did_info = Function.get_did_info_dict(self, dcm_DcmDspDidInfos, dcm_DcmDspDids, key)
            dspDataFreezeCurrentStateFnc = Function.get_func_client_server_control_FreezeCurrentState(self, value["DcmDspDataUsePort"]["Value"], value["DcmDspDataFreezeCurrentStateFnc"]["Value"], did_info, "Rte_Call_DataServices_" + key + "_FreezeCurrentState")
            if dspDataFreezeCurrentStateFnc != None and dspDataFreezeCurrentStateFnc != "NULL":
                return True
        return False

    @staticmethod
    @pass_context
    def isDspDataShortTermAdjustmentFncEnable(self, dcm_DcmDspDatas, dcm_DcmDspDidInfos, dcm_DcmDspDids):
        for key, value in dcm_DcmDspDatas.items():
            did_info = Function.get_did_info_dict(self, dcm_DcmDspDidInfos, dcm_DcmDspDids, key)
            dspDataShortTermAdjustmentFnc = Function.get_func_client_server_control_ShortTermAdjustment(self, value["DcmDspDataUsePort"]["Value"], value["DcmDspDataShortTermAdjustmentFnc"]["Value"], did_info, "Rte_Call_DataServices_" + key + "_ShortTermAdjustment")
            if dspDataShortTermAdjustmentFnc != None and dspDataShortTermAdjustmentFnc != "NULL":
                return True
        return False