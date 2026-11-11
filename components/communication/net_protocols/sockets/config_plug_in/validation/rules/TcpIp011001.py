from func_module.validation.common.basic_rule import BasicRule, timing_decorator
from durable.lang import *
from durable.engine import *
from .TcpIp_utils import TcpIpRuleUtils


class TcpIp011001Rule(BasicRule):
    ruleset_name = 'TCPIP011001'
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

                @when_all(m.value != None)
                def fact(c):
                    self.__class__.validation_result = True

            pass

    @timing_decorator
    def validation_run(self, ecuc_dict):
        # 1）准备校验数据
        result = True

        tcpip_utils = TcpIpRuleUtils()
        flattened_config = tcpip_utils.validate_utils_flatten_config(ecuc_dict)

        self.__class__.validation_result = True
        for entry in flattened_config:
            try:
                post(self.__class__.ruleset_name, entry)
            except Exception as e:
                self.__class__.validation_result = False
                result = False
                self.vlog("TCPIP011001", "validate_null_value.", f"Null value.", "/ActiveEcuC/TcpIp" + entry['path'],
                          "fail")

        return result

    def add_rule_time(self):
        pass
