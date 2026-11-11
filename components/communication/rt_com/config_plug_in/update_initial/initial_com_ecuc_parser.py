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
from func_module.health_monitor.logger import logger

class InitialComEcucParser():

    def __init__(self, mod_autosar_dict):
        self.mod_autosar_dict = mod_autosar_dict
        self.imported_dbc_name_list = []
        self.imported_ldf_name_list = []
        self.module_autosar_dict = None
        self.ecuc_ref_dict = None
        self.ipdu_handle_id = 0
        self.ipdu_init_handle_id = 0
        self.com_mainfunction_coreId_refs = {}

    def update_initial_ecuc_from_dbc_ldf(self, initial_autosar, cfg_dict):
        self.updata_com_info_from_dbc_slot(initial_autosar, cfg_dict)
        self.updata_com_info_from_ldf_slot(initial_autosar, cfg_dict)
        self.updata_com_signalgw_from_excel_slot(initial_autosar, cfg_dict)

    # def updata_com_signalgw_form_excel_slot(initial_autosar, cfg_dict):

    def com_class_to_autosar_dict(self, initial: object) -> dict:
        autosar_dict = None
        autosar_dict = AutosarUtil.to_bsw_dict(initial)
        return autosar_dict

    def get_ipdu_handle_id(self, autosar_dict):
        ipdu_handle_id_max = 0
        if autosar_dict is None:
            return 0
        com_config = _get_dict_value_by_nested_keys(autosar_dict, ['ArPackages', 'InitialEcuC', 'Com', 'ComConfig'])
        if not isinstance(com_config, dict):
            return 0
        for signal_name, signal_info in com_config.items():
            if signal_name in ["Tag", "DefinitionRef", "Description"] or not isinstance(signal_info, dict):
                continue
            if signal_info["DefinitionRef"] == "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu":
                if signal_info["ComIPduHandleId"] is not None:
                    ipdu_handle_id_max = ipdu_handle_id_max if ipdu_handle_id_max > signal_info["ComIPduHandleId"][
                        "Value"] else (signal_info["ComIPduHandleId"]["Value"] + 1)
        return ipdu_handle_id_max

    def get_supported_ipdu_groups(self, autosar_dict):
        ipdu_group_num = 0
        if autosar_dict is None:
            return 0
        com_config = _get_dict_value_by_nested_keys(autosar_dict, ['ArPackages', 'InitialEcuC', 'Com', 'ComConfig'])
        if not isinstance(com_config, dict):
            return 0
        for name, info in com_config.items():
            if name in ["Tag", "DefinitionRef", "Description"] or not isinstance(info, dict):
                continue
            if info["DefinitionRef"] == "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPduGroup":
                ipdu_group_num += 1
        return ipdu_group_num


    def remove_base_mainfunction_in_initial(self, initial_autosar):
        ComConfigSet = initial_autosar.find("/InitialEcuC/Com/ComConfig").SUB_CONTAINERS.ECUC_CONTAINER_VALUE
        for ele in reversed(ComConfigSet):
            if get_definition_ref(ele) == "/AUTOSAR/EcucDefs/Com/ComConfig/ComMainFunctionRx":
                if get_short_name(ele) == "Com_MainFunction_Rx":
                    AutosarUtil.remove_container_value(ComConfigSet, ele)
            elif get_definition_ref(ele) == "/AUTOSAR/EcucDefs/Com/ComConfig/ComMainFunctionTx":
                if get_short_name(ele) == "Com_MainFunction_Tx":
                    AutosarUtil.remove_container_value(ComConfigSet, ele)

    def updata_com_info_from_dbc_slot(self, initial_autosar, dict):
        self.com_dbc_data_obj = ComModuleInitData()
        if dict["dbc"] is not None:
            self.module_autosar_dict = self.com_class_to_autosar_dict(initial_autosar)
            self.ipdu_handle_id = self.get_ipdu_handle_id(self.module_autosar_dict)
            self.ipdu_init_handle_id = self.ipdu_handle_id
            self.supported_ipdu_groups = self.get_supported_ipdu_groups(self.module_autosar_dict)
            self.update_com_data_from_add_dbc(dict, self.com_dbc_data_obj)
            # 多核部署场景下才需要推导创建mainfunction
            if len(self.com_mainfunction_coreId_refs) > 1:
                isupdate_mainfunction = True
                # 多核场景下  MainFunction通过推到生成  删除initial里面的MainFunction
                self.remove_base_mainfunction_in_initial(initial_autosar)
            else: 
                isupdate_mainfunction = False
            self.update_com_arxml_obj_from_com_data(isupdate_mainfunction, initial_autosar, self.com_dbc_data_obj)

    def updata_com_info_from_ldf_slot(self, initial_autosar, dict):
        self.com_ldf_data_obj = ComModuleInitData()
        if dict["ldf"] is not None:
            self.module_autosar_dict = self.com_class_to_autosar_dict(initial_autosar)
            self.ipdu_handle_id = self.get_ipdu_handle_id(self.module_autosar_dict)
            self.ipdu_init_handle_id = self.ipdu_handle_id
            self.supported_ipdu_groups = self.get_supported_ipdu_groups(self.module_autosar_dict)
            # 已经导入DBC或者单核场景下  不需要推导MainFunction
            if dict["dbc"] is not None or len(self.com_mainfunction_coreId_refs) <= 1:
                isupdate_mainfunction = False
            else:
                # 多核场景下  MainFunction通过推到生成  删除initial里面的MainFunction
                self.remove_base_mainfunction_in_initial(initial_autosar)
                isupdate_mainfunction = True
            self.update_com_data_from_add_ldf(isupdate_mainfunction, dict, self.com_ldf_data_obj)
            self.update_com_arxml_obj_from_com_data(isupdate_mainfunction, initial_autosar, self.com_ldf_data_obj)

    def update_com_data_from_add_ldf(self, isupdate_mainfunction, dict, com_data_obj):
        com_config_json_dict = get_config_json_template_input_ldf_list(dict)
        self.com_ldf_config = com_config_json_dict["config"]
        self.ipdu_group_refs = com_config_json_dict["ipdu_group_refs"]
        self.com_mainfunction_coreId_refs.update(com_config_json_dict["com_mainfunction_coreId_refs"])
        for comm_diag_name, value in self.com_ldf_config.items():
            self.node_selected = value["node_selected"]
            for message in value["details"]:
                group_signals_list = []
                signal_groups_list = []
                siganl_groups_brief_info_dict = com_data_obj.update_com_signal_groups_info_from_message(message)
                for key, group_signals in siganl_groups_brief_info_dict.items():
                    signal_groups_list.append(key)
                    group_signals_list.extend(group_signals)
                if 0 == len(message["signals"]):
                    continue
                com_data_obj.update_com_ipdu_info_from_message(message, signal_groups_list, group_signals_list,
                                                               self.node_selected, self.ipdu_handle_id, value["coreId"], self.com_mainfunction_coreId_refs, True)
                self.ipdu_handle_id += 1
                for i in range(len(message["signals"])):
                    if message["signals"][i]["signal_name"] not in group_signals_list:
                        com_data_obj.update_com_signal_info_from_message(message["signals"][i])
        com_data_obj.update_com_general_info_from_message(self.ipdu_group_refs, self.supported_ipdu_groups)
        com_data_obj.update_com_ipdu_groups_info_from_message(self.ipdu_group_refs, self.supported_ipdu_groups)
        # 当没有DBC导入的时候 这里需要创建MainFunction
        if isupdate_mainfunction == True:
            com_data_obj.updata_com_com_mainfunction_info_from_message(self.com_mainfunction_coreId_refs)



    def update_com_arxml_obj_from_com_data(self, isupdate_mainfunction, initial_autosar: object, com_data_obj):
        self.update_com_arxml_ipdus(initial_autosar, com_data_obj)
        self.update_com_arxml_signals(initial_autosar, com_data_obj)
        self.update_com_arxml_signal_groups(initial_autosar, com_data_obj)
        self.update_com_arxml_ipdu_groups(initial_autosar, com_data_obj)
        self.update_com_arxml_supported_ipdu_groups(initial_autosar, com_data_obj)
        if isupdate_mainfunction == True:
            self.update_com_arxml_mainfunctions(initial_autosar, com_data_obj)

    def update_com_arxml_obj_from_excel_data(self, initial_autosar: object, com_data_obj):
        self.update_com_arxml_signalgw(initial_autosar, com_data_obj)

    def updata_com_signalgw_from_excel_slot(self, initial_autosar, dict):
        self.com_excel_data_obj = ComModuleInitData()
        if "excel" in dict:
            if "ComCfg.xlsx" in dict["excel"]:
                self.update_com_signal_gateway(dict["excel"]["ComCfg.xlsx"], self.com_excel_data_obj)
                self.update_com_arxml_obj_from_excel_data(initial_autosar, self.com_excel_data_obj)

    def parse_signalgw_from_dbc(self, target, signalgw_solit, comgw_dict, isdestination):
        for dbc_name, value in self.com_dbc_config.items():
            target_list = target.split('_')
            if target_list[1] in dbc_name:
                for message in value["details"]:
                    for i in range(len(message["signals"])):
                        signalname_list = message["signals"][i]["signal_name"].split('_o')
                        if signalgw_solit[0] == signalname_list[0]:
                            if isdestination == False:
                                comgw_dict["source"] = message["signals"][i]["signal_name"] + "_Rx"
                                return
                            else:
                                comgw_dict["destination"].append(message["signals"][i]["signal_name"] + "_Tx")
                                return

    def parse_signalgw_from_ldf(self, target, signalgw_solit, comgw_dict, isdestination):
        for ldf_name, value in self.com_ldf_config.items():
            if (len(value["details"]) > 0):
                target_list = target.split('_')
                if value["details"][0]["bus_name"] == target_list[1]:
                    for message in value["details"]:
                        for i in range(len(message["signals"])):
                            signalname_list = message["signals"][i]["signal_name"].split('_o')
                            if signalgw_solit[0] == signalname_list[0]:
                                if isdestination == False:
                                    comgw_dict["source"] = message["signals"][i]["signal_name"] + "_Rx"
                                    return
                                else:
                                    comgw_dict["destination"].append(message["signals"][i]["signal_name"] + "_Tx")
                                    return

    def update_com_signal_gateway(self, com_excel_dict, com_data_obj):
        comgw_dict = {"destination": [], "source": ""}
        comgw_cnt = 0
        for signalgw_source, signalgw_destination_list in com_excel_dict.items():
            signalgw_source_solit = signalgw_source.split('_o')
            for target in signalgw_source_solit:
                if "CAN" in target:
                    self.parse_signalgw_from_dbc(target, signalgw_source_solit, comgw_dict, False)

            for i in range(len(signalgw_destination_list)):
                signalgw_destination_split = signalgw_destination_list[i].split('_o')
                for target in signalgw_destination_split:
                    if "CAN" in target:
                        self.parse_signalgw_from_dbc(target, signalgw_destination_split, comgw_dict, True)

            if comgw_dict["source"] == '' or len(comgw_dict["destination"]) == 0:
                break
            com_data_obj.update_com_signalgw_info_from_signal(comgw_dict, comgw_cnt)
            comgw_dict["destination"] = []
            comgw_cnt = comgw_cnt + 1

    def update_com_data_from_add_dbc(self, config_dict, com_data_obj):
        com_config_json_dict = get_config_json_template_input_dbc_list(config_dict)
        self.com_dbc_config = com_config_json_dict["config"]
        self.ipdu_group_refs = com_config_json_dict["ipdu_group_refs"]
        self.com_mainfunction_coreId_refs.update(com_config_json_dict["com_mainfunction_coreId_refs"])
        for comm_diag_name, value in self.com_dbc_config.items():
            self.node_selected = value["node_selected"]
            for message in value["details"]:
                group_signals_list = []
                signal_groups_list = []
                siganl_groups_brief_info_dict = com_data_obj.update_com_signal_groups_info_from_message(message)
                for key, group_signals in siganl_groups_brief_info_dict.items():
                    signal_groups_list.append(key)
                    group_signals_list.extend(group_signals)
                if 0 == len(message["signals"]):
                    continue
                com_data_obj.update_com_ipdu_info_from_message(message, signal_groups_list, group_signals_list,
                                                               self.node_selected, self.ipdu_handle_id, value["coreId"], self.com_mainfunction_coreId_refs, False)
                self.ipdu_handle_id += 1
                for i in range(len(message["signals"])):
                    if message["signals"][i]["signal_name"] not in group_signals_list:
                        com_data_obj.update_com_signal_info_from_message(message["signals"][i])
        com_data_obj.update_com_general_info_from_message(self.ipdu_group_refs, self.supported_ipdu_groups)
        com_data_obj.update_com_ipdu_groups_info_from_message(self.ipdu_group_refs, self.supported_ipdu_groups)
        # 多核部署场景下才需要推导创建mainfunction
        if len(self.com_mainfunction_coreId_refs) > 1:
            com_data_obj.updata_com_com_mainfunction_info_from_message(self.com_mainfunction_coreId_refs)

    def _creat_com_sub_container(self, parent_container, container_name, container_mod_name, value_dict,
                                 initial_autosar):
        created_container = None
        ecuc_autosar = initial_autosar
        ecuc_def = self.mod_autosar_dict["Com"]
        parent_path = get_definition_ref(parent_container)
        container_path = f"{parent_path}/{container_name}".replace("/AUTOSAR/EcucDefs", "/ArPackages/InitialEcuC")
        key_list = [key for key in container_path.split("/") if key != ""]
        had_container = _check_had_container(self.module_autosar_dict, key_list)
        if had_container:
            return created_container
        if ecuc_def and parent_path and container_name and container_mod_name and isinstance(value_dict, dict):
            cur_conf_def = ecuc_def.find(
                parent_path.replace("/MICROSAR", "/AUTOSAR/EcucDefs") + f"/{container_mod_name}")
            created_container = AutosarUtil.create_sub_container_value(container_name, cur_conf_def, parent_container)
            self._set_com_container_obj_value_by_def_ref(created_container, value_dict)
        return created_container

    def _set_com_container_obj_value_by_def_ref(self, container_obj, value_dict):
        ecuc_def = self.mod_autosar_dict["Com"]
        if not container_obj or not value_dict:
            return
        if container_obj.REFERENCE_VALUES and container_obj.REFERENCE_VALUES.ECUC_REFERENCE_VALUE:
            ref_num = len(container_obj.REFERENCE_VALUES.ECUC_REFERENCE_VALUE)
            for i in range(ref_num):
                para = container_obj.REFERENCE_VALUES.ECUC_REFERENCE_VALUE[i]
                def_ref = get_definition_ref(para)
                cur_conf_def = ecuc_def.find(def_ref.replace("/MICROSAR", "/AUTOSAR/EcucDefs"))
                if def_ref not in value_dict.keys():
                    continue
                if isinstance(value_dict[def_ref]["Value"], list):
                    if len(value_dict[def_ref]["Value"]):
                        set_value_ref(para, str(value_dict[def_ref]["Value"][0]))
                        AutosarUtil.add_annotation_value(para, "AutoDerived")
                        for index in range(len(value_dict[def_ref]["Value"]) - 1):
                            obj = AutosarUtil.set_reference(container_obj.REFERENCE_VALUES, cur_conf_def)
                            set_value_ref(obj, str(value_dict[def_ref]["Value"][index + 1]))
                            AutosarUtil.add_annotation_value(obj, "AutoDerived")
                else:
                    set_value_ref(para, str(value_dict[def_ref]["Value"]))
                    AutosarUtil.add_annotation_value(para, "AutoDerived")
        if container_obj.PARAMETER_VALUES and container_obj.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE:
            for para in container_obj.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE:
                def_ref = get_definition_ref(para)
                if def_ref in value_dict.keys():
                    if value_dict[def_ref]["Value"] != None:
                        set_value(para, str(value_dict[def_ref]["Value"]))
                        AutosarUtil.add_annotation_value(para, "AutoDerived")
        if container_obj.PARAMETER_VALUES and container_obj.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE:
            for para in container_obj.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE:
                def_ref = get_definition_ref(para)
                if def_ref in value_dict.keys():
                    set_value(para, str(value_dict[def_ref]["Value"]))
                    AutosarUtil.add_annotation_value(para, "AutoDerived")

    def sort_com_ipduhandleid(self, dict_value, inithandle):
        ipdu_dict_list = []
        key_temp = "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComIPduDirection"
        key_temp1 = "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxModeTrue/ComTxMode/ComTxModeMode"
        key_temp2 = "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxModeTrue/ComTxMode/ComTxModeTimePeriod"
        for com_ipdu_l1_name, com_ipdu_l1_value in dict_value.items():
            if com_ipdu_l1_name in ["Tag", "DefinitionRef", "Description"] or not isinstance(com_ipdu_l1_value, dict):
                continue
            ipdu_dict = {'name': com_ipdu_l1_name}
            if _get_dict_value_by_nested_keys(com_ipdu_l1_value, [key_temp, "Value"]) == "SEND":
                ipdu_dict.update({"condition1": 0})
                if _get_dict_value_by_nested_keys(com_ipdu_l1_value["ComTxIPdu"]["ComModeTrue"],
                                                  [key_temp1, "Value"]) != "DIRECT":
                    ipdu_dict.update({"condition2": 0})
                    ipdu_dict.update({"condition3": com_ipdu_l1_value['ComTxIPdu']['ComModeTrue'][key_temp2]['Value']})
                else:
                    ipdu_dict.update({"condition2": 1})
                    ipdu_dict.update({"condition3": 0})
            else:
                ipdu_dict.update({"condition1": 1})
                ipdu_dict.update({"condition2": 0})
                ipdu_dict.update({"condition3": 0})
            ipdu_dict_list.append(ipdu_dict)
        sorted_ipdu_list = sorted(ipdu_dict_list, key=lambda s: (s['condition1'], s['condition2'], s['condition3']))

        for index, item in enumerate(sorted_ipdu_list):
            if item["name"] in dict_value.keys():
                dict_value[item["name"]]['/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComIPduHandleId'][
                    'Value'] = index + inithandle
        return

    def update_com_arxml_ipdus(self, initial_autosar, com_data_obj):
        ecuc_autosar = initial_autosar
        parent = ecuc_autosar.find("/InitialEcuC/Com/ComConfig")
        dict_value = _get_dict_value_by_nested_keys(com_data_obj.data, ["ComConfig", "ComIPdus"])
        if not isinstance(dict_value, dict):
            dict_value = {}

        self.sort_com_ipduhandleid(dict_value, self.ipdu_init_handle_id)

        for com_ipdu_l1_name, com_ipdu_l1_value in dict_value.items():
            if com_ipdu_l1_name in ["Tag", "DefinitionRef", "Description"] or not isinstance(com_ipdu_l1_value, dict):
                continue
            container_value = self._creat_com_sub_container(parent, com_ipdu_l1_name, "ComIPdu", com_ipdu_l1_value,
                                                            initial_autosar)

            key_temp = "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComIPduDirection"
            if _get_dict_value_by_nested_keys(com_ipdu_l1_value, [key_temp, "Value"]) == "SEND":
                if _get_dict_value_by_nested_keys(com_ipdu_l1_value, ["ComTxIPdu"]):
                    com_tx_ipdu_container = self._creat_com_sub_container(container_value, "ComTxIPdu", "ComTxIPdu",
                                                                          com_ipdu_l1_value["ComTxIPdu"],
                                                                          initial_autosar)
                    com_tx_mode_true_container = self._creat_com_sub_container(
                        com_tx_ipdu_container, "ComTxModeTrue", "ComTxModeTrue",
                        com_ipdu_l1_value["ComTxIPdu"]["ComModeTrue"], initial_autosar)
                    com_tx_mode_container = self._creat_com_sub_container(com_tx_mode_true_container, "ComTxMode",
                                                                          "ComTxMode",
                                                                          com_ipdu_l1_value["ComTxIPdu"]["ComModeTrue"],
                                                                          initial_autosar)

                    com_tx_mode_false_container = self._creat_com_sub_container(
                        com_tx_ipdu_container, "ComTxModeFalse", "ComTxModeFalse",
                        com_ipdu_l1_value["ComTxIPdu"]["ComModeFalse"], initial_autosar)
                    com_tx_mode_container = self._creat_com_sub_container(
                        com_tx_mode_false_container, "ComTxMode", "ComTxMode",
                        com_ipdu_l1_value["ComTxIPdu"]["ComModeFalse"], initial_autosar)

    def update_com_arxml_signals(self, initial_autosar, com_data_obj):
        ecuc_autosar = initial_autosar
        parent = ecuc_autosar.find("/InitialEcuC/Com/ComConfig")
        dict_value = _get_dict_value_by_nested_keys(com_data_obj.data, ["ComConfig", "ComSignals"])
        if not isinstance(dict_value, dict):
            dict_value = {}
        for com_signal_name, com_signal_value in dict_value.items():
            if com_signal_name in ["Tag", "DefinitionRef", "Description"] or not isinstance(com_signal_value, dict):
                continue
            container_value = self._creat_com_sub_container(parent, com_signal_name, "ComSignal", com_signal_value,
                                                            initial_autosar)

    def update_com_arxml_signalgw(self, initial_autosar, com_data_obj):
        ecuc_autosar = initial_autosar
        parent = ecuc_autosar.find("/InitialEcuC/Com/ComConfig")
        dict_value = _get_dict_value_by_nested_keys(com_data_obj.data, ["ComConfig", "ComGwMapping"])
        if not isinstance(dict_value, dict):
            dict_value = {}
        for com_gw_name, com_gw_value in dict_value.items():
            if com_gw_name in ["Tag", "DefinitionRef", "Description"] or not isinstance(com_gw_value, dict):
                continue
            container_value = self._creat_com_sub_container(parent, com_gw_name, "ComGwMapping", com_gw_value,
                                                            initial_autosar)
            for comgwkey_name, comgwkey_value in com_gw_value.items():
                if comgwkey_name in ["Tag", "DefinitionRef", "Description"] or not isinstance(comgwkey_value, dict):
                    continue
                if (comgwkey_name == "ComGwSource"):
                    container_value_source = self._creat_com_sub_container(container_value, comgwkey_name,
                                                                           "ComGwSource", comgwkey_value,
                                                                           initial_autosar)
                    container_value_sourcesignal = self._creat_com_sub_container(container_value_source, "ComGwSignal",
                                                                                 "ComGwSignal",
                                                                                 comgwkey_value["ComGwSignal"],
                                                                                 initial_autosar)

                else:
                    container_value_destination = self._creat_com_sub_container(container_value, comgwkey_name,
                                                                                "ComGwDestination", comgwkey_value,
                                                                                initial_autosar)
                    container_value_dssignal = self._creat_com_sub_container(container_value_destination, "ComGwSignal",
                                                                             "ComGwSignal",
                                                                             comgwkey_value["ComGwSignal"],
                                                                             initial_autosar)

    def update_com_arxml_signal_groups(self, initial_autosar, com_data_obj):
        ecuc_autosar = initial_autosar
        parent = ecuc_autosar.find("/InitialEcuC/Com/ComConfig")
        dict_value = _get_dict_value_by_nested_keys(com_data_obj.data, ["ComConfig", "ComSignalGroups"])
        if not isinstance(dict_value, dict):
            dict_value = {}
        for com_signal_group_name, com_signal_group_value in dict_value.items():
            if com_signal_group_name in ["Tag", "DefinitionRef", "Description"
                                        ] or not isinstance(com_signal_group_value, dict):
                continue
            container_value = self._creat_com_sub_container(parent, com_signal_group_name, "ComSignalGroup",
                                                            com_signal_group_value, initial_autosar)
            signals_in_group = _get_dict_value_by_nested_keys(com_signal_group_value, ["ComGroupSignals"])
            if not isinstance(signals_in_group, dict):
                signals_in_group = {}
            for signal_name, signal_value in signals_in_group.items():
                if signal_name in ["Tag", "DefinitionRef", "Description"] or not isinstance(signal_value, dict):
                    continue
                com_group_signal_container = self._creat_com_sub_container(container_value, signal_name,
                                                                           "ComGroupSignal", signal_value,
                                                                           initial_autosar)
    
    def update_com_arxml_mainfunctions(self, initial_autosar, com_data_obj):
        ecuc_autosar = initial_autosar
        parent = ecuc_autosar.find("/InitialEcuC/Com/ComConfig")

        dict_value = _get_dict_value_by_nested_keys(com_data_obj.data, ["ComConfig", "ComMainFunctionTx"])
        if not isinstance(dict_value, dict):
            dict_value = {}
        for com_mainfunction_name, com_mainfunction_value in dict_value.items():
            if com_mainfunction_name in ["Tag", "DefinitionRef", "Description"] and not isinstance(com_mainfunction_value, dict):
                continue
            container_value = self._creat_com_sub_container(parent, com_mainfunction_name, "ComMainFunctionTx", com_mainfunction_value,
                                                            initial_autosar)
        

        dict_value = _get_dict_value_by_nested_keys(com_data_obj.data, ["ComConfig", "ComMainFunctionRx"])
        if not isinstance(dict_value, dict):
            dict_value = {}
        for com_mainfunction_name, com_mainfunction_value in dict_value.items():
            if com_mainfunction_name in ["Tag", "DefinitionRef", "Description"] and not isinstance(com_mainfunction_value, dict):
                continue
            container_value = self._creat_com_sub_container(parent, com_mainfunction_name, "ComMainFunctionRx", com_mainfunction_value,
                                                            initial_autosar)

    def update_com_arxml_ipdu_groups(self, initial_autosar, com_data_obj):
        ecuc_autosar = initial_autosar
        parent = ecuc_autosar.find("/InitialEcuC/Com/ComConfig")

        dict_value = _get_dict_value_by_nested_keys(com_data_obj.data, ["ComConfig", "ComIPduGroups"])
        if not isinstance(dict_value, dict):
            dict_value = {}
        for ipdu_group_name, ipdu_group_value in dict_value.items():
            if ipdu_group_name in ["Tag", "DefinitionRef", "Description"] and not isinstance(ipdu_group_value, dict):
                continue
            container_value = self._creat_com_sub_container(parent, ipdu_group_name, "ComIPduGroup", ipdu_group_value,
                                                            initial_autosar)

    def update_com_arxml_supported_ipdu_groups(self, initial_autosar, com_data_obj):
        com_general_dict = _get_dict_value_by_nested_keys(com_data_obj.data, ["ComGeneral", "Parameters"])
        ecuc_autosar = initial_autosar
        com_general = ecuc_autosar.find("/InitialEcuC/Com/ComGeneral")
        if com_general_dict is None or com_general is None:
            return
        self._set_com_container_obj_value_by_def_ref(com_general, com_general_dict)

