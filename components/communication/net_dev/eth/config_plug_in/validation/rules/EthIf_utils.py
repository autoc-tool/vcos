# /*
#  * Copyright (c) 2025 Li Auto Inc. and its affiliates
#  * Licensed under the Apache License, Version 2.0(the "License");
#  * you may not use this file except in compliance with the License.
#  * You may obtain a copy of the License at
#  *
#  *     http://www.apache.org/licenses/LICENSE-2.0
#  *
#  * Unless required by applicable law or agreed to in writing, software
#  * distributed under the License is distributed on an "AS IS" BASIS,
#  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  * See the License for the specific language governing permissions and
#  * limitations under the License.
#  */
class EthIfRuleUtils():
    EthIf_Target = None

    def __init__(self) -> None:
        pass

    @classmethod
    def set_ethif_target(cls, target: str):
        cls.EthIf_Target = target

    @classmethod
    def set_mcal_gendata_path(cls, path: str):
        cls.Mcal_Gendata_Path = path

    def ethif_get_dict_by_definition_ref(input_dict: dict, definition_ref: str) -> dict:
        for key, value in input_dict.items():
            if isinstance(value, dict):
                if "DefinitionRef" in value.keys():
                    if value["DefinitionRef"].split("/")[-1] == definition_ref:
                        return key, value
        return "", {}