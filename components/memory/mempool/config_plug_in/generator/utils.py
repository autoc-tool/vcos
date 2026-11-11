############################################################################
#  Copyright (c) 2025 Li Auto Inc. and its affiliates
#  Licensed under the Apache License, Version 2.0(the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at

#      http://www.apache.org/licenses/LICENSE-2.0

#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
############################################################################

import functools
from collections import defaultdict

def get_last_item(x):
    """string.split('/') ,return the last part; if string=<empty>,return '' """
    if x is None or x == "<empty>":
        return ""
    else:
        return x.split('/')[-1]
def get_coreid_from_appnam(appName, ecuc_hardware_dict, config_dict):
    """ get core number from appname """
    coreid = 0
    OsApplicationCoreRef = ""
    #get OsApplicationCoreRef
    os_dict = config_dict["bsw"]["os"]["ArPackages"]["ActiveEcuC"]["Os"]
    for k, v in os_dict.items():
        if "DefinitionRef" in v and "/Os/OsApplication" in v["DefinitionRef"] and k == appName:
            OsApplicationCoreRef = get_last_item(v["OsApplicationCoreRef"]["ValueRef"])
    #ger coreid
    for k, v in ecuc_hardware_dict.items():
        if k == OsApplicationCoreRef:
            coreid = v["EcucCoreId"]["Value"]
    return coreid

def get_os_mempool_type_coreid(ui_type, app_name_list, mem_pool_value, config_dict):
    mempooltype = "OS_MEMPOOLTYPE_GLOBAL"
    type_coreid = {}
    coreid = 0
    ecuc_hardware_dict = config_dict.get("bsw",{}).get("ecuc",{}).get("ArPackages",{}).get("ActiveEcuC",{}).get("EcuC",{}).get("EcucHardware",{})
    coreid_set = set()
    for app_name in app_name_list:
        coreid_set.add(get_coreid_from_appnam(app_name, ecuc_hardware_dict, config_dict))

    if (ui_type == "GLOBAL"):
        mempooltype =  "OS_MEMPOOLTYPE_GLOBAL"
    elif (ui_type == "CORE"):
        if len(app_name_list) != 1:
            raise("[mempool] mempool模块配置core级别只能关联1个application，请检查配置")
        coreid = coreid_set.pop()
        mempooltype =  f"OS_MEMPOOLTYPE_CORE{coreid}"
    elif (ui_type == "AUTOMATIC"):
        if len(app_name_list) == 1:
            mempooltype =  "OS_MEMPOOLTYPE_PRIVATE"
            coreid = coreid_set.pop()
        elif (len(app_name_list) > 1):
            if (len(coreid_set) > 1):
                mempooltype =  "OS_MEMPOOLTYPE_GLOBAL"
        else:
            pass
    else:
        pass

    type_coreid["type"] = mempooltype
    type_coreid["coreid"] = coreid

    return type_coreid

def get_os_mempool_config_dict(os_mempool_ui_dict: dict, os_mempool_module_cfg:dict, config_dict:dict):
    os_mempool_cfg = {}
    for mempool_name, mempool_value in os_mempool_ui_dict.items():
        if isinstance(mempool_value, dict) and "/AUTOSAR/EcucDefs/MemPool/MemPool" == mempool_value["DefinitionRef"]:
            os_mempool_cfg[mempool_name] = {}
            os_mempool_cfg[mempool_name]["MemBlockCount"] = mempool_value["MemBlockCount"]["Value"]
            os_mempool_cfg[mempool_name]["MemBlockSize"] = mempool_value["MemBlockSize"]["Value"]
            os_mempool_cfg[mempool_name]["MemAlignment"] = mempool_value["MemAlignment"]["Value"]

            if (os_mempool_cfg[mempool_name]["MemAlignment"] < 4):
                os_mempool_cfg[mempool_name]["MemAlignment"] = 4

            access_appname_set = set()
            for module_name in os_mempool_module_cfg:
                if mempool_name in os_mempool_module_cfg[module_name]["mempool_name_list"]:
                    access_appname_set.update(set(os_mempool_module_cfg[module_name]["app_name_list"]))

            os_mempool_cfg[mempool_name]["app_name_list"] = list(access_appname_set)

            type_coreid = get_os_mempool_type_coreid(mempool_value["MemType"]["Value"],
                                                    os_mempool_cfg[mempool_name]["app_name_list"],
                                                    mempool_value,
                                                    config_dict)

            os_mempool_cfg[mempool_name]["MemType"] = type_coreid["type"]
            os_mempool_cfg[mempool_name]["CoreID"] = type_coreid["coreid"]
            os_mempool_cfg[mempool_name]["name"] = mempool_name
    return os_mempool_cfg