class ComModuleInitData():

    def __init__(self):
        self.data = {
            "Tag": "ECUC-MODULE-CONFIGURATION-VALUES",
            "DefinitionRef": "/AUTOSAR/EcucDefs/Com",
            "ComGeneral": {
                "Tag": "ECUC-CONTAINER-VALUE",
                "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComGeneral",
                "Description": "General configuration of Com module",
                "Parameters": {
                    "/AUTOSAR/EcucDefs/Com/ComGeneral/ComSupportedIPduGroups": {
                        "Tag": "ECUC-NUMERICAL-PARAM-VALUE",
                        "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComGeneral/ComSupportedIPduGroups",
                        "Description": "Max Number of IPdu Groups",
                        "Value": 0
                    }
                }
            },
            "ComConfig": {
                "Description": "Data configuration of Com module",
                "ComIPduGroups": {
                    "Tag": "ECUC-CONTAINER-VALUE",
                    "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPduGroup",
                    "Description": "list of Ipdu Groups' name",
                },
                "ComIPdus": {
                    "Description": "All I-PDU",
                    "Tag": "ECUC-CONTAINER-VALUE",
                    "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu",
                },
                "ComSignals": {
                    "Tag": "ECUC-CONTAINER-VALUE",
                    "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig",
                    "Description": "All signals belongs to this message",
                },
                "ComSignalGroups": {
                    "Tag": "ECUC-CONTAINER-VALUE",
                    "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig",
                    "Description": "Signal Groups",
                },
                "ComGwMapping": {
                    "Tag": "ECUC-CONTAINER-VALUE",
                    "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig",
                    "Description": "Signal GateWay",
                },
                "ComMainFunctionTx": {
                    "Tag": "ECUC-CONTAINER-VALUE",
                    "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComMainFunctionTx",
                    "Description": "list of ComMainFunctionTx' name",
                },
                "ComMainFunctionRx": {
                    "Tag": "ECUC-CONTAINER-VALUE",
                    "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComMainFunctionRx",
                    "Description": "list of ComMainFunctionRx' name",
                }
            }
        }

    def update_com_signal_info_from_message(self, signal):
        signal_name = signal["signal_name"] + "_" + signal["direction"]
        if signal_name not in self.data["ComConfig"]["ComSignals"]:
            self.data["ComConfig"]["ComSignals"][signal_name] = {
                "Description": "signal object",
                "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignal",
                "ShortName": {
                    "Description": "Signal Name",
                    "Value": signal_name
                },
                "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignal/ComBitPosition": {
                    "Tag": "ECUC-NUMERICAL-PARAM-VALUE",
                    "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignal/ComBitPosition",
                    "Description": "Start Bit Position",
                    "Value": signal["bit_position"]
                },
                "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignal/ComBitSize": {
                    "Tag": "ECUC-NUMERICAL-PARAM-VALUE",
                    "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignal/ComBitSize",
                    "Description": "Signal Length in bit",
                    "Value": signal["signal_size"]
                },
                "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignal/ComSignalLength": {
                    "Tag": "ECUC-NUMERICAL-PARAM-VALUE",
                    "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignal/ComSignalLength",
                    "Description": "Signal Length in byte",
                    "Value": signal["ComSignalLength"]
                },
                "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignal/ComUpdateBitPosition": {
                    "Tag": "ECUC-NUMERICAL-PARAM-VALUE",
                    "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignal/ComUpdateBitPosition",
                    "Description": "Update Bit Position",
                    "Value": signal["UpdateBit_Position"]
                },
                "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignal/ComSignalEndianness": {
                    "Tag": "ECUC-TEXTUAL-PARAM-VALUE",
                    "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignal/ComSignalEndianness",
                    "Description": "Byte Order",
                    "Range": ["BIG_ENDIAN", "LITTLE_ENDIAN", "OPAQUE"],
                    "Value": signal["byte_order"]
                },
                "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignal/ComSignalType": {
                    "Tag": "ECUC-TEXTUAL-PARAM-VALUE",
                    "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignal/ComSignalType",
                    "Description": "Signal Type",
                    "Range": [
                        "BOOLEAN", "UINT8", "UINT16", "UINT32", "UINT64", "UINT8_N", "UINT8_DYN", "SINT8", "SINT16",
                        "SINT32", "SINT64", "FLOAT32", "FLOAT64"
                    ],
                    "Value": signal["signal_type"]
                },
                "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignal/ComTransferProperty": {
                    "Tag": "ECUC-TEXTUAL-PARAM-VALUE",
                    "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignal/ComTransferProperty",
                    "Description": "Signal Transfer Property",
                    "Range": [
                        "PENDING", "TRIGGERED", "TRIGGERED_WITHOUT_REPETITION",
                        "TRIGGERED_ON_CHANGE_WITHOUT_REPETITION", "TRIGGERED_ON_CHANGE"
                    ],
                    "Value": signal["gen_sig_send_type"]
                },
                "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignal/ComSignalInitValue": {
                    "Tag": "ECUC-TEXTUAL-PARAM-VALUE",
                    "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignal/ComSignalInitValue",
                    "Description": "Init Value",
                    "Value": signal["init_value"]
                },
            }

    def update_com_signal_groups_info_from_message(self, message):
        siganl_groups_brief_info_dict = {}
        for i in range(len(message["signal_groups"])):
            siganl_groups_name = message["signal_groups"][i]["group_name"] + "_o" + message[
                "message_name"] + "_" + message["signal_groups"][i]["direction"]
            self.data["ComConfig"]["ComSignalGroups"][siganl_groups_name] = {
                "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignalGroup",
                "ShortName": {
                    "Tag": "ECUC-CONTAINER-VALUE",
                    "Description": "Short Name",
                    "Value": siganl_groups_name
                },
                "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignalGroup/ComUpdateBitPosition": {
                    "Tag": "ECUC-NUMERICAL-PARAM-VALUE",
                    "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignalGroup/ComUpdateBitPosition",
                    "Description": "Update Bit Position",
                    "Value": 0
                },
                "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignalGroup/ComTransferProperty": {
                    "Tag": "ECUC-NUMERICAL-PARAM-VALUE",
                    "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignalGroup/ComTransferProperty",
                    "Description": "Signal Transfer Property",
                    "Range": [
                        "PENDING", "TRIGGERED", "TRIGGERED_WITHOUT_REPETITION",
                        "TRIGGERED_ON_CHANGE_WITHOUT_REPETITION", "TRIGGERED_ON_CHANGE"
                    ],
                    "Value": "PENDING"
                },
                "ComGroupSignals": {
                    "Description": "Groups Signals of SignalGroup",
                }
            }
            group_signals = {}
            siganl_groups_brief_info_dict[siganl_groups_name] = []
            for j in range(len(message["signal_groups"][i]["group_signals_name"])):
                signal_name = message["signal_groups"][i]["group_signals_name"][j] + "_o" + message["message_name"]
                for signal_info in message["signals"]:
                    if signal_name != signal_info["signal_name"]:
                        continue
                    signal_name_alias = signal_name + "_" + message["signal_groups"][i]["direction"]
                    siganl_groups_brief_info_dict[siganl_groups_name].append(signal_name)
                    if signal_info["gen_sig_send_type"] in {"TRIGGERED_WITHOUT_REPETITION", "TRIGGERED", "TRIGGERED_ON_CHANGE_WITHOUT_REPETITION" }:
                        signal_info["gen_sig_send_type"] = "PENDING"
                    # siganl_groups_dict[siganl_groups_name].append(signal_name)
                    group_signals[signal_name_alias] = {
                        "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignalGroup",
                        "ShortName": {
                            "Tag": "ECUC-CONTAINER-VALUE",
                            "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignalGroup/ComGroupSignal",
                            "Description": "Signal Name",
                            "Value": signal_name_alias
                        },
                        "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignalGroup/ComGroupSignal/ComBitPosition": {
                            "Tag":
                                "ECUC-NUMERICAL-PARAM-VALUE",
                            "DefinitionRef":
                                "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignalGroup/ComGroupSignal/ComBitPosition",
                            "Description":
                                "Start Bit Position",
                            "Value":
                                signal_info["bit_position"]
                        },
                        "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignalGroup/ComGroupSignal/ComBitSize": {
                            "Tag": "ECUC-NUMERICAL-PARAM-VALUE",
                            "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignalGroup/ComGroupSignal/ComBitSize",
                            "Description": "Signal Length in bit",
                            "Value": signal_info["signal_size"]
                        },
                        "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignalGroup/ComGroupSignal/ComSignalLength": {
                            "Tag":
                                "ECUC-NUMERICAL-PARAM-VALUE",
                            "DefinitionRef":
                                "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignalGroup/ComGroupSignal/ComSignalLength",
                            "Description":
                                "Signal Length in byte",
                            "Value":
                                signal_info["ComSignalLength"]
                        },
                        "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignalGroup/ComGroupSignal/ComSignalEndianness": {
                            "Tag":
                                "ECUC-TEXTUAL-PARAM-VALUE",
                            "DefinitionRef":
                                "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignalGroup/ComGroupSignal/ComSignalEndianness",
                            "Description":
                                "Byte Order",
                            "Range": ["BIG_ENDIAN", "LITTLE_ENDIAN", "OPAQUE"],
                            "Value":
                                "BIG_ENDIAN"
                        },
                        "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignalGroup/ComGroupSignal/ComSignalType": {
                            "Tag":
                                "ECUC-TEXTUAL-PARAM-VALUE",
                            "DefinitionRef":
                                "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignalGroup/ComGroupSignal/ComSignalType",
                            "Description":
                                "Signal Type",
                            "Range": [
                                "BOOLEAN", "UINT8", "UINT16", "UINT32", "UINT64", "UINT8_N", "UINT8_DYN", "SINT8",
                                "SINT16", "SINT32", "SINT64", "FLOAT32", "FLOAT64"
                            ],
                            "Value":
                                signal_info["signal_type"]
                        },
                        "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignalGroup/ComGroupSignal/ComTransferProperty": {
                            "Tag":
                                "ECUC-TEXTUAL-PARAM-VALUE",
                            "DefinitionRef":
                                "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignalGroup/ComGroupSignal/ComTransferProperty",
                            "Description":
                                "Signal Transfer Property",
                            "Range": [
                                "PENDING", "TRIGGERED", "TRIGGERED_WITHOUT_REPETITION",
                                "TRIGGERED_ON_CHANGE_WITHOUT_REPETITION", "TRIGGERED_ON_CHANGE"
                            ],
                            "Value":
                                signal_info["gen_sig_send_type"]
                        },
                        "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignalGroup/ComGroupSignal/ComSignalInitValue": {
                            "Tag":
                                "ECUC-TEXTUAL-PARAM-VALUE",
                            "DefinitionRef":
                                "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignalGroup/ComGroupSignal/ComSignalInitValue",
                            "Description":
                                "Init Value",
                            "Value":
                                signal_info["init_value"]
                        },
                    }
            # siganl_groups_brief_info_dict[siganl_groups_name] = list(group_signals.keys())
            self.data["ComConfig"]["ComSignalGroups"][siganl_groups_name]["ComGroupSignals"].update(group_signals)
        return siganl_groups_brief_info_dict

    def update_com_ipdu_info_from_message(self, message, signal_group_refs, group_signals_list, node_selected,
                                          ipdu_handle_id, coreId, com_mainfunction_coreId_refs, islinmsg):
        dir = None
        idpu_group_ref = None
        ipdu_signal_refs = []
        signal_group_refs_full = []
        ipdu_name = message["message_name"] + "_" + message["signals"][0]["direction"]
        for ref in signal_group_refs:
            signal_group_refs_full.append(f"/ActiveEcuC/Com/ComConfig/{ref}")
        if ipdu_name not in self.data["ComConfig"]["ComIPdus"]:
            if message["senders"] == node_selected:
                dir = "SEND"
                idpu_group_ref = message['bus_name'] + "_Tx"
                com_pdu_id_ref = message['message_name'] + "_oCom_PduR_Tx"
                # 当只有一个core部署  并且部署的是Core0
                if len(com_mainfunction_coreId_refs) == 1:
                    com_mainfunction_ref = "Com_MainFunction_Tx"
                else:
                    com_mainfunction_ref = "Com_MainFunction_Tx" + "_Core" + str(coreId)
            else:
                idpu_group_ref = message['bus_name'] + "_Rx"
                dir = "RECEIVE"
                com_pdu_id_ref = message['message_name'] + "_oPduR_Com_Rx"
                # 当只有一个core部署  并且部署的是Core0
                if len(com_mainfunction_coreId_refs) == 1:
                    com_mainfunction_ref = "Com_MainFunction_Rx"
                else:
                    com_mainfunction_ref = "Com_MainFunction_Rx" + "_Core" + str(coreId)
            for sig in message["signals"]:
                if sig["signal_name"] not in group_signals_list:
                    ipdu_signal_refs.append("/ActiveEcuC/Com/ComConfig/" + sig["signal_name"] + "_" + sig["direction"])
            if "pdu_type" in message and message["pdu_type"] == "cannm":
                com_pdu_id_ref = message['message_name'] + "_oCom_CanNm_Tx"
            com_pdu_id_ref = f"/ActiveEcuC/EcuC/EcucConfigSet/EcucPduCollection/{com_pdu_id_ref}"
            com_mainfunction_ref = f"/ActiveEcuC/Com/ComConfig/{com_mainfunction_ref}"
            if islinmsg == True:
                ComTxIPduUnusedAreasDefault = 255
            else:
                ComTxIPduUnusedAreasDefault = 0

            # if _check_had_container(ecuc_ref_dict, ["ArPackages", "ActiveEcuC", "EcuC", "EcucConfigSet", "EcucPduCollection", com_pdu_id_ref]):
            #     pass
            # else:
            #     com_pdu_id_ref = ""
            self.data["ComConfig"]["ComIPdus"][ipdu_name] = {
                "Description": "message object",
                "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu",
                "ShortName": {
                    "Description": "Message_Name",
                    "Value": ipdu_name
                },
                "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComIPduDirection": {
                    "Tag": "ECUC-TEXTUAL-PARAM-VALUE",
                    "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComIPduDirection",
                    "Description": "IPdu Direction",
                    "Value": dir
                },
                "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComIPduHandleId": {
                    "Tag": "ECUC-TEXTUAL-PARAM-VALUE",
                    "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComIPduHandleId",
                    "Description": "IPdu id",
                    "Value": ipdu_handle_id
                },
                "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComIPduSize": {
                    "Tag": "ECUC-TEXTUAL-PARAM-VALUE",
                    "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComIPduSize",
                    "Description": "Ways of dealing with signal",
                    "Description": "Message DLC",
                    "Value": message["message_size"]
                },
                "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComIPduGroupRef": {
                    "Tag": "ECUC-REFERENCE-VALUE",
                    "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComIPduGroupRef",
                    "Description": "IPDU Groups which this message belongs to. Value is the IPDU Groups' name.",
                    "Value": f"/ActiveEcuC/Com/ComConfig/{idpu_group_ref}"
                },
                "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComIPduSignalGroupRef": {
                    "Tag": "ECUC-REFERENCE-VALUE",
                    "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComIPduSignalGroupRef",
                    "Description": "IPDU Signal Groups",
                    "Value": signal_group_refs_full
                },
                "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComIPduSignalRef": {
                    "Tag": "ECUC-REFERENCE-VALUE",
                    "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComIPduSignalRef",
                    "Description": "IPDU Signal Refs.",
                    "Value": ipdu_signal_refs
                },
                "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComPduIdRef": {
                    "Tag":
                        "ECUC-REFERENCE-VALUE",
                    "DefinitionRef":
                        "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComPduIdRef",
                    "Description":
                        "Reference to the global Pdu structure to allow harmonization of handle IDs in the COM-Stack",
                    "Value":
                        com_pdu_id_ref
                },
                "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/IPduMainFunctionRef": {
                    "Tag":
                        "ECUC-REFERENCE-VALUE",
                    "DefinitionRef":
                        "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/IPduMainFunctionRef",
                    "Description":
                        "Reference to the global Pdu structure to allow harmonization of MainFunction in the COM-Stack",
                    "Value":
                        com_mainfunction_ref
                },
                "ComTxIPdu": {
                    "Tag": "ECUC-CONTAINER-VALUE",
                    "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu",
                    "Description": "I-PDU Tx Config",
                    "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComMinimumDelayTime": {
                        "Tag": "ECUC-NUMERICAL-PARAM-VALUE",
                        "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComMinimumDelayTime",
                        "Description": "Minimum Tx Delay Time",
                        "Value": message["gen_msg_delay_time"]
                    },
                    "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxIPduClearUpdateBit": {
                        "Tag": "ECUC-TEXTUAL-PARAM-VALUE",
                        "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxIPduClearUpdateBit",
                        "Description": "When will COM clear Update Bit",
                        "Range": ["Transmit", "Confirmation", "Triggertransmit"],
                        "Value": "Transmit"
                    },
                    "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxIPduUnusedAreasDefault": {
                        "Tag": "ECUC-NUMERICAL-PARAM-VALUE",
                        "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxIPduUnusedAreasDefault",
                        "Description": "Com TxIPdu UnusedAreas DefaultValue",
                        "Value": ComTxIPduUnusedAreasDefault
                    },
                    "ComModeTrue": {
                        "Tag": "ECUC-CONTAINER-VALUE",
                        "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxModeTrue",
                        "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxModeTrue/ComTxMode/ComTxModeMode": {
                            "Tag":
                                "ECUC-TEXTUAL-PARAM-VALUE",
                            "DefinitionRef":
                                "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxModeTrue/ComTxMode/ComTxModeMode",
                            "Description":
                                "Message Transmit Mode",
                            "Range": ["NONE", "DIRECT", "MIXED", "PERIODIC"],
                            "Value":
                                message["gen_msg_send_type"]
                        },
                        "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxModeTrue/ComTxMode/ComTxModeNumberOfRepetitions": {
                            "DefinitionRef":
                                "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxModeTrue/ComTxMode/ComTxModeNumberOfRepetitions",
                            "Description":
                                "Transmit Times",
                            "Value":
                                message["gen_msg_nr_of_repetition"]
                        },
                        "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxModeTrue/ComTxMode/ComTxModeRepetitionPeriod": {
                            "DefinitionRef":
                                "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxModeTrue/ComTxMode/ComTxModeRepetitionPeriod",
                            "Description":
                                "Repetition Transmit Delay Time",
                            "Value":
                                message["gen_msg_cycle_time_fast"]
                        },
                        "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxModeTrue/ComTxMode/ComTxModeTimeOffset": {
                            "Tag":
                                "ECUC-NUMERICAL-PARAM-VALUE",
                            "DefinitionRef":
                                "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxModeTrue/ComTxMode/ComTxModeTimeOffset",
                            "Description":
                                "Time before first transmission of this IPDU",
                            "Value":
                                message["gen_msg_start_delay_time"]
                        },
                        "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxModeTrue/ComTxMode/ComTxModeTimePeriod": {
                            "Tag":
                                "ECUC-NUMERICAL-PARAM-VALUE",
                            "DefinitionRef":
                                "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxModeTrue/ComTxMode/ComTxModeTimePeriod",
                            "Description":
                                "Period of cyclic transmission",
                            "Value":
                                message["gen_msg_cycle_time"]
                        }
                    },
                    "ComModeFalse": {
                        "Tag": "ECUC-CONTAINER-VALUE",
                        "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxModeFalse",
                        "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxModeFalse/ComTxMode/ComTxModeMode": {
                            "Tag":
                                "ECUC-TEXTUAL-PARAM-VALUE",
                            "DefinitionRef":
                                "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxModeFalse/ComTxMode/ComTxModeMode",
                            "Description":
                                "Message Transmit Mode",
                            "Range": ["NONE", "DIRECT", "MIXED", "PERIODIC"],
                            "Value":
                                message["gen_msg_send_type"]
                        },
                        "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxModeFalse/ComTxMode/ComTxModeNumberOfRepetitions": {
                            "Tag":
                                "ECUC-NUMERICAL-PARAM-VALUE",
                            "DefinitionRef":
                                "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxModeFalse/ComTxMode/ComTxModeNumberOfRepetitions",
                            "Description":
                                "Transmit Times",
                            "Value":
                                message["gen_msg_nr_of_repetition"]
                        },
                        "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxModeFalse/ComTxMode/ComTxModeRepetitionPeriod": {
                            "Tag":
                                "ECUC-NUMERICAL-PARAM-VALUE",
                            "DefinitionRef":
                                "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxModeFalse/ComTxMode/ComTxModeRepetitionPeriod",
                            "Description":
                                "Repetition Transmit Delay Time",
                            "Value":
                                message["gen_msg_cycle_time_fast"]
                        },
                        "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxModeFalse/ComTxMode/ComTxModeTimeOffset": {
                            "Tag":
                                "ECUC-NUMERICAL-PARAM-VALUE",
                            "DefinitionRef":
                                "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxModeFalse/ComTxMode/ComTxModeTimeOffset",
                            "Description":
                                "Time before first transmission of this IPDU",
                            "Value":
                                message["gen_msg_start_delay_time"]
                        },
                        "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxModeFalse/ComTxMode/ComTxModeTimePeriod": {
                            "Tag":
                                "ECUC-NUMERICAL-PARAM-VALUE",
                            "DefinitionRef":
                                "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu/ComTxIPdu/ComTxModeFalse/ComTxMode/ComTxModeTimePeriod",
                            "Description":
                                "Period of cyclic transmission",
                            "Value":
                                message["gen_msg_cycle_time"]
                        }
                    }
                }
            }

    def update_com_general_info_from_message(self, ipdu_group_refs, supported_ipdu_groups):
        self.data["ComGeneral"]["Parameters"]["/AUTOSAR/EcucDefs/Com/ComGeneral/ComSupportedIPduGroups"]["Value"] = (
            supported_ipdu_groups + len(ipdu_group_refs) * 2)
        
    def updata_com_com_mainfunction_info_from_message(self, com_mainfunction_coreId_refs):
        for coreId in list(com_mainfunction_coreId_refs.keys()):
            # 当只部署一个Core 并且是Core0
            if len(com_mainfunction_coreId_refs) == 1:
                tx_mainFunction = "Com_MainFunction_Tx"
                rx_mainfunction = "Com_MainFunction_Rx"
            else:
                tx_mainFunction = "Com_MainFunction_Tx" + "_Core" + str(coreId)
                rx_mainfunction = "Com_MainFunction_Rx" + "_Core" + str(coreId)
            if tx_mainFunction not in self.data["ComConfig"]["ComMainFunctionTx"].keys():
                self.data["ComConfig"]["ComMainFunctionTx"][tx_mainFunction] = {
                    "Description": "ComMainFunctionTx",
                    "/AUTOSAR/EcucDefs/Com/ComConfig/ComMainFunctionTx/MainTxTimeBase": {
                        "Tag": "ECUC-NUMERICAL-PARAM-VALUE",
                        "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComMainFunctionTx/MainTxTimeBase",
                        "Description": "mainfunction tx timebase",
                        "Value": 0.005
                    },
                }
            if rx_mainfunction not in self.data["ComConfig"]["ComMainFunctionRx"].keys():
                self.data["ComConfig"]["ComMainFunctionRx"][rx_mainfunction] = {
                    "Description": "ComMainFunctionRx",
                    "/AUTOSAR/EcucDefs/Com/ComConfig/ComMainFunctionRx/MainRxTimeBase": {
                        "Tag": "ECUC-NUMERICAL-PARAM-VALUE",
                        "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComMainFunctionRx/MainRxTimeBase",
                        "Description": "mainfunction rx timebase",
                        "Value": 0.005
                    },
                }

    def update_com_ipdu_groups_info_from_message(self, ipdu_group_refs, ipdu_ipdugrouphandle_id):
        for ipdu_group in ipdu_group_refs:
            tx_group = ipdu_group + "_Tx"
            rx_group = ipdu_group + "_Rx"
            if tx_group not in self.data["ComConfig"]["ComIPduGroups"].keys():
                self.data["ComConfig"]["ComIPduGroups"][tx_group] = {
                    "Description": "IPdu Group",
                    "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPduGroup/ComIPduGroupHandleId": {
                        "Tag": "ECUC-NUMERICAL-PARAM-VALUE",
                        "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPduGroup/ComIPduGroupHandleId",
                        "Description": "IPdu Group id",
                        "Value": ipdu_ipdugrouphandle_id
                    },
                }
                ipdu_ipdugrouphandle_id += 1
            if rx_group not in self.data["ComConfig"]["ComIPduGroups"].keys():
                self.data["ComConfig"]["ComIPduGroups"][rx_group] = {
                    "Description": "IPdu Group",
                    "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPduGroup/ComIPduGroupHandleId": {
                        "Tag": "ECUC-NUMERICAL-PARAM-VALUE",
                        "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPduGroup/ComIPduGroupHandleId",
                        "Description": "IPdu Group id",
                        "Value": ipdu_ipdugrouphandle_id
                    },
                }
                ipdu_ipdugrouphandle_id += 1

    def update_com_signalgw_info_from_signal(self, comgw_dict, comgw_cnt):
        ComGwDestination = {}
        ComGwMapping_sname = "ComGwMapping_" + str(comgw_cnt)
        for i in range(len(comgw_dict['destination'])):
            signal_destination_name = comgw_dict['destination'][i]
            ComGwDestination_name = "ComGwDestination_" + str(i)
            ComGwDestination[ComGwDestination_name] = {
                "Tag": "ECUC-CONTAINER-VALUE",
                "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComGwMapping/ComGwDestination",
                "Description": "ComGwDestination",
                "ComGwSignal": {
                    "Tag": "ECUC-CONTAINER-VALUE",
                    "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComGwMapping/ComGwDestination/ComGwSignal",
                    "Description": "ComGwSignal",
                    "/AUTOSAR/EcucDefs/Com/ComConfig/ComGwMapping/ComGwDestination/ComGwSignal/ComGwSignalRef": {
                        "Tag":
                            "ECUC-TEXTUAL-PARAM-VALUE",
                        "DefinitionRef":
                            "/AUTOSAR/EcucDefs/Com/ComConfig/ComGwMapping/ComGwDestination/ComGwSignal/ComGwSignalRef",
                        "Description":
                            "ComGwSignalRef",
                        "Value":
                            f"/ActiveEcuC/Com/ComConfig/" + signal_destination_name
                    }
                }
            }

        self.data["ComConfig"]["ComGwMapping"][ComGwMapping_sname] = {
            "Description": "ComGwMapping object",
            "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComGwMapping",
            "ComGwSource": {
                "Tag": "ECUC-CONTAINER-VALUE",
                "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComGwMapping/ComGwSource",
                "Description": "ComGwSource",
                "ComGwSignal": {
                    "Tag": "ECUC-CONTAINER-VALUE",
                    "DefinitionRef": "/AUTOSAR/EcucDefs/Com/ComConfig/ComGwMapping/ComGwSource/ComGwSignal",
                    "Description": "ComGwSignal",
                    "/AUTOSAR/EcucDefs/Com/ComConfig/ComGwMapping/ComGwSource/ComGwSignal/ComGwSignalRef": {
                        "Tag":
                            "ECUC-TEXTUAL-PARAM-VALUE",
                        "DefinitionRef":
                            "/AUTOSAR/EcucDefs/Com/ComConfig/ComGwMapping/ComGwSource/ComGwSignal/ComGwSignalRef",
                        "Description":
                            "ComGwSignalRef",
                        "Value":
                            f"/ActiveEcuC/Com/ComConfig/" + comgw_dict["source"]
                    }
                }
            }
        }
        self.data["ComConfig"]["ComGwMapping"][ComGwMapping_sname].update(ComGwDestination)


