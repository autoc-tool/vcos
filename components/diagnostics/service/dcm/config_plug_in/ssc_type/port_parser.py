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

from ..generator import filter
import hashlib
from basic_func_module.autosar_utils.autosar_util import AutosarUtil


def check_enabled(enabled):
    if isinstance(enabled, str):
        if enabled == "true" or enabled == "TRUE":
            return True
    return False


def get_value_from_dict_by_key(input_dict, input_key):
    output_dict = {}
    for key, value in input_dict.items():
        if key == input_key:
            output_dict = value
    return output_dict


def update_short_name_length(short_name) -> str:
    if short_name and len(short_name) > 128:
        hash_obj = hashlib.sha1(short_name[120:].encode(encoding="utf-8"))
        hash_str = hash_obj.hexdigest()
        short_name = short_name[:120] + hash_str[:8]
    return short_name


class SecurityAccess(object):

    def __init__(self, input_key, input_value):
        self.interface_dict = {}
        self.data_types = {"elements": []}
        self.rport = {}
        self.SecurityLevel = input_key
        self.input_dict = input_value
        self.DcmDspSecurityUsePort = input_value["DcmDspSecurityUsePort"]["Value"]
        self.DcmDspSecurityAttemptCounterEnabled = input_value["DcmDspSecurityAttemptCounterEnabled"]["Value"]
        self.DcmDspSecurityADRSize = input_value["DcmDspSecurityADRSize"]["Value"]
        self.set_interface_dict()
        self.set_data_types_dict()
        self.set_rport()

    def set_interface_dict(self):
        if self.DcmDspSecurityUsePort == "USE_ASYNCH_CLIENT_SERVER":
            possible_errors = [{
                "short_name": "E_OK",
                "error_code": 0
            }, {
                "short_name": "E_NOT_OK",
                "error_code": 1
            }, {
                "short_name": "DCM_E_PENDING",
                "error_code": 10
            }, {
                "short_name": "DCM_E_COMPARE_KEY_FAILED",
                "error_code": 11
            }]
            self.interface_dict = {
                "short_name": "SecurityAccess_" + self.SecurityLevel,  # 8.8.3.1
                "is_server": True,
                "is_synch": False,
                "port_type": "rport",
                "possible_errors": possible_errors,
                "operations": []
            }
            self.interface_dict["operations"].append(self.operation_CompareKey())
            self.interface_dict["operations"].append(self.operation_GetSeed())
            if str(self.DcmDspSecurityAttemptCounterEnabled).lower() == "true":
                self.interface_dict["operations"].append(self.operation_GetSecurityAttemptCounter())
                self.interface_dict["operations"].append(self.operation_SetSecurityAttemptCounter())

    def set_data_types_dict(self):
        if self.DcmDspSecurityUsePort == "USE_ASYNCH_CLIENT_SERVER":
            Dcm_KeyArray_element = {
                "short_name": "Dcm_KeyArray_" + self.SecurityLevel + "Type",  # 8.8.1.39
                "category": "Array",
                "sw_data_def": {
                    "calibration_access": "READ-ONLY"
                },
                "sub_element": {
                    "array_size": self.input_dict["DcmDspSecurityKeySize"]["Value"],
                    "element_type": "uint8"
                }
            }
            self.data_types["elements"].append(Dcm_KeyArray_element)

            Dcm_SeedArray_element = {
                "short_name": "Dcm_SeedArray_" + self.SecurityLevel + "Type",  # 8.8.1.40
                "category": "Array",
                "sw_data_def": {
                    "calibration_access": "READ-ONLY"
                },
                "sub_element": {
                    "array_size": self.input_dict["DcmDspSecuritySeedSize"]["Value"],
                    "element_type": "uint8"
                }
            }
            self.data_types["elements"].append(Dcm_SeedArray_element)

            #Dcm_SecurityAccessDataRecordArray_{SecurityLevel}Type  8.8.1.41
            #校验规则待修复
            if self.DcmDspSecurityADRSize != "" and self.DcmDspSecurityADRSize != 0:
                Dcm_SecurityAccessDataRecordArray_element = {
                    "short_name": "Dcm_SecurityAccessDataRecordArray_" + self.SecurityLevel + "Type",  # 8.8.1.41
                    "category": "Array",
                    "sw_data_def": {
                        "calibration_access": "READ-ONLY"
                    },
                    "sub_element": {
                        "array_size": self.DcmDspSecurityADRSize,
                        "element_type": "uint8"
                    }
                }
                self.data_types["elements"].append(Dcm_SecurityAccessDataRecordArray_element)

    def set_rport(self):
        if self.DcmDspSecurityUsePort == "USE_ASYNCH_CLIENT_SERVER":
            self.rport = {
                "short_name": "SecurityAccess_" + self.SecurityLevel,  # 8.8.5.15
                "interface_type": "client_server",
                "interface_ref": "SecurityAccess_" + self.SecurityLevel
            }

    def operation_CompareKey(self):
        operation = {
            "short_name": "CompareKey",
            "arguments": [{
                "short_name": "Key",
                "IsType": True,
                "TypeTref": "Dcm_KeyArray_" + self.SecurityLevel + "Type",
                "Direction": "IN"
            }, {
                "short_name": "OpStatus",
                "IsType": True,
                "TypeTref": "Dcm_OpStatusType",
                "Direction": "IN"
            }, {
                "short_name": "error_code",
                "IsType": True,
                "TypeTref": "Dcm_NegativeResponseCodeType",
                "Direction": "OUT"
            }],
            "possible_errors": ["E_OK", "E_NOT_OK", "DCM_E_PENDING", "DCM_E_COMPARE_KEY_FAILED"]
        }
        return operation

    def operation_GetSecurityAttemptCounter(self):
        operation = {
            "short_name": "GetSecurityAttemptCounter",
            "arguments": [{
                "short_name": "OpStatus",
                "IsType": True,
                "TypeTref": "Dcm_OpStatusType",
                "Direction": "IN"
            }, {
                "short_name": "AttemptCounter",
                "IsType": False,
                "TypeTref": "uint8",
                "Direction": "OUT"
            }],
            "possible_errors": ["E_OK", "E_NOT_OK", "DCM_E_PENDING"]
        }
        return operation

    def operation_SetSecurityAttemptCounter(self):
        operation = {
            "short_name": "SetSecurityAttemptCounter",
            "arguments": [{
                "short_name": "OpStatus",
                "IsType": True,
                "TypeTref": "Dcm_OpStatusType",
                "Direction": "IN"
            }, {
                "short_name": "AttemptCounter",
                "IsType": False,
                "TypeTref": "uint8",
                "Direction": "IN"
            }],
            "possible_errors": ["E_OK", "E_NOT_OK", "DCM_E_PENDING"]
        }
        return operation

    def operation_GetSeed(self):
        arguments = []
        #校验规则待修复
        if self.DcmDspSecurityADRSize == "" or self.DcmDspSecurityADRSize == 0:
            arguments = [{
                "short_name": "OpStatus",
                "IsType": True,
                "TypeTref": "Dcm_OpStatusType",
                "Direction": "IN"
            }, {
                "short_name": "Seed",
                "IsType": True,
                "TypeTref": "Dcm_SeedArray_" + self.SecurityLevel + "Type",
                "Direction": "OUT"
            }, {
                "short_name": "error_code",
                "IsType": True,
                "TypeTref": "Dcm_NegativeResponseCodeType",
                "Direction": "OUT"
            }]
        else:
            arguments = [{
                "short_name": "SecurityAccessDataRecord",
                "IsType": True,
                "TypeTref": "Dcm_SecurityAccessDataRecordArray_" + self.SecurityLevel + "Type",
                "Direction": "IN"
            }, {
                "short_name": "OpStatus",
                "IsType": True,
                "TypeTref": "Dcm_OpStatusType",
                "Direction": "IN"
            }, {
                "short_name": "Seed",
                "IsType": True,
                "TypeTref": "Dcm_SeedArray_" + self.SecurityLevel + "Type",
                "Direction": "OUT"
            }, {
                "short_name": "error_code",
                "IsType": True,
                "TypeTref": "Dcm_NegativeResponseCodeType",
                "Direction": "OUT"
            }]
        operation = {
            "short_name": "GetSeed",
            "arguments": arguments,
            "possible_errors": ["E_OK", "E_NOT_OK", "DCM_E_PENDING"]
        }
        return operation

    def get_interface_dict(self):
        return self.interface_dict

    def get_data_types(self):
        return self.data_types

    def get_rport(self):
        return self.rport


