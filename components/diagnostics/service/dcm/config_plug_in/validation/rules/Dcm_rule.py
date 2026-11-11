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

import importlib
from func_module.health_monitor.logger import logger

from func_module.validation.common.validation_def import ValidationData, ExceptionRuleLog

VALIDATE_MARK_LIST = [
    "012005", "012006", "012007", "012008", "012009", "012010",
    "012011", "012012", "012013", "012014", "012015", "012016", "012017", "012018", "011008", "011009",
]

# 语义校验  -  重复配置
#  011001    DcmDsdSidTabServiceId
#  011002    DcmDsdSubServiceId
#  011003    DcmDslProtocolRxConnectionId
#  011004    DcmDslProtocolRxPduRef
#  011005    DcmDspSecurityLevel
#  011006    DcmDspSessionLevel
#  011007    DcmAgrumentRef

# 语义校验  -  动态配置项缺失
#  012001    DcmDspSecurityUsePort  DcmDspSecurityCompareKeyFnc
#  012002    DcmDspSecurityUsePort  DcmDspSecurityGetSeedFnc
#  012003    DcmDspSecurityUsePort  DcmDspSecurityGetAttemptCounterFnc
#  012004    DcmDspSecurityUsePort  DcmDspSecuritySetAttemptCounterFnc
#  012005    DcmDspDataUsePort  USE_DATA_SYNCH_FNC  DcmDspDataReadFnc
#  012006    DcmDspDataUsePort  USE_DATA_ASYNCH_FNC  DcmDspDataReadFnc
#  012007    DcmDspDataUsePort  USE_DATA_SYNCH_FNC  DcmDspDataWriteFnc
#  012008    DcmDspDataUsePort  USE_DATA_ASYNCH_FNC  DcmDspDataWriteFnc
#  012009    DcmDspDataUsePort  USE_DATA_SYNCH_FNC  DcmDspDataFreezeCurrentStateFnc
#  012010    DcmDspDataUsePort  USE_DATA_ASYNCH_FNC  DcmDspDataFreezeCurrentStateFnc
#  012011    DcmDspDataUsePort  USE_DATA_SYNCH_FNC  DcmDspDataResetToDefaultFnc
#  012012    DcmDspDataUsePort  USE_DATA_ASYNCH_FNC  DcmDspDataResetToDefaultFnc
#  012013    DcmDspDataUsePort  USE_DATA_SYNCH_FNC  DcmDspDataShortTermAdjustmentFnc
#  012014    DcmDspDataUsePort  USE_DATA_ASYNCH_FNC  DcmDspDataShortTermAdjustmentFnc
#  012015    DcmDspDataUsePort  USE_DATA_SYNCH_FNC  DcmDspDataReturnControlToEcuFnc
#  012016    DcmDspDataUsePort  USE_DATA_ASYNCH_FNC  DcmDspDataReturnControlToEcuFnc
#  012017    DcmDspDataUsePort  USE_DATA_SYNCH_FNC  DcmDspDataConditionCheckReadFnc
#  012018    DcmDspDataUsePort  USE_DATA_ASYNCH_FNC  DcmDspDataConditionCheckReadFnc

# 语义校验  -  互斥配置
#  013001    DcmDspSecurityAttemptCounterEnabled 与 DcmDspSecurityNumAttDelay 互斥配置


class DcmRule():

    def __init__(self) -> None:
        self.logs = {}

    def validate(self, validation_data:ValidationData):
        bsw_ecuc_cfg_class_dict = validation_data.bsw_ecuc_dict
        ecuc_dcm_dict = bsw_ecuc_cfg_class_dict['Dcm']['ArPackages']['ActiveEcuC']['Dcm']

        for validate_code in VALIDATE_MARK_LIST:
            validate_rule_package = importlib.import_module(f'.DCM{validate_code}', __package__)
            validate_rule_class = getattr(validate_rule_package, f'Dcm{validate_code}Rule')
            try:
                bsw_module_rule_obj = validate_rule_class(self.logs, bsw_ecuc_cfg_class_dict)
                # 校验当前校验规则的前置条件是否满足
                if bsw_module_rule_obj.check_validation_run():
                    bsw_module_rule_obj.validation_run(ecuc_dcm_dict)
            except Exception as e:
                ExceptionRuleLog.set_rule_exception_log(self.logs, "Dcm", validate_code, f"{type(e)}: {str(e)}")

        return self.logs





