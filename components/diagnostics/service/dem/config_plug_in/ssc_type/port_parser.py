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
Dem配置文件解析模块.
解析输入的Dem配置文件, 然后生成模板config.josn
"""

import os
import uuid
import hashlib
from basic_func_module.autosar_utils.autosar_util import AutosarUtil

CUR_FILE_PATH = os.path.dirname(__file__)
DEFAULT_TEMPLATE_FILE_DIR = os.path.join(CUR_FILE_PATH, "template")


def update_short_name_length(short_name) -> str:
    if short_name and len(short_name) > 128:
        hash_obj = hashlib.sha1(short_name[120:].encode(encoding="utf-8"))
        hash_str = hash_obj.hexdigest()
        short_name = short_name[:120] + hash_str[:8]
    return short_name

def dem_get_dict_by_definition_ref(input_dict: dict, definition_ref: str) -> dict:
    for value in input_dict.values():
        if isinstance(value, dict):
            if "DefinitionRef" in value.keys():
                if value["DefinitionRef"].split("/")[-1] == definition_ref:
                    return value
    return {}

class PortDerive():

    def init_dem_config(self):
        data_types = {
            "uuid": str(uuid.uuid4()),
            "elements": [{
                "uuid": str(uuid.uuid4()),
                "short_name": "Dem_DTCOriginType",
                "category": "TYPE_REFERENCE",
                "sw_data_def": {
                    "compu_method_ref": "Dem_DTCOriginType",
                    "data_constr_ref": "Dem_DTCOriginType_Constr",
                    "implementation_data_type": "uint16",
                    "IsType": False
                }
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "Dem_EventIdType",
                "category": "TYPE_REFERENCE",
                "sw_data_def": {
                    "data_constr_ref": "Dem_EventIdType_Constr",
                    "implementation_data_type": "uint16",
                    "IsType": False
                }
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "Dem_EventStatusType",
                "category": "TYPE_REFERENCE",
                "sw_data_def": {
                    "compu_method_ref": "Dem_EventStatusType",
                    "data_constr_ref": "Dem_EventStatusType_Constr",
                    "implementation_data_type": "uint8",
                    "IsType": False
                }
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "Dem_DTCFormatType",
                "category": "TYPE_REFERENCE",
                "sw_data_def": {
                    "compu_method_ref": "Dem_DTCFormatType",
                    "data_constr_ref": "Dem_DTCFormatType_Constr",
                    "implementation_data_type": "uint8",
                    "IsType": False
                }
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "Dem_UdsStatusByteType",
                "category": "TYPE_REFERENCE",
                "sw_data_def": {
                    "compu_method_ref": "Dem_UdsStatusByteType",
                    "data_constr_ref": "Dem_UdsStatusByteType_Constr",
                    "implementation_data_type": "uint8",
                    "IsType": False
                }
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "Dem_DebouncingStateType",
                "category": "TYPE_REFERENCE",
                "sw_data_def": {
                    "compu_method_ref": "Dem_DebouncingStateType",
                    "data_constr_ref": "Dem_DebouncingStateType_Constr",
                    "implementation_data_type": "uint8",
                    "IsType": False
                }
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "Dem_MaxDataValueType",
                "category": "ARRAY",
                "sw_data_def": {
                    "calibration_access": "READ-ONLY"
                },
                "sub_element": {
                    "array_size": "31",
                    "element_type": "uint8"
                }
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "Dem_MonitorStatusType",
                "category": "TYPE_REFERENCE",
                "sw_data_def": {
                    "compu_method_ref": "Dem_MonitorStatusType",
                    "data_constr_ref": "Dem_MonitorStatusType_Constr",
                    "implementation_data_type": "uint8",
                    "IsType": False
                }
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "Dem_DebounceResetStatusType",
                "category": "TYPE_REFERENCE",
                "sw_data_def": {
                    "compu_method_ref": "Dem_DebounceResetStatusType",
                    "data_constr_ref": "Dem_DebounceResetStatusType_Constr",
                    "implementation_data_type": "uint8",
                    "IsType": False
                }
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "Dem_MonitorDataType",
                "category": "TYPE_REFERENCE",
                "sw_data_def": {
                    "implementation_data_type": "uint32",
                    "IsType": False
                }
            }],
            "data_constrs": {
                "uuid":
                    str(uuid.uuid4()),
                "elements": [{
                    "uuid": str(uuid.uuid4()),
                    "short_name": "Dem_DTCOriginType_Constr",
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": 1,
                            "upper_limit": 4,
                        }
                    }]
                }, {
                    "uuid": str(uuid.uuid4()),
                    "short_name": "Dem_EventIdType_Constr",
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": 1,
                            "upper_limit": 65535
                        }
                    }]
                }, {
                    "uuid": str(uuid.uuid4()),
                    "short_name": "Dem_EventStatusType_Constr",
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": 0,
                            "upper_limit": 4
                        }
                    }]
                }, {
                    "uuid": str(uuid.uuid4()),
                    "short_name": "Dem_DTCFormatType_Constr",
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": 0,
                            "upper_limit": 2
                        }
                    }]
                }, {
                    "uuid": str(uuid.uuid4()),
                    "short_name": "Dem_UdsStatusByteType_Constr",
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": 0,
                            "upper_limit": 255
                        }
                    }]
                }, {
                    "uuid": str(uuid.uuid4()),
                    "short_name": "Dem_DebouncingStateType_Constr",
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": 0,
                            "upper_limit": 255
                        }
                    }]
                }, {
                    "uuid": str(uuid.uuid4()),
                    "short_name": "Dem_MonitorStatusType_Constr",
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": 0,
                            "upper_limit": 255
                        }
                    }]
                }, {
                    "uuid": str(uuid.uuid4()),
                    "short_name": "Dem_DebounceResetStatusType_Constr",
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": 0,
                            "upper_limit": 255
                        }
                    }]
                }]
            },
            "compu_methods": {
                "uuid":
                    str(uuid.uuid4()),
                "elements": [{
                    "uuid":
                        str(uuid.uuid4()),
                    "short_name":
                        "Dem_DTCOriginType",
                    "category":
                        "TEXTTABLE",
                    "compu_scales": [{
                        "short_label": "DEM_DTC_ORIGIN_PRIMARY_MEMORY",
                        "lower_limit": 0,
                        "upper_limit": 0
                    }, {
                        "short_label": "DEM_DTC_ORIGIN_MIR_MEMORY",
                        "lower_limit": 1,
                        "upper_limit": 1
                    }, {
                        "short_label": "DEM_DTC_ORIGIN_PERMANENT_MEMORY",
                        "lower_limit": 2,
                        "upper_limit": 2
                    }, {
                        "short_label": "DEM_DTC_ORIGIN_RELEVANT_MEMORY",
                        "lower_limit": 3,
                        "upper_limit": 3
                    }, {
                        "short_label": "DEM_DTC_ORIGIN_USERDEFINED_MEMORY",
                        "lower_limit": 4,
                        "upper_limit": 4
                    }]
                }, {
                    "uuid":
                        str(uuid.uuid4()),
                    "short_name":
                        "Dem_EventStatusType",
                    "category":
                        "TEXTTABLE",
                    "compu_scales": [{
                        "short_label": "DEM_EVENT_STATUS_PASSED",
                        "lower_limit": 0,
                        "upper_limit": 0
                    }, {
                        "short_label": "DEM_EVENT_STATUS_FAILED",
                        "lower_limit": 1,
                        "upper_limit": 1
                    }, {
                        "short_label": "DEM_EVENT_STATUS_PREPASSED",
                        "lower_limit": 2,
                        "upper_limit": 2
                    }, {
                        "short_label": "DEM_EVENT_STATUS_PREFAILED",
                        "lower_limit": 3,
                        "upper_limit": 3
                    }, {
                        "short_label": "DEM_EVENT_STATUS_FDC_THRESHOLD_REACHED",
                        "lower_limit": 4,
                        "upper_limit": 4
                    }]
                }, {
                    "uuid":
                        str(uuid.uuid4()),
                    "short_name":
                        "Dem_DTCFormatType",
                    "category":
                        "TEXTTABLE",
                    "compu_scales": [{
                        "short_label": "DEM_DTC_FORMAT_OBD",
                        "lower_limit": 0,
                        "upper_limit": 0
                    }, {
                        "short_label": "DEM_DTC_FORMAT_UDS",
                        "lower_limit": 1,
                        "upper_limit": 1
                    }, {
                        "short_label": "DEM_DTC_FORMAT_J1939",
                        "lower_limit": 2,
                        "upper_limit": 2
                    }]
                }, {
                    "uuid":
                        str(uuid.uuid4()),
                    "short_name":
                        "Dem_UdsStatusByteType",
                    "category":
                        "TEXTTABLE",
                    "compu_scales": [{
                        "short_label": "EM_UDS_STATUS_TF",
                        "lower_limit": 0x01,
                        "upper_limit": 0x01
                    }, {
                        "short_label": "EM_UDS_STATUS_TFTOC",
                        "lower_limit": 0x02,
                        "upper_limit": 0x02
                    }, {
                        "short_label": "EM_UDS_STATUS_PDTC",
                        "lower_limit": 0x04,
                        "upper_limit": 0x04
                    }, {
                        "short_label": "EM_UDS_STATUS_CDTC",
                        "lower_limit": 0x08,
                        "upper_limit": 0x08
                    }, {
                        "short_label": "EM_UDS_STATUS_TNCSLC",
                        "lower_limit": 0x10,
                        "upper_limit": 0x10
                    }, {
                        "short_label": "EM_UDS_STATUS_TFSLC",
                        "lower_limit": 0x20,
                        "upper_limit": 0x20
                    }, {
                        "short_label": "EM_UDS_STATUS_TNCTOC",
                        "lower_limit": 0x40,
                        "upper_limit": 0x40
                    }, {
                        "short_label": "EM_UDS_STATUS_WIR",
                        "lower_limit": 0x80,
                        "upper_limit": 0x80
                    }]
                }, {
                    "uuid":
                        str(uuid.uuid4()),
                    "short_name":
                        "Dem_DebouncingStateType",
                    "category":
                        "TEXTTABLE",
                    "compu_scales": [{
                        "short_label": "DEM_TEMP_DEFECTIVE",
                        "lower_limit": 1,
                        "upper_limit": 1
                    }, {
                        "short_label": "DEM_FINALLY_DEFECTIVE",
                        "lower_limit": 2,
                        "upper_limit": 2
                    }, {
                        "short_label": "DEM_TEMP_HEALED",
                        "lower_limit": 4,
                        "upper_limit": 4
                    }, {
                        "short_label": "DEM_TEST_COMPLETE",
                        "lower_limit": 8,
                        "upper_limit": 8
                    }, {
                        "short_label": "DEM_DTR_UPDATE",
                        "lower_limit": 16,
                        "upper_limit": 16
                    }]
                }, {
                    "uuid":
                        str(uuid.uuid4()),
                    "short_name":
                        "Dem_MonitorStatusType",
                    "category":
                        "TEXTTABLE",
                    "compu_scales": [{
                        "short_label": "DEM_MONITOR_TF_STATUS",
                        "lower_limit": 1,
                        "upper_limit": 1
                    }, {
                        "short_label": "DEM_MONITOR_TNCTOC_STATUS",
                        "lower_limit": 2,
                        "upper_limit": 2
                    }]
                }, {
                    "uuid":
                        str(uuid.uuid4()),
                    "short_name":
                        "Dem_DebounceResetStatusType",
                    "category":
                        "TEXTTABLE",
                    "compu_scales": [{
                        "short_label": "DEM_DEBOUNCE_STATUS_FREEZE",
                        "lower_limit": 0,
                        "upper_limit": 0
                    }, {
                        "short_label": "DEM_DEBOUNCE_STATUS_RESET",
                        "lower_limit": 1,
                        "upper_limit": 1
                    }]
                }]
            }
        }

        client_server_interfaces = [{
            "uuid":
                str(uuid.uuid4()),
            "short_name":
                "CallbackDTCStatusChange",
            "is_server":
                True,
            "port_type":
                "rport",
            "operations": [{
                "uuid": str(uuid.uuid4()),
                "short_name": "DTCStatusChanged",
                "arguments": [{
                    "uuid": str(uuid.uuid4()),
                    "short_name": "DTC",
                    "IsType": False,
                    "TypeTref": "uint32",
                    "Direction": "IN",
                }, {
                    "uuid": str(uuid.uuid4()),
                    "short_name": "DTCStatusOld",
                    "IsType": True,
                    "TypeTref": "Dem_UdsStatusByteType",
                    "Direction": "IN",
                }, {
                    "uuid": str(uuid.uuid4()),
                    "short_name": "DTCStatusNew",
                    "IsType": True,
                    "TypeTref": "Dem_UdsStatusByteType",
                    "Direction": "IN",
                }],
                "possible_errors": ["E_NOT_OK", "E_OK"]
            }],
            "possible_errors": [{
                "uuid": str(uuid.uuid4()),
                "short_name": "E_OK",
                "error_code": 0
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "E_NOT_OK",
                "error_code": 1
            }],
            "only_master":True
        }, {
            "uuid": str(uuid.uuid4()),
            "short_name": "CallbackEventDataChanged",
            "is_server": True,
            "port_type": "rport",
            "operations": [{
                "uuid": str(uuid.uuid4()),
                "short_name": "EventDataChanged",
                "arguments": [],
                "possible_errors": []
            }],
            "possible_errors": [],
            "only_master":True
        }, {
            "uuid": str(uuid.uuid4()),
            "short_name": "CallbackEventUdsStatusChanged",
            "is_server": True,
            "port_type": "rport",
            "operations": [{
                "uuid": str(uuid.uuid4()),
                "short_name": "CallbackEventUdsStatusChanged",
                "arguments": [{
                    "uuid": str(uuid.uuid4()),
                    "short_name": "EventStatusByteOld",
                    "IsType": True,
                    "TypeTref": "Dem_UdsStatusByteType",
                    "Direction": "IN",
                }, {
                    "uuid": str(uuid.uuid4()),
                    "short_name": "EventStatusByteNew",
                    "IsType": True,
                    "TypeTref": "Dem_UdsStatusByteType",
                    "Direction": "IN",
                }],
                "possible_errors": []
            }],
            "possible_errors": [],
            "only_master":True
        }, {
            "uuid":
                str(uuid.uuid4()),
            "short_name":
                "DiagnosticInfo",
            "is_server":
                True,
            "port_type":
                "pport",
            "operations": [{
                "uuid": str(uuid.uuid4()),
                "short_name": "GetDTCOfEvent",
                "arguments": [{
                    "uuid": str(uuid.uuid4()),
                    "short_name": "DTCFormat",
                    "IsType": True,
                    "TypeTref": "Dem_DTCFormatType",
                    "Direction": "IN",
                }, {
                    "uuid": str(uuid.uuid4()),
                    "short_name": "DTCOfEvent",
                    "IsType": False,
                    "TypeTref": "uint32",
                    "Direction": "OUT",
                }],
                "possible_errors": ["E_OK", "E_NOT_OK", "DEM_E_NO_DTC_AVAILABLE"]
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "GetEventUdsStatus",
                "arguments": [{
                    "uuid": str(uuid.uuid4()),
                    "short_name": "UDSStatusByte",
                    "IsType": True,
                    "TypeTref": "Dem_UdsStatusByteType",
                    "Direction": "OUT",
                }],
                "possible_errors": ["E_OK", "E_NOT_OK"]
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "GetFaultDetectionCounter",
                "arguments": [{
                    "uuid": str(uuid.uuid4()),
                    "short_name": "FaultDetectionCounter",
                    "IsType": False,
                    "TypeTref": "sint8",
                    "Direction": "OUT",
                }],
                "possible_errors": ["E_OK", "E_NOT_OK", "DEM_E_NO_FDC_AVAILABLE"]
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "GetMonitorStatus",
                "arguments": [{
                    "uuid": str(uuid.uuid4()),
                    "short_name": "MonitorStatus",
                    "IsType": True,
                    "TypeTref": "Dem_MonitorStatusType",
                    "Direction": "OUT",
                }],
                "possible_errors": ["E_OK", "E_NOT_OK"]
            }],
            "possible_errors": [{
                "uuid": str(uuid.uuid4()),
                "short_name": "E_OK",
                "error_code": 0
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "E_NOT_OK",
                "error_code": 1
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "DEM_E_NO_DTC_AVAILABLE",
                "error_code": 10
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "DEM_E_NO_FDC_AVAILABLE",
                "error_code": 14
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "DEM_BUFFER_TOO_SMALL",
                "error_code": 21
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "DEM_NO_SUCH_ELEMENT",
                "error_code": 48
            }],
            "only_master":True
        }, {
            "uuid":
                str(uuid.uuid4()),
            "short_name":
                "DiagnosticMonitor",
            "is_server":
                True,
            "port_type":
                "pport",
            "operations": [{
                "uuid": str(uuid.uuid4()),
                "short_name": "ResetEventStatus",
                "arguments": [],
                "possible_errors": ["E_OK", "E_NOT_OK"]
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "SetEventStatus",
                "arguments": [{
                    "uuid": str(uuid.uuid4()),
                    "short_name": "EventStatus",
                    "IsType": True,
                    "TypeTref": "Dem_EventStatusType",
                    "Direction": "IN",
                }],
                "possible_errors": ["E_OK", "E_NOT_OK"]
            }],
            "possible_errors": [{
                "uuid": str(uuid.uuid4()),
                "short_name": "E_OK",
                "error_code": 0
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "E_NOT_OK",
                "error_code": 1
            }],
            "only_master":True
        }, {
            "uuid":
                str(uuid.uuid4()),
            "short_name":
                "EnableCondition",
            "is_server":
                True,
            "port_type":
                "pport",
            "operations": [{
                "uuid": str(uuid.uuid4()),
                "short_name": "SetEnableCondition",
                "arguments": [{
                    "uuid": str(uuid.uuid4()),
                    "short_name": "ConditionFulfilled",
                    "IsType": False,
                    "TypeTref": "boolean",
                    "Direction": "IN",
                }],
                "possible_errors": ["E_OK", "E_NOT_OK"]
            }],
            "possible_errors": [{
                "uuid": str(uuid.uuid4()),
                "short_name": "E_OK",
                "error_code": 0
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "E_NOT_OK",
                "error_code": 1
            }],
            "only_master":True
        }, {
            "uuid":
                str(uuid.uuid4()),
            "short_name":
                "EvMemOverflowIndication",
            "is_server":
                True,
            "port_type":
                "pport",
            "operations": [{
                "uuid": str(uuid.uuid4()),
                "short_name": "GetNumberOfEventMemoryEntries",
                "arguments": [{
                    "uuid": str(uuid.uuid4()),
                    "short_name": "GetNumberOfEventMemoryEntries",
                    "IsType": False,
                    "TypeTref": "uint8",
                    "Direction": "OUT",
                }],
                "possible_errors": ["E_OK", "E_NOT_OK"]
            }],
            "possible_errors": [{
                "uuid": str(uuid.uuid4()),
                "short_name": "E_OK",
                "error_code": 0
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "E_NOT_OK",
                "error_code": 1
            }],
            "only_master":True
        }, {
            "uuid":
                str(uuid.uuid4()),
            "short_name":
                "OperationCycle",
            "is_server":
                True,
            "port_type":
                "pport",
            "operations": [{
                "uuid": str(uuid.uuid4()),
                "short_name": "GetCycleQualified",
                "arguments": [{
                    "uuid": str(uuid.uuid4()),
                    "short_name": "isQualified",
                    "IsType": False,
                    "TypeTref": "boolean",
                    "Direction": "OUT",
                }],
                "possible_errors": ["E_OK", "E_NOT_OK"]
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "RestartOperationCycle",
                "arguments": [],
                "possible_errors": ["E_OK", "E_NOT_OK"]
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "SetCycleQualified",
                "arguments": [],
                "possible_errors": ["E_OK", "E_NOT_OK"]
            }],
            "possible_errors": [{
                "uuid": str(uuid.uuid4()),
                "short_name": "E_OK",
                "error_code": 0
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "E_NOT_OK",
                "error_code": 1
            }],
            "only_master":True
        }, {
            "uuid":
                str(uuid.uuid4()),
            "short_name":
                "StorageCondition",
            "is_server":
                True,
            "port_type":
                "pport",
            "operations": [{
                "uuid": str(uuid.uuid4()),
                "short_name": "SetStorageCondition",
                "arguments": [{
                    "uuid": str(uuid.uuid4()),
                    "short_name": "ConditionFulfilled",
                    "IsType": False,
                    "TypeTref": "boolean",
                    "Direction": "IN",
                }],
                "possible_errors": ["E_OK", "E_NOT_OK"]
            }],
            "possible_errors": [{
                "uuid": str(uuid.uuid4()),
                "short_name": "E_OK",
                "error_code": 0
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "E_NOT_OK",
                "error_code": 1
            }],
            "only_master":True
        }, {
            "uuid":
                str(uuid.uuid4()),
            "short_name":
                "ClearDTC",
            "is_server":
                True,
            "port_type":
                "pport",
            "operations": [{
                "uuid":
                    str(uuid.uuid4()),
                "short_name":
                    "ClearDTC",
                "arguments": [],
                "possible_errors": [
                    "E_OK", "E_NOT_OK", "DEM_PENDING", "DEM_CLEAR_BUSY", "DEM_CLEAR_MEMORY_ERROR", "DEM_CLEAR_FAILED",
                    "DEM_WRONG_DTC", "DEM_WRONG_DTCORIGIN", "DEM_BUSY"
                ]
            }, {
                "uuid":
                    str(uuid.uuid4()),
                "short_name":
                    "SelectDTC",
                "arguments": [{
                    "uuid": str(uuid.uuid4()),
                    "short_name": "DTC",
                    "IsType": False,
                    "TypeTref": "uint32",
                    "Direction": "IN",
                }, {
                    "uuid": str(uuid.uuid4()),
                    "short_name": "DTCFormat",
                    "IsType": True,
                    "TypeTref": "Dem_DTCFormatType",
                    "Direction": "IN",
                }, {
                    "uuid": str(uuid.uuid4()),
                    "short_name": "DTCOrigin",
                    "IsType": True,
                    "TypeTref": "Dem_DTCOriginType",
                    "Direction": "IN",
                }],
                "possible_errors": [
                    "E_OK", "E_NOT_OK"
                ]
            }],
            "possible_errors": [{
                "uuid": str(uuid.uuid4()),
                "short_name": "E_OK",
                "error_code": 0
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "E_NOT_OK",
                "error_code": 1
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "DEM_PENDING",
                "error_code": 4
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "DEM_CLEAR_BUSY",
                "error_code": 5
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "DEM_CLEAR_MEMORY_ERROR",
                "error_code": 6
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "DEM_CLEAR_FAILED",
                "error_code": 7
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "DEM_WRONG_DTC",
                "error_code": 8
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "DEM_WRONG_DTCORIGIN",
                "error_code": 9
            }, {
                "uuid": str(uuid.uuid4()),
                "short_name": "DEM_BUSY",
                "error_code": 22
            }],
            "only_master":True
        }, {
            "uuid": str(uuid.uuid4()),
            "short_name": "GeneralCallbackEventUdsStatusChanged",
            "is_server": True,
            "port_type": "rport",
            "operations": [{
                "uuid": str(uuid.uuid4()),
                "short_name": "GeneralCallbackEventUdsStatusChanged",
                "arguments": [{
                    "uuid": str(uuid.uuid4()),
                    "short_name": "EventId",
                    "IsType": True,
                    "TypeTref": "Dem_EventIdType",
                    "Direction": "IN",
                }, {
                    "uuid": str(uuid.uuid4()),
                    "short_name": "EventStatusByteOld",
                    "IsType": True,
                    "TypeTref": "Dem_UdsStatusByteType",
                    "Direction": "IN",
                }, {
                    "uuid": str(uuid.uuid4()),
                    "short_name": "EventStatusByteNew",
                    "IsType": True,
                    "TypeTref": "Dem_UdsStatusByteType",
                    "Direction": "IN",
                }],
                "possible_errors": []
            }],
            "possible_errors": [],
            "only_master":True
        }]

        pports = []
        rports = []

        main_function_runnable = {
            "uuid": str(uuid.uuid4()),
            "period": 0.01,
            "mode_switch_points": [],
            "server_call_points": [],
            "data_send_points": [],
            "data_receive_points": []
        }

        self.application_list = []
        server_runnables = []
        operation_invoked_events = []
        self.data_types = data_types
        self.client_server_interfaces = client_server_interfaces
        self.pports = pports
        self.rports = rports
        self.main_function_runnable = main_function_runnable
        self.server_runnables = server_runnables
        self.operation_invoked_events = operation_invoked_events
        return

    def __init__(self, ecucs_autosar_dict=None):
        self.ecucs_autosar_dict = ecucs_autosar_dict["ArPackages"]["ActiveEcuC"]["Dem"]
        self.dem_general = dem_get_dict_by_definition_ref(self.ecucs_autosar_dict, "DemGeneral")
        self.dem_config_set = dem_get_dict_by_definition_ref(self.ecucs_autosar_dict, "DemConfigSet")
        self.init_dem_config()
        self.append_dem_data_type_data()
        self.get_append_interface()
        self.dem_client_container = {}
        self._init_dem_client_container()
        self.dem_event_memory_set_container = {}
        self._init_dem_event_memory_set_container()
        self.dem_event_parameters_container = {}
        self._init_dem_event_parameters_container()
        self.dem_opcycle_container = {}
        self._init_dem_opcycle_container()
        self.dem_enable_condition_container = {}
        self._init_dem_enable_condition_container()
        self.dem_storage_condition_container = {}
        self._init_dem_storage_condition_container()
        self.dem_data_elem_class_container = {}
        self._init_dem_data_elem_class_container()
        self.cs_port_interface_ref_list = []
        self._init_cs_port_interface_ref_list_in_arxml()
        self.port_name_rules_map = {}
        self._init_port_name_rules_map()

        self.config_module_dict = {}
        self.update_dem_applications()
        self.update_dem_pports()
        self.update_dem_rports()
        self.update_main_function_runnable()
        self.update_server_runnables()
        self.update_invoked_events()
        self.set_config_dict()

    def update_dem_applications(self):
        dem_applications = []
        dem_general = self.dem_general
        dem_event_parameters = self.dem_event_parameters_container
        # master_application_name = "Application0_Core0" #默认值
        master_application_name = "SystemApplication_Core0" #默认值

        master_application_ref = dem_general["DemMasterOsApplicationRef"]["ValueRef"]
        if master_application_ref is None or master_application_ref == "" or master_application_ref == "<empty>":
            # 单个application情况下使用Dem_Mainfunctin()
            dem_applications = ["Dem"]
            self.application_list = dem_applications
        else:
            master_application_name = master_application_ref.split("/")[-1]
            dem_applications.append("DemMaster_" + master_application_name)

            for _, event_parameter in dem_event_parameters.items():
                application_ref = event_parameter["DemOsApplicationRef"]["ValueRef"]
                if application_ref is not None and application_ref != "" and application_ref != "<empty>":
                    application_name = application_ref.split("/")[-1]
                    if "DemSatellite_" + application_name not in dem_applications:
                        dem_applications.append("DemSatellite_" + application_name)
                elif "DemSatellite_" + master_application_name not in dem_applications:
                    dem_applications.append("DemSatellite_" + master_application_name)
                else:
                    pass

        self.application_list = dem_applications
        self.master_application_name = master_application_name
        return

    def _init_port_name_rules_map(self):
        self.port_name_rules_map = {
            "CallbackDTCStatusChange": "CBStatusDTC_",
            "CallbackEventDataChanged": "CBDataEvt_",
            "CallbackEventUdsStatusChanged": "CBEventUdsStatusChanged_",
            "ClearDtcNotification": "ClearDtcNotification_",
            "StorageCondition": "StorageCond_",
            "OperationCycle": "OpCycle_",
            "DiagnosticMonitor": "Event_",
            "DiagnosticInfo": "EventInfo_",
            "EnableCondition": "EnableCond_",
            "EventAvailable": "ControlEventAvailable",
            "StorageCondition": "StorageCond_",
            "ClearDTC": "ClearDTC_",
            "DTCSuppression": "ControlDTCSuppression",
            "EvMemOverflowIndication_PrimaryMemory": "OverflowIndPrimaryMemory_",
            "EvMemOverflowIndication_UserDefinedMemory": "OverflowIndUserDefinedMemory_",
            "GeneralCallbackEventUdsStatusChanged": "GeneralCBStatusEvt"
        }

    def _init_cs_port_interface_ref_list_in_arxml(self):
        self.cs_port_interface_ref_list = []
        for interface in self.client_server_interfaces:
            cs_port_interface_ref = {}
            cs_port_interface_ref.update({"short_name": interface["short_name"]})
            path = interface["short_name"]
            cs_port_interface_ref.update({"path": path})
            cs_port_interface_ref["operations_ref"] = []
            if "operations" in interface:
                for operation in interface["operations"]:
                    operation_ref = path + "/" + operation["short_name"]
                    cs_port_interface_ref["operations_ref"].append(operation_ref)
            self.cs_port_interface_ref_list.append(cs_port_interface_ref)

    def _init_dem_client_container(self):
        for name, value in self.dem_general.items():
            if "DefinitionRef" not in value or value["DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemClient":
                continue
            self.dem_client_container.update({name: value})
        return

    def _init_dem_event_memory_set_container(self):
        for name, value in self.dem_general.items():
            if "DefinitionRef" not in value or value[
                    "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemEventMemorySet":
                continue
            self.dem_event_memory_set_container.update({name: value})
        return

    def _init_dem_event_parameters_container(self):
        for name, event_parameter in self.dem_config_set.items():
            if "DefinitionRef" not in event_parameter or event_parameter[
                    "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemEventParameter":
                continue
            self.dem_event_parameters_container.update({name: event_parameter})
        return

    def _init_dem_enable_condition_container(self):
        for name, enable_condition in self.dem_general.items():
            if "DefinitionRef" not in enable_condition or enable_condition[
                    "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemEnableCondition":
                continue
            self.dem_enable_condition_container.update({name: enable_condition})
        return

    def _init_dem_opcycle_container(self):
        for name, opcycle in self.dem_general.items():
            if "DefinitionRef" not in opcycle or opcycle[
                    "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemOperationCycle":
                continue
            self.dem_opcycle_container.update({name: opcycle})
        return

    def _init_dem_storage_condition_container(self):
        for name, storage_condition in self.dem_general.items():
            if "DefinitionRef" not in storage_condition or storage_condition[
                    "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemStorageCondition":
                continue
            self.dem_storage_condition_container.update({name: storage_condition})
        return

    def _init_dem_data_elem_class_container(self):
        for name, data_elem_class in self.dem_general.items():
            if "DefinitionRef" not in data_elem_class or data_elem_class[
                    "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemDataElementClass":
                continue
            self.dem_data_elem_class_container.update({name: data_elem_class})
        return

    def update_dem_pports(self):
        self.dem_pports_append_clear_dtc_notification()  # interface ClearDtcNotification
        self.dem_pports_append_clear_dtc()  # interface ClearDTC
        self.dem_pports_append_dtc_suppression()  # interface DTCSuppression
        self.dem_pports_append_eventinfo_name()  # interface DiagnosticInfo
        self.dem_pports_append_event_name()  # interface DiagnosticMonitor
        self.dem_pports_append_enable_cond_name()  # interface EnableCondition
        self.dem_pports_append_control_event_available()  # interface EventAvailable
        self.dem_pports_append_over_flow_indication_primary_memory()  # interface EvMemOverflowIndication
        self.dem_pports_append_over_flow_indication_user_defined_memory()  # interface EvMemOverflowIndication
        self.dem_pports_append_opcycle_name()  # interface OperationCycle
        self.dem_pports_append_storage_cond_name()  # interface StorageCondition
        for pport in self.pports:
            if "Applications" not in pport and self.application_list != []:
                pport["Applications"] = [self.application_list[0]]  #对所有master上的port口生成
        return

    def dem_pports_append_clear_dtc_notification(self):
        interface_name = "ClearDtcNotification"

        port_interface_dict = self.get_port_interface_by_interface_name(interface_name)
        if port_interface_dict == {}:
            return
        for event_memory_set_name, event_memory_set in self.dem_event_memory_set_container.items():
            for clear_dtc_notification_name, clear_dtc_notification in event_memory_set.items():
                if "DefinitionRef" not in clear_dtc_notification or clear_dtc_notification[
                        "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemEventMemorySet/DemClearDTCNotification":
                    continue
                pport_dict = {
                    'uuid': "",
                    'short_name': "",
                    "interface_type": "client_server",
                    "interface_ref": interface_name,
                    'provided_interface_tref': "",
                    "path": "",
                    "operations_ref": port_interface_dict["operations_ref"],
                    "port_arg_values": []
                }
                port_name = self.port_name_rules_map[
                    interface_name] + event_memory_set_name + "_" + clear_dtc_notification_name
                provided_interface_tref = port_interface_dict["path"]
                pport_dict.update({"short_name": update_short_name_length(port_name)})
                pport_dict.update({"provided_interface_tref": provided_interface_tref})
                pport_dict.update({"uuid": str(uuid.uuid4())})
                pport_dict.update({"path": f"AUTOSAR/Dem_swc/ComponentTypes/Dem/{port_name}"})

                self.pports.append(pport_dict)

        return

    def dem_pports_append_clear_dtc(self):
        interface_name = "ClearDTC"

        port_interface_dict = self.get_port_interface_by_interface_name(interface_name)
        if port_interface_dict == {}:
            return
        for client_name, client in self.dem_client_container.items():
            if client["DemClientUsesRte"]["Value"] != True or client["DemClientFunctionality"][
                    "Value"] != "DEM_CLIENT_USES_FULL_FUNCTIONALITY":
                continue
            pport_dict = {
                'uuid': "",
                'short_name': "",
                "interface_type": "client_server",
                "interface_ref": interface_name,
                'provided_interface_tref': "",
                "path": "",
                "operations_ref": port_interface_dict["operations_ref"],
                "port_arg_values": [{
                    "value": client["DemClientId"]["Value"],
                    "value_type_ref": "uint8",
                    "IsType": False
                }]
            }
            port_name = self.port_name_rules_map[interface_name] + client_name
            provided_interface_tref = port_interface_dict["path"]
            pport_dict.update({"short_name": update_short_name_length(port_name)})
            pport_dict.update({"provided_interface_tref": provided_interface_tref})
            pport_dict.update({"uuid": str(uuid.uuid4())})
            pport_dict.update({"path": f"AUTOSAR/Dem_swc/ComponentTypes/Dem/{port_name}"})

            self.pports.append(pport_dict)

        return

    def dem_pports_append_dtc_suppression(self):
        interface_name = "DTCSuppression"

        port_interface_dict = self.get_port_interface_by_interface_name(interface_name)
        if port_interface_dict == {}:
            return
        if self.dem_general["DemSuppressionSupport"]["Value"] != "DEM_DTC_SUPPRESSION":
            return

        pport_dict = {
            'uuid': "",
            'short_name': "",
            "interface_type": "client_server",
            "interface_ref": interface_name,
            'provided_interface_tref': "",
            "path": "",
            "operations_ref": port_interface_dict["operations_ref"],
            "port_arg_values": []
        }
        port_name = self.port_name_rules_map[interface_name]
        provided_interface_tref = port_interface_dict["path"]
        pport_dict.update({"short_name": update_short_name_length(port_name)})
        pport_dict.update({"provided_interface_tref": provided_interface_tref})
        pport_dict.update({"uuid": str(uuid.uuid4())})
        pport_dict.update({"path": f"AUTOSAR/Dem_swc/ComponentTypes/Dem/{port_name}"})

        self.pports.append(pport_dict)

        return

    def dem_pports_append_eventinfo_name(self):
        interface_name = "DiagnosticInfo"
        application_list = []

        # DiagnosticInfo 只在Satellite中生成
        for application in self.application_list:
            if application != "DemMaster_" + self.master_application_name:
                application_list.append(application)

        port_interface_dict = self.get_port_interface_by_interface_name(interface_name)
        if port_interface_dict == {}:
            return
        for name, value in self.dem_event_parameters_container.items():
            Applications = []
            pport_dict = {
                'uuid': "",
                'short_name': "",
                "interface_type": "client_server",
                "interface_ref": interface_name,
                'provided_interface_tref': "",
                "path": "",
                "operations_ref": port_interface_dict["operations_ref"],
                "port_arg_values": [{
                    "value": value["DemEventId"]["Value"],
                    "value_type_ref": "Dem_EventIdType",
                    "IsType": True
                }],
            }
            if "Dem" in self.application_list:
                Applications.append("Dem")
            else:
                Applications.append(self.get_event_application(value))
            pport_dict.update({"Applications": Applications})
            port_name = self.port_name_rules_map[interface_name] + name
            provided_interface_tref = port_interface_dict["path"]
            pport_dict.update({"short_name": update_short_name_length(port_name)})
            pport_dict.update({"provided_interface_tref": provided_interface_tref})
            pport_dict.update({"uuid": str(uuid.uuid4())})
            pport_dict.update({"path": f"AUTOSAR/Dem_swc/ComponentTypes/Dem/{port_name}"})

            self.pports.append(pport_dict)

        return

    def get_event_application(self, event_parameter) -> str:
        application_name = self.master_application_name
        application_ref = event_parameter["DemOsApplicationRef"]["ValueRef"]
        if application_ref is not None and application_ref != "" and application_ref != "<empty>":
            application_name = application_ref.split("/")[-1]
        return "DemSatellite_" + application_name

    def dem_pports_append_event_name(self):
        interface_name = "DiagnosticMonitor"

        port_interface_dict = self.get_port_interface_by_interface_name(interface_name)
        if port_interface_dict == {}:
            return
        for name, value in self.dem_event_parameters_container.items():
            Applications = []
            pport_dict = {
                'uuid': "",
                'short_name': "",
                "interface_type": "client_server",
                "interface_ref": interface_name,
                'provided_interface_tref': "",
                "path": "",
                "operations_ref": port_interface_dict["operations_ref"],
                "port_arg_values": [{
                    "value": value["DemEventId"]["Value"],
                    "value_type_ref": "Dem_EventIdType",
                    "IsType": True
                }]
            }
            if "Dem" in self.application_list:
                Applications.append("Dem")
            else:
                Applications.append(self.get_event_application(value))
            pport_dict.update({"Applications": Applications})
            port_name = self.port_name_rules_map[interface_name] + name
            provided_interface_tref = port_interface_dict["path"]
            pport_dict.update({"short_name": update_short_name_length(port_name)})
            pport_dict.update({"provided_interface_tref": provided_interface_tref})
            pport_dict.update({"uuid": str(uuid.uuid4())})
            pport_dict.update({"path": f"AUTOSAR/Dem_swc/ComponentTypes/Dem/{port_name}"})

            self.pports.append(pport_dict)

        return

    def dem_pports_append_enable_cond_name(self):
        interface_name = "EnableCondition"

        port_interface_dict = self.get_port_interface_by_interface_name(interface_name)
        if port_interface_dict == {}:
            return
        for name, value in self.dem_enable_condition_container.items():
            pport_dict = {
                'uuid': "",
                'short_name': "",
                "interface_type": "client_server",
                "interface_ref": interface_name,
                'provided_interface_tref': "",
                "path": "",
                "operations_ref": port_interface_dict["operations_ref"],
                "port_arg_values": [{
                    "value": value["DemEnableConditionId"]["Value"],
                    "value_type_ref": "uint8",
                    "IsType": False
                }]
            }
            port_name = self.port_name_rules_map[interface_name] + name
            provided_interface_tref = port_interface_dict["path"]
            pport_dict.update({"short_name": update_short_name_length(port_name)})
            pport_dict.update({"provided_interface_tref": provided_interface_tref})
            pport_dict.update({"uuid": str(uuid.uuid4())})
            pport_dict.update({"path": f"AUTOSAR/Dem_swc/ComponentTypes/Dem/{port_name}"})

            self.pports.append(pport_dict)

        return

    def dem_pports_append_control_event_available(self):
        interface_name = "EventAvailable"

        port_interface_dict = self.get_port_interface_by_interface_name(interface_name)
        if port_interface_dict == {}:
            return
        for name, value in self.dem_event_parameters_container.items():
            pport_dict = {
                'uuid': "",
                'short_name': "",
                "interface_type": "client_server",
                "interface_ref": interface_name,
                'provided_interface_tref': "",
                "path": "",
                "operations_ref": port_interface_dict["operations_ref"],
                "port_arg_values": [{
                    "value": value["DemEventId"]["Value"],
                    "value_type_ref": "Dem_EventIdType",
                    "IsType": True
                }]
            }
            port_name = self.port_name_rules_map[interface_name] + name
            provided_interface_tref = port_interface_dict["path"]
            pport_dict.update({"short_name": update_short_name_length(port_name)})
            pport_dict.update({"provided_interface_tref": provided_interface_tref})
            pport_dict.update({"uuid": str(uuid.uuid4())})
            pport_dict.update({"path": f"AUTOSAR/Dem_swc/ComponentTypes/Dem/{port_name}"})

            self.pports.append(pport_dict)

        return

    def dem_pports_append_over_flow_indication_primary_memory(self):
        interface_name = "EvMemOverflowIndication"
        map_name = interface_name + "_PrimaryMemory"

        port_interface_dict = self.get_port_interface_by_interface_name(interface_name)
        if port_interface_dict == {}:
            return
        for name, value in self.dem_client_container.items():
            if value["DemClientUsesRte"]["Value"] != True:
                continue
            pport_dict = {
                'uuid': "",
                'short_name': "",
                "interface_type": "client_server",
                "interface_ref": interface_name,
                'provided_interface_tref': "",
                "path": "",
                "operations_ref": port_interface_dict["operations_ref"],
                "port_arg_values": [{
                    "value_type_ref": "uint8",
                    "IsType": False
                }, {
                    "value": 1,
                    "value_type_ref": "Dem_DTCOriginType",
                    "IsType": True
                }]
            }
            port_name = self.port_name_rules_map[map_name] + name
            provided_interface_tref = port_interface_dict["path"]
            pport_dict.update({"short_name": port_name})
            pport_dict.update({"provided_interface_tref": provided_interface_tref})
            pport_dict.update({"uuid": str(uuid.uuid4())})
            pport_dict.update({"path": f"AUTOSAR/Dem_swc/ComponentTypes/Dem/{port_name}"})

            self.pports.append(pport_dict)

        return

    def dem_pports_append_over_flow_indication_user_defined_memory(self):
        interface_name = "EvMemOverflowIndication"
        map_name = interface_name + "_UserDefinedMemory"

        port_interface_dict = self.get_port_interface_by_interface_name(interface_name)
        if port_interface_dict == {}:
            return
        for client_name, value in self.dem_client_container.items():
            if value["DemClientUsesRte"]["Value"] != True:
                continue
            event_memory_set = AutosarUtil.get_by_path_from_dict(
                self.ecucs_autosar_dict, value["DemEventMemorySetRef"]["ValueRef"].replace("/ActiveEcuC/Dem/", "/"))
            if event_memory_set is None or "DemUserDefinedMemory" not in event_memory_set or event_memory_set[
                    "DemUserDefinedMemory"] == "" or event_memory_set[
                        "DemUserDefinedMemory"] == "<empty>" or event_memory_set[
                            "DemUserDefinedMemory"] is None:
                continue
            for user_defined_memory_name, user_defined_memory in event_memory_set.items():
                if "DefinitionRef" not in user_defined_memory or user_defined_memory[
                        "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemEventMemorySet/DemUserDefinedMemory":
                    continue
                pport_dict = {
                    'uuid':
                        "",
                    'short_name':
                        "",
                    "interface_type":
                        "client_server",
                    "interface_ref":
                        interface_name,
                    'provided_interface_tref':
                        "",
                    "path":
                        "",
                    "operations_ref":
                        port_interface_dict["operations_ref"],
                    "port_arg_values": [{
                        "value_type_ref": "uint8",
                        "IsType": False
                    }, {
                        "value": user_defined_memory["DemUserDefinedMemoryIdentifier"]["Value"],
                        "value_type_ref": "Dem_DTCOriginType",
                        "IsType": True
                    }]
                }
                port_name = self.port_name_rules_map[map_name] + client_name + "_" + user_defined_memory_name
                provided_interface_tref = port_interface_dict["path"]
                pport_dict.update({"short_name": port_name})
                pport_dict.update({"provided_interface_tref": provided_interface_tref})
                pport_dict.update({"uuid": str(uuid.uuid4())})
                pport_dict.update({"path": f"AUTOSAR/Dem_swc/ComponentTypes/Dem/{port_name}"})

                self.pports.append(pport_dict)

        return

    def dem_pports_append_opcycle_name(self):
        interface_name = "OperationCycle"

        port_interface_dict = self.get_port_interface_by_interface_name(interface_name)
        if port_interface_dict == {}:
            return
        for name, value in self.dem_opcycle_container.items():
            pport_dict = {
                'uuid': "",
                'short_name': "",
                "interface_type": "client_server",
                "interface_ref": interface_name,
                'provided_interface_tref': "",
                "path": "",
                "operations_ref": port_interface_dict["operations_ref"],
                "port_arg_values": [{
                    "value": value["DemOperationCycleId"]["Value"],
                    "value_type_ref": "uint8",
                    "IsType": False
                }]
            }
            port_name = self.port_name_rules_map[interface_name] + name
            provided_interface_tref = port_interface_dict["path"]
            pport_dict.update({"short_name": update_short_name_length(port_name)})
            pport_dict.update({"provided_interface_tref": provided_interface_tref})
            pport_dict.update({"uuid": str(uuid.uuid4())})
            pport_dict.update({"path": f"AUTOSAR/Dem_swc/ComponentTypes/Dem/{port_name}"})

            self.pports.append(pport_dict)
        return

    def dem_pports_append_storage_cond_name(self):
        interface_name = "StorageCondition"

        port_interface_dict = self.get_port_interface_by_interface_name(interface_name)
        if port_interface_dict == {}:
            return
        for name, value in self.dem_storage_condition_container.items():
            pport_dict = {
                'uuid': "",
                'short_name': "",
                "interface_type": "client_server",
                "interface_ref": interface_name,
                'provided_interface_tref': "",
                "path": "",
                "operations_ref": port_interface_dict["operations_ref"],
                "port_arg_values": [{
                    "value": value["DemStorageConditionId"]["Value"],
                    "value_type_ref": "uint8",
                    "IsType": False
                }]
            }
            port_name = self.port_name_rules_map[interface_name] + name
            provided_interface_tref = port_interface_dict["path"]
            pport_dict.update({"short_name": update_short_name_length(port_name)})
            pport_dict.update({"provided_interface_tref": provided_interface_tref})
            pport_dict.update({"uuid": str(uuid.uuid4())})
            pport_dict.update({"path": f"AUTOSAR/Dem_swc/ComponentTypes/Dem/{port_name}"})

            self.pports.append(pport_dict)

        return

    def get_port_interface_by_interface_name(self, short_name: str) -> dict:
        port_interface_dict = {}

        for port_interface in self.cs_port_interface_ref_list:
            if short_name == port_interface["short_name"]:
                port_interface_dict = port_interface
                break

        return port_interface_dict

    def update_dem_rports(self):
        # self.dem_rports_append_callback_dtc_status_change() # interface CallbackDTCStatusChange
        self.dem_rports_append_callback_event_data_changed()  # interface CallbackEventDataChanged
        self.dem_rports_append_callback_event_uds_status_changed()  # interface CallbackEventUdsStatusChanged
        self.dem_rports_append_data_services_data()  # interface DataServices_{Data}
        self.dem_rports_append_general_callback_uds_status_changed()  # interface GeneralCallbackEventUdsStatusChanged
        for rport in self.rports:
            if "Applications" not in rport and self.application_list != []:
                rport["Applications"] = [self.application_list[0]]  #对所有master上的port口生成
        return

    def dem_rports_append_callback_dtc_status_change(self):
        interface_name = "CallbackDTCStatusChange"

        port_interface_dict = self.get_port_interface_by_interface_name(interface_name)
        if port_interface_dict == {}:
            return
        for _, client in self.dem_client_container.items():
            for name, value in client.items():
                if "DefinitionRef" not in value or value[
                        "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemClient/DemCallbackDTCStatusChanged":
                    continue
                rport_dict = {
                    'uuid': "",
                    'short_name': "",
                    "interface_type": "client_server",
                    "interface_ref": interface_name,
                    'provided_interface_tref': "",
                    "path": "",
                    "operations_ref": port_interface_dict["operations_ref"],
                    "port_arg_values": []
                }
                port_name = self.port_name_rules_map[interface_name] + name
                provided_interface_tref = port_interface_dict["path"]
                rport_dict.update({"short_name": update_short_name_length(port_name)})
                rport_dict.update({"provided_interface_tref": provided_interface_tref})
                rport_dict.update({"uuid": str(uuid.uuid4())})
                rport_dict.update({"path": f"AUTOSAR/Dem_swc/ComponentTypes/Dem/{port_name}"})

                self.rports.append(rport_dict)

        return

    def dem_rports_append_callback_event_data_changed(self):
        interface_name = "CallbackEventDataChanged"

        port_interface_dict = self.get_port_interface_by_interface_name(interface_name)
        if port_interface_dict == {}:
            return
        for name, event_parameter in self.dem_event_parameters_container.items():
            for _, value in event_parameter.items():
                if "DefinitionRef" not in value or value[
                        "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemEventParameter/DemCallbackEventDataChanged" or "DemCallbackEventDataChangedFnc" not in value:
                    continue
                if value["DemCallbackEventDataChangedFnc"]["Value"] != "" and value["DemCallbackEventDataChangedFnc"][
                        "Value"] != "<empty>" and value["DemCallbackEventDataChangedFnc"]["Value"] is not None:
                    continue
                rport_dict = {
                    'uuid': "",
                    'short_name': "",
                    "interface_type": "client_server",
                    "interface_ref": interface_name,
                    'provided_interface_tref': "",
                    "path": "",
                    "operations_ref": port_interface_dict["operations_ref"],
                    "port_arg_values": []
                }
                port_name = self.port_name_rules_map[interface_name] + name
                provided_interface_tref = port_interface_dict["path"]
                rport_dict.update({"short_name": update_short_name_length(port_name)})
                rport_dict.update({"provided_interface_tref": provided_interface_tref})
                rport_dict.update({"uuid": str(uuid.uuid4())})
                rport_dict.update({"path": f"AUTOSAR/Dem_swc/ComponentTypes/Dem/{port_name}"})

                self.rports.append(rport_dict)

        return

    def dem_rports_append_callback_event_uds_status_changed(self):
        interface_name = "CallbackEventUdsStatusChanged"

        port_interface_dict = self.get_port_interface_by_interface_name(interface_name)
        if port_interface_dict == {}:
            return
        for event_name, event_parameter in self.dem_event_parameters_container.items():
            for callback_name, value in event_parameter.items():
                if "DefinitionRef" not in value or value[
                        "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemEventParameter/DemCallbackEventUdsStatusChanged":
                    continue
                rport_dict = {
                    'uuid': "",
                    'short_name': "",
                    "interface_type": "client_server",
                    "interface_ref": interface_name,
                    'provided_interface_tref': "",
                    "path": "",
                    "operations_ref": port_interface_dict["operations_ref"],
                    "port_arg_values": []
                }
                port_name = self.port_name_rules_map[interface_name] + event_name + "_" + callback_name
                provided_interface_tref = port_interface_dict["path"]
                rport_dict.update({"short_name": update_short_name_length(port_name)})
                rport_dict.update({"provided_interface_tref": provided_interface_tref})
                rport_dict.update({"uuid": str(uuid.uuid4())})
                rport_dict.update({"path": f"AUTOSAR/Dem_swc/ComponentTypes/Dem/{port_name}"})

                self.rports.append(rport_dict)

        return

    def dem_rports_append_data_services_data(self):
        for data_elem_class_name, data_elem_class in self.dem_data_elem_class_container.items():
            if self.get_data_services_data_config_state(data_elem_class) == False:
                continue
            interface_name = "DataServices_" + data_elem_class_name
            port_interface_dict = self.get_port_interface_by_interface_name(interface_name)
            if port_interface_dict == {}:
                return
            rport_dict = {
                'uuid': "",
                'short_name': "",
                "interface_type": "client_server",
                "interface_ref": interface_name,
                'provided_interface_tref': "",
                "path": "",
                "operations_ref": port_interface_dict["operations_ref"],
                "port_arg_values": []
            }
            if "Dem" in self.application_list:
                rport_dict["Applications"] = "Dem"
            else:
                rport_dict["Applications"] = ["DemMaster_" + self.master_application_name, "DemSatellite_" + self.master_application_name]
            port_name = interface_name
            provided_interface_tref = port_interface_dict["path"]
            rport_dict.update({"short_name": update_short_name_length(port_name)})
            rport_dict.update({"provided_interface_tref": provided_interface_tref})
            rport_dict.update({"uuid": str(uuid.uuid4())})
            rport_dict.update({"path": f"AUTOSAR/Dem_swc/ComponentTypes/Dem/{port_name}"})

            self.rports.append(rport_dict)

        return

    def dem_rports_append_general_callback_uds_status_changed(self):
        interface_name = "GeneralCallbackEventUdsStatusChanged"

        port_interface_dict = self.get_port_interface_by_interface_name(interface_name)
        if port_interface_dict == {}:
            return
        for name, value in self.dem_general.items():
            if "DefinitionRef" not in value or value[
                    "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemGeneralInterfaceSupport":
                continue
            if value["Value"] == True:
                rport_dict = {
                    'uuid': "",
                    'short_name': "",
                    "interface_type": "client_server",
                    "interface_ref": interface_name,
                    'provided_interface_tref': "",
                    "path": "",
                    "operations_ref": port_interface_dict["operations_ref"],
                    "port_arg_values": []
                }
                port_name = self.port_name_rules_map[interface_name]
                provided_interface_tref = port_interface_dict["path"]
                rport_dict.update({"short_name": update_short_name_length(port_name)})
                rport_dict.update({"provided_interface_tref": provided_interface_tref})
                rport_dict.update({"uuid": str(uuid.uuid4())})
                rport_dict.update({"path": f"AUTOSAR/Dem_swc/ComponentTypes/Dem/{port_name}"})

                self.rports.append(rport_dict)
                break

        return

    def get_data_services_data_config_state(self, data_elem_class: dict):
        state = False

        for _, value in data_elem_class.items():
            if "DefinitionRef" in value and value[
                    "DefinitionRef"] == "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemDataElementClass/DemExternalCSDataElementClass":
                if value["DemDataElementUsePort"]["Value"] == True:
                    state = True
            elif "DefinitionRef" in value and value[
                    "DefinitionRef"] == "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemDataElementClass/DemExternalSRDataElementClass":
                state = True

        return state

    def append_dem_data_type_data(self):
        self.append_dem_data_array_type_data()
        self.append_dem_data_primitive_type_data()
        return

    def get_size_limit_of_base_type(self, type):
        base_type = {
            "boolean": {
                "lower_limit": 0,
                "upper_limit": 1
            },
            "sint8": {
                "lower_limit": -128,
                "upper_limit": 127
            },
            "uint8": {
                "lower_limit": 0,
                "upper_limit": 255
            },
            "sint16": {
                "lower_limit": -32768,
                "upper_limit": 32767
            },
            "uint16": {
                "lower_limit": 0,
                "upper_limit": 65535
            },
            "sint32": {
                "lower_limit": -2147483648,
                "upper_limit": 2147483647
            },
            "uint32": {
                "lower_limit": 0,
                "upper_limit": 4294967295
            }
        }
        if type in base_type.keys():
            return base_type[type]["lower_limit"], base_type[type]["upper_limit"]
        else:
            return base_type["uint8"]["lower_limit"], base_type["uint8"]["upper_limit"]

    def append_dem_data_array_type_data(self):
        for data_elem_class_name, data_elem_class in self.dem_general.items():
            if "DefinitionRef" not in data_elem_class or data_elem_class[
                    "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemDataElementClass":
                continue
            implementation_data_type = ""
            type = ""
            category = ""
            data_array_type_name = ""
            data_type_data_name = ""
            array_size = 0
            for _, value in data_elem_class.items():
                if "DefinitionRef" in value and value[
                        "DefinitionRef"] == "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemDataElementClass/DemExternalCSDataElementClass":
                    if value["DemDataElementDataType"]["Value"] in [
                            "SINT16_N", "SINT32_N", "SINT8_N", "UINT16_N", "UINT32_N", "UINT8_N"
                    ]:
                        type = value["DemDataElementDataType"]["Value"].split("_")[0].lower()
                        data_array_type_name = "Dem_DataArrayType_" + data_elem_class_name
                        category = "ARRAY"
                        array_size = value["DemDataElementArraySize"]["Value"]
                        if value["DemDataElementUsePort"]["Value"] == True:
                            implementation_data_type = data_array_type_name
                    else:
                        continue
                elif "DefinitionRef" in value and value[
                        "DefinitionRef"] == "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemDataElementClass/DemExternalSRDataElementClass":
                    if value["DemDataElementDataType"]["Value"] in [
                            "SINT16_N", "SINT32_N", "SINT8_N", "UINT16_N", "UINT32_N", "UINT8_N"
                    ]:
                        type = value["DemDataElementDataType"]["Value"].split("_")[0].lower()
                        data_array_type_name = "Dem_DataArrayType_" + data_elem_class_name
                        category = "ARRAY"
                        array_size = value["DemDataElementArraySize"]["Value"]
                        implementation_data_type = data_array_type_name
                    else:
                        continue
                else:
                    continue

                data_array_type_constr_name = data_array_type_name + "_Constr"
                data_array_type_element = {
                    "uuid": str(uuid.uuid4()),
                    "short_name": update_short_name_length(data_array_type_name),
                    "category": category,
                    "sw_data_def": {
                        "calibration_access": "READ-ONLY"
                    },
                    "sub_element": {
                        "array_size": array_size,
                        "element_type": type
                    }
                }
                self.data_types["elements"].append(data_array_type_element)

                lower_limit, upper_limit = self.get_size_limit_of_base_type(type)

                data_base_type_constr = {
                    "uuid": str(uuid.uuid4()),
                    "short_name": update_short_name_length(data_array_type_constr_name),
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": lower_limit,
                            "upper_limit": upper_limit
                        }
                    }]
                }
                self.data_types["data_constrs"]["elements"].append(data_base_type_constr)

                if implementation_data_type != "":
                    data_type_data_name = "Dem_DataType_" + data_elem_class_name
                    data_type_constr_name = data_type_data_name + "_Constr"
                    data_type_element = {
                        "uuid": str(uuid.uuid4()),
                        "short_name": update_short_name_length(data_type_data_name),
                        "category": "TYPE_REFERENCE",
                        "sw_data_def": {
                            "data_constr_ref": data_type_constr_name,
                            "implementation_data_type": type,
                            "IsType": False
                        }
                    }
                    self.data_types["elements"].append(data_type_element)
                    data_type_constr = {
                        "uuid": str(uuid.uuid4()),
                        "short_name": update_short_name_length(data_type_constr_name),
                        "data_constr_rules": [{
                            "internal_consters": {
                                "lower_limit": lower_limit,
                                "upper_limit": upper_limit
                            }
                        }]
                    }
                    self.data_types["data_constrs"]["elements"].append(data_type_constr)

        return

    def append_dem_data_primitive_type_data(self):
        category = ""
        data_primitive_type_name = ""
        data_type_data_name = ""

        for data_elem_class_name, data_elem_class in self.dem_general.items():
            if "DefinitionRef" not in data_elem_class or data_elem_class[
                    "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemDataElementClass":
                continue
            implementation_data_type = ""
            type = ""
            for name, value in data_elem_class.items():
                if "DefinitionRef" in value and value[
                        "DefinitionRef"] == "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemDataElementClass/DemExternalCSDataElementClass":
                    if value["DemDataElementDataType"]["Value"] in [
                            "BOOLEAN", "SINT16", "SINT32", "SINT8", "UINT16", "UINT32", "UINT8"
                    ]:
                        type = "uint8"  #value["DemDataElementDataType"]["Value"].lower() 工具传进来的类型有误，暂时规避
                        data_primitive_type_name = "Dem_DataPrimitiveType_" + data_elem_class_name
                        category = "TYPE_REFERENCE"
                        if value["DemDataElementUsePort"]["Value"] == True:
                            implementation_data_type = data_primitive_type_name
                    else:
                        continue
                elif "DefinitionRef" in value and value[
                        "DefinitionRef"] == "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemDataElementClass/DemExternalSRDataElementClass":
                    if value["DemDataElementDataType"]["Value"] in [
                            "BOOLEAN", "SINT16", "SINT32", "SINT8", "UINT16", "UINT32", "UINT8"
                    ]:
                        type = value["DemDataElementDataType"]["Value"].lower()
                        data_primitive_type_name = "Dem_DataPrimitiveType_" + data_elem_class_name
                        category = "TYPE_REFERENCE"
                        implementation_data_type = data_primitive_type_name
                    else:
                        continue
                else:
                    continue

                data_primitive_type_constr_name = data_primitive_type_name + "_Constr"
                data_primitive_type_element = {
                    "uuid": str(uuid.uuid4()),
                    "short_name": update_short_name_length(data_primitive_type_name),
                    "category": category,
                    "sw_data_def": {
                        "data_constr_ref": data_primitive_type_constr_name,
                        "implementation_data_type": type,
                        "IsType": False
                    }
                }
                self.data_types["elements"].append(data_primitive_type_element)

                lower_limit, upper_limit = self.get_size_limit_of_base_type(type)

                data_base_type_constr = {
                    "uuid": str(uuid.uuid4()),
                    "short_name": update_short_name_length(data_primitive_type_constr_name),
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": lower_limit,
                            "upper_limit": upper_limit
                        }
                    }]
                }
                self.data_types["data_constrs"]["elements"].append(data_base_type_constr)

                if implementation_data_type != "":
                    data_type_data_name = "Dem_DataType_" + data_elem_class_name
                    data_type_constr_name = data_type_data_name + "_Constr"
                    data_type_element = {
                        "uuid": str(uuid.uuid4()),
                        "short_name": update_short_name_length(data_type_data_name),
                        "category": "TYPE_REFERENCE",
                        "sw_data_def": {
                            "data_constr_ref": data_type_constr_name,
                            "implementation_data_type": type,
                            "IsType": False
                        }
                    }
                    self.data_types["elements"].append(data_type_element)
                    data_type_constr = {
                        "uuid": str(uuid.uuid4()),
                        "short_name": update_short_name_length(data_type_constr_name),
                        "data_constr_rules": [{
                            "internal_consters": {
                                "lower_limit": lower_limit,
                                "upper_limit": upper_limit
                            }
                        }]
                    }
                    self.data_types["data_constrs"]["elements"].append(data_type_constr)

        return

    def get_append_interface(self):
        cs_interfaces = self.client_server_interfaces

        if self.get_clear_dtc_notification_config_state() == True:
            clear_dtc_notification_interface = {
                "uuid": str(uuid.uuid4()),
                "short_name": "ClearDtcNotification",
                "is_server": True,
                "port_type": "pport",
                "operations": [{
                    "uuid": str(uuid.uuid4()),
                    "short_name": "ClearDtcNotification",
                    "arguments": [{
                        "uuid": str(uuid.uuid4()),
                        "short_name": "DTC",
                        "IsType": False,
                        "TypeTref": "uint32",
                        "Direction": "IN",
                    }, {
                        "uuid": str(uuid.uuid4()),
                        "short_name": "DTCFormat",
                        "IsType": True,
                        "TypeTref": "Dem_DTCFormatType",
                        "Direction": "IN",
                    }, {
                        "uuid": str(uuid.uuid4()),
                        "short_name": "DTCOrigin",
                        "IsType": True,
                        "TypeTref": "Dem_DTCOriginType",
                        "Direction": "IN",
                    }],
                    "possible_errors": ["E_OK"]
                }],
                "possible_errors": [{
                    "uuid": str(uuid.uuid4()),
                    "short_name": "E_OK",
                    "error_code": 0
                }],
                "only_master":True
            }
            cs_interfaces.append(clear_dtc_notification_interface)

        if self.get_debouncing_of_event_config_state() == True:
            get_debouncing_of_event_operation = {
                "uuid": str(uuid.uuid4()),
                "short_name": "GetDebouncingOfEvent",
                "arguments": [{
                    "uuid": str(uuid.uuid4()),
                    "short_name": "DebouncingState",
                    "IsType": True,
                    "TypeTref": "Dem_DebouncingStateType",
                    "Direction": "OUT",
                }],
                "possible_errors": ["E_OK", "E_NOT_OK"]
            }
            self.append_operation_for_interface(get_debouncing_of_event_operation, "DiagnosticInfo")

        # 暂不支持配置，默认使能去抖
        # if self.dem_general["DemDebounceCounterBasedSupport"]["Value"] == True or self.dem_general["DemDebounceTimeBasedSupport"]["Value"] == True:
        reset_event_debounce_status_operation = {
            "uuid": str(uuid.uuid4()),
            "short_name": "ResetEventDebounceStatus",
            "arguments": [{
                "uuid": str(uuid.uuid4()),
                "short_name": "DebounceResetStatus",
                "IsType": True,
                "TypeTref": "Dem_DebounceResetStatusType",
                "Direction": "IN",
            }],
            "possible_errors": ["E_OK", "E_NOT_OK"]
        }
        self.append_operation_for_interface(reset_event_debounce_status_operation, "DiagnosticMonitor")
        '''
        if self.dem_general["DemOBDSupport"]["Value"] != "DEM_OBD_NO_OBD_SUPPORT":
            set_event_disabled_operation = {
                "uuid": str(uuid.uuid4()),
                "short_name": "SetEventDisabled",
                "arguments": [],
                "possible_errors": ["E_OK", "E_NOT_OK"]
            }
            self.append_operation_for_interface(set_event_disabled_operation, "DiagnosticMonitor")
        '''

        if self.dem_general["DemAvailabilitySupport"]["Value"] == "DEM_EVENT_AVAILABILITY":
            event_available_interface = {
                "uuid":
                    str(uuid.uuid4()),
                "short_name":
                    "EventAvailable",
                "is_server":
                    True,
                "port_type":
                    "pport",
                "operations": [{
                    "uuid": str(uuid.uuid4()),
                    "short_name": "SetEventAvailable",
                    "arguments": [{
                        "uuid": str(uuid.uuid4()),
                        "short_name": "AvailableStatus",
                        "IsType": False,
                        "TypeTref": "boolean",
                        "Direction": "IN",
                    }],
                    "possible_errors": ["E_OK", "E_NOT_OK"]
                }],
                "possible_errors": [{
                    "uuid": str(uuid.uuid4()),
                    "short_name": "E_OK",
                    "error_code": 0
                }, {
                    "uuid": str(uuid.uuid4()),
                    "short_name": "E_NOT_OK",
                    "error_code": 1
                }],
                "only_master":True
            }
            cs_interfaces.append(event_available_interface)

        if self.dem_general["DemSuppressionSupport"]["Value"] == "DEM_DTC_SUPPRESSION":
            dtc_suppression_interface = {
                "uuid":
                    str(uuid.uuid4()),
                "short_name":
                    "DTCSuppression",
                "is_server":
                    True,
                "port_type":
                    "pport",
                "operations": [{
                    "uuid": str(uuid.uuid4()),
                    "short_name": "GetDTCSuppression",
                    "arguments": [{
                        "uuid": str(uuid.uuid4()),
                        "short_name": "ClientID",
                        "IsType": False,
                        "TypeTref": "uint8",
                        "Direction": "IN",
                    }, {
                        "uuid": str(uuid.uuid4()),
                        "short_name": "SuppressionStatus",
                        "IsType": False,
                        "TypeTref": "boolean",
                        "Direction": "OUT",
                    }],
                    "possible_errors": ["E_OK", "E_NOT_OK", "DEM_PENDING", "DEM_WRONG_DTC", "DEM_WRONG_DTCORIGIN"]
                }, {
                    "uuid": str(uuid.uuid4()),
                    "short_name": "SetDTCSuppression",
                    "arguments": [{
                        "uuid": str(uuid.uuid4()),
                        "short_name": "ClientID",
                        "IsType": False,
                        "TypeTref": "uint8",
                        "Direction": "IN",
                    }, {
                        "uuid": str(uuid.uuid4()),
                        "short_name": "SuppressionStatus",
                        "IsType": False,
                        "TypeTref": "boolean",
                        "Direction": "IN",
                    }],
                    "possible_errors": ["E_OK", "E_NOT_OK", "DEM_WRONG_DTC", "DEM_WRONG_DTCORIGIN"]
                }],
                "possible_errors": [{
                    "uuid": str(uuid.uuid4()),
                    "short_name": "E_OK",
                    "error_code": 0
                }, {
                    "uuid": str(uuid.uuid4()),
                    "short_name": "E_NOT_OK",
                    "error_code": 1
                }, {
                    "uuid": str(uuid.uuid4()),
                    "short_name": "DEM_PENDING",
                    "error_code": 4
                }, {
                    "uuid": str(uuid.uuid4()),
                    "short_name": "DEM_WRONG_DTC",
                    "error_code": 8
                }, {
                    "uuid": str(uuid.uuid4()),
                    "short_name": "DEM_WRONG_DTCORIGIN",
                    "error_code": 9
                }],
                "only_master":True
            }
            cs_interfaces.append(dtc_suppression_interface)

        self.interfaces_append_data_services_data(cs_interfaces)
        return

    def interfaces_append_data_services_data(self, cs_interfaces: list):
        for data_elem_class_name, data_elem_class in self.dem_general.items():
            if "DefinitionRef" not in data_elem_class or data_elem_class[
                    "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemDataElementClass":
                continue
            monitor_data0 = False
            data = data_elem_class_name
            cs_data_elem_class_use_port_found = False
            for inner_data_elem_class in data_elem_class.values():
                if "DefinitionRef" not in inner_data_elem_class or inner_data_elem_class[
                        "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemDataElementClass/DemExternalCSDataElementClass":
                    continue

                if inner_data_elem_class["DemDataElementUsePort"]["Value"] == True:
                    cs_data_elem_class_use_port_found = True
                    if "DemDataElementProvideMonitorData" in inner_data_elem_class and inner_data_elem_class[
                                "DemDataElementProvideMonitorData"]["Value"] == True:
                        monitor_data0 = True
                else:
                    continue
            if cs_data_elem_class_use_port_found == False:
                continue

            arguments = [{
                "uuid": str(uuid.uuid4()),
                "short_name": "Data",
                "IsType": True,
                "TypeTref": "Dem_DataType_" + data,
                "Direction": "OUT",
            }]

            if monitor_data0 is True:
                arguments_monitor_data0 = {
                    "uuid": str(uuid.uuid4()),
                    "short_name": "monitorData0",
                    "IsType": True,
                    "TypeTref": "Dem_MonitorDataType",
                    "Direction": "IN",
                }
                arguments.append(arguments_monitor_data0)

            data_services_data_interface = {
                "uuid":
                    str(uuid.uuid4()),
                "short_name":
                    update_short_name_length("DataServices_" + data),
                "is_server":
                    True,
                "port_type":
                    "rport",
                "operations": [{
                    "uuid": str(uuid.uuid4()),
                    "short_name": "ReadData",
                    "arguments": arguments,
                    "possible_errors": ["E_OK", "E_NOT_OK"]
                }],
                "possible_errors": [{
                    "uuid": str(uuid.uuid4()),
                    "short_name": "E_OK",
                    "error_code": 0
                }, {
                    "uuid": str(uuid.uuid4()),
                    "short_name": "E_NOT_OK",
                    "error_code": 1
                }],
                "only_master":True
            }
            cs_interfaces.append(data_services_data_interface)

        return

    def get_debouncing_of_event_config_state(self):
        for _, event_parameter in self.dem_config_set.items():
            if "DefinitionRef" not in event_parameter or event_parameter[
                    "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemEventParameter":
                continue
            if "DemDebounceAlgorithmClass" not in event_parameter:
                continue
            for _, value in event_parameter["DemDebounceAlgorithmClass"].items():
                if "DefinitionRef" not in value:
                    continue
                if value["DefinitionRef"] == "/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemEventParameter/DemDebounceAlgorithmClass/DemDebounceCounterBased" or \
                        value["DefinitionRef"] == "/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemEventParameter/DemDebounceAlgorithmClass/DemDebounceTimeBase":
                    return True

        return False

    def get_clear_dtc_notification_config_state(self):
        state = False
        return
        for _, value in self.dem_general.items():
            if "DefinitionRef" not in value or value[
                    "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemEventMemorySet":
                continue
            for _, clear_dtc_notification in value.items():
                if "DefinitionRef" not in clear_dtc_notification or clear_dtc_notification[
                        "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemEventMemorySet/DemClearDTCNotification":
                    continue
                if "DemClearDtcNotificationFnc" not in clear_dtc_notification:
                    continue
                if clear_dtc_notification["DemClearDtcNotificationFnc"]["Value"] == "" or clear_dtc_notification[
                        "DemClearDtcNotificationFnc"]["Value"] == "<empty>" or clear_dtc_notification["DemClearDtcNotificationFnc"]["Value"] is None:
                    state = True
                    break
            return state

    def append_operation_for_interface(self, operation: dict, interface_name: str):
        cs_interfaces = self.client_server_interfaces
        for interface in cs_interfaces:
            if interface_name == interface["short_name"]:
                interface["operations"].append(operation)
        return

    def update_main_function_runnable(self):
        for rport in self.rports:
            #server_call_points
            if rport["interface_type"] == "client_server":
                for cs_interface in self.client_server_interfaces:
                    if rport["interface_ref"] == cs_interface["short_name"]:
                        for operation in cs_interface["operations"]:
                            self.main_function_runnable["server_call_points"].append({
                                'uuid':
                                    str(uuid.uuid4()),
                                "short_name":
                                    update_short_name_length("OpServerCallDem_MainFunction_" + operation["short_name"] +
                                                             "_" + rport["short_name"]),
                                "port_ref":
                                    rport["short_name"],
                                "interface_ref":
                                    cs_interface["short_name"],
                                "operation_ref":
                                    operation["short_name"],
                                "Applications":
                                    rport["Applications"]
                            })
        for pport in self.pports:
            #mode-switch-point
            if pport["interface_type"] == "mode_switch":
                for sw_interface in self.mode_switch_interfaces:
                    if pport["interface_ref"] == sw_interface["short_name"]:
                        self.main_function_runnable["mode_switch_points"].append({
                            'uuid':
                                str(uuid.uuid4()),
                            "short_name":
                                update_short_name_length("ModeSwitchPointDem_MainFunction_" +
                                                         sw_interface["short_name"] + "_" + sw_interface["short_name"]),
                            "port_ref":
                                pport["short_name"],
                            "interface_ref":
                                sw_interface["short_name"]
                        })

    def get_runnable_applications(self, cs_interface:dict) -> list:
        runnable_applications = []
        for pport in self.pports:
            if pport["interface_ref"] == cs_interface["short_name"]:
                for application in pport["Applications"]:
                    if application not in runnable_applications:
                        runnable_applications.append(application)
        return runnable_applications

    def update_server_runnables(self):
        for cs_interface in self.client_server_interfaces:
            if cs_interface["port_type"] == "rport":
                continue
            for operation in cs_interface["operations"]:
                self.server_runnables.append({
                    "uuid": str(uuid.uuid4()),
                    "short_name": operation["short_name"],
                    "symbol": "Dem_" + operation["short_name"],
                    "Applications": self.get_runnable_applications(cs_interface)
                })

    def update_invoked_events(self):
        for pport in self.pports:
            for cs_interface in self.client_server_interfaces:
                if pport["interface_ref"] == cs_interface["short_name"]:
                    for operation in cs_interface["operations"]:
                        self.operation_invoked_events.append({
                            "uuid":
                                str(uuid.uuid4()),
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
                                operation["short_name"],
                            "Applications":
                                pport["Applications"],
                        })

    def set_config_dict(self):
        self.config_module_dict = {
            "dem_package": {
                "autosar_uuid": str(uuid.uuid4()),
                "uuid": str(uuid.uuid4()),
                "short_name": "Dem_swc",
                "datatypes_pkg": self.data_types,
                "interfaces_pkg": {
                    "uuid": str(uuid.uuid4()),
                    "client_server": self.client_server_interfaces
                },
                "componenttypes_pkg_uuid": str(uuid.uuid4()),
                "componenttypes_pkg": {
                    "uuid": str(uuid.uuid4()),
                    "internal_behaviors_uuid": str(uuid.uuid4()),
                    "timing_event_uuid": str(uuid.uuid4()),
                    "pports": self.pports,
                    "rports": self.rports,
                    "main_function": self.main_function_runnable,
                    "server_runnables": self.server_runnables,
                    "operation_events": self.operation_invoked_events,
                },
                "Applications": self.application_list,
            }
        }
        return

    def get_config_dict(self):
        return self.config_module_dict


def get_config_dict(module_ecuc_cfg):
    dem_module_cfg = module_ecuc_cfg["Dem"] if "Dem" in module_ecuc_cfg else None
    if not dem_module_cfg:
        return {}
    dem_ecuc_cfg_dict = AutosarUtil.to_bsw_dict(dem_module_cfg)
    port_derive = PortDerive(dem_ecuc_cfg_dict)
    return port_derive.get_config_dict()
