#
# Copyright (C) 2015-2022 Lixiang Group Holding Limited
#

import hashlib
from basic_func_module.autosar_utils.autosar_util import AutosarUtil
import copy

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


class BswMPortDerive(object):

    def __init__(self, ecucs_autosar_dict=None):
        self.ecucs_autosar_dict = ecucs_autosar_dict
        self.init_bswm_config()
        self.update_mode_declaration_groups()
        self.update_interface()
        self.update_port()

        self.update_server_runnables()
        self.update_invoked_events()
        self.update_main_function_runnable()
        #update 多核信息 以生成多个 rte文件

        #最后一步 匹配信息
        self.config_module_dict = {}
        self.set_config_dict()

    def init_bswm_config(self):
        self.Applications = []
        self.mode_declaration_groups = []
        self.data_types = {
            "elements": [

            ],
            "data_constrs": {
                
            },
            "compu_methods": {
                
            }
        }
        self.mode_switch_interfaces = [
            
        ]
        self.client_server_interfaces = [

        ]
        self.sender_reciver_interfaces = []
        self.pports = []
        self.rports = []
        ## 单独处理notification port
        self.notificationport = []
        self.main_function_runnable = {
            "period": "0.01",
            "mode_switch_points": [],
            "server_call_points": [],
            "data_send_points": [],
            "data_receive_points": []
        }
        self.server_runnables = []
        self.operation_invoked_events = []

    def update_mode_declaration_groups(self):
        bswm_ecuc_arxml_copy = copy.deepcopy(self.ecucs_autosar_dict)
        del bswm_ecuc_arxml_copy["ArPackages"]["ActiveEcuC"]["BswM"]['DefinitionRef']
        del bswm_ecuc_arxml_copy["ArPackages"]["ActiveEcuC"]["BswM"]["BswMConfig"]['DefinitionRef']
        bswm_group_mode_dict = bswm_ecuc_arxml_copy["ArPackages"]["ActiveEcuC"]["BswM"]["BswMConfig"]
        for key,value in bswm_group_mode_dict.items():
            temp_mode_declaration_groups = {}
            if "BswMRteModeDclGroup" in value["DefinitionRef"]:
                ## find the group
                temp_mode_declaration_groups["name"] = key
                temp_mode_declaration_groups["initial_mode_ref"] = "/AUTOSAR/BswM_swc/"+value["BswMInitMode"]["ValueRef"].split("/")[-2] + "/" +value["BswMInitMode"]["ValueRef"].split("/")[-1]
                del bswm_group_mode_dict[key]['DefinitionRef']
                temp1 = {}
                cnt = 0
                for key1,value1 in value.items():
                    if "BswMRteModeDclGroup/BswMRteMode" in value1["DefinitionRef"]:
                        temp1[key1] = value1["BswMModePhysicalValue"]["Value"]
                        cnt = cnt+1
                temp_mode_declaration_groups["TRANSITION"] = cnt
                temp_mode_declaration_groups["mode_value"] = temp1

                self.mode_declaration_groups.append(temp_mode_declaration_groups)

    def update_interface(self):
        return
        # bswm_ecuc_arxml_copy = copy.deepcopy(self.ecucs_autosar_dict)
        # del bswm_ecuc_arxml_copy["ArPackages"]["ActiveEcuC"]["BswM"]['DefinitionRef']
        # del bswm_ecuc_arxml_copy["ArPackages"]["ActiveEcuC"]["BswM"]["BswMConfig"]['DefinitionRef']
        # bswm_arbitration_dict = bswm_ecuc_arxml_copy["ArPackages"]["ActiveEcuC"]["BswM"]["BswMConfig"]["BswMArbitration"]
        # del bswm_arbitration_dict['DefinitionRef']
        # for key,value in bswm_arbitration_dict.items():
        #     temp_interface = {}
        #     if "BswM/BswMConfig/BswMArbitration/BswMModeRequestPort" in value["DefinitionRef"]:
        #         ## find the group
        #         del bswm_arbitration_dict[key]['DefinitionRef']
        #         for key1,value1 in value.items():
        #             if "BswMConfig/BswMArbitration/BswMModeRequestPort/BswMModeRequestSource" in value1["DefinitionRef"]:
        #                 for key2,value2 in value1.items():
        #                     if (key2 == "DefinitionRef"):
        #                         continue
        #                     if "BswMSwcModeRequest" in value2["DefinitionRef"]:
        #                         interface_ref = value1["BswMSwcModeRequest"]["BswMSwcModeRequestVariableDataPrototypeRef"]["ValueRef"]
        #                         temp_interface["sr_inf_name"] = "BswM_SRI_"+interface_ref.split("/")[-2]+"_"+interface_ref.split("/")[-1]
        #                         temp_interface["data_elements_name"] = "requestModeType"
        #                         temp_interface["type_ref"] = interface_ref
        #                         self.sender_reciver_interfaces.append(temp_interface)
                        
  

    def update_server_runnables(self):
        bswm_ecuc_arxml_copy = copy.deepcopy(self.ecucs_autosar_dict)
        del bswm_ecuc_arxml_copy["ArPackages"]["ActiveEcuC"]["BswM"]['DefinitionRef']
        del bswm_ecuc_arxml_copy["ArPackages"]["ActiveEcuC"]["BswM"]["BswMConfig"]['DefinitionRef']
        bswm_group_mode_dict = bswm_ecuc_arxml_copy["ArPackages"]["ActiveEcuC"]["BswM"]["BswMConfig"]

        for cs_interface in self.client_server_interfaces:
            if cs_interface["port_type"] == "rport":
                continue
            for operation in cs_interface["operations"]:
                self.server_runnables.append({
                    "short_name": operation["short_name"],
                    "symbol": "BswM_" + operation["short_name"]
                })

    def update_main_function_runnable(self):
        period = self.ecucs_autosar_dict['ArPackages']['ActiveEcuC']['BswM']['BswMGeneral']['BswMMainFunctionPeriod']['Value']
        self.main_function_runnable['period'] = period
        return

    def update_invoked_events(self):
        return

    def update_port(self):
        bswm_ecuc_arxml_copy = copy.deepcopy(self.ecucs_autosar_dict)
        del bswm_ecuc_arxml_copy["ArPackages"]["ActiveEcuC"]["BswM"]['DefinitionRef']
        del bswm_ecuc_arxml_copy["ArPackages"]["ActiveEcuC"]["BswM"]["BswMConfig"]['DefinitionRef']
        bswm_arbitration_dict = bswm_ecuc_arxml_copy["ArPackages"]["ActiveEcuC"]["BswM"]["BswMConfig"]["BswMArbitration"]
        del bswm_arbitration_dict['DefinitionRef']
        bswm_modecontrol_dict = bswm_ecuc_arxml_copy["ArPackages"]["ActiveEcuC"]["BswM"]["BswMConfig"]["BswMModeControl"]
        for key,value in bswm_arbitration_dict.items():
            if "BswM/BswMConfig/BswMArbitration/BswMModeRequestPort" in value["DefinitionRef"]:
                del bswm_arbitration_dict[key]['DefinitionRef']
                for key1,value1 in value.items():
                    if "BswMConfig/BswMArbitration/BswMModeRequestPort/BswMModeRequestSource" in value1["DefinitionRef"]:  
                        for key2,value2 in value1.items():
                            if (key2 == "DefinitionRef"):
                                continue
                            if "BswMSwcModeRequest" in value2["DefinitionRef"]:
                                rport_temp = {}
                                rport_temp["name"] = "Request_" + key
                                interface_ref = value1["BswMSwcModeRequest"]["BswMSwcModeRequestVariableDataPrototypeRef"]["ValueRef"]
                                rport_temp["init_value"] = value["BswMModeInitValue"]["BswMBswModeInitValue"]["Value"]
                                rport_temp["data_element_ref"] = interface_ref
                                rport_temp["require_interface_ref"] = interface_ref.rpartition('/')[0]
                                self.rports.append(rport_temp)
                            if "BswMSwcModeNotification" in value2["DefinitionRef"]:
                                notify_port_temp = {}
                                notify_port_temp["name"] = "Notification" + key
                                interface_ref = value1["BswMSwcModeNotification"]["BswMSwcModeNotificationModeDeclarationGroupPrototypeRef"]["ValueRef"]
                                notify_port_temp["init_value"] = value["BswMModeInitValue"]["BswMBswModeInitValue"]["Value"]
                                notify_port_temp["require_interface_ref"] = interface_ref.rpartition('/')[0]
                                notify_port_temp["modegroup_name"] = interface_ref.split('/')[-1][10:]
                                self.notificationport.append(notify_port_temp)

        # 判断bswm swicth mode 是否配置port
        del bswm_modecontrol_dict['DefinitionRef']
        for key,value in bswm_modecontrol_dict.items():
            if "BswM/BswMConfig/BswMModeControl/BswMSwitchPort" in value["DefinitionRef"]:
                pport_temp = {}
                pport_temp["name"] = "Switch_" + key
                pport_temp["admin_data_sdx_ref"] = '/ActiveEcuC/BswM/BswMConfig/BswMModeControl/' + key
                pport_temp["provide_interface_ref"] = value["BswMModeSwitchInterfaceRef"]["ValueRef"]
                pport_temp["modegroup_name"] = value["BswMModeSwitchInterfaceRef"]["ValueRef"].split('/')[-1][9:]
                self.pports.append(pport_temp)
        return

    def set_config_dict(self):
        self.config_module_dict = {
            "bswm_package": {
                "short_name": "BswM_swc",
                "mode_declaration": self.mode_declaration_groups,
                "datatypes_pkg": self.data_types,
                "interfaces_pkg": {
                    "mode_switch": self.mode_switch_interfaces,
                    "client_server": self.client_server_interfaces,
                    "sender_reciver":self.sender_reciver_interfaces,
                },
                "componenttypes_pkg": {
                    "pports": self.pports,
                    "rports": self.rports,
                    "notifyports": self.notificationport,
                    "main_function": self.main_function_runnable,
                    "server_runnables": self.server_runnables,
                    "operation_events": self.operation_invoked_events
                },
                "Applications": self.Applications,
            }
        }

    def get_config_dict(self):
        return self.config_module_dict


def get_config_dict(module_ecuc_cfg):

    bswm_module_cfg = module_ecuc_cfg["BswM"] if "BswM" in module_ecuc_cfg else None
    if not bswm_module_cfg:
        return {}
    bswm_ecuc_cfg_dict = AutosarUtil.to_bsw_dict(bswm_module_cfg)
    # print(bswm_ecuc_cfg_dict)
    # bswm_ecuc_dict = filter.get_template_input_parameter_dict(bswm_ecuc_cfg_dict)

    port_derive = BswMPortDerive(bswm_ecuc_cfg_dict)
    # with open('BswM_swc.json', 'w') as f:
    #     json.dump(port_derive.get_config_dict(), f, indent=4)
    return port_derive.get_config_dict()
