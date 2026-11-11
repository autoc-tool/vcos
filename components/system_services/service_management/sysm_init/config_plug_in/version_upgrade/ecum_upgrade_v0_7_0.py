from basic_func_module.autosar_utils.utils import (get_definition_ref, set_value, get_value, get_short_name, set_short_name)
from basic_func_module.autosar_utils.autosar_util import AutosarUtil
from func_module.logger import logger
import math


def update_short_name(key_name: str) -> str:
    key_name = key_name.split("(")[0]
    return key_name


class EcuMUpgradeV070():

    def __init__(self):
        pass

    def upgrade_module(self, bsw_ecuc_autosar_class, autosar_ib_class, mod_autosar_dict, sys_info_dict) -> bool:
        ecum_autosar = bsw_ecuc_autosar_class["EcuM"]

        list_zero = ecum_autosar.find(
            "/ActiveEcuC/EcuM/EcuMConfiguration/EcuMCommonConfiguration/EcuMDriverInitListZero")
        list_one = ecum_autosar.find("/ActiveEcuC/EcuM/EcuMConfiguration/EcuMCommonConfiguration/EcuMDriverInitListOne")

        ecum_common = ecum_autosar.find("/ActiveEcuC/EcuM/EcuMConfiguration/EcuMCommonConfiguration")
        ecum_flex = ecum_autosar.find("/ActiveEcuC/EcuM/EcuMConfiguration/EcuMFlexConfiguration")

        # EcuMDriverInitListZero
        if list_zero.SUB_CONTAINERS is not None:
            # print("list_zero is not None")
            for module in list_zero.SUB_CONTAINERS.ECUC_CONTAINER_VALUE:
                module_name = get_short_name(module)
                short_name = update_short_name(module_name)
                set_short_name(module, short_name)
                # print(get_short_name(module))

        # EcuMDriverInitListOne
        if list_one.SUB_CONTAINERS is not None:
            # print("list_one is not None")
            for module in list_one.SUB_CONTAINERS.ECUC_CONTAINER_VALUE:
                module_name = get_short_name(module)
                short_name = update_short_name(module_name)
                set_short_name(module, short_name)
                # print(get_short_name(module))

        # EcuMWakeupSource
        if ecum_common.SUB_CONTAINERS is not None:
            # print("ecum_common is not None")
            for element in ecum_common.SUB_CONTAINERS.ECUC_CONTAINER_VALUE:
                if get_definition_ref(element).split("/")[-1] == "EcuMWakeupSource":
                    for source_id in element.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE:
                        if get_definition_ref(source_id).split("/")[-1] == "EcuMWakeupSourceId":
                            id = get_value(source_id)
                            # print(id)
                            new_id = math.log2(int(id))
                            set_value(source_id, str(int(new_id)))
                            # print(get_value(source_id))

        # EcuMDriverInitListBswM
        if ecum_flex.SUB_CONTAINERS is not None:
            # print("ecum_flex is not None")
            core_id = 1
            for element in ecum_flex.SUB_CONTAINERS.ECUC_CONTAINER_VALUE:
                if get_definition_ref(element).split("/")[-1] == "EcuMDriverInitListBswM":
                    list_name = get_short_name(element)
                    if element.SUB_CONTAINERS is not None:
                        # print("list_bswm is not None")
                        for module in element.SUB_CONTAINERS.ECUC_CONTAINER_VALUE:
                            module_name = get_short_name(module)
                            short_name = update_short_name(module_name)
                            set_short_name(module, short_name)
                            # print(get_short_name(module))

                            if short_name == "WdgM_Init":
                                ref_name = module.REFERENCE_VALUES.ECUC_REFERENCE_VALUE[0]
                                set_value_ref(ref_name, "<empty>")
                                # print(get_value_ref(ref_name))

                                # 新增参数并填值
                                initcode_def = mod_autosar_dict["EcuM"].find(
                                    "/AUTOSAR/EcucDefs/EcuM/EcuMConfiguration/EcuMFlexConfiguration/EcuMDriverInitListBswM/EcuMDriverInitItem/EcuMAdditionalInitCode"
                                )
                                header_def = mod_autosar_dict["EcuM"].find(
                                    "/AUTOSAR/EcucDefs/EcuM/EcuMConfiguration/EcuMFlexConfiguration/EcuMDriverInitListBswM/EcuMDriverInitItem/EcuMMoudleHeader"
                                )
                                ecuc_ref = "/ActiveEcuC/EcuM/EcuMConfiguration/EcuMFlexConfiguration/" + str(
                                    list_name) + "/WdgM_Init"

                                module_ecuc = ecum_autosar.find(ecuc_ref)
                                config_name = "WdgM_Init(&WdgM_core" + str(core_id) + "_config);"
                                core_id += 1

                                ret1 = AutosarUtil.add_ecuc_parameter(module_ecuc, initcode_def, config_name)
                                if ret1 == False:
                                    logger.error("Add InitCode fail")
                                ret2 = AutosarUtil.add_ecuc_parameter(module_ecuc, header_def, "WdgM.h")
                                if ret2 == False:
                                    logger.error("Add Header fail")

        # dict_config = AutosarUtil.to_bsw_dict(ecum_object)
        # with open('060.json', 'w') as f:
        #     json.dump(dict_config, f, indent=4)

        return True
