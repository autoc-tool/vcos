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
from basic_func_module.autosar_utils.autosar_util import AutosarUtil

class PduRRule():

    def __init__(self) -> None:
        self.log = ""
        self.ecu_cfg = None

    def validate_pdur_pdursrcpduref(self, pdur_cfg: dict) -> bool:
        valid = True

        for name, pdursrctpdu in pdur_cfg["PduRRoutingPaths"].items():
            if "DefinitionRef" not in pdursrctpdu or pdursrctpdu[
              "DefinitionRef"] != "/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRSrcPdu":
                continue
            if not AutosarUtil.get_by_path_from_dict(pdursrctpdu, "/PduRSrcPduRef/ValueRef"):
                continue
            pdursrcpduref_path = pdursrctpdu["PduRSrcPduRef"]["ValueRef"]
            pdursrcpduref = AutosarUtil.get_by_path_from_dict(self.ecu_cfg, pdursrcpduref_path)
            if not pdursrcpduref:
                valid = False and valid
                self.log += "the configuration {} referenced by /ActiveEcuC/PduR/PduRRoutingPaths/PduRSrcPdu/{}/PduRSrcPduRef does not exist!\n".format(
                  pdursrcpduref_path, name)
                return valid
            else:
                # TODO: check the reference config
                pass

        return valid

    def validate_pdur_pdurdestpduref(self, pdur_cfg: dict) -> bool:
        valid = True

        for name, pdurdestpdu in pdur_cfg["PduRRoutingPaths"].items():
            if "DefinitionRef" not in pdurdestpdu or pdurdestpdu[
              "DefinitionRef"] != "/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRDestPdu":
                continue
            if not AutosarUtil.get_by_path_from_dict(pdurdestpdu, "/PduRDestPduRef/ValueRef"):
                continue
            pdurdestpduref_path = pdurdestpdu["PduRDestPduRef"]["ValueRef"]
            pdurdestpduref = AutosarUtil.get_by_path_from_dict(self.ecu_cfg, pdurdestpduref_path)
            if not pdurdestpduref:
                valid = False and valid
                self.log += "the configuration {} referenced by /ActiveEcuC/PduR/PduRRoutingPaths/PduRDestPdu/{}/PduRDestPduRef does not exist!\n".format(
                  pdurdestpduref_path, name)
                return valid
            else:
                # TODO: check the reference config
                pass

        return valid

    def validate_pdur_pdurroutingpath_pdursrcpdurref(self, pdur_cfg: dict) -> bool:
        valid = True

        for name, pdurroutingpath in pdur_cfg["PduRRoutingPaths"].items():
            if "DefinitionRef" not in pdurroutingpath or pdurroutingpath[
              "DefinitionRef"] != "/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRRoutingPath":
                continue
            if not AutosarUtil.get_by_path_from_dict(pdurroutingpath, "/PduRSrcPduRRef/ValueRef"):
                continue
            pdursrcpdurref_path = pdurroutingpath["PduRSrcPduRRef"]["ValueRef"]
            pdursrcpdurref = AutosarUtil.get_by_path_from_dict(self.ecu_cfg, pdursrcpdurref_path)
            if not pdursrcpdurref:
                valid = False and valid
                self.log += "the configuration {} referenced by /ActiveEcuC/PduR/PduRRoutingPaths/PduRRoutingPath/{}/PduRSrcPduRRef does not exist!\n".format(
                  pdursrcpdurref_path, name)
                return valid
            else:
                # TODO: check the reference config
                pass

        return valid

    def validate_pdur_pdurroutingpath_pdurdestpdurref(self, pdur_cfg: dict) -> bool:
        valid = True

        for name, pdurroutingpath in pdur_cfg["PduRRoutingPaths"].items():
            if "DefinitionRef" not in pdurroutingpath or pdurroutingpath[
              "DefinitionRef"] != "/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRRoutingPath":
                continue
            if not AutosarUtil.get_by_path_from_dict(pdurroutingpath, "/PduRDestPduRRef/ValueRef"):
                continue
            pdurdestpdurref_path = pdurroutingpath["PduRDestPduRRef"]["ValueRef"]
            pdurdestpduref = AutosarUtil.get_by_path_from_dict(self.ecu_cfg, pdurdestpdurref_path)
            if not pdurdestpduref:
                valid = False and valid
                self.log += "the configuration {} referenced by /ActiveEcuC/PduR/PduRRoutingPaths/PduRRoutingPath/{}/PduRDestPduRef does not exist!\n".format(pdurdestpdurref_path, name)
            else:
                # TODO: check the reference config
                pass

        return valid

    def validate_pdur_pdusrchandleid(self, pdur_cfg: dict) -> bool:
        valid = True
        maxroutingpathcnt = 65535 # 作废PduRMaxRoutingPathCnt
        for name, pdursrcpdu in pdur_cfg["PduRRoutingPaths"].items():
            if "DefinitionRef" not in pdursrcpdu or pdursrcpdu[
              "DefinitionRef"] != "/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRSrcPdu":
                continue
            srcpduhandleid = int(pdursrcpdu["PduRSourcePduHandleId"]["Value"])
            if srcpduhandleid > maxroutingpathcnt:
                valid = False and valid
                self.log += "the value {} of /ActiveEcuC/PduR/PduRRoutingPaths/PduRSrcPdu/{}/PduRSourcePduHandleId is not less than the maximum value:{}".format(srcpduhandleid, name, maxroutingpathcnt) + \
                    " of /ActiveEcuC/PduR/PduRRoutingPaths/PduRMaxRoutingPathCnt.\n"
                return valid
            else:
                # TODO: check the reference config
                pass

        return valid

    def validate_pdur_pdudesthandleid(self, pdur_cfg: dict) -> bool:
        valid = True
        maxroutingpathcnt = 65535

        for name, pdurdestpdu in pdur_cfg["PduRRoutingPaths"].items():
            if "DefinitionRef" not in pdurdestpdu or pdurdestpdu[
              "DefinitionRef"] != "/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRDestPdu":
                continue
            dstpduhandleid = int(pdurdestpdu["PduRDestPduHandleId"]["Value"])
            if dstpduhandleid > maxroutingpathcnt:
                valid = False and valid
                self.log += "the value {} of /ActiveEcuC/PduR/PduRRoutingPaths/PduRDestPdu/{}/PduRDestPduHandleId is not less than the maximum value:{}".format(dstpduhandleid, name, maxroutingpathcnt) + \
                    " of /ActiveEcuC/PduR/PduRRoutingPaths/PduRMaxRoutingPathCnt.\n"
                return valid
            else:
                # TODO: check the reference config
                pass

        return valid

    def validate_pdur_pdusrchandleid_routingpathgroupcnt(self, pdur_cfg: dict) -> bool:
        valid = True
        maxroutingpathgroupcnt = 0xFFFF # PduRMaxRoutingPathGroupCnt作废

        for name, pdursrcpdu in pdur_cfg["PduRRoutingPaths"].items():
            if "DefinitionRef" not in pdursrcpdu or pdursrcpdu[
              "DefinitionRef"] != "/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRSrcPdu":
                continue
            srcpduhandleid = int(pdursrcpdu["PduRSourcePduHandleId"]["Value"])
            if srcpduhandleid > maxroutingpathgroupcnt:
                valid = False and valid
                self.log += "the value {} of /ActiveEcuC/PduR/PduRRoutingPaths/PduRSrcPdu/{}/PduRSourcePduHandleId is not less than the maximum value:{}".format(srcpduhandleid, name, maxroutingpathgroupcnt) + \
                    " of /ActiveEcuC/PduR/PduRRoutingPaths/PduRMaxRoutingPathGroupCnt.\n"
                return valid
            else:
                # TODO: check the reference config
                pass

        return valid

    def validate_pdur_pdudesthandleid_routingpathgroupcnt(self, pdur_cfg: dict) -> bool:
        valid = True
        maxroutingpathgroupcnt = 65535

        for name, pdurdestpdu in pdur_cfg["PduRRoutingPaths"].items():
            if "DefinitionRef" not in pdurdestpdu or pdurdestpdu[
              "DefinitionRef"] != "/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRDestPdu":
                continue
            dstpduhandleid = int(pdurdestpdu["PduRDestPduHandleId"]["Value"])
            if dstpduhandleid > maxroutingpathgroupcnt:
                valid = False and valid
                self.log += "the value {} of /ActiveEcuC/PduR/PduRRoutingPaths/PduRDestPdu/{}/PduRDestPduHandleId is not less than the maximum value:{}".format(dstpduhandleid, name, maxroutingpathgroupcnt) + \
                    " of /ActiveEcuC/PduR/PduRRoutingPaths/PduRMaxRoutingPathGroupCnt.\n"
                return valid
            else:
                # TODO: check the reference config
                pass

        return valid

    def validate_pdur_pdurdestpduref_pdurdestpdudataprovision(self, pdur_cfg: dict) -> bool:
        valid = True

        for name, pdurdestpdu in pdur_cfg["PduRRoutingPaths"].items():
            if "DefinitionRef" not in pdurdestpdu or pdurdestpdu[
              "DefinitionRef"] != "/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRDestPdu":
                continue
            if not AutosarUtil.get_by_path_from_dict(pdurdestpdu, "/PduRDestPduRef/ValueRef"):
                continue
            pdurdestpduref_path = pdurdestpdu["PduRDestPduRef"]["ValueRef"]
            pdurdestpdudataprovision = pdurdestpdu["PduRDestPduDataProvision"]["Value"]
            # ignore: lin diagnostic frames
            if "MasterReq" in pdurdestpduref_path or "SlaveResp" in pdurdestpduref_path:
                continue

            if "Com_CanNm_Tx" in pdurdestpduref_path and pdurdestpdudataprovision == "PDUR_DIRECT":
                valid = False and valid
                self.log += "PduRDestPduDataProvision[{}] should be set to PDUR_TRIGGERTRANSMIT in ActiveEcuC/PduR/PduRRoutingPaths/PduRDestPdu/{}!\n".format(
                  pdurdestpdudataprovision, name)
                return valid

            if "PduR_CanNm_Tx" in pdurdestpduref_path and pdurdestpdudataprovision == "PDUR_DIRECT":
                valid = False and valid
                self.log += "PduRDestPduDataProvision[{}] should be set to PDUR_TRIGGERTRANSMIT in ActiveEcuC/PduR/PduRRoutingPaths/PduRDestPdu/{}!\n".format(
                  pdurdestpdudataprovision, name)
                return valid

        return valid

    def validate(self, ecu_cfg):
        valid = True
        self.log = ""
        self.ecu_cfg = ecu_cfg
        pdur_cfg = self.ecu_cfg["ActiveEcuC"]["PduR"]

        valid = self.validate_pdur_pdursrcpduref(pdur_cfg)
        valid = self.validate_pdur_pdurdestpduref(pdur_cfg) and valid
        valid = self.validate_pdur_pdurroutingpath_pdursrcpdurref(pdur_cfg) and valid
        valid = self.validate_pdur_pdurroutingpath_pdurdestpdurref(pdur_cfg) and valid
        valid = self.validate_pdur_pdusrchandleid(pdur_cfg) and valid
        valid = self.validate_pdur_pdudesthandleid(pdur_cfg) and valid
        valid = self.validate_pdur_pdusrchandleid_routingpathgroupcnt(pdur_cfg) and valid
        valid = self.validate_pdur_pdudesthandleid_routingpathgroupcnt(pdur_cfg) and valid
        valid = self.validate_pdur_pdurdestpduref_pdurdestpdudataprovision(pdur_cfg) and valid
        return valid
