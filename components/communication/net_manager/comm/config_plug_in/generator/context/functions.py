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
ComM模板内函数定义模块.
定义jinja2模板中所使用的函数.
"""

import copy
from func_module.generator.common.common import FunctionBase


class Function(FunctionBase):
    """
    Class containing all functions. Add new static functions here
    to be included in the jinja2 context
    """

    @staticmethod
    def dict_to_arxml_dict(cur_dict: dict, cur_key_name: str, parent_dict: dict, parent_key_name: str):
        if not isinstance(cur_dict, dict):
            return
        if "Tag" in cur_dict and cur_dict["Tag"] == "CONTAINER-GROUP":
            for container_name, container_dict in list(cur_dict.items()):
                if container_name == "Tag" or container_name == "DefinitionRef":
                    continue
                Function.dict_to_arxml_dict(container_dict, container_name, cur_dict, cur_key_name)
                parent_dict.update({container_name: container_dict})
            del parent_dict[cur_key_name]
        else:
            for container_name, container_dict in list(cur_dict.items()):
                if container_name == "Tag" or container_name == "DefinitionRef":
                    continue
                Function.dict_to_arxml_dict(container_dict, container_name, cur_dict, cur_key_name)

    @staticmethod
    def get_dict_by_ref_path(config_dict: dict, ref_path: str) -> dict:
        bswm_ecuc_arxml_dict = copy.deepcopy(config_dict)
        Function.dict_to_arxml_dict(bswm_ecuc_arxml_dict["BswMGeneral"], "BswMGeneral", bswm_ecuc_arxml_dict, "BswM")
        Function.dict_to_arxml_dict(bswm_ecuc_arxml_dict["BswMConfig"], "BswMConfig", bswm_ecuc_arxml_dict, "BswM")
        ref_path_list = ref_path.split("/")
        bswm_ecuc_arxml_dict = {"BswM": bswm_ecuc_arxml_dict}
        for id, path_key in enumerate(ref_path_list):
            if path_key == "ActiveEcuC" or 0 == id:
                continue
            if path_key in bswm_ecuc_arxml_dict:
                bswm_ecuc_arxml_dict = bswm_ecuc_arxml_dict[path_key]
            else:
                return {}
        return bswm_ecuc_arxml_dict

    @staticmethod
    def str_to_list(key_value: str) -> list:
        res = []
        if isinstance(key_value, str):
            res.append(key_value)
        else:
            res = key_value
        return res

    @staticmethod
    def get_key_name(key_name: str) -> str:
        key_name = key_name.split("/")[-1]
        return key_name

    @staticmethod
    def get_headers(marco_list: list) -> list:
        res = []
        if "CONFIG_MODULE_NM" in marco_list:
            res.append("Nm.h")
        if "CONFIG_MODULE_CANSM" in marco_list:
            res.append("CanSM.h")
        if "VCOS_MODULE_CONFIG_LINSM" in marco_list:
            res.append("LinSM.h")
        if "VCOS_MODULE_CONFIG_ETHSM" in marco_list:
            res.append("EthSM.h")
        return res

    @staticmethod
    def gen_switch_macro(key_value: str) -> str:
        if key_value == False:
            res = "STD_OFF"
        else:
            res = "STD_ON"
        return res

    @staticmethod
    def get_time_ms(timer: float) -> int:
        res = 0
        if timer is not None and timer != "":
            res = int(1000*timer)
        return res

    @staticmethod
    def gen_user_channels(user_name: str, channels: dict) -> list:
        res = []
        for channel_name, channel_value in channels.items():
            if channel_name != "Tag" and "ComMUserPerChannels" in channel_value:
                for name, value in list(channel_value["ComMUserPerChannels"].items()):
                    if name != "Tag" and user_name == Function.get_key_name(value["ComMUserChannel"]["ValueRef"]):
                        res.append(channel_name)
        return res

    @staticmethod
    def gen_pnc_userchannels(user_name: str, pnc_userchannels: dict) -> list:
        res = ["NULL_PTR", 0]
        if user_name in pnc_userchannels:
            res[0] = "ComM_ComMUser_" + user_name + "_PncChannels"
            res[1] = len(pnc_userchannels[user_name])
        return res

    def refs_to_names(refs: list) -> list:
        ref_list = Function.str_to_list(refs)
        name_list = []
        for ref in ref_list:
            ref = Function.get_key_name(ref)
            name_list.append(ref)
        return name_list

    @staticmethod
    def get_channeluser_data(pnc_value: dict) -> dict:
        channel_refs = pnc_value["ComMChannelPerPnc"]["ValueRef"]
        user_refs = pnc_value["ComMUserPerPnc"]["ValueRef"]
        channels = Function.refs_to_names(channel_refs)
        if user_refs is not None:
            users = Function.refs_to_names(user_refs)
        else:
            users = []

        channeluser_data = {"channels": channels, "users": users}
        return channeluser_data

    @staticmethod
    def gen_pncuser_channels(pnc_dict: dict) -> dict:
        res = {}
        if len(pnc_dict) != 0:
            for pnc_name, pnc_value in list(pnc_dict.items()):
                if pnc_name != "Tag":
                    #如果配置了User
                    if pnc_value["ComMUserPerPnc"]["ValueRef"] is not None:
                        user_list = Function.refs_to_names(pnc_value["ComMUserPerPnc"]["ValueRef"])
                        channel_list = Function.refs_to_names(pnc_value["ComMChannelPerPnc"]["ValueRef"])
                        for user in user_list:
                            if user not in res:
                                user_channel = copy.deepcopy(channel_list)
                                res[user] = user_channel
                            else:
                                for channel in channel_list:
                                    if channel not in res[user]:
                                        res[user].append(channel)
        return res

    @staticmethod
    def get_signal_data(pnc_dict: dict) -> dict:
        eira_rx = ""
        era_rx = {}
        eira_tx = {}

        for pnc_name, pnc_value in list(pnc_dict.items()):
            if pnc_name != "Tag" and "ComMPncComSignals" in pnc_value:
                signal_dict = pnc_value["ComMPncComSignals"]
                for signal_name, signal_value in list(signal_dict.items()):
                    if signal_name != "Tag":
                        direction = signal_value["ComMPncComSignalDirection"]["Value"]
                        kind = signal_value["ComMPncComSignalKind"]["Value"]
                        signal = Function.get_key_name(signal_value["ComMPncComSignalRef"]["ValueRef"])
                        channel_ref = signal_value["ComMPncComSignalChannelRef"]["ValueRef"]

                        if direction == "RX" and kind == "EIRA" and eira_rx == "":
                            eira_rx = signal

                        if channel_ref is not None:
                            channel = Function.get_key_name(channel_ref)
                            if direction == "RX" and kind == "ERA":
                                if channel not in era_rx:
                                    era_rx[channel] = signal
                            else: # direction == "TX"
                                if channel_ref not in eira_tx:
                                    eira_tx[channel] = signal
                        else:
                            continue

        channel_signal_data = {"eira_rx": eira_rx, "era_rx": era_rx, "eira_tx": eira_tx}
        return channel_signal_data

    @staticmethod
    def get_pnc_data(pnc_name: str, pnc_value: dict) -> dict:
        channel_list = Function.str_to_list(pnc_value["ComMChannelPerPnc"]["ValueRef"])

        pnc_data = {"PncChnlRefNum": len(channel_list),
                    "PncUserRefNum": 0,
                    "PncId": pnc_value["ComMPncId"]["Value"],
                    "PncChnlRef": str(pnc_name) + "_Channels",
                    "PncUserRef": "NULL_PTR"}
        #如果配置了User
        if pnc_value["ComMUserPerPnc"]["ValueRef"] is not None:
            user_list = Function.str_to_list(pnc_value["ComMUserPerPnc"]["ValueRef"])
            pnc_data["PncUserRefNum"] = len(user_list)
            pnc_data["PncUserRef"] = str(pnc_name) + "_Users"
        return pnc_data

    @staticmethod
    def gen_pnc_config(pnc_dict: dict) -> dict:
        config = {"Pnc": "NULL_PTR", "PncIdToPncCountVal": "NULL_PTR",
                  "PncEIRARxComSigIdRef": "NULL_PTR", "PncTxComSigIdRefs": "NULL_PTR",
                  "PncERARxComSigIdRefs": "NULL_PTR"}
        #获取PNC数组
        if(len(pnc_dict)) != 0:
            config = {"Pnc": "ComM_Pncs", "PncIdToPncCountVal": "IdToCount",
                      "PncEIRARxComSigIdRef": "&EIRARx_ref", "PncTxComSigIdRefs": "EIRATx_refs",
                      "PncERARxComSigIdRefs": "ERARx_refs"}
        return config
