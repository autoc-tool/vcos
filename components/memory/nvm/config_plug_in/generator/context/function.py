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

from jinja2 import pass_context
from func_module.generator.common.common import FunctionBase


class Function(FunctionBase):

    @staticmethod
    @pass_context
    def test(context):
        return 0

    @staticmethod
    @pass_context
    def check_nvm_notifybswm(context):
        """
         判断是否有block需要通知到BswM
        """
        blockdescriptors = context["NvM_Blocks"]
        ret = False
        for block in blockdescriptors.values():
            if (block["NvMBswMBlockStatusInformation"]["Value"] == True):
                ret = True
                break
        return ret

    @staticmethod
    @pass_context
    def get_nvm_blocks(context):
        """
         获取所有NvM的BlockDescriptor数量，总数量为配置的block数加保留block（block 0）
        """
        blockdescriptors = context["NvM_Blocks"]
        cnt = len(blockdescriptors) + 1
        return cnt

    @staticmethod
    @pass_context
    def get_nvm_maxblocklength(context):
        """
         获取NvM中最大的blocklength，这里额外加上了CRC64的长度（16）
        """
        blockdescriptors = context["NvM_Blocks"]
        maxlength = 0
        for block in blockdescriptors.values():
            if (int(block["NvMNvBlockLength"]["Value"]) > int(maxlength)):
                maxlength = block["NvMNvBlockLength"]["Value"]

        return int(maxlength) + 32

    @staticmethod
    @pass_context
    def check_crc32_inuse(context):
        inuse = "STD_OFF"
        blockdescriptors = context["NvM_Blocks"]
        for block in blockdescriptors.values():
            if block["NvMBlockCrcType"]["Value"] == "NVM_CRC32":
                inuse = "STD_ON"
                break
        return inuse


    @staticmethod
    @pass_context
    def get_nvm_includefiles(context):
        func_list = []
        for key, value in context["NvM_General"]["NvMCommonVendorParams"].items():
            if (key == "Tag" or key == "DefinitionRef"):
                continue
            if ("NvMCommonVendorParams/NvMCfgCbkIncludeList" in value["DefinitionRef"]):
                #进入notification
                if (isinstance(value["Value"], str) == True):
                    if (value["Value"] != "<empty>"):
                        func_list.append(value["Value"])
                elif (isinstance(value["Value"], list) == True):
                    for value2 in value["Value"]:
                        if (value2 != "<empty>"):
                            func_list.append(value2)
        return func_list

    @staticmethod
    def gen_switch_macro(base_dict: dict, key_str: str):
        res = "STD_OFF"
        if base_dict.get(key_str):
            if base_dict.get(key_str).get('Value') == True:
                res = "STD_ON"
        return res

    @staticmethod
    def gen_switch_bool(key_value: str) -> str:
        if key_value == False:
            res = "FALSE"
        else:
            res = "TRUE"
        return res

    @staticmethod
    def get_prt_value(base_dict: dict, key_str: str):
        if base_dict.get(key_str):
            res = base_dict.get(key_str).get('Value')
            if res == '' or res == None:
                res = "NULL"
        else:
            res = "NULL"
        if res != "NULL":
            res = f"&{res}"
        return  res

    @staticmethod
    def get_array_addr(base_dict: dict, key_str: str):
        if base_dict.get(key_str):
            res = base_dict.get(key_str).get('Value')
            if res == '' or res == None:
                res = "NULL"
        else:
            res = "NULL"
        return  res

    @staticmethod
    def get_nvm_address(base_dict: dict, key_str: str):
        if base_dict.get(key_str):
            res = base_dict.get(key_str).get('Value')
            if res == '' or res == None:
                return "NULL"
        else:
            return "NULL"
        if res != "NULL":
            res = f"(uint8*)({res})"
        return res

    @staticmethod
    def get_int_value(base_dict: dict, key_str: str):
        if base_dict.get(key_str):
            res = base_dict.get(key_str).get('Value')
            if res == '' or res == None:
                res = 0
            if key_str == 'NvMSizeImmediateJobQueue' and res == 0:
                res = 1
        else:
            res = 0
        return res

    @staticmethod
    def get_crc_type(base_dict: dict, key_str: str):
        if base_dict.get(key_str):
            res = base_dict.get(key_str).get('Value')
            if res == "NVM_CRC8":
                res = "NVM_CRC_OFF"
        else:
            res = "NVM_CRC_OFF"
        return res

    @staticmethod
    @pass_context
    def get_bsw_callout_user_code_list(context):
        usercode_list = context["functions"]
        return usercode_list
