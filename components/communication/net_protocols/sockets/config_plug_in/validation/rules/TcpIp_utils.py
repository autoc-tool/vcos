import os
from copy import deepcopy

class TcpIpRuleUtils():
    def __init__(self) -> None:
        pass

    def validate_utils_flatten_config(self, config_dict: dict, path: str = "") -> dict:
        result = []
        for key, val in config_dict.items():
            if key == 'DefinitionRef':
                continue
            if type(val) == dict:
                result += self.validate_utils_flatten_config(val, path + '/' + key)
            else:
                result += [{"path": path, "value": val}]
        return result

    def valudate_utils_get_refered_ctrl(self, config_dict: dict) -> dict:
        result = []
        tcpipconfig = config_dict['TcpIpConfig']
        for key, val in tcpipconfig.items():
            if "DefinitionRef" not in val:
                continue
            if val["DefinitionRef"] != "/AUTOSAR/EcucDefs/TcpIp/TcpIpConfig/TcpIpCtrl":
                continue
            try:
                result.append(val["TcpIpEthIfCtrlRef"]["ValueRef"])
            except:
                pass
        
        return result
    
    def tcpip_get_dict_by_definition_ref(input_dict: dict, definition_ref: str) -> dict:
        for value in input_dict.values():
            if isinstance(value, dict):
                if "DefinitionRef" in value.keys():
                    if value["DefinitionRef"].split("/")[-1] == definition_ref:    
                        return value
        return {}