class DataServices_Data(object):

    def __init__(self, input_key, input_dspdata, input_didinfo, input_datainfo):
        self.interface = {"interface_type": "", "interface_dict": {}}
        self.port = {"port_type": "", "port_dict": {}}
        self.data_types = {"elements": [], "data_constrs": [], "compu_methods": []}
        self.operation_list = []
        self.type_check_list = []  #重复Types定义剔除
        self.Data = input_key
        self.input_dspdata_dict = input_dspdata
        self.input_didinfo_dict = input_didinfo
        self.input_datainfo_dict = input_datainfo
        self.DcmDspDataUsePort = input_dspdata["DcmDspDataUsePort"]["Value"]
        self.set_interface_port_dict()

    def set_interface_port_dict(self):
        #C/S interface
        if (self.DcmDspDataUsePort == "USE_DATA_SYNCH_CLIENT_SERVER" or
                self.DcmDspDataUsePort == "USE_DATA_ASYNCH_CLIENT_SERVER" or
                self.DcmDspDataUsePort == "USE_DATA_ASYNCH_CLIENT_SERVER_ERROR"):
            self.input_dspdata_dict["DcmDspDataType"]["Value"] = "UINT8"
            self.append_cs_interface_operations()
            self.interface["interface_type"] = "client_server"
            self.interface["interface_dict"] = {
                "short_name": "DataServices_" + self.Data,  # 8.8.3.2
                "is_server": True,
                "is_synch": False,
                "port_type": "rport",
                "possible_errors": [{
                    "short_name": "E_OK",
                    "error_code": 0
                }, {
                    "short_name": "E_NOT_OK",
                    "error_code": 1
                }, {
                    "short_name": "DCM_E_PENDING",
                    "error_code": 10
                }],
                "operations": self.operation_list
            }
            if self.DcmDspDataUsePort == "USE_DATA_SYNCH_CLIENT_SERVER":
                self.interface["interface_dict"].update({"is_synch": True})
            self.port["port_type"] = "rport"
            self.port["port_dict"] = {
                "short_name": "DataServices_" + self.Data,  # 8.8.5.4
                "interface_type": "client_server",
                "interface_ref": "DataServices_" + self.Data
            }

    def append_cs_interface_operations(self):
        self.operation_ConditionCheckRead()
        self.operation_FreezeCurrentState()
        self.operation_GetScalingInformation()
        self.operation_ReadData()
        self.operation_ReadDataLength()
        self.operation_ResetToDefault()
        self.operation_ReturnControlToECU()
        self.operation_ShortTermAdjustment()
        self.operation_WriteData()

    def check_use_port(self):
        if self.DcmDspDataUsePort == "USE_DATA_ASYNCH_CLIENT_SERVER" or self.DcmDspDataUsePort == "USE_DATA_ASYNCH_CLIENT_SERVER_ERROR":
            return "ASYNC_PORT"
        elif self.DcmDspDataUsePort == "USE_DATA_SYNCH_CLIENT_SERVER":
            return "SYNCH_PORT"
        else:
            return "ERROR_PORT"

    def check_did_read(self):
        if "DcmDspDidRead" in self.input_didinfo_dict.keys():
            return True
        else:
            return False

    def check_did_write(self):
        if "DcmDspDidWrite" in self.input_didinfo_dict.keys():
            return True
        else:
            return False

    def check_ConditionCheckRead(self):
        if "DcmDspDataConditionCheckReadFncUsed" in self.input_dspdata_dict.keys():
            if "TRUE" == str(self.input_dspdata_dict["DcmDspDataConditionCheckReadFncUsed"]["Value"]).upper():
                return True
        return False

    def check_FreezeCurrentState(self):
        if "DcmDspDidControl" in self.input_didinfo_dict.keys():
            if "TRUE" == str(
                    self.input_didinfo_dict["DcmDspDidControl"]["DcmDspDidFreezeCurrentState"]["Value"]).upper():
                return True
        return False

    def check_GetScalingInformation(self):
        if "DcmDspDataScalingInfoSize" in self.input_datainfo_dict.keys():
            if self.input_datainfo_dict["DcmDspDataScalingInfoSize"]["Value"] > 0:  #校验规则待修复
                return True
        return False

    def check_ReadDataLength(self):
        if self.input_dspdata_dict["DcmDspDataType"]["Value"] == "UINT8_DYN":
            return True
        return False

    def check_ResetToDefault(self):
        if "DcmDspDidControl" in self.input_didinfo_dict.keys():
            if "TRUE" == str(self.input_didinfo_dict["DcmDspDidControl"]["DcmDspDidResetToDefault"]["Value"]).upper():
                return True
        return False

    def check_ReturnControlToECU(self):
        if "DcmDspDidControl" in self.input_didinfo_dict.keys():
            if ((self.check_FreezeCurrentState() == True) or (self.check_ResetToDefault() == True) or
                (self.check_ShortTermAdjustment() == True)):
                return True
        return False

    def check_ShortTermAdjustment(self):
        if "DcmDspDidControl" in self.input_didinfo_dict.keys():
            if "TRUE" == str(
                    self.input_didinfo_dict["DcmDspDidControl"]["DcmDspDidShortTermAdjustment"]["Value"]).upper():
                return True
        return False

    def operation_ConditionCheckRead(self):
        if self.check_ConditionCheckRead() != True or self.check_did_read() != True:
            return

        operation = {
            "short_name": "ConditionCheckRead",
        }
        use_port = self.check_use_port()
        if use_port == "ERROR_PORT":
            return

        Arguments = []
        PossibleErrors = ["E_OK", "E_NOT_OK"]
        if use_port == "ASYNC_PORT":
            Arguments.append({
                "short_name": "OpStatus",
                "IsType": True,
                "TypeTref": "Dcm_OpStatusType",
                "Direction": "IN"
            })
            PossibleErrors.append("DCM_E_PENDING")
        Arguments.append({
            "short_name": "ErrorCode",
            "IsType": True,
            "TypeTref": "Dcm_NegativeResponseCodeType",
            "Direction": "OUT"
        })
        operation.update({"arguments": Arguments})
        operation.update({"possible_errors": PossibleErrors})
        self.operation_list.append(operation)

    def operation_FreezeCurrentState(self):
        if self.check_FreezeCurrentState() != True:
            return
        operation = {
            "short_name": "FreezeCurrentState",
        }
        use_port = self.check_use_port()
        if use_port == "ERROR_PORT":
            return
        # did_control_mask = self.input_didinfo_dict["DcmDspDidControl"]["DcmDspDidControlMask"]["Value"]
        did_control_mask = "DCM_CONTROLMASK_INTERNAL"

        Arguments = []
        PossibleErrors = ["E_OK", "E_NOT_OK"]
        if use_port == "ASYNC_PORT":
            Arguments.append({
                "short_name": "OpStatus",
                "IsType": True,
                "TypeTref": "Dcm_OpStatusType",
                "Direction": "IN"
            })
            PossibleErrors.append("DCM_E_PENDING")
        if did_control_mask == "DCM_CONTROLMASK_EXTERNAL":
            Arguments.append({
                "short_name": "controlMask",
                "IsType": True,
                "TypeTref": "Dcm_ControlMask" + self.Data + "Type",
                "Direction": "IN"
            })
            control_mask_size = self.input_didinfo_dict["DcmDspDidControl"]["DcmDspDidControlMaskSize"]["Value"]
            self.update_control_mask_type(control_mask_size)
        Arguments.append({
            "short_name": "ErrorCode",
            "IsType": True,
            "TypeTref": "Dcm_NegativeResponseCodeType",
            "Direction": "OUT"
        })
        operation.update({"arguments": Arguments})
        operation.update({"possible_errors": PossibleErrors})
        self.operation_list.append(operation)

    def operation_GetScalingInformation(self):
        if self.check_GetScalingInformation() != True:
            return
        operation = {
            "short_name": "GetScalingInformation",
        }
        use_port = self.check_use_port()
        if use_port == "ERROR_PORT":
            return

        Arguments = []
        PossibleErrors = ["E_OK", "E_NOT_OK"]

        if use_port == "ASYNC_PORT":
            Arguments.append({
                "short_name": "OpStatus",
                "IsType": True,
                "TypeTref": "Dcm_OpStatusType",
                "Direction": "IN"
            })
            PossibleErrors.append("DCM_E_PENDING")
        Arguments.append(
            {
                "short_name": "ScalingInfo",
                "IsType": True,
                "TypeTref": "Dcm_ScalingInfoArray_" + self.Data + "Type",
                "Direction": "OUT"
            }, {
                "short_name": "ErrorCode",
                "IsType": True,
                "TypeTref": "Dcm_NegativeResponseCodeType",
                "Direction": "OUT"
            })
        data_scaling_size = self.input_datainfo_dict["DcmDspDataScalingInfoSize"]["Value"]
        self.update_scaling_info_array_type(data_scaling_size)
        operation.update({"arguments": Arguments})
        operation.update({"possible_errors": PossibleErrors})
        self.operation_list.append(operation)

    def operation_ReadData(self):
        if self.check_did_read() != True:
            return

        operation = {
            "short_name": "ReadData",
        }

        Arguments = []
        PossibleErrors = ["E_OK", "E_NOT_OK"]

        if self.DcmDspDataUsePort == "USE_DATA_ASYNCH_CLIENT_SERVER_ERROR":  # 不支持此选项
            Arguments = [{
                "short_name": "OpStatus",
                "IsType": True,
                "TypeTref": "Dcm_OpStatusType",
                "Direction": "IN"
            }, {
                "short_name": "Data",
                "IsType": True,
                "TypeTref": "Dcm_DataElement_" + self.Data + "Type",
                "Direction": "OUT"
            }, {
                "short_name": "ErrorCode",
                "IsType": True,
                "TypeTref": "Dcm_NegativeResponseCodeType",
                "Direction": "OUT"
            }]
            PossibleErrors.append("DCM_E_PENDING")
        elif self.DcmDspDataUsePort == "USE_DATA_ASYNCH_CLIENT_SERVER":
            Arguments = [{
                "short_name": "OpStatus",
                "IsType": True,
                "TypeTref": "Dcm_OpStatusType",
                "Direction": "IN"
            }, {
                "short_name": "Data",
                "IsType": True,
                "TypeTref": "Dcm_DataElement_" + self.Data + "Type",
                "Direction": "OUT"
            }]
            PossibleErrors.append("DCM_E_PENDING")
        elif self.DcmDspDataUsePort == "USE_DATA_SYNCH_CLIENT_SERVER":
            Arguments = [{
                "short_name": "Data",
                "IsType": True,
                "TypeTref": "Dcm_DataElement_" + self.Data + "Type",
                "Direction": "OUT"
            }]
        else:
            return
        self.update_data_elem_type()
        operation.update({"arguments": Arguments})
        operation.update({"possible_errors": PossibleErrors})
        self.operation_list.append(operation)

    def operation_ReadDataLength(self):
        if self.check_ReadDataLength() != True or self.check_did_read() != True:
            return

        operation = {
            "short_name": "ReadDataLength",
        }

        use_port = self.check_use_port()
        if use_port == "ERROR_PORT":
            return

        Arguments = []
        PossibleErrors = ["E_OK", "E_NOT_OK"]

        if use_port == "ASYNC_PORT":
            Arguments.append({
                "short_name": "OpStatus",
                "IsType": True,
                "TypeTref": "Dcm_OpStatusType",
                "Direction": "IN"
            })
            PossibleErrors.append("DCM_E_PENDING")
        Arguments.append({"short_name": "DataLength", "IsType": False, "TypeTref": "uint16", "Direction": "OUT"})
        operation.update({"arguments": Arguments})
        operation.update({"possible_errors": PossibleErrors})
        self.operation_list.append(operation)

    def operation_ResetToDefault(self):
        if self.check_ResetToDefault() != True:
            return

        operation = {
            "short_name": "ResetToDefault",
        }
        use_port = self.check_use_port()
        if use_port == "ERROR_PORT":
            return

        # did_control_mask = self.input_didinfo_dict["DcmDspDidControl"]["DcmDspDidControlMask"]["Value"]
        did_control_mask = "DCM_CONTROLMASK_INTERNAL"
        Arguments = []
        PossibleErrors = ["E_OK", "E_NOT_OK"]
        if use_port == "ASYNC_PORT":
            Arguments.append({
                "short_name": "OpStatus",
                "IsType": True,
                "TypeTref": "Dcm_OpStatusType",
                "Direction": "IN"
            })
            PossibleErrors.append("DCM_E_PENDING")
        if did_control_mask == "DCM_CONTROLMASK_EXTERNAL":
            Arguments.append({
                "short_name": "controlMask",
                "IsType": True,
                "TypeTref": "Dcm_ControlMask" + self.Data + "Type",
                "Direction": "IN"
            })
            control_mask_size = self.input_didinfo_dict["DcmDspDidControl"]["DcmDspDidControlMaskSize"]["Value"]
            self.update_control_mask_type(control_mask_size)
        Arguments.append({
            "short_name": "ErrorCode",
            "IsType": True,
            "TypeTref": "Dcm_NegativeResponseCodeType",
            "Direction": "OUT"
        })
        operation.update({"arguments": Arguments})
        operation.update({"possible_errors": PossibleErrors})
        self.operation_list.append(operation)

    def operation_ReturnControlToECU(self):
        if self.check_ReturnControlToECU() != True:
            return

        use_port = self.check_use_port()
        if use_port == "ERROR_PORT":
            return

        operation = {
            "short_name": "ReturnControlToECU",
        }

        # did_control_mask = self.input_didinfo_dict["DcmDspDidControl"]["DcmDspDidControlMask"]["Value"]
        did_control_mask = "DCM_CONTROLMASK_INTERNAL"
        Arguments = []
        PossibleErrors = ["E_OK", "E_NOT_OK"]
        if did_control_mask == "DCM_CONTROLMASK_EXTERNAL":
            Arguments.append({
                "short_name": "controlMask",
                "IsType": True,
                "TypeTref": "Dcm_ControlMask" + self.Data + "Type",
                "Direction": "IN"
            })
            control_mask_size = self.input_didinfo_dict["DcmDspDidControl"]["DcmDspDidControlMaskSize"]["Value"]
            self.update_control_mask_type(control_mask_size)
        Arguments.append({
            "short_name": "ErrorCode",
            "IsType": True,
            "TypeTref": "Dcm_NegativeResponseCodeType",
            "Direction": "OUT"
        })

        operation.update({"arguments": Arguments})
        operation.update({"possible_errors": PossibleErrors})
        self.operation_list.append(operation)

    def operation_ShortTermAdjustment(self):
        if self.check_ShortTermAdjustment() != True:
            return

        operation = {
            "short_name": "ShortTermAdjustment",
        }

        use_port = self.check_use_port()
        if use_port == "ERROR_PORT":
            return

        # did_control_mask = self.input_didinfo_dict["DcmDspDidControl"]["DcmDspDidControlMask"]["Value"]
        did_control_mask = "DCM_CONTROLMASK_INTERNAL"
        Arguments = [{
            "short_name": "ControlStateInfo",
            "IsType": True,
            "TypeTref": "Dcm_DataArrayTypeUint8_" + self.Data + "Type",
            "Direction": "IN"
        }]
        self.update_data_array_type_uint8()
        PossibleErrors = ["E_OK", "E_NOT_OK"]
        if self.check_ReadDataLength() == True:
            Arguments.append({"short_name": "DataLength", "IsType": False, "TypeTref": "uint16", "Direction": "IN"})
        if use_port == "ASYNC_PORT":
            Arguments.append({
                "short_name": "OpStatus",
                "IsType": True,
                "TypeTref": "Dcm_OpStatusType",
                "Direction": "IN"
            })
            PossibleErrors.append("DCM_E_PENDING")
        if did_control_mask == "DCM_CONTROLMASK_EXTERNAL":
            Arguments.append({
                "short_name": "controlMask",
                "IsType": True,
                "TypeTref": "Dcm_ControlMask" + self.Data + "Type",
                "Direction": "IN"
            })
            control_mask_size = self.input_didinfo_dict["DcmDspDidControl"]["DcmDspDidControlMaskSize"]["Value"]
            self.update_control_mask_type(control_mask_size)
        Arguments.append({
            "short_name": "ErrorCode",
            "IsType": True,
            "TypeTref": "Dcm_NegativeResponseCodeType",
            "Direction": "OUT"
        })
        operation.update({"arguments": Arguments})
        operation.update({"possible_errors": PossibleErrors})
        self.operation_list.append(operation)

    def operation_WriteData(self):
        if self.check_did_write() != True:
            return

        operation = {
            "short_name": "WriteData",
        }

        use_port = self.check_use_port()
        if use_port == "ERROR_PORT":
            return

        Arguments = [{
            "short_name": "Data",
            "IsType": True,
            "TypeTref": "Dcm_DataElement_" + self.Data + "Type",
            "Direction": "IN"
        }]
        self.update_data_elem_type()
        PossibleErrors = ["E_OK", "E_NOT_OK"]
        if self.check_ReadDataLength() == True:
            Arguments.append({"short_name": "DataLength", "IsType": False, "TypeTref": "uint16", "Direction": "IN"})
        if use_port == "ASYNC_PORT":
            Arguments.append({
                "short_name": "OpStatus",
                "IsType": True,
                "TypeTref": "Dcm_OpStatusType",
                "Direction": "IN"
            })
            PossibleErrors.append("DCM_E_PENDING")
        Arguments.append({
            "short_name": "ErrorCode",
            "IsType": True,
            "TypeTref": "Dcm_NegativeResponseCodeType",
            "Direction": "OUT"
        })
        operation.update({"arguments": Arguments})
        operation.update({"possible_errors": PossibleErrors})
        self.operation_list.append(operation)

    def update_control_mask_type(self, input):
        element = {
            "short_name": "Dcm_ControlMask" + self.Data + "Type",  # 8.8.1.43
            "category": "Type",
        }
        if input == 0x1:
            element["sw_data_def"] = {
                "IsType": False,
                "implementation_data_type": "uint8",
            }
        elif input == 0x2:
            element["sw_data_def"] = {
                "IsType": False,
                "implementation_data_type": "uint16",
            }
        elif input == 0x4:
            element["sw_data_def"] = {
                "IsType": False,
                "implementation_data_type": "uint32",
            }
        elif input >= 0x5:
            element["category"] = "Array"
            element["sw_data_def"] = {"calibration_access": "READ-ONLY"}
            element["sub_element"] = {"array_size": input, "element_type": "uint8"}
        if element["short_name"] not in self.type_check_list:
            self.type_check_list.append(element["short_name"])
            self.data_types["elements"].append(element)

    def update_scaling_info_array_type(self, input):
        element = {
            "short_name": "Dcm_ScalingInfoArray_" + self.Data + "Type",  # 8.8.1.14
            "category": "Type",
            "sw_data_def": {
                "calibration_access": "READ-ONLY"
            },
            "sub_element": {
                "array_size": input,
                "element_type": "uint8"
            }
        }
        if element["short_name"] not in self.type_check_list:
            self.type_check_list.append(element["short_name"])
            self.data_types["elements"].append(element)

    def update_data_elem_type(self):
        element = {
            "short_name": "Dcm_DataElement_" + self.Data + "Type",  # 8.8.1.7
            "category": "Array",
            "sw_data_def": {
                "calibration_access": "READ-ONLY"
            },
            "sub_element": {
                "array_size": self.input_dspdata_dict["DcmDspDataByteSize"]["Value"],
                "element_type": str(self.input_dspdata_dict["DcmDspDataType"]["Value"]).lower()
            }
        }
        if element["short_name"] not in self.type_check_list:
            self.type_check_list.append(element["short_name"])
            self.data_types["elements"].append(element)

    def update_data_array_type_uint8(self):
        element = {
            "short_name": "Dcm_DataArrayTypeUint8_" + self.Data + "Type",  # 8.8.1.8
            "category": "Array",
            "sw_data_def": {
                "calibration_access": "READ-ONLY"
            },
            "sub_element": {
                "array_size": self.input_dspdata_dict["DcmDspDataByteSize"]["Value"],
                "element_type": "uint8"
            }
        }
        if element["short_name"] not in self.type_check_list:
            self.type_check_list.append(element["short_name"])
            self.data_types["elements"].append(element)

    def get_interface_dict(self):
        return self.interface

    def get_data_types(self):
        return self.data_types

    def get_port(self):
        return self.port


class DataServices_PidData(object):

    def __init__(self, data_name, input_pid_data):
        self.interface = {"interface_type": "", "interface_dict": {}}
        self.port = {"port_type": "", "port_dict": {}}
        self.data_types = {"elements": [], "data_constrs": [], "compu_methods": []}
        self.operation_list = []
        self.data_name = data_name
        self.pid_data = input_pid_data
        self.set_interface_port_dict()

    def set_interface_port_dict(self):
        for service01_key, service01_value in self.pid_data.items():
            if isinstance(service01_value, dict):
                if "DefinitionRef" in service01_value.keys():
                    if service01_value["DefinitionRef"].split("/")[-1] == "DcmDspPidService01":
                        if service01_value["DcmDspPidDataUsePort"]["Value"] == "USE_DATA_SYNCH_CLIENT_SERVER":
                            self.interface["interface_type"] = "client_server"
                            self.operation_ReadData()
                            self.interface["interface_dict"] = {
                              "short_name": "DataServices_" + self.data_name,  # 8.8.3.2
                              "is_server": True,
                              "is_synch": True,
                              "port_type": "rport",
                              "possible_errors": [{
                                  "short_name": "E_OK",
                                  "error_code": 0
                              }, {
                                  "short_name": "E_NOT_OK",
                                  "error_code": 1
                              }, {
                                  "short_name": "DCM_E_PENDING",
                                  "error_code": 10
                              }],
                              "operations": self.operation_list
                            }
                            self.port["port_type"] = "rport"
                            self.port["port_dict"] = {
                                "short_name": "DataServices_" + self.data_name,  # 8.8.5.4
                                "interface_type": "client_server",
                                "interface_ref": "DataServices_" + self.data_name
                            }

    def operation_ReadData(self):
        operation = {
            "short_name": "ReadData",
        }

        PossibleErrors = ["E_OK", "E_NOT_OK"]
        Arguments = [{
            "short_name": "Data",
            "IsType": True,
            "TypeTref": "Dcm_DataElement_" + self.data_name + "Type",
            "Direction": "OUT"
        }]

        self.update_data_elem_type()
        operation.update({"arguments": Arguments})
        operation.update({"possible_errors": PossibleErrors})
        self.operation_list.append(operation)

    def update_data_elem_type(self):
        element = {
            "short_name": "Dcm_DataElement_" + self.data_name + "Type",  # 8.8.1.7
            "category": "Array",
            "sw_data_def": {
                "calibration_access": "READ-ONLY"
            },
            "sub_element": {
                "array_size": self.pid_data["DcmDspPidDataByteSize"]["Value"],
                "element_type": "uint8"
            }
        }
        self.data_types["elements"].append(element)

    def get_interface_dict(self):
        return self.interface

    def get_data_types(self):
        return self.data_types

    def get_port(self):
        return self.port


