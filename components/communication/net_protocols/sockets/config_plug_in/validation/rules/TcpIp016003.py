from func_module.validation.common.basic_rule import BasicRule, timing_decorator
from durable.lang import *
from durable.engine import *
from .TcpIp_utils import TcpIpRuleUtils


# TcpIpDefaultRoute 易错校验
class TcpIp016003Rule(BasicRule):
    ruleset_name = 'TCPIP016003'
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.bsw_ecuc_cfg_class_dict = bsw_ecuc_cfg_class_dict
        self.ecuc_tcpip_dict = bsw_ecuc_cfg_class_dict['TcpIp']['ArPackages']['ActiveEcuC']['TcpIp']
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "TcpIp Default Routes should be valid!"

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
            if "TcpIpLocalAddr" == config_value["DefinitionRef"].split("/")[-1] and "TCPIP_UNICAST" == config_value["TcpIpAddressType"]["Value"]:

                valid_path = base_path + "/" + config_key + "/" + "TcpIpStaticIpAddressConfig"+ "/" + "TcpIpDefaultRouter"
                validation_data = {}
                if not (config_value["TcpIpStaticIpAddressConfig"]["TcpIpNetmask"]["Value"]):
                    continue
                validation_data["tcpipdefaultrouter"] = config_value["TcpIpStaticIpAddressConfig"]["TcpIpDefaultRouter"]["Value"]
                validation_data["tcpipnetmask"] = config_value["TcpIpStaticIpAddressConfig"]["TcpIpNetmask"]["Value"]
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
            #   真正校验规则
            # 将IP地址和子网掩码转换为二进制字符串
            router_bin = ''.join(format(int(x), '08b') for x in validation_data["tcpipdefaultrouter"] .split('.'))
            ip_bin = ''.join(format(int(x), '08b') for x in validation_data["tcpipstaticipaddress"].split('.'))

            # 截取相应长度的子网掩码
            subnet_mask = int(validation_data["tcpipnetmask"])
            subnet_mask_bin = '1' * subnet_mask + '0' * (32 - subnet_mask)

            # 进行逻辑与运算
            router_network = ''.join(str(int(a) & int(b)) for a, b in zip(router_bin, subnet_mask_bin))
            ip_network = ''.join(str(int(a) & int(b)) for a, b in zip(ip_bin, subnet_mask_bin))

            # 判断是否处于同一网段
            if router_network == ip_network:
                self.__class__.validation_result = True
            else:
                self.__class__.validation_result = False

            # 格式："错误码", "摘要", "错误信息", "错误索引路径" "结果"
            if self.__class__.validation_result:
                self.vlog(self.validation_code,  self.validation_summary,
                            f"TcpIpDefaultRouter {validation_data['tcpipdefaultrouter']} Validity Check Success.",
                            validataion_data_path_list[index], "pass")
            else:
                self.vlog(self.validation_code,  self.validation_summary,
                            f"TcpIpDefaultRouter {validation_data['tcpipdefaultrouter']} Validity Check Fail.",
                            validataion_data_path_list[index], "fail")

        # 3）清空ruleset中的所有fact，为下一次校验做准备
        if len(validation_data_list) > 0:
            retract_facts(self.__class__.ruleset_name, validation_data_list)
        pass


    def add_rule_time(self):
        pass