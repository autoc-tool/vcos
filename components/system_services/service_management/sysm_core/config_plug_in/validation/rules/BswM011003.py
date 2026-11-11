from func_module.validation.common.basic_rule import BasicRule, timing_decorator
from durable.lang import *
from durable.engine import *


# BswMActionListItemIndex 重复校验
class BswM011003Rule(BasicRule):
    ruleset_name = 'BswM011003'
    validation_result = False

    def __init__(self, logs, bsw_dict, bswm_dict) -> None:
        super().__init__()
        self.logs = logs
        self.bsw_dict = bsw_dict
        self.bswm_dict = bswm_dict

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

                @when_all(+m.item_idx)
                def fact(c):
                    self.__class__.validation_result = True

            pass

    @timing_decorator
    def validation_run(self):
        valid = True
        # 1）准备校验数据
        ecuc_bswm_dict = self.bswm_dict

        bswm_dict = ecuc_bswm_dict['BswMConfig']['BswMArbitration']
        base_path = '/ActiveEcuC/BswM/BswMConfig/BswMArbitration'

        validataion_data_path_list = []
        for request_key, request_value in bswm_dict.items():
            if not isinstance(request_value, dict):
                continue
            if 'BswMLogicalExpression' == request_value['DefinitionRef'].split(
              '/')[-1] :
                if isinstance(request_value["BswMArgumentRef"]["ValueRef"], str):
                    if request_key == request_value["BswMArgumentRef"]["ValueRef"].split('/')[-1]:
                        path = base_path + '/' + request_key
                        validataion_data_path_list.append(path)
                elif isinstance(request_value["BswMArgumentRef"]["ValueRef"], list):
                    for value_exp in request_value["BswMArgumentRef"]["ValueRef"]:
                        if request_key == value_exp.split('/')[-1]:
                            path = base_path + '/' + request_key
                            validataion_data_path_list.append(path)
            
                
        # 格式：'错误码', '摘要', '错误信息', '错误索引路径' '结果'
        if len(validataion_data_path_list) == 0:
            pass
        else:
            for i,invalid_item in enumerate(validataion_data_path_list):
                self.vlog('BswM011003', 'Logical expression cannot ref itself', f"Logical expression { invalid_item } cannot ref itself.",
                        validataion_data_path_list[i], 'fail')
            valid = False
        # 3）清空ruleset中的所有fact，为下一次校验做准备
        if len(validataion_data_path_list) > 0:
            retract_facts(self.__class__.ruleset_name, validataion_data_path_list)
        pass            
        return valid

    def add_rule_time(self):
        pass
