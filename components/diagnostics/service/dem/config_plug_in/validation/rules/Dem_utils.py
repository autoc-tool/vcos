# This Python file uses the following encoding: utf-8
# ##############################################################################
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
# ##############################################################################

class DemRuleUtils():

    def __init__(self) -> None:
        pass

    def dem_get_dict_by_definition_ref(input_dict: dict, definition_ref: str) -> dict:
        for key, value in input_dict.items():
            if isinstance(value, dict):
                if "DefinitionRef" in value.keys():
                    if value["DefinitionRef"].split("/")[-1] == definition_ref:
                        return key, value
        return "", {}