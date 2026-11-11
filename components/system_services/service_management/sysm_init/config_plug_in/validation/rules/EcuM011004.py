from func_module.validation.common.basic_rule import BasicRule, timing_decorator
from durable.lang import *
from durable.engine import *


# EcuMSleepModeId 重复校验
class EcuM011004Rule(BasicRule):
    ruleset_name = 'EcuM011004'
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

                @when_all(+m.status)
                def event(c):
                    pass

                @when_all(+m.mode_id)
                def fact(c):
                    self.__class__.validation_result = True

            pass

    @timing_decorator
    def validation_run(self):
        valid = True
        # 1）准备校验数据
        ecuc_ecum_dict = self.ecum_dict

        ecum_dict = ecuc_ecum_dict['EcuMConfiguration']['EcuMCommonConfiguration']
        base_path = '/ActiveEcuC/EcuM/EcuMConfiguration/EcuMCommonConfiguration'
        validation_data_list = []
        validataion_data_path_list = []
        for mode_key, mode_value in ecum_dict.items():
            if not isinstance(mode_value, dict):
                continue
            if 'EcuMSleepMode' == mode_value['DefinitionRef'].split('/')[-1]:
                service_table_path = base_path + '/' + mode_key
                service_path = service_table_path + '/' + 'EcuMSleepModeId'

                validation_data = {}
                validation_data['mode_id'] = mode_value['EcuMSleepModeId']['Value']
                validation_data_list.append(validation_data)
                validataion_data_path_list.append(service_path)

        # 2） 开始校验
        for index, validation_data in enumerate(validation_data_list):
            try:
                self.__class__.validation_result = False
                assert_fact(self.__class__.ruleset_name, validation_data)
            except Exception as e:
                self.__class__.validation_result = False
                pass

            # 格式：'错误码', '摘要', '错误信息', '错误索引路径' '结果'
            if self.__class__.validation_result:
                pass
            else:
                self.vlog('EcuM011004', 'SleepMode Id should be unique.',
                          f"SleepMode Id {validation_data['mode_id']} should be unique.",
                          validataion_data_path_list[index], 'fail')
                valid = False

        # 3）清空ruleset中的所有fact，为下一次校验做准备
        if len(validation_data_list) > 0:
            retract_facts(self.__class__.ruleset_name, validation_data_list)
        pass

        return valid

    def add_rule_time(self):
        pass