class RoutineServices(object):

    def __init__(self, input_key, input_routine):
        self.RoutineName = input_key
        self.input_routine_dict = input_routine
        self.rport = {
            "short_name": "RoutineServices_" + self.RoutineName,  # 8.8.5.14
            "interface_type": "client_server",
            "interface_ref": "RoutineServices_" + self.RoutineName
        }
        self.interface_dict = {}
        self.operation_list = []
        self.data_types = {"elements": []}
        self.DcmDspRequestRoutineResultsInSignal = {}
        self.DcmDspRequestRoutineResultsOutSignal = {}
        self.DcmDspStartRoutineInSignal = {}
        self.DcmDspStartRoutineOutSignal = {}
        self.DcmDspStopRoutineInSignal = {}
        self.DcmDspStopRoutineOutSignal = {}
        self.updateRoutineSignal()
        self.set_interface_dict()
        self.set_datatypes()

    def updateRoutineSignal(self):
        if "DcmDspRequestRoutineResults" in self.input_routine_dict.keys():
            if "DcmDspRequestRoutineResultsIn" in self.input_routine_dict["DcmDspRequestRoutineResults"].keys():
                self.DcmDspRequestRoutineResultsInSignal = self.input_routine_dict["DcmDspRequestRoutineResults"][
                    "DcmDspRequestRoutineResultsIn"]
                del self.DcmDspRequestRoutineResultsInSignal["DefinitionRef"]
            if "DcmDspRequestRoutineResultsOut" in self.input_routine_dict["DcmDspRequestRoutineResults"].keys():
                self.DcmDspRequestRoutineResultsOutSignal = self.input_routine_dict["DcmDspRequestRoutineResults"][
                    "DcmDspRequestRoutineResultsOut"]
                del self.DcmDspRequestRoutineResultsOutSignal["DefinitionRef"]
        if "DcmDspStartRoutine" in self.input_routine_dict.keys():
            if "DcmDspStartRoutineIn" in self.input_routine_dict["DcmDspStartRoutine"].keys():
                self.DcmDspStartRoutineInSignal = self.input_routine_dict["DcmDspStartRoutine"]["DcmDspStartRoutineIn"]
                del self.DcmDspStartRoutineInSignal["DefinitionRef"]
            if "DcmDspStartRoutineOut" in self.input_routine_dict["DcmDspStartRoutine"].keys():
                self.DcmDspStartRoutineOutSignal = self.input_routine_dict["DcmDspStartRoutine"][
                    "DcmDspStartRoutineOut"]
                del self.DcmDspStartRoutineOutSignal["DefinitionRef"]
        if "DcmDspStopRoutine" in self.input_routine_dict.keys():
            if "DcmDspStopRoutineIn" in self.input_routine_dict["DcmDspStopRoutine"].keys():
                self.DcmDspStopRoutineInSignal = self.input_routine_dict["DcmDspStopRoutine"]["DcmDspStopRoutineIn"]
                del self.DcmDspStopRoutineInSignal["DefinitionRef"]
            if "DcmDspStopRoutineOut" in self.input_routine_dict["DcmDspStopRoutine"].keys():
                self.DcmDspStopRoutineOutSignal = self.input_routine_dict["DcmDspStopRoutine"]["DcmDspStopRoutineOut"]
                del self.DcmDspStopRoutineOutSignal["DefinitionRef"]

    def set_interface_dict(self):
        self.append_cs_interface_operations()
        self.interface_dict = {
            "short_name": "RoutineServices_" + self.RoutineName,  # 8.8.3.5
            "is_server": True,
            "is_synch": True,
            "port_type": "rport",
            "possible_errors": [{
                "short_name": "E_OK",
                "error_code": 0
            }, {
                "short_name": "E_NOT_OK",
                "error_code": 1
            }, {
                "short_name": "DCM_E_PENDING",
                "error_code": 10
            }, {
                "short_name": "DCM_E_FORCE_RCRRP",
                "error_code": 12
            }],
            "operations": self.operation_list
        }

    def get_interface(self):
        return self.interface_dict

    def get_data_types(self):
        return self.data_types

    def get_port(self):
        return self.rport

    def append_cs_interface_operations(self):
        self.operation_RequestResults()
        self.operation_RequestResultsConfirmation()
        self.operation_Start()
        self.operation_StartConfirmation()
        self.operation_Stop()
        self.operation_StopConfirmation()

    def check_RequestResultsConfirmation(self):
        if "DcmDspRequestRoutineResults" in self.input_routine_dict.keys():
            if "DcmDspRequestRoutineResultsConfirmationEnabled" in self.input_routine_dict[
                    "DcmDspRequestRoutineResults"]:
                if "TRUE" == str(self.input_routine_dict["DcmDspRequestRoutineResults"]
                                 ["DcmDspRequestRoutineResultsConfirmationEnabled"]["Value"]).upper():
                    return True
        return False

    def check_StartConfirmation(self):
        if "DcmDspStartRoutine" in self.input_routine_dict.keys():
            if "DcmDspStartRoutineConfirmationEnabled" in self.input_routine_dict["DcmDspStartRoutine"]:
                if "TRUE" == str(self.input_routine_dict["DcmDspStartRoutine"]["DcmDspStartRoutineConfirmationEnabled"]
                                 ["Value"]).upper():
                    return True
        return False

    def check_StopConfirmation(self):
        if "DcmDspStopRoutine" in self.input_routine_dict.keys():
            if "DcmDspStopRoutineConfirmationEnabled" in self.input_routine_dict["DcmDspStopRoutine"]:
                if "TRUE" == str(self.input_routine_dict["DcmDspStopRoutine"]["DcmDspStopRoutineConfirmationEnabled"]
                                 ["Value"]).upper():
                    return True
        return False

    def operation_RequestResults(self):
        if "DcmDspRequestRoutineResults" not in self.input_routine_dict.keys():
            return

        data_length_flag = False
        Arguments = []
        if self.DcmDspRequestRoutineResultsInSignal:
            for signal_short_name, signal_dict in self.DcmDspRequestRoutineResultsInSignal.items():
                Argument = {
                    "short_name": "DataIn_" + signal_short_name,
                    "IsType": True,
                    "TypeTref": "Dcm_RequestDataIn_" + self.RoutineName + "_" + signal_short_name + "Type",
                    "Direction": "IN"
                }
                if signal_dict["DcmDspRoutineSignalType"]["Value"] == "VARIABLE_LENGTH":
                    data_length_flag = True
                    Argument.update({
                        "TypeTref":
                            "Dcm_RequestFlexibleInArrayData_" + self.RoutineName + "_" + signal_short_name + "Type"
                    })
                Arguments.append(Argument)
        Arguments.append({"short_name": "OpStatus", "IsType": True, "TypeTref": "Dcm_OpStatusType", "Direction": "IN"})
        if self.DcmDspRequestRoutineResultsOutSignal:
            for signal_short_name, signal_dict in self.DcmDspRequestRoutineResultsOutSignal.items():
                Argument = {
                    "short_name": "DataOut_" + signal_short_name,
                    "IsType": True,
                    "TypeTref": "Dcm_RequestDataOut_" + self.RoutineName + "_" + signal_short_name + "Type",
                    "Direction": "OUT"
                }
                if signal_dict["DcmDspRoutineSignalType"]["Value"] == "VARIABLE_LENGTH":
                    data_length_flag = True
                    Argument.update({
                        "TypeTref":
                            "Dcm_RequestFlexibleOutArrayData_" + self.RoutineName + "_" + signal_short_name + "Type"
                    })
                Arguments.append(Argument)
        if data_length_flag:
            Arguments.append({
                "short_name": "currentDataLength",
                "IsType": False,
                "TypeTref": "uint16",
                "Direction": "OUT"
            })
        Arguments.append({
            "short_name": "ErrorCode",
            "IsType": True,
            "TypeTref": "Dcm_NegativeResponseCodeType",
            "Direction": "OUT"
        })

        operation = {
            "short_name": "RequestResults",
            "arguments": Arguments,
            "possible_errors": ["E_OK", "E_NOT_OK", "DCM_E_PENDING", "DCM_E_FORCE_RCRRP"]
        }
        self.operation_list.append(operation)

    def operation_RequestResultsConfirmation(self):
        if self.check_RequestResultsConfirmation() != True:
            return

        operation = {
            "short_name": "RequestResultsConfirmation",
            "arguments": [{
                "short_name": "ConfirmationStatus",
                "IsType": True,
                "TypeTref": "Dcm_ConfirmationStatusType",
                "Direction": "IN"
            }],
            "possible_errors": ["E_OK", "E_NOT_OK"]
        }

        self.operation_list.append(operation)

    def operation_Start(self):
        if "DcmDspStartRoutine" not in self.input_routine_dict.keys():
            return

        data_length_flag = False
        Arguments = []
        if self.DcmDspStartRoutineInSignal:
            for signal_short_name, signal_dict in self.DcmDspStartRoutineInSignal.items():
                Argument = {
                    "short_name": "DataIn_" + signal_short_name,
                    "IsType": True,
                    "TypeTref": "Dcm_StartDataIn_" + self.RoutineName + "_" + signal_short_name + "Type",
                    "Direction": "IN"
                }
                if signal_dict["DcmDspRoutineSignalType"]["Value"] == "VARIABLE_LENGTH":
                    data_length_flag = True
                    Argument.update({
                        "TypeTref":
                            "Dcm_StartFlexibleInArrayData_" + self.RoutineName + "_" + signal_short_name + "Type"
                    })
                Arguments.append(Argument)
        Arguments.append({"short_name": "OpStatus", "IsType": True, "TypeTref": "Dcm_OpStatusType", "Direction": "IN"})
        if self.DcmDspStartRoutineOutSignal:
            for signal_short_name, signal_dict in self.DcmDspStartRoutineOutSignal.items():
                Argument = {
                    "short_name": "DataOut_" + signal_short_name,
                    "IsType": True,
                    "TypeTref": "Dcm_StartDataOut_" + self.RoutineName + "_" + signal_short_name + "Type",
                    "Direction": "OUT"
                }
                if signal_dict["DcmDspRoutineSignalType"]["Value"] == "VARIABLE_LENGTH":
                    data_length_flag = True
                    Argument.update({
                        "TypeTref":
                            "Dcm_StartFlexibleOutArrayData_" + self.RoutineName + "_" + signal_short_name + "Type"
                    })
                Arguments.append(Argument)
        if data_length_flag:
            Arguments.append({
                "short_name": "currentDataLength",
                "IsType": False,
                "TypeTref": "uint16",
                "Direction": "OUT"
            })
        Arguments.append({
            "short_name": "ErrorCode",
            "IsType": True,
            "TypeTref": "Dcm_NegativeResponseCodeType",
            "Direction": "OUT"
        })

        operation = {
            "short_name": "Start",
            "arguments": Arguments,
            "possible_errors": ["E_OK", "E_NOT_OK", "DCM_E_PENDING", "DCM_E_FORCE_RCRRP"]
        }
        self.operation_list.append(operation)

    def operation_StartConfirmation(self):
        if self.check_StartConfirmation() != True:
            return

        operation = {
            "short_name": "StartConfirmation",
            "arguments": [{
                "short_name": "ConfirmationStatus",
                "IsType": True,
                "TypeTref": "Dcm_ConfirmationStatusType",
                "Direction": "IN"
            }],
            "possible_errors": ["E_OK", "E_NOT_OK"]
        }

        self.operation_list.append(operation)

    def operation_Stop(self):
        if "DcmDspStopRoutine" not in self.input_routine_dict.keys():
            return

        data_length_flag = False
        Arguments = []
        if self.DcmDspStopRoutineInSignal:
            for signal_short_name, signal_dict in self.DcmDspStopRoutineInSignal.items():
                Argument = {
                    "short_name": "DataIn_" + signal_short_name,
                    "IsType": True,
                    "TypeTref": "Dcm_StopDataIn_" + self.RoutineName + "_" + signal_short_name + "Type",
                    "Direction": "IN"
                }
                if signal_dict["DcmDspRoutineSignalType"]["Value"] == "VARIABLE_LENGTH":
                    data_length_flag = True
                    Argument.update({
                        "TypeTref": "Dcm_StopFlexibleInArrayData_" + self.RoutineName + "_" + signal_short_name + "Type"
                    })
                Arguments.append(Argument)
        Arguments.append({"short_name": "OpStatus", "IsType": True, "TypeTref": "Dcm_OpStatusType", "Direction": "IN"})
        if self.DcmDspStopRoutineOutSignal:
            for signal_short_name, signal_dict in self.DcmDspStopRoutineOutSignal.items():
                Argument = {
                    "short_name": "DataOut_" + signal_short_name,
                    "IsType": True,
                    "TypeTref": "Dcm_StopDataOut_" + self.RoutineName + "_" + signal_short_name + "Type",
                    "Direction": "OUT"
                }
                if signal_dict["DcmDspRoutineSignalType"]["Value"] == "VARIABLE_LENGTH":
                    data_length_flag = True
                    Argument.update({
                        "TypeTref":
                            "Dcm_StopFlexibleOutArrayData_" + self.RoutineName + "_" + signal_short_name + "Type"
                    })
                Arguments.append(Argument)
        if data_length_flag:
            Arguments.append({
                "short_name": "currentDataLength",
                "IsType": False,
                "TypeTref": "uint16",
                "Direction": "OUT"
            })
        Arguments.append({
            "short_name": "ErrorCode",
            "IsType": True,
            "TypeTref": "Dcm_NegativeResponseCodeType",
            "Direction": "OUT"
        })

        operation = {
            "short_name": "Stop",
            "arguments": Arguments,
            "possible_errors": ["E_OK", "E_NOT_OK", "DCM_E_PENDING", "DCM_E_FORCE_RCRRP"]
        }
        self.operation_list.append(operation)

    def operation_StopConfirmation(self):
        if self.check_StopConfirmation() != True:
            return

        operation = {
            "short_name": "StopConfirmation",
            "arguments": [{
                "short_name": "ConfirmationStatus",
                "IsType": True,
                "TypeTref": "Dcm_ConfirmationStatusType",
                "Direction": "IN"
            }],
            "possible_errors": ["E_OK", "E_NOT_OK"]
        }

        self.operation_list.append(operation)

    def set_datatypes(self):
        if self.DcmDspRequestRoutineResultsInSignal:
            for signal_short_name, signal_dict in self.DcmDspRequestRoutineResultsInSignal.items():
                # Dcm_RequestFlexibleInArrayData_{Routine}_{Signal}Type
                if signal_dict["DcmDspRoutineSignalType"]["Value"] == "VARIABLE_LENGTH":
                    self.data_types["elements"].append({
                        "short_name":
                            "Dcm_RequestFlexibleInArrayData_" + self.RoutineName + "_" + signal_short_name +
                            "Type",  # 8.8.1.22
                        "category":
                            "Array",
                        "sw_data_def": {
                            "calibration_access": "READ-ONLY"
                        },
                        "sub_element": {
                            "array_size": signal_dict["DcmDspRoutineParameterSize"]["Value"],
                            "element_type": "uint8"
                        }
                    })
                elif "_" in signal_dict["DcmDspRoutineSignalType"]["Value"]:
                    # Dcm_RequestDataIn_{Routine}_{Signal}Type
                    self.data_types["elements"].append({
                        "short_name": "Dcm_RequestDataIn_" + self.RoutineName + "_" + signal_short_name +
                                      "Type",  # 8.8.1.18
                        "category": "Type",
                        "sw_data_def": {
                            "IsType":
                                True,
                            "implementation_data_type":
                                "Dcm_RequestDataIn_" + self.RoutineName + "_" + signal_short_name + "ArrayType",
                        }
                    })
                    element_type = str(signal_dict["DcmDspRoutineSignalType"]["Value"].split("_")[0]).lower()
                    array_size = signal_dict["DcmDspRoutineParameterSize"]["Value"]
                    if "16" in element_type:
                        array_size = int(signal_dict["DcmDspRoutineParameterSize"]["Value"]) / 2
                    if "32" in element_type:
                        array_size = int(signal_dict["DcmDspRoutineParameterSize"]["Value"]) / 4
                    # Dcm_RequestDataIn_{Routine}_{Signal}ArrayType
                    self.data_types["elements"].append({
                        "short_name": "Dcm_RequestDataIn_" + self.RoutineName + "_" + signal_short_name +
                                      "ArrayType",  # 8.8.1.20
                        "category": "Array",
                        "sw_data_def": {
                            "calibration_access": "READ-ONLY"
                        },
                        "sub_element": {
                            "array_size": array_size,
                            "element_type": element_type
                        }
                    })
                elif signal_dict["DcmDspRoutineSignalType"]["Value"] != "BOOLEAN":
                    # Dcm_RequestDataIn_{Routine}_{Signal}Type
                    self.data_types["elements"].append({
                        "short_name": "Dcm_RequestDataIn_" + self.RoutineName + "_" + signal_short_name +
                                      "Type",  # 8.8.1.18
                        "category": "Type",
                        "sw_data_def": {
                            "IsType":
                                True,
                            "implementation_data_type":
                                "Dcm_RequestDataIn_" + self.RoutineName + "_" + signal_short_name + "PrimitivType",
                        }
                    })
                    # Dcm_RequestDataIn_{Routine}_{Signal}PrimitiveType
                    self.data_types["elements"].append({
                        "short_name":
                            "Dcm_RequestDataIn_" + self.RoutineName + "_" + signal_short_name +
                            "PrimitivType",  # 8.8.1.16
                        "category":
                            "Type",
                        "sw_data_def": {
                            "IsType": False,
                            "implementation_data_type": str(signal_dict["DcmDspRoutineSignalType"]["Value"]).lower()
                        }
                    })

        if self.DcmDspRequestRoutineResultsOutSignal:
            for signal_short_name, signal_dict in self.DcmDspRequestRoutineResultsOutSignal.items():
                if signal_dict["DcmDspRoutineSignalType"]["Value"] == "VARIABLE_LENGTH":
                    # Dcm_RequestFlexibleOutArrayData_{Routine}_{Signal}Type
                    self.data_types["elements"].append({
                        "short_name":
                            "Dcm_RequestFlexibleOutArrayData_" + self.RoutineName + "_" + signal_short_name +
                            "Type",  # 8.8.1.21
                        "category":
                            "Array",
                        "sw_data_def": {
                            "calibration_access": "READ-ONLY"
                        },
                        "sub_element": {
                            "array_size": signal_dict["DcmDspRoutineParameterSize"]["Value"],
                            "element_type": "uint8"
                        }
                    })
                elif "_" in signal_dict["DcmDspRoutineSignalType"]["Value"]:
                    # Dcm_RequestDataOut_{Routine}_{Signal}Type
                    self.data_types["elements"].append({
                        "short_name": "Dcm_RequestDataOut_" + self.RoutineName + "_" + signal_short_name +
                                      "Type",  # 8.8.1.17
                        "category": "Type",
                        "sw_data_def": {
                            "IsType":
                                True,
                            "implementation_data_type":
                                "Dcm_RequestDataOut_" + self.RoutineName + "_" + signal_short_name + "ArrayType",
                        }
                    })
                    element_type = str(signal_dict["DcmDspRoutineSignalType"]["Value"].split("_")[0]).lower()
                    array_size = signal_dict["DcmDspRoutineParameterSize"]["Value"]
                    if "16" in element_type:
                        array_size = int(signal_dict["DcmDspRoutineParameterSize"]["Value"]) / 2
                    if "32" in element_type:
                        array_size = int(signal_dict["DcmDspRoutineParameterSize"]["Value"]) / 4
                    # Dcm_RequestDataOut_{Routine}_{Signal}ArrayType
                    self.data_types["elements"].append({
                        "short_name": "Dcm_RequestDataOut_" + self.RoutineName + "_" + signal_short_name +
                                      "ArrayType",  # 8.8.1.19
                        "category": "Array",
                        "sw_data_def": {
                            "calibration_access": "READ-ONLY"
                        },
                        "sub_element": {
                            "array_size": array_size,
                            "element_type": element_type
                        }
                    })
                elif signal_dict["DcmDspRoutineSignalType"]["Value"] != "BOOLEAN":
                    # Dcm_RequestDataOut_{Routine}_{Signal}Type
                    self.data_types["elements"].append({
                        "short_name": "Dcm_RequestDataOut_" + self.RoutineName + "_" + signal_short_name +
                                      "Type",  # 8.8.1.17
                        "category": "Type",
                        "sw_data_def": {
                            "IsType":
                                True,
                            "implementation_data_type":
                                "Dcm_RequestDataOut_" + self.RoutineName + "_" + signal_short_name + "PrimitivType",
                        }
                    })
                    # Dcm_RequestDataOut_{Routine}_{Signal}PrimitivType
                    self.data_types["elements"].append({
                        "short_name":
                            "Dcm_RequestDataOut_" + self.RoutineName + "_" + signal_short_name +
                            "PrimitivType",  # 8.8.1.15
                        "category":
                            "Type",
                        "sw_data_def": {
                            "IsType": False,
                            "implementation_data_type": str(signal_dict["DcmDspRoutineSignalType"]["Value"]).lower()
                        }
                    })

        if self.DcmDspStartRoutineInSignal:
            for signal_short_name, signal_dict in self.DcmDspStartRoutineInSignal.items():
                # Dcm_StartFlexibleInArrayData_{Routine}_{Signal}Type
                if signal_dict["DcmDspRoutineSignalType"]["Value"] == "VARIABLE_LENGTH":
                    self.data_types["elements"].append({
                        "short_name":
                            "Dcm_StartFlexibleInArrayData_" + self.RoutineName + "_" + signal_short_name +
                            "Type",  # 8.8.1.29
                        "category":
                            "Array",
                        "sw_data_def": {
                            "calibration_access": "READ-ONLY"
                        },
                        "sub_element": {
                            "array_size": signal_dict["DcmDspRoutineParameterSize"]["Value"],
                            "element_type": "uint8"
                        }
                    })
                elif "_" in signal_dict["DcmDspRoutineSignalType"]["Value"]:
                    # Dcm_StartDataIn_{Routine}_{Signal}Type
                    self.data_types["elements"].append({
                        "short_name": "Dcm_StartDataIn_" + self.RoutineName + "_" + signal_short_name +
                                      "Type",  # 8.8.1.24
                        "category": "Type",
                        "sw_data_def": {
                            "IsType":
                                True,
                            "implementation_data_type":
                                "Dcm_StartDataIn_" + self.RoutineName + "_" + signal_short_name + "ArrayType",
                        }
                    })
                    element_type = str(signal_dict["DcmDspRoutineSignalType"]["Value"].split("_")[0]).lower()
                    array_size = signal_dict["DcmDspRoutineParameterSize"]["Value"]
                    if "16" in element_type:
                        array_size = int(signal_dict["DcmDspRoutineParameterSize"]["Value"]) / 2
                    if "32" in element_type:
                        array_size = int(signal_dict["DcmDspRoutineParameterSize"]["Value"]) / 4
                    # Dcm_StartDataIn_{Routine}_{Signal}ArrayType
                    self.data_types["elements"].append({
                        "short_name": "Dcm_StartDataIn_" + self.RoutineName + "_" + signal_short_name +
                                      "ArrayType",  # 8.8.1.25
                        "category": "Array",
                        "sw_data_def": {
                            "calibration_access": "READ-ONLY"
                        },
                        "sub_element": {
                            "array_size": array_size,
                            "element_type": element_type
                        }
                    })
                elif signal_dict["DcmDspRoutineSignalType"]["Value"] != "BOOLEAN":
                    # Dcm_StartDataIn_{Routine}_{Signal}Type
                    self.data_types["elements"].append({
                        "short_name": "Dcm_StartDataIn_" + self.RoutineName + "_" + signal_short_name +
                                      "Type",  # 8.8.1.24
                        "category": "Type",
                        "sw_data_def": {
                            "IsType":
                                True,
                            "implementation_data_type":
                                "Dcm_StartDataIn_" + self.RoutineName + "_" + signal_short_name + "PrimitivType",
                        }
                    })
                    # Dcm_StartDataIn_{Routine}_{Signal}PrimitiveType
                    self.data_types["elements"].append({
                        "short_name": "Dcm_StartDataIn_" + self.RoutineName + "_" + signal_short_name +
                                      "PrimitivType",  # 8.8.1.19
                        "category": "Type",
                        "sw_data_def": {
                            "IsType": False,
                            "implementation_data_type": str(signal_dict["DcmDspRoutineSignalType"]["Value"]).lower()
                        }
                    })

        if self.DcmDspStartRoutineOutSignal:
            for signal_short_name, signal_dict in self.DcmDspStartRoutineOutSignal.items():
                if signal_dict["DcmDspRoutineSignalType"]["Value"] == "VARIABLE_LENGTH":
                    # Dcm_StartFlexibleOutArrayData_{Routine}_{Signal}Type
                    self.data_types["elements"].append({
                        "short_name":
                            "Dcm_StartFlexibleOutArrayData_" + self.RoutineName + "_" + signal_short_name +
                            "Type",  # 8.8.1.30
                        "category":
                            "Array",
                        "sw_data_def": {
                            "calibration_access": "READ-ONLY"
                        },
                        "sub_element": {
                            "array_size": signal_dict["DcmDspRoutineParameterSize"]["Value"],
                            "element_type": "uint8"
                        }
                    })
                elif "_" in signal_dict["DcmDspRoutineSignalType"]["Value"]:
                    # Dcm_StartDataOut_{Routine}_{Signal}Type
                    self.data_types["elements"].append({
                        "short_name": "Dcm_StartDataOut_" + self.RoutineName + "_" + signal_short_name +
                                      "Type",  # 8.8.1.27
                        "category": "Type",
                        "sw_data_def": {
                            "IsType":
                                True,
                            "implementation_data_type":
                                "Dcm_StartDataOut_" + self.RoutineName + "_" + signal_short_name + "ArrayType",
                        }
                    })
                    element_type = str(signal_dict["DcmDspRoutineSignalType"]["Value"].split("_")[0]).lower()
                    array_size = signal_dict["DcmDspRoutineParameterSize"]["Value"]
                    if "16" in element_type:
                        array_size = int(signal_dict["DcmDspRoutineParameterSize"]["Value"]) / 2
                    if "32" in element_type:
                        array_size = int(signal_dict["DcmDspRoutineParameterSize"]["Value"]) / 4
                    # Dcm_StartDataOut_{Routine}_{Signal}ArrayType
                    self.data_types["elements"].append({
                        "short_name": "Dcm_StartDataOut_" + self.RoutineName + "_" + signal_short_name +
                                      "ArrayType",  # 8.8.1.28
                        "category": "Array",
                        "sw_data_def": {
                            "calibration_access": "READ-ONLY"
                        },
                        "sub_element": {
                            "array_size": array_size,
                            "element_type": element_type
                        }
                    })
                elif signal_dict["DcmDspRoutineSignalType"]["Value"] != "BOOLEAN":
                    # Dcm_StartDataOut_{Routine}_{Signal}Type
                    self.data_types["elements"].append({
                        "short_name": "Dcm_StartDataOut_" + self.RoutineName + "_" + signal_short_name +
                                      "Type",  # 8.8.1.27
                        "category": "Type",
                        "sw_data_def": {
                            "IsType":
                                True,
                            "implementation_data_type":
                                "Dcm_StartDataOut_" + self.RoutineName + "_" + signal_short_name + "PrimitivType",
                        }
                    })
                    # Dcm_StartDataOut_{Routine}_{Signal}PrimitivType
                    self.data_types["elements"].append({
                        "short_name": "Dcm_StartDataOut_" + self.RoutineName + "_" + signal_short_name +
                                      "PrimitivType",  # 8.8.1.26
                        "category": "Type",
                        "sw_data_def": {
                            "IsType": False,
                            "implementation_data_type": str(signal_dict["DcmDspRoutineSignalType"]["Value"]).lower()
                        }
                    })

        if self.DcmDspStopRoutineInSignal:
            for signal_short_name, signal_dict in self.DcmDspStopRoutineInSignal.items():
                # Dcm_StopFlexibleInArrayData_{Routine}_{Signal}Type
                if signal_dict["DcmDspRoutineSignalType"]["Value"] == "VARIABLE_LENGTH":
                    self.data_types["elements"].append({
                        "short_name":
                            "Dcm_StopFlexibleInArrayData_" + self.RoutineName + "_" + signal_short_name +
                            "Type",  # 8.8.1.37
                        "category":
                            "Array",
                        "sw_data_def": {
                            "calibration_access": "READ-ONLY"
                        },
                        "sub_element": {
                            "array_size": signal_dict["DcmDspRoutineParameterSize"]["Value"],
                            "element_type": "uint8"
                        }
                    })
                elif "_" in signal_dict["DcmDspRoutineSignalType"]["Value"]:
                    # Dcm_StopDataIn_{Routine}_{Signal}Type
                    self.data_types["elements"].append({
                        "short_name": "Dcm_StopDataIn_" + self.RoutineName + "_" + signal_short_name +
                                      "Type",  # 8.8.1.32
                        "category": "Type",
                        "sw_data_def": {
                            "IsType":
                                True,
                            "implementation_data_type":
                                "Dcm_StopDataIn_" + self.RoutineName + "_" + signal_short_name + "ArrayType",
                        }
                    })
                    element_type = str(signal_dict["DcmDspRoutineSignalType"]["Value"].split("_")[0]).lower()
                    array_size = signal_dict["DcmDspRoutineParameterSize"]["Value"]
                    if "16" in element_type:
                        array_size = int(signal_dict["DcmDspRoutineParameterSize"]["Value"]) / 2
                    if "32" in element_type:
                        array_size = int(signal_dict["DcmDspRoutineParameterSize"]["Value"]) / 4
                    # Dcm_StopDataIn_{Routine}_{Signal}ArrayType
                    self.data_types["elements"].append({
                        "short_name": "Dcm_StopDataIn_" + self.RoutineName + "_" + signal_short_name +
                                      "ArrayType",  # 8.8.1.33
                        "category": "Array",
                        "sw_data_def": {
                            "calibration_access": "READ-ONLY"
                        },
                        "sub_element": {
                            "array_size": array_size,
                            "element_type": element_type
                        }
                    })
                elif signal_dict["DcmDspRoutineSignalType"]["Value"] != "BOOLEAN":
                    # Dcm_StopDataIn_{Routine}_{Signal}Type
                    self.data_types["elements"].append({
                        "short_name": "Dcm_StopDataIn_" + self.RoutineName + "_" + signal_short_name +
                                      "Type",  # 8.8.1.32
                        "category": "Type",
                        "sw_data_def": {
                            "IsType":
                                True,
                            "implementation_data_type":
                                "Dcm_StopDataIn_" + self.RoutineName + "_" + signal_short_name + "PrimitivType",
                        }
                    })
                    # Dcm_StopDataIn_{Routine}_{Signal}PrimitiveType
                    self.data_types["elements"].append({
                        "short_name": "Dcm_StopDataIn_" + self.RoutineName + "_" + signal_short_name +
                                      "PrimitivType",  # 8.8.1.31
                        "category": "Type",
                        "sw_data_def": {
                            "IsType": False,
                            "implementation_data_type": str(signal_dict["DcmDspRoutineSignalType"]["Value"]).lower()
                        }
                    })

        if self.DcmDspStopRoutineOutSignal:
            for signal_short_name, signal_dict in self.DcmDspStopRoutineOutSignal.items():
                if signal_dict["DcmDspRoutineSignalType"]["Value"] == "VARIABLE_LENGTH":
                    # Dcm_StopFlexibleOutArrayData_{Routine}_{Signal}Type
                    self.data_types["elements"].append({
                        "short_name":
                            "Dcm_StopFlexibleOutArrayData_" + self.RoutineName + "_" + signal_short_name +
                            "Type",  # 8.8.1.38
                        "category":
                            "Array",
                        "sw_data_def": {
                            "calibration_access": "READ-ONLY"
                        },
                        "sub_element": {
                            "array_size": signal_dict["DcmDspRoutineParameterSize"]["Value"],
                            "element_type": "uint8"
                        }
                    })
                elif "_" in signal_dict["DcmDspRoutineSignalType"]["Value"]:
                    # Dcm_StopDataOut_{Routine}_{Signal}Type
                    self.data_types["elements"].append({
                        "short_name": "Dcm_StopDataOut_" + self.RoutineName + "_" + signal_short_name +
                                      "Type",  # 8.8.1.35
                        "category": "Type",
                        "sw_data_def": {
                            "IsType":
                                True,
                            "implementation_data_type":
                                "Dcm_StopDataOut_" + self.RoutineName + "_" + signal_short_name + "ArrayType",
                        }
                    })
                    element_type = str(signal_dict["DcmDspRoutineSignalType"]["Value"].split("_")[0]).lower()
                    array_size = signal_dict["DcmDspRoutineParameterSize"]["Value"]
                    if "16" in element_type:
                        array_size = int(signal_dict["DcmDspRoutineParameterSize"]["Value"]) / 2
                    if "32" in element_type:
                        array_size = int(signal_dict["DcmDspRoutineParameterSize"]["Value"]) / 4
                    # Dcm_StopDataOut_{Routine}_{Signal}ArrayType
                    self.data_types["elements"].append({
                        "short_name": "Dcm_StopDataOut_" + self.RoutineName + "_" + signal_short_name +
                                      "ArrayType",  # 8.8.1.36
                        "category": "Array",
                        "sw_data_def": {
                            "calibration_access": "READ-ONLY"
                        },
                        "sub_element": {
                            "array_size": array_size,
                            "element_type": element_type
                        }
                    })
                elif signal_dict["DcmDspRoutineSignalType"]["Value"] != "BOOLEAN":
                    # Dcm_StopDataOut_{Routine}_{Signal}Type
                    self.data_types["elements"].append({
                        "short_name": "Dcm_StopDataOut_" + self.RoutineName + "_" + signal_short_name +
                                      "Type",  # 8.8.1.35
                        "category": "Type",
                        "sw_data_def": {
                            "IsType":
                                True,
                            "implementation_data_type":
                                "Dcm_StopDataOut_" + self.RoutineName + "_" + signal_short_name + "PrimitivType",
                        }
                    })
                    # Dcm_StopDataOut_{Routine}_{Signal}PrimitivType
                    self.data_types["elements"].append({
                        "short_name": "Dcm_StopDataOut_" + self.RoutineName + "_" + signal_short_name +
                                      "PrimitivType",  # 8.8.1.34
                        "category": "Type",
                        "sw_data_def": {
                            "IsType": False,
                            "implementation_data_type": str(signal_dict["DcmDspRoutineSignalType"]["Value"]).lower()
                        }
                    })


