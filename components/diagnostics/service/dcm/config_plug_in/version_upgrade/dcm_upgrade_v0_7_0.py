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

from basic_func_module.autosar_utils.utils import get_definition_ref, set_value, get_value, get_short_name
from basic_func_module.autosar_utils.autosar_util import AutosarUtil
from func_module.health_monitor.logger import logger


class DcmUpgradeV070():

    def __init__(self):
        pass

    def upgrade_module(self, bsw_ecuc_autosar_class, autosar_ib_class, mod_autosar_dict, sys_info_dict) -> bool:
        print("upgrade_DCMmodule")
        srcdata = bsw_ecuc_autosar_class["Dcm"]

        dcmgeneral = srcdata.find("/ActiveEcuC/Dcm/DcmGeneral").PARAMETER_VALUES
        headerlist = dcmgeneral.ECUC_TEXTUAL_PARAM_VALUE
        isfound = False
        for headerfile in headerlist:
            if get_definition_ref(headerfile) == "/AUTOSAR/EcucDefs/Dcm/DcmGeneral/DcmHeaderFileInclusion":
                isfound = True
                continue
            else:
                pass
        if isfound == False:
            #create DcmHeaderFileInclusion
            container_def = mod_autosar_dict["Dcm"].find("/AUTOSAR/EcucDefs/Dcm/DcmGeneral/DcmHeaderFileInclusion")
            container_ecuc = srcdata.find("/ActiveEcuC/Dcm/DcmGeneral")
            # AutosarUtils.set_autosar_ecuc_pararm_value(container_ecuc, container_def, "Dcm_Callout_Stubs.h")
            ret = AutosarUtil.add_ecuc_parameter(container_ecuc, container_def, "Dcm_Callout_Stubs.h")
            if ret == False:
                logger.error("add ecuc parameter fail")
                pass

        return True
