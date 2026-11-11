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
from jinja2 import pass_context
# from func_module.health_monitor.logger import logger
from func_module.generator.common.common import FunctionBase


class Function(FunctionBase):

    @staticmethod
    @pass_context
    def test(context):
        return 0

    @staticmethod
    def gen_switch_macro(base_dict: dict, key_str: str):
        res = "STD_OFF"
        if base_dict.get(key_str):
            if base_dict.get(key_str).get('Value') == True:
                res = "STD_ON"
        return res

    @staticmethod
    def gen_switch_bool(key_value: str) -> str:
        if key_value == False:
            res = "FALSE"
        else:
            res = "TRUE"
        return res

    @staticmethod
    def get_prt_value(base_dict: dict, key_str: str):
        if base_dict.get(key_str):
            res = base_dict.get(key_str).get('Value')
            if res == '' or res == None:
                res = "NULL"
        else:
            res = "NULL"
        return res

    @staticmethod
    def get_cannm_status(base_dict: dict):
        for container_name, container_dict in base_dict.items():
            if (container_name == "DefinitionRef"):
                continue
            if (container_dict["NmBusType"]["NmStandardBusNmConfig"]["NmStandardBusType"]["Value"] == "NM_BUSNM_CANNM"):
                res = Function.gen_switch_macro(container_dict, "NmPassiveModeEnabled")
                break
            else:
                res = "STD_OFF"
        return res

    @staticmethod
    def get_udpnm_status(base_dict: dict):
        for container_name, container_dict in base_dict.items():
            if (container_name == "DefinitionRef"):
                continue
            if (container_dict["NmBusType"]["NmStandardBusNmConfig"]["NmStandardBusType"]["Value"] == "NM_BUSNM_UDPNM"):
                res = Function.gen_switch_macro(container_dict, "NmPassiveModeEnabled")
                break
            else:
                res = "STD_OFF"
        return res

    @staticmethod
    def get_key_name(key_name: str) -> str:
        key_name = key_name.split("/")[-1]
        return key_name

    def nm_comm_compare(comm_list: list, nm_dict: dict, comm_dict: dict):
        channel = {}
        bustype = {}
        res = [channel, bustype]
        for i in range(len(comm_list)):
            found = False
            for nm_name, nm_action in nm_dict.items():
                if (nm_name != "DefinitionRef"):
                    nm_channelname = nm_action["NmComMChannelRef"]["ValueRef"].split("/")[-1]
                    bus_type = nm_action["NmBusType"]["NmStandardBusNmConfig"]["NmStandardBusType"]["Value"]
                    if str(i) == str(comm_dict[nm_channelname]["ComMChannelId"]["Value"]):
                        bustype[str(i)] = bus_type
                        channel[str(i)] = "COMM_NETWORK_HANDLE_" + nm_channelname
                        found = True
                        break
            if found == False:
                bustype[str(i)] = "NM_BUSNM_UNDEF"
                channel[str(i)] = "0xFFu"
        return res

    @staticmethod
    def get_nm_includefiles(nm_dict: dict):
        func_list = []
        for key, value in nm_dict["NmGlobalConfig"]["NmGlobalFeatures"].items():
            if (key == "Tag" or key == "DefinitionRef"):
                continue
            if ("NmGlobalFeatures/NmCallbacksPrototypeHeader" in value["DefinitionRef"]):
                #进入notification
                if (isinstance(value["Value"], str) == True):
                    if (value["Value"] != "<empty>" and value["Value"] != None):
                        func_list.append(value["Value"])
                elif (isinstance(value["Value"], list) == True):
                    for value2 in value["Value"]:
                        if (value2 != "<empty>" and value2 != None):
                            func_list.append(value2)
        return func_list

    @staticmethod
    def gen_pdurxindication(nm_dict: dict):
        res = {}
        for channel_name, channel_value in nm_dict.items():
            if not isinstance(channel_value, dict):
                continue
            if channel_value['DefinitionRef'].split('/')[-1] == "NmChannelConfig":
                res[channel_name] = channel_value["NmBusType"]["NmStandardBusNmConfig"]["NmGenericBusNmPduRxIndication"]["Value"]
        return res
    
    @staticmethod
    def get_nmpduRxIndication(nm_dict: dict):
        res = ""
        for key, value in nm_dict["NmGlobalConfig"]["NmGlobalFeatures"].items():
            if (key == "Tag" or key == "DefinitionRef"):
                continue
            if ("NmGlobalFeatures/NmPduRcvIndCallback" in value["DefinitionRef"]):
                #进入notification
                if (isinstance(value["Value"], str) == True):
                    if (value["Value"] != "<empty>" and value["Value"] != None):
                        res = value["Value"]
        return res
