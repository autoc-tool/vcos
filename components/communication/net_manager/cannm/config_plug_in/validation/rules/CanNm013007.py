from func_module.validation.common.basic_rule import BasicRule, timing_decorator
from durable.lang import *
from durable.engine import *


# 通过definition 获取对应的dict
def cannm_tools_get_dict_by_definition_ref(input_dict: dict, definition_ref: str) -> dict:
    for k, v in input_dict.items():
        if isinstance(v, dict):
            if "DefinitionRef" in v.keys():
                if v["DefinitionRef"].split("/")[-1] == definition_ref:
                    return v
    return {}

class CanNm013007Rule(BasicRule):
    ruleset_name = "CanNm013007"
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "The count of CanNmMainFunction cfg should be equal to the core count of channels!"
        self.bsw_ecuc_cfg_class_dict = bsw_ecuc_cfg_class_dict

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

                @when_all(+m.frame_id)
                def fact(c):
                    self.__class__.validation_result = True

            pass

    def check_validation_run(self):
        if not self.modules_are_exist(["CanNm"], self.bsw_ecuc_cfg_class_dict):
            self.vlog(self.validation_code, self.validation_summary, "Data required for validation is missing",
                self.validation_summary, "fail")
            return False
        return True

    @timing_decorator
    def validation_run(self):
        # 1）准备校验数据
        ecuc_config_dict = self.bsw_ecuc_cfg_class_dict["EcuC"]["ArPackages"]["ActiveEcuC"]["EcuC"]
        validation_data_list = []
        valid = True
        samecount = 0
        pducoreiddict = {}
        mainfunctioncount = 0
        pducount = 0
        ecuc_pdu = ecuc_config_dict["EcucConfigSet"]["EcucPduCollection"]
        cannm_globalconfig_dict = {}
        CanNmDict = self.bsw_ecuc_cfg_class_dict["CanNm"]["ArPackages"]["ActiveEcuC"]["CanNm"]
        for CanNmConfigKey, CanNmConfigValue in CanNmDict.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                cannm_globalconfig_dict = CanNmDict[CanNmConfigKey]
                for channelkey, channelvalue in cannm_globalconfig_dict.items():
                    if not isinstance(cannm_globalconfig_dict, dict):
                        continue
                    if "DefinitionRef" not in cannm_globalconfig_dict:
                        continue
                    if channelkey == 'DefinitionRef':
                        continue
                    if "DefinitionRef" not in channelvalue:
                        continue
                    if channelvalue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmMainFunction':
                        mainfunctioncount = mainfunctioncount + 1
                    if channelvalue["DefinitionRef"] != '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig':
                        continue

                    CanNmRxPdu_dict = cannm_globalconfig_dict[channelkey]
                    for rxpdukey, rxpduvalue in CanNmRxPdu_dict.items():
                        if not isinstance(CanNmRxPdu_dict, dict):
                            continue
                        if (not rxpdukey.endswith('_Tx')):
                            continue
                        txpdu_temp = rxpdukey.strip('_Tx')
                        # print("CanNmRxPdu_dict,key:", rxpdukey)
                        CanNmTxPduRef = CanNmRxPdu_dict[txpdu_temp]["CanNmTxPduRef"]["ValueRef"]
                        if CanNmTxPduRef is None:
                            valid = False
                            break

                        elif len(CanNmTxPduRef) == 0 or CanNmTxPduRef == "<empty>":
                            valid = False
                            break

                        else:
                            CanNmTxPduRef = CanNmTxPduRef.split('/')[-1]
                            for ecuckey, ecucvalue in ecuc_pdu.items():
                                if ecuckey == CanNmTxPduRef:
                                    EcucPduDefaultCoreRef = ecuc_pdu[ecuckey]["EcucPduDefaultCoreRef"]["ValueRef"]
                                    if EcucPduDefaultCoreRef is None:
                                        pducoreiddict[samecount] = 0
                                    elif len(EcucPduDefaultCoreRef) == 0 or EcucPduDefaultCoreRef == "<empty>":
                                        pducoreiddict[samecount] = 0
                                    else:
                                        pducoreiddict[samecount] = ecuc_pdu[ecuckey]["EcucPduDefaultCoreRef"]["ValueRef"][-1]
                                    samecount = samecount + 1

        pducount = len(set(pducoreiddict.values()))
        if pducount == mainfunctioncount:
            valid = True
        else:
            valid = False

        service_id_path = "/ActiveEcuC/CanNm/" + CanNmConfigKey + "/CanNmMainFunction"
        # 2） 开始校验
        # 格式："错误码", "摘要", "错误信息", "错误索引路径" "结果"
        if valid:
            self.vlog(
            self.validation_code, self.validation_summary,
            f"The count[{mainfunctioncount}] of /ActiveEcuC/CanNm/{CanNmConfigKey}/CanNmMainFunction cfg should be equal to the core count[{pducount}] of channels!",
            service_id_path, "pass")
        else:
            self.vlog(
            self.validation_code, self.validation_summary,
            f"The count[{mainfunctioncount}] of /ActiveEcuC/CanNm/{CanNmConfigKey}/CanNmMainFunction cfg should be equal to the core count[{pducount}] of channels!",
            service_id_path, "fail")
        # 3）清空ruleset中的所有fact，为下一次校验做准备
        if len(validation_data_list) > 0:
            retract_facts(self.__class__.ruleset_name, validation_data_list)
        return valid

    def add_rule_time(self):
        pass

