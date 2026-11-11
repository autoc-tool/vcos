from func_module.validation.common.basic_rule import BasicRule, timing_decorator
from durable.lang import *
from durable.engine import *


class CanNm111001Rule(BasicRule):
    ruleset_name = "CanNm111001"
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "CanNmRxPduId should not be duplicated!"
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
        valid = True
        CanNmDict = self.bsw_ecuc_cfg_class_dict["CanNm"]["ArPackages"]["ActiveEcuC"]["CanNm"]
        for CanNmConfigKey, CanNmConfigValue in CanNmDict.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                cannm_globalconfig_dict = CanNmDict[CanNmConfigKey]
                base_path = "/ActiveEcuC/CanNm/" + CanNmConfigKey
                for channel_key, channel_value in cannm_globalconfig_dict.items():
                    if not isinstance(channel_value, dict):
                        continue
                    if "CanNmChannelConfig" == channel_value["DefinitionRef"].split("/")[-1]:
                        channel_path = base_path + "/" + channel_key
                        for cannmrxpdu_key, cannmrxpdu_value in channel_value.items():
                            if not isinstance(cannmrxpdu_value, dict):
                                continue
                            if "CanNmRxPdu" == cannm_globalconfig_dict[channel_key]["DefinitionRef"].split("/")[-1]:

                                cannmrxpdu_path = channel_path + "/" + cannmrxpdu_key
                                validation_data_list = []
                                validataion_data_path_list = []

                                service_path = cannmrxpdu_path
                                service_id_path = service_path + "/" + "CanNmRxPdu"
                                validation_data = {}
                                validation_data["CanNmRxPduId"] = cannm_globalconfig_dict[channel_key]["CanNmRxPduId"]["Value"]
                                validation_data_list.append(validation_data)
                                validataion_data_path_list.append(service_id_path)
                                # 2） 开始校验
                                for index, validation_data in enumerate(validation_data_list):
                                    try:
                                        self.__class__.validation_result = False
                                        assert_fact(self.__class__.ruleset_name, validation_data)
                                    except Exception as e:
                                        self.__class__.validation_result = False
                                        pass
                                    valid = self.__class__.validation_result
                                    # 格式："错误码", "摘要", "错误信息", "错误索引路径" "结果"
                                    if valid:
                                        self.vlog(self.validation_code, self.validation_summary,
                                                f"CanNmRxPduId {validation_data['CanNmRxPduId']} is not duplicated.",
                                                validataion_data_path_list[index], "pass")
                                    else:
                                        self.vlog(self.validation_code, self.validation_summary,
                                                f"CanNmRxPduId {validation_data['CanNmRxPduId']} is duplicated!",
                                                validataion_data_path_list[index], "fail")

                                # 3）清空ruleset中的所有fact，为下一次校验做准备
                                if len(validation_data_list) > 0:
                                    retract_facts(self.__class__.ruleset_name, validation_data_list)
                                return valid
        return valid

    def add_rule_time(self):
        pass