def get_gen_msg_send_type(gen_msg_send_type_value, msg_dict, msg_type):
    """
    Get message send type.

    Arg:
        gen_msg_send_type_value: value from dbc file;
        msg_dict: dictionary that stores message.
    """

    if 0 == gen_msg_send_type_value or 2 == gen_msg_send_type_value:
        msg_dict["gen_msg_send_type"] = "PERIODIC"
    elif 1 == gen_msg_send_type_value or 3 == gen_msg_send_type_value or 6 == gen_msg_send_type_value:
        msg_dict["gen_msg_send_type"] = "DIRECT"
    elif 4 == gen_msg_send_type_value or 5 == gen_msg_send_type_value:
        msg_dict["gen_msg_send_type"] = "MIXED"
    elif 7 == gen_msg_send_type_value or 8 == gen_msg_send_type_value:
        msg_dict["gen_msg_send_type"] = "NONE"

    # 网络管理报文发送模式为None
    if msg_type == "cannm":
        msg_dict["gen_msg_send_type"] = "NONE"

def get_gen_sig_send_type(gen_sig_send_type_value, signal_dict):
    """
    Get singal send type.

    Arg:
        gen_sig_send_type_value: value from dbc file;
        signal_dict: dictionary that stores signal.
    """

    if 0 == gen_sig_send_type_value or 7 == gen_sig_send_type_value:
        signal_dict["gen_sig_send_type"] = "PENDING"
    elif 1 == gen_sig_send_type_value or 5 == gen_sig_send_type_value:
        signal_dict["gen_sig_send_type"] = "TRIGGERED_WITHOUT_REPETITION"
    elif 2 == gen_sig_send_type_value or 6 == gen_sig_send_type_value:
        signal_dict["gen_sig_send_type"] = "TRIGGERED"
    elif 3 == gen_sig_send_type_value:
        signal_dict["gen_sig_send_type"] = "TRIGGERED_ON_CHANGE_WITHOUT_REPETITION"
    elif 4 == gen_sig_send_type_value:
        signal_dict["gen_sig_send_type"] = "TRIGGERED_ON_CHANGE"


