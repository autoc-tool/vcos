from basic_func_module.autosar_utils.utils import get_definition_ref, set_value, get_value, get_short_name
from basic_func_module.autosar_utils.autosar_util import AutosarUtil

class TcpIpUpgradeV0110():

    def __init__(self):
        pass

    def set_numerical_param_value(self, parameter_def_name, container_obj, value):
        numerical_param_list = container_obj.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE
        for numerical_parameter in numerical_param_list:
            if get_definition_ref(numerical_parameter).split("/")[-1] == parameter_def_name:
                set_value(numerical_parameter, value)
                return

    def upgrade_module(self, bsw_ecuc_autosar_class, autosar_ib_class, mod_autosar_dict, sys_info_dict) -> bool:
        print("upgrade TcpIp module")
        srcdata = bsw_ecuc_autosar_class["TcpIp"]
        tcpiptcpconfig = srcdata.find("/ActiveEcuC/TcpIp/TcpIpConfig/TcpIpTcpConfig").PARAMETER_VALUES
        TEXTUAL_PARAM = tcpiptcpconfig.ECUC_NUMERICAL_PARAM_VALUE

        for ele in TEXTUAL_PARAM:
            if get_definition_ref(ele) == "/AUTOSAR/EcucDefs/TcpIp/TcpIpConfig/TcpIpTcpConfig/TcpIpTcpSendBufferMax":
                set_value(ele,"12")

        tcpipbuffermemoryconfig = mod_autosar_dict["TcpIp"].find("/AUTOSAR/EcucDefs/TcpIp/TcpIpConfig/TcpIpBufferMemoryConfig")
        tcpip_parent_container = srcdata.find("/ActiveEcuC/TcpIp/TcpIpConfig")
        tcpip_memory_parent_container = srcdata.find("/ActiveEcuC/TcpIp/TcpIpConfig/TcpIpBufferMemoryConfig")

        if tcpip_parent_container is not None and tcpip_memory_parent_container is None:
            # 对于每一个新的子容器，创建并设置其参数
            for i in range(1, 5):
                # 使用API创建新的子容器
                new_container = AutosarUtil.create_sub_container_value(f"TcpIpBufferMemoryConfig_{i}",
                                                                    tcpipbuffermemoryconfig,
                                                                    tcpip_parent_container)
                if i == 1:
                    self.set_numerical_param_value('TcpIpBufferMemoryLen', new_container, "128")
                    self.set_numerical_param_value('TcpIpBufferMemoryNum', new_container, "50")
                elif i == 2:
                    self.set_numerical_param_value('TcpIpBufferMemoryLen', new_container, "256")
                    self.set_numerical_param_value('TcpIpBufferMemoryNum', new_container, "15")
                elif i == 3:
                    self.set_numerical_param_value('TcpIpBufferMemoryLen', new_container, "512")
                    self.set_numerical_param_value('TcpIpBufferMemoryNum', new_container, "10")
                else:
                    self.set_numerical_param_value('TcpIpBufferMemoryLen', new_container, "1540")
                    self.set_numerical_param_value('TcpIpBufferMemoryNum', new_container, "6")

        return True
