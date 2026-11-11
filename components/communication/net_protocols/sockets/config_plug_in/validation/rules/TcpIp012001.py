from func_module.validation.common.basic_rule import BasicRule, timing_decorator
from durable.lang import *
from durable.engine import *
from .TcpIp_utils import TcpIpRuleUtils


# TcpIpEthIfCtrlRef 动态配置缺失
class TcpIp012001Rule(BasicRule):
    ruleset_name = 'TCPIP012001'
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.bsw_ecuc_cfg_class_dict = bsw_ecuc_cfg_class_dict
        self.ecuc_tcpip_dict = bsw_ecuc_cfg_class_dict['TcpIp']['ArPackages']['ActiveEcuC']['TcpIp']
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "TcpIp EthIf Controller Reference should not be empty!"

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

                @when_all((m.tcpipethifctrlref != '') & (m.tcpipethifctrlref != '<empty>') & (m.tcpipethifctrlref != None))
                def fact(c):
                    self.__class__.validation_result = True

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
        base_path = "/ActiveEcuC/TcpIp/TcpIpConfig"
        validation_data_list = []
        validataion_data_path_list = []

        for config_key, config_value in tcpipconfig_dict.items():
            if not isinstance(config_value, dict):
                continue
            if "TcpIpCtrl" == config_value["DefinitionRef"].split("/")[-1]:
                valid_path = base_path + "/" + config_key + "/" + "TcpIpEthIfCtrlRef"
                validation_data = {}
                validation_data["tcpipethifctrlref"] = config_value["TcpIpEthIfCtrlRef"]["ValueRef"]
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
            # 格式："错误码", "摘要", "错误信息", "错误索引路径" "结果"
            if self.__class__.validation_result:
                self.vlog(self.validation_code,  self.validation_summary,
                            f"TcpIpEthIfCtrlRef {validation_data['tcpipethifctrlref']} Missing Check Success.",
                            validataion_data_path_list[index], "pass")
            else:
                self.vlog(self.validation_code,  self.validation_summary,
                            f"TcpIpEthIfCtrlRef {validation_data['tcpipethifctrlref']} Missing Check Fail.",
                            validataion_data_path_list[index], "fail")

        # 3）清空ruleset中的所有fact，为下一次校验做准备
        if len(validation_data_list) > 0:
            retract_facts(self.__class__.ruleset_name, validation_data_list)
        pass


    def add_rule_time(self):
        pass