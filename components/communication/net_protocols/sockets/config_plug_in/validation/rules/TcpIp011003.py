from func_module.validation.common.basic_rule import BasicRule, timing_decorator
from durable.lang import *
from durable.engine import *
from .TcpIp_utils import TcpIpRuleUtils


class TcpIp011003Rule(BasicRule):
    ruleset_name = 'TCPIP011003'
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.bsw_ecuc_cfg_class_dict = bsw_ecuc_cfg_class_dict
        self.durable_init()

    def durable_init(self):
        try:
            get_host().get_ruleset(self.__class__.ruleset_name)
        except Exception as e:
            with ruleset(self.__class__.ruleset_name):

                @when_all(m.required == m.actual)
                def fact(c):
                    self.__class__.validation_result = True

            pass

    @timing_decorator
    def validation_run(self, ecuc_dict):
        # 1）准备校验数据
        result = True

        tcpip_utils = TcpIpRuleUtils()
        ctrl_list = tcpip_utils.valudate_utils_get_refered_ctrl(ecuc_dict)
        print(ctrl_list)
        entry = {}
        entry['required'] = len(set(ctrl_list))
        entry['actual'] = len(ctrl_list)
        self.__class__.validation_result = True

        try:
            post(self.__class__.ruleset_name, entry)
        except Exception as e:
            self.__class__.validation_result = False
            result = False
            self.vlog("TcpIp011003", "validate_dup_ctrl.", f"Duplicated TcpIpCtrl.",
                      f"/ActiveEcuC/TcpIp/TcpIpConfig/TcpIpCtrl", "fail")

        return result

    def add_rule_time(self):
        pass
