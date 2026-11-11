from basic_func_module.autosar_utils.autosar_util import AutosarUtil

class CanNmRule():

    def __init__(self) -> None:
        self.log = ""
        self.ecu_cfg = None

    def validate_cannm_bus_load_reduction_enabled(self, cannm_cfg: dict) -> bool:
        valid = True
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                CanNmBusLoadReductionEnabled = cannm_cfg[CanNmConfigKey]["CanNmBusLoadReductionEnabled"]["Value"]
                CanNmPassiveModeEnabled = cannm_cfg[CanNmConfigKey]["CanNmPassiveModeEnabled"]["Value"]
                CanNmGlobalPnSupport = cannm_cfg[CanNmConfigKey]["CanNmGlobalPnSupport"]["Value"]
                if (True == CanNmPassiveModeEnabled or True == CanNmGlobalPnSupport) and (True == CanNmBusLoadReductionEnabled):
                    valid = False
                    self.log += "CanNm/{}/CanNmBusLoadReductionEnabled[{}] should be False if CanNm/{}/CanNmPassiveModeEnabled[{}] is set to True or CanNm/{}/CanNmGlobalPnSupport[{}] is set to True!\n".format(
                    CanNmConfigKey, CanNmBusLoadReductionEnabled, CanNmConfigKey, CanNmPassiveModeEnabled, CanNmConfigKey, CanNmGlobalPnSupport)
                    return valid
        return valid

    def validate_cannm_bus_synchronization_enabled(self, cannm_cfg: dict, nm_cfg: dict) -> bool:
        valid = True
        for NmConfigKey, NmConfigValue in nm_cfg.items():
            if NmConfigKey == "DefinitionRef":
                continue
            if NmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/Nm/NmGlobalConfig':
                for Nmchannelkey, Nmchannelvalue in nm_cfg[NmConfigKey].items():
                    if not isinstance(nm_cfg[NmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in nm_cfg[NmConfigKey]:
                        continue
                    if Nmchannelkey == "DefinitionRef":
                        continue
                    if "DefinitionRef" not in Nmchannelvalue:
                        continue
                    if Nmchannelvalue["DefinitionRef"] == '/AUTOSAR/EcucDefs/Nm/NmGlobalConfig/NmGlobalFeatures':
                        NmBusSynchronizationEnabled = nm_cfg[NmConfigKey][Nmchannelkey]["NmBusSynchronizationEnabled"]["Value"]
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                CanNmBusSynchronizationEnabled = cannm_cfg[CanNmConfigKey]["CanNmBusSynchronizationEnabled"]["Value"]
                CanNmPassiveModeEnabled = cannm_cfg[CanNmConfigKey]["CanNmPassiveModeEnabled"]["Value"]
                if False == CanNmPassiveModeEnabled and (CanNmBusSynchronizationEnabled != NmBusSynchronizationEnabled):
                    valid = False
                    self.log += "CanNm/CanNmGlobalConfig/CanNmBusSynchronizationEnabled[{}] should be the same as Nm/NmGlobalConfig/NmGlobalFeatures/NmBusSynchronizationEnabled[{}] if CanNm/CanNmGlobalConfig/CanNmPassiveModeEnabled[{}] is False!\n".format(
                    CanNmBusSynchronizationEnabled, NmBusSynchronizationEnabled, CanNmPassiveModeEnabled)
                    return valid
                if True == CanNmPassiveModeEnabled and (True == CanNmBusSynchronizationEnabled):
                    valid = False
                    self.log += "CanNm/CanNmGlobalConfig/CanNmBusSynchronizationEnabled[{}] should be False if CanNm/CanNmGlobalConfig/CanNmPassiveModeEnabled[{}] is set to True!\n".format(
                    CanNmBusSynchronizationEnabled, CanNmPassiveModeEnabled)
                    return valid
        return valid

    def validate_cannm_com_control_enabled(self, cannm_cfg: dict, nm_cfg: dict) -> bool:
        valid = True
        for NmConfigKey, NmConfigValue in nm_cfg.items():
            if NmConfigKey == "DefinitionRef":
                continue
            if NmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/Nm/NmGlobalConfig':
                for Nmchannelkey, Nmchannelvalue in nm_cfg[NmConfigKey].items():
                    if not isinstance(nm_cfg[NmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in nm_cfg[NmConfigKey]:
                        continue
                    if Nmchannelkey == "DefinitionRef":
                        continue
                    if "DefinitionRef" not in Nmchannelvalue:
                        continue
                    if Nmchannelvalue["DefinitionRef"] == '/AUTOSAR/EcucDefs/Nm/NmGlobalConfig/NmGlobalFeatures':
                        NmComControlEnabled = nm_cfg[NmConfigKey][Nmchannelkey]["NmComControlEnabled"]["Value"]
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                CanNmComControlEnabled = cannm_cfg[CanNmConfigKey]["CanNmComControlEnabled"]["Value"]
                CanNmPassiveModeEnabled = cannm_cfg[CanNmConfigKey]["CanNmPassiveModeEnabled"]["Value"]
                if False == CanNmPassiveModeEnabled and (CanNmComControlEnabled != NmComControlEnabled):
                    valid = False
                    self.log += "CanNm/CanNmGlobalConfig/CanNmComControlEnabled[{}] should be the same as Nm/NmGlobalConfig/NmGlobalFeatures/NmComControlEnabled[{}] if CanNm/CanNmGlobalConfig/CanNmPassiveModeEnabled[{}] is False!\n".format(
                    CanNmComControlEnabled, NmComControlEnabled, CanNmPassiveModeEnabled)
                    return valid
                if True == CanNmPassiveModeEnabled and (True == CanNmComControlEnabled):
                    valid = False
                    self.log += "CanNm/CanNmGlobalConfig/CanNmComControlEnabled[{}] should be False if CanNm/CanNmGlobalConfig/CanNmPassiveModeEnabled[{}] is set to True!\n".format(
                    CanNmComControlEnabled, CanNmPassiveModeEnabled)
                    return valid
        return valid

    def validate_cannm_com_user_data_support(self, cannm_cfg: dict) -> bool:
        valid = True
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                CanNmComUserDataSupport = cannm_cfg[CanNmConfigKey]["CanNmComUserDataSupport"]["Value"]
                CanNmPassiveModeEnabled = cannm_cfg[CanNmConfigKey]["CanNmPassiveModeEnabled"]["Value"]
                if True == CanNmPassiveModeEnabled and (True == CanNmComUserDataSupport):
                    valid = False
                    self.log += "CanNm/CanNmGlobalConfig/CanNmComUserDataSupport[{}] should be False if CanNm/CanNmGlobalConfig/CanNmPassiveModeEnabled[{}] is set to True!\n".format(
                    CanNmComUserDataSupport, CanNmPassiveModeEnabled)
                    return valid
        return valid

    def validate_cannm_coordinator_sync_support(self, cannm_cfg: dict) -> bool:
        valid = True
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                CanNmCoordinatorSyncSupport = cannm_cfg[CanNmConfigKey]["CanNmCoordinatorSyncSupport"]["Value"]
                CanNmPassiveModeEnabled = cannm_cfg[CanNmConfigKey]["CanNmPassiveModeEnabled"]["Value"]
                if True == CanNmPassiveModeEnabled and (True == CanNmCoordinatorSyncSupport):
                    valid = False
                    self.log += "CanNm/CanNmGlobalConfig/CanNmCoordinatorSyncSupport[{}] has to be set to False if CanNm/CanNmGlobalConfig/CanNmPassiveModeEnabled[{}] is set to True!\n".format(
                    CanNmCoordinatorSyncSupport, CanNmPassiveModeEnabled)
                    return valid
        return valid

    def validate_cannm_immediate_txconf_enabled(self, cannm_cfg: dict) -> bool:
        valid = True
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                CanNmImmediateTxconfEnabled = cannm_cfg[CanNmConfigKey]["CanNmImmediateTxconfEnabled"]["Value"]
                CanNmPassiveModeEnabled = cannm_cfg[CanNmConfigKey]["CanNmPassiveModeEnabled"]["Value"]
                if True == CanNmPassiveModeEnabled and (True == CanNmImmediateTxconfEnabled):
                    valid = False
                    self.log += "CanNm/CanNmGlobalConfig/CanNmImmediateTxconfEnabled[{}] should be False if CanNm/CanNmGlobalConfig/CanNmPassiveModeEnabled[{}] is set to True!\n".format(
                    CanNmImmediateTxconfEnabled, CanNmPassiveModeEnabled)
                    return valid
        return valid

    def validate_cannm_pdu_rx_indication_enabled(self, cannm_cfg: dict, nm_cfg: dict) -> bool:
        valid = True
        for NmConfigKey, NmConfigValue in nm_cfg.items():
            if NmConfigKey == "DefinitionRef":
                continue
            if NmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/Nm/NmGlobalConfig':
                for Nmchannelkey, Nmchannelvalue in nm_cfg[NmConfigKey].items():
                    if not isinstance(nm_cfg[NmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in nm_cfg[NmConfigKey]:
                        continue
                    if Nmchannelkey == "DefinitionRef":
                        continue
                    if "DefinitionRef" not in Nmchannelvalue:
                        continue
                    if Nmchannelvalue["DefinitionRef"] == '/AUTOSAR/EcucDefs/Nm/NmGlobalConfig/NmGlobalFeatures':
                        NmPduRxIndicationEnabled = nm_cfg[NmConfigKey][Nmchannelkey]["NmPduRxIndicationEnabled"]["Value"]
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                CanNmPduRxIndicationEnabled = cannm_cfg[CanNmConfigKey]["CanNmPduRxIndicationEnabled"]["Value"]
                if CanNmPduRxIndicationEnabled != NmPduRxIndicationEnabled:
                    valid = False
                    self.log += "CanNm/CanNmGlobalConfig/CanNmPduRxIndicationEnabled[{}] should be the same as Nm/NmGlobalConfig/NmGlobalFeatures/NmPduRxIndicationEnabled[{}]!\n".format(
                    CanNmPduRxIndicationEnabled, NmPduRxIndicationEnabled)
                    return valid
        return valid

    def validate_cannm_pn_eira_calc_enabled(self, cannm_cfg: dict) -> bool:
        valid = True
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                CanNmPnEiraCalcEnabled = cannm_cfg[CanNmConfigKey]["CanNmPnEiraCalcEnabled"]["Value"]
                CanNmGlobalPnSupport = cannm_cfg[CanNmConfigKey]["CanNmGlobalPnSupport"]["Value"]
                if (True == CanNmPnEiraCalcEnabled) and (False == CanNmGlobalPnSupport):
                    valid = False
                    self.log += "CanNm/CanNmGlobalConfig/CanNmPnEiraCalcEnabled[{}] should be True if CanNm/CanNmGlobalConfig/CanNmGlobalPnSupport[{}] is set to True!\n".format(
                    CanNmPnEiraCalcEnabled, CanNmGlobalPnSupport)
                    return valid
        return valid

    def validate_cannm_remote_sleep_ind_enabled(self, cannm_cfg: dict, nm_cfg: dict) -> bool:
        valid = True
        for NmConfigKey, NmConfigValue in nm_cfg.items():
            if NmConfigKey == "DefinitionRef":
                continue
            if NmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/Nm/NmGlobalConfig':
                for Nmchannelkey, Nmchannelvalue in nm_cfg[NmConfigKey].items():
                    if not isinstance(nm_cfg[NmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in nm_cfg[NmConfigKey]:
                        continue
                    if Nmchannelkey == "DefinitionRef":
                        continue
                    if "DefinitionRef" not in Nmchannelvalue:
                        continue
                    if Nmchannelvalue["DefinitionRef"] == '/AUTOSAR/EcucDefs/Nm/NmGlobalConfig/NmGlobalFeatures':
                        NmRemoteSleepIndEnabled = nm_cfg[NmConfigKey][Nmchannelkey]["NmRemoteSleepIndEnabled"]["Value"]
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                CanNmRemoteSleepIndEnabled = cannm_cfg[CanNmConfigKey]["CanNmRemoteSleepIndEnabled"]["Value"]
                CanNmPassiveModeEnabled = cannm_cfg[CanNmConfigKey]["CanNmPassiveModeEnabled"]["Value"]
                if False == CanNmPassiveModeEnabled and (CanNmRemoteSleepIndEnabled != NmRemoteSleepIndEnabled):
                    valid = False
                    self.log += "CanNm/CanNmGlobalConfig/CanNmRemoteSleepIndEnabled[{}] should be the same as Nm/NmGlobalConfig/NmGlobalFeatures/NmRemoteSleepIndEnabled[{}] if CanNm/CanNmGlobalConfig/CanNmPassiveModeEnabled[{}] is False!\n".format(
                    CanNmRemoteSleepIndEnabled, NmRemoteSleepIndEnabled, CanNmPassiveModeEnabled)
                    return valid
                if True == CanNmPassiveModeEnabled and (True == CanNmRemoteSleepIndEnabled):
                    valid = False
                    self.log += "CanNm/CanNmGlobalConfig/CanNmRemoteSleepIndEnabled[{}] should be False if CanNm/CanNmGlobalConfig/CanNmPassiveModeEnabled[{}] is set to True!\n".format(
                    CanNmRemoteSleepIndEnabled, CanNmPassiveModeEnabled)
                    return valid
        return valid

    def validate_cannm_state_change_ind_enabled(self, cannm_cfg: dict, nm_cfg: dict) -> bool:
        valid = True
        for NmConfigKey, NmConfigValue in nm_cfg.items():
            if NmConfigKey == "DefinitionRef":
                continue
            if NmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/Nm/NmGlobalConfig':
                for Nmchannelkey, Nmchannelvalue in nm_cfg[NmConfigKey].items():
                    if not isinstance(nm_cfg[NmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in nm_cfg[NmConfigKey]:
                        continue
                    if Nmchannelkey == "DefinitionRef":
                        continue
                    if "DefinitionRef" not in Nmchannelvalue:
                        continue
                    if Nmchannelvalue["DefinitionRef"] == '/AUTOSAR/EcucDefs/Nm/NmGlobalConfig/NmGlobalFeatures':
                        NmStateChangeIndEnabled = nm_cfg[NmConfigKey][Nmchannelkey]["NmStateChangeIndEnabled"]["Value"]
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                CanNmStateChangeIndEnabled = cannm_cfg[CanNmConfigKey]["CanNmStateChangeIndEnabled"]["Value"]
                if CanNmStateChangeIndEnabled != NmStateChangeIndEnabled:
                    valid = False
                    self.log += "CanNm/CanNmGlobalConfig/CanNmStateChangeIndEnabled[{}] should be the same as Nm/NmGlobalConfig/NmGlobalFeatures/NmStateChangeIndEnabled[{}]!\n".format(
                    CanNmStateChangeIndEnabled, NmStateChangeIndEnabled)
                    return valid
        return valid

    def validate_cannm_user_data_enabled(self, cannm_cfg: dict, nm_cfg: dict) -> bool:
        valid = True
        for NmConfigKey, NmConfigValue in nm_cfg.items():
            if NmConfigKey == "DefinitionRef":
                continue
            if NmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/Nm/NmGlobalConfig':
                for Nmchannelkey, Nmchannelvalue in nm_cfg[NmConfigKey].items():
                    if not isinstance(nm_cfg[NmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in nm_cfg[NmConfigKey]:
                        continue
                    if Nmchannelkey == "DefinitionRef":
                        continue
                    if "DefinitionRef" not in Nmchannelvalue:
                        continue
                    if Nmchannelvalue["DefinitionRef"] == '/AUTOSAR/EcucDefs/Nm/NmGlobalConfig/NmGlobalFeatures':
                        NmUserDataEnabled = nm_cfg[NmConfigKey][Nmchannelkey]["NmUserDataEnabled"]["Value"]
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                CanNmUserDataEnabled = cannm_cfg[CanNmConfigKey]["CanNmUserDataEnabled"]["Value"]
                if CanNmUserDataEnabled != NmUserDataEnabled:
                    valid = False
                    self.log += "CanNm/CanNmGlobalConfig/CanNmUserDataEnabled[{}] should be the same as Nm/NmGlobalConfig/NmGlobalFeatures/NmUserDataEnabled[{}]!\n".format(
                    CanNmUserDataEnabled, NmUserDataEnabled)
                    return valid
        return valid

    def validate_cannm_pn_reset_time(self, cannm_cfg: dict) -> bool:
        valid = True
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                CanNmPnResetTime = cannm_cfg[CanNmConfigKey]["CanNmPnResetTime"]["Value"]
                CanNmChannelConfig = {}
                CanNmChannelConfig = dict.fromkeys(["CanNmMsgCycleTime", "CanNmTimeoutTime"])

                for channelkey, channelvalue in cannm_cfg[CanNmConfigKey].items():
                    if not isinstance(cannm_cfg[CanNmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in cannm_cfg[CanNmConfigKey]:
                        continue
                    if channelkey == 'DefinitionRef':
                        continue
                    if "DefinitionRef" not in channelvalue:
                        continue
                    if channelvalue["DefinitionRef"] != '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig':
                        continue

                    CanNmChannelConfig_dict = cannm_cfg[CanNmConfigKey][channelkey]
                    CanNmChannelConfig["CanNmMsgCycleTime"] = CanNmChannelConfig_dict["CanNmMsgCycleTime"]["Value"]
                    CanNmChannelConfig["CanNmTimeoutTime"] = CanNmChannelConfig_dict["CanNmTimeoutTime"]["Value"]
                    if CanNmChannelConfig["CanNmMsgCycleTime"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmMsgCycleTime[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmMsgCycleTime"])
                        return valid
                    if CanNmChannelConfig["CanNmTimeoutTime"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmTimeoutTime[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmTimeoutTime"])
                        return valid
                    if CanNmPnResetTime <= CanNmChannelConfig["CanNmMsgCycleTime"] or CanNmPnResetTime >= CanNmChannelConfig[
                    "CanNmTimeoutTime"]:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmPnResetTime[{}] should be greater than or equal to CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmMsgCycleTime[{}] and less than or equal to CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmTimeoutTime[{}]!\n".format(
                        CanNmPnResetTime, channelkey, CanNmChannelConfig["CanNmMsgCycleTime"], channelkey,
                        CanNmChannelConfig["CanNmTimeoutTime"])
                        return valid
        return valid

    def validate_cannm_pn_eira_rxnsdu_ref(self, cannm_cfg: dict, ecuc_cfg: dict) -> bool:
        valid = True
        samecount = 0
        ecuc_pdu = ecuc_cfg["EcucConfigSet"]["EcucPduCollection"]
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                for globalkey, globalvalue in cannm_cfg[CanNmConfigKey].items():
                    if globalkey == "CanNmPnEiraRxNSduRef":
                        CanNmPnEiraRxNSduRef = cannm_cfg[CanNmConfigKey]["CanNmPnEiraRxNSduRef"]["ValueRef"]
                        if CanNmPnEiraRxNSduRef is None:
                            valid = False
                            self.log += "CanNm/CanNmGlobalConfig/CanNmPnEiraRxNSduRef should not be empty!\n"
                            return valid
                        elif len(CanNmPnEiraRxNSduRef) == 0 or CanNmPnEiraRxNSduRef == "<empty>":
                            # valid = False
                            self.log += "CanNm/CanNmGlobalConfig/CanNmPnEiraRxNSduRef should not be empty!\n"
                            return valid
                        else:
                            CanNmPnEiraRxNSduRef = CanNmPnEiraRxNSduRef.split('/')[-1]
                            for ecuckey, ecucvalue in ecuc_pdu.items():
                                if ecuckey == CanNmPnEiraRxNSduRef:
                                    samecount = 1
                            if samecount == 0:
                                valid = False
                                self.log += "CanNm/CanNmGlobalConfig/CanNmPnEiraRxNSduRef referenced by EcuC/EcucConfigSet/EcucPduCollection/Pdus does not exist!\n"
                                return valid
        return valid

    def validate_cannm_bus_load_reduction_active(self, cannm_cfg: dict) -> bool:
        valid = True
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                CanNmBusLoadReductionEnabled = cannm_cfg[CanNmConfigKey]["CanNmBusLoadReductionEnabled"]["Value"]
                CanNmChannelConfig = {}
                CanNmChannelConfig = dict.fromkeys(["CanNmBusLoadReductionActive", "CanNmTimeoutTime"])
                for channelkey, channelvalue in cannm_cfg[CanNmConfigKey].items():
                    if not isinstance(cannm_cfg[CanNmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in cannm_cfg[CanNmConfigKey]:
                        continue
                    if channelkey == 'DefinitionRef':
                        continue
                    if "DefinitionRef" not in channelvalue:
                        continue
                    if channelvalue["DefinitionRef"] != '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig':
                        continue
                    CanNmChannelConfig_dict = cannm_cfg[CanNmConfigKey][channelkey]
                    CanNmChannelConfig["CanNmBusLoadReductionActive"] = CanNmChannelConfig_dict["CanNmBusLoadReductionActive"][
                    "Value"]
                    if False == CanNmBusLoadReductionEnabled and True == CanNmChannelConfig["CanNmBusLoadReductionActive"]:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmBusLoadReductionActive[{}] should be False if CanNm/CanNmGlobalConfig/CanNmBusLoadReductionEnabled[{}] is set to False!\n".format(
                        channelkey, CanNmChannelConfig["CanNmBusLoadReductionActive"], CanNmBusLoadReductionEnabled)
                        return valid
        return valid

    def validate_cannm_car_wake_up_filter_enabled(self, cannm_cfg: dict) -> bool:
        valid = True
        CanNmChannelConfig = {}
        CanNmChannelConfig = dict.fromkeys(["CanNmCarWakeUpFilterEnabled", "CanNmCarWakeUpRxEnabled"])
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                for channelkey, channelvalue in cannm_cfg[CanNmConfigKey].items():
                    if not isinstance(cannm_cfg[CanNmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in cannm_cfg[CanNmConfigKey]:
                        continue

                    if channelkey == 'DefinitionRef':
                        continue
                    if "DefinitionRef" not in channelvalue:
                        continue
                    if channelvalue["DefinitionRef"] != '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig':
                        continue

                    CanNmChannelConfig_dict = cannm_cfg[CanNmConfigKey][channelkey]
                    CanNmChannelConfig["CanNmCarWakeUpFilterEnabled"] = CanNmChannelConfig_dict["CanNmCarWakeUpFilterEnabled"][
                    "Value"]
                    CanNmChannelConfig["CanNmCarWakeUpRxEnabled"] = CanNmChannelConfig_dict["CanNmCarWakeUpRxEnabled"]["Value"]

                    if True == CanNmChannelConfig["CanNmCarWakeUpRxEnabled"] and False == CanNmChannelConfig[
                    "CanNmCarWakeUpFilterEnabled"]:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmCarWakeUpFilterEnabled[{}] should be True if CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmCarWakeUpRxEnabled[{}] is set to True!\n".format(
                        channelkey, CanNmChannelConfig["CanNmCarWakeUpFilterEnabled"], channelkey,
                        CanNmChannelConfig["CanNmCarWakeUpRxEnabled"])
                        return valid
        return valid

    def validate_cannm_node_detection_enabled(self, cannm_cfg: dict) -> bool:
        valid = True
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                CanNmPassiveModeEnabled = cannm_cfg[CanNmConfigKey]["CanNmPassiveModeEnabled"]["Value"]
                CanNmChannelConfig = {}
                CanNmChannelConfig = dict.fromkeys(["CanNmNodeIdEnabled", "CanNmNodeDetectionEnabled"])

                for channelkey, channelvalue in cannm_cfg[CanNmConfigKey].items():
                    if not isinstance(cannm_cfg[CanNmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in cannm_cfg[CanNmConfigKey]:
                        continue

                    if channelkey == 'DefinitionRef':
                        continue
                    if "DefinitionRef" not in channelvalue:
                        continue
                    if channelvalue["DefinitionRef"] != '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig':
                        continue

                    CanNmChannelConfig_dict = cannm_cfg[CanNmConfigKey][channelkey]
                    CanNmChannelConfig["CanNmNodeIdEnabled"] = CanNmChannelConfig_dict["CanNmNodeIdEnabled"]["Value"]
                    CanNmChannelConfig["CanNmNodeDetectionEnabled"] = CanNmChannelConfig_dict["CanNmNodeDetectionEnabled"][
                    "Value"]

                    if True == CanNmChannelConfig["CanNmNodeIdEnabled"] and False == CanNmChannelConfig[
                    "CanNmNodeDetectionEnabled"]:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmNodeDetectionEnabled[{}] should be True if CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmNodeIdEnabled[{}] is set to True!\n".format(
                        channelkey, CanNmChannelConfig["CanNmNodeDetectionEnabled"], channelkey,
                        CanNmChannelConfig["CanNmNodeIdEnabled"])
                        return valid
                    if True == CanNmPassiveModeEnabled and True == CanNmChannelConfig["CanNmNodeDetectionEnabled"]:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmNodeDetectionEnabled[{}] should be False if CanNm/CanNmGlobalConfig/CanNmPassiveModeEnabled[{}] is set to True!\n".format(
                        channelkey, CanNmChannelConfig["CanNmNodeDetectionEnabled"], CanNmPassiveModeEnabled)
                        return valid
        return valid

    def validate_cannm_pn_enabled(self, cannm_cfg: dict) -> bool:
        valid = True
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                CanNmGlobalPnSupport = cannm_cfg[CanNmConfigKey]["CanNmGlobalPnSupport"]["Value"]
                CanNmChannelConfig = {}
                CanNmChannelConfig = dict.fromkeys(["CanNmPnEnabled", "CanNmNodeDetectionEnabled"])

                for channelkey, channelvalue in cannm_cfg[CanNmConfigKey].items():
                    if not isinstance(cannm_cfg[CanNmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in cannm_cfg[CanNmConfigKey]:
                        continue

                    if channelkey == 'DefinitionRef':
                        continue
                    if "DefinitionRef" not in channelvalue:
                        continue
                    if channelvalue["DefinitionRef"] != '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig':
                        continue

                    CanNmChannelConfig_dict = cannm_cfg[CanNmConfigKey][channelkey]
                    CanNmChannelConfig["CanNmPnEnabled"] = CanNmChannelConfig_dict["CanNmPnEnabled"]["Value"]

                    if (True == CanNmChannelConfig["CanNmPnEnabled"]) and (False == CanNmGlobalPnSupport):
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmPnEnabled[{}] should be True if CanNm/CanNmGlobalConfig/CanNmGlobalPnSupport[{}] is set to True!\n".format(
                        channelkey, CanNmChannelConfig["CanNmPnEnabled"], CanNmGlobalPnSupport)
                        return valid
        return valid

    def validate_cannm_pn_era_calc_enabled(self, cannm_cfg: dict) -> bool:
        valid = True
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                CanNmGlobalPnSupport = cannm_cfg[CanNmConfigKey]["CanNmGlobalPnSupport"]["Value"]
                CanNmChannelConfig = {}
                CanNmChannelConfig = dict.fromkeys(["CanNmPnEraCalcEnabled", "CanNmNodeDetectionEnabled"])

                for channelkey, channelvalue in cannm_cfg[CanNmConfigKey].items():
                    if not isinstance(cannm_cfg[CanNmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in cannm_cfg[CanNmConfigKey]:
                        continue

                    if channelkey == 'DefinitionRef':
                        continue
                    if "DefinitionRef" not in channelvalue:
                        continue
                    if channelvalue["DefinitionRef"] != '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig':
                        continue

                    CanNmChannelConfig_dict = cannm_cfg[CanNmConfigKey][channelkey]
                    CanNmChannelConfig["CanNmPnEraCalcEnabled"] = CanNmChannelConfig_dict["CanNmPnEraCalcEnabled"]["Value"]

                    if (True == CanNmChannelConfig["CanNmPnEraCalcEnabled"]) and (False == CanNmGlobalPnSupport):
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmPnEraCalcEnabled[{}] should be True if CanNm/CanNmGlobalConfig/CanNmGlobalPnSupport[{}] is set to True!\n".format(
                        channelkey, CanNmChannelConfig["CanNmPnEraCalcEnabled"], CanNmGlobalPnSupport)
                        return valid
        return valid

    def validate_cannm_pn_handle_multiple_network_requests(self, cannm_cfg: dict) -> bool:
        valid = True
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                CanNmGlobalPnSupport = cannm_cfg[CanNmConfigKey]["CanNmGlobalPnSupport"]["Value"]
                CanNmChannelConfig = {}
                CanNmChannelConfig = dict.fromkeys(["CanNmPnHandleMultipleNetworkRequests", "CanNmNodeDetectionEnabled"])

                for channelkey, channelvalue in cannm_cfg[CanNmConfigKey].items():
                    if not isinstance(cannm_cfg[CanNmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in cannm_cfg[CanNmConfigKey]:
                        continue

                    if channelkey == 'DefinitionRef':
                        continue
                    if "DefinitionRef" not in channelvalue:
                        continue
                    if channelvalue["DefinitionRef"] != '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig':
                        continue

                    CanNmChannelConfig_dict = cannm_cfg[CanNmConfigKey][channelkey]
                    CanNmChannelConfig["CanNmPnHandleMultipleNetworkRequests"] = CanNmChannelConfig_dict[
                    "CanNmPnHandleMultipleNetworkRequests"]["Value"]

                    if (True == CanNmChannelConfig["CanNmPnHandleMultipleNetworkRequests"]) and (False == CanNmGlobalPnSupport):
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmPnHandleMultipleNetworkRequests[{}] should be True if CanNm/CanNmGlobalConfig/CanNmGlobalPnSupport[{}] is set to True!\n".format(
                        channelkey, CanNmChannelConfig["CanNmPnHandleMultipleNetworkRequests"], CanNmGlobalPnSupport)
                        return valid
        return valid

    def validate_cannm_immediate_nm_cycle_time(self, cannm_cfg: dict) -> bool:
        valid = True
        CanNmChannelConfig = {}
        CanNmChannelConfig = dict.fromkeys(["CanNmImmediateNmCycleTime", "CanNmImmediateNmTransmissions"])
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                for channelkey, channelvalue in cannm_cfg[CanNmConfigKey].items():
                    if not isinstance(cannm_cfg[CanNmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in cannm_cfg[CanNmConfigKey]:
                        continue

                    if channelkey == 'DefinitionRef':
                        continue
                    if "DefinitionRef" not in channelvalue:
                        continue
                    if channelvalue["DefinitionRef"] != '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig':
                        continue

                    CanNmChannelConfig_dict = cannm_cfg[CanNmConfigKey][channelkey]
                    CanNmChannelConfig["CanNmImmediateNmCycleTime"] = CanNmChannelConfig_dict["CanNmImmediateNmCycleTime"][
                    "Value"]
                    CanNmChannelConfig["CanNmImmediateNmTransmissions"] = CanNmChannelConfig_dict[
                    "CanNmImmediateNmTransmissions"]["Value"]
                    if CanNmChannelConfig["CanNmImmediateNmCycleTime"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmImmediateNmCycleTime[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmImmediateNmCycleTime"])
                        return valid
                    if CanNmChannelConfig["CanNmImmediateNmTransmissions"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmImmediateNmTransmissions[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmImmediateNmTransmissions"])
                        return valid
                    if CanNmChannelConfig["CanNmImmediateNmTransmissions"] <= CanNmChannelConfig["CanNmImmediateNmCycleTime"]:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmImmediateNmCycleTime[{}] should be less than CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmImmediateNmTransmissions[{}]!\n".format(
                        channelkey, CanNmChannelConfig["CanNmImmediateNmCycleTime"], channelkey,
                        CanNmChannelConfig["CanNmImmediateNmTransmissions"])
                        return valid
        return valid

    def validate_cannm_msg_cycle_offset(self, cannm_cfg: dict) -> bool:
        valid = True
        CanNmChannelConfig = {}
        CanNmChannelConfig = dict.fromkeys(["CanNmMsgCycleTime", "CanNmMsgCycleOffset"])
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                for channelkey, channelvalue in cannm_cfg[CanNmConfigKey].items():
                    if not isinstance(cannm_cfg[CanNmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in cannm_cfg[CanNmConfigKey]:
                        continue

                    if channelkey == 'DefinitionRef':
                        continue
                    if "DefinitionRef" not in channelvalue:
                        continue
                    if channelvalue["DefinitionRef"] != '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig':
                        continue

                    CanNmChannelConfig_dict = cannm_cfg[CanNmConfigKey][channelkey]
                    CanNmChannelConfig["CanNmMsgCycleOffset"] = CanNmChannelConfig_dict["CanNmMsgCycleOffset"]["Value"]
                    CanNmChannelConfig["CanNmMsgCycleTime"] = CanNmChannelConfig_dict["CanNmMsgCycleTime"]["Value"]
                    if CanNmChannelConfig["CanNmMsgCycleOffset"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmMsgCycleOffset[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmMsgCycleOffset"])
                        return valid
                    if CanNmChannelConfig["CanNmMsgCycleTime"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmMsgCycleTime[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmMsgCycleTime"])
                        return valid
                    if CanNmChannelConfig["CanNmMsgCycleTime"] <= CanNmChannelConfig["CanNmMsgCycleOffset"]:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmMsgCycleOffset[{}] should be less than CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmMsgCycleTime[{}]!\n".format(
                        channelkey, CanNmChannelConfig["CanNmMsgCycleOffset"], channelkey,
                        CanNmChannelConfig["CanNmMsgCycleTime"])
                        return valid
        return valid

    def validate_cannm_msg_reduced_time(self, cannm_cfg: dict) -> bool:
        valid = True
        CanNmChannelConfig = {}
        CanNmChannelConfig = dict.fromkeys(["CanNmMsgReducedTime", "CanNmMsgCycleTime"])
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                for channelkey, channelvalue in cannm_cfg[CanNmConfigKey].items():
                    if not isinstance(cannm_cfg[CanNmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in cannm_cfg[CanNmConfigKey]:
                        continue

                    if channelkey == 'DefinitionRef':
                        continue
                    if "DefinitionRef" not in channelvalue:
                        continue
                    if channelvalue["DefinitionRef"] != '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig':
                        continue

                    CanNmChannelConfig_dict = cannm_cfg[CanNmConfigKey][channelkey]
                    CanNmChannelConfig["CanNmMsgCycleTime"] = CanNmChannelConfig_dict["CanNmMsgCycleTime"]["Value"]
                    CanNmChannelConfig["CanNmMsgReducedTime"] = CanNmChannelConfig_dict["CanNmMsgReducedTime"]["Value"]
                    if CanNmChannelConfig["CanNmMsgCycleTime"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmMsgCycleTime[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmMsgCycleTime"])
                        return valid
                    if CanNmChannelConfig["CanNmMsgReducedTime"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmMsgReducedTime[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmMsgReducedTime"])
                        return valid
                    if CanNmChannelConfig["CanNmMsgReducedTime"] >= CanNmChannelConfig[
                    "CanNmMsgCycleTime"] or CanNmChannelConfig[
                        "CanNmMsgReducedTime"] < 0.5 * CanNmChannelConfig["CanNmMsgCycleTime"]:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmMsgReducedTime[{}] should be less than CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmMsgCycleTime[{}] and greater than or equal to {}!\n".format(
                        channelkey, CanNmChannelConfig["CanNmMsgReducedTime"], channelkey,
                        CanNmChannelConfig["CanNmMsgCycleTime"], 0.5 * CanNmChannelConfig["CanNmMsgCycleTime"])
                        return valid
        return valid

    def validate_cannm_msg_timeout_time(self, cannm_cfg: dict) -> bool:
        valid = True
        CanNmChannelConfig = {}
        CanNmChannelConfig = dict.fromkeys(["CanNmMsgCycleTime", "CanNmMsgTimeoutTime"])
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                for channelkey, channelvalue in cannm_cfg[CanNmConfigKey].items():
                    if not isinstance(cannm_cfg[CanNmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in cannm_cfg[CanNmConfigKey]:
                        continue

                    if channelkey == 'DefinitionRef':
                        continue
                    if "DefinitionRef" not in channelvalue:
                        continue
                    if channelvalue["DefinitionRef"] != '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig':
                        continue

                    CanNmChannelConfig_dict = cannm_cfg[CanNmConfigKey][channelkey]
                    CanNmChannelConfig["CanNmMsgTimeoutTime"] = CanNmChannelConfig_dict["CanNmMsgTimeoutTime"]["Value"]
                    CanNmChannelConfig["CanNmMsgCycleTime"] = CanNmChannelConfig_dict["CanNmMsgCycleTime"]["Value"]
                    if CanNmChannelConfig["CanNmMsgTimeoutTime"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmMsgTimeoutTime[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmMsgTimeoutTime"])
                        return valid
                    if CanNmChannelConfig["CanNmMsgCycleTime"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmMsgCycleTime[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmMsgCycleTime"])
                        return valid
                    if CanNmChannelConfig["CanNmMsgCycleTime"] <= CanNmChannelConfig["CanNmMsgTimeoutTime"]:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmMsgTimeoutTime[{}] should be less than CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmMsgCycleTime[{}]!\n".format(
                        channelkey, CanNmChannelConfig["CanNmMsgTimeoutTime"], channelkey,
                        CanNmChannelConfig["CanNmMsgCycleTime"])
                        return valid
        return valid

    def validate_cannm_remote_sleep_ind_time(self, cannm_cfg: dict) -> bool:
        valid = True
        CanNmChannelConfig = {}
        CanNmChannelConfig = dict.fromkeys(["CanNmMsgCycleTime", "CanNmRemoteSleepIndTime"])
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                for channelkey, channelvalue in cannm_cfg[CanNmConfigKey].items():
                    if not isinstance(cannm_cfg[CanNmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in cannm_cfg[CanNmConfigKey]:
                        continue

                    if channelkey == 'DefinitionRef':
                        continue
                    if "DefinitionRef" not in channelvalue:
                        continue
                    if channelvalue["DefinitionRef"] != '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig':
                        continue

                    CanNmChannelConfig_dict = cannm_cfg[CanNmConfigKey][channelkey]
                    CanNmChannelConfig["CanNmRemoteSleepIndTime"] = CanNmChannelConfig_dict["CanNmRemoteSleepIndTime"]["Value"]
                    CanNmChannelConfig["CanNmMsgCycleTime"] = CanNmChannelConfig_dict["CanNmMsgCycleTime"]["Value"]
                    if CanNmChannelConfig["CanNmRemoteSleepIndTime"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmRemoteSleepIndTime[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmRemoteSleepIndTime"])
                        return valid
                    if CanNmChannelConfig["CanNmMsgCycleTime"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmMsgCycleTime[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmMsgCycleTime"])
                        return valid
                    if CanNmChannelConfig["CanNmMsgCycleTime"] > CanNmChannelConfig["CanNmRemoteSleepIndTime"]:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmRemoteSleepIndTime[{}] should be greater than or equal to CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmMsgCycleTime[{}]!\n".format(
                        channelkey, CanNmChannelConfig["CanNmRemoteSleepIndTime"], channelkey,
                        CanNmChannelConfig["CanNmMsgCycleTime"])
                        return valid
        return valid

    def validate_cannm_repeat_message_time(self, cannm_cfg: dict) -> bool:
        valid = True
        CanNmChannelConfig = {}
        CanNmChannelConfig = dict.fromkeys(
          ["CanNmRepeatMessageTime", "CanNmMsgCycleTime", "CanNmImmediateNmTransmissions", "CanNmImmediateNmCycleTime"])
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                for channelkey, channelvalue in cannm_cfg[CanNmConfigKey].items():
                    if not isinstance(cannm_cfg[CanNmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in cannm_cfg[CanNmConfigKey]:
                        continue

                    if channelkey == 'DefinitionRef':
                        continue
                    if "DefinitionRef" not in channelvalue:
                        continue
                    if channelvalue["DefinitionRef"] != '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig':
                        continue

                    CanNmChannelConfig_dict = cannm_cfg[CanNmConfigKey][channelkey]
                    CanNmChannelConfig["CanNmRepeatMessageTime"] = CanNmChannelConfig_dict["CanNmRepeatMessageTime"]["Value"]
                    CanNmChannelConfig["CanNmMsgCycleTime"] = CanNmChannelConfig_dict["CanNmMsgCycleTime"]["Value"]
                    CanNmChannelConfig["CanNmImmediateNmTransmissions"] = CanNmChannelConfig_dict[
                    "CanNmImmediateNmTransmissions"]["Value"]
                    CanNmChannelConfig["CanNmImmediateNmCycleTime"] = CanNmChannelConfig_dict["CanNmImmediateNmCycleTime"][
                    "Value"]

                    if CanNmChannelConfig["CanNmRepeatMessageTime"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmRepeatMessageTime[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmRepeatMessageTime"])
                        return valid
                    if CanNmChannelConfig["CanNmMsgCycleTime"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmMsgCycleTime[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmMsgCycleTime"])
                        return valid
                    if CanNmChannelConfig["CanNmImmediateNmTransmissions"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmImmediateNmTransmissions[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmImmediateNmTransmissions"])
                        return valid
                    if CanNmChannelConfig["CanNmImmediateNmCycleTime"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmImmediateNmCycleTime[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmImmediateNmCycleTime"])
                        return valid
                    if CanNmChannelConfig["CanNmRepeatMessageTime"] % CanNmChannelConfig["CanNmMsgCycleTime"] != 0:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmRepeatMessageTime[{}] should be a multiple of CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmMsgCycleTime[{}]!\n".format(
                        channelkey, CanNmChannelConfig["CanNmRepeatMessageTime"], channelkey,
                        CanNmChannelConfig["CanNmMsgCycleTime"])
                        return valid
                    if CanNmChannelConfig["CanNmRepeatMessageTime"] <= CanNmChannelConfig[
                    "CanNmImmediateNmTransmissions"] * CanNmChannelConfig["CanNmImmediateNmCycleTime"]:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmRepeatMessageTime[{}] should be larger than CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmImmediateNmTransmissions[{}] * CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmImmediateNmCycleTime[{}]!\n".format(
                        channelkey, CanNmChannelConfig["CanNmRepeatMessageTime"], channelkey,
                        CanNmChannelConfig["CanNmImmediateNmTransmissions"], channelkey,
                        CanNmChannelConfig["CanNmImmediateNmCycleTime"])
                        return valid
        return valid

    def validate_cannm_timeout_time(self, cannm_cfg: dict) -> bool:
        valid = True
        CanNmChannelConfig = {}
        CanNmChannelConfig = dict.fromkeys(["CanNmMsgCycleTime", "CanNmTimeoutTime"])
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                for channelkey, channelvalue in cannm_cfg[CanNmConfigKey].items():
                    if not isinstance(cannm_cfg[CanNmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in cannm_cfg[CanNmConfigKey]:
                        continue

                    if channelkey == 'DefinitionRef':
                        continue
                    if "DefinitionRef" not in channelvalue:
                        continue
                    if channelvalue["DefinitionRef"] != '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig':
                        continue

                    CanNmChannelConfig_dict = cannm_cfg[CanNmConfigKey][channelkey]
                    CanNmChannelConfig["CanNmTimeoutTime"] = CanNmChannelConfig_dict["CanNmTimeoutTime"]["Value"]
                    CanNmChannelConfig["CanNmMsgCycleTime"] = CanNmChannelConfig_dict["CanNmMsgCycleTime"]["Value"]

                    if CanNmChannelConfig["CanNmTimeoutTime"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmTimeoutTime[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmTimeoutTime"])
                        return valid
                    if CanNmChannelConfig["CanNmMsgCycleTime"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmMsgCycleTime[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmMsgCycleTime"])
                        return valid
                    if CanNmChannelConfig["CanNmMsgCycleTime"] >= CanNmChannelConfig["CanNmTimeoutTime"]:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmTimeoutTime[{}] should be larger than CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmMsgCycleTime[{}]!\n".format(
                        channelkey, CanNmChannelConfig["CanNmTimeoutTime"], channelkey,
                        CanNmChannelConfig["CanNmMsgCycleTime"])
                        return valid
        return valid

    def validate_cannm_car_wake_up_byte_position(self, cannm_cfg: dict) -> bool:
        valid = True
        channel_max = 16
        CanNmChannelConfig = {}
        for channelindex in range(channel_max):
            CanNmChannelConfig = dict.fromkeys(
              ["CanNmCarWakeUpBytePosition", "CanNmPduCbvPosition", "CanNmPduNidPosition"])
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                for channelkey, channelvalue in cannm_cfg[CanNmConfigKey].items():
                    if not isinstance(cannm_cfg[CanNmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in cannm_cfg[CanNmConfigKey]:
                        continue

                    if channelkey == 'DefinitionRef':
                        continue
                    if "DefinitionRef" not in channelvalue:
                        continue
                    if channelvalue["DefinitionRef"] != '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig':
                        continue

                    CanNmChannelConfig_dict = cannm_cfg[CanNmConfigKey][channelkey]
                    CanNmChannelConfig["CanNmCarWakeUpBytePosition"] = CanNmChannelConfig_dict["CanNmCarWakeUpBytePosition"][
                    "Value"]
                    CanNmChannelConfig["CanNmPduCbvPosition"] = CanNmChannelConfig_dict["CanNmPduCbvPosition"]["Value"]
                    CanNmChannelConfig["CanNmPduNidPosition"] = CanNmChannelConfig_dict["CanNmPduNidPosition"]["Value"]
                    if CanNmChannelConfig["CanNmCarWakeUpBytePosition"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmCarWakeUpBytePosition[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmCarWakeUpBytePosition"])
                        return valid
                    if CanNmChannelConfig["CanNmPduCbvPosition"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmPduCbvPosition[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmPduCbvPosition"])
                        return valid
                    if CanNmChannelConfig["CanNmPduNidPosition"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmPduNidPosition[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmPduNidPosition"])
                        return valid
                    if CanNmChannelConfig["CanNmPduNidPosition"] == "CANNM_PDU_BYTE_1":
                        CanNmChannelConfig["CanNmPduNidPosition"] = 1
                    else:
                        CanNmChannelConfig["CanNmPduNidPosition"] = 0

                    if CanNmChannelConfig["CanNmPduCbvPosition"] == "CANNM_PDU_BYTE_1":
                        CanNmChannelConfig["CanNmPduCbvPosition"] = 1
                    else:
                        CanNmChannelConfig["CanNmPduCbvPosition"] = 0

                    if CanNmChannelConfig["CanNmCarWakeUpBytePosition"] < CanNmChannelConfig[
                    "CanNmPduCbvPosition"] + CanNmChannelConfig["CanNmPduNidPosition"]:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmCarWakeUpBytePosition[{}] should be larger than or equal to CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmPduNidPosition[{}] + CanNmPduCbvPosition[{}]!\n".format(
                        channelkey, CanNmChannelConfig["CanNmCarWakeUpBytePosition"], channelkey,
                        CanNmChannelConfig["CanNmPduNidPosition"], CanNmChannelConfig["CanNmPduCbvPosition"])
                        return valid
        return valid

    def validate_cannm_immediate_nm_transmissions(self, cannm_cfg: dict) -> bool:
        valid = True
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                CanNmImmediateRestartEnabled = cannm_cfg[CanNmConfigKey]["CanNmImmediateRestartEnabled"]["Value"]

                CanNmChannelConfig = {}
                CanNmChannelConfig = dict.fromkeys(["CanNmImmediateNmTransmissions", "CanNmPnHandleMultipleNetworkRequests"])

                for channelkey, channelvalue in cannm_cfg[CanNmConfigKey].items():
                    if not isinstance(cannm_cfg[CanNmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in cannm_cfg[CanNmConfigKey]:
                        continue

                    if channelkey == 'DefinitionRef':
                        continue
                    if "DefinitionRef" not in channelvalue:
                        continue
                    if channelvalue["DefinitionRef"] != '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig':
                        continue

                    CanNmChannelConfig_dict = cannm_cfg[CanNmConfigKey][channelkey]
                    CanNmChannelConfig["CanNmImmediateNmTransmissions"] = CanNmChannelConfig_dict[
                    "CanNmImmediateNmTransmissions"]["Value"]
                    CanNmChannelConfig["CanNmPnHandleMultipleNetworkRequests"] = CanNmChannelConfig_dict[
                    "CanNmPnHandleMultipleNetworkRequests"]["Value"]

                    if CanNmChannelConfig["CanNmImmediateNmTransmissions"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmImmediateNmTransmissions[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmImmediateNmTransmissions"])
                        return valid
                    if CanNmChannelConfig["CanNmPnHandleMultipleNetworkRequests"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmPnHandleMultipleNetworkRequests[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmPnHandleMultipleNetworkRequests"])
                        return valid
                    if True == CanNmImmediateRestartEnabled and CanNmChannelConfig["CanNmImmediateNmTransmissions"] != 0:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmImmediateNmTransmissions[{}] should be 0 if CanNm/CanNmGlobalConfig/CanNmImmediateRestartEnabled[{}] is set to True!\n".format(
                        channelkey, CanNmChannelConfig["CanNmImmediateNmTransmissions"], CanNmImmediateRestartEnabled)
                        return valid
                    if True == CanNmChannelConfig[
                    "CanNmPnHandleMultipleNetworkRequests"] and CanNmChannelConfig["CanNmImmediateNmTransmissions"] <= 0:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmImmediateNmTransmissions[{}] should be larger than 0 if CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmPnHandleMultipleNetworkRequests[{}] is set to True!\n".format(
                        channelkey, CanNmChannelConfig["CanNmImmediateNmTransmissions"], channelkey,
                        CanNmChannelConfig["CanNmPnHandleMultipleNetworkRequests"])
                        return valid
        return valid

    def validate_cannm_pdu_cbv_position(self, cannm_cfg: dict) -> bool:
        valid = True
        channel_max = 16
        CanNmChannelConfig = {}
        for channelindex in range(channel_max):
            CanNmChannelConfig = dict.fromkeys(
              ["CanNmNodeDetectionEnabled", "CanNmPduCbvPosition", "CanNmPduNidPosition"])
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                for channelkey, channelvalue in cannm_cfg[CanNmConfigKey].items():
                    if not isinstance(cannm_cfg[CanNmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in cannm_cfg[CanNmConfigKey]:
                        continue

                    if channelkey == 'DefinitionRef':
                        continue
                    if "DefinitionRef" not in channelvalue:
                        continue
                    if channelvalue["DefinitionRef"] != '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig':
                        continue

                    CanNmChannelConfig_dict = cannm_cfg[CanNmConfigKey][channelkey]
                    CanNmChannelConfig["CanNmNodeDetectionEnabled"] = CanNmChannelConfig_dict["CanNmNodeDetectionEnabled"][
                    "Value"]
                    CanNmChannelConfig["CanNmPduCbvPosition"] = CanNmChannelConfig_dict["CanNmPduCbvPosition"]["Value"]
                    CanNmChannelConfig["CanNmPduNidPosition"] = CanNmChannelConfig_dict["CanNmPduNidPosition"]["Value"]
                    if CanNmChannelConfig["CanNmNodeDetectionEnabled"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmNodeDetectionEnabled[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmNodeDetectionEnabled"])
                        return valid
                    if CanNmChannelConfig["CanNmPduCbvPosition"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmPduCbvPosition[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmPduCbvPosition"])
                        return valid
                    if CanNmChannelConfig["CanNmPduNidPosition"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmPduNidPosition[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmPduNidPosition"])
                        return valid
                    if True == CanNmChannelConfig["CanNmNodeDetectionEnabled"] and CanNmChannelConfig[
                    "CanNmPduCbvPosition"] == "CANNM_PDU_OFF":
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmPduCbvPosition[{}] should not be CANNM_PDU_OFF if CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmNodeDetectionEnabled[{}] is True!\n".format(
                        channelkey, CanNmChannelConfig["CanNmPduCbvPosition"], channelkey,
                        CanNmChannelConfig["CanNmNodeDetectionEnabled"])
                        return valid
                    if CanNmChannelConfig["CanNmPduCbvPosition"] != "CANNM_PDU_OFF" and CanNmChannelConfig[
                    "CanNmPduNidPosition"] != "CANNM_PDU_OFF" and CanNmChannelConfig[
                        "CanNmPduCbvPosition"] == CanNmChannelConfig["CanNmPduNidPosition"]:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmPduCbvPosition[{}] can not be the same as CanNmPduNidPosition[{}] when they are both not CANNM_PDU_OFF!\n".format(
                        channelkey, CanNmChannelConfig["CanNmPduCbvPosition"], CanNmChannelConfig["CanNmPduNidPosition"])
                        return valid
                    if CanNmChannelConfig["CanNmPduCbvPosition"] != "CANNM_PDU_OFF" and CanNmChannelConfig[
                    "CanNmPduNidPosition"] == "CANNM_PDU_OFF" and CanNmChannelConfig[
                        "CanNmPduCbvPosition"] != "CANNM_PDU_BYTE_0":
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmPduCbvPosition[{}] should be CANNM_PDU_BYTE_0 if not CANNM_PDU_OFF and CanNmPduNidPosition[{}] is CANNM_PDU_OFF!\n".format(
                        channelkey, CanNmChannelConfig["CanNmPduCbvPosition"], CanNmChannelConfig["CanNmPduNidPosition"])
                        return valid
        return valid

    def validate_cannm_pdu_nid_position(self, cannm_cfg: dict) -> bool:
        valid = True
        channel_max = 16
        CanNmChannelConfig = {}
        for channelindex in range(channel_max):
            CanNmChannelConfig = dict.fromkeys(
              ["CanNmNodeDetectionEnabled", "CanNmPduCbvPosition", "CanNmPduNidPosition"])
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                for channelkey, channelvalue in cannm_cfg[CanNmConfigKey].items():
                    if not isinstance(cannm_cfg[CanNmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in cannm_cfg[CanNmConfigKey]:
                        continue

                    if channelkey == 'DefinitionRef':
                        continue
                    if "DefinitionRef" not in channelvalue:
                        continue
                    if channelvalue["DefinitionRef"] != '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig':
                        continue

                    CanNmChannelConfig_dict = cannm_cfg[CanNmConfigKey][channelkey]
                    CanNmChannelConfig["CanNmNodeDetectionEnabled"] = CanNmChannelConfig_dict["CanNmNodeDetectionEnabled"][
                    "Value"]
                    CanNmChannelConfig["CanNmPduCbvPosition"] = CanNmChannelConfig_dict["CanNmPduCbvPosition"]["Value"]
                    CanNmChannelConfig["CanNmPduNidPosition"] = CanNmChannelConfig_dict["CanNmPduNidPosition"]["Value"]
                    if CanNmChannelConfig["CanNmNodeDetectionEnabled"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmNodeDetectionEnabled[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmNodeDetectionEnabled"])
                        return valid
                    if CanNmChannelConfig["CanNmPduCbvPosition"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmPduCbvPosition[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmPduCbvPosition"])
                        return valid
                    if CanNmChannelConfig["CanNmPduNidPosition"] is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmPduNidPosition[{}] should not be none!\n".format(
                        channelkey, CanNmChannelConfig["CanNmPduNidPosition"])
                        return valid
                    if True == CanNmChannelConfig["CanNmNodeDetectionEnabled"] and CanNmChannelConfig[
                    "CanNmPduNidPosition"] == "CANNM_PDU_OFF":
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmPduNidPosition[{}] should not be CANNM_PDU_OFF if CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmNodeDetectionEnabled[{}] is True!\n".format(
                        channelkey, CanNmChannelConfig["CanNmPduNidPosition"], channelkey,
                        CanNmChannelConfig["CanNmNodeDetectionEnabled"])
                        return valid
                    if CanNmChannelConfig["CanNmPduCbvPosition"] != "CANNM_PDU_OFF" and CanNmChannelConfig[
                    "CanNmPduNidPosition"] != "CANNM_PDU_OFF" and CanNmChannelConfig[
                        "CanNmPduCbvPosition"] == CanNmChannelConfig["CanNmPduNidPosition"]:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmPduNidPosition[{}] can not be the same as CanNmPduCbvPosition[{}] when they are both not CANNM_PDU_OFF!\n".format(
                        channelkey, CanNmChannelConfig["CanNmPduNidPosition"], CanNmChannelConfig["CanNmPduCbvPosition"])
                        return valid
                    if CanNmChannelConfig["CanNmPduNidPosition"] != "CANNM_PDU_OFF" and CanNmChannelConfig[
                    "CanNmPduCbvPosition"] == "CANNM_PDU_OFF" and CanNmChannelConfig[
                        "CanNmPduNidPosition"] != "CANNM_PDU_BYTE_0":
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmPduNidPosition[{}] should be CANNM_PDU_BYTE_0 if not CANNM_PDU_OFF and CanNmPduCbvPosition[{}] is CANNM_PDU_OFF!\n".format(
                        channelkey, CanNmChannelConfig["CanNmPduNidPosition"], CanNmChannelConfig["CanNmPduCbvPosition"])
                        return valid
        return valid

    def validate_cannm_comm_network_handle_ref(self, cannm_cfg: dict, comm_cfg: dict) -> bool:
        valid = True
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                for channelkey, channelvalue in cannm_cfg[CanNmConfigKey].items():
                    if not isinstance(cannm_cfg[CanNmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in cannm_cfg[CanNmConfigKey]:
                        continue

                    if channelkey == 'DefinitionRef':
                        continue
                    if "DefinitionRef" not in channelvalue:
                        continue
                    if channelvalue["DefinitionRef"] != '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig':
                        continue

                    samecount = 0
                    CanNmChannelConfig_dict = cannm_cfg[CanNmConfigKey][channelkey]
                    CanNmComMNetworkHandleRef = CanNmChannelConfig_dict["CanNmComMNetworkHandleRef"]["ValueRef"]
                    if CanNmComMNetworkHandleRef is None:
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/{}/CanNmComMNetworkHandleRef should not be empty!\n".format(
                        channelkey)
                        return valid
                    elif len(CanNmComMNetworkHandleRef) == 0 or CanNmComMNetworkHandleRef == "<empty>":
                        valid = False
                        self.log += "CanNm/CanNmGlobalConfig/{}/CanNmComMNetworkHandleRef should not be empty!\n".format(
                        channelkey)
                        return valid
                    else:
                        connectors_dict = {}
                        connectors_dict['channelkey'] = CanNmComMNetworkHandleRef.split('/')[-1]
                        for commchannelkey, commchannelvalue in comm_cfg["ComMConfigSet"].items():
                            if commchannelkey == connectors_dict['channelkey']:
                                samecount = 1
                        if samecount == 0:
                            valid = False
                            self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmComMNetworkHandleRef[{}] does not exist!\n".format(
                            channelkey, CanNmComMNetworkHandleRef)
                            return valid
        return valid

    def validate_cannm_pn_era_rxnsdu_ref(self, cannm_cfg: dict, ecuc_cfg: dict) -> bool:
        valid = True
        samecount = 0
        ecuc_pdu = ecuc_cfg["EcucConfigSet"]["EcucPduCollection"]
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                for channelkey, channelvalue in cannm_cfg[CanNmConfigKey].items():
                    if not isinstance(cannm_cfg[CanNmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in cannm_cfg[CanNmConfigKey]:
                        continue

                    if channelkey == 'DefinitionRef':
                        continue
                    if "DefinitionRef" not in channelvalue:
                        continue
                    if channelvalue["DefinitionRef"] != '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig':
                        continue

                    samecount = 0
                    CanNmChannelConfig_dict = cannm_cfg[CanNmConfigKey][channelkey]
                    CanNmPnEraRxNSduRef = CanNmChannelConfig_dict["CanNmPnEraRxNSduRef"]["ValueRef"]
                    if CanNmPnEraRxNSduRef is None:
                        # valid = False
                        self.log += "CanNm/CanNmGlobalConfig/{}/CanNmPnEraRxNSduRef should not be empty!\n".format(channelkey)
                        return valid
                    elif len(CanNmPnEraRxNSduRef) == 0 or CanNmPnEraRxNSduRef == "<empty>":
                        # valid = False
                        self.log += "CanNm/CanNmGlobalConfig/{}/CanNmPnEraRxNSduRef should not be empty!\n".format(channelkey)
                        return valid
                    else:
                        CanNmPnEraRxNSduRef = CanNmPnEraRxNSduRef.split('/')[-1]
                        for ecuckey, ecucvalue in ecuc_pdu.items():
                            if ecuckey == CanNmPnEraRxNSduRef:
                                samecount = 1
                        if samecount == 0:
                            valid = False
                            self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmPnEraRxNSduRef[{}] does not exist!\n".format(
                            channelkey, CanNmPnEraRxNSduRef)
                            return valid
        return valid

    def validate_cannm_rx_pdu_ref(self, cannm_cfg: dict, ecuc_cfg: dict) -> bool:
        valid = True
        samecount = 0
        ecuc_pdu = ecuc_cfg["EcucConfigSet"]["EcucPduCollection"]
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                for channelkey, channelvalue in cannm_cfg[CanNmConfigKey].items():
                    if not isinstance(cannm_cfg[CanNmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in cannm_cfg[CanNmConfigKey]:
                        continue

                    if channelkey == 'DefinitionRef':
                        continue
                    if "DefinitionRef" not in channelvalue:
                        continue
                    if channelvalue["DefinitionRef"] != '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig':
                        continue

                    samecount = 0
                    CanNmRxPdu_dict = cannm_cfg[CanNmConfigKey][channelkey]
                    for rxpdukey, rxpduvalue in CanNmRxPdu_dict.items():
                        if not isinstance(CanNmRxPdu_dict, dict):
                            continue
                        if (not rxpdukey.endswith('_Tx')):
                            continue
                        txuserdatapdu_temp = rxpdukey
                        txpdu_temp = rxpdukey.strip('_Tx')

                    for rxpdukey, rxpduvalue in CanNmRxPdu_dict.items():
                        if not isinstance(CanNmRxPdu_dict, dict):
                            continue
                        if rxpdukey in [
                        txuserdatapdu_temp, txpdu_temp, "DefinitionRef", "CanNmActiveWakeupBitEnabled",
                        "CanNmAllNmMessagesKeepAwake", "CanNmBusLoadReductionActive", "CanNmCarWakeUpFilterEnabled",
                        "CanNmCarWakeUpRxEnabled", "CanNmNodeDetectionEnabled", "CanNmNodeIdEnabled", "CanNmPnEnabled",
                        "CanNmPnEraCalcEnabled", "CanNmPnHandleMultipleNetworkRequests", "CanNmFirstPduFailedRetry", "CanNmRepeatMsgIndEnabled",
                        "CanNmImmediateNmCycleTime", "CanNmMsgCycleOffset", "CanNmMsgCycleTime", "CanNmMsgReducedTime",
                        "CanNmMsgTimeoutTime", "CanNmRemoteSleepIndTime", "CanNmRepeatMessageTime", "CanNmTimeoutTime",
                        "CanNmWaitBusSleepTime", "CanNmCarWakeUpBitPosition", "CanNmCarWakeUpBytePosition",
                        "CanNmCarWakeUpFilterNodeId", "CanNmImmediateNmTransmissions", "CanNmNodeId", "CanNmPduCbvPosition",
                        "CanNmPduNidPosition", "CanNmComMNetworkHandleRef", "CanNmPnEraRxNSduRef"
                        ]:
                            continue

                        samecount = 0
                        # print("CanNmRxPdu_dict,key:", rxpdukey)
                        CanNmRxPduRef = CanNmRxPdu_dict[rxpdukey]["CanNmRxPduRef"]["ValueRef"]
                        if CanNmRxPduRef is None:
                            valid = False
                            self.log += "CanNm/CanNmGlobalConfig/{}/{}/CanNmRxPduRef should not be empty!\n".format(
                            channelkey, rxpdukey)
                            return valid
                        elif len(CanNmRxPduRef) == 0 or CanNmRxPduRef == "<empty>":
                            valid = False
                            self.log += "CanNm/CanNmGlobalConfig/{}/{}/CanNmRxPduRef should not be empty!\n".format(
                            channelkey, rxpdukey)
                            return valid
                        else:
                            CanNmRxPduRef = CanNmRxPduRef.split('/')[-1]
                            for ecuckey, ecucvalue in ecuc_pdu.items():
                                if ecuckey == CanNmRxPduRef:
                                    samecount = 1
                            if samecount == 0:
                                valid = False
                                self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/CanNmRxPdus/{}/CanNmRxPduRef[{}] does not exist!\n".format(
                                channelkey, rxpdukey, CanNmRxPduRef)
                                return valid
        return valid

    def validate_cannm_tx_pdu_ref(self, cannm_cfg: dict, ecuc_cfg: dict) -> bool:
        valid = True
        samecount = 0
        ecuc_pdu = ecuc_cfg["EcucConfigSet"]["EcucPduCollection"]
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                for channelkey, channelvalue in cannm_cfg[CanNmConfigKey].items():
                    if not isinstance(cannm_cfg[CanNmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in cannm_cfg[CanNmConfigKey]:
                        continue

                    if channelkey == 'DefinitionRef':
                        continue
                    if "DefinitionRef" not in channelvalue:
                        continue
                    if channelvalue["DefinitionRef"] != '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig':
                        continue

                    samecount = 0
                    CanNmRxPdu_dict = cannm_cfg[CanNmConfigKey][channelkey]
                    for rxpdukey, rxpduvalue in CanNmRxPdu_dict.items():
                        if not isinstance(CanNmRxPdu_dict, dict):
                            continue
                        if (not rxpdukey.endswith('_Tx')):
                            continue
                        txpdu_temp = rxpdukey.strip('_Tx')
                        samecount = 0
                        # print("CanNmRxPdu_dict,key:", rxpdukey)
                        CanNmTxPduRef = CanNmRxPdu_dict[txpdu_temp]["CanNmTxPduRef"]["ValueRef"]
                        if CanNmTxPduRef is None:
                            valid = False
                            self.log += "CanNm/CanNmGlobalConfig/{}/{}/CanNmTxPduRef should not be empty!\n".format(
                            channelkey, txpdu_temp)
                            return valid
                        elif len(CanNmTxPduRef) == 0 or CanNmTxPduRef == "<empty>":
                            valid = False
                            self.log += "CanNm/CanNmGlobalConfig/{}/{}/CanNmTxPduRef should not be empty!\n".format(
                            channelkey, txpdu_temp)
                            return valid
                        else:
                            CanNmTxPduRef = CanNmTxPduRef.split('/')[-1]
                            for ecuckey, ecucvalue in ecuc_pdu.items():
                                if ecuckey == CanNmTxPduRef:
                                    samecount = 1
                            if samecount == 0:
                                valid = False
                                self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/{}/CanNmTxPduRef[{}] does not exist!\n".format(
                                channelkey, txpdu_temp, CanNmTxPduRef)
                                return valid
        return valid

    def validate_cannm_tx_user_data_pdu_ref(self, cannm_cfg: dict, ecuc_cfg: dict) -> bool:
        valid = True
        samecount = 0
        ecuc_pdu = ecuc_cfg["EcucConfigSet"]["EcucPduCollection"]
        for CanNmConfigKey, CanNmConfigValue in cannm_cfg.items():
            if CanNmConfigKey == "DefinitionRef":
                continue
            if "DefinitionRef" not in CanNmConfigValue:
                continue
            if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
                for channelkey, channelvalue in cannm_cfg[CanNmConfigKey].items():
                    if not isinstance(cannm_cfg[CanNmConfigKey], dict):
                        continue
                    if "DefinitionRef" not in cannm_cfg[CanNmConfigKey]:
                        continue

                    if channelkey == 'DefinitionRef':
                        continue
                    if "DefinitionRef" not in channelvalue:
                        continue
                    if channelvalue["DefinitionRef"] != '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig':
                        continue

                    samecount = 0
                    CanNmRxPdu_dict = cannm_cfg[CanNmConfigKey][channelkey]
                    for rxpdukey, rxpduvalue in CanNmRxPdu_dict.items():
                        if not isinstance(CanNmRxPdu_dict, dict):
                            continue
                        if (not rxpdukey.endswith('_Tx')):
                            continue

                        samecount = 0
                        # print("CanNmRxPdu_dict,key:", rxpdukey)
                        CanNmTxUserDataPduRef = CanNmRxPdu_dict[rxpdukey]["CanNmTxUserDataPduRef"]["ValueRef"]
                        if CanNmTxUserDataPduRef is None:
                            valid = False
                            self.log += "CanNm/CanNmGlobalConfig/{}/{}/CanNmTxUserDataPduRef should not be empty!\n".format(
                            channelkey, rxpdukey)
                            return valid
                        elif len(CanNmTxUserDataPduRef) == 0 or CanNmTxUserDataPduRef == "<empty>":
                            valid = False
                            self.log += "CanNm/CanNmGlobalConfig/{}/{}/CanNmTxUserDataPduRef should not be empty!\n".format(
                            channelkey, rxpdukey)
                            return valid
                        else:
                            CanNmTxUserDataPduRef = CanNmTxUserDataPduRef.split('/')[-1]
                            for ecuckey, ecucvalue in ecuc_pdu.items():
                                if ecuckey == CanNmTxUserDataPduRef:
                                    samecount = 1
                            if samecount == 0:
                                valid = False
                                self.log += "CanNm/CanNmGlobalConfig/CanNmChannelConfigs/{}/{}/CanNmTxUserDataPduRef[{}] does not exist!\n".format(
                                channelkey, rxpdukey, CanNmTxUserDataPduRef)
                                return valid
        return valid

    def validate(self, ecu_cfg):
        valid = True
        self.log = ""
        self.ecu_cfg = ecu_cfg
        cannm_cfg = self.ecu_cfg["ActiveEcuC"]["CanNm"]
        nm_cfg = self.ecu_cfg["ActiveEcuC"]["Nm"]
        ecuc_cfg = self.ecu_cfg["ActiveEcuC"]["EcuC"]
        comm_cfg = self.ecu_cfg["ActiveEcuC"]["ComM"]

        valid = self.validate_cannm_bus_load_reduction_enabled(cannm_cfg) and valid
        valid = self.validate_cannm_bus_synchronization_enabled(cannm_cfg, nm_cfg) and valid
        valid = self.validate_cannm_com_control_enabled(cannm_cfg, nm_cfg) and valid
        valid = self.validate_cannm_com_user_data_support(cannm_cfg) and valid
        valid = self.validate_cannm_coordinator_sync_support(cannm_cfg) and valid
        valid = self.validate_cannm_immediate_txconf_enabled(cannm_cfg) and valid
        valid = self.validate_cannm_pdu_rx_indication_enabled(cannm_cfg, nm_cfg) and valid
        valid = self.validate_cannm_pn_eira_calc_enabled(cannm_cfg) and valid
        valid = self.validate_cannm_remote_sleep_ind_enabled(cannm_cfg, nm_cfg) and valid
        valid = self.validate_cannm_state_change_ind_enabled(cannm_cfg, nm_cfg) and valid
        valid = self.validate_cannm_user_data_enabled(cannm_cfg, nm_cfg) and valid
        valid = self.validate_cannm_pn_reset_time(cannm_cfg) and valid
        valid = self.validate_cannm_pn_eira_rxnsdu_ref(cannm_cfg, ecuc_cfg) and valid
        valid = self.validate_cannm_bus_load_reduction_active(cannm_cfg) and valid
        valid = self.validate_cannm_car_wake_up_filter_enabled(cannm_cfg) and valid
        valid = self.validate_cannm_node_detection_enabled(cannm_cfg) and valid
        valid = self.validate_cannm_pn_enabled(cannm_cfg) and valid
        valid = self.validate_cannm_pn_era_calc_enabled(cannm_cfg) and valid
        valid = self.validate_cannm_pn_handle_multiple_network_requests(cannm_cfg) and valid
        valid = self.validate_cannm_immediate_nm_cycle_time(cannm_cfg) and valid
        valid = self.validate_cannm_msg_cycle_offset(cannm_cfg) and valid
        valid = self.validate_cannm_msg_reduced_time(cannm_cfg) and valid
        valid = self.validate_cannm_msg_timeout_time(cannm_cfg) and valid
        valid = self.validate_cannm_remote_sleep_ind_time(cannm_cfg) and valid
        valid = self.validate_cannm_repeat_message_time(cannm_cfg) and valid
        valid = self.validate_cannm_timeout_time(cannm_cfg) and valid
        valid = self.validate_cannm_car_wake_up_byte_position(cannm_cfg) and valid
        valid = self.validate_cannm_immediate_nm_transmissions(cannm_cfg) and valid
        valid = self.validate_cannm_pdu_cbv_position(cannm_cfg) and valid
        valid = self.validate_cannm_pdu_nid_position(cannm_cfg) and valid
        valid = self.validate_cannm_comm_network_handle_ref(cannm_cfg, comm_cfg) and valid
        valid = self.validate_cannm_pn_era_rxnsdu_ref(cannm_cfg, ecuc_cfg) and valid
        valid = self.validate_cannm_rx_pdu_ref(cannm_cfg, ecuc_cfg) and valid
        valid = self.validate_cannm_tx_pdu_ref(cannm_cfg, ecuc_cfg) and valid
        valid = self.validate_cannm_tx_user_data_pdu_ref(cannm_cfg, ecuc_cfg) and valid

        return valid
