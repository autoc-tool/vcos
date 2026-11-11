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
    def get_cantp_json_config(context):
        cantp_json_config = context["cantp_json"]
        return cantp_json_config

    @staticmethod
    @pass_context
    def get_cantp_json_config_for_DiagPhysicalRx(context):
        DiagPhysicalRx_list = []
        cantp_json_config = context["cantp_json"]
        for dbc_value in cantp_json_config.values():
            if len(dbc_value["DiagPhysicalRx"]) < 1:
                continue
            for msg_name in dbc_value["DiagPhysicalRx"]:
                DiagPhysicalRx_list.append(msg_name)

        return DiagPhysicalRx_list

    @staticmethod
    @pass_context
    def get_cantp_json_config_for_DiagFunctionalRx(context):
        DiagFunctionalRx_list = []
        cantp_json_config = context["cantp_json"]
        for dbc_value in cantp_json_config.values():
            if len(dbc_value["DiagFunctionalRx"]) < 1:
                continue
            for msg_name in dbc_value["DiagFunctionalRx"]:
                DiagFunctionalRx_list.append(msg_name)
        return DiagFunctionalRx_list

    @staticmethod
    @pass_context
    def get_cantp_json_config_for_DiagPhysicalTx(context):
        DiagPhysicalTx_list = []
        cantp_json_config = context["cantp_json"]
        for dbc_value in cantp_json_config.values():
            if len(dbc_value["DiagPhysicalTx"]) < 1:
                continue
            for msg_name in dbc_value["DiagPhysicalTx"]:
                DiagPhysicalTx_list.append(msg_name)
        return DiagPhysicalTx_list

    @staticmethod
    @pass_context
    def get_cantp_nsa_value(self, RxNSdu: dict) -> int:
        nsa_value = 0
        if "CanTpNSa" in RxNSdu:
            nsa_value = RxNSdu["CanTpNSa"]["CanTpNSa"]["Value"]

        return nsa_value

    @staticmethod
    @pass_context
    def get_cantp_nta_value(self, RxNSdu: dict) -> int:
        nsa_value = 0
        if "CanTpNTa" in RxNSdu:
            nsa_value = RxNSdu["CanTpNTa"]["CanTpNTa"]["Value"]

        return nsa_value

    @staticmethod
    @pass_context
    def get_enable_value_from_boolean(self, value: bool) -> str:
        enable_value = "STD_OFF"
        if value:
            enable_value = "STD_ON"

        return enable_value

    @staticmethod
    @pass_context
    def get_cantp_transmit_cancel_state(self, cantp_channels: dict):
        state = "STD_OFF"

        # 遍历所有的CanTpTxNSdus.CanTpTc，只要有一个配置为True就打开
        for _, cantp_channel in cantp_channels.items():
            for _, CanTpTxNSdu in cantp_channel["CanTpTxNSdus"].items():
                if CanTpTxNSdu["CanTpTc"]["Value"] is True:
                    state = "STD_ON"
                    break

        return state

    @staticmethod
    @pass_context
    def get_rx_nsdu_ref(self, CanTpRxNPdu: dict) -> str:
        rx_sdu_ref = ""
        ref = CanTpRxNPdu["CanTpRxNPduRef"]["ValueRef"].split("/")[-1]

        suffix_list = ["_oCanIf_CanTp_Rx", "_oCanTp_PduR_Rx"]
        for suffix in suffix_list:
            if suffix in ref:
                rx_sdu_ref = ref.split(suffix)[0]
                break

        return rx_sdu_ref

    @staticmethod
    @pass_context
    def get_tx_nsdu_ref(self, CanTpTxNPdu: dict) -> str:
        tx_sdu_ref = ""
        ref = CanTpTxNPdu["CanTpTxNPduRef"]["ValueRef"].split("/")[-1]

        suffix_list = ["_oPduR_CanTp_Tx", "_oCanTp_CanIf_Tx"]
        for suffix in suffix_list:
            if suffix in ref:
                tx_sdu_ref = ref.split(suffix)[0]
                break

        return tx_sdu_ref

    @staticmethod
    @pass_context
    def get_cantp_tx_fnc_npdu(self, prefix_name: str, RxNSdu: dict, suffix_name: str) -> str:
        cantp_tx_fnc_npdu = "CANTP_NO_REFERRING_TX_INDEX"
        if "CanTpTxFcNPdu" in RxNSdu and RxNSdu["CanTpTxFcNPdu"]["CanTpTxFcNPduRef"]["ValueRef"] != "" and RxNSdu[
                "CanTpTxFcNPdu"]["CanTpTxFcNPduRef"]["ValueRef"] != "<empty>" and RxNSdu["CanTpTxFcNPdu"][
                    "CanTpTxFcNPduRef"]["ValueRef"] is not None:
            ref = RxNSdu["CanTpTxFcNPdu"]["CanTpTxFcNPduRef"]["ValueRef"].split("/")[-1]
            tx_fnc_npdu_ref = ""
            suffix_list = ["_oPduR_CanTp_Tx", "_oCanTp_CanIf_Tx"]
            for suffix in suffix_list:
                if suffix in ref:
                    tx_fnc_npdu_ref = ref.split(suffix)[0]
                    break
            cantp_tx_fnc_npdu = prefix_name + tx_fnc_npdu_ref + suffix_name
        return cantp_tx_fnc_npdu

    @staticmethod
    @pass_context
    def get_nof_cantp_sdu(self, cantp_channels) -> int:
        cantp_sdu_num = 0
        for _, cantp_channel in cantp_channels.items():
            if isinstance(cantp_channel, dict):
                cantp_sdu_num += len(cantp_channel["Channel_Cfg"]["CanTpRxNSdus"]) + len(cantp_channel["Channel_Cfg"]["CanTpTxNSdus"])

        return cantp_sdu_num

    @staticmethod
    @pass_context
    def get_cantp_padding_activation(self, CanTpChannelByCoreId: dict) -> str:
        cantp_padding_activation = "CANTP_ON"
        for _, cantp_cord in CanTpChannelByCoreId.items():
            for _, cantp_channel in cantp_cord.items():
                if isinstance(cantp_channel, dict):
                    for _, rx_nsdu in cantp_channel["Channel_Cfg"]["CanTpRxNSdus"].items():
                        if rx_nsdu["CanTpRxPaddingActivation"]["Value"] != cantp_padding_activation:
                            cantp_padding_activation = "CANTP_OFF"
                            break
                    for _, tx_nsdu in cantp_channel["Channel_Cfg"]["CanTpTxNSdus"].items():
                        if tx_nsdu["CanTpTxPaddingActivation"]["Value"] != cantp_padding_activation:
                            cantp_padding_activation = "CANTP_OFF"
                            break
        return cantp_padding_activation

    @staticmethod
    @pass_context
    def convert_from_s_to_ms(self, milliseconds: float) -> int:
        if (milliseconds is not None and milliseconds != ''):
            return int(1000.0 * milliseconds)
        else:
            return 0

    @staticmethod
    @pass_context
    def convert_from_s_to_count(self, cantp_mainfunc: dict, coreid: int, milliseconds: float ) -> int:
        if (milliseconds is not None and milliseconds != ''):
            for _, mainfunc in cantp_mainfunc.items():
                if mainfunc[0]["coreid"] == coreid:
                    repet_num = int(milliseconds / mainfunc[0]["period"])
                    return repet_num
        else:
            return 0

    @staticmethod
    @pass_context
    def convert_ui_int_value_to_cfg(self, value) -> int:
        if (value is not None and value != ''):
            return int(value)
        else:
            return 0

    @staticmethod
    @pass_context
    def get_max_pdu_id(self, cantp_channels: dict) -> int:
        max_id = 0
        for _, cantp_channel in cantp_channels.items():
            for _, rx_nsdu in cantp_channel["CanTpRxNSdus"].items():
                if rx_nsdu["CanTpRxNSduId"]["Value"] > max_id:
                    max_id = rx_nsdu["CanTpRxNSduId"]["Value"]
            for _, tx_nsdu in cantp_channel["CanTpTxNSdus"].items():
                if tx_nsdu["CanTpTxNSduId"]["Value"] > max_id:
                    max_id = tx_nsdu["CanTpTxNSduId"]["Value"]
        return max_id + 1  # 后面的in range 函数需要保证max_id自身被包括进去

    @staticmethod
    @pass_context
    def get_mainfunction_config(self, cantp_config: dict) -> dict:
        mainfunction_config = {}
        for key, value in cantp_config.items():
            if key == "DefinitionRef":
                continue
            if value["DefinitionRef"].split("/")[-1] == "CanTpMainFunction":
                mainfunction_config[key] = {"name":key, "period":value["CanTpMainFunctionPeriod"]["Value"]}
        return mainfunction_config

    @staticmethod
    @pass_context
    def get_mainfunc_core_app_memmap_tag(self, cantp_core_app_map: dict, coreid: int) -> dict:
        for core_app in cantp_core_app_map:
            if core_app["coreid"] == coreid:
                return core_app["app"].upper() + "_"
        return ""