#
# Copyright (C) 2015-2022 Lixiang Group Holding Limited
#
"""
EcuM模板内函数定义模块.
定义jinja2模板中所使用的函数.
"""

import copy
from jinja2 import pass_context
from func_module.generator.common.common import FunctionBase


class Function(FunctionBase):
    """
    Class containing all functions. Add new static functions here
    to be included in the jinja2 context
    """
    @staticmethod
    def dict_to_arxml_dict(cur_dict: dict, cur_key_name: str, parent_dict: dict, parent_key_name: str):
        if not isinstance(cur_dict, dict):
            return
        if "Tag" in cur_dict and cur_dict["Tag"] == "CONTAINER-GROUP":
            for container_name, container_dict in list(cur_dict.items()):
                if container_name == "Tag" or container_name == "DefinitionRef":
                    continue
                Function.dict_to_arxml_dict(container_dict, container_name, cur_dict, cur_key_name)
                parent_dict.update({container_name: container_dict})
            del parent_dict[cur_key_name]
        else:
            for container_name, container_dict in list(cur_dict.items()):
                if container_name == "Tag" or container_name == "DefinitionRef":
                    continue
                Function.dict_to_arxml_dict(container_dict, container_name, cur_dict, cur_key_name)

    @staticmethod
    def get_dict_by_ref_path(config_dict: dict, ref_path: str) -> dict:
        bswm_ecuc_arxml_dict = copy.deepcopy(config_dict)
        Function.dict_to_arxml_dict(bswm_ecuc_arxml_dict["BswMGeneral"], "BswMGeneral", bswm_ecuc_arxml_dict, "BswM")
        Function.dict_to_arxml_dict(bswm_ecuc_arxml_dict["BswMConfig"], "BswMConfig", bswm_ecuc_arxml_dict, "BswM")
        ref_path_list = ref_path.split("/")
        bswm_ecuc_arxml_dict = {"BswM": bswm_ecuc_arxml_dict}
        for id, path_key in enumerate(ref_path_list):
            if path_key == "ActiveEcuC" or id == 0:
                continue
            if path_key in bswm_ecuc_arxml_dict:
                bswm_ecuc_arxml_dict = bswm_ecuc_arxml_dict[path_key]
            else:
                return {}
        return bswm_ecuc_arxml_dict

    @staticmethod
    @pass_context
    def get_curr_target(context: dict) -> str:
        target = context['sys_cfg']['target']
        return target

    @staticmethod
    def get_key_name(key_name: str) -> str:
        key_name = key_name.split("/")[-1]
        return key_name

    @staticmethod
    def get_key_name_lower(key_name: str) -> str:
        key_name = key_name.split("/")[-1]
        return key_name.lower()

    @staticmethod
    def float_to_int(number: float) -> int:
        number = int(number)
        return number

    @staticmethod
    def str_to_list(key_value: str) -> list:
        res = []
        if isinstance(key_value, str):
            res.append(key_value)
        else:
            res = key_value
        return res

    @staticmethod
    def get_post_modules(marco_list: list) -> list:
        res = []
        if "BswM" in marco_list:
            res.append("bswm")
        if "CanIf" in marco_list:
            res.append("canif")
        if "CanNm" in marco_list:
            res.append("cannm")
        if "CanTp" in marco_list:
            res.append("cantp")
        if "Com" in marco_list:
            res.append("com")
        if "ComM" in marco_list:
            res.append("comm")
        if "PduR" in marco_list:
            res.append("pdur")
        return res

    @staticmethod
    def check_exist(value: str, parent: dict) -> str:
        if value in parent:
            res = parent[value]
        else:
            res = "NULL"
        return res

    @staticmethod
    def get_defautvalue(config_dict: dict) -> list:
        res = []
        target_dict = config_dict["EcuMConfiguration"]["EcuMCommonConfiguration"]["EcuMDefaultShutdownTarget"]

        target = "ECUM_SHUTDOWN_TARGET_" + str(target_dict["EcuMDefaultShutdownTarget"]["Value"][18:]).upper()
        if target_dict["EcuMDefaultResetModeRef"]["ValueRef"] is not None:
            reset_mode = Function.get_key_name(target_dict["EcuMDefaultResetModeRef"]["ValueRef"])
        else:
            reset_mode = None

        if target_dict["EcuMDefaultSleepModeRef"]["ValueRef"] is not None:
            sleep_mode = Function.get_key_name(target_dict["EcuMDefaultSleepModeRef"]["ValueRef"])
        else:
            sleep_mode = None

        res.append(target)
        if target == "ECUM_SHUTDOWN_TARGET_RESET" and reset_mode is not None:
            res.append("Conf_" + reset_mode)
        elif target == "ECUM_SHUTDOWN_TARGET_SLEEP" and sleep_mode is not None:
            res.append(sleep_mode)
        else:
            res.append("0u")
        return res

    @staticmethod
    def get_mask(config_dict: dict) -> str:
        res = ""
        mask_refs = Function.str_to_list(config_dict["EcuMWakeupSourceMask"]["ValueRef"])

        res = Function.get_key_name(mask_refs[0])
        if len(mask_refs) > 1:
            for i in range(1, len(mask_refs)):
                wakeup_source = Function.get_key_name(mask_refs[i])
                res = res + "|" + wakeup_source
        return res

    @staticmethod
    def get_timeout(timeout: float) -> int:
        res = 0
        if timeout is not None and timeout != "":
            res = int(1000*timeout)
        return res

    @staticmethod
    def get_coreNums(cores: dict) -> int:
        res = 0
        temp = len(cores) - 1
        if (len(cores)-1) == 0:
            res = int(1)
        else:
            res = int(len(cores)-1)
        return res

    @staticmethod
    def get_wakeupchannel(channel_ref: str) -> str:
        res = ""
        if channel_ref == "<empty>" or channel_ref is None:
            res = "ECUM_COMM_CHANNEL_ILL"
        else:
            res = "ComMConf_ComMChannel_" + Function.get_key_name(channel_ref)
        return res

    @staticmethod
    def get_modules(marco_list: list) -> list:
        res = []
        if "LinIf" in marco_list:
            res.append("linif")
        if "LinTp" in marco_list:
            res.append("lintp")
        if "Nm" in marco_list:
            res.append("nm")
        if "CanSM" in marco_list:
            res.append("cansm")
        if "LinSM" in marco_list:
            res.append("linsm")
        if "EthSM" in marco_list:
            res.append("ethsm")
        if "LinNm" in marco_list:
            res.append("linnm")
        if "TcpIp" in marco_list:
            res.append("tcpip")
        if "SoAd" in marco_list:
            res.append("soad")
        if "DoIP" in marco_list:
            res.append("doip")
        if "Eep" in marco_list:
            res.append("eep")
        if "WdgM" in marco_list:
            res.append("wdgm")
        if "EthTrcv" in marco_list:
            res.append("ethtrcv")
        if "EthIf" in marco_list:
            res.append("ethif")
        if "EthTSyn" in marco_list:
            res.append("ethtsyn")
        if "CanTSyn" in marco_list:
            res.append("cantsyn")
        if "Dcm" in marco_list:
            res.append("dcm")
        if "Dem" in marco_list:
            res.append("dem")
        if "StbM" in marco_list:
            res.append("time_manager")
        if "Dlt" in marco_list:
            res.append("dlt")
        return res

    @staticmethod
    def get_module_marcos(marco_list: list) -> list:
        res = []
        if "Rte" in marco_list:
            res.append("ECUM_USE_RTE")
        if "SchM" in marco_list:
            res.append("ECUM_USE_SCHM")
        if "ComM" in marco_list:
            res.append("ECUM_USE_COMM")
        if "Dem" in marco_list:
            res.append("ECUM_USE_DEM")
        if "BswM" in marco_list:
            res.append("ECUM_USE_BSWM")
        if "Eep" in marco_list:
            res.append("ECUM_USE_EEP")
        if "Ea" in marco_list:
            res.append("ECUM_USE_EA")
        if "Dlt" in marco_list:
            res.append("ECUM_USE_DLT")
        return res

    @staticmethod
    def gen_switch_macro(key_value: str) -> str:
        if key_value == False:
            res = "STD_OFF"
        else:
            res = "STD_ON"
        return res

    @staticmethod
    def get_ecum_mode(key_value: str) -> str:
        if key_value == True:
            res = "STD_OFF"
        else:
            res = "STD_ON"
        return res

    @staticmethod
    def get_module_name(module: dict) -> str:
        ModuleRef = module["EcuMModuleRef"]["ValueRef"]
        if ModuleRef != "<empty>" and ModuleRef is not None and ModuleRef != '':
            res = Function.get_key_name(module["EcuMModuleRef"]["ValueRef"])
        else:
            res = module["EcuMMoudleHeader"]["Value"][:-2]
        return res.lower()

    @staticmethod
    def get_init_modules(config_dict: dict) -> list:
        res = []

        if "EcuMDriverInitListZero" in config_dict["EcuMConfiguration"]["EcuMCommonConfiguration"] and \
            "EcuMDriverInitItems" in config_dict["EcuMConfiguration"]["EcuMCommonConfiguration"]["EcuMDriverInitListZero"]:
            list_zero = config_dict["EcuMConfiguration"]["EcuMCommonConfiguration"]["EcuMDriverInitListZero"][
                "EcuMDriverInitItems"]
            for module_name, module_value in list(list_zero.items()):
                if module_name != "Tag":
                    module = Function.get_module_name(module_value)
                    if module not in res:
                        res.append(module)

        if "EcuMDriverInitListOne" in config_dict["EcuMConfiguration"]["EcuMCommonConfiguration"] and \
            "EcuMDriverInitItems" in config_dict["EcuMConfiguration"]["EcuMCommonConfiguration"]["EcuMDriverInitListOne"]:
            list_one = config_dict["EcuMConfiguration"]["EcuMCommonConfiguration"]["EcuMDriverInitListOne"][
                "EcuMDriverInitItems"]
            for module_name, module_value in list(list_one.items()):
                if module_name != "Tag":
                    module = Function.get_module_name(module_value)
                    if module not in res:
                        res.append(module)

        list_bswm = config_dict["EcuMConfiguration"]["EcuMFlexConfiguration"]
        for element_name, element_value in list(list_bswm.items()):
            if (element_name != "DefinitionRef") and ("DefinitionRef" in element_value) and \
                (Function.get_key_name(element_value["DefinitionRef"]) == "EcuMDriverInitListBswM") and \
                "EcuMDriverInitItems" in element_value:
                for module_name, module_value in list(element_value["EcuMDriverInitItems"].items()):
                    if module_name != "Tag":
                        module = Function.get_module_name(module_value)
                        if module not in res:
                            res.append(module)

        return res

    @staticmethod
    def gen_init_func(module_name: str, module_value: dict) -> str:
        short_name = module_name
        parameter = module_value["EcuMModuleParameter"]["Value"]
        module_ref = module_value["EcuMModuleRef"]["ValueRef"]
        # service = module_value["EcuMModuleService"]["Value"]
        # service_list = ["Init", "PreInit", "Start"]
        postbuild_module = ["BswM", "CanIf", "CanNm", "CanTp", "Com", "ComM", "PduR", "CanTrcv"]

        # Standard autosar modules
        if module_ref != "<empty>" and module_ref is not None and module_ref != '':
            module = Function.get_key_name_lower(module_ref)

            if module == "can":
                res = "can_init(&can_config);"
                return res
            elif module == "lin":
                res = "lin_init(&lin_global_config);"
                return res

            if parameter == "NULL_PTR":
                res = short_name + "(NULL_PTR);"
            elif parameter == "VOID":
                res = short_name + "();"
            else:
                if module in postbuild_module:
                    res = short_name + "(config_ptr->post_build_config->" + module.lower() + "_config_ptr);"
                else:
                    res = short_name + "(config_ptr->" + module + "_config_ptr);"
        # Other modules
        else:
            res = module_value["EcuMAdditionalInitCode"]["Value"]

        return res

    func_name_list = []
    get_initfunc_name_id = -1
    @staticmethod
    def get_initfunc_name(func_name, id) -> str:
        if Function.get_initfunc_name_id != id:
            Function.func_name_list = []
            Function.get_initfunc_name_id = id
        func_name = func_name.split('(')[0]
        inum = 0
        while func_name in Function.func_name_list:
            if inum == 0:
                func_name = func_name + "_" + str(inum)
            else:
                func_name = func_name[:-2] + "_" + str(inum)
            inum = inum + 1
        Function.func_name_list.append(func_name) 
        return func_name

    @staticmethod
    @pass_context
    def get_bsw_callout_user_code_list(context):
        usercode_list = context["functions"]
        return usercode_list
    
    @staticmethod
    def get_VldWakeupSources(sources):
        wksourcelist = ["ECUM_WKSOURCE_POWER" , "ECUM_WKSOURCE_RESET" , "ECUM_WKSOURCE_INTERNAL_RESET" , "ECUM_WKSOURCE_INTERNAL_WDG" , "ECUM_WKSOURCE_EXTERNAL_WDG"]
        VldWakeupSources = ""
        for source_name, source_value in sources.items():
            if source_name in wksourcelist:
                if VldWakeupSources == "":
                    VldWakeupSources = source_name
                else:
                    VldWakeupSources = VldWakeupSources + " | " + source_name
        return VldWakeupSources

    @staticmethod
    def get_VldWakeupSourceNum(sources):
        wksourcelist = ["ECUM_WKSOURCE_POWER" , "ECUM_WKSOURCE_RESET" , "ECUM_WKSOURCE_INTERNAL_RESET" , "ECUM_WKSOURCE_INTERNAL_WDG" , "ECUM_WKSOURCE_EXTERNAL_WDG"]
        num = 0
        for source_name, source_value in sources.items():
            if source_name in wksourcelist:
                continue
            else:
                num += 1
        return num - 1
    
    @staticmethod
    def get_index(str1, num):
        str1 = str1 + str(num)
        return str1

    @staticmethod
    def get_resetReasonRef(source_name,refs):
        res = ""
        if  isinstance(refs, str):
            res = '    { ' + source_name + ' , ' + refs.split('/')[-1] + ' },'
        else:
            for ref in refs:
                res += '    { ' + source_name + ' , ' + ref.split('/')[-1] + ' },\n'
            if(res.endswith('\n')):
                res = res[:-1]
        return res
    
    @staticmethod
    def get_ecum_McuResetRefNum(sources):
        num = 0
        if sources != "NULL":
            for source_name, source_value in sources.items():
                if source_name != "Tag" :
                    if("EcuMResetReasonRef" in source_value) :
                        if("ValueRef" in source_value["EcuMResetReasonRef"]) :
                            refs = source_value["EcuMResetReasonRef"]["ValueRef"] 
                            if refs != None:
      
                                if  isinstance(refs, str):
                                    num = num + 1
                                else:
                                    for ref in refs:
                                        num = num + 1
        return num
    

    last_initfunc = ''
    @staticmethod
    def Getlast_initfunc(initfunc) -> str:
        func = Function.last_initfunc
        Function.last_initfunc = initfunc
        return func