def get_dbc_signals(signal_dict, signal_value, msg_type, msg_value):
    """
    Get all signals from the dbc file.

    Arg:
        signal_dict: dictionary that stores signal;
        signal_value: The single signal obtained.
    """
    temp = ''
    signal_dict["signal_name"] = signal_value.signal_name
    signal_dict["signal_alias_name"] = signal_value.signal_alias_name
    signal_dict["UpdateBit_Position"] = 0
    if msg_type == "cannm":
        signal_dict["bit_position"] = signal_value.start - 16
    else:
        signal_dict["bit_position"] = signal_value.start
    signal_dict["signal_size"] = signal_value.length
    signal_dict["ComSignalLength"] = signal_dict["signal_size"] // 8 + int(bool(signal_dict["signal_size"] % 8))
    gen_sig_send_type_value = signal_value.signal_attributes["GenSigSendType"].value
    get_gen_sig_send_type(gen_sig_send_type_value, signal_dict)
    if "big_endian" == signal_value.byte_order:
        signal_dict["byte_order"] = "BIG_ENDIAN"
    elif "little_endian" == signal_value.byte_order:
        signal_dict["byte_order"] = "LITTLE_ENDIAN"
    else:
        signal_dict["byte_order"] = "OPAQUE"
    SignalLength = signal_dict["ComSignalLength"]
    if signal_value.initial == None:
        signal_value.initial = 0  #dbc 0*
    get_signal_type(signal_dict, signal_value)
    if signal_value.length > 32 and signal_dict["signal_type"] == "UINT8_N":
        if "LITTLE_ENDIAN" == signal_value.byte_order:
            for index in range(signal_dict["ComSignalLength"]):
                temp += str((signal_value.initial >> 8 * index) & 0xFF)
                temp += " "
        else:
            for index in range(signal_dict["ComSignalLength"]):
                temp += str((signal_value.initial >> 8 * (signal_dict["ComSignalLength"] - index - 1)) & 0xFF)
                temp += " "
        signal_dict["init_value"] = temp[:-1]
    else:
        signal_dict["init_value"] = signal_value.initial
    set_signal_updatebit(msg_value, signal_dict)


