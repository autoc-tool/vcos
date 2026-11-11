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
import uuid


class Function:
    UuidList = [
    '7415ed9d-2d93-4fae-8e14-a8e2d1a72c8b',
    '6ee6d384-ecf9-413a-8c9b-712e5a2a5e01',
    'd25656e9-f9dc-4dae-b6d3-b2098a346f2a',
    '4fe8a8f0-2457-4b88-bfae-84f14f2aee67',
    '63d45691-d217-4e7d-8d26-db8e9affa1f0',
    '34e6d360-2775-481e-99b4-b80d9b180a88',
    'bfe07bdc-ee02-4fa5-8e2f-63c6a73a7d64',
    'bf18a9d7-ba64-4028-96dc-0b1ded083617',
    '272411d7-f7f9-4d7d-9cda-80dc3c091beb',
    'e4f1c370-e531-4050-952e-a4e6096a7ed7',
    '8963b618-2f7a-404b-844f-2ee228d7cda7',
    'cddd0974-f0c8-4f1b-88d9-49fb98c8c90a',
    '536ac268-0454-41ed-b85d-b5fe9ecfcdc6',
    'ae1da156-f1b6-499e-92d7-6f4076329397',
    '32a96542-4f4a-48f4-9153-4739ff1ee987',
    'a10fdfda-032f-4e3f-ba4b-4fd59c9a00ad',
    '78fd680f-a8f1-4bb8-bc5f-7e5a9a842a54',
    'a0ca23df-5167-4d19-b1cc-7bfcedae076a',
    '3671f162-67cc-4b47-b2e5-49e3d7b7392d',
    '5d6325e8-df67-4d88-959d-f831d9c6c50a',
    '1db59fd1-aea6-4bbb-8afa-6bd8c7330537',
    '47d702a4-0817-465c-9f63-40a1e8106228',
    'd3f7c24e-4fe4-403a-88b1-742b0f1a0a12',
    '84c52be1-5bf4-4c58-819d-5c3d25f422e9',
    '652b6c04-94c9-4f09-b9cb-eb576d958919',
    '03083f77-7f45-496a-ae95-223d2b55b6ba',
    'e02f6bf3-726c-4a2c-8828-66a52bf5b814',
    '2565148f-67a2-4e02-ba58-57f4226b7045',
    'e6e6352e-6b5a-4d71-a3cd-c4e63f891d69',
    '8d6e3c74-c9f2-44c7-b781-0554586e9c14',
    '0c77833e-8193-40d6-abf0-32a9e3f43018',
    'de1140f3-58f6-4b83-9cf6-5f956c997f99',
    '5c918c60-4634-4a31-ab54-446c6a490ebe',
    '891c6569-9c60-4992-b5bf-45b9429b6eee',
    'db4ca59f-d51a-4995-9ee5-c895a4f48ad8',
    '7f355520-f82a-44cf-bf82-a71f31045acd',
    '6a6ad003-9eb5-4ef4-8943-7742bdae097f',
    '6521d5b7-8899-4f4c-a48b-f249dbe4d67e',
    'f2e2a3f3-bf1e-4dcb-909f-456b2c0877e7',
    '7f86c12a-d1e2-4bcf-b166-8c8e6265ff47',
    'f97c0fa8-537e-4e31-bb75-5d5c5a82e5b1',
    '03028cc9-2df9-49d3-a9bf-50732980a641',
    '4b6a7762-5a14-41eb-809d-5b81bf5b3282',
    '04f5ba58-5b5c-4c8b-92d5-92a348a0c2d7',
    '4294d13c-b68e-432d-8846-93fed8a93eb2',
    '6b40450f-39c9-4478-8d7d-e03d29efb3b8',
    '67adf8a2-950b-4d64-a472-5b656f832d0b',
    'ffe54615-61c3-4a0b-bbcd-e011bf2a9561',
    '442f081e-3bac-4c7d-a6b9-bd5f2de3a822',
    'e16ef254-37c7-4dda-a8d6-f0453a9a1eea'
    ]

    UuidNum = 0

    @staticmethod
    @pass_context
    def test(context):
        return 0
    
    @staticmethod
    def gen_uuid() -> str:
        temp = Function.UuidList[Function.UuidNum]
        if(Function.UuidNum < 49):
            Function.UuidNum = Function.UuidNum + 1
        else:
            Function.UuidNum = 0
        return str(temp)
    
    @staticmethod
    def reset_UuidNum():
        Function.UuidNum = 0
        return

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

        return int(maxlength) + 16

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
        return res

    @staticmethod
    def get_int_value(base_dict: dict, key_str: str):
        if base_dict.get(key_str):
            res = base_dict.get(key_str).get('Value')
            if res == '' or res == None:
                res = 0
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
