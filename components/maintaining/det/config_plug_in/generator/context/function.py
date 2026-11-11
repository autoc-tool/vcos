from jinja2 import pass_context
import copy
import math
from func_module.generator.common.common import FunctionBase
import json
module_id_dict = {
    "Adc": 123,
    "BswM": 42,
    "Can": 80,
    "CanIf": 60,
    "CanNm": 31,
    "CanSM": 140,
    "CanTrcv": 70,
    "CanTp": 35,
    "Com": 50,
    "ComM": 12,
    "ComXf": 175,
    "CorTst": 103,
    "Crypto": 114,
    "CryIf": 112,
    "Csm": 110,
    "Dcm": 53,
    "Dem": 54,
    "Det": 15,
    "Dlt": 55,
    "DoIP": 173,
    "Mirror": 48,
    "SchM": 130,
    "Dio": 120,
    "E2EXf": 176,
    "EcuM": 10,
    "Ea": 40,
    "Eep": 90,
    "Eth": 880,
    "EthIf": 65,
    "EthSM": 143,
    "EthSwt": 89,
    "EthTrcv": 73,
    "Fee": 21,
    "FiM": 11,
    "Fls": 92,
    "FlsTst": 104,
    "Fr": 81,
    "FrArTp": 38,
    "FrIf": 61,
    "FrNm": 32,
    "FrSM": 142,
    "FrTp": 36,
    "FrTrcv": 71,
    "Gpt": 100,
    "Icu": 122,
    "IpduM": 52,
    "KeyM": 109,
    "LdCom": 49,
    "Lin": 82,
    "LinIf": 62,
    "LinSM": 141,
    "LinTrcv": 64,
    "Mcu": 101,
    "MemIf": 22,
    "Mirror": 48,
    "Nm": 29,
    "NvM": 20,
    "Ocu": 125,
    "Os": 1,
    "PduR": 51,
    "Port": 124,
    "Pwm": 121,
    "RamTst": 93,
    "Rte": 2,
    "SchM": 130,
    "Sd": 171,
    "SecOC": 150,
    "SoAd": 56,
    "SomeIpTp": 177,
    "SomeIpXf": 174,
    "Spi": 83,
    "StbM": 160,
    "TcpIp": 170,
    "Tm": 14,
    "Ttcan": 84,
    "TtcanIf": 66,
    "UdpNm": 33,
    "V2xBtp": 183,
    "V2xFac": 184,
    "V2xGn": 182,
    "V2xM": 185,
    "Wdg": 102,
    "WdgIf": 43,
    "WdgM": 13,
    "WEth": 87,
    "WEthTrcv": 74,
    "Xcp": 212,
    "Crc":201,
    "Bfx": 205,
    "E2E":207,
    "Efx":208,
    "IfI":209,
    "Mfl":210,
    "Mfx":211,
    "Ifx":213,


}