def get_signal_type(signal_dict, signal_value):
    """
    Get all signals from the dbc file.

    Arg:
        signal_dict: dictionary that stores signal;
        signal_value: The single signal obtained.
    """

    if True == signal_value.is_float:
        if signal_value.length <= 32:
            signal_dict["signal_type"] = "FLOAT32"
        else:
            signal_dict["signal_type"] = "FLOAT64"
    elif False == signal_value.is_signed and False == signal_value.is_float:
        get_unsigned_type(signal_dict, signal_value)
    elif True == signal_value.is_signed and False == signal_value.is_float:
        get_signed_type(signal_dict, signal_value)


def get_unsigned_type(signal_dict, signal_value):
    """
    Get an unsigned type.

    Arg:
        signal_dict: dictionary that stores signal;
        signal_value: The single signal obtained.
    """
    if signal_value.length == 1:
        signal_dict["signal_type"] = "BOOLEAN"
    elif signal_value.length <= 8 and signal_value.length > 1:
        signal_dict["signal_type"] = "UINT8"
    elif signal_value.length <= 16:
        signal_dict["signal_type"] = "UINT16"
    elif signal_value.length <= 32:
        signal_dict["signal_type"] = "UINT32"
    elif signal_value.length > 32 and signal_value.length <= 64 and bool(signal_dict["signal_size"] % 8):
        signal_dict["signal_type"] = "UINT64"
    else:
        if signal_dict["byte_order"] == "BIG_ENDIAN":
            # 获取ComEndBitPosition MSB
            ComBitPositionMsb = signal_dict["bit_position"]
            # 获取ComEndBitPosition LSB
            ComBitSize = signal_dict["signal_size"]
            ComLsbInx = ((ComBitPositionMsb ^ 0x7) + ComBitSize - 1) ^ 7  
            if bool(ComLsbInx % 8):
                logger.ui_error(f'ComUI: Signal ({signal_dict["signal_name"]}) is Parsed into UINT8_N, bit_position need be 8n (n >= 0)')
            if bool(ComBitSize % 8):
                logger.ui_error(f'ComUI: Signal ({signal_dict["signal_name"]}) is Parsed into UINT8_N, bit_size need be 8n (n >= 0)')
        else:
            ComBitSize = signal_dict["signal_size"]
            if bool(signal_dict["bit_position"] % 8):
                logger.ui_error(f'ComUI: Signal ({signal_dict["signal_name"]}) is Parsed into UINT8_N, bit_position need be 8n (n >= 0)')
            if bool(ComBitSize % 8):
                logger.ui_error(f'ComUI: Signal ({signal_dict["signal_name"]}) is Parsed into UINT8_N, bit_size need be 8n (n >= 0)')
        signal_dict["signal_type"] = "UINT8_N"
        signal_dict["signal_size"] = 0


