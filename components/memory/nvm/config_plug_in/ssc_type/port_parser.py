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
NvM配置文件解析模块.
解析输入的NvM配置文件, 然后生成模板config.json
"""
import copy
from basic_func_module.autosar_utils.autosar_util import AutosarUtil
from . import filter
import hashlib



def update_short_name_length(short_name) -> str:
    if short_name and len(short_name) > 128:
        hash_obj = hashlib.sha1(short_name[120:].encode(encoding="utf-8"))
        hash_str = hash_obj.hexdigest()
        short_name = short_name[:120] + hash_str[:8]
    return short_name


class PortDerive(object):
    
    def __init__(self, ecucs_autosar_dict=None):
        self.ecucs_autosar_dict = ecucs_autosar_dict
        self.init_nvm_config()
        self.update_data_types()
        self.update_interfaces()
        self.update_pports()
        self.update_server_runnables()
        self.update_invoked_events()
        self.config_module_dict = {}
        self.set_config_dict()

    def init_nvm_config(self):
        self.data_types = {
            "elements": [
                {
                    "short_name": "nvm_request_result_type",  # [SWS_NvM_00841]
                    "category": "Type",
                    "sw_data_def": {
                        "data_constr_ref": "NvM_RequestResultType_Constr",
                        "IsType": False,
                        "implementation_data_type": "uint8",
                    }
                },
                {
                    "short_name": "nvm_block_id_t",  # SWS_NvM_00842
                    "category": "Type",
                    "sw_data_def": {
                        "data_constr_ref": "NvM_BlockIdType_Constr",
                        "IsType": False,
                        "implementation_data_type": "uint16",
                    }
                },
                {
                    "short_name": "NvM_InitBlockRequestType",  # [SWS_NvM_00841]
                    "category": "Type",
                    "sw_data_def": {
                        "data_constr_ref": "NvM_InitBlockRequestType_Constr",
                        "IsType": False,
                        "implementation_data_type": "uint8",
                    }
                },
                {
                    "short_name": "NvM_BlockRequestType",  # [SWS_NvM_00841]
                    "category": "Type",
                    "sw_data_def": {
                        "data_constr_ref": "NvM_BlockRequestType_Constr",
                        "IsType": False,
                        "implementation_data_type": "uint8",
                    }
                },
            ],
            "data_constrs": {
                "elements": [{
                    "short_name": "NvM_RequestResultType_Constr",
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": 0,
                            "upper_limit": 255,
                        }
                    }]
                },
                {
                    "short_name": "NvM_BlockIdType_Constr",
                    "data_constr_rules": []  
                },{
                    "short_name": "NvM_InitBlockRequestType_Constr",
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": 0,
                            "upper_limit": 255,
                        }
                    }]
                },{
                    "short_name": "NvM_BlockRequestType_Constr",
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": 0,
                            "upper_limit": 255,
                        }
                    }]
                }], 
            },
            "compu_methods": {
                "elements": [{
                    "short_name":
                        "nvm_request_result_type",
                    "category":
                        "TEXTTABLE",
                    "compu_scales": [{
                        "short_label": "NVM_REQ_OK",
                        "lower_limit": 0,
                        "upper_limit": 0,
                    }, {
                        "short_label": "NVM_REQ_NOT_OK",
                        "lower_limit": 1,
                        "upper_limit": 1,
                    }, {
                        "short_label": "NVM_REQ_PENDING",
                        "lower_limit": 2,
                        "upper_limit": 2,
                    }, {
                        "short_label": "NVM_REQ_INTEGRITY_FAILED",
                        "lower_limit": 3,
                        "upper_limit": 3,
                    }, {
                        "short_label": "NVM_REQ_BLOCK_SKIPPED",
                        "lower_limit": 4,
                        "upper_limit": 4,
                    }, {
                        "short_label": "NVM_REQ_NV_INVALIDATED",
                        "lower_limit": 5,
                        "upper_limit": 5,
                    }, {
                        "short_label": "NVM_REQ_CANCELED",
                        "lower_limit": 6,
                        "upper_limit": 6,
                    }, {
                        "short_label": "NVM_REQ_RESTORED_FROM_ROM",
                        "lower_limit": 8,
                        "upper_limit": 8,
                    }]
                },{
                    "short_name":
                        "NvM_InitBlockRequestType",
                    "category":
                        "TEXTTABLE",
                    "compu_scales": [{
                        "short_label": "NVM_INIT_READ_BLOCK",
                        "lower_limit": 0,
                        "upper_limit": 0,
                    }, {
                        "short_label": "NVM_INIT_RESTORE_BLOCK_DEFAULTS",
                        "lower_limit": 1,
                        "upper_limit": 1,
                    }, {
                        "short_label": "NVM_INIT_READ_ALL_BLOCK",
                        "lower_limit": 2,
                        "upper_limit": 2,
                    }, {
                        "short_label": "NVM_INIT_FIRST_INIT_ALL",
                        "lower_limit": 3,
                        "upper_limit": 3,
                    }]
                },{
                    "short_name":
                        "NvM_BlockRequestType",
                    "category":
                        "TEXTTABLE",
                    "compu_scales": [{
                        "short_label": "NVM_READ_BLOCK_SID",
                        "lower_limit": 0,
                        "upper_limit": 0,
                    }, {
                        "short_label": "NVM_WRITE_BLOCK",
                        "lower_limit": 1,
                        "upper_limit": 1,
                    }, {
                        "short_label": "NVM_RESTORE_BLOCK_DEFAULTS_SID",
                        "lower_limit": 2,
                        "upper_limit": 2,
                    }, {
                        "short_label": "NVM_ERASE_NV_BLOCK",
                        "lower_limit": 3,
                        "upper_limit": 3,
                    }, {
                        "short_label": "NVM_INVALIDATE_NV_BLOCK_SID",
                        "lower_limit": 4,
                        "upper_limit": 4,
                    }, {
                        "short_label": "NVM_READ_ALL_BLOCK",
                        "lower_limit": 5,
                        "upper_limit": 5,
                    }]
                }],
            }
        }
        self.mode_switch_interfaces = []
        self.sender_receiver_interfaces = []
        self.client_server_interfaces = [
            {
                "short_name":
                    "NvMAdmin",  # SWS_NvM_00738
                "is_server":
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
                "operations":[]
            },
            {
                "short_name":
                    "NvMMirror",  # SWS_NvM_00738
                "is_server":
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
                    "short_name": "ReadRamBlockFromNvM",
                    "arguments": [{
                        "short_name": "SrcBuffer",
                        "IsType": False,
                        "TypeTref": "dtRef_const_VOID",
                        "Direction": "IN",
                    }],
                    "possible_errors": ["E_OK", "E_NOT_OK"]
                }, {
                    "short_name": "WriteRamBlockToNvM",
                    "arguments": [{
                        "short_name": "DstBuffer",
                        "IsType": False,
                        "TypeTref": "dtRef_VOID",
                        "Direction": "IN",
                    }],
                    "possible_errors": ["E_OK", "E_NOT_OK"]
                }]
            },
            {
                "short_name":
                    "NvMNotifyInitBlock",  # SWS_NvM_00736
                "is_server":
                    True,
                "port_type":
                    "pport",
                "possible_errors": [{
                    "short_name": "E_OK",
                    "error_code": 0
                }],
                "operations": [{
                    "short_name": "InitBlock",
                    "arguments": [{
                        "short_name": "InitBlockRequest",
                        "IsType": True,
                        "TypeTref": "NvM_InitBlockRequestType",
                        "Direction": "IN",
                    }],
                    "possible_errors": ["E_OK"]
                }] 
            },
            {
                "short_name":
                    "NvMNotifyJobFinished",  # SWS_NvM_00735
                "is_server":
                    True,
                "port_type":
                    "pport",
                "possible_errors": [{
                    "short_name": "E_OK",
                    "error_code": 0
                }],
                "operations": [{ 
                    "short_name": "JobFinished",
                    "arguments": [{
                        "short_name": "BlockRequest",
                        "IsType": True,
                        "TypeTref": "NvM_BlockRequestType",
                        "Direction": "IN",
                    },{
                        "short_name": "job_result",
                        "IsType": True,
                        "TypeTref": "nvm_request_result_type",
                        "Direction": "IN",
                    }],
                    "possible_errors": ["E_OK"]
                }] 
            },
            {
                "short_name":
                    "NvMService",  # SWS_NvM_00735
                "is_server":
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
                    "short_name": "GetErrorStatus",
                    "arguments": [{
                        "short_name": "RequestResultPtr",
                        "IsType": True,
                        "TypeTref": "nvm_request_result_type",
                        "Direction": "OUT",
                    }],
                    "possible_errors": ["E_OK","E_NOT_OK"]
                }]
            },
        ]
        self.pports = []
        self.rports = []
        self.main_function_runnable = {
            "period": self.ecucs_autosar_dict["ActiveEcuC"]["NvM"]["NvMCommon"] ["NvMMainFunctionPeriod"]["Value"],
            "mode_switch_points": [],  # mode_switch p
            "server_call_points": [],  # cs r
            "data_send_points": [],  # sr p
            "data_receive_points": []  # sr r
        }
        self.server_runnables = []
        self.operation_invoked_events = []

    def update_data_types(self):
        nvMCommon = self.ecucs_autosar_dict["ActiveEcuC"]["NvM"]["NvMCommon"] 
        NvMDatasetSelectionBits = nvMCommon["NvMDatasetSelectionBits"]["Value"]
        value = 2^(16 - NvMDatasetSelectionBits) - 1 
        idtype_element = {
            "internal_consters": {
            "lower_limit": 0,
            "upper_limit": value,
           }
        }
        self.data_types["data_constrs"]["elements"][1]["data_constr_rules"].append(idtype_element)

    def count_ramstatus(self):
        nvm_dict = self.ecucs_autosar_dict['ActiveEcuC']['NvM']
        DataSetsNumber = 0
        for name, blockdescriptor in nvm_dict.items():
            if "DefinitionRef" not in blockdescriptor or blockdescriptor[
              "DefinitionRef"] != "/AUTOSAR/EcucDefs/NvM/NvMBlockDescriptor":
                continue
            blockstatus = blockdescriptor["NvMBlockUseSetRamBlockStatus"]["Value"]
            if blockstatus == True:
                DataSetsNumber += 1
        return DataSetsNumber
    
    def count_datasets(self):
        nvm_dict = self.ecucs_autosar_dict['ActiveEcuC']['NvM']
        DataSetsNumber = 0
        for name, blockdescriptor in nvm_dict.items():
            if "DefinitionRef" not in blockdescriptor or blockdescriptor[
              "DefinitionRef"] != "/AUTOSAR/EcucDefs/NvM/NvMBlockDescriptor":
                continue
            ManagementType = blockdescriptor["NvMBlockManagementType"]["Value"]
            if ManagementType == "NVM_BLOCK_DATASET":
                DataSetsNumber += 1
        return DataSetsNumber
    
    def block_list(self):
        nvm_dict = self.ecucs_autosar_dict['ActiveEcuC']['NvM']
        names = []
        for name, blockdescriptor in nvm_dict.items():
            if name == "NvMConfigBlock" or "DefinitionRef" not in blockdescriptor or blockdescriptor[
              "DefinitionRef"] != "/AUTOSAR/EcucDefs/NvM/NvMBlockDescriptor":
                continue
            blockid = blockdescriptor['NvMNvramBlockIdentifier']['Value']
            names.append((name, blockid))
        return names

    def update_interfaces(self):
        flex_element = self.ecucs_autosar_dict["ActiveEcuC"]["NvM"]["NvMCommon"]
        DataSetsNumber = self.count_datasets()
        RamBlockStatus = self.count_ramstatus()
        NvMApiConfig = flex_element["NvMApiConfigClass"]["Value"]

        if RamBlockStatus > 0:
            service_elment_0 = {
                "short_name": "SetRamBlockStatus",
                "arguments": [{            
                    "short_name": "BlockChanged",
                    "IsType": False,
                    "TypeTref": "boolean",
                    "Direction": "IN"
                    }],
                "possible_errors": ["E_OK", "E_NOT_OK"]
            }
            self.client_server_interfaces[4]["operations"].append(service_elment_0)

        if NvMApiConfig == "NVM_API_CONFIG_CLASS_3" or NvMApiConfig == "NVM_API_CONFIG_CLASS_2":
            service_elment_4 = {
                "short_name": "ReadBlock",
                "arguments": [{            
                    "short_name": "DstBuffer",
                    "IsType": False,
                    "TypeTref": "dtRef_VOID",
                    "Direction": "IN",
                    }],
                "possible_errors": ["E_OK", "E_NOT_OK"]
            }
            service_elment_5 = {
                "short_name": "ReadPRAMBlock",
                "arguments": [],
                "possible_errors": ["E_OK", "E_NOT_OK"]
            }
            service_elment_6 = {
                "short_name": "RestoreBlockDefaults",
                "arguments": [{            
                    "short_name": "DstBuffer",
                    "IsType": False,
                    "TypeTref": "dtRef_VOID",
                    "Direction": "IN",
                    }],
                "possible_errors": ["E_OK", "E_NOT_OK"]
            }
            service_elment_7 = {
                "short_name": "RestorePRAMBlockDefaults",
                "arguments": [],
                "possible_errors": ["E_OK", "E_NOT_OK"]
            }
            service_elment_10 = {
                "short_name": "WriteBlock",
                "arguments": [{            
                    "short_name": "SrcBuffer",
                    "IsType": False,
                    "TypeTref": "dtRef_const_VOID",
                    "Direction": "IN",
                    }],
                "possible_errors": ["E_OK", "E_NOT_OK"]
            }
            service_elment_11 = {
                "short_name": "WritePRAMBlock",
                "arguments": [],
                "possible_errors": ["E_OK", "E_NOT_OK"]
            }
            self.client_server_interfaces[4]["operations"].append(service_elment_4)
            self.client_server_interfaces[4]["operations"].append(service_elment_5)
            self.client_server_interfaces[4]["operations"].append(service_elment_6)
            self.client_server_interfaces[4]["operations"].append(service_elment_7)
            self.client_server_interfaces[4]["operations"].append(service_elment_10)
            self.client_server_interfaces[4]["operations"].append(service_elment_11)
            if DataSetsNumber > 0 :   
                service_elment_2 = {
                    "short_name": "GetDataIndex",
                    "arguments": [{
                        "short_name": "DataIndexPtr",
                        "IsType": False,
                        "TypeTref": "uint8",
                        "Direction": "OUT",
                    }],
                    "possible_errors": ["E_OK","E_NOT_OK"]
                }
                service_elment_8 = {
                    "short_name": "SetDataIndex",
                    "arguments": [{
                        "short_name": "DataIndex",
                        "IsType": False,
                        "TypeTref": "uint8",
                        "Direction": "IN",
                    }],
                    "possible_errors": ["E_OK","E_NOT_OK"]
                }
                self.client_server_interfaces[4]["operations"].append(service_elment_2)
                self.client_server_interfaces[4]["operations"].append(service_elment_8)

        if NvMApiConfig == "NVM_API_CONFIG_CLASS_3":
            admin_element = {
                "short_name": "SetBlockProtection",
                "arguments": [{
                    "short_name": "ProtectionEnabled",
                    "IsType": False,
                    "TypeTref": "boolean",
                    "Direction": "IN",
                }],
                "possible_errors": ["E_OK","E_NOT_OK"]
            }
            service_elment_1 = {
                "short_name": "EraseBlock",
                "arguments": [],
                "possible_errors": ["E_OK","E_NOT_OK"]
            }
            service_elment_3 = {
                "short_name": "InvalidateNvBlock",
                "arguments": [],
                "possible_errors": ["E_OK","E_NOT_OK"]
            }
            self.client_server_interfaces[0]["operations"].append(admin_element)
            self.client_server_interfaces[4]["operations"].append(service_elment_1)
            self.client_server_interfaces[4]["operations"].append(service_elment_3)


    def update_pports(self):
        name_list = self.block_list()
        for name, blockid in name_list:
            PAdmin = {
                    "short_name": "PAdmin_" + name,  
                    "interface_type": "client_server",
                    "interface_ref": "NvMAdmin",
                    "argument": {
                            "type": "nvm_block_id_t",
                            "value": blockid
                        }
            }
            PM = {
                    "short_name": "PM_" + name,  
                    "interface_type": "client_server",
                    "interface_ref": "NvMMirror",
                    "argument": None
            }
            PNIB = {
                    "short_name": "PNIB_" + name,  
                    "interface_type": "client_server",
                    "interface_ref": "NvMNotifyInitBlock",
                    "argument": None
            }
            PNJF = {
                    "short_name": "PNJF_" + name,  
                    "interface_type": "client_server",
                    "interface_ref": "NvMNotifyJobFinished",
                    "argument": None
            }
            PS = {
                    "short_name": "PS_" + name,  
                    "interface_type": "client_server",
                    "interface_ref": "NvMService",
                    "argument": {
                            "type": "nvm_block_id_t",
                            "value": blockid
                        }
            }
            self.pports.append(PAdmin)
            self.rports.append(PM)
            self.rports.append(PNIB)
            self.rports.append(PNJF)
            self.pports.append(PS)

    def update_server_runnables(self):
        for cs_interface in self.client_server_interfaces:
            # print(cs_interface)
            # print("\n")
            if cs_interface["port_type"] == "rport":
                continue
            for operation in cs_interface["operations"]:
                if operation["short_name"] == "EraseBlock":
                    element = {"short_name": operation["short_name"], "symbol": "nvm_erase_nv_block"}
                else:
                    element = {"short_name": operation["short_name"], "symbol": "NvM_" + operation["short_name"]}
                if element not in self.server_runnables:
                    self.server_runnables.append(element)

    def update_invoked_events(self):  # cs p
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
            "nvm_package": {
                "short_name": "NvM_swc",
                "datatypes_pkg": self.data_types,
                "interfaces_pkg": {
                    "mode_switch": self.mode_switch_interfaces,
                    "sender_receiver": self.sender_receiver_interfaces,
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
    nvm_module_cfg = module_ecuc_cfg["NvM"] if "NvM" in module_ecuc_cfg else None
    if not nvm_module_cfg:
        return {}
    nvm_ecuc_cfg_dict = AutosarUtil.to_bsw_dict(nvm_module_cfg)
    dict_config = copy.deepcopy(nvm_ecuc_cfg_dict["ArPackages"])
    # print(dict_config)
    nvm_ecuc_dict = filter.get_template_input_parameter_dict(dict_config)
    port_derive = PortDerive(nvm_ecuc_dict)
    return port_derive.get_config_dict()