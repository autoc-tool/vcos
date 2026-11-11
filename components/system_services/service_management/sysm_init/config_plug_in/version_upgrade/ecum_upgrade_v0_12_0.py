from basic_func_module.autosar_utils.utils import (get_definition_ref, set_value, get_value, get_short_name, set_short_name, set_value_ref)

from basic_func_module.autosar_utils.autosar_util import AutosarUtil
from func_module.logger import logger
import math


def update_short_name(key_name: str) -> str:
    key_name = key_name.split("(")[0]
    return key_name


class EcuMUpgradeV0120():

    def __init__(self):
        pass

    def upgrade_module(self, bsw_ecuc_autosar_class, autosar_ib_class, mod_autosar_dict, sys_info_dict) -> bool:
        ecum_autosar = bsw_ecuc_autosar_class["EcuM"]
        ecum_flex = ecum_autosar.find("/ActiveEcuC/EcuM/EcuMConfiguration/EcuMFlexConfiguration")

        # EcuMDriverInitListBswM
        if ecum_flex.SUB_CONTAINERS is not None:
            # print("ecum_flex is not None")
            for element in ecum_flex.SUB_CONTAINERS.ECUC_CONTAINER_VALUE:
                if get_definition_ref(element).split("/")[-1] == "EcuMDriverInitListBswM":
                    list_name = get_short_name(element)
                    if element.SUB_CONTAINERS is not None:
                        # print("list_bswm is not None")
                        for module in element.SUB_CONTAINERS.ECUC_CONTAINER_VALUE:
                            module_name = get_short_name(module)
                            short_name = update_short_name(module_name)
                            set_short_name(module, short_name)

                            if short_name == "Csm_Init":
                                ref_name = module.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE[0]
                                set_value(ref_name, "NULL_PTR")
        return True