def get_os_mempool_type_config_dict(os_mempool_pool_cfg: dict):
    os_mempool_type_cfg = {}
    os_mempool_type_cfg["private"] = {}
    os_mempool_type_cfg["global"] = []

    os_private_mempool_cfg_list = []
    os_global_mempool_cfg_list = []

    for name in os_mempool_pool_cfg:
        mempool_cfg = os_mempool_pool_cfg[name]
        if (mempool_cfg["MemType"] == "OS_MEMPOOLTYPE_PRIVATE" or "OS_MEMPOOLTYPE_CORE" in mempool_cfg["MemType"]):
            os_private_mempool_cfg_list.append(mempool_cfg)
        elif (mempool_cfg["MemType"] == "OS_MEMPOOLTYPE_GLOBAL"):
            os_global_mempool_cfg_list.append(mempool_cfg)
        else:
            pass

    for mempool_cfg in os_private_mempool_cfg_list:
        app_name = mempool_cfg["app_name_list"][0]

        if app_name not in os_mempool_type_cfg["private"]:
            os_mempool_type_cfg["private"][app_name] = []

        os_mempool_type_cfg["private"][app_name].append(mempool_cfg)



    for mempool_cfg in os_global_mempool_cfg_list:
        os_mempool_type_cfg["global"].append(mempool_cfg)

    return os_mempool_type_cfg


def deal_module_cfg(os_mempool_module_cfg):
    # 初始化一个 defaultdict 来按 module_name 聚合
    grouped = {}

    for item in os_mempool_module_cfg.values():
        mod = item["module_name"]
        if mod not in grouped:
            grouped[mod] = {
                "module_name": mod,
                "mempool_name_list": [],
                "pool_count": 0,
                "mempool_cfg_list": []
            }

        grouped[mod]["mempool_name_list"].extend(item.get("mempool_name_list", []))
        grouped[mod]["pool_count"] += item.get("pool_count", 0)
        grouped[mod]["mempool_cfg_list"].extend(item.get("mempool_cfg_list", []))

    for val in grouped.values():
        val["mempool_name_list"] = list(sorted(set(val["mempool_name_list"])))
    return grouped

def get_os_mempool_all_config_dict(os_mempool_ui_dict: dict, config_dict:dict):
    os_mempool_all_cfg = {}
    os_mempool_module_cfg = {}
    os_mempool_type_cfg = {}
    os_mempool_pool_cfg = {}

    for name, value in os_mempool_ui_dict.items():
        if isinstance(value, dict) and "/AUTOSAR/EcucDefs/MemPool/MemModule" in value["DefinitionRef"]:

            module_name = value["BswModuleRef"]["ValueRef"]
            module_name = module_name.split("/")[-1]

            app_name_list = []
            if isinstance(value["OsApplicationRef"]["ValueRef"], list):
                app_name_list.extend(value["OsApplicationRef"]["ValueRef"])
            elif isinstance(value["OsApplicationRef"]["ValueRef"], str):
                app_name_list.append(value["OsApplicationRef"]["ValueRef"])
            for index in range(len(app_name_list)):
                app_name_list[index] = app_name_list[index].split("/")[-1]

            os_mempool_name_list = []
            if isinstance(value["MemPoolRef"]["ValueRef"], list):
                os_mempool_name_list.extend(value["MemPoolRef"]["ValueRef"])
            elif isinstance(value["MemPoolRef"]["ValueRef"], str):
                os_mempool_name_list.append(value["MemPoolRef"]["ValueRef"])
            for index in range(len(os_mempool_name_list)):
                os_mempool_name_list[index] = os_mempool_name_list[index].split("/")[-1]

            os_mempool_module_cfg[name] = {}
            os_mempool_module_cfg[name]["module_name"] = module_name
            os_mempool_module_cfg[name]["app_name_list"] = app_name_list
            os_mempool_module_cfg[name]["mempool_name_list"] = os_mempool_name_list
            os_mempool_module_cfg[name]["pool_count"] = len(os_mempool_name_list)

    os_mempool_pool_cfg = get_os_mempool_config_dict(os_mempool_ui_dict, os_mempool_module_cfg, config_dict)

    for module_name, config in os_mempool_module_cfg.items():
        for mempool_name in config["mempool_name_list"]:
            if "module_list" not in os_mempool_pool_cfg[mempool_name]:
                os_mempool_pool_cfg[mempool_name]["module_list"] = []
            os_mempool_pool_cfg[mempool_name]["module_list"].append(module_name)

    for module_name in os_mempool_module_cfg:
        mempool_cfg_list = []
        for mempool_name in os_mempool_module_cfg[module_name]["mempool_name_list"]:
            mempool_cfg_list.append(os_mempool_pool_cfg[mempool_name])

        mempool_cfg_list = sorted(mempool_cfg_list, key=lambda x: (len(x['module_list']), x["MemBlockSize"]))

        os_mempool_module_cfg[module_name]["mempool_cfg_list"] = mempool_cfg_list

    os_mempool_type_cfg = get_os_mempool_type_config_dict(os_mempool_pool_cfg)

    os_mempool_all_cfg["module_cfg"] = deal_module_cfg(os_mempool_module_cfg)
    os_mempool_all_cfg["type_cfg"] = os_mempool_type_cfg
    os_mempool_all_cfg["pool_cfg"] = os_mempool_pool_cfg
    coreid_set = set()
    for pool_name in os_mempool_pool_cfg:
        coreid_set.add(os_mempool_pool_cfg[pool_name]["CoreID"])
    os_mempool_all_cfg["coreid_set"] = list(coreid_set)
    return os_mempool_all_cfg