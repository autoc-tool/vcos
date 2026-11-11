from func_module.validation.common.basic_rule import BasicRule, timing_decorator
from durable.lang import *
from durable.engine import *
from .TcpIp_utils import TcpIpRuleUtils


# TcpIpStaticIpAddress 易错校验
class TcpIp016001Rule(BasicRule):
    ruleset_name = 'TCPIP016001'
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.bsw_ecuc_cfg_class_dict = bsw_ecuc_cfg_class_dict
        self.ecuc_tcpip_dict = bsw_ecuc_cfg_class_dict['TcpIp']['ArPackages']['ActiveEcuC']['TcpIp']
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "TcpIp Static Ip Address should be valid!"

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

                @when_all(1)
                def fact(c):
                    self.__class__.validation_result = False

    def check_validation_run(self):
        if not self.modules_are_exist(["TcpIp"], self.bsw_ecuc_cfg_class_dict):
            self.vlog(self.validation_code, self.validation_summary, "Data required for validation is missing",
                      self.validation_summary, "fail")
            return False
        return True

    @timing_decorator
    def validation_run(self):
        ecuc_tcpip_dict = self.ecuc_tcpip_dict
        # 1）准备校验数据
        tcpipconfig_dict = TcpIpRuleUtils.tcpip_get_dict_by_definition_ref(ecuc_tcpip_dict, "TcpIpConfig")
        base_path = "/ActiveEcuC/TcpIp/TcpIpConfig/TcpIpLocalAddr"
        validation_data_list = []
        validataion_data_path_list = []

        for config_key, config_value in tcpipconfig_dict.items():
            if not isinstance(config_value, dict):
                continue
            if "TcpIpLocalAddr" == config_value["DefinitionRef"].split("/")[-1]:
                valid_path = base_path + "/" + config_key + "/" + "TcpIpStaticIpAddressConfig"+ "/" + "TcpIpStaticIpAddress"
                validation_data = {}
                validation_data["tcpipstaticipaddress"] = config_value["TcpIpStaticIpAddressConfig"]["TcpIpStaticIpAddress"]["Value"]
                validation_data_list.append(validation_data)
                validataion_data_path_list.append(valid_path)

        # 2） 开始校验
        for index, validation_data in enumerate(validation_data_list):
            try:
                self.__class__.validation_result = False
                post(self.__class__.ruleset_name, validation_data)
            except Exception as e:
                self.__class__.validation_result = False
                pass

            #真正校验规则
            self.__class__.validation_result = True

            parts = ""
            parts = validation_data["tcpipstaticipaddress"].split('.')

            # 判断切割后的字符串长度是否为4
            if len(parts) != 4:
                self.__class__.validation_result = False

            # 判断每个部分是否是合法的数字，并且在范围0-255内
            for i, part in enumerate(parts):
                try:
                    num = int(part)
                    if i == 0:
                        if num < 1 or num > 255:
                            self.__class__.validation_result = False
                    else:
                        if num < 0 or num > 255:
                            self.__class__.validation_result = False
                except ValueError:
                    self.__class__.validation_result = False

            # 格式："错误码", "摘要", "错误信息", "错误索引路径" "结果"
            if self.__class__.validation_result:
                self.vlog(self.validation_code,  self.validation_summary,
                            f"TcpIpStaticIpAddress {validation_data['tcpipstaticipaddress']} Validity Check Success.",
                            validataion_data_path_list[index], "pass")
            else:
                self.vlog(self.validation_code,  self.validation_summary,
                            f"TcpIpStaticIpAddress {validation_data['tcpipstaticipaddress']} Validity Check Fail.",
                            validataion_data_path_list[index], "fail")

        # 3）清空ruleset中的所有fact，为下一次校验做准备
        if len(validation_data_list) > 0:
            retract_facts(self.__class__.ruleset_name, validation_data_list)
        pass


    def add_rule_time(self):
        pass