class Function(FunctionBase):
    """
    Class containing all functions. Add new static functions here
    to be included in the jinja2 context
    """
    @staticmethod
    def get_detflow_name(key_name: str) -> str:
        key_name = key_name.split("/")[-1]
        return key_name

    @staticmethod
    @pass_context
    def get_detgeneral_json_config(context):
        return context["det_json"]["DetGeneral"]

    @staticmethod
    @pass_context
    def get_det_module_flow_config(context) -> list:
        output = {}
        module_info_list = []
        det_flow_control_cfg = context["det_json"]["DetGeneral"]

        for key, value in det_flow_control_cfg.items():
            if key == "DefinitionRef":
                continue
            else:
                if(Function.get_detflow_name(value["DefinitionRef"]) == "DetFlowControl"):
                    output["ModuleName"] = key
                    output["ModuleId"] = module_id_dict.get((value["ModuleRef"]["ValueRef"]).split("/")[-1], 255)
                    output["ModuleFlowNum"] = value["ModuleTokenNum"]["Value"]
                    output["ModuleDetFlowControl"] = value["ModuleDetFlowControl"]["Value"]
                    module_info_list.insert(output['ModuleId'], copy.deepcopy(output))
        return module_info_list


    @staticmethod
    @pass_context
    def get_det_flow_control_config(context):
        flow_config = {}
        DetFlowTokenPeriodExponent = 10
        det_period = [256, 512, 1024, 2048, 4096, 8192]
        FlowControlEnable = context["det_json"]["DetGeneral"]["FlowControlEnable"]["Value"]
        DetFlowTokenNum = context["det_json"]["DetGeneral"]["DetTokenNum"]["Value"]
        DetFlowTokenPeriod = context["det_json"]["DetGeneral"]["DetTokenPeriod"]["Value"]
        if int(DetFlowTokenPeriod) in det_period:
            DetFlowTokenPeriodExponent = int(math.log(int(DetFlowTokenPeriod), 2))
        FlowControlNum = 0
        det_flow_control_cfg = context["det_json"]["DetGeneral"]
        for key, value in det_flow_control_cfg.items():
            if key == "DefinitionRef":
                continue
            else:
                if(Function.get_detflow_name(value["DefinitionRef"]) == "DetFlowControl"):
                    FlowControlNum = FlowControlNum+1

        flow_config["FlowControlEnable"] = FlowControlEnable
        flow_config["DetFlowTokenNum"] = DetFlowTokenNum
        flow_config["DetFlowTokenPeriod"] = DetFlowTokenPeriod
        flow_config["DetFlowTokenPeriodExponent"] = DetFlowTokenPeriodExponent
        flow_config["FlowControlNum"] = FlowControlNum
        return flow_config


    @staticmethod
    def get_module_info_array():
            return [[(0, False)] for _ in range(256)]

    @staticmethod
    @pass_context
    def get_module_flow_control_info(context):
        modified_array = Function.get_module_info_array()
        module_list = Function.get_det_module_flow_config(context)
        if len(module_list) == 0:
            print("module_list 是空的")
        else:
            for index, item in enumerate(module_list):
                module_id = item["ModuleId"]
                if (0 <= module_id < len(modified_array)):
                    if (item["ModuleDetFlowControl"] == True):
                        modified_array[module_id] = (index, True)
                    else:
                        modified_array[module_id] = (index, False)
        # 格式化输出
        formatted_output = "\n"
        for i, item in enumerate(modified_array):
            if isinstance(item, (list, tuple)):
                id_value, bool_value = item[0] if isinstance(item, list) else item
            else:
                id_value, bool_value = item, False  # 假设单个值为 ID，布尔值默认为 False

            formatted_item = f"     {{{id_value}, {'(boolean)TRUE' if bool_value else '(boolean)FALSE'}}}"
            if i % 4 == 0 and i != 0:
                formatted_output += ",\n"
            elif i != 0:
                formatted_output += ", "
            formatted_output += formatted_item
            if i == len(modified_array) - 1:
                formatted_output += ""
        formatted_output += "\n"
        return formatted_output


    def switch_bool_to_std_on(value) -> str:
        if (value == True):
            return "STD_ON"
        elif (value == False):
            return "STD_OFF"
        else:
            return "error"

    @staticmethod
    @pass_context
    def get_detnotification_json_config(context):
        return context["det_json"]["DetNotification"]

    @staticmethod
    @pass_context
    def compute_runtime_json_config(context):
        len = Function.compute_runtime_nbr_json_config(context)
        if (len == 0):
            return str("STD_OFF")
        else:
            return str("STD_ON")

    @staticmethod
    @pass_context
    def compute_runtime_nbr_json_config(context):
        len = 0
        for key, value in context["det_json"].items():
            if (key == "Tag" or key == "DefinitionRef"):
                continue
            if ("Det/DetNotification" in value["DefinitionRef"]):
                #进入notification
                if (isinstance(value["DetReportRuntimeErrorCallout"]["Value"], str) == True):
                    if (value["DetReportRuntimeErrorCallout"]["Value"] != "<empty>"):
                        len = len + 1
                elif (isinstance(value["DetReportRuntimeErrorCallout"]["Value"], list) == True):
                    for value2 in value["DetReportRuntimeErrorCallout"]["Value"]:
                        if (value2 != "<empty>"):
                            len = len + 1
        return len

    @staticmethod
    @pass_context
    def get_runtime_func_config(context):
        len = 0
        func_list = []
        for key, value in context["det_json"].items():
            if (key == "Tag" or key == "DefinitionRef"):
                continue
            if ("Det/DetNotification" in value["DefinitionRef"]):
                #进入notification
                if (isinstance(value["DetReportRuntimeErrorCallout"]["Value"], str) == True):
                    if (value["DetReportRuntimeErrorCallout"]["Value"] != "<empty>"):
                        func_list.append(value["DetReportRuntimeErrorCallout"]["Value"])
                elif (isinstance(value["DetReportRuntimeErrorCallout"]["Value"], list) == True):
                    for value2 in value["DetReportRuntimeErrorCallout"]["Value"]:
                        if (value2 != "<empty>"):
                            func_list.append(value2)
        return func_list

    @staticmethod
    @pass_context
    def compute_transient_json_config(context):
        len = Function.compute_transient_nbr_json_config(context)
        if (len == 0):
            return str("STD_OFF")
        else:
            return str("STD_ON")

    @staticmethod
    @pass_context
    def compute_transient_nbr_json_config(context):
        len = 0
        for key, value in context["det_json"].items():
            if (key == "Tag" or key == "DefinitionRef"):
                continue
            if ("Det/DetNotification" in value["DefinitionRef"]):
                #进入notification
                if (isinstance(value["DetReportTransientFaultCallout"]["Value"], str) == True):
                    if (value["DetReportTransientFaultCallout"]["Value"] != "<empty>"):
                        len = len + 1
                elif (isinstance(value["DetReportTransientFaultCallout"]["Value"], list) == True):
                    for value2 in value["DetReportTransientFaultCallout"]["Value"]:
                        if (value2 != "<empty>"):
                            len = len + 1
        return len

    @staticmethod
    @pass_context
    def get_transient_func_config(context):
        len = 0
        func_list = []
        for key, value in context["det_json"].items():
            if (key == "Tag" or key == "DefinitionRef"):
                continue
            if ("Det/DetNotification" in value["DefinitionRef"]):
                #进入notification
                if (isinstance(value["DetReportTransientFaultCallout"]["Value"], str) == True):
                    if (value["DetReportTransientFaultCallout"]["Value"] != "<empty>"):
                        func_list.append(value["DetReportTransientFaultCallout"]["Value"])
                elif (isinstance(value["DetReportTransientFaultCallout"]["Value"], list) == True):
                    for value2 in value["DetReportTransientFaultCallout"]["Value"]:
                        if (value2 != "<empty>"):
                            func_list.append(value2)
        return func_list

    @staticmethod
    @pass_context
    def compute_errorhook_json_config(context):
        len = Function.compute_errorhook_nbr_json_config(context)
        if (len == 0):
            return str("STD_OFF")
        else:
            return str("STD_ON")

    @staticmethod
    @pass_context
    def compute_errorhook_nbr_json_config(context):
        len = 0
        for key, value in context["det_json"].items():
            if (key == "Tag" or key == "DefinitionRef"):
                continue
            if ("Det/DetNotification" in value["DefinitionRef"]):
                #进入notification
                if (isinstance(value["DetErrorHook"]["Value"], str) == True):
                    if (value["DetErrorHook"]["Value"] != "<empty>"):
                        len = len + 1
                elif (isinstance(value["DetErrorHook"]["Value"], list) == True):
                    for value2 in value["DetErrorHook"]["Value"]:
                        if (value2 != "<empty>"):
                            len = len + 1
        return len

    @staticmethod
    @pass_context
    def get_errorhook_func_config(context):
        len = 0
        func_list = []
        for key, value in context["det_json"].items():
            if (key == "Tag" or key == "DefinitionRef"):
                continue
            if ("Det/DetNotification" in value["DefinitionRef"]):
                #进入notification
                if (isinstance(value["DetErrorHook"]["Value"], str) == True):
                    if (value["DetErrorHook"]["Value"] != "<empty>"):
                        func_list.append(value["DetErrorHook"]["Value"])
                elif (isinstance(value["DetErrorHook"]["Value"], list) == True):
                    for value2 in value["DetErrorHook"]["Value"]:
                        if (value2 != "<empty>"):
                            func_list.append(value2)
        return func_list

    @staticmethod
    @pass_context
    def get_all_modulename(context):
        temp = {}
        DetConfigSet = context["det_json"]["DetConfigSet"]
        for key, value in DetConfigSet.items():
            if (key == "Tag" or key == "DefinitionRef"):
                continue
            if ("Det/DetConfigSet/DetModule" in value["DefinitionRef"]):
                temp[str(key)] = value["DetModuleId"]["Value"]
        return temp

    @staticmethod
    @pass_context
    def get_all_instance_name(context):
        temp = {}
        DetConfigSet = context["det_json"]["DetConfigSet"]
        for key, value in DetConfigSet.items():
            if (key == "Tag" or key == "DefinitionRef"):
                continue
            if ("Det/DetConfigSet/DetModule" in value["DefinitionRef"]):
                temp_instance = key
                for key2, value2 in value.items():
                    if (key2 == "DefinitionRef"):
                        continue
                    if ("Det/DetConfigSet/DetModule/DetModuleInstance" in value2["DefinitionRef"]):
                        temp_instance = temp_instance + "_" + str(key2)
                        temp[str(temp_instance)] = value2["DetInstanceId"]["Value"]
        return temp


    @staticmethod
    @pass_context
    def get_bsw_callout_user_code_list(context):
        usercode_list = context["functions"]
        return usercode_list
