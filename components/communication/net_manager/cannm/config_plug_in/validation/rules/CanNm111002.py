from func_module.validation.common.basic_rule import BasicRule, timing_decorator
from durable.lang import *
from durable.engine import *


class CanNm111002Rule(BasicRule):
    ruleset_name = "CanNm111002"
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "CanNmPnInfo and CanNmPnFilterMaskByte should be configurated if CanNmGlobalPnSupport is set to True!"
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
        validation_data_list = []
        valid = True
        CanNmDict = self.bsw_ecuc_cfg_class_dict["CanNm"]["ArPackages"]["ActiveEcuC"]["CanNm"]
        for CanNmConfigKey, CanNmConfigValue in CanNmDict.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                cannm_globalconfig_dict = CanNmDict[CanNmConfigKey]
                CanNmPnFilterMaskByteNum = 0
                CanNmGlobalPnSupport = cannm_globalconfig_dict["CanNmGlobalPnSupport"]["Value"]
                for CanNmPnInfoConfigKey, CanNmPnInfoConfigValue in cannm_globalconfig_dict.items():
                    if CanNmPnInfoConfigKey == "DefinitionRef":
                        continue
                    if CanNmPnInfoConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmPnInfo':
                        for CanNmPnInfoKey, CanNmPnInfoValue in CanNmPnInfoConfigValue.items():
                            if CanNmPnInfoKey == 'DefinitionRef':
                                continue
                            if CanNmPnInfoKey in ["CanNmPnInfoLength", "CanNmPnInfoOffset"]:
                                continue
                            if CanNmPnInfoValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmPnInfo/CanNmPnFilterMaskByte':
                                CanNmPnFilterMaskByteNum = CanNmPnFilterMaskByteNum + 1

                if (0 == CanNmPnFilterMaskByteNum) and (True == CanNmGlobalPnSupport):
                    valid = False

                service_id_path = "/ActiveEcuC/CanNm/" + CanNmConfigKey
                # 2） 开始校验
                # 格式："错误码", "摘要", "错误信息", "错误索引路径" "结果"
                if valid:
                    self.vlog(
                    self.validation_code, self.validation_summary,
                    f"/ActiveEcuC/CanNm/{CanNmConfigKey}/CanNmPnInfo should be configurated if CanNm/{CanNmConfigKey}/CanNmGlobalPnSupport[{CanNmGlobalPnSupport}] is set to True!",
                    service_id_path, "pass")
                else:
                    self.vlog(
                    self.validation_code, self.validation_summary,
                    f"/ActiveEcuC/CanNm/{CanNmConfigKey}/CanNmPnInfo should be configurated if CanNm/{CanNmConfigKey}/CanNmGlobalPnSupport[{CanNmGlobalPnSupport}] is set to True!",
                    service_id_path, "fail")
                # 3）清空ruleset中的所有fact，为下一次校验做准备
                if len(validation_data_list) > 0:
                    retract_facts(self.__class__.ruleset_name, validation_data_list)
                return valid
        return valid

    def add_rule_time(self):
        pass

