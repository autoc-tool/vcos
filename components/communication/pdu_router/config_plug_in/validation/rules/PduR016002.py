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
from .PduR_rule_func import pdur_validate_get_routing_path

class PduR016002Rule(BasicRule):
    ruleset_name = "PduR016002"
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "PduRSourcePduHandleId should be less than PduRMaxRoutingPathCnt!"
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

                @when_all(+m.frame_id)
                def fact(c):
                    self.__class__.validation_result = True

            pass

    def check_validation_run(self):
        if not self.modules_are_exist(["PduR"], self.bsw_ecuc_cfg_class_dict):
            self.vlog(self.validation_code, self.validation_summary, "Data required for validation is missing",
                self.validation_summary, "fail")
            return False
        return True

    @timing_decorator
    def validation_run(self):
        # 1）准备校验数据
        ecuc_pdur_dict = self.bsw_ecuc_cfg_class_dict["PduR"]["ArPackages"]["ActiveEcuC"]["PduR"]
        maxroutingpathcnt = 0 # PduRMaxRoutingPathCnt作废
        totalSrcPduNum = 0
        service_id_path =''
        PduRRoutingPaths, PduRRoutingPaths_Name, valid = pdur_validate_get_routing_path(ecuc_pdur_dict)
        if valid is False:
            service_id_path = "/ActiveEcuC/PduR/"
            self.vlog(self.validation_code, self.validation_summary, f"Cannot Find Container PduRRoutingPaths!",
                      service_id_path, "fail")
            return False
        for name, pdursrcpdu in PduRRoutingPaths.items():
            if "DefinitionRef" not in pdursrcpdu or pdursrcpdu[
              "DefinitionRef"] != "/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRSrcPdu":
                continue
            totalSrcPduNum += 1
            srcpduhandleid = int(pdursrcpdu["PduRSourcePduHandleId"]["Value"])
            if srcpduhandleid > maxroutingpathcnt:
                maxroutingpathcnt = srcpduhandleid
                service_id_path = "/ActiveEcuC/PduR/"+ PduRRoutingPaths_Name + "/" + name
        if totalSrcPduNum == 0:
            return True
        if totalSrcPduNum <= maxroutingpathcnt: # 要求连续，所以最大值不能大于总数
            validation_data_list = []
            # 2） 开始校验
            # 格式："错误码", "摘要", "错误信息", "错误索引路径" "结果"
            self.vlog(
                self.validation_code, self.validation_summary,
                f"PduRSourcePduHandleId not continuous!",
                    service_id_path, "fail")
            # 3）清空ruleset中的所有fact，为下一次校验做准备
            if len(validation_data_list) > 0:
                retract_facts(self.__class__.ruleset_name, validation_data_list)

            return False
        return True

    def add_rule_time(self):
        pass
