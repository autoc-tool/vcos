from func_module.validation.common.basic_rule import BasicRule, timing_decorator
from durable.lang import *
from durable.engine import *


# DriverInitListOne UserCode 动态配置项缺失
class EcuM012005Rule(BasicRule):
    ruleset_name = "EcuM012005"
    validation_result = False

    def __init__(self, logs, bsw_dict, ecum_dict) -> None:
        super().__init__()
        self.logs = logs
        self.bsw_dict = bsw_dict
        self.ecum_dict = ecum_dict

        self.durable_init()

    def durable_init(self):
        # 注：固定名称的规则集只能初始化一次，否则会异常报错
        try:
            get_host().get_ruleset(self.__class__.ruleset_name)
        except Exception as e:
            with ruleset(self.__class__.ruleset_name):

                @when_all(m.module != '<empty>')
                def auto_gen(c):
                    self.__class__.validation_result = True

                @when_all((m.code != '') & (m.code != None))
                def dynamic_lost(c):
                    self.__class__.validation_result = True

            pass

    @timing_decorator
    def validation_run(self):
        valid = True
        # 1）准备校验数据
        ecuc_ecum_dict = self.ecum_dict

        ecum_dict = ecuc_ecum_dict['EcuMConfiguration']['EcuMCommonConfiguration']['EcuMDriverInitListOne']
        base_path = '/ActiveEcuC/EcuM/EcuMConfiguration/EcuMCommonConfiguration/EcuMDriverInitListOne'

        for module_key, module_value in ecum_dict.items():
            validation_data = {}
            validation_data['module'] = None
            validation_data['code'] = None
            if not isinstance(module_value, dict):
                continue
            if 'EcuMDriverInitItem' == module_value['DefinitionRef'].split('/')[-1]:
                module_path = base_path + '/' + module_key
                validataion_data_path = module_path + '/EcuMAdditionalInitCode'

                validation_data['module'] = module_value['EcuMModuleRef']['ValueRef']
                validation_data['code'] = module_value['EcuMAdditionalInitCode']['Value']

                # 2） 开始校验
                try:
                    self.__class__.validation_result = False
                    post(self.__class__.ruleset_name, validation_data)
                except Exception as e:
                    self.__class__.validation_result = False

                # 3） 打印日志
                if self.__class__.validation_result:
                    pass
                else:
                    self.vlog("ECUM012005", "EcuM ModuleRef is not defined, InitCode should be defined.",
                              f"{module_key} EcuM ModuleRef is not defined, InitCode should be defined.",
                              validataion_data_path, "fail")
                    valid = False

                retract_facts(self.__class__.ruleset_name, validation_data)
        return valid

    def add_rule_time(self):
        pass