def get_signed_type(signal_dict, signal_value):
    """
    Get an signed type.

    Arg:
        signal_dict: dictionary that stores signal;
        signal_value: The single signal obtained.
    """

    if signal_value.length == 1:
        signal_dict["signal_type"] = "BOOLEAN"
    elif signal_value.length <= 8 and signal_value.length > 1:
        signal_dict["signal_type"] = "SINT8"
    elif signal_value.length <= 16:
        signal_dict["signal_type"] = "SINT16"
    elif signal_value.length <= 32:
        signal_dict["signal_type"] = "SINT32"
    elif signal_value.length > 32 and signal_value.length <= 64 and bool(signal_dict["signal_size"] % 8):
        signal_dict["signal_type"] = "SINT64"
    else:
        if signal_dict["byte_order"] == "BIG_ENDIAN":
            # 获取ComEndBitPosition MSB
            ComBitPositionMsb = signal_dict["bit_position"]
            # 获取ComEndBitPosition LSB
            ComBitSize = signal_dict["signal_size"]
            ComLsbInx = ((ComBitPositionMsb ^ 0x7) + ComBitSize - 1) ^ 7  
            if bool(ComLsbInx % 8):
                logger.ui_error(f'ComUI: Signal ({signal_dict["signal_name"]}) is Parsed into UINT8_N, bit_position need be 8n (n >= 0)')
            if bool(ComBitSize % 8):
                logger.ui_error(f'ComUI: Signal ({signal_dict["signal_name"]}) is Parsed into UINT8_N, bit_size need be 8n (n >= 0)')
        else:
            if bool(signal_dict["bit_position"] % 8):
                logger.ui_error(f'ComUI: Signal ({signal_dict["signal_name"]}) is Parsed into UINT8_N, bit_position need be 8n (n >= 0)')
            if bool(ComBitSize % 8):
                logger.ui_error(f'ComUI: Signal ({signal_dict["signal_name"]}) is Parsed into UINT8_N, bit_size need be 8n (n >= 0)')
        signal_dict["signal_type"] = "UINT8_N"
        signal_dict["signal_size"] = 0

