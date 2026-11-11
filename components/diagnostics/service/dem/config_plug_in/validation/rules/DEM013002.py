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

from func_module.validation.common.basic_rule import BasicRule, timing_decorator
from durable.lang import *
from durable.engine import *
from .Dem_utils import DemRuleUtils

class DEM013002Rule(BasicRule):
    ruleset_name = "DEM013002"
    validation_result = False

    def __init__(self, logs, bsw_ecuc_dict: dict) -> None:
        super().__init__()
        self.logs = logs
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "DemEventCombinationSupport should be valid!"
        self.bsw_ecuc_cfg_class_dict = bsw_ecuc_dict

    def check_validation_run(self):
        if not self.modules_are_exist(["Dem"], self.bsw_ecuc_cfg_class_dict):
            self.vlog(self.validation_code, self.validation_summary,
                    "Data required for validation is missing",
                    self.validation_summary,
                    "fail")
            return False
        return True

    def dem_check_evcomb_disabled(self, validation_code:dict, validation_summary:dict, dem_config_set:dict):
        valid = True
        dem_config_set_name, dem_config_set = DemRuleUtils.dem_get_dict_by_definition_ref(self.bsw_ecuc_cfg_class_dict["Dem"]["ArPackages"]["ActiveEcuC"]["Dem"], "DemConfigSet")
        base_path = "/AUTOSAR/EcucDefs/Dem/" + dem_config_set_name
        dtc_event_ref_dict = {}
        for event_parameter_name, event_parameter in dem_config_set.items():
            if "DefinitionRef" not in event_parameter or event_parameter[
                    "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemEventParameter":
                continue
            dtc_ref = event_parameter["DemDTCRef"]["ValueRef"].split('/')[-1]
            if dtc_ref not in dtc_event_ref_dict.keys():
                dtc_event_ref_dict.update({dtc_ref: [event_parameter_name]})
            else:
                dtc_event_ref_dict[dtc_ref].append(event_parameter_name)
        for dtc_ref, event_parameter_list in dtc_event_ref_dict.items():
            if len(event_parameter_list) > 1:
                valid = False
                for event_parameter_name in event_parameter_list:
                    self.vlog(validation_code, validation_summary,
                            "DemDTCRef should not be duplicated!",
                            f"{base_path}/{event_parameter_name}/DemDTCRef", "fail")

        return valid

    def dem_check_evcomb_onstorage(self, validation_code:dict, validation_summary:dict, dem_config_set:dict):
        valid = False
        dtc_ref_list = []
        for event_parameter in dem_config_set.values():
            if "DefinitionRef" not in event_parameter or event_parameter[
                    "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemEventParameter":
                continue
            dtc_ref = event_parameter["DemDTCRef"]["ValueRef"].split('/')[-1]
            if dtc_ref in dtc_ref_list:
                valid = True
                break
            else:
                dtc_ref_list.append(dtc_ref)

        return valid

    @timing_decorator
    def validation_run(self):
        # 1）准备校验数据
        dem_general_name, dem_general = DemRuleUtils.dem_get_dict_by_definition_ref(self.bsw_ecuc_cfg_class_dict["Dem"]["ArPackages"]["ActiveEcuC"]["Dem"], "DemGeneral")
        _, dem_config_set = DemRuleUtils.dem_get_dict_by_definition_ref(self.bsw_ecuc_cfg_class_dict["Dem"]["ArPackages"]["ActiveEcuC"]["Dem"], "DemConfigSet")
        base_path = "/ActiveEcuC/Dem/" + dem_general_name
        valid = True

        if dem_general["DemEventCombinationSupport"]["Value"] == "DEM_EVCOMB_ONRETRIEVAL":
            self.vlog(self.validation_code, self.validation_summary,
                    "DEM_EVCOMB_ONRETRIEVAL is not supported.",
                    f"{base_path}/DemEventCombinationSupport", "fail")
            valid = False
        elif dem_general["DemEventCombinationSupport"]["Value"] == "DEM_EVCOMB_DISABLED":
            valid = self.dem_check_evcomb_disabled(self.validation_code, self.validation_summary, dem_config_set)
            if valid is False:
                self.vlog(self.validation_code, self.validation_summary,
                    "DEM_EVCOMB_DISABLED is not expected.",
                    f"{base_path}/DemEventCombinationSupport", "fail")
        elif dem_general["DemEventCombinationSupport"]["Value"] == "DEM_EVCOMB_ONSTORAGE":
            valid = self.dem_check_evcomb_onstorage(self.validation_code, self.validation_summary, dem_config_set)
            if valid is False:
                self.vlog(self.validation_code, self.validation_summary,
                    "DEM_EVCOMB_ONSTORAGE is not expected.",
                    f"{base_path}/DemEventCombinationSupport", "fail")
        else:
            self.vlog(self.validation_code, self.validation_summary,
                    f"{dem_general['DemEventCombinationSupport']['Value']} is not supported.",
                    f"{base_path}/DemEventCombinationSupport", "fail")
            valid = False

        if valid is True:
            self.vlog(self.validation_code, self.validation_summary,
                    f"{dem_general['DemEventCombinationSupport']['Value']} is valid.",
                    f"{base_path}/DemEventCombinationSupport", "pass")

        return valid

    def add_rule_time(self):
        pass
