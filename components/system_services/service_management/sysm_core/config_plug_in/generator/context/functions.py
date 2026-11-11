#
# Copyright (C) 2015-2022 Lixiang Group Holding Limited
#
"""
BswM模板内函数定义模块.
定义jinja2模板中所使用的函数.
"""

import copy
from collections import deque
from basic_func_module.autosar_utils.autosar_util import AutosarUtil
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
    def get_dict_by_ref_path_base(config_dict: dict, ref_path: str) -> dict:
        res = AutosarUtil.get_by_path_from_dict(config_dict, ref_path)
        return res

    @staticmethod
    def get_condition(reqidx: str, orperator: str, conditionright: str, usercondion: bool) -> str:
        if usercondion:
            condition_return = reqidx + "\x20" + orperator + "\x20" + conditionright
        else:
            conditionright = "(uint32)" + conditionright
            condition_return = "bswm_mode_req_mirrors[BSWM_" + reqidx + "_REQ_IDX]" + "\x20" + orperator + "\x20" + conditionright
        return condition_return

    @staticmethod
    def get_key_name(key_name: str) -> str:
        if key_name is not None:
            key_name = key_name.split("/")[-1]
        else:
            key_name = None
        return key_name

    @staticmethod
    def float_to_int(number: float) -> int:
        number = int(number)
        return number

    @staticmethod
    def int_to_str(number: int) -> str:
        number = str(number)
        return number

    @staticmethod
    def get_headers(marco_list: list) -> list:
        res = []
        if "Com" in marco_list:
            res.append("com.h")
        if "Dem" in marco_list:
            res.append("dem.h")
        return res

    @staticmethod
    def get_module_marcos(marco_list: list) -> list:
        res = []
        if "Dlt" in marco_list:
            res.append("BSWM_USE_DLT")
        if "ComM" in marco_list:
            res.append("BSWM_USE_COMM")
        if "Nm" in marco_list:
            res.append("BSWM_USE_NM")
        if "CanSM" in marco_list:
            res.append("BSWM_USE_CANSM")
        if "LinSM" in marco_list:
            res.append("BSWM_USE_LINSM")
        if "EthSM" in marco_list:
            res.append("BSWM_USE_ETHSM")
        if "Dcm" in marco_list:
            res.append("BSWM_USE_DCM")
        if "NvM" in marco_list:
            res.append("BSWM_USE_NVM")
        return res

    @staticmethod
    def gen_switch_macro(key_value: str) -> str:
        if key_value == False:
            res = "STD_OFF"
        else:
            res = "STD_ON"
        return res

    @staticmethod
    def get_boolean_value(key_value: str) -> str:
        if key_value == False:
            res = "FALSE"
        else:
            res = "TRUE"
        return res

    @staticmethod
    def str_to_list(key_value: str) -> list:
        res = []
        if isinstance(key_value, str):
            res.append(key_value)
        else:
            res = key_value
        return res

    @staticmethod
    def get_init_list(list_ref: str) -> str:
        list_name = Function.get_key_name(list_ref)

        if list_name == "EcuMDriverInitListTwo":
            res = "ecum_al_driver_init_two(ecum_world.config)"
        elif list_name == "EcuMDriverInitListThree":
            res = "ecum_al_driver_init_three(ecum_world.config)"
        else:
            res = list_name

        return res

    @staticmethod
    def check_none(config_dict: dict, key: str) -> dict:
        arbitration_dict = config_dict["BswMConfig"]["BswMArbitration"]
        modeControl_dict = config_dict["BswMConfig"]["BswMModeControl"]

        condition1 = key in arbitration_dict
        condition2 = key in modeControl_dict

        if condition1:
            return arbitration_dict[key]
        elif condition2:
            return modeControl_dict[key]
        else:
            return False

    @staticmethod
    def judge_usercallout(config_dict: dict) -> bool:
        res = False
        actions = Function.check_none(config_dict, "BswMActions")
        if actions:
            for action_name, action_value in list(actions.items()):
                if action_name != "Tag":
                    if "BswMUserCallout" in action_value["BswMAvailableActions"]:
                        res = True
                        break
        return res

    @staticmethod
    def get_reqinitvalue(request_dict: dict) -> dict:
        res = {}
        for req_name, req_value in list(request_dict.items()):
            if req_name != "Tag":
                req_str = "[BSWM_" + req_name.upper() + "_REQ_IDX]"
                value_str = "BSWM_UNDEFINED_REQUEST_VALUE"
                if "BswMModeInitValue" in req_value:
                    init_value = req_value["BswMModeInitValue"]["BswMBswModeInitValue"]["Value"]
                    if init_value is not None and init_value != "":
                        value_str = init_value
                res[req_str] = value_str
        return res

    @staticmethod
    def search_requests(config_dict: dict, ref_paths: list) -> list:
        request_names = []
        request_values = []
        queue = deque()
        for ref in ref_paths:
            queue.append(ref)
        while queue:
            for i in range(len(queue)):
                ref = queue.popleft()
                condition_value = Function.get_dict_by_ref_path(config_dict, ref)
                condition_type = Function.get_key_name(condition_value["DefinitionRef"])
                if condition_type == "BswMModeCondition":
                    request_name = Function.get_key_name(condition_value["BswMConditionMode"]["ValueRef"])
                    request_value = Function.get_dict_by_ref_path(config_dict,
                                                                  condition_value["BswMConditionMode"]["ValueRef"])
                    if request_name not in request_names:
                        request_names.append(request_name)
                        request_values.append(request_value)
                elif condition_type == "BswMLogicalExpression":
                    condition_ref = Function.str_to_list(condition_value["BswMArgumentRef"]["ValueRef"])
                    for ref in condition_ref:
                        queue.append(ref)
        return request_values

    @staticmethod
    def gen_conditions(config_dict: dict, conditions: list) -> list:
        res = []
        for i in range(len(conditions)):
            usercondition = False
            condition_name = Function.get_key_name(conditions[i])
            condition_value = Function.get_dict_by_ref_path(config_dict, conditions[i])
            if Function.get_key_name(condition_value["DefinitionRef"]) == "BswMModeCondition":
                req_name = Function.get_key_name(condition_value["BswMConditionMode"]["ValueRef"])
                req_value = Function.get_dict_by_ref_path(config_dict, condition_value["BswMConditionMode"]["ValueRef"])
                if "BswMUserConditionRequest" in req_value["BswMModeRequestSource"]:
                    req_to_condition = req_value["BswMModeRequestSource"]["BswMUserConditionRequest"]["User Condition"][
                        "Value"]
                    usercondition = True
                elif len(req_value["BswMModeRequestSource"]) > 1:
                    timer = Function.get_reqsource_def(req_value["BswMModeRequestSource"])
                    if timer != "False":
                        req_to_condition = "bswm_get_timer_state(" + timer +")"
                        usercondition = True
                    else:
                        req_to_condition = req_name.upper()
                else:
                    req_to_condition = req_name.upper()

                if condition_value["BswMConditionType"]["Value"] == "BSWM_EQUALS":
                    condition = Function.get_condition(
                        req_to_condition, "==",
                        condition_value["BswMConditionValue"]["BswMBswMode"]["BswMBswRequestedMode"]["Value"],
                        usercondition)
                else:
                    condition = Function.get_condition(
                        req_to_condition, "!=",
                        condition_value["BswMConditionValue"]["BswMBswMode"]["BswMBswRequestedMode"]["Value"],
                        usercondition)
                res.append(condition)
            else:  # condition is expression
                condition = condition_name + "()"
                res.append(condition)
        return res

    @staticmethod
    def gen_expression(condition_list: list, operator: str) -> str:
        res = ""

        if len(condition_list) == 1 and operator != "BSWM_NOT":
            res = "(" + condition_list[0] + ")"
            return res

        if operator == "BSWM_NOT":
            res = "!(" + condition_list[0] + ")"
        elif operator == "BSWM_NAND":
            res = "!((" + condition_list[0] + ")" + "\x20" + "&&\n" + 14 * "\x20" + "(" + condition_list[1] + "))"
        elif operator == "BSWM_AND":
            for condition in condition_list:
                res += "(" + condition + ")" + "\x20" + "&&\n" + 13 * "\x20"
            res = res[:-17]
        elif operator == "BSWM_OR":
            for condition in condition_list:
                res += "(" + condition + ")" + "\x20" + "||\n" + 13 * "\x20"
            res = res[:-17]
        else:
            for condition in condition_list:
                res += "(" + condition + ")" + "\x20" + "^\n" + 13 * "\x20"
            res = res[:-16]
        return res

    @staticmethod
    def get_actions_num(config_dict: dict) -> dict:
        res = [0, 0,0,0]
        actions = Function.check_none(config_dict, "BswMActions")
        if actions:
            for action_name, action_value in list(actions.items()):
                if action_name != "Tag":
                    if "BswMPduGroupSwitch" in action_value["BswMAvailableActions"]:
                        res[0] += 1
                    elif "BswMDeadlineMonitoringControl" in action_value["BswMAvailableActions"]:
                        res[1] += 1
                    elif "BswMPduRouterControl" in action_value["BswMAvailableActions"]:
                        res[2] += 1
                    elif "BswMNMControl" in action_value["BswMAvailableActions"]:
                        res[3] += 1                                                
                    else:
                        continue
        return res

    @staticmethod
    def get_reqsource_def(source_dict: dict) -> str:
        for source_name, source_value in list(source_dict.items()):
            if source_name != "DefinitionRef":
                if Function.get_key_name(source_value["DefinitionRef"]) == "BswMTimer":
                    return source_name
                else:
                    return "False"

    @staticmethod
    def gen_timers(config_dict: dict) -> list:
        res = []
        requests = Function.check_none(config_dict, "BswMModeRequestPorts")
        if requests:
            for req_name, req_value in list(requests.items()):
                if req_name != "Tag":
                    if len(req_value["BswMModeRequestSource"]) > 1:
                        timer = Function.get_reqsource_def(req_value["BswMModeRequestSource"])
                        if timer not in res and timer != "False":
                            res.append(timer)
        return res

    @staticmethod
    def get_pdugroup(actionname: str, disablepdugroup: str, enablepdugroup: str) -> list:
        res = []
        if disablepdugroup is not None:
            group1 = Function.str_to_list(disablepdugroup)
            if group1[0] == "<empty>":
                group1 = []
        else:
            group1 = []

        if enablepdugroup is not None:
            group2 = Function.str_to_list(enablepdugroup)
            if group2[0] == "<empty>":
                group2 = []
        else:
            group2 = []

        if len(group1) > 0:
            res1 = "&BswM_" + actionname + "_DisabledGroup[0]"
        else:
            res1 = "NULL"
        if len(group2) > 0:
            res2 = "&BswM_" + actionname + "_EnabledGroup[0]"
        else:
            res2 = "NULL"
        res.append(res1)
        res.append(res2)
        return res

    @staticmethod
    def gen_pdugroup(config_dict: dict) -> dict:
        res = {}
        actions = Function.check_none(config_dict, "BswMActions")
        if actions:
            for action_name, action_value in list(actions.items()):
                if action_name != "Tag":
                    if "BswMPduGroupSwitch" in action_value["BswMAvailableActions"]:
                        pdugroup = Function.get_pdugroup(
                            action_name, action_value["BswMAvailableActions"]["BswMPduGroupSwitch"]
                            ["BswMDisabledPduGroupRef"]["ValueRef"], action_value["BswMAvailableActions"]
                            ["BswMPduGroupSwitch"]["BswMEnabledPduGroupRef"]["ValueRef"])
                        if pdugroup[0] != "NULL":
                            disable_groups = Function.str_to_list(
                                action_value["BswMAvailableActions"]["BswMPduGroupSwitch"]["BswMDisabledPduGroupRef"]
                                ["ValueRef"])
                            res[pdugroup[0]] = disable_groups
                        if pdugroup[1] != "NULL":
                            enable_groups = Function.str_to_list(
                                action_value["BswMAvailableActions"]["BswMPduGroupSwitch"]["BswMEnabledPduGroupRef"]
                                ["ValueRef"])
                            res[pdugroup[1]] = enable_groups
                    elif "BswMDeadlineMonitoringControl" in action_value["BswMAvailableActions"]:
                        pdugroup = Function.get_pdugroup(
                            action_name, action_value["BswMAvailableActions"]["BswMDeadlineMonitoringControl"]
                            ["BswMDisabledDMPduGroupRef"]["ValueRef"], action_value["BswMAvailableActions"]
                            ["BswMDeadlineMonitoringControl"]["BswMEnabledDMPduGroupRef"]["ValueRef"])
                        if pdugroup[0] != "NULL":
                            disable_groups = Function.str_to_list(
                                action_value["BswMAvailableActions"]["BswMDeadlineMonitoringControl"]
                                ["BswMDisabledDMPduGroupRef"]["ValueRef"])
                            res[pdugroup[0]] = disable_groups
                        if pdugroup[1] != "NULL":
                            enable_groups = Function.str_to_list(
                                action_value["BswMAvailableActions"]["BswMDeadlineMonitoringControl"]
                                ["BswMEnabledDMPduGroupRef"]["ValueRef"])
                            res[pdugroup[1]] = enable_groups
                    else:
                        continue
        return res

    @staticmethod
    def gen_pdur_pathgroup(config_bswm: dict, config_pdur: dict) -> dict:
        res = {}
        actions = Function.check_none(config_bswm, "BswMActions")
        if actions:
            for action_name, action_value in list(actions.items()):
                if action_name != "Tag":
                    if "BswMPduRouterControl" in action_value["BswMAvailableActions"]:
                        pdur_pathgroupref = Function.str_to_list(
                            action_value["BswMAvailableActions"]["BswMPduRouterControl"]["BswMPduRoutingPathGroupRef"]
                            ["ValueRef"])
                        pathgroupid = []
                        if pdur_pathgroupref[0] == "<empty>":
                            pathgroupid.append(-1)
                        else:
                            for ref in pdur_pathgroupref:
                                pathgroup = Function.get_dict_by_ref_path_base(config_pdur, ref)
                                pathgroupid.append(pathgroup["PduRRoutingPathGroupId"]["Value"])
                        res["BswM_" + action_name[:-6] + "_PathGroup"] = pathgroupid
        return res

    @staticmethod
    def get_linchannel(config_dict: dict, lin_schedule: str) -> str:
        res = ""
        linsm_channel = lin_schedule.split("/")[-2]
        comm_lin_channel = Function.get_key_name(
            config_dict["LinSMConfigSet"][linsm_channel]["LinSMComMNetworkHandleRef"]["ValueRef"])
        res = "ComMConf_ComMChannel_" + comm_lin_channel

        return res

    @staticmethod
    def get_sleepmode(action_dict: dict) -> list:
        res = []

        shutdown_target = action_dict["BswMEcuMShutdownTarget"]["Value"][5:]
        res.append(shutdown_target)
        reset_mode = Function.get_key_name(action_dict["BswMEcuMResetModeRef"]["ValueRef"])
        sleep_mode = Function.get_key_name(action_dict["BswMEcuMSleepModeRef"]["ValueRef"])

        if shutdown_target == "ECUM_SHUTDOWN_TARGET_RESET":
            res.append("Conf_" + reset_mode)
        elif shutdown_target == "ECUM_SHUTDOWN_TARGET_SLEEP":
            res.append(sleep_mode)
        else:
            res.append("0u")
        return res

    @staticmethod
    def get_actionlist_idx(actionlist_name: str) -> str:
        res = ""
        if actionlist_name is not None and actionlist_name != "<empty>":
            res = "BSWM_" + actionlist_name.upper() + "_LIST_IDX"
        else:
            res = "-1"
        return res

    @staticmethod
    def gen_atomic_actions(available_actions: dict, dict_bsw: dict) -> list:
        res = ""
        if "BswMUserCallout" in available_actions:
            res = available_actions["BswMUserCallout"]["BswMUserCalloutFunction"]["Value"]
        elif "BswMLinScheduleSwitch" in available_actions:
            linchannel = Function.get_linchannel(
                dict_bsw["dict_linsm"]["ActiveEcuC"]["LinSM"],
                available_actions["BswMLinScheduleSwitch"]["BswMLinScheduleRef"]["ValueRef"])
            res = "(void)LinSM_ScheduleRequest(" + linchannel + ", item->LinScheduleRef)"
        elif "BswMPduGroupSwitch" in available_actions:
            res = "bswm_internal_pdu_group_switch_action(item)"
        elif "BswMDeadlineMonitoringControl" in available_actions:
            res = "bswm_internal_dm_control_action(item)"
        elif "BswMTimerControl" in available_actions:
            res = "bswm_update_timer(item)"
        elif "BswMPduRouterControl" in available_actions:
            res = "bswm_internal_pdu_router_control_action(item)"
        elif "BswMNMControl" in available_actions:
            nm_channel = Function.get_key_name(
                available_actions["BswMNMControl"]["BswMComMNetworkHandleRef"]["ValueRef"])
            res = "bswm_nm_control(item, ComMConf_ComMChannel_" + nm_channel + ")"
        elif "BswMComMAllowCom" in available_actions:
            comm_channel = Function.get_key_name(
                available_actions["BswMComMAllowCom"]["BswMComMAllowChannelRef"]["ValueRef"])
            if_allowed = Function.get_boolean_value(available_actions["BswMComMAllowCom"]["BswMComAllowed"]["Value"])
            res = "ComM_CommunicationAllowed((NetworkHandleType)ComMConf_ComMChannel_" + comm_channel + ", " + '(boolean)'+ if_allowed + ")"
        elif "BswMComMModeLimitation" in available_actions:
            comm_channel = Function.get_key_name(
                available_actions["BswMComMModeLimitation"]["BswMComMLimitChannelRef"]["ValueRef"])
            if_limit = Function.get_boolean_value(
                available_actions["BswMComMModeLimitation"]["BswMComMLimitMode"]["Value"])
            res = "(void)ComM_LimitChannelToNoComMode((NetworkHandleType)ComMConf_ComMChannel_" + comm_channel + ", " + '(boolean)' + if_limit + ")"
        elif "BswMComMModeSwitch" in available_actions:
            comm_user = Function.get_key_name(available_actions["BswMComMModeSwitch"]["BswMComMUserRef"]["ValueRef"])
            comm_mode = available_actions["BswMComMModeSwitch"]["BswMComMRequestedMode"]["Value"][5:]
            res = "(void)ComM_RequestComMode(ComMConf_ComMUser_" + comm_user + ", " + comm_mode + ")"
        elif "BswMEcuMGoDownHaltPoll" in available_actions:
            ecum_user = Function.get_key_name(
                available_actions["BswMEcuMGoDownHaltPoll"]["BswMEcuMUserIdRef"]["ValueRef"])
            res = "(void)ecum_go_down(ECUM_" + ecum_user + ")"
        elif "BswMEcuMSelectShutdownTarget" in available_actions:
            sleep_mode = Function.get_sleepmode(available_actions["BswMEcuMSelectShutdownTarget"])
            res = "(void)ecum_select_shutdown_target(" + sleep_mode[0] + ", " + sleep_mode[1] + ")"
        elif "BswMEcuMDriverInitListBswM" in available_actions:
            res = Function.get_init_list(available_actions["BswMEcuMDriverInitListBswM"]["BswMEcuMDriverInitListBswMRef"]["ValueRef"])
        elif "BswMRteSwitch" in available_actions:
            rte_mode = available_actions["BswMRteSwitch"]["BswMSwitchedMode"]["ValueRef"].split("/")[-2]
            return_mode = available_actions["BswMRteSwitch"]["BswMSwitchedMode"]["ValueRef"].split("/")[-1]
            rte_port = available_actions["BswMRteSwitch"]["BswMRteSwitchPortRef"]["ValueRef"].split("/")[-1]
            res = "BswM_Switch_"+rte_port+"_"+rte_mode +" = "+return_mode+";"+ \
            f'(void)VCOS_LOG(BSWM_MODULE_ID, VCOS_LOG_INFO, "BswM_Switch occur{rte_port}.\\n");'
        else:
            res = "(void)*item"
        return res

    @staticmethod
    def get_ruledata(config_dict: dict) -> dict:
        res = {}
        deferred_rules = []
        immediate_rules = {}
        immediate_rulesOninit_temp = []
        dcm = {}
        ecum_curstate = {}
        ecum_wakeup = {}
        ecum_reqstate = {}
        comm = {}
        comm_pnc = {}
        nm = {}
        cansm = {}
        linsm = {}
        ethsm = {}
        linsch = {}
        lintp = {}
        nvm_job = {}
        nvm_req = {}
        generic = {}
        extra = {}
        rules = Function.check_none(config_dict, "BswMRules")
        if rules:
            for rule_name, rule_value in list(rules.items()):
                if rule_name != "Tag":
                    if rule_value["BswMNestedExecutionOnly"]["Value"] == True:
                        continue
                    else:
                        exp_value = Function.get_dict_by_ref_path(config_dict,
                                                                  rule_value["BswMRuleExpressionRef"]["ValueRef"])
                        condition_ref = Function.str_to_list(exp_value["BswMArgumentRef"]["ValueRef"])
                        request_values = Function.search_requests(config_dict, condition_ref)

                        rule_idx = "BSWM_" + rule_name.upper() + "_RULE_IDX"
                        for request_value in request_values:
                            if request_value["BswMRequestProcessing"]["Value"] == "BSWM_DEFERRED":
                                if rule_idx not in deferred_rules:
                                    deferred_rules.append(rule_idx)
                            elif request_value["BswMRequestProcessing"]["Value"] == "BSWM_IMMEDIATE":
                                try:
                                    if request_value["BswMArbitrateOnInit"]["Value"] == True or request_value["BswMArbitrateOnInit"]["Value"] == None:
                                        immediate_rulesOninit_temp.append(rule_idx)
                                except:
                                    # 找不到的 也统一添加到init中
                                    immediate_rulesOninit_temp.append(rule_idx)
                                if "BswMDcmComModeRequest" in request_value["BswMModeRequestSource"]:
                                    channel = Function.get_key_name(request_value["BswMModeRequestSource"]["BswMDcmComModeRequest"]["BswMDcmComMChannelRef"]["ValueRef"])
                                    if channel in dcm:
                                        dcm[channel].append(rule_idx)
                                    else:
                                        dcm[channel] = [rule_idx]
                                elif "BswMEcuMIndication" in request_value["BswMModeRequestSource"]:
                                    if "ecum" in ecum_curstate:
                                        ecum_curstate["ecum"].append(rule_idx)
                                    else:
                                        ecum_curstate["ecum"] = [rule_idx]
                                elif "BswMEcuMWakeupSource" in request_value["BswMModeRequestSource"]:
                                    source = Function.get_key_name(request_value["BswMModeRequestSource"]["BswMEcuMWakeupSource"]["BswMEcuMWakeupSrcRef"]["ValueRef"])
                                    if source in ecum_wakeup:
                                        ecum_wakeup[source].append(rule_idx)
                                    else:
                                        ecum_wakeup[source] = [rule_idx]
                                elif "BswMEcuMRUNRequestIndication" in request_value["BswMModeRequestSource"]:
                                    run_req = request_value["BswMModeRequestSource"]["BswMEcuMRUNRequestIndication"]["BswMEcuMRUNRequestProtocolPort"]["Value"][5:]
                                    if run_req in ecum_reqstate:
                                        ecum_reqstate[run_req].append(rule_idx)
                                    else:
                                        ecum_reqstate[run_req] = [rule_idx]
                                elif "BswMComMIndication" in request_value["BswMModeRequestSource"]:
                                    channel = Function.get_key_name(request_value["BswMModeRequestSource"]["BswMComMIndication"]["BswMComMChannelRef"]["ValueRef"])
                                    if channel in comm:
                                        comm[channel].append(rule_idx)
                                    else:
                                        comm[channel] = [rule_idx]
                                elif "BswMComMPncRequest" in request_value["BswMModeRequestSource"]:
                                    pnc = Function.get_key_name(request_value["BswMModeRequestSource"]["BswMComMPncRequest"]["BswMComMPncRef"]["ValueRef"])
                                    if pnc in comm_pnc:
                                        comm_pnc[pnc].append(rule_idx)
                                    else:
                                        comm_pnc[pnc] = [rule_idx]
                                elif "BswMNmStateChangeNotification" in request_value["BswMModeRequestSource"]:
                                    channel = Function.get_key_name(request_value["BswMModeRequestSource"]["BswMNmStateChangeNotification"]["BswMNmChannelRef"]["ValueRef"])
                                    if channel in nm:
                                        nm[channel].append(rule_idx)
                                    else:
                                        nm[channel] = [rule_idx]
                                elif "BswMCanSMIndication" in request_value["BswMModeRequestSource"]:
                                    channel = Function.get_key_name(request_value["BswMModeRequestSource"]["BswMCanSMIndication"]["BswMCanSMChannelRef"]["ValueRef"])
                                    if channel in cansm:
                                        cansm[channel].append(rule_idx)
                                    else:
                                        cansm[channel] = [rule_idx]
                                elif "BswMLinSMIndication" in request_value["BswMModeRequestSource"]:
                                    channel = Function.get_key_name(request_value["BswMModeRequestSource"]["BswMLinSMIndication"]["BswMLinSMChannelRef"]["ValueRef"])
                                    if channel in linsm:
                                        linsm[channel].append(rule_idx)
                                    else:
                                        linsm[channel] = [rule_idx]
                                elif "BswMEthSMIndication" in request_value["BswMModeRequestSource"]:
                                    channel = Function.get_key_name(request_value["BswMModeRequestSource"]["BswMEthSMIndication"]["BswMEthSMChannelRef"]["ValueRef"])
                                    if channel in ethsm:
                                        ethsm[channel].append(rule_idx)
                                    else:
                                        ethsm[channel] = [rule_idx]
                                elif "BswMLinScheduleIndication" in request_value["BswMModeRequestSource"]:
                                    channel = Function.get_key_name(request_value["BswMModeRequestSource"]["BswMLinScheduleIndication"]["BswMLinSMChannelRef"]["ValueRef"])
                                    if channel in linsch:
                                        linsch[channel].append(rule_idx)
                                    else:
                                        linsch[channel] = [rule_idx]
                                elif "BswMLinTpModeRequest" in request_value["BswMModeRequestSource"]:
                                    channel = Function.get_key_name(request_value["BswMModeRequestSource"]["BswMLinTpModeRequest"]["BswMLinTpChannelRef"]["ValueRef"])
                                    if channel in lintp:
                                        lintp[channel].append(rule_idx)
                                    else:
                                        lintp[channel] = [rule_idx]
                                elif "BswMNvMJobModeIndication" in request_value["BswMModeRequestSource"]:
                                    service = request_value["BswMModeRequestSource"]["BswMNvMJobModeIndication"]["BswMNvmService"]["Value"]
                                    if service in nvm_job:
                                        nvm_job[service].append(rule_idx)
                                    else:
                                        nvm_job[service] = [rule_idx]
                                elif "BswMNvMRequest" in request_value["BswMModeRequestSource"]:
                                    block = Function.get_key_name(request_value["BswMModeRequestSource"]["BswMNvMRequest"]["BswMNvMBlockRef"]["ValueRef"])
                                    if block in nvm_req:
                                        nvm_req[block].append(rule_idx)
                                    else:
                                        nvm_req[block] = [rule_idx]
                                elif "BswMGenericRequest" in request_value["BswMModeRequestSource"]:
                                    user_id = str(request_value["BswMModeRequestSource"]["BswMGenericRequest"]["BswMModeRequesterId"]["Value"])
                                    if user_id in generic:
                                        generic[user_id].append(rule_idx)
                                    else:
                                        generic[user_id] = [rule_idx]
                                else:
                                    if "extra" in extra:
                                        extra["extra"].append(rule_idx)
                                    else:
                                        extra["extra"] = [rule_idx]
        # rule 排序是根据下面这个字典定死的顺序，请注意
        module = {
            "extra": extra,
            "Dcm": dcm,
            "EcuM_State": ecum_curstate,
            "EcuM_Wakeup": ecum_wakeup,
            "EcuM_Request": ecum_reqstate,
            "ComM": comm,
            "ComM_Pnc": comm_pnc,
            "Nm": nm,
            "CanSM": cansm,
            "LinSM": linsm,
            "EthSM": ethsm,
            "Linsch": linsch,
            "LinTp": lintp,
            "NvM_Job": nvm_job,
            "NvM_Req": nvm_req,
            "Generic": generic
        }

        for module_name, module_value in list(module.items()):
            if len(module_value) > 0:
                immediate_rules[module_name] = module_value

        res["DeferredRules"] = deferred_rules
        res["ImmediateRules"] = immediate_rules

        temp=[]
        for module_name, rule_idxes in immediate_rules.items():
            for rule_user,rule_idx in rule_idxes.items():
                for idx in rule_idx:
                    temp.append(idx)
        immediate_rulesOninit = []
        # 排序以上面字典module中的排序为准
        cnt = 0
        for j in temp:
            for i in immediate_rulesOninit_temp:
                if i == j:
                    immediate_rulesOninit.append(cnt)
                    break
            cnt = cnt+1

        res["ImmediateRulesOninit"] = immediate_rulesOninit
        return res

    @staticmethod
    def get_rule_num(config_dict: dict) -> list:
        res = 0
        immediate_rules = Function.get_ruledata(config_dict)["ImmediateRules"]
        if len(immediate_rules) > 0:
            for module_name, module_value in list(immediate_rules.items()):
                for user, value in list(module_value.items()):
                    res += len(value)
        return res

    @staticmethod
    def get_immreq_index(config_dict: dict) -> dict:
        dcm_idx = {}
        ecumstate_idx = {}
        ecumwakeup_idx = {}
        ecumreqstate_idx = {}
        comm_idx = {}
        commpnc_idx = {}
        nm_idx = {}
        cansm_idx = {}
        linsm_idx = {}
        ethsm_idx = {}
        linsch_idx = {}
        lintp_idx = {}
        nvmjob_idx = {}
        nvmreq_idx = {}
        generic_idx = {}
        extra_idx = {}

        immediate_rules = Function.get_ruledata(config_dict["ActiveEcuC"]["BswM"])["ImmediateRules"]
        imm_rule_idx = 0
        if len(immediate_rules) > 0:
            for module, rule in list(immediate_rules.items()):
                if module == "Dcm":
                    for channel, idxes in list(rule.items()):
                        dcm_idx[channel] = [imm_rule_idx, imm_rule_idx+len(idxes)-1]
                        imm_rule_idx += len(idxes)
                elif module == "EcuM_State":
                    for ecum, idxes in list(rule.items()):
                        ecumstate_idx[ecum] = [imm_rule_idx, imm_rule_idx+len(idxes)-1]
                        imm_rule_idx += len(idxes)
                elif module == "EcuM_Wakeup":
                    for source, idxes in list(rule.items()):
                        ecumwakeup_idx[source] = [imm_rule_idx, imm_rule_idx+len(idxes)-1]
                        imm_rule_idx += len(idxes)
                elif module == "EcuM_Request":
                    for req, idxes in list(rule.items()):
                        ecumreqstate_idx[req] = [imm_rule_idx, imm_rule_idx+len(idxes)-1]
                        imm_rule_idx += len(idxes)
                elif module == "ComM":
                    for channel, idxes in list(rule.items()):
                        comm_idx[channel] = [imm_rule_idx, imm_rule_idx+len(idxes)-1]
                        imm_rule_idx += len(idxes)
                elif module == "ComM_Pnc":
                    for pnc, idxes in list(rule.items()):
                        commpnc_idx[pnc] = [imm_rule_idx, imm_rule_idx+len(idxes)-1]
                        imm_rule_idx += len(idxes)
                elif module == "Nm":
                    for channel, idxes in list(rule.items()):
                        nm_idx[channel] = [imm_rule_idx, imm_rule_idx+len(idxes)-1]
                        imm_rule_idx += len(idxes)
                elif module == "CanSM":
                    for channel, idxes in list(rule.items()):
                        cansm_idx[channel] = [imm_rule_idx, imm_rule_idx+len(idxes)-1]
                        imm_rule_idx += len(idxes)
                elif module == "LinSM":
                    for channel, idxes in list(rule.items()):
                        linsm_idx[channel] = [imm_rule_idx, imm_rule_idx+len(idxes)-1]
                        imm_rule_idx += len(idxes)
                elif module == "EthSM":
                    for channel, idxes in list(rule.items()):
                        ethsm_idx[channel] = [imm_rule_idx, imm_rule_idx+len(idxes)-1]
                        imm_rule_idx += len(idxes)
                elif module == "Linsch":
                    for channel, idxes in list(rule.items()):
                        linsch_idx[channel] = [imm_rule_idx, imm_rule_idx+len(idxes)-1]
                        imm_rule_idx += len(idxes)
                elif module == "LinTp":
                    for channel, idxes in list(rule.items()):
                        lintp_idx[channel] = [imm_rule_idx, imm_rule_idx+len(idxes)-1]
                        imm_rule_idx += len(idxes)
                elif module == "NvM_Job":
                    for service, idxes in list(rule.items()):
                        nvmjob_idx[service] = [imm_rule_idx, imm_rule_idx+len(idxes)-1]
                        imm_rule_idx += len(idxes)
                elif module == "NvM_Req":
                    for block, idxes in list(rule.items()):
                        nvmreq_idx[block] = [imm_rule_idx, imm_rule_idx+len(idxes)-1]
                        imm_rule_idx += len(idxes)
                elif module == "Generic":
                    for user, idxes in list(rule.items()):
                        generic_idx[user] = [imm_rule_idx, imm_rule_idx+len(idxes)-1]
                        imm_rule_idx += len(idxes)
                else:
                    for extra, idxes in list(rule.items()):
                        extra_idx[extra] = [imm_rule_idx, imm_rule_idx+len(idxes)-1]
                        imm_rule_idx += len(idxes)

        module_idx = {
            "extra": extra_idx,
            "Dcm": dcm_idx,
            "EcuM_State": ecumstate_idx,
            "EcuM_Wakeup": ecumwakeup_idx,
            "EcuM_Request": ecumreqstate_idx,
            "ComM": comm_idx,
            "ComM_Pnc": commpnc_idx,
            "Nm": nm_idx,
            "CanSM": cansm_idx,
            "LinSM": linsm_idx,
            "EthSM": ethsm_idx,
            "Linsch": linsch_idx,
            "LinTp": lintp_idx,
            "NvM_Job": nvmjob_idx,
            "NvM_Req": nvmreq_idx,
            "Generic": generic_idx
        }
        return module_idx

    @staticmethod
    def get_swcrequest_port(config_dict: dict) -> list:
        arbitration_dict = config_dict["BswMConfig"]["BswMArbitration"]
        modeControl_dict = config_dict["BswMConfig"]["BswMModeControl"]
        ret_list = []
        for key,value in arbitration_dict.items():
            if (key == "DefinitionRef"):
                continue
            if "BswM/BswMConfig/BswMArbitration/BswMModeRequestPort" in value["DefinitionRef"]:
                for key1,value1 in value.items():
                    if (key1 == "DefinitionRef"):
                        continue
                    if "BswMConfig/BswMArbitration/BswMModeRequestPort/BswMModeRequestSource" in value1["DefinitionRef"]:  
                        for key2,value2 in value1.items():
                            if (key2 == "DefinitionRef"):
                                continue
                            if "BswMSwcModeRequest" in value2["DefinitionRef"]:
                                rport_temp = {}
                                rport_temp["name"] = "Request_" + key
                                interface_ref = value1["BswMSwcModeRequest"]["BswMSwcModeRequestVariableDataPrototypeRef"]["ValueRef"]
                                rport_temp["init_value"] = value["BswMModeInitValue"]["BswMBswModeInitValue"]["Value"]
                                rport_temp["data_element_ref"] = interface_ref
                                rport_temp["require_interface_ref"] = interface_ref.rpartition('/')[0]
                                rport_temp["data_type"] = interface_ref.split('/')[-1]
                                rport_temp["moderequest_name"] = key
                                ret_list.append(rport_temp)
        return ret_list

    # 获取notification port的配置，用来生成service mapping 时，bswm 本身的更新逻辑
    @staticmethod
    def get_notification_request_port(config_dict: dict) -> list:
        arbitration_dict = config_dict["BswMConfig"]["BswMArbitration"]
        modeControl_dict = config_dict["BswMConfig"]["BswMModeControl"]
        ret_list = []
        for key,value in arbitration_dict.items():
            if (key == "DefinitionRef"):
                continue
            if "BswM/BswMConfig/BswMArbitration/BswMModeRequestPort" in value["DefinitionRef"]:
                for key1,value1 in value.items():
                    if (key1 == "DefinitionRef"):
                        continue
                    if "BswMConfig/BswMArbitration/BswMModeRequestPort/BswMModeRequestSource" in value1["DefinitionRef"]:  
                        for key2,value2 in value1.items():
                            if (key2 == "DefinitionRef"):
                                continue
                            if "BswMSwcModeNotification" in value2["DefinitionRef"]:
                                notify_port_temp = {}
                                notify_port_temp["name"] = "Notification" + key
                                interface_ref = value1["BswMSwcModeNotification"]["BswMSwcModeNotificationModeDeclarationGroupPrototypeRef"]["ValueRef"]
                                notify_port_temp["init_value"] = value["BswMModeInitValue"]["BswMBswModeInitValue"]["Value"]
                                notify_port_temp["require_interface_ref"] = interface_ref.rpartition('/')[0]
                                notify_port_temp["modegroup_name"] = interface_ref.split('/')[-1][10:]
                                notify_port_temp["moderequest_name"] = key
                                ret_list.append(notify_port_temp)
        return ret_list

    @staticmethod
    def bswm_get_coreNums(cores: dict) -> int:
        res = 0
        if (len(cores)-1) == 0:
            res = int(1)
        else:
            res = int(len(cores)-1)
        return res
    
    @staticmethod
    def get_modeswitch_request_port(config_dict: dict) -> list:
        modeControl_dict = config_dict["BswMConfig"]["BswMModeControl"]
        ret_list = []
        for key,value in modeControl_dict.items():
            if (key == "DefinitionRef"):
                continue            
            if "BswM/BswMConfig/BswMModeControl/BswMSwitchPort" in value["DefinitionRef"]:
                pport_temp = {}
                pport_temp["name"] = "Switch_" + key
                pport_temp["admin_data_sdx_ref"] = '/ActiveEcuC/BswM/BswMConfig/BswMModeControl/' + key
                pport_temp["provide_interface_ref"] = value["BswMModeSwitchInterfaceRef"]["ValueRef"]
                pport_temp["modegroup_name"] = value["BswMModeSwitchInterfaceRef"]["ValueRef"].split('/')[-1][9:]
                ret_list.append(pport_temp)
        return ret_list


    @staticmethod
    def get_mode_declaration_groups(config_dict: dict):
        ret_list = []
        bswm_group_mode_dict = config_dict["BswMConfig"]
        for key,value in bswm_group_mode_dict.items():
            if key == "DefinitionRef" :
                continue
            temp_mode_declaration_groups = {}
            if "BswMRteModeDclGroup" in value["DefinitionRef"]:
                ## find the group
                temp_mode_declaration_groups["name"] = key
                temp_mode_declaration_groups["initial_mode_ref"] = "/AUTOSAR/BswM_swc/"+value["BswMInitMode"]["ValueRef"].split("/")[-2] + "/" +value["BswMInitMode"]["ValueRef"].split("/")[-1]
                temp1 = {}
                cnt = 0
                for key1,value1 in value.items():
                    if key1 == "DefinitionRef":
                        continue
                    if "BswMRteModeDclGroup/BswMRteMode" in value1["DefinitionRef"]:
                        temp1[key1] = value1["BswMModePhysicalValue"]["Value"]
                        cnt = cnt+1
                temp_mode_declaration_groups["TRANSITION"] = cnt
                temp_mode_declaration_groups["mode_value"] = temp1
                ret_list.append(temp_mode_declaration_groups)
        
        return ret_list