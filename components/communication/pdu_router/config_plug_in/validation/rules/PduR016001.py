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
from basic_func_module.autosar_utils.autosar_util import AutosarUtil
from .PduR_rule_func import pdur_validate_get_routing_path


class PduR016001Rule(BasicRule):
    ruleset_name = "PduR016001"
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "PduRDestPduDataProvision should be set to PDUR_TRIGGERTRANSMIT!"
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
        valid = True
        ecuc_pdur_dict = self.bsw_ecuc_cfg_class_dict["PduR"]["ArPackages"]["ActiveEcuC"]["PduR"]
        PduRRoutingPaths, PduRRoutingPaths_Name, valid = pdur_validate_get_routing_path(ecuc_pdur_dict)
        if valid is False:
            service_id_path = "/ActiveEcuC/PduR/"
            self.vlog(self.validation_code, self.validation_summary, f"Cannot Find Container PduRRoutingPaths!",
                      service_id_path, "fail")
            return False
        for name, pdurdestpdu in PduRRoutingPaths.items():
            if "DefinitionRef" not in pdurdestpdu or pdurdestpdu[
              "DefinitionRef"] != "/AUTOSAR/EcucDefs/PduR/" + PduRRoutingPaths_Name + "/PduRDestPdu":
                continue
            if not AutosarUtil.get_by_path_from_dict(pdurdestpdu, "/PduRDestPduRef/ValueRef"):
                continue
            pdurdestpduref_path = pdurdestpdu["PduRDestPduRef"]["ValueRef"]
            pdurdestpdudataprovision = pdurdestpdu["PduRDestPduDataProvision"]["Value"]
            # ignore: lin diagnostic frames
            if "MasterReq" in pdurdestpduref_path or "SlaveResp" in pdurdestpduref_path:
                continue
            if "Com_CanNm_Tx" in pdurdestpduref_path and pdurdestpdudataprovision == "PDUR_DIRECT":
                valid = False
                validation_data_list = []
                service_id_path = "/ActiveEcuC/PduR/" + PduRRoutingPaths_Name + "/PduRDestPdu/" + name
                # 2） 开始校验
                # 格式："错误码", "摘要", "错误信息", "错误索引路径" "结果"
                if valid:
                    self.vlog(
                      self.validation_code, self.validation_summary,
                      f"PduRDestPduDataProvision[{pdurdestpdudataprovision}] should be set to PDUR_TRIGGERTRANSMIT!",
                      service_id_path, "pass")
                else:
                    self.vlog(
                      self.validation_code, self.validation_summary,
                      f"PduRDestPduDataProvision[{pdurdestpdudataprovision}] should be set to PDUR_TRIGGERTRANSMIT!",
                      service_id_path, "fail")
                # 3）清空ruleset中的所有fact，为下一次校验做准备
                if len(validation_data_list) > 0:
                    retract_facts(self.__class__.ruleset_name, validation_data_list)
        return valid

    def add_rule_time(self):
        pass
