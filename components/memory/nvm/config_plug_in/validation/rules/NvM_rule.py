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
from func_module.validation.common.validation_def import ValidationData, ExceptionRuleLog
import importlib
from func_module.health_monitor.logger import logger

VALIDATE_MARK_LIST = [
    "011001", "011002"
]

class NvMRule():

    def __init__(self) -> None:
        self.logs = {}

    def validate(self, validation_data:ValidationData):
        bsw_ecuc_cfg_class_dict = validation_data.bsw_ecuc_dict
        nvm_dict = bsw_ecuc_cfg_class_dict['NvM']['ArPackages']['ActiveEcuC']['NvM']

        for validate_code in VALIDATE_MARK_LIST:
            validate_rule_package = importlib.import_module(f'.NvM{validate_code}', __package__)
            validate_rule_class = getattr(validate_rule_package, f'NvM{validate_code}Rule')
            try:
                bsw_module_rule_obj = validate_rule_class(self.logs, bsw_ecuc_cfg_class_dict, nvm_dict)
            # 校验当前校验规则的前置条件是否满足
                if bsw_module_rule_obj.check_validation_run():
                    bsw_module_rule_obj.validation_run()
            except Exception as e:
                ExceptionRuleLog.set_rule_exception_log(self.logs, "Nvm", validate_code, f"{type(e)}: {str(e)}")

        return self.logs
