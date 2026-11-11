from func_module.validation.common.basic_rule import BasicRule, timing_decorator
from durable.lang import *
from durable.engine import *

class MemPool015001Rule(BasicRule):
    """
    同一任务下的runnable周期应保持一致
    """
    ruleset_name = "MemPool015001"
    validation_result = True

    def __init__(self, logs, ecuc_mempool_dict) -> None:
        super().__init__()
        self.logs = logs
        self.ecuc_mempool_dict = ecuc_mempool_dict
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "The mempool module at the core level should be associated with one applications. Please check the configuration."
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
        """015表示:附加校验, 001表示:os模块子校验码(001~999)"""
        core_mmempool_list = []
        for mempool, value in self.ecuc_mempool_dict.items():
            if (not hasattr(value, 'get')) or (value.get('MemType') is None) or (value.get('DefinitionRef') is None):
                continue
            if value['MemType']['Value'] == 'CORE':
                core_mmempool_list.append(mempool)
        for mempool, value in self.ecuc_mempool_dict.items():
            if (not hasattr(value, 'get')) or (value.get('MemPoolRef') is None) or (value.get('MemPoolRef').get('ValueRef') is None):
                continue
            mempool_ref_list = [i.split("/")[-1] for i in value['MemPoolRef']['ValueRef']]
            if any(item in core_mmempool_list for item in mempool_ref_list):
                count = 0
                if isinstance(value['OsApplicationRef']['ValueRef'], list):
                    count = len(value['OsApplicationRef']['ValueRef'])
                elif isinstance(value['OsApplicationRef']['ValueRef'], str):
                    count = 1
                param_path = f"/ActiveEcuC/MemPool/{mempool}/OsApplicationRef"
                if count != 1:
                    self.__class__.validation_result = False
                    # 格式："错误码", "摘要", "错误信息", "错误索引路径" "结果"
                    if self.__class__.validation_result:
                        self.vlog(self.validation_code, self.validation_summary, f"validation_data:{param_path}",
                                param_path, "pass")
                    else:
                        self.vlog(self.validation_code, self.validation_summary, f"validation_data:{param_path}",
                                param_path, "fail")
                    self.__class__.validation_result = True
                else:
                    self.vlog(self.validation_code, self.validation_summary, f"validation_data:{param_path}",
                                param_path, "pass")