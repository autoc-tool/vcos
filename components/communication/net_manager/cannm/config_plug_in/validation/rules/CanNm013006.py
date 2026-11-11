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

def check_prefix(string):
    prefix = 'CanNm_MainFunction'
    # 检查字符串前面的字符是否为'CanNm_MainFunction'
    if string.startswith(prefix):
        return True
    else:
        return False

# 互斥校验：CanNm MainFunction必须有Ref关联
class CanNm013006Rule(BasicRule):
    ruleset_name = "CanNm013006"
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.bsw_ecuc_cfg_class_dict = bsw_ecuc_cfg_class_dict
        self.ecuc_cannm_dict = bsw_ecuc_cfg_class_dict
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "Validation Of CanNmMainFunction Name!"

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

                @when_all(+m.comsignal_name)
                def fact(c):
                    self.__class__.validation_result = True

    def check_validation_run(self):
        if not self.modules_are_exist(["CanNm"], self.bsw_ecuc_cfg_class_dict):
            self.vlog(self.validation_code, self.validation_summary, "Data required for validation is missing",
                self.validation_summary, "fail")
            return False
        return True


    @timing_decorator
    def validation_run(self):
        validation_data_list = []
        validataion_data_path_list = []
        valid = True

        # 1）准备校验数据
        CanNmGlobalConfig = {}
        ecuc_cannm_dict = self.ecuc_cannm_dict
        if "CanNm" in ecuc_cannm_dict.keys():
            cannm_dict = ecuc_cannm_dict['CanNm']['ArPackages']['ActiveEcuC']['CanNm']
            cannm_config_dict = cannm_tools_get_dict_by_definition_ref(cannm_dict,"CanNmGlobalConfig")
        else:
            cannm_config_dict = cannm_tools_get_dict_by_definition_ref(ecuc_cannm_dict,"CanNmGlobalConfig")

        CanNmMainFunction = {}
        for CanNmConfigKey, value in cannm_config_dict.items():
            if CanNmConfigKey == "DefinitionRef" or CanNmConfigKey == "Tag":
                continue
            if isinstance(value, dict):
                if value["DefinitionRef"] == "/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmMainFunction":
                    CanNmMainFunction[CanNmConfigKey] = value
        
        base_path = "/ActiveEcuC/CanNm/CanNmGlobalConfig"

        check_result = True
        # 2）每个MainFunction进行校验
        for CanNmMainFunction in list(CanNmMainFunction.keys()):
            validation_data = {}
            validation_data["mainfunction_name"] = CanNmMainFunction
            cannm_mainfunction_ref_path = base_path + "/" + CanNmMainFunction
            check_result = check_prefix(CanNmMainFunction)

            if  check_result == False:
                self.vlog(self.validation_code, self.validation_summary,
                            f"CanNmMainFunction  {validation_data['mainfunction_name']} container name prefix must be CanNm_MainFunction", 
                            cannm_mainfunction_ref_path, "fail")
                valid = False
                return valid

        return valid

