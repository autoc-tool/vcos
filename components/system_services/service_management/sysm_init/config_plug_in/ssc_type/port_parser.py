#
# Copyright (C) 2015-2022 Lixiang Group Holding Limited
#
from . import filter
import hashlib
from basic_func_module.autosar_utils.autosar_util import AutosarUtil
import copy


# Functions
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


def get_key_name(key_name: str) -> str:
    key_name = key_name.split("/")[-1]
    return key_name


class PortDerive(object):

    def __init__(self, ecucs_autosar_dict=None):
        self.ecucs_autosar_dict = ecucs_autosar_dict
        self.init_ecum_config()
        self.update_data_types()
        # self.update_mode_declaration_to_date()
        self.update_main_function_runnable()
        self.update_server_runnables()
        self.update_invoked_events()
        self.config_module_dict = {}
        self.set_config_dict()

    def init_ecum_config(self):
        self.mode_declaration_groups = [{
            "short_name": "EcuM_Mode",  # 8.7.6.1
            "initial_value": "STARTUP",
            "modes": ["STARTUP", "RUN", "POST_RUN", "SLEEP", "SHUTDOWN"]
        }]
        self.data_types = {
            "elements": [
                {
                    "short_name": "EcuM_UserType",  # SWS_EcuM_91004
                    "category": "Type",
                    "sw_data_def": {
                        "data_constr_ref": "EcuM_UserType_Constr",
                        "IsType": False,
                        "implementation_data_type": "uint8",
                    }
                },
                {
                    "short_name": "EcuM_TimeType",  # SWS_EcuM_04102
                    "category": "Type",
                    "sw_data_def": {
                        "data_constr_ref": "EcuM_TimeType_Constr",
                        "IsType": False,
                        "implementation_data_type": "uint32",
                    }
                },
                {
                    "short_name": "EcuM_BootTargetType",
                    "category": "Type",
                    "sw_data_def": {
                        "compu_method_ref": "EcuM_BootTargetType",
                        "data_constr_ref": "EcuM_BootTargetType_Constr",
                        "IsType": False,
                        "implementation_data_type": "uint8",
                    }
                },
                {
                    "short_name": "EcuM_ShutdownCauseType",
                    "category": "Type",
                    "sw_data_def": {
                        "compu_method_ref": "EcuM_ShutdownCauseType",
                        "data_constr_ref": "EcuM_ShutdownCauseType_Constr",
                        "IsType": False,
                        "implementation_data_type": "uint8",
                    }
                },
                {
                    "short_name": "EcuM_ShutdownModeType",
                    "category": "Type",
                    "sw_data_def": {
                        "compu_method_ref": "EcuM_ShutdownModeType",
                        "data_constr_ref": "EcuM_ShutdownModeType_Constr",
                        "IsType": False,
                        "implementation_data_type": "uint16",
                    }
                },
                {
                    "short_name": "EcuM_ShutdownTargetType",
                    "category": "Type",
                    "sw_data_def": {
                        "compu_method_ref": "EcuM_ShutdownTargetType",
                        "data_constr_ref": "EcuM_ShutdownTargetType_Constr",
                        "IsType": False,
                        "implementation_data_type": "uint8",
                    }
                }
            ],
            "data_constrs": {
                "elements": [{
                    "short_name": "EcuM_UserType_Constr",
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": 0,
                            "upper_limit": 255,
                        }
                    }]
                }, {
                    "short_name": "EcuM_TimeType_Constr",
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": 0,
                            "upper_limit": 4294967295,
                        }
                    }]
                }, {
                    "short_name": "EcuM_BootTargetType_Constr",
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": 0,
                            "upper_limit": 2,
                        }
                    }]
                }, {
                    "short_name": "EcuM_ShutdownCauseType_Constr",
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": 0,
                            "upper_limit": 3,
                        }
                    }]
                }, {
                    "short_name": "EcuM_ShutdownModeType_Constr",
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": 0,
                            "upper_limit": 4294967295,
                        }
                    }]
                }, {
                    "short_name": "EcuM_ShutdownTargetType_Constr",
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
                        "EcuM_BootTargetType",
                    "category":
                        "TEXTTABLE",
                    "compu_scales": [{
                        "short_label": "ECUM_BOOT_TARGET_APP",
                        "lower_limit": 0,
                        "upper_limit": 0,
                    }, {
                        "short_label": "ECUM_BOOT_TARGET_OEM_BOOTLOADER",
                        "lower_limit": 1,
                        "upper_limit": 1,
                    }, {
                        "short_label": "ECUM_BOOT_TARGET_SYS_BOOTLOADER",
                        "lower_limit": 2,
                        "upper_limit": 2,
                    }]
                }, {
                    "short_name":
                        "EcuM_ShutdownCauseType",
                    "category":
                        "TEXTTABLE",
                    "compu_scales": [{
                        "short_label": "ECUM_CAUSE_UNKNOWN",
                        "lower_limit": 0,
                        "upper_limit": 0,
                    }, {
                        "short_label": "ECUM_CAUSE_ECU_STATE",
                        "lower_limit": 1,
                        "upper_limit": 1,
                    }, {
                        "short_label": "ECUM_CAUSE_WDGM",
                        "lower_limit": 2,
                        "upper_limit": 2,
                    }, {
                        "short_label": "ECUM_CAUSE_DCM",
                        "lower_limit": 3,
                        "upper_limit": 3,
                    }]
                }, {
                    "short_name": "EcuM_ShutdownModeType",
                    "category": "TEXTTABLE",
                    "compu_scales": []
                }, {
                    "short_name":
                        "EcuM_ShutdownTargetType",
                    "category":
                        "TEXTTABLE",
                    "compu_scales": [{
                        "short_label": "ECUM_SHUTDOWN_TARGET_SLEEP",
                        "lower_limit": 0,
                        "upper_limit": 0,
                    }, {
                        "short_label": "ECUM_SHUTDOWN_TARGET_RESET",
                        "lower_limit": 1,
                        "upper_limit": 1,
                    }, {
                        "short_label": "ECUM_SHUTDOWN_TARGET_OFF",
                        "lower_limit": 2,
                        "upper_limit": 2,
                    }]
                }],
            }
        }
        self.mode_switch_interfaces = [{
            "short_name": "EcuM_CurrentMode",  # 8.8.6.3
            "is_server": True,
            "mode_group": "EcuM_Mode",
            "prototype": "currentMode"
        }]
        self.sender_receiver_interfaces = []
        self.client_server_interfaces = [
            {
                "short_name":
                    "EcuM_ShutdownTarget",  # 8.8.1.2
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
                    "short_name": "GetLastShutdownTarget",
                    "arguments": [{
                        "short_name": "shutdownTarget",
                        "IsType": True,
                        "TypeTref": "EcuM_ShutdownTargetType",
                        "Direction": "OUT",
                    }, {
                        "short_name": "shutdownMode",
                        "IsType": True,
                        "TypeTref": "EcuM_ShutdownModeType",
                        "Direction": "OUT",
                    }],
                    "possible_errors": ["E_OK", "E_NOT_OK"]
                }, {
                    "short_name": "GetShutdownCause",
                    "arguments": [{
                        "short_name": "shutdownCause",
                        "IsType": True,
                        "TypeTref": "EcuM_ShutdownCauseType",
                        "Direction": "OUT",
                    }],
                    "possible_errors": ["E_OK", "E_NOT_OK"]
                }, {
                    "short_name": "GetShutdownTarget",
                    "arguments": [{
                        "short_name": "shutdownTarget",
                        "IsType": True,
                        "TypeTref": "EcuM_ShutdownTargetType",
                        "Direction": "OUT",
                    }, {
                        "short_name": "shutdownMode",
                        "IsType": True,
                        "TypeTref": "EcuM_ShutdownModeType",
                        "Direction": "OUT",
                    }],
                    "possible_errors": ["E_OK", "E_NOT_OK"]
                }, {
                    "short_name": "SelectShutdownCause",
                    "arguments": [{
                        "short_name": "shutdownCause",
                        "IsType": True,
                        "TypeTref": "EcuM_ShutdownCauseType",
                        "Direction": "IN",
                    }],
                    "possible_errors": ["E_OK", "E_NOT_OK"]
                }, {
                    "short_name": "SelectShutdownTarget",
                    "arguments": [{
                        "short_name": "shutdownTarget",
                        "IsType": True,
                        "TypeTref": "EcuM_ShutdownTargetType",
                        "Direction": "IN",
                    }, {
                        "short_name": "shutdownMode",
                        "IsType": True,
                        "TypeTref": "EcuM_ShutdownModeType",
                        "Direction": "IN",
                    }],
                    "possible_errors": ["E_OK", "E_NOT_OK"]
                }]
            },
            {
                "short_name":
                    "EcuM_BootTarget",  # 8.8.2.2
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
                    "short_name": "GetBootTarget",
                    "arguments": [{
                        "short_name": "target",
                        "IsType": True,
                        "TypeTref": "EcuM_BootTargetType",
                        "Direction": "OUT",
                    }],
                    "possible_errors": ["E_OK"]
                }, {
                    "short_name": "SelectBootTarget",
                    "arguments": [{
                        "short_name": "target",
                        "IsType": True,
                        "TypeTref": "EcuM_BootTargetType",
                        "Direction": "IN",
                    }],
                    "possible_errors": ["E_OK", "E_NOT_OK"]
                }]
            },
            {
                "short_name":
                    "EcuM_Time",  # 8.8.4.3
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
                    "short_name": "GetCurrentTime",
                    "arguments": [{
                        "short_name": "time",
                        "IsType": True,
                        "TypeTref": "EcuM_TimeType",
                        "Direction": "OUT",
                    }],
                    "possible_errors": ["E_OK", "E_NOT_OK"]
                }, {
                    "short_name": "GetWakeupTime",
                    "arguments": [{
                        "short_name": "time",
                        "IsType": True,
                        "TypeTref": "EcuM_TimeType",
                        "Direction": "OUT",
                    }],
                    "possible_errors": ["E_OK", "E_NOT_OK"]
                }]
            },
            {
                "short_name":
                    "EcuM_StateRequest",  # 8.8.5.3
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
                "operations": [
                    {
                        "short_name": "ReleasePOSTRUN",
                        "possible_errors": ["E_OK", "E_NOT_OK"]
                    },
                    {
                        "short_name": "ReleaseRUN",
                        "possible_errors": ["E_OK", "E_NOT_OK"]
                    },
                    {
                        "short_name": "RequestPOSTRUN",
                        "possible_errors": ["E_OK", "E_NOT_OK"]
                    },
                    {
                        "short_name": "RequestRUN",
                        "possible_errors": ["E_OK", "E_NOT_OK"]
                    },
                ]
            },
        ]
        self.pports = [
            {
                "short_name": "time",  # SWS_EcuM_04113
                "interface_type": "client_server",
                "interface_ref": "EcuM_Time",
                "argument": None
            },
            {
                "short_name": "currentMode",  # SWS_EcuM_04112
                "interface_type": "mode_switch",
                "interface_ref": "EcuM_CurrentMode",
                "argument": None
            }
        ]
        self.rports = []
        self.main_function_runnable = {
            "period": self.ecucs_autosar_dict["ActiveEcuC"]["EcuM"]["EcuMGeneral"]["EcuMMainFunctionPeriod"]["Value"],
            "mode_switch_points": [],  # mode_switch p
            "server_call_points": [],  # cs r
            "data_send_points": [],  # sr p
            "data_receive_points": []  # sr r
        }
        self.server_runnables = []
        self.operation_invoked_events = []

    def update_data_types(self):
        flex_element = self.ecucs_autosar_dict["ActiveEcuC"]["EcuM"]["EcuMConfiguration"]["EcuMFlexConfiguration"]
        common_element = self.ecucs_autosar_dict["ActiveEcuC"]["EcuM"]["EcuMConfiguration"]["EcuMCommonConfiguration"]

        # data types
        for element_name, element_value in list(common_element.items()):
            if element_name != "DefinitionRef" and get_key_name(element_value["DefinitionRef"]) == "EcuMSleepMode":
                sleep_element = {
                    "short_label": element_name,
                    "lower_limit": element_value["EcuMSleepModeId"]["Value"],
                    "upper_limit": element_value["EcuMSleepModeId"]["Value"],
                }
                self.data_types["compu_methods"]["elements"][-2]["compu_scales"].append(sleep_element)

        # Client-Server-interfaces
        if self.ecucs_autosar_dict["ActiveEcuC"]["EcuM"]["EcuMFlexGeneral"]["EcuMAlarmClockPresent"]["Value"]:
            ecum_alarmclock = {
                "short_name":
                    "EcuM_AlarmClock",  # 8.8.3.2
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
                }, {
                    "short_name": "ECUM_E_EARLIER_ACTIVE",
                    "error_code": 3
                }, {
                    "short_name": "ECUM_E_PAST",
                    "error_code": 4
                }, {
                    "short_name": "ECUM_E_NOT_ACTIVE",
                    "error_code": 5
                }],
                "operations": [{
                    "short_name": "AbortWakeupAlarm",
                    "possible_errors": ["E_OK", "E_NOT_OK", "ECUM_E_NOT_ACTIVE"]
                }, {
                    "short_name": "SetAbsWakeupAlarm",
                    "arguments": [{
                        "short_name": "time",
                        "IsType": True,
                        "TypeTref": "EcuM_TimeType",
                        "Direction": "IN",
                    }],
                    "possible_errors": ["E_OK", "E_NOT_OK", "ECUM_E_EARLIER_ACTIVE", "ECUM_E_PAST"]
                }, {
                    "short_name": "SetClock",
                    "arguments": [{
                        "short_name": "time",
                        "IsType": True,
                        "TypeTref": "EcuM_TimeType",
                        "Direction": "IN",
                    }],
                    "possible_errors": ["E_OK", "E_NOT_OK"]
                }, {
                    "short_name": "SetRelWakeupAlarm",
                    "arguments": [{
                        "short_name": "time",
                        "IsType": True,
                        "TypeTref": "EcuM_TimeType",
                        "Direction": "IN",
                    }],
                    "possible_errors": ["E_OK", "E_NOT_OK", "ECUM_E_EARLIER_ACTIVE"]
                }]
            }
            self.client_server_interfaces.append(ecum_alarmclock)

            # P-Ports
            for element_name, element_value in list(flex_element.items()):
                if element_name != "DefinitionRef" and get_key_name(
                        element_value["DefinitionRef"]) == "EcuMAlarmClock":  # Alarm Clock Name
                    user_name = get_key_name(element_value["EcuMAlarmClockUser"]["ValueRef"])
                    alarmclock = {
                        "short_name": "AlarmClock_" + element_name,  # SWS_EcuM_03017
                        "interface_type": "client_server",
                        "interface_ref": "EcuM_AlarmClock",
                        "argument": {
                            "type": "EcuM_UserType",
                            "value": flex_element[user_name]["EcuMFlexUser"]["Value"]
                        }
                    }
                    self.pports.append(alarmclock)

        for element_name, element_value in list(flex_element.items()):
            if element_name != "DefinitionRef" and get_key_name(
                    element_value["DefinitionRef"]) == "EcuMFlexUserConfig":  # UserName
                shutdowntarget = {
                    "short_name": "ShutdownTarget_" + element_name,  # SWS_EcuM_04111
                    "interface_type": "client_server",
                    "interface_ref": "EcuM_ShutdownTarget",
                    "argument": None
                }
                self.pports.append(shutdowntarget)

                boottarget = {
                    "short_name": "BootTarget_" + element_name,  # SWS_EcuM_04111
                    "interface_type": "client_server",
                    "interface_ref": "EcuM_BootTarget",
                    "argument": None
                }
                self.pports.append(boottarget)

                staterequest = {
                    "short_name": "StateRequest_" + element_name,  # SWS_EcuM_04135
                    "interface_type": "client_server",
                    "interface_ref": "EcuM_StateRequest",
                    "argument": {
                        "type": "EcuM_UserType",
                        "value": element_value["EcuMFlexUser"]["Value"]
                    }
                }
                self.pports.append(staterequest)

            # Sender-Receiver-interfaces

            # data types
            elif element_name != "DefinitionRef" and get_key_name(element_value["DefinitionRef"]) == "EcuMResetMode":
                reset_element = {
                    "short_label": "Conf_" + element_name,
                    "lower_limit": 256 + element_value["EcuMResetModeId"]["Value"],
                    "upper_limit": 256 + element_value["EcuMResetModeId"]["Value"],
                }
                self.data_types["compu_methods"]["elements"][-2]["compu_scales"].append(reset_element)

    def update_main_function_runnable(self):
        for pport in self.pports:
            #mode-switch-points
            if pport["interface_type"] == "mode_switch":
                for sw_interface in self.mode_switch_interfaces:
                    if pport["interface_ref"] == sw_interface["short_name"]:
                        self.main_function_runnable["mode_switch_points"].append({
                            "short_name":
                                update_short_name_length("SWITCH_" + pport["short_name"] + "_" +
                                                         sw_interface["mode_group"]),
                            "port_ref":
                                pport["short_name"],
                            "interface_ref":
                                sw_interface["short_name"],
                            "prototype_ref":
                                sw_interface["prototype"]
                        })
            #sender_receiver-points
            elif pport["interface_type"] == "sender_receiver":
                for sr_interface in self.sender_receiver_interfaces:
                    if pport["interface_ref"] == sr_interface["short_name"]:
                        for main_function in self.main_function_runnable:
                            for data in sr_interface["data_elements"]:
                                main_function["data_send_points"].append({
                                    "short_name":
                                        update_short_name_length("SEND_" + pport["short_name"] + "_" + data["type"]),
                                    "port_ref":
                                        pport["short_name"],
                                    "interface_ref":
                                        sr_interface["short_name"],
                                    "data":
                                        data
                                })

    def update_server_runnables(self):
        used_interfaces = []
        # 找到pport中包含的所有interface
        for pport in self.pports:
            if pport["interface_ref"] not in used_interfaces:
                used_interfaces.append(pport["interface_ref"])

        for cs_interface in self.client_server_interfaces:
            if cs_interface["port_type"] == "rport":
                continue
            if cs_interface["short_name"] in used_interfaces:
                for operation in cs_interface["operations"]:
                    symbol = "EcuM_" + operation["short_name"]
                    if "POST" in operation["short_name"]:
                        symbol = symbol[:-3] + "_RUN"
                    element = {"short_name": operation["short_name"], "symbol": symbol}
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
            "ecum_package": {
                "short_name": "EcuM_swc",
                "mode_declaration": self.mode_declaration_groups,
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
    ecum_module_cfg = module_ecuc_cfg["EcuM"] if "EcuM" in module_ecuc_cfg else None
    if not ecum_module_cfg:
        return {}
    ecum_ecuc_cfg_dict = AutosarUtil.to_bsw_dict(ecum_module_cfg)
    # tmp add
    ######################################################################################
    dict_config = copy.deepcopy(ecum_ecuc_cfg_dict["ArPackages"])

    # child_convert(dict_config["ActiveEcuC"]["ComM"]["ComMConfigSet"], "ComMConfigSet",
    #                    dict_config["ActiveEcuC"]["ComM"], "ComM")
    # arxml_dict_to_dict(dict_config["ActiveEcuC"]["ComM"]["ComMConfigSet"], "ComMConfigSet",
    #                    dict_config["ActiveEcuC"]["ComM"], "ComM")
    ######################################################################################
    # print(dict_config)
    ecum_ecuc_dict = filter.get_template_input_parameter_dict(dict_config)
    port_derive = PortDerive(ecum_ecuc_dict)
    return port_derive.get_config_dict()
