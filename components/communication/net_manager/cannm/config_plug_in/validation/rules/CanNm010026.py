from func_module.validation.common.basic_rule import BasicRule, timing_decorator
from durable.lang import *
from durable.engine import *



class CanNm010026Rule(BasicRule):
    ruleset_name = "CanNm010026"
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "CanNmCarWakeUpBytePosition should be larger than or equal to CanNmPduNidPosition + CanNmPduCbvPosition!"
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
                for channelkey, channelvalue in cannm_globalconfig_dict.items():
                    if not isinstance(cannm_globalconfig_dict, dict):
                        continue
                    if "DefinitionRef" not in cannm_globalconfig_dict:
                        continue
                    if channelkey == 'DefinitionRef':
                        continue
                    if "DefinitionRef" not in channelvalue:
                        continue
                    if channelvalue["DefinitionRef"] != '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig':
                        continue

                    CanNmChannelConfig_dict = cannm_globalconfig_dict[channelkey]
                    CanNmCarWakeUpBytePosition = CanNmChannelConfig_dict["CanNmCarWakeUpBytePosition"]["Value"]
                    CanNmPduCbvPosition = CanNmChannelConfig_dict["CanNmPduCbvPosition"]["Value"]
                    CanNmPduNidPosition = CanNmChannelConfig_dict["CanNmPduNidPosition"]["Value"]
                    if False == CanNmChannelConfig_dict["CanNmCarWakeUpFilterEnabled"]["Value"]:
                        break

                    if CanNmCarWakeUpBytePosition == None or CanNmCarWakeUpBytePosition == "":
                        valid = False
                        break

                    if CanNmPduCbvPosition == None or CanNmPduCbvPosition == "":
                        valid = False
                        break

                    if CanNmPduNidPosition == None or CanNmPduNidPosition == "":
                        valid = False
                        break

                    if CanNmPduNidPosition == "CANNM_PDU_BYTE_1":
                        CanNmPduNidPosition = 1
                    else:
                        CanNmPduNidPosition = 0

                    if CanNmPduCbvPosition == "CANNM_PDU_BYTE_1":
                        CanNmPduCbvPosition = 1
                    else:
                        CanNmPduCbvPosition = 0

                    if CanNmCarWakeUpBytePosition < CanNmPduCbvPosition + CanNmPduNidPosition:
                        valid = False
                        break
                service_id_path = "/ActiveEcuC/CanNm/" + CanNmConfigKey + "/" + channelkey + "/CanNmCarWakeUpBytePosition"
                # 2） 开始校验
                # 格式："错误码", "摘要", "错误信息", "错误索引路径" "结果"
                if valid:
                    self.vlog(
                    self.validation_code, self.validation_summary,
                    f"/ActiveEcuC/CanNm/{CanNmConfigKey}/CanNmChannelConfig/{channelkey}/CanNmCarWakeUpBytePosition[{CanNmCarWakeUpBytePosition}] should be larger than or equal to CanNm/{CanNmConfigKey}/CanNmChannelConfig/{channelkey}/CanNmPduNidPosition[{CanNmPduNidPosition}] + CanNmPduCbvPosition[{CanNmPduCbvPosition}]!",
                    service_id_path, "pass")
                else:
                    self.vlog(
                    self.validation_code, self.validation_summary,
                    f"/ActiveEcuC/CanNm/{CanNmConfigKey}/CanNmChannelConfig/{channelkey}/CanNmCarWakeUpBytePosition[{CanNmCarWakeUpBytePosition}] should be larger than or equal to CanNm/{CanNmConfigKey}/CanNmChannelConfig/{channelkey}/CanNmPduNidPosition[{CanNmPduNidPosition}] + CanNmPduCbvPosition[{CanNmPduCbvPosition}]!",
                    service_id_path, "fail")
                # 3）清空ruleset中的所有fact，为下一次校验做准备
                if len(validation_data_list) > 0:
                    retract_facts(self.__class__.ruleset_name, validation_data_list)
                return valid
        return valid

    def add_rule_time(self):
        pass
