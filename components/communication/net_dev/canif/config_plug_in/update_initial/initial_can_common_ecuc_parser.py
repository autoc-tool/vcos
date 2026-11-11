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
from basic_func_module.base_logger import logger

AUTO_DERIVED_FLAG = "AutoDerived"

class InitialCanCommonEcucParser():
    """
    All Can module page jump, display, save logic code.
    """
    net_to_core_dict = {}
    resourcem_appeared_multicore_flag = False
    
    @classmethod
    def reset_net_to_core_dict(cls):
        cls.net_to_core_dict = {}
    
    @classmethod
    def set_net_to_core_dict(cls, net_to_core_dict):
        if cls.net_to_core_dict is not None:
            cls.net_to_core_dict = net_to_core_dict
        else:
            cls.net_to_core_dict = {}
            
    @classmethod
    def set_resourcem_appeared_multicore_flag(cls, flag: bool):
        cls.resourcem_appeared_multicore_flag = flag
    
    @classmethod
    def set_dbc_to_core_dict(cls, net_to_core_dict: dict, cfg_dict: dict):
        dbc_list = []
        ldf_list = []
        if "dbc" in cfg_dict.keys():
            dbc_list = cfg_dict["dbc"]
        if "ldf" in cfg_dict.keys():
            ldf_list = cfg_dict["ldf"]
        for i in range(len(dbc_list)):
            if cls.resourcem_appeared_multicore_flag == False:
                dbc_list[i]["coreId"] = None
            else:
                dbc_list[i]["coreId"] = 0
                NetWorkName = dbc_list[i]['attributes']['network']['define_value']['DBName']
                if NetWorkName in net_to_core_dict.keys():
                    dbc_list[i]["coreId"] = net_to_core_dict[NetWorkName]
        for i in range(len(ldf_list)):
            if cls.resourcem_appeared_multicore_flag == False:
                ldf_list[i]["coreId"] = None
            else:
                ldf_list[i]["coreId"] = 0
    
    @classmethod
    def get_can_ctrl_of_can_ecuc_arxml_dict(cls, can_dict: dict):
        can_configset_item_list = getattr(getattr(can_dict.find("/ActiveEcuC/Can/CanConfigSet"), "SUB_CONTAINERS", None), "ECUC_CONTAINER_VALUE", [])
        all_can_ctrl_name_dict = {}
        for _item in can_configset_item_list:
            if get_definition_ref(_item).split("/")[-1] == "CanController":
                can_ctrl_name = _item.SHORT_NAME.valueOf_
                can_ctrl_name = can_ctrl_name.split("/")[-1]
                all_can_ctrl_name_dict[can_ctrl_name] = 0
                       
        return all_can_ctrl_name_dict
        
    @classmethod
    def parser_net_and_core_mapping_by_resourcem_ecuc_arxml(cls, resourcem_dict: dict, can_dict: dict):
        resourcem_mcal_config_dict = {}
        resourcem_nodes = resourcem_dict.find_nodes("ECUC-MODULE-CONFIGURATION-VALUES")
        if resourcem_nodes:
            tmp_resourcem_mcal_config_list = getattr(getattr(resourcem_nodes[0], "CONTAINERS", None), "ECUC_CONTAINER_VALUE", [])
            if tmp_resourcem_mcal_config_list:
                resourcem_mcal_config_dict = tmp_resourcem_mcal_config_list[0]
       
        core_to_net_dict = {}
        resourcem_appeared_core_cnt_set = set()
        cls.set_resourcem_appeared_multicore_flag(False)
        
        aim_ecuc_container_value_list = getattr(getattr(resourcem_mcal_config_dict, "SUB_CONTAINERS", None), "ECUC_CONTAINER_VALUE", [])
        for _item in aim_ecuc_container_value_list:
            if get_definition_ref(_item).split("/")[-1] in ["ResourceMMcalCore", "ResMMcalCore"]:
                if hasattr(_item.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE[0], "VALUE") and \
                    hasattr(_item.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE[0].VALUE, "valueOf_"):
                    ResourceMCoreID = _item.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE[0].VALUE.valueOf_
                    if ResourceMCoreID != "":
                        ctrl_chnl_list = []
                        _aim_ecuc_container_value_list = getattr(getattr(_item, "SUB_CONTAINERS", None), "ECUC_CONTAINER_VALUE", [])
                        for _mcal_allocation_item in _aim_ecuc_container_value_list:
                            if get_definition_ref(_mcal_allocation_item).split("/")[-1] in ["ResourceMAllocation", "ResMAllocation"]:
                                ResourceMModuleName = ""
                                ResourceMResourceRef = ""
                                if hasattr(_mcal_allocation_item.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE[0], "VALUE") and \
                                    hasattr(_mcal_allocation_item.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE[0].VALUE, "valueOf_"):
                                    ResourceMModuleName =  _mcal_allocation_item.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE[0].VALUE.valueOf_
                                if hasattr(_mcal_allocation_item.REFERENCE_VALUES.ECUC_REFERENCE_VALUE[0], "VALUE_REF") and \
                                    hasattr(_mcal_allocation_item.REFERENCE_VALUES.ECUC_REFERENCE_VALUE[0].VALUE_REF, "valueOf_"):
                                    ResourceMResourceRef = _mcal_allocation_item.REFERENCE_VALUES.ECUC_REFERENCE_VALUE[0].VALUE_REF.valueOf_
                                if ResourceMModuleName != "" and ResourceMResourceRef != "":
                                    cur_core_ctrl_name = ResourceMResourceRef.split("/")[-1]
                                    if ResourceMModuleName == "CAN":                            
                                        ctrl_chnl_list.append(cur_core_ctrl_name)
                                        resourcem_appeared_core_cnt_set.add(ResourceMCoreID)
                                                
                        coreid = int(ResourceMCoreID[4:])
                        core_to_net_dict[ResourceMCoreID] = {"ctrl_chnl_list": ctrl_chnl_list, "coreid": coreid}
                
        all_can_ctrl_name_appeared_list = []
        if len(core_to_net_dict) > 0:
            for _core_name, _core_attr in core_to_net_dict.items():
                for key, value in _core_attr.items():
                    if key == "ctrl_chnl_list":
                        all_can_ctrl_name_appeared_list = all_can_ctrl_name_appeared_list + value
            
        all_can_ctrl_name_dict = cls.get_can_ctrl_of_can_ecuc_arxml_dict(can_dict)
            
        for _ctrl_name in all_can_ctrl_name_dict.keys():
            if _ctrl_name not in all_can_ctrl_name_appeared_list:
                if "CORE0" in core_to_net_dict.keys():
                    core_to_net_dict["CORE0"]["ctrl_chnl_list"].append(_ctrl_name) 
                else:
                    core_to_net_dict["CORE0"] = {}
                    core_to_net_dict["CORE0"]["ctrl_chnl_list"] = [_ctrl_name]
                    core_to_net_dict["CORE0"]["coreid"] = 0
                
                
        net_to_core_dict = {}
        if len(core_to_net_dict) > 0:
            for _core_name, _core_attr in core_to_net_dict.items():
                for _ctrl_name in _core_attr["ctrl_chnl_list"]:
                    net_to_core_dict[_ctrl_name] = core_to_net_dict[_core_name]["coreid"]
        
        core_cnt_found = 0 
        for key, value in core_to_net_dict.items():
            if len(value["ctrl_chnl_list"]) > 0:
                core_cnt_found = core_cnt_found + 1
                
        if core_cnt_found > 1:
            cls.set_resourcem_appeared_multicore_flag(True)
        else:
            cls.set_resourcem_appeared_multicore_flag(False)
            
        return net_to_core_dict
    
    @classmethod
    def parser_net_and_core_mapping_by_resourcemanager_ecuc_arxml(cls, resourcem_dict: dict, can_dict: dict):
        resourcem_mcal_config_list = []
        resourcem_nodes = resourcem_dict.find_nodes("ECUC-MODULE-CONFIGURATION-VALUES")
        if resourcem_nodes:
            tmp_resourcem_mcal_config_list = getattr(getattr(resourcem_nodes[0], "CONTAINERS", None), "ECUC_CONTAINER_VALUE", [])
            for idx, mcal_core_item in enumerate(tmp_resourcem_mcal_config_list):
                if get_definition_ref(mcal_core_item).split("/")[-1] == "ResMMcalCore":
                    resourcem_mcal_config_list.append(mcal_core_item)
       
        core_to_net_dict = {}
        resourcem_appeared_core_cnt_set = set()
        cls.set_resourcem_appeared_multicore_flag(False)
        
        for _item in resourcem_mcal_config_list:
            if get_definition_ref(_item).split("/")[-1] in ["ResourceMMcalCore", "ResMMcalCore"]:
                if hasattr(_item.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE[0], "VALUE") and \
                    hasattr(_item.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE[0].VALUE, "valueOf_"):
                    ResourceMCoreID = _item.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE[0].VALUE.valueOf_
                    if ResourceMCoreID != "":
                        ctrl_chnl_list = []
                        _aim_ecuc_container_value_list = getattr(getattr(_item, "SUB_CONTAINERS", None), "ECUC_CONTAINER_VALUE", [])
                        for _mcal_allocation_item in _aim_ecuc_container_value_list:
                            if get_definition_ref(_mcal_allocation_item).split("/")[-1] in ["ResourceMAllocation", "ResMAllocation"]:
                                ResourceMModuleName = ""
                                ResourceMResourceRef = ""
                                if hasattr(_mcal_allocation_item.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE[0], "VALUE") and \
                                    hasattr(_mcal_allocation_item.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE[0].VALUE, "valueOf_"):
                                    ResourceMModuleName =  _mcal_allocation_item.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE[0].VALUE.valueOf_
                                if hasattr(_mcal_allocation_item.REFERENCE_VALUES.ECUC_REFERENCE_VALUE[0], "VALUE_REF") and \
                                    hasattr(_mcal_allocation_item.REFERENCE_VALUES.ECUC_REFERENCE_VALUE[0].VALUE_REF, "valueOf_"):
                                    ResourceMResourceRef = _mcal_allocation_item.REFERENCE_VALUES.ECUC_REFERENCE_VALUE[0].VALUE_REF.valueOf_
                                if ResourceMModuleName != "" and ResourceMResourceRef != "":
                                    cur_core_ctrl_name = ResourceMResourceRef.split("/")[-1]
                                    if ResourceMModuleName == "CAN":                            
                                        ctrl_chnl_list.append(cur_core_ctrl_name)
                                        resourcem_appeared_core_cnt_set.add(ResourceMCoreID)
                                                
                        coreid = int(ResourceMCoreID[4:])
                        core_to_net_dict[ResourceMCoreID] = {"ctrl_chnl_list": ctrl_chnl_list, "coreid": coreid}
                
        all_can_ctrl_name_appeared_list = []
        if len(core_to_net_dict) > 0:
            for _core_name, _core_attr in core_to_net_dict.items():
                for key, value in _core_attr.items():
                    if key == "ctrl_chnl_list":
                        all_can_ctrl_name_appeared_list = all_can_ctrl_name_appeared_list + value
            
        all_can_ctrl_name_dict = cls.get_can_ctrl_of_can_ecuc_arxml_dict(can_dict)
            
        for _ctrl_name in all_can_ctrl_name_dict.keys():
            if _ctrl_name not in all_can_ctrl_name_appeared_list:
                if "CORE0" in core_to_net_dict.keys():
                    core_to_net_dict["CORE0"]["ctrl_chnl_list"].append(_ctrl_name) 
                else:
                    core_to_net_dict["CORE0"] = {}
                    core_to_net_dict["CORE0"]["ctrl_chnl_list"] = [_ctrl_name]
                    core_to_net_dict["CORE0"]["coreid"] = 0
                
                
        net_to_core_dict = {}
        if len(core_to_net_dict) > 0:
            for _core_name, _core_attr in core_to_net_dict.items():
                for _ctrl_name in _core_attr["ctrl_chnl_list"]:
                    net_to_core_dict[_ctrl_name] = core_to_net_dict[_core_name]["coreid"]
        
        core_cnt_found = 0 
        for key, value in core_to_net_dict.items():
            if len(value["ctrl_chnl_list"]) > 0:
                core_cnt_found = core_cnt_found + 1
                
        if core_cnt_found > 1:
            cls.set_resourcem_appeared_multicore_flag(True)
        else:
            cls.set_resourcem_appeared_multicore_flag(False)
            
        return net_to_core_dict
    
    @classmethod
    def get_coreid_of_net(cls, net_name: str):
        core_id: int = 0
        if (cls.net_to_core_dict is not None) and (cls.net_to_core_dict != {}) and (net_name in cls.net_to_core_dict.keys()):
            core_id = cls.net_to_core_dict[net_name]
        return core_id
    
    @classmethod
    def update_dbc_coreInfo_by_resourcem(cls, ecuc_autosar_dict:dict, cfg_dict: dict):
        resourcem_dict = {}
        can_dict = {}
        isresourcemanager = False
        if "ResourceM" in ecuc_autosar_dict.keys():
            resourcem_dict = ecuc_autosar_dict["ResourceM"].AR_PACKAGES.AR_PACKAGE[0].ELEMENTS.ECUC_MODULE_CONFIGURATION_VALUES[0]
        if "ResourceManager" in ecuc_autosar_dict.keys():
            resourcem_dict = ecuc_autosar_dict["ResourceManager"].AR_PACKAGES.AR_PACKAGE[0].ELEMENTS.ECUC_MODULE_CONFIGURATION_VALUES[0]
            isresourcemanager = True
        if "Can" in ecuc_autosar_dict.keys():
            can_dict = ecuc_autosar_dict["Can"].AR_PACKAGES.AR_PACKAGE[0].ELEMENTS.ECUC_MODULE_CONFIGURATION_VALUES[0]
        if isresourcemanager == False:
            net_to_core_dict = cls.parser_net_and_core_mapping_by_resourcem_ecuc_arxml(resourcem_dict, can_dict)
        else:
            net_to_core_dict = cls.parser_net_and_core_mapping_by_resourcemanager_ecuc_arxml(resourcem_dict, can_dict)
        cls.reset_net_to_core_dict()
        cls.set_net_to_core_dict(net_to_core_dict)
        cls.set_dbc_to_core_dict(net_to_core_dict, cfg_dict)
    