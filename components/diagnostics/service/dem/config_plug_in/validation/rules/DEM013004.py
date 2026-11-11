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
from func_module.generator.common.common import find_definition_ref_paths


# DemStorageConditionId顺序检查
class DEM013004Rule(BasicRule):
    ruleset_name = 'DEM013004'
    validation_result = False


    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "DemStorageConditionId is not sequential!"
        self.bsw_ecuc_cfg_class_dict = bsw_ecuc_cfg_class_dict

        self.durable_init()


    def durable_init(self):
        # 注：固定名称的规则集只能初始化一次，否则会异常报错
        try:
            get_host().get_ruleset(self.__class__.ruleset_name)
        except Exception as e:
            with ruleset(self.__class__.ruleset_name):

                @when_all(+m.status)
                def event(c):
                    pass
                @when_all(+m.list)
                def check_continuity_1(c):
                    # 假设 m.list 是传入的列表
                    lst = c.m.list
                    is_consecutive = is_valid_seq(lst, 0)

                    # 根据连续性结果进行下一步操作
                    if is_consecutive:
                        # 进行连续性的后续逻辑
                        self.__class__.validation_result = True
                    else:
                        # 进行非连续性的后续逻辑
                        self.__class__.validation_result = False
                        pass

    def check_validation_run(self):
        if not self.modules_are_exist(["Dem"], self.bsw_ecuc_cfg_class_dict):
            self.vlog(self.validation_code, self.validation_summary,
                    "Data required for validation is missing",
                    self.validation_summary,
                    "fail")
            return False
        return True

    @timing_decorator
    def validation_run(self):
        # 1）准备校验数据
        from dem.config_plug_in.generator import filter as dem_filter
        test_dict = {}
        Dem_dict = self.bsw_ecuc_cfg_class_dict['Dem']['ArPackages']['ActiveEcuC']['Dem']
        ref_path = find_definition_ref_paths(Dem_dict)
        DemGeneralShortName = dem_filter.get_dem_general_short_name(ref_path)
        DemGeneral = []
        if DemGeneralShortName != None:
            DemGeneral = Dem_dict[DemGeneralShortName]
            StorageConditionIdlist = []
            for key, value in DemGeneral.items():
                if key in ["DefinitionRef"]:
                    continue
                DemStorageCondition = value["DefinitionRef"].split("/")[-1]
                if "DemStorageCondition" == DemStorageCondition:
                    tem_conditionid = value["DemStorageConditionId"]["Value"]
                    StorageConditionIdlist.append(tem_conditionid)
            # StorageConditionIdlist = sorted(StorageConditionIdlist)
            test_dict["list"] = StorageConditionIdlist
            # 2） 开始校验
            try:
                self.__class__.validation_result = True
                post(self.__class__.ruleset_name, test_dict)
            except Exception as e:
                self.__class__.validation_result = False
                pass

            # 格式："错误码", "摘要", "错误信息", "错误索引路径" "结果"
            if self.__class__.validation_result == True:
                self.vlog(self.validation_code, self.validation_summary, f"The list is consecutive.",
                        f"/ActiveEcuC/Dem/DemGeneral/DemStorageCondition", "pass")
            else:
                self.vlog(self.validation_code, self.validation_summary,
                        f"The list is consecutive",
                        f"/ActiveEcuC/Dem/DemGeneral/DemStorageCondition", "fail")
            return self.__class__.validation_result
        else:
            self.__class__.validation_result = False
            self.vlog(self.validation_code, self.validation_summary,
                    f"Invalid Dem configuration",
                    f"/ActiveEcuC/Dem/DemGeneral/", "fail")
            return self.__class__.validation_result


    def add_rule_time(self):
        pass
