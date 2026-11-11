from func_module.validation.common.basic_rule import BasicRule, timing_decorator
from durable.lang import *
from durable.engine import *

class MemPool015002Rule(BasicRule):
    """
    同一任务下的runnable周期应保持一致
    """
    ruleset_name = "MemPool015002"
    validation_result = True

    def __init__(self, logs, ecuc_mempool_dict) -> None:
        super().__init__()
        self.logs = logs
        self.ecuc_mempool_dict = ecuc_mempool_dict
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "The memalignment parameter of each mempool must be consistent," \
                    "The inconsistent configurations is:"
        self.durable_init()

    def durable_init(self):
        try:
            get_host().get_ruleset(self.__class__.ruleset_name)
        except Exception as e:
            with ruleset(self.__class__.ruleset_name):
                @when_all(+m.app_name == '')
                def action(c):
                    self.__class__.validation_result = True


    @timing_decorator
    def validation_run(self):
        """015表示:附加校验, 002表示:os模块子校验码(001~999)"""
        for mempool, value in self.ecuc_mempool_dict.items():
            if (not hasattr(value, 'get')) or (value.get('MemPoolRef') is None) or (value.get('MemPoolRef').get('ValueRef') is None):
                continue
            mempool_ref_list = [i.split("/")[-1] for i in value['MemPoolRef']['ValueRef']]
            MemAlignment = -1
            param_path = f"/ActiveEcuC/MemPool/{mempool}/MemPoolRef"
            for mempool, value in self.ecuc_mempool_dict.items():
                if (not hasattr(value, 'get')) or (value.get('MemAlignment') is None) or (value.get('DefinitionRef') is None) or (mempool not in mempool_ref_list):
                   continue
                if (MemAlignment != -1) and (MemAlignment != value['MemAlignment']['Value']):
                    self.vlog(self.validation_code, self.validation_summary + f"{mempool}", f"validation_data:{param_path}",
                                param_path, "fail")
                    self.__class__.validation_result = True
                    break
                MemAlignment = value['MemAlignment']['Value']
            if self.__class__.validation_result == True:
                break
            else:
                self.vlog(self.validation_code, self.validation_summary, f"validation_data:{param_path}",
                                param_path, "pass")