class DcmPortDerive(object):

    def __init__(self, ecucs_autosar_dict=None):
        self.ecucs_autosar_dict = ecucs_autosar_dict
        self.init_dcm_config()
        self.update_variants()
        self.update_data_types()
        self.update_mode_declaration_to_date()
        self.update_main_function_runnable()
        self.update_server_runnables()
        self.update_invoked_events()
        self.config_module_dict = {}
        self.set_config_dict()

    def init_dcm_config(self):
        self.mode_declaration_groups = [
            {
                "short_name":
                    "DiagnosticSessionControl",  # 8.8.6.1
                "initial_value":
                    "DEFAULT_SESSION",
                "modes": [
                    "DEFAULT_SESSION", "PROGRAMMING_SESSION", "EXTENDED_DIAGNOSTIC_SESSION",
                    "SAFETY_SYSTEM_DIAGNOSTIC_SESSION"
                ]
            },
            {
                "short_name":
                    "EcuReset",  # 8.8.6.2
                "initial_value":
                    "NONE",
                "modes": [
                    "NONE", "HARD", "KEYONOFF", "SOFT", "JUMPTOBOOTLOADER", "JUMPTOSYSSUPPLIERBOOTLOADER", "EXECUTE"
                ]
            },
            {
                "short_name": "ModeRapidPowerShutDown",  # 8.8.6.3
                "initial_value": "ENABLE_RAPIDPOWERSHUTDOWN",
                "modes": ["ENABLE_RAPIDPOWERSHUTDOWN", "DISABLE_RAPIDPOWERSHUTDOWN"]
            },
            {
                "short_name":
                    "CommunicationControl",  # 8.8.6.4
                "initial_value":
                    "ENABLE_RX_TX_NORM_NM",
                "modes": [
                    "ENABLE_RX_TX_NORM", "ENABLE_RX_DISABLE_TX_NORM", "DISABLE_RX_ENABLE_TX_NORM",
                    "DISABLE_RX_TX_NORMAL", "ENABLE_RX_TX_NM", "ENABLE_RX_DISABLE_TX_NM", "DISABLE_RX_ENABLE_TX_NM",
                    "DISABLE_RX_TX_NM", "ENABLE_RX_TX_NORM_NM", "ENABLE_RX_DISABLE_TX_NORM_NM",
                    "DISABLE_RX_ENABLE_TX_NORM_NM", "DISABLE_RX_TX_NORM_NM"
                ]
            },
            {
                "short_name": "ControlDTCSetting",  # 8.8.6.5
                "initial_value": "ENABLEDTCSETTING",
                "modes": ["ENABLEDTCSETTING", "DISABLEDTCSETTING"]
            },
            {
                "short_name": "ResponseOnEvent",  # 8.8.6.6
                "initial_value": "EVENT_CLEARED",
                "modes": ["EVENT_STARTED", "EVENT_STOPPED", "EVENT_CLEARED"]
            },
            {
                "short_name": "SecurityAccess",  # 8.8.6.7
                "initial_value": "SEC_LEV_LOCKED",
                "modes": []
            },
            {
                "short_name": "AuthenticationState",  # 8.8.6.8
                "initial_value": "DEAUTHENTICATED",
                "modes": ["DEAUTHENTICATED", "AUTHENTICATED"]
            }
        ]
        self.data_types = {
            "elements": [
                {
                    "short_name": "Dcm_OpStatusType",  # 8.8.1.1
                    "category": "Type",
                    "sw_data_def": {
                        "compu_method_ref": "Dcm_OpStatusType",
                        "data_constr_ref": "Dcm_OpStatusType_Constr",
                        "IsType": False,
                        "implementation_data_type": "uint8",
                    }
                },
                {
                    "short_name": "Dcm_ConfirmationStatusType",  # 8.8.1.2
                    "category": "Type",
                    "sw_data_def": {
                        "compu_method_ref": "Dcm_ConfirmationStatusType",
                        "data_constr_ref": "Dcm_ConfirmationStatusType_Constr",
                        "IsType": False,
                        "implementation_data_type": "uint8",
                    }
                },
                {
                    "short_name": "Dcm_SecLevelType",  # 8.8.1.3
                    "category": "Type",
                    "sw_data_def": {
                        "compu_method_ref": "Dcm_SecLevelType",
                        "data_constr_ref": "Dcm_SecLevelType_Constr",
                        "IsType": False,
                        "implementation_data_type": "uint8",
                    }
                },
                {
                    "short_name": "Dcm_SesCtrlType",  # 8.8.1.4
                    "category": "Type",
                    "sw_data_def": {
                        "compu_method_ref": "Dcm_SesCtrlType",
                        "data_constr_ref": "Dcm_SesCtrlType_Constr",
                        "IsType": False,
                        "implementation_data_type": "uint8",
                    }
                },
                {
                    "short_name": "Dcm_ProtocolType",  # 8.8.1.5
                    "category": "Type",
                    "sw_data_def": {
                        "compu_method_ref": "Dcm_ProtocolType",
                        "data_constr_ref": "Dcm_ProtocolType_Constr",
                        "IsType": False,
                        "implementation_data_type": "uint8",
                    }
                },
                {
                    "short_name": "Dcm_NegativeResponseCodeType",  # 8.8.1.6
                    "category": "Type",
                    "sw_data_def": {
                        "compu_method_ref": "Dcm_NegativeResponseCodeType",
                        "data_constr_ref": "Dcm_NegativeResponseCodeType_Constr",
                        "IsType": False,
                        "implementation_data_type": "uint8",
                    }
                },
                {
                    "short_name": "Dcm_IOOperationResponseType",  # 8.8.1.47
                    "category": "Type",
                    "sw_data_def": {
                        "compu_method_ref": "Dcm_IOOperationResponseType",
                        "data_constr_ref": "Dcm_IOOperationResponseType_Constr",
                        "IsType": False,
                        "implementation_data_type": "uint8",
                    }
                },
                {
                    "short_name": "Dcm_DidSupportedType",  # 8.8.1.48
                    "category": "Type",
                    "sw_data_def": {
                        "compu_method_ref": "Dcm_DidSupportedType",
                        "data_constr_ref": "Dcm_DidSupportedType_Constr",
                        "IsType": False,
                        "implementation_data_type": "uint8",
                    }
                }
            ],
            "data_constrs": {
                "elements": [{
                    "short_name": "Dcm_OpStatusType_Constr",
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": 0,
                            "upper_limit": 3,
                        }
                    }]
                }, {
                    "short_name": "Dcm_ConfirmationStatusType_Constr",
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": 0,
                            "upper_limit": 3,
                        }
                    }]
                }, {
                    "short_name": "Dcm_SecLevelType_Constr",
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": 0,
                            "upper_limit": 63,
                        }
                    }]
                }, {
                    "short_name": "Dcm_SesCtrlType_Constr",
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": 0,
                            "upper_limit": 127,
                        }
                    }]
                }, {
                    "short_name": "Dcm_ProtocolType_Constr",
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": 0,
                            "upper_limit": 255,
                        }
                    }]
                }, {
                    "short_name": "Dcm_NegativeResponseCodeType_Constr",
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": 0,
                            "upper_limit": 255,
                        }
                    }]
                }, {
                    "short_name": "Dcm_IOOperationResponseType_Constr",
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": 0,
                            "upper_limit": 255,
                        }
                    }]
                }, {
                    "short_name": "Dcm_DidSupportedType_Constr",
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": 0,
                            "upper_limit": 255,
                        }
                    }]
                }],
            },
            "compu_methods": {
                "elements": [
                    {
                        "short_name":
                            "Dcm_OpStatusType",
                        "category":
                            "TEXTTABLE",
                        "compu_scales": [{
                            "short_label": "DCM_INITIAL",
                            "lower_limit": 0,
                            "upper_limit": 0,
                        }, {
                            "short_label": "DCM_PENDING",
                            "lower_limit": 1,
                            "upper_limit": 1,
                        }, {
                            "short_label": "DCM_CANCEL",
                            "lower_limit": 2,
                            "upper_limit": 2,
                        }, {
                            "short_label": "DCM_FORCE_RCRRP_OK",
                            "lower_limit": 3,
                            "upper_limit": 3,
                        }]
                    },
                    {
                        "short_name":
                            "Dcm_ConfirmationStatusType",
                        "category":
                            "TEXTTABLE",
                        "compu_scales": [{
                            "short_label": "DCM_RES_POS_OK",
                            "lower_limit": 0,
                            "upper_limit": 0,
                        }, {
                            "short_label": "DCM_RES_POS_NOT_OK",
                            "lower_limit": 1,
                            "upper_limit": 1,
                        }, {
                            "short_label": "DCM_RES_NEG_OK",
                            "lower_limit": 2,
                            "upper_limit": 2,
                        }, {
                            "short_label": "DCM_RES_NEG_NOT_OK",
                            "lower_limit": 3,
                            "upper_limit": 3,
                        }]
                    },
                    {
                        "short_name":
                            "Dcm_SecLevelType",
                        "category":
                            "TEXTTABLE",
                        "compu_scales": [{
                            "short_label": "DCM_SEC_LEV_LOCKED",
                            "lower_limit": 0,
                            "upper_limit": 0,
                        }, {
                            "short_label": "DCM_SEC_LEV_L1",
                            "lower_limit": 1,
                            "upper_limit": 1,
                        }, {
                            "short_label": "DCM_SEC_LEV_L2",
                            "lower_limit": 2,
                            "upper_limit": 2,
                        }, {
                            "short_label": "DCM_SEC_LEV_L3",
                            "lower_limit": 3,
                            "upper_limit": 3,
                        }, {
                            "short_label": "DCM_SEC_LEV_L4",
                            "lower_limit": 4,
                            "upper_limit": 4,
                        }, {
                            "short_label": "DCM_SEC_LEV_L5",
                            "lower_limit": 5,
                            "upper_limit": 5,
                        }, {
                            "short_label": "DCM_SEC_LEV_L6",
                            "lower_limit": 6,
                            "upper_limit": 6,
                        }, {
                            "short_label": "DCM_SEC_LEV_L7",
                            "lower_limit": 7,
                            "upper_limit": 7,
                        }, {
                            "short_label": "DCM_SEC_LEV_L8",
                            "lower_limit": 8,
                            "upper_limit": 8,
                        }, {
                            "short_label": "DCM_SEC_LEV_L9",
                            "lower_limit": 9,
                            "upper_limit": 9,
                        }]
                    },
                    {
                        "short_name":
                            "Dcm_SesCtrlType",
                        "category":
                            "TEXTTABLE",
                        "compu_scales": [{
                            "short_label": "DCM_DEFAULT_SESSION",
                            "lower_limit": 1,
                            "upper_limit": 1,
                        }, {
                            "short_label": "DCM_PROGRAMMING_SESSION",
                            "lower_limit": 2,
                            "upper_limit": 2,
                        }, {
                            "short_label": "DCM_EXTENDED_DIAGNOSTIC_SESSION",
                            "lower_limit": 3,
                            "upper_limit": 3,
                        }, {
                            "short_label": "DCM_SAFETY_SYSTEM_DIAGNOSTIC_SESSION",
                            "lower_limit": 4,
                            "upper_limit": 4,
                        }]
                    },
                    {
                        "short_name":
                            "Dcm_ProtocolType",
                        "category":
                            "TEXTTABLE",
                        "compu_scales": [{
                            "short_label": "DCM_OBD_ON_CAN",
                            "lower_limit": 0,
                            "upper_limit": 0,
                        }, {
                            "short_label": "DCM_OBD_ON_FLEXRAY",
                            "lower_limit": 1,
                            "upper_limit": 1,
                        }, {
                            "short_label": "DCM_OBD_ON_IP",
                            "lower_limit": 2,
                            "upper_limit": 2,
                        }, {
                            "short_label": "DCM_UDS_ON_CAN",
                            "lower_limit": 3,
                            "upper_limit": 3,
                        }, {
                            "short_label": "DCM_UDS_ON_FLEXRAY",
                            "lower_limit": 4,
                            "upper_limit": 4,
                        }, {
                            "short_label": "DCM_UDS_ON_IP",
                            "lower_limit": 5,
                            "upper_limit": 5,
                        }, {
                            "short_label": "DCM_ROE_ON_CAN",
                            "lower_limit": 6,
                            "upper_limit": 6,
                        }, {
                            "short_label": "DCM_ROE_ON_FLEXRAY",
                            "lower_limit": 7,
                            "upper_limit": 7,
                        }, {
                            "short_label": "DCM_ROE_ON_IP",
                            "lower_limit": 8,
                            "upper_limit": 8,
                        }, {
                            "short_label": "DCM_PERIODICTRANS_ON_CAN",
                            "lower_limit": 9,
                            "upper_limit": 9,
                        }, {
                            "short_label": "DCM_PERIODICTRANS_ON_FLEXRAY",
                            "lower_limit": 10,
                            "upper_limit": 10,
                        }, {
                            "short_label": "DCM_PERIODICTRANS_ON_IP",
                            "lower_limit": 11,
                            "upper_limit": 11,
                        }, {
                            "short_label": "DCM_NO_ACTIVE_PROTOCOL",
                            "lower_limit": 12,
                            "upper_limit": 12,
                        }, {
                            "short_label": "DCM_UDS_ON_LIN",
                            "lower_limit": 13,
                            "upper_limit": 13,
                        }]
                    },
                    {
                        "short_name":
                            "Dcm_NegativeResponseCodeType",
                        "category":
                            "TEXTTABLE",
                        "compu_scales": [{
                            "short_label": "DCM_POS_RESP",
                            "lower_limit": 0,
                            "upper_limit": 0,
                        }, {
                            "short_label": "DCM_E_SERVICENOTSUPPORTED",
                            "lower_limit": 0x11,
                            "upper_limit": 0x11,
                        }, {
                            "short_label": "DCM_E_SUBFUNCTIONNOTSUPPORTED",
                            "lower_limit": 0x12,
                            "upper_limit": 0x12,
                        }, {
                            "short_label": "DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT",
                            "lower_limit": 0x13,
                            "upper_limit": 0x13,
                        }, {
                            "short_label": "DCM_E_RESPONSETOOLONG",
                            "lower_limit": 0x14,
                            "upper_limit": 0x14,
                        }, {
                            "short_label": "DCM_E_BUSYREPEATREQUEST",
                            "lower_limit": 0x21,
                            "upper_limit": 0x21,
                        }, {
                            "short_label": "DCM_E_CONDITIONSNOTCORRECT",
                            "lower_limit": 0x22,
                            "upper_limit": 0x22,
                        }, {
                            "short_label": "DCM_E_REQUESTSEQUENCEERROR",
                            "lower_limit": 0x24,
                            "upper_limit": 0x24,
                        }, {
                            "short_label": "DCM_E_NORESPONSEFROMSUBNETCOMPONENT",
                            "lower_limit": 0x25,
                            "upper_limit": 0x25,
                        }, {
                            "short_label": "DCM_E_FAILUREPREVENTSEXECUTIONOFREQUESTEDACTION",
                            "lower_limit": 0x26,
                            "upper_limit": 0x26,
                        }, {
                            "short_label": "DCM_E_REQUESTOUTOFRANGE",
                            "lower_limit": 0x31,
                            "upper_limit": 0x31,
                        }, {
                            "short_label": "DCM_E_SECURITYACCESSDENIED",
                            "lower_limit": 0x33,
                            "upper_limit": 0x33,
                        }, {
                            "short_label": "DCM_E_INVALIDKEY",
                            "lower_limit": 0x35,
                            "upper_limit": 0x35,
                        }, {
                            "short_label": "DCM_E_EXCEEDNUMBEROFATTEMPTS",
                            "lower_limit": 0x36,
                            "upper_limit": 0x36,
                        }, {
                            "short_label": "DCM_E_REQUIREDTIMEDELAYNOTEXPIRED",
                            "lower_limit": 0x37,
                            "upper_limit": 0x37,
                        }, {
                            "short_label": "DCM_E_UPLOADDOWNLOADNOTACCEPTED",
                            "lower_limit": 0x70,
                            "upper_limit": 0x70,
                        }, {
                            "short_label": "DCM_E_TRANSFERDATASUSPENDED",
                            "lower_limit": 0x71,
                            "upper_limit": 0x71,
                        }, {
                            "short_label": "DCM_E_GENERALPROGRAMMINGFAILURE",
                            "lower_limit": 0x72,
                            "upper_limit": 0x72,
                        }, {
                            "short_label": "DCM_E_WRONGBLOCKSEQUENCECOUNTER",
                            "lower_limit": 0x73,
                            "upper_limit": 0x73,
                        }, {
                            "short_label": "DCM_E_SUBFUNCTIONNOTSUPPORTEDINACTIVESESSION",
                            "lower_limit": 0x7e,
                            "upper_limit": 0x7e,
                        }, {
                            "short_label": "DCM_E_SERVICENOTSUPPORTEDINACTIVESESSION",
                            "lower_limit": 0x7f,
                            "upper_limit": 0x7f,
                        }, {
                            "short_label": "DCM_E_RPMTOOHIGH",
                            "lower_limit": 0x81,
                            "upper_limit": 0x81,
                        }, {
                            "short_label": "DCM_E_RPMTOOLOW",
                            "lower_limit": 0x82,
                            "upper_limit": 0x82,
                        }, {
                            "short_label": "DCM_E_ENGINEISRUNNING",
                            "lower_limit": 0x83,
                            "upper_limit": 0x83,
                        }, {
                            "short_label": "DCM_E_ENGINEISNOTRUNNING",
                            "lower_limit": 0x84,
                            "upper_limit": 0x84,
                        }, {
                            "short_label": "DCM_E_ENGINERUNTIMETOOLOW",
                            "lower_limit": 0x85,
                            "upper_limit": 0x85,
                        }, {
                            "short_label": "DCM_E_TEMPERATURETOOHIGH",
                            "lower_limit": 0x86,
                            "upper_limit": 0x86,
                        }, {
                            "short_label": "DCM_E_TEMPERATURETOOLOW",
                            "lower_limit": 0x87,
                            "upper_limit": 0x87,
                        }, {
                            "short_label": "DCM_E_VEHICLESPEEDTOOHIGH",
                            "lower_limit": 0x88,
                            "upper_limit": 0x88,
                        }, {
                            "short_label": "DCM_E_VEHICLESPEEDTOOLOW",
                            "lower_limit": 0x89,
                            "upper_limit": 0x89,
                        }, {
                            "short_label": "DCM_E_THROTTLE_PEDALTOOHIGH",
                            "lower_limit": 0x8a,
                            "upper_limit": 0x8a,
                        }, {
                            "short_label": "DCM_E_THROTTLE_PEDALTOOLOW",
                            "lower_limit": 0x8b,
                            "upper_limit": 0x8b,
                        }, {
                            "short_label": "DCM_E_TRANSMISSIONRANGENOTINNEUTRAL",
                            "lower_limit": 0x8c,
                            "upper_limit": 0x8c,
                        }, {
                            "short_label": "DCM_E_TRANSMISSIONRANGENOTINGEAR",
                            "lower_limit": 0x8d,
                            "upper_limit": 0x8d,
                        }, {
                            "short_label": "DCM_E_BRAKESWITCH_NOTCLOSED",
                            "lower_limit": 0x8f,
                            "upper_limit": 0x8f,
                        }, {
                            "short_label": "DCM_E_SHIFTERLEVERNOTINPARK",
                            "lower_limit": 0x90,
                            "upper_limit": 0x90,
                        }, {
                            "short_label": "DCM_E_TORQUECONVERTERCLUTCHLOCKED",
                            "lower_limit": 0x91,
                            "upper_limit": 0x91,
                        }, {
                            "short_label": "DCM_E_VOLTAGETOOHIGH",
                            "lower_limit": 0x92,
                            "upper_limit": 0x92,
                        }, {
                            "short_label": "DCM_E_VOLTAGETOOLOW",
                            "lower_limit": 0x93,
                            "upper_limit": 0x93,
                        }, {
                            "short_label": "DCM_E_VMSCNC_0",
                            "lower_limit": 0xf0,
                            "upper_limit": 0xf0,
                        }, {
                            "short_label": "DCM_E_VMSCNC_1",
                            "lower_limit": 0xf1,
                            "upper_limit": 0xf1,
                        }, {
                            "short_label": "DCM_E_VMSCNC_2",
                            "lower_limit": 0xf2,
                            "upper_limit": 0xf2,
                        }, {
                            "short_label": "DCM_E_VMSCNC_3",
                            "lower_limit": 0xf3,
                            "upper_limit": 0xf3,
                        }, {
                            "short_label": "DCM_E_VMSCNC_4",
                            "lower_limit": 0xf4,
                            "upper_limit": 0xf4,
                        }, {
                            "short_label": "DCM_E_VMSCNC_5",
                            "lower_limit": 0xf5,
                            "upper_limit": 0xf5,
                        }, {
                            "short_label": "DCM_E_VMSCNC_6",
                            "lower_limit": 0xf6,
                            "upper_limit": 0xf6,
                        }, {
                            "short_label": "DCM_E_VMSCNC_7",
                            "lower_limit": 0xf7,
                            "upper_limit": 0xf7,
                        }, {
                            "short_label": "DCM_E_VMSCNC_8",
                            "lower_limit": 0xf8,
                            "upper_limit": 0xf8,
                        }, {
                            "short_label": "DCM_E_VMSCNC_9",
                            "lower_limit": 0xf9,
                            "upper_limit": 0xf9,
                        }, {
                            "short_label": "DCM_E_VMSCNC_A",
                            "lower_limit": 0xfa,
                            "upper_limit": 0xfa,
                        }, {
                            "short_label": "DCM_E_VMSCNC_B",
                            "lower_limit": 0xfb,
                            "upper_limit": 0xfb,
                        }, {
                            "short_label": "DCM_E_VMSCNC_C",
                            "lower_limit": 0xfc,
                            "upper_limit": 0xfc,
                        }, {
                            "short_label": "DCM_E_VMSCNC_D",
                            "lower_limit": 0xfd,
                            "upper_limit": 0xfd,
                        }, {
                            "short_label": "DCM_E_VMSCNC_E",
                            "lower_limit": 0xfe,
                            "upper_limit": 0xfe,
                        }]
                    },
                    {
                        "short_name":
                            "Dcm_IOOperationResponseType",
                        "category":
                            "TEXTTABLE",
                        "compu_scales": [{
                            "short_label": "DCM_POSITIVE_RESPONSE",
                            "lower_limit": 0,
                            "upper_limit": 0,
                        }, {
                            "short_label": "DCM_GENERAL_REJECT",
                            "lower_limit": 0x10,
                            "upper_limit": 0x10,
                        }, {
                            "short_label": "DCM_BUSY_REPEAT_REQUEST",
                            "lower_limit": 0x21,
                            "upper_limit": 0x21,
                        }, {
                            "short_label": "DCM_CONDITIONS_NOT_CORRECT",
                            "lower_limit": 0x22,
                            "upper_limit": 0x22,
                        }, {
                            "short_label": "DCM_FAILURE_PREVENTS_EXECUTION",
                            "lower_limit": 0x26,
                            "upper_limit": 0x26,
                        }, {
                            "short_label": "DCM_REQUEST_OUT_OF_RANGE",
                            "lower_limit": 0x31,
                            "upper_limit": 0x31,
                        }, {
                            "short_label": "DCM_RESPONSE_PENDING",
                            "lower_limit": 0x78,
                            "upper_limit": 0x78,
                        }]
                    },
                    {
                        "short_name":
                            "Dcm_DidSupportedType",
                        "category":
                            "TEXTTABLE",
                        "compu_scales": [{
                            "short_label": "DCM_DID_SUPPORTED",
                            "lower_limit": 0,
                            "upper_limit": 0,
                        }, {
                            "short_label": "DCM_DID_NOT_SUPPORTED",
                            "lower_limit": 1,
                            "upper_limit": 1,
                        }]
                    },
                ],
            }
        }
        self.mode_switch_interfaces = [
            {
                "short_name": "Dcm_DiagnosticSessionControlModeSwitchInterface",  # 8.8.7.1
                "is_server": True,
                "mode_group": "DcmDiagnosticSessionControl"
            },
            {
                "short_name": "Dcm_EcuResetModeSwitchInterface",  # 8.8.7.2
                "is_server": True,
                "mode_group": "DcmEcuReset"
            },
            {
                "short_name": "Dcm_ModeRapidPowerShutDownModeSwitchInterface",  # 8.8.7.3
                "is_server": True,
                "mode_group": "DcmModeRapidPowerShutDown"
            },
            {
                "short_name": "Dcm_ControlDTCSettingModeSwitchInterface",  # 8.8.7.5
                "is_server": True,
                "mode_group": "DcmControlDTCSetting"
            },
            {
                "short_name": "Dcm_SecurityAccessModeSwitchInterface",  # 8.8.7.7
                "is_server": True,
                "mode_group": "DcmSecurityAccess"
            },
            {
                "short_name": "Dcm_AuthenticationStateModeSwitchInterface",  # 8.8.7.8
                "is_server": True,
                "mode_group": "DcmAuthenticationState"
            }
        ]
        self.client_server_interfaces = [
            {
                "short_name":
                    "CallbackDCMRequestServices",  # 8.8.3.7
                "is_server":
                    True,
                "is_synch":
                    True,
                "port_type":
                    "rport",
                "possible_errors": [{
                    "short_name": "E_OK",
                    "error_code": 0
                }, {
                    "short_name": "E_NOT_OK",
                    "error_code": 1
                }, {
                    "short_name": "DCM_E_PROTOCOL_NOT_ALLOWED",
                    "error_code": 5
                }],
                "operations": [
                    {
                        "short_name": "StartProtocol",
                        "arguments": [
                            {
                                "short_name": "ProtocolType",
                                "IsType": True,
                                "TypeTref": "Dcm_ProtocolType",
                                "Direction": "IN",
                            },
                            {
                                "short_name": "TesterSourceAddress",
                                "IsType": False,
                                "TypeTref": "uint16",
                                "Direction": "IN",
                            },
                            {
                                "short_name": "ConnectionId",
                                "IsType": False,
                                "TypeTref": "uint16",
                                "Direction": "IN",
                            },
                        ],
                        "possible_errors": ["E_OK", "E_NOT_OK", "DCM_E_PROTOCOL_NOT_ALLOWED"]
                    },
                    {
                        "short_name": "StopProtocol",
                        "arguments": [
                            {
                                "short_name": "ProtocolType",
                                "IsType": True,
                                "TypeTref": "Dcm_ProtocolType",
                                "Direction": "IN",
                            },
                            {
                                "short_name": "TesterSourceAddress",
                                "IsType": False,
                                "TypeTref": "uint16",
                                "Direction": "IN",
                            },
                            {
                                "short_name": "ConnectionId",
                                "IsType": False,
                                "TypeTref": "uint16",
                                "Direction": "IN",
                            },
                        ],
                        "possible_errors": ["E_OK", "E_NOT_OK"]
                    },
                ]
            },
            {
                "short_name":
                    "DCMServices",  # 8.8.3.11
                "is_server":
                    True,
                "is_synch":
                    True,
                "port_type":
                    "pport",
                "possible_errors": [{
                    "short_name": "E_OK",
                    "error_code": 0
                }, {
                    "short_name": "E_NOT_OK",
                    "error_code": 1
                }],
                "operations": [{
                    "short_name": "GetActiveProtocol",
                    "arguments": [{
                        "short_name": "ActiveProtocolType",
                        "IsType": True,
                        "TypeTref": "Dcm_ProtocolType",
                        "Direction": "OUT",
                    }, {
                        "short_name": "ConnectionId",
                        "IsType": False,
                        "TypeTref": "uint16",
                        "Direction": "OUT",
                    }, {
                        "short_name": "TesterSourceAddress",
                        "IsType": False,
                        "TypeTref": "uint16",
                        "Direction": "OUT",
                    }],
                    "possible_errors": ["E_OK"]
                }, {
                    "short_name": "GetSecurityLevel",
                    "arguments": [{
                        "short_name": "SecLevel",
                        "IsType": True,
                        "TypeTref": "Dcm_SecLevelType",
                        "Direction": "OUT",
                    }],
                    "possible_errors": ["E_OK", "E_NOT_OK"]
                }, {
                    "short_name": "GetSesCtrlType",
                    "arguments": [{
                        "short_name": "SesCtrlType",
                        "IsType": True,
                        "TypeTref": "Dcm_SesCtrlType",
                        "Direction": "OUT",
                    }],
                    "possible_errors": ["E_OK", "E_NOT_OK"]
                }, {
                    "short_name": "ResetToDefaultSession",
                    "arguments": [],
                    "possible_errors": ["E_OK", "E_NOT_OK"]
                }, {
                    "short_name": "SetActiveDiagnostic",
                    "arguments": [{
                        "short_name": "active",
                        "IsType": False,
                        "TypeTref": "boolean",
                        "Direction": "IN",
                    }],
                    "possible_errors": ["E_OK"]
                }]
            }
        ]
        self.sender_receive_interfaces = []
        self.pports = [
            {
                "short_name": "DCMServices",  # 8.8.5.8
                "interface_type": "client_server",
                "interface_ref": "DCMServices",
            },
            {
                "short_name": "DiagnosticSessionControlModeSwitchInterface",  # 8.8.5.16
                "interface_type": "mode_switch",
                "interface_ref": "Dcm_DiagnosticSessionControlModeSwitchInterface",
                "mode_group_ref": "DcmDiagnosticSessionControl"
            },
            {
                "short_name": "EcuResetModeSwitchInterface",  # 8.8.5.17
                "interface_type": "mode_switch",
                "interface_ref": "Dcm_EcuResetModeSwitchInterface",
                "mode_group_ref": "DcmEcuReset"
            },
            {
                "short_name": "ModeRapidPowerShutDownModeSwitchInterface",  # 8.8.5.18
                "interface_type": "mode_switch",
                "interface_ref": "Dcm_ModeRapidPowerShutDownModeSwitchInterface",
                "mode_group_ref": "DcmModeRapidPowerShutDown"
            },
            {
                "short_name": "ControlDTCSettingModeSwitchInterface",  # 8.8.5.20
                "interface_type": "mode_switch",
                "interface_ref": "Dcm_ControlDTCSettingModeSwitchInterface",
                "mode_group_ref": "DcmControlDTCSetting"
            },
            {
                "short_name": "SecurityAccessModeSwitchInterface",  # 8.8.5.22
                "interface_type": "mode_switch",
                "interface_ref": "Dcm_SecurityAccessModeSwitchInterface",
                "mode_group_ref": "DcmSecurityAccess"
            }
        ]
        self.rports = []
        self.main_function_runnable = {
            "period": self.ecucs_autosar_dict["dcm_DcmGeneral"]["DcmTaskTime"]["Value"],
            "mode_switch_points": [],
            "server_call_points": [],
            "data_send_points": [],
            "data_receive_points": []
        }
        self.server_runnables = []
        self.operation_invoked_events = []

    def update_variants(self):
        self.variant_security_access()  # SecurityAccess
        self.variant_data_services_data()  # DataServices_{Data}
        self.variant_routine_services()  # RoutineServices
        self.variant_callback_dcm_request_services()  # Dcm_CallbackDCMRequestServices
        self.variant_service_request_notification()  # ServiceRequestNotification
        self.variant_upload_download_services()  # UploadDownloadServices
        self.variant_communication_control()  # CommunicationControlModeSwitchInterface
        self.variant_infotype_services()  # InfotypeServices_{VehInfoData}

    def variant_security_access(self):
        dcm_DcmDspSecurityRow = self.ecucs_autosar_dict["dcm_DcmDspSecurityRow"]
        for key, value in dcm_DcmDspSecurityRow.items():
            security_access = SecurityAccess(key, value)
            self.client_server_interfaces_append(security_access.get_interface_dict())
            self.rports_append(security_access.get_rport())
            self.data_types_append(security_access.get_data_types())
            self.security_access_add_mode_declaration_group(key)  #8.8.6.7

    def variant_data_services_data(self):
        dcm_DcmDspDataInfos = self.ecucs_autosar_dict["dcm_DcmDspDataInfos"]
        dcm_DcmDspDatas = self.ecucs_autosar_dict["dcm_DcmDspDatas"]
        dcm_DcmDspDidInfos = self.ecucs_autosar_dict["dcm_DcmDspDidInfos"]
        dcm_DcmDspDids = self.ecucs_autosar_dict["dcm_DcmDspDids"]
        #只有遍历所有的Did 才能找到每个DspData 对应的DataInfo 和 DidInfo
        dsp_data_list = []
        for did_short_name, did_value in dcm_DcmDspDids.items():
            did_info_ref = did_value["DcmDspDidInfoRef"]["ValueRef"].split("/")[-1]
            did_info_ref_dict = get_value_from_dict_by_key(dcm_DcmDspDidInfos, did_info_ref)
            #USE_DATA_ELEMENT_SPECIFIC_INTERFACES   生成 DataServices_{Data}
            if did_value["DcmDspDidUsePort"]["Value"] == "USE_DATA_ELEMENT_SPECIFIC_INTERFACES":
                for did_value_key, did_value_value in did_value.items():
                    if isinstance(did_value_value, dict):
                        if "DefinitionRef" in did_value_value.keys():
                            if did_value_value["DefinitionRef"].split("/")[-1] == "DcmDspDidSignal":
                                dsp_data_ref = did_value_value["DcmDspDidDataRef"]["ValueRef"].split("/")[
                                    -1]  #dsp_data_short_name
                                dsp_data_ref_dict = get_value_from_dict_by_key(dcm_DcmDspDatas, dsp_data_ref)
                                data_info_ref = dsp_data_ref_dict["DcmDspDataInfoRef"]["ValueRef"].split("/")[
                                    -1]  #dsp_data_info_short_name
                                data_info_ref_dict = get_value_from_dict_by_key(dcm_DcmDspDataInfos, data_info_ref)
                                # 过滤相同的 DcmDspDidDataRef
                                if dsp_data_ref not in dsp_data_list:
                                    dsp_data_list.append(dsp_data_ref)
                                    dataservices_data = DataServices_Data(dsp_data_ref, dsp_data_ref_dict,
                                                                          did_info_ref_dict, data_info_ref_dict)
                                    if dataservices_data.get_interface_dict()["interface_type"] == "client_server":
                                        self.client_server_interfaces_append(
                                            dataservices_data.get_interface_dict()["interface_dict"])
                                    if dataservices_data.get_port()["port_type"] == "rport":
                                        self.rports_append(dataservices_data.get_port()["port_dict"])
                                    if dataservices_data.get_port()["port_type"] == "pport":
                                        self.pports_append(dataservices_data.get_port()["port_dict"])
                                    self.data_types_append(dataservices_data.get_data_types())
        #Pid的ServiceMapping
        dcm_DcmDspPids = self.ecucs_autosar_dict["dcm_DcmDspPids"]
        for pid_short_name, pid_value in dcm_DcmDspPids.items():
            if pid_value["DcmDspPidService"]["Value"] == "DCM_SERVICE_02":
                continue
            for data_key, data_value in pid_value.items():
                if isinstance(data_value, dict):
                    if "DefinitionRef" in data_value.keys():
                        if data_value["DefinitionRef"].split("/")[-1] == "DcmDspPidData":
                            dataservices_piddata = DataServices_PidData(data_key, data_value)
                            if dataservices_piddata.get_interface_dict()["interface_type"] == "client_server":
                                self.client_server_interfaces_append(dataservices_piddata.get_interface_dict()["interface_dict"])
                            if dataservices_piddata.get_port()["port_type"] == "rport":
                                self.rports_append(dataservices_piddata.get_port()["port_dict"])
                            if dataservices_piddata.get_port()["port_type"] == "pport":
                                self.pports_append(dataservices_piddata.get_port()["port_dict"])
                            self.data_types_append(dataservices_piddata.get_data_types())

    def variant_routine_services(self):
        dcm_DcmDspRoutines = self.ecucs_autosar_dict["dcm_DcmDspRoutines"]
        for routine_short_name, routine_dict in dcm_DcmDspRoutines.items():
            if str(routine_dict["DcmDspRoutineUsePort"]["Value"]).lower() == "true":
                routine = RoutineServices(routine_short_name, routine_dict)
                self.client_server_interfaces_append(routine.get_interface())
                self.rports_append(routine.get_port())
                self.data_types_append(routine.get_data_types())

    def variant_callback_dcm_request_services(self):
        dcm_DcmDslCallbackDCMRequestServices = self.ecucs_autosar_dict["dcm_DcmDslCallbackDCMRequestServices"]
        for key, value in dcm_DcmDslCallbackDCMRequestServices.items():
            rport = {
                "short_name": "Dcm_CallbackDCMRequestServices_" + key,  # 8.8.5.1
                "interface_type": "client_server",
                "interface_ref": "CallbackDCMRequestServices"
            }
            self.rports_append(rport)

    def variant_service_request_notification(self):
        dcm_DcmDsdServiceRequestManufacturerNotification = self.ecucs_autosar_dict[
            "dcm_DcmDsdServiceRequestManufacturerNotification"]
        dcm_DcmDsdServiceRequestSupplierNotification = self.ecucs_autosar_dict[
            "dcm_DcmDsdServiceRequestSupplierNotification"]
        if not dcm_DcmDsdServiceRequestManufacturerNotification and not dcm_DcmDsdServiceRequestSupplierNotification:
            return
        interface = {
            "short_name":
                "ServiceRequestNotification",  # 8.8.3.8
            "is_server":
                True,
            "is_synch":
                True,
            "port_type":
                "rport",
            "possible_errors": [{
                "short_name": "E_OK",
                "error_code": 0
            }, {
                "short_name": "E_NOT_OK",
                "error_code": 1
            }, {
                "short_name": "E_REQUEST_NOT_ACCEPTED",
                "error_code": 8
            }],
            "operations": [{
                "short_name": "Confirmation",
                "arguments": [{
                    "short_name": "SID",
                    "IsType": False,
                    "TypeTref": "uint8",
                    "Direction": "IN",
                }, {
                    "short_name": "ReqType",
                    "IsType": False,
                    "TypeTref": "uint8",
                    "Direction": "IN",
                }, {
                    "short_name": "ConnectionId",
                    "IsType": False,
                    "TypeTref": "uint16",
                    "Direction": "IN",
                }, {
                    "short_name": "ConfirmationStatus",
                    "IsType": True,
                    "TypeTref": "Dcm_ConfirmationStatusType",
                    "Direction": "IN",
                }, {
                    "short_name": "ProtocolType",
                    "IsType": True,
                    "TypeTref": "Dcm_ProtocolType",
                    "Direction": "IN",
                }, {
                    "short_name": "TesterSourceAddress",
                    "IsType": False,
                    "TypeTref": "uint16",
                    "Direction": "IN",
                }],
                "possible_errors": ["E_OK", "E_NOT_OK"]
            }, {
                "short_name": "Indication",
                "arguments": [{
                    "short_name": "SID",
                    "IsType": False,
                    "TypeTref": "uint8",
                    "Direction": "IN",
                }, {
                    "short_name": "RequestData",
                    "IsType": True,
                    "TypeTref": "Dcm_RequestDataArrayType",
                    "Direction": "IN",
                }, {
                    "short_name": "DataSize",
                    "IsType": False,
                    "TypeTref": "uint16",
                    "Direction": "IN",
                }, {
                    "short_name": "ReqType",
                    "IsType": False,
                    "TypeTref": "uint8",
                    "Direction": "IN",
                }, {
                    "short_name": "ConnectionId",
                    "IsType": False,
                    "TypeTref": "uint16",
                    "Direction": "IN",
                }, {
                    "short_name": "ErrorCode",
                    "IsType": True,
                    "TypeTref": "Dcm_NegativeResponseCodeType",
                    "Direction": "OUT",
                }, {
                    "short_name": "ProtocolType",
                    "IsType": True,
                    "TypeTref": "Dcm_ProtocolType",
                    "Direction": "IN",
                }, {
                    "short_name": "TesterSourceAddress",
                    "IsType": False,
                    "TypeTref": "uint16",
                    "Direction": "IN",
                }],
                "possible_errors": ["E_OK", "E_NOT_OK", "E_REQUEST_NOT_ACCEPTED"]
            }]
        }
        self.client_server_interfaces_append(interface)
        if dcm_DcmDsdServiceRequestManufacturerNotification:
            for key, value in dcm_DcmDsdServiceRequestManufacturerNotification.items():
                rport = {
                    "short_name": "ServiceRequestManufacturerNotification_" + key,  # 8.8.5.12
                    "interface_type": "client_server",
                    "interface_ref": "ServiceRequestNotification"
                }
                self.rports.append(rport)
        if dcm_DcmDsdServiceRequestSupplierNotification:
            for key, value in dcm_DcmDsdServiceRequestSupplierNotification.items():
                rport = {
                    "short_name": "ServiceRequestSupplierNotification_" + key,  # 8.8.5.13
                    "interface_type": "client_server",
                    "interface_ref": "ServiceRequestNotification"
                }
                self.rports.append(rport)

    def variant_upload_download_services(self):
        dcm_DcmDspMemoryTransfer = self.ecucs_autosar_dict["dcm_DcmDspMemoryTransfer"]
        if dcm_DcmDspMemoryTransfer:
            if str(dcm_DcmDspMemoryTransfer["DcmDspMemoryTransferUsePort"]["Value"]).lower() == "true":
                interface = {
                    "short_name":
                        "UploadDownloadServices",  # 8.8.3.9
                    "is_server":
                        True,
                    "is_synch":
                        True,
                    "port_type":
                        "rport",
                    "possible_errors": [{
                        "short_name": "E_OK",
                        "error_code": 0
                    }, {
                        "short_name": "E_NOT_OK",
                        "error_code": 1
                    }, {
                        "short_name": "DCM_E_PENDING",
                        "error_code": 10
                    }, {
                        "short_name": "DCM_E_FORCE_RCRRP",
                        "error_code": 12
                    }],
                    "operations": [
                        {
                            "short_name": "ProcessRequestDownload",
                            "arguments": [{
                                "short_name": "OpStatus",
                                "IsType": True,
                                "TypeTref": "Dcm_OpStatusType",
                                "Direction": "IN",
                            }, {
                                "short_name": "DataFormatIdentifier",
                                "IsType": False,
                                "TypeTref": "uint8",
                                "Direction": "IN",
                            }, {
                                "short_name": "MemoryIdentifier",
                                "IsType": False,
                                "TypeTref": "uint16",
                                "Direction": "IN",
                            }, {
                                "short_name": "MemoryAddress",
                                "IsType": False,
                                "TypeTref": "uint32",
                                "Direction": "IN",
                            }, {
                                "short_name": "MemorySize",
                                "IsType": False,
                                "TypeTref": "uint32",
                                "Direction": "IN",
                            }, {
                                "short_name": "BlockLength",
                                "IsType": False,
                                "TypeTref": "uint32",
                                "Direction": "INOUT",
                            }, {
                                "short_name": "error_code",
                                "IsType": True,
                                "TypeTref": "Dcm_NegativeResponseCodeType",
                                "Direction": "OUT",
                            }],
                            "possible_errors": ["E_OK", "E_NOT_OK", "DCM_E_PENDING"]
                        },
                        {
                            "short_name": "ProcessRequestTransferExit",
                            "arguments": [{
                                "short_name": "OpStatus",
                                "IsType": True,
                                "TypeTref": "Dcm_OpStatusType",
                                "Direction": "IN",
                            }, {
                                "short_name": "error_code",
                                "IsType": True,
                                "TypeTref": "Dcm_NegativeResponseCodeType",
                                "Direction": "OUT",
                            }],
                            "possible_errors": ["E_OK", "E_NOT_OK", "E_REQUEST_NOT_ACCEPTED"]
                        },
                        {
                            "short_name": "ProcessRequestUpload",
                            "arguments": [{
                                "short_name": "OpStatus",
                                "IsType": True,
                                "TypeTref": "Dcm_OpStatusType",
                                "Direction": "IN",
                            }, {
                                "short_name": "DataFormatIdentifier",
                                "IsType": False,
                                "TypeTref": "uint8",
                                "Direction": "IN",
                            }, {
                                "short_name": "MemoryIdentifier",
                                "IsType": False,
                                "TypeTref": "uint16",
                                "Direction": "IN",
                            }, {
                                "short_name": "MemoryAddress",
                                "IsType": False,
                                "TypeTref": "uint32",
                                "Direction": "IN",
                            }, {
                                "short_name": "MemorySize",
                                "IsType": False,
                                "TypeTref": "uint32",
                                "Direction": "IN",
                            }, {
                                "short_name": "BlockLength",
                                "IsType": False,
                                "TypeTref": "uint32",
                                "Direction": "INOUT",
                            }, {
                                "short_name": "error_code",
                                "IsType": True,
                                "TypeTref": "Dcm_NegativeResponseCodeType",
                                "Direction": "OUT",
                            }],
                            "possible_errors": ["E_OK", "E_NOT_OK", "DCM_E_PENDING"]
                        },
                        {
                            "short_name": "ProcessTransferDataRead",
                            "arguments": [{
                                "short_name": "OpStatus",
                                "IsType": True,
                                "TypeTref": "Dcm_OpStatusType",
                                "Direction": "IN",
                            }, {
                                "short_name": "MemoryIdentifier",
                                "IsType": False,
                                "TypeTref": "uint16",
                                "Direction": "IN",
                            }, {
                                "short_name": "MemoryAddress",
                                "IsType": False,
                                "TypeTref": "uint32",
                                "Direction": "IN",
                            }, {
                                "short_name": "MemorySize",
                                "IsType": False,
                                "TypeTref": "uint32",
                                "Direction": "IN",
                            }, {
                                "short_name": "MemoryData",
                                "IsType": True,
                                "TypeTref": "Dcm_RequestDataArrayType",
                                "Direction": "OUT",
                            }, {
                                "short_name": "error_code",
                                "IsType": True,
                                "TypeTref": "Dcm_NegativeResponseCodeType",
                                "Direction": "OUT",
                            }],
                            "possible_errors": ["E_OK", "E_NOT_OK", "DCM_E_PENDING", "DCM_E_FORCE_RCRRP"]
                        },
                        {
                            "short_name": "ProcessTransferDataWrite",
                            "arguments": [{
                                "short_name": "OpStatus",
                                "IsType": True,
                                "TypeTref": "Dcm_OpStatusType",
                                "Direction": "IN",
                            }, {
                                "short_name": "MemoryIdentifier",
                                "IsType": False,
                                "TypeTref": "uint16",
                                "Direction": "IN",
                            }, {
                                "short_name": "MemoryAddress",
                                "IsType": False,
                                "TypeTref": "uint32",
                                "Direction": "IN",
                            }, {
                                "short_name": "MemorySize",
                                "IsType": False,
                                "TypeTref": "uint32",
                                "Direction": "IN",
                            }, {
                                "short_name": "MemoryData",
                                "IsType": True,
                                "TypeTref": "Dcm_RequestDataArrayType",
                                "Direction": "IN",
                            }, {
                                "short_name": "error_code",
                                "IsType": True,
                                "TypeTref": "Dcm_NegativeResponseCodeType",
                                "Direction": "OUT",
                            }],
                            "possible_errors": ["E_OK", "E_NOT_OK", "DCM_E_PENDING", "DCM_E_FORCE_RCRRP"]
                        },
                    ]
                }
                self.client_server_interfaces_append(interface)
                rport = {
                    "short_name": "UploadDownloadServices",  # 8.8.5.23
                    "interface_type": "client_server",
                    "interface_ref": "UploadDownloadServices"
                }
                self.rports.append(rport)

    def variant_communication_control(self):
        dcm_DcmDspComControl = self.ecucs_autosar_dict["dcm_DcmDspComControl"]
        for key, value in dcm_DcmDspComControl.items():
            if key != "DefinitionRef":
                if str(value["DefinitionRef"]).split("/")[-1] == "DcmDspComControlAllChannel":
                    channel_name = str(value["DcmDspAllComMChannelRef"]["ValueRef"]).split("/")[-1]
                    mode_switch_interface = {
                        "short_name":
                            update_short_name_length("Dcm_CommunicationControlModeSwitchInterface_" +
                                                     channel_name),  # 8.8.7.4
                        "is_server":
                            True,
                        "mode_group":
                            "DcmCommunicationControl"
                    }
                    self.mode_switch_interfaces_append(mode_switch_interface)
                    pport = {
                        "short_name": "CommunicationControlModeSwitchInterface_" + channel_name,  # 8.8.5.19
                        "interface_type": "mode_switch",
                        "interface_ref": "Dcm_CommunicationControlModeSwitchInterface_" + channel_name,
                        "mode_group_ref": "DcmCommunicationControl"
                    }
                    self.pports_append(pport)

    def variant_infotype_services(self):
        veh_info_datas = self.ecucs_autosar_dict["dcm_DcmDspVehInfoDatas"]
        for data_key, data_value in veh_info_datas.items():
            if data_value["DcmDspVehInfoDataUsePort"]["Value"] == False:
                continue

            # InfotypeServices_{VehInfoData}
            interface = {
                "short_name": "InfotypeServices_" + data_key,  # 8.8.3.4
                "is_server": True,
                "is_synch": True,
                "port_type": "rport",
                "possible_errors": [{
                    "short_name": "E_OK",
                    "error_code": 0
                }, {
                    "short_name": "E_NOT_OK",
                    "error_code": 1
                }, {
                    "short_name": "DCM_E_PENDING",
                    "error_code": 10
                }],
                "operations": [{
                    "short_name": "GetInfotypeValueData",
                    "arguments": [{
                        "short_name": "OpStatus",
                        "IsType": True,
                        "TypeTref": "Dcm_OpStatusType",
                        "Direction": "IN",
                    }, {
                        "short_name": "DataValueBuffer",
                        "IsType": True,
                        "TypeTref": "Dcm_InfoTypeServicesArray_" + data_key + "Type",
                        "Direction": "OUT",
                    }, {
                        "short_name": "DataValueBufferSize",
                        "IsType": False,
                        "TypeTref": "uint8",
                        "Direction": "OUT",
                    }],
                    "possible_errors": ["E_OK", "E_NOT_OK", "DCM_E_PENDING"]
                }]
            }
            self.client_server_interfaces_append(interface)

            # Dcm_InfoTypeServicesArray_{VehInfoData}Type
            self.data_types["elements"].append({
                "short_name": "Dcm_InfoTypeServicesArray_" + data_key + "Type",  # 8.8.1.11
                "category": "Array",
                "sw_data_def": {
                    "calibration_access": "READ-ONLY"
                },
                "sub_element": {
                    "array_size": str(int(data_value["DcmDspVehInfoDataSize"]["Value"])-1),
                    "element_type": "uint8"
                }
            })

            rport = {
                "short_name": "InfotypeServices_" + data_key,  # 8.8.5.9
                "interface_type": "client_server",
                "interface_ref": "InfotypeServices_" + data_key
            }
            self.rports.append(rport)

    def update_data_types(self):
        dcm_DcmDslBuffers = self.ecucs_autosar_dict["dcm_DcmDslBuffers"]
        max_dsl_buffer_size = 0
        for key, value in dcm_DcmDslBuffers.items():
            if value["DcmDslBufferSize"]["Value"] > max_dsl_buffer_size:
                max_dsl_buffer_size = value["DcmDslBufferSize"]["Value"]
        data_type_element = {
            "short_name": "Dcm_RequestDataArrayType",  # 8.8.1.42
            "category": "Array",
            "sw_data_def": {
                "calibration_access": "READ-ONLY"
            },
            "sub_element": {
                "array_size": max_dsl_buffer_size,
                "element_type": "uint8"
            }
        }
        self.data_types["elements"].append(data_type_element)

    def security_access_add_mode_declaration_group(self, input):
        for mode_declaration_group in self.mode_declaration_groups:
            if mode_declaration_group["short_name"] == "SecurityAccess":
                mode_declaration_group["modes"].append("SEC_LEV_" + str(input).upper())

    def update_mode_declaration_to_date(self):
        for mode_declaration_group in self.mode_declaration_groups:
            data_type_element = {}
            data_constrs_element = {}
            compu_method_element = {}
            data_type_element["short_name"] = "Dcm_" + str(mode_declaration_group["short_name"]) + "Type"
            data_type_element["category"] = "Type"
            data_type_element["sw_data_def"] = {
                "compu_method_ref": data_type_element["short_name"],
                "data_constr_ref": data_type_element["short_name"] + "_Constr",
                "IsType": False,
                "implementation_data_type": "uint8"
            }
            data_constrs_element["short_name"] = data_type_element["short_name"] + "_Constr"
            data_constrs_element["data_constr_rules"] = []
            data_constrs_element["data_constr_rules"].append(
                {"internal_consters": {
                    "lower_limit": 0,
                    "upper_limit": 255,
                }})
            compu_method_element["short_name"] = data_type_element["short_name"]
            compu_method_element["category"] = "TEXTTABLE"
            compu_method_element["compu_scales"] = []
            for index, mode in enumerate(mode_declaration_group["modes"]):
                limit = index
                if compu_method_element["short_name"] == "Dcm_DiagnosticSessionControlType":
                    limit = index + 1
                compu_method_element["compu_scales"].append({
                    "short_label": "DCM_ENUM_" + mode,
                    "lower_limit": limit,
                    "upper_limit": limit,
                })
            self.data_types["elements"].append(data_type_element)
            self.data_types["data_constrs"]["elements"].append(data_constrs_element)
            self.data_types["compu_methods"]["elements"].append(compu_method_element)

    def update_main_function_runnable(self):
        for rport in self.rports:
            if rport["interface_type"] == "client_server":  #server_call_points
                for cs_interface in self.client_server_interfaces:
                    if rport["interface_ref"] == cs_interface["short_name"]:
                        for operation in cs_interface["operations"]:
                            self.main_function_runnable["server_call_points"].append({
                                "short_name":
                                    update_short_name_length("OpServerCallDcm_MainFunction_" + operation["short_name"] +
                                                             "_" + rport["short_name"]),
                                "port_ref":
                                    rport["short_name"],
                                "is_synch":
                                    cs_interface["is_synch"],
                                "interface_ref":
                                    cs_interface["short_name"],
                                "operation_ref":
                                    operation["short_name"]
                            })

        for pport in self.pports:
            #mode-switch-points
            if pport["interface_type"] == "mode_switch":
                for sw_interface in self.mode_switch_interfaces:
                    if pport["interface_ref"] == sw_interface["short_name"]:
                        self.main_function_runnable["mode_switch_points"].append({
                            "short_name":
                                update_short_name_length("ModeSwitchPointDcm_MainFunction_" +
                                                         sw_interface["short_name"] + "_" + sw_interface["short_name"]),
                            "port_ref":
                                pport["short_name"],
                            "interface_ref":
                                sw_interface["short_name"],
                            "mode_group_ref":
                                sw_interface["mode_group"]
                        })

    def update_server_runnables(self):
        for cs_interface in self.client_server_interfaces:
            if cs_interface["port_type"] == "rport":
                continue
            for operation in cs_interface["operations"]:
                self.server_runnables.append({
                    "short_name": operation["short_name"],
                    "symbol": "Dcm_" + operation["short_name"]
                })

    def update_invoked_events(self):
        for pport in self.pports:
            for cs_interface in self.client_server_interfaces:
                if pport["interface_ref"] == cs_interface["short_name"]:
                    for operation in cs_interface["operations"]:
                        self.operation_invoked_events.append({
                            "short_name":
                                update_short_name_length("OpEvent" + operation["short_name"] + "_" +
                                                         operation["short_name"] + "_" + pport["short_name"]),
                            "start_on_event_ref":
                                operation["short_name"],
                            "pport_ref":
                                pport["short_name"],
                            "interface_ref":
                                cs_interface["short_name"],
                            "operation_ref":
                                operation["short_name"]
                        })

    def set_config_dict(self):
        self.config_module_dict = {
            "dcm_package": {
                "short_name": "Dcm_swc",
                "mode_declaration": self.mode_declaration_groups,
                "datatypes_pkg": self.data_types,
                "interfaces_pkg": {
                    "mode_switch": self.mode_switch_interfaces,
                    "client_server": self.client_server_interfaces
                },
                "componenttypes_pkg": {
                    "pports": self.pports,
                    "rports": self.rports,
                    "main_function": self.main_function_runnable,
                    "server_runnables": self.server_runnables,
                    "operation_events": self.operation_invoked_events
                }
            }
        }

    def get_config_dict(self):
        return self.config_module_dict

    def client_server_interfaces_append(self, input_dict):
        if input_dict:
            self.client_server_interfaces.append(input_dict)

    def mode_switch_interfaces_append(self, input_dict):
        if input_dict:
            self.mode_switch_interfaces.append(input_dict)

    def pports_append(self, input_dict):
        if input_dict:
            self.pports.append(input_dict)

    def rports_append(self, input_dict):
        if input_dict:
            self.rports.append(input_dict)

    def data_types_append(self, input_dict):
        if "elements" in input_dict:
            for element in input_dict["elements"]:
                self.data_types["elements"].append(element)
        if "data_constrs" in input_dict:
            for data_constr in input_dict["data_constrs"]:
                self.data_types["data_constrs"]["elements"].append(data_constr)
        if "compu_methods" in input_dict:
            for compu_method in input_dict["compu_methods"]:
                self.data_types["compu_methods"]["elements"].append(compu_method)


def get_config_dict(module_ecuc_cfg):
    dcm_module_cfg = module_ecuc_cfg["Dcm"] if "Dcm" in module_ecuc_cfg else None
    dem_module_cfg = module_ecuc_cfg["Dem"] if "Dem" in module_ecuc_cfg else None
    if not dcm_module_cfg or not dem_module_cfg:
        return {}
    dcm_ecuc_cfg_dict = AutosarUtil.to_bsw_dict(dcm_module_cfg)
    dcm_ecuc_dict = filter.get_template_input_parameter_dict(dcm_ecuc_cfg_dict)
    port_derive = DcmPortDerive(dcm_ecuc_dict)
    return port_derive.get_config_dict()
