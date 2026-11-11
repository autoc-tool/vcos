from func_module.validation.common.basic_rule import BasicRule, timing_decorator
from durable.lang import *
from durable.engine import *
from .TcpIp_utils import TcpIpRuleUtils


# TcpIpDefaultRoute 易错校验
class TcpIp110001Rule(BasicRule):
    ruleset_name = 'TCPIP110001'
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.bsw_ecuc_cfg_class_dict = bsw_ecuc_cfg_class_dict
        self.ecuc_tcpip_dict = bsw_ecuc_cfg_class_dict['TcpIp']['ArPackages']['ActiveEcuC']['TcpIp']
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "The product of TcpIpArpTableSizeMax and TcpIpArpPacketQueueLength is advised not to exceed MEMP_NUM_ARP_QUEUE (default 30) to avoid memory insufficiency."

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
        tcpipipconfig_dict = TcpIpRuleUtils.tcpip_get_dict_by_definition_ref(tcpipconfig_dict, "TcpIpIpConfig")
        ipv4_dict = TcpIpRuleUtils.tcpip_get_dict_by_definition_ref(tcpipipconfig_dict, "TcpIpIpV4Config")
        arp_dict = TcpIpRuleUtils.tcpip_get_dict_by_definition_ref(ipv4_dict, "TcpIpArpConfig")
        base_path = "/ActiveEcuC/TcpIp/TcpIpConfig/TcpIpIpConfig/TcpIpIpV4Config/TcpIpArpConfig"

        ARPQueLen = None
        ARPTableSizeMax = None
        ArpQueueEnabled =  False
        MEMP_NUM_ARP_QUEUE = 30
        for key, value in arp_dict.items():
            if not isinstance(value, dict):
                continue
            if "TcpIpArpPacketQueueLength" == value["DefinitionRef"].split("/")[-1]:
                if value["Value"] != None and value["Value"] != "":
                    ARPQueLen = value["Value"]
                    validation_path = base_path + "/" + key

            if "TcpIpArpTableSizeMax" == value["DefinitionRef"].split("/")[-1]:
                if value["Value"] != None and value["Value"] != "":
                    ARPTableSizeMax = value["Value"]

            if "TcpIpArpPacketQueueEnabled" == value["DefinitionRef"].split("/")[-1]:
                if value["Value"] != None and value["Value"] != "":
                    ArpQueueEnabled = value["Value"]

        if ArpQueueEnabled == True and ARPQueLen != None and ARPTableSizeMax != None:
            if ARPQueLen * ARPTableSizeMax > MEMP_NUM_ARP_QUEUE:
                self.vlog(self.validation_code, self.validation_summary,
                        f"Based on the current configuration, there is a risk of insufficient memory. It is recommended to set the value to 3.",
                        validation_path, "fail")

    def add_rule_time(self):
        pass