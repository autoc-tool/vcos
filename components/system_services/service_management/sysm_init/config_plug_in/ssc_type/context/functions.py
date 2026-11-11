#
# Copyright (C) 2015-2022 Lixiang Group Holding Limited
#
"""
EcuM 模板内函数定义模块.
定义jinja2模板中所使用的函数.
"""

import uuid


class Function:
    """
    Class containing all functions. Add new static functions here
    to be included in the jinja2 context
    """
    UuidList = [
    'f47ac10b-58cc-4372-a567-0e02b2c3d479',
    '623e3b8d-9eda-435a-9f57-ade001396cb3',
    '3b01b77f-9507-47ee-9370-4ef009d46e8c',
    'f17f5a8e-90dc-4d2d-ae80-6d8519a2f9c2',
    '083f7580-a9ae-4eb1-941d-9b815d521d90',
    'ae3d1a3f-ddd7-420d-b6c6-afedb6997520',
    '4eb9a645-7da7-4f67-83ea-b7fdf758ccef',
    '309e393e-9d10-4f4a-9aeb-5b90aefc67fd',
    'b2f5ff47-0820-4c60-b9f9-dfa4a8298b6b',
    '4b9e7c8e-58e2-4689-8a59-a5324723b96e',
    'b637bb47-b52a-4be4-aa0d-7c08e5d02db4',
    'a9f64647-d744-4fc6-aae2-c5df6d18b7df',
    '5c0745e3-79d2-4f37-9f54-2a5eecc045cb',
    '921f2490-527f-41af-9b41-d1cf91db2fd0',
    'd15b3476-52ed-4ac5-9fe1-a9d25381e6e9',
    '77b1c8d3-335e-4549-88d0-3a9e34503518',
    'a39125a7-a9e7-4bfc-94f4-df6fa85b0f78',
    '1ff48102-7c43-4e33-9c0a-fdb2196ba73b',
    'c2eded3b-1771-48ef-ad9e-caa8016c1b87',
    'e5b13267-fb5d-4b52-b1ea-1a8bd6203001',
    '8a318f70-5a6e-4e58-b4f7-c38119ed374a',
    '55dd9c82-318e-4d0d-b4a4-0ad08dedd32a',
    '56457d60-74d8-4c8c-9b2b-4d7bea9edc70',
    '3c476546-647b-4bc2-aacd-ebe10d5a4a08',
    'fd0b3289-c3f5-4b27-a2d5-45d058a1b8c7',
    '9c431a82-d8cc-4acf-b6c7-fa0cdd6d424a',
    '010bdf53-5d88-4bee-b117-c1a171a3e3bb',
    'ee96c2f2-aeda-477f-b1e6-eb9f44b40ed5',
    '7f04d9f9-ca9d-489e-9d19-ad9600505d53',
    '621dcc47-0aba-4ad5-ba34-4a4dca7d5d7c',
    '8b069e63-0d17-4b06-bce4-fc6caf95ff4d',
    'c578353e-5686-41a7-938c-d5c3c60dabe3',
    '45e5b5d7-fd69-4f01-963f-932b45811e99',
    '2b2be0f7-ac44-48b1-8866-cafaa667c1db',
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
    'd7fbd1a3-c53d-4738-bede-622ae9a4e6f6'
    ]

    UuidNum = 0

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
    def get_key_name(key_name: str) -> str:
        key_name = key_name.split("_")[-1]
        return key_name