def get_config_json_template_input_dbc_list(config_dict: dict) -> dict:
    """
    Get input parameter for template com_cfg.json.

    Arg:
        config_list : All config list

    Return:
        Returns a dictionary of template input parameter 
    """
    com_config_json_dict = {}
    com_config_json_dict["config"] = {}
    ipdu_group_refs = []
    com_mainfunction_coreId_refs = {}
    coreId = 0 
    dbc_list = config_dict["dbc"]  

    # 需要提前遍历ldf  获取ldf的core信息
    if config_dict["ldf"] is not None:
        ldf_list = config_dict["ldf"]
        for ldf in ldf_list:
            if "coreId" in ldf.keys():
                if ldf["coreId"] == None:
                    coreId = 0
                else:
                    coreId =  ldf["coreId"]
            else:
                coreId = 0
            if coreId not in com_mainfunction_coreId_refs:
                com_mainfunction_coreId_refs[coreId] = {}
            com_mainfunction_coreId_refs[coreId].update({coreId: coreId})

    for dbc in dbc_list:
        if "coreId" in dbc.keys():
            if dbc["coreId"] == None:
                coreId = 0
            else:
                coreId =  dbc["coreId"]
        else:
            coreId = 0
        if coreId not in com_mainfunction_coreId_refs:
            com_mainfunction_coreId_refs[coreId] = {}
        com_mainfunction_coreId_refs[coreId].update({coreId: coreId})
        config_json_dict = []
        for msg_key, msg_value in dbc["details"].items():
            msg_dict = {}
            msg_dict["comm_diag_type"] = "dbc"
            msg_dict["message_name"] = msg_value.message_name
            msg_dict["bus_name"] = msg_value.bus_name
            if msg_value.bus_name not in ipdu_group_refs and "GenMsgILSupport" in msg_value.message_attributes and msg_value.message_attributes[
                    "GenMsgILSupport"].value == 1:
                ipdu_group_refs.append(msg_value.bus_name)
            msg_dict["message_size"] = msg_value.length
            msg_dict["has_signal_groups"] = msg_value.has_signal_groups
            if "GenMsgSendType" in msg_value.message_attributes:
                gen_msg_send_type_value = msg_value.message_attributes["GenMsgSendType"].value
            else:
                gen_msg_send_type_value = 0
            if "GenMsgCycleTime" in msg_value.message_attributes:
                msg_dict["gen_msg_cycle_time"] = msg_value.message_attributes["GenMsgCycleTime"].value / 1000  # ms->s
            else:
                msg_dict["gen_msg_cycle_time"] = 0

            if "GenMsgStartDelayTime" in msg_value.message_attributes:
                msg_dict["gen_msg_start_delay_time"] = msg_value.message_attributes[
                    "GenMsgStartDelayTime"].value / 1000  # ms->s
            else:
                msg_dict["gen_msg_start_delay_time"] = 0

            if "GenMsgNrOfRepetition" in msg_value.message_attributes:
                msg_dict["gen_msg_nr_of_repetition"] = msg_value.message_attributes["GenMsgNrOfRepetition"].value
            else:
                msg_dict["gen_msg_nr_of_repetition"] = 0

            if "GenMsgCycleTimeFast" in msg_value.message_attributes:
                msg_dict["gen_msg_cycle_time_fast"] = msg_value.message_attributes[
                    "GenMsgCycleTimeFast"].value / 1000  # ms->s
            else:
                msg_dict["gen_msg_cycle_time_fast"] = 0

            if "GenMsgDelayTime" in msg_value.message_attributes:
                msg_dict["gen_msg_delay_time"] = msg_value.message_attributes["GenMsgDelayTime"].value / 1000  # ms->s
            else:
                msg_dict["gen_msg_delay_time"] = 0
            
            if "NmAsrMessage" in msg_value.message_attributes and msg_value.message_attributes[
                    "NmAsrMessage"].value == 1:
                msg_type = "cannm"
            else:
                msg_type = ""

            get_gen_msg_send_type(gen_msg_send_type_value, msg_dict, msg_type)

            msg_dict["senders"] = msg_value.senders[0]
            if dbc["node_selected"] == msg_dict["senders"]:
                direction = "Tx"
            else:
                direction = "Rx"
            signal_groups = []
            for signal_groups_key, signal_groups_value in msg_value.signal_groups.items():
                signal_group_dict = {}
                signal_group_dict["direction"] = direction
                signal_group_dict["group_name"] = signal_groups_value.signal_group_name
                signal_group_dict["repetitions"] = signal_groups_value.repetitions
                signal_group_dict["group_signals_name"] = signal_groups_value.group_signals_name
                signal_groups.append(signal_group_dict)
            msg_dict["signal_groups"] = signal_groups
            signals = []
         
            for signal_key, signal_value in msg_value.signals.items():
                if msg_type == "cannm" and signal_value.start <= 15:
                    continue
                signal_dict = {}
                signal_dict["direction"] = direction
                get_dbc_signals(signal_dict, signal_value, msg_type, msg_value)
                if signal_value.signal_alias_name[-3:] == "_UB":
                    continue
                signals.append(signal_dict)
            msg_dict["signals"] = signals
            if ("DiagRequest" in msg_value.message_attributes and msg_value.message_attributes["DiagRequest"].value
                    == 1) or ("DiagResponse" in msg_value.message_attributes and
                              msg_value.message_attributes["DiagResponse"].value
                              == 1) or ("DiagState" in msg_value.message_attributes and
                                        msg_value.message_attributes["DiagState"].value
                                        == 1) or ("GenMsgILSupport" in msg_value.message_attributes and
                                                  msg_value.message_attributes['GenMsgILSupport'].value
                                                  == 0) or ("NmAsrMessage" in msg_value.message_attributes and
                                                            msg_value.message_attributes["NmAsrMessage"].value == 1 and
                                                            dbc["node_selected"] != msg_dict["senders"]):
                continue
            if "NmAsrMessage" in msg_value.message_attributes and msg_value.message_attributes[
                    "NmAsrMessage"].value == 1:
                msg_dict["pdu_type"] = "cannm"
            elif "GenMsgILSupport" in msg_value.message_attributes and msg_value.message_attributes[
                    'GenMsgILSupport'].value == 1:
                msg_dict["pdu_type"] = "com"
            else:
                continue
            # 如果该IPDU没有任何信号 需要报警
            if len(signals) == 0:
                logger.ui_error(f'ComUI: DBC({dbc["dbc_name"]}) IPdu({msg_key}) need at least one signal')
            
            config_json_dict.append(msg_dict)
        com_config_json_dict["config"][dbc["dbc_name"]] = {}
        com_config_json_dict["config"][dbc["dbc_name"]]["details"] = config_json_dict
        com_config_json_dict["config"][dbc["dbc_name"]]["node_selected"] = dbc["node_selected"]
        com_config_json_dict["config"][dbc["dbc_name"]]["coreId"] = coreId
    
    com_config_json_dict["ipdu_group_refs"] = ipdu_group_refs
    com_config_json_dict["com_mainfunction_coreId_refs"] = com_mainfunction_coreId_refs
    return com_config_json_dict


