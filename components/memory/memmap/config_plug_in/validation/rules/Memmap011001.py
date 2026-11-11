# This Python file uses the following encoding: utf-8
############################################################################
# Copyright (c) 2025 Li Auto Inc. and its affiliates
# Licensed under the Apache License, Version 2.0(the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
############################################################################
from func_module.validation.common.basic_rule import BasicRule, timing_decorator
from durable.lang import *
from durable.engine import *

class Memmap011001Rule(BasicRule):
    """Memmap 模块必须配置唯一一个internal layer"""
    ruleset_name = "Memmap011001"
    validation_result = False

    def __init__(self, logs, ecuc_memmap_dict) -> None:
        super().__init__()
        self.logs = logs
        self.ecuc_memmap_dict = ecuc_memmap_dict
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "Memmap module must config one and only one internal layer."
        self.internal_layer = []
        self.durable_init()

    def durable_init(self):
        try:
            get_host().get_ruleset(self.__class__.ruleset_name)
        except Exception as e:
            pass

    @timing_decorator
    def validation_run(self):
        for memmap_key, memmap_instance in self.ecuc_memmap_dict.items():
            if not isinstance(memmap_instance, dict):
                continue
            if "/MemmapLayer" in memmap_instance["DefinitionRef"]:
                if memmap_instance['InternalLayer']['Value']:
                    self.internal_layer.append(memmap_key)

        if len(self.internal_layer) == 1:
            self.__class__.validation_result = True
        else:
            self.__class__.validation_result = False
            validation_data = {
                'all_internal_layer': self.internal_layer
            }
            try:
                post(self.__class__.ruleset_name, validation_data)
            except Exception as e:
                pass

        param_path = "/ActiveEcuC/Memmap/layer/InternalLayer"
        self.vlog(
            self.validation_code,
            self.validation_summary,
            f"validation_data:{param_path}",
            param_path,
            "pass" if self.__class__.validation_result else "fail"
        )
