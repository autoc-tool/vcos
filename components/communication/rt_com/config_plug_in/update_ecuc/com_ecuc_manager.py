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
from func_module.module_management.arxml_handler.com_stack_ecuc.com_stack_ecuc_base_class import ComStackEcucBaseClass
from basic_func_module.autosar_utils.utils import get_short_name, get_definition_ref, get_value, set_value, set_definition_ref
from basic_func_module.autosar_utils.autosar_util import AutosarUtil
from basic_func_module.autosar_utils.types import PARAMETERS_CLASS
from func_module.health_monitor.logger import logger

SIGNAL_NOTIFICATION_DEFAULT_VALUE_LIST = [None, "None", "", " ", "0"]


class ComEcucManager(ComStackEcucBaseClass):

    def __init__(self):
        sort_path_list = ["/ActiveEcuC/Com/ComConfig"]
        super().__init__("Com", sort_path_list)

    def get_new_ecuc_from_different_initial(self, old_initial, new_initial, autosar_ecuc):
        com_ecuc = super().get_new_ecuc_from_different_initial(old_initial, new_initial, autosar_ecuc)
        # com_list = com_ecuc.find("/ActiveEcuC/Com/ComConfig")
        # if com_list is not None:
        #     if isinstance(getattr(com_list.SUB_CONTAINERS, "ECUC_CONTAINER_VALUE"), list):
        #         com_list.SUB_CONTAINERS.ECUC_CONTAINER_VALUE.sort(key = lambda x: get_short_name(x))
        #         com_list.SUB_CONTAINERS.ECUC_CONTAINER_VALUE.sort(key = lambda x: self.get_com_ipdu_handle_id(x))
        return com_ecuc

    def get_com_ipdu_handle_id(self, ecuc_container_obj):
        rv = 65535
        def_ref = get_definition_ref(ecuc_container_obj)
        if def_ref is None:
            def_ref = ""
        if def_ref.split("/")[-1] == "ComTimeBase":
            rv = 0
        elif def_ref.split("/")[-1] == "ComIPdu":
            if isinstance(getattr(ecuc_container_obj.PARAMETER_VALUES, "ECUC_NUMERICAL_PARAM_VALUE"), list):
                for num_para in ecuc_container_obj.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE:
                    num_para_def_ref = get_definition_ref(num_para)
                    if num_para_def_ref is None:
                        continue
                    if num_para_def_ref.split("/")[-1] == "ComIPduHandleId":
                        handle_id = get_value(num_para)
                        if not isinstance(handle_id, str):
                            break
                        if handle_id.isdigit():
                            rv = int(num_para.VALUE.valueOf_) + 1
                        break
        return rv

    @classmethod
    def reverse_short_name(cls, container):
        short_name = get_short_name(container)
        if isinstance(short_name, str):
            return short_name[::-1]
        else:
            logger.error("Com container don't have short name")
            return ""

    @classmethod
    def get_com_ecuc_notifications(cls, com_ecuc):
        '''获取非空值的Timeout和Notifications'''
        com_ecuc_notifications = {
            "ComNotification": {},
            "ComTimeout": {},
            "ComTimeoutNotification": {},
            "ComErrorNotification": {},
            "ComInvalidNotification": {}
        }
        ecuc_container_values = com_ecuc.find_nodes("ECUC-CONTAINER-VALUE")
        for signal_container in ecuc_container_values:
            signal_name = get_short_name(signal_container)
            if str(get_definition_ref(signal_container)).split("/")[-1] not in ["ComSignal", "ComSignalGroup"]:
                continue
            para_list = signal_container.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE + signal_container.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE
            for textual_param in para_list:
                if str(get_definition_ref(textual_param)).split("/")[-1] not in com_ecuc_notifications.keys():
                    continue
                com_notification_value = get_value(textual_param)
                if com_notification_value in SIGNAL_NOTIFICATION_DEFAULT_VALUE_LIST:
                    continue
                com_ecuc_notifications[str(get_definition_ref(textual_param)).split("/")[-1]].update({
                    signal_name: {
                        "signal_name": signal_name,
                        "notification_containr": textual_param,
                        "notification_value": com_notification_value
                    }
                })
        return com_ecuc_notifications

    @classmethod
    def get_com_module_appl_cbk_list(cls, com_ecuc):
        appl_cbk_list = []
        if com_ecuc is None:
            return appl_cbk_list

        com_ecuc_notifications = cls.get_com_ecuc_notifications(com_ecuc)

        for key, notifications in com_ecuc_notifications.items():
            if key == "ComTimeout":
                continue
            for notification in notifications.values():
                com_notification_value = notification.get("notification_value", None)
                if com_notification_value.startswith("Rte_"):
                    continue
                appl_cbk_list.append(com_notification_value)
        return sorted(appl_cbk_list)

    @classmethod
    def clear_rte_auto_derived_config(cls, com_ecuc):
        if com_ecuc is None:
            return
        ComEcucManager.update_com_usercbk_headerfile(com_ecuc, "Rte_Cbk.h", "del")
        com_ecuc_notifications = cls.get_com_ecuc_notifications(com_ecuc)

        com_notification_dict = com_ecuc_notifications.get("ComNotification", {})
        for parameter_dict in com_notification_dict.values():
            # 清除不需要的ComNotification
            param_containr = parameter_dict.get("notification_containr", None)
            cls.clear_auto_derived_container_value(param_containr, "")

        com_notification_dict = com_ecuc_notifications.get("ComTimeoutNotification", {})
        for parameter_dict in com_notification_dict.values():
            # 清除不需要的TimeoutNotification
            param_containr = parameter_dict.get("notification_containr", None)
            cls.clear_auto_derived_container_value(param_containr, "")

        com_notification_dict = com_ecuc_notifications.get("ComTimeout", {})
        for parameter_dict in com_notification_dict.values():
            # 不需要的Timeout清0
            param_containr = parameter_dict.get("notification_containr", None)
            cls.clear_auto_derived_container_value(param_containr, "0")

    @classmethod
    def clear_auto_derived_container_value(cls, value_containr, set_val=""):
        if value_containr is None:
            return
        if AutosarUtil.is_auto_derived(value_containr):
            AutosarUtil.remove_annotation_value(value_containr, "AutoDerived")
            if not AutosarUtil.is_user_defined(value_containr):
                set_value(value_containr, set_val)

    @classmethod
    def update_com_usercbk_headerfile(cls, com_ecuc, header_name, operation):
        empty_obj = None
        found_header_obj = []
        com_general = com_ecuc.find("/ActiveEcuC/Com/ComGeneral")
        textual_param_value_list = getattr(getattr(com_general, "PARAMETER_VALUES", None), "ECUC_TEXTUAL_PARAM_VALUE",
                                           None)
        if textual_param_value_list is None or not isinstance(textual_param_value_list, list):
            return
        comusercbkheaderfile_num = 0
        for val_obj in textual_param_value_list:
            if get_definition_ref(val_obj).split("/")[-1] == "ComUserCbkHeaderFile":
                comusercbkheaderfile_num += 1
                val = get_value(val_obj)
                if val == header_name:
                    found_header_obj.append(val_obj)
                elif val is None:
                    empty_obj = val_obj
        if operation == "del":
            for usercbk_obj in found_header_obj:
                if usercbk_obj is not None and AutosarUtil.is_auto_derived(usercbk_obj):
                    AutosarUtil.remove_annotation_value(usercbk_obj, "AutoDerived")
                    if comusercbkheaderfile_num > 1:
                        AutosarUtil.remove_node_value(textual_param_value_list, usercbk_obj)
                    else:
                        set_value(usercbk_obj, None)
                    break
        elif operation == "add" and len(found_header_obj) == 0:
            if empty_obj is None:
                def_ref = "/AUTOSAR/EcucDefs/Com/ComGeneral/ComUserCbkHeaderFile"
                empty_obj = PARAMETERS_CLASS["ECUC-TEXTUAL-PARAM-VALUE"]()
                set_definition_ref(empty_obj, "ECUC-STRING-PARAM-DEF", def_ref)
                AutosarUtil.set_tag(empty_obj, "ECUC-TEXTUAL-PARAM-VALUE")
                AutosarUtil.set_parent(empty_obj, com_general.PARAMETER_VALUES)
                textual_param_value_list.append(empty_obj)
            set_value(empty_obj, header_name)
            if not AutosarUtil.is_auto_derived(empty_obj):
                AutosarUtil.add_annotation_value(empty_obj, "AutoDerived")
