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
class CanIfSetMethod:
    @classmethod
    def get_canif_pdurx_name(cls, data, input_show_data_dict, data_management, bsw_editor_ui_func=None, method_parameter=None):
        """修改CanIfRxPduUserRxIndicationUL联动CanIfRxPduUserRxIndicationName"""
        data = data.strip()
        if data == "PDUR":
            return "pdur_canif_rx_indication", None
        elif data == "XCP":
            return "Xcp_CanIfRxIndication", None
        elif data == "CDD":
            return "cdd_CanIfRxIndication", None
        elif data == "DLT":
            return "Dlt_CanIfRxIndication", None
        elif data == "CAN_NM":
            return "cannm_rx_indication", None
        elif data == "CAN_TP":
            return "CanTp_RxIndication", None
        elif data == "CAN_TSYN":
            return "cantsyn_tx_confirmation", None
        return "", None

    @classmethod
    def get_canif_pdutx_name(cls, data, input_show_data_dict, data_management, bsw_editor_ui_func=None, method_parameter=None):
        """修改CanIfTxPduUserTxConfirmationUL联动CanIfTxPduUserTxConfirmationName"""
        if data == "PDUR":
            return "pdur_canif_tx_confirmation", None
        elif data == "XCP":
            return "Xcp_CanIfTxConfirmation", None
        elif data == "CDD":
            return "cdd_CanIfTxConfirmation", None
        elif data == "DLT":
            return "Dlt_CanIfTxConfirmation", None
        elif data == "CAN_NM":
            return "cannm_tx_confirmation", None
        elif data == "CAN_TP":
            return "CanTp_TxConfirmation", None
        elif data == "CAN_TSYN":
            return "cantsyn_tx_confirmation", None
        return "", None

    @classmethod
    def canif_trcv_mutex(cls, data, input_show_data_dict, data_management, bsw_editor_ui_func=None, method_parameter=None):
        """CanIfTrcvCanTrcvRef与其他配置项互斥"""
        if data != "":
            return "", True

        container_path = input_show_data_dict["ecuc_path"]
        mod_path_list = method_parameter[0]
        for mod_path in mod_path_list:
            show_data_dict = bsw_editor_ui_func.bsw_editor_get_the_show_data_of_the_item(container_path, mod_path, False)
            if show_data_dict["cfg_value"] is not None and show_data_dict["cfg_value"] != "":
                return "", True

        return input_show_data_dict["cfg_value"], False

