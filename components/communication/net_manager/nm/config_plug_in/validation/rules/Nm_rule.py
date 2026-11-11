from func_module.validation.common.validation_def import ValidationData, ExceptionRuleLog
import importlib
from func_module.health_monitor.logger import logger

VALIDATE_MARK_LIST = [
]


class NmRule():

    def __init__(self) -> None:
        self.logs = {}

    def validate(self, validation_data:ValidationData):
        bsw_ecuc_cfg_class_dict = validation_data.bsw_ecuc_dict

        Nm_dict = bsw_ecuc_cfg_class_dict['Nm']['ArPackages']['ActiveEcuC']['Nm']
        # 语义校验  -  重复配置
        for validate_code in VALIDATE_MARK_LIST:
            validate_rule_package = importlib.import_module(f'.Nm{validate_code}', __package__)
            validate_rule_class = getattr(validate_rule_package, f'Nm{validate_code}Rule')
            try:
                bsw_module_rule_obj = validate_rule_class(self.logs, bsw_ecuc_cfg_class_dict, Nm_dict)
            # 校验当前校验规则的前置条件是否满足
                if bsw_module_rule_obj.check_validation_run():
                    bsw_module_rule_obj.validation_run()
            except Exception as e:
                ExceptionRuleLog.set_rule_exception_log(self.logs, "Nm", validate_code, f"{type(e)}: {str(e)}")

        return self.logs
