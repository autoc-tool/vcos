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


class NmEcucManager(ComStackEcucBaseClass):

    def __init__(self):
        sort_path_list = ["/ActiveEcuC/Nm/NmChannelConfig"]
        super().__init__("Nm", sort_path_list)

    def get_new_ecuc_from_different_initial(self, old_initial, new_initial, autosar_ecuc):
        ecuc_sutosar = super().get_new_ecuc_from_different_initial(old_initial, new_initial, autosar_ecuc)
        sort_list = ecuc_sutosar.find("/ActiveEcuC/Nm/NmChannelConfig")
        if sort_list is not None:
            if isinstance(getattr(sort_list.SUB_CONTAINERS, "ECUC_CONTAINER_VALUE"), list):
                sort_list.SUB_CONTAINERS.ECUC_CONTAINER_VALUE.sort(key=lambda x: self.get_sort_id(x))
        return ecuc_sutosar

    def get_sort_id(self, ecuc_container_obj):
        rv = 65535
        def_ref = get_definition_ref(ecuc_container_obj)
        if def_ref is None:
            def_ref = ""
        elif def_ref.split("/")[-1] in COMM_CONFIG_SET_SORT_DICT.keys():
            if isinstance(getattr(ecuc_container_obj.PARAMETER_VALUES, "ECUC_NUMERICAL_PARAM_VALUE"), list):
                for num_para in ecuc_container_obj.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE:
                    num_para_def_ref = get_definition_ref(num_para)
                    if num_para_def_ref is None:
                        continue
                    if num_para_def_ref.split("/")[-1] == COMM_CONFIG_SET_SORT_DICT[def_ref.split("/")[-1]]:
                        sort_id = get_value(num_para)
                        if not isinstance(sort_id, str):
                            break
                        if sort_id.isdigit():
                            rv = int(sort_id)
                        break
        return rv