def set_signal_updatebit(msg_value, signal_dict):
    for signal_key, signal_value in msg_value.signals.items():
        if signal_value.signal_alias_name[-3:] == "_UB":
            if signal_value.signal_alias_name[:-3] == signal_dict["signal_alias_name"]:
                signal_dict["UpdateBit_Position"] = signal_value.start


def get_config_json_template_input_ldf_list(dict: dict) -> dict:
    """
    Get input parameter for template com_cfg.json.

    Arg:
        ldf_list : All config dictionary

    Return:
        Returns a dictionary of template input parameter
    """
    com_config_json_dict = {}
    com_config_json_dict["config"] = {}
    ipdu_group_refs = []
    
    ldf_list = dict["ldf"]
    com_mainfunction_coreId_refs = {}
    coreId = 0
    for ldf in ldf_list:
        config_json_dict = []
        if "coreId" in ldf.keys():
            if ldf["coreId"] == None:
                coreId = 0
            else:
                coreId = ldf["coreId"]
        else:
            coreId = 0
        if coreId not in com_mainfunction_coreId_refs:
            com_mainfunction_coreId_refs[coreId] = {}
        com_mainfunction_coreId_refs[coreId].update({coreId: coreId})

        for msg_key, msg_value in ldf["details"].items():
            msg_dict = {}
            msg_dict["bus_name"] = msg_value.bus_name
            ipdu_group_name = msg_value.bus_name
            if ipdu_group_name not in ipdu_group_refs:
                ipdu_group_refs.append(ipdu_group_name)
            msg_dict["comm_diag_type"] = "ldf"
            msg_dict["message_name"] = msg_value.message_name
            msg_dict["senders"] = msg_value.node_name
            msg_dict["has_signal_groups"] = 0
            msg_dict["signal_groups"] = []
            msg_dict["gen_msg_send_type"] = "NONE"
            msg_dict["message_size"] = msg_value.length
            msg_dict["gen_msg_cycle_time"] = 0
            msg_dict["gen_msg_cycle_time_fast"] = 0
            msg_dict["gen_msg_start_delay_time"] = 0
            msg_dict["gen_msg_nr_of_repetition"] = 0
            msg_dict["gen_msg_delay_time"] = 0
            msg_dict["frame_id"] = msg_value.frame_id
            # 过滤诊断帧 诊断帧ID 0x3C 0x3D
            if msg_dict["frame_id"] in [0x3C, 0x3D]:
                continue
            signals = []
            for signal_key, signal_value in msg_value.signals.items():
                if signal_value.signal_alias_name[-3:] == "_UB":
                    continue
                signal_dict = {}
                signal_dict["UpdateBit_Position"] = 0
                set_signal_updatebit(msg_value, signal_dict)
                signal_dict["signal_name"] = signal_value.signal_name
                signal_dict["bit_position"] = signal_value.start
                signal_dict["signal_size"] = signal_value.length
                signal_dict["ComSignalLength"] = signal_dict["signal_size"] // 8 + int(
                    bool(signal_dict["signal_size"] % 8))
                signal_dict["byte_order"] = "LITTLE_ENDIAN"
                get_unsigned_type(signal_dict, signal_value)
                signal_dict["init_value"] = signal_value.initial
                if ldf["node_selected"] == msg_dict["senders"]:
                    signal_dict["direction"] = "Tx"
                else:
                    signal_dict["direction"] = "Rx"
                signal_dict["gen_sig_send_type"] = "PENDING"
                signals.append(signal_dict)
            msg_dict["signals"] = signals
            config_json_dict.append(msg_dict)
        com_config_json_dict["config"][ldf["ldf_name"]] = {}
        com_config_json_dict["config"][ldf["ldf_name"]]["details"] = config_json_dict
        com_config_json_dict["config"][ldf["ldf_name"]]["node_selected"] = ldf["node_selected"]
        com_config_json_dict["config"][ldf["ldf_name"]]["coreId"] = coreId
    com_config_json_dict["ipdu_group_refs"] = ipdu_group_refs
    com_config_json_dict["com_mainfunction_coreId_refs"] = com_mainfunction_coreId_refs
    # com_config_json_dict["node_selected"] = config_dict["dbc"][0]["node_selected"]

    return com_config_json_dict


def _get_dict_value_by_nested_keys(target_dict, nested_key_list):
    rv = None
    find_value = target_dict
    for key in nested_key_list:
        if isinstance(find_value, dict) and key in find_value.keys():
            find_value = find_value[key]
        else:
            break
    else:
        rv = find_value
    return rv


def _check_had_container(target_dict, nested_key_list):
    rv = False
    find_value = target_dict
    if nested_key_list is None or len(nested_key_list) == 0:
        return rv
    for key in nested_key_list:
        if isinstance(find_value, dict) and key in find_value.keys():
            find_value = find_value[key]
        else:
            break
    else:
        rv = True
    return rv
