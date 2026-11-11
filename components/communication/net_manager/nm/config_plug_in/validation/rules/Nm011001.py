from func_module.validation.common.basic_rule import BasicRule, timing_decorator
from durable.lang import *
from durable.engine import *


# Channel ref 重复校验
class Nm011001Rule(BasicRule):
    ruleset_name = 'Nm011001'
    validation_result = False

    def __init__(self, logs, bsw_dict, nm_dict) -> None:
        super().__init__()
        self.logs = logs
        self.bsw_dict = bsw_dict
        self.nm_dict = nm_dict

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

                @when_all(+m.channel_ref)
                def fact(c):
                    self.__class__.validation_result = True

            pass

    @timing_decorator
    def validation_run(self):
        valid = True
        # 1）准备校验数据
        ecuc_nm_dict = self.nm_dict

        nm_dict = ecuc_nm_dict
        base_path = '/ActiveEcuC/Nm'
        validation_data_list = []
        validataion_data_path_list = []
        for channel_key, channel_value in nm_dict.items():
            if not isinstance(channel_value, dict):
                continue
            if 'NmChannelConfig' == channel_value['DefinitionRef'].split('/')[-1]:
                service_table_path = base_path + '/' + channel_key
                service_path = service_table_path + '/NmComMChannelRef'

                validation_data = {}
                validation_data['channel_ref'] = channel_value['NmComMChannelRef']['ValueRef']
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
                self.vlog('Nm011001', 'Channel ref should be unique.',
                          f"Channel ref {validation_data['channel_ref']} should be unique.",
                          validataion_data_path_list[index], 'fail')
                valid = False

        # 3）清空ruleset中的所有fact，为下一次校验做准备
        if len(validation_data_list) > 0:
            retract_facts(self.__class__.ruleset_name, validation_data_list)
        pass

        return valid

    def add_rule_time(self):
        pass
