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
"""
过滤出生成COM模块动态代码所需的dict
过滤出生成Com_Cfg.json所需的dict
"""

from func_module.health_monitor.logger import logger
import copy


def get_dynamic_code_template_input_parameter_dict(Config_dict: dict) -> dict:
    """
    extract parameter for generator
    """
    # 从RTE的dict获取相应port信息（data mapping设置成On_Data_Reception的port）
    # rxcbk_list = {}
    # if "data_mapping_config" in Config_dict['rte'].keys():
    #     if "data_received_event_list" in Config_dict['rte']['data_mapping_config'].keys():
    #         rxcbk_list = Config_dict['rte']['data_mapping_config']['data_received_event_list']
    RxCbk = {
    }  #{cbk_item["port_alias_name"] + "_Rx": ("Rte_COMCbk") for cbk_item in rxcbk_list}    # del_json_description(Config_dict)
    # ComConfig = Config_dict["ComConfig"]
    # ComGeneral = Config_dict["ComGeneral"]

    EcuC_dict = Config_dict['bsw']['ecuc']['ArPackages']['ActiveEcuC']['EcuC']
    Com_dict = Config_dict['bsw']['com']['ArPackages']['ActiveEcuC']['Com']
    ComGeneral = com_tools_get_dict_by_definition_ref(Com_dict, "ComGeneral")
    ComConfig_old = com_tools_get_dict_by_definition_ref(Com_dict, "ComConfig")
    # ComGeneral = Config_dict['bsw']['com']['ArPackages']['ActiveEcuC']['Com']['ComGeneral']
    # ComConfig_old = Config_dict['bsw']['com']['ArPackages']['ActiveEcuC']['Com']['ComConfig']
    app_coreid_dict = {}
    Os_dict = {}
    if "os" in Config_dict['bsw'].keys():
        Os_dict = Config_dict['bsw']['os']['ArPackages']['ActiveEcuC']['Os']
    if Os_dict != {}:
        app_coreid_dict = get_application_core_map(Os_dict, EcuC_dict)
    else:
        app_coreid_dict[0] = ""
        app_coreid_dict[1] = ""
        app_coreid_dict[2] = ""
        app_coreid_dict[3] = ""
        app_coreid_dict[4] = ""
        app_coreid_dict[5] = ""
    ComConfig = {}
    ComIpdus_new = {}
    ComSignal_new = {}
    ComGwMapping_new = {}
    ComIpduGroup_new = {}
    ComSignalGroup_new = {}
    ComMainFunction_tx = {}
    ComMainFunction_rx = {}
    ComMainFunction_gw = {}
    ComTxSignalFifo = {}

    signal_id = 0
    message_id = 0
    message_id_tx = 0
    message_id_rx = 0
    signal_id_tx = 0
    signal_id_rx = 0
    TxIpdu_id = 0
    signalNotification_id = 0
    rx_callout_id = 0
    tx_callout_id = 0
    tt_callout_id = 0
    TxIpdus = []
    

    # IPDUGroup 需要是全局的
    IpduGroup_Id = 0

    # system = ComGeneral["Parameters"]["System"]["Value"]

    # 从ComConfig中筛选出Ipdu和signal ComGwMapping
    for ComConfigKey, value in ComConfig_old.items():
        if ComConfigKey == "DefinitionRef" or ComConfigKey == "Tag":
            continue
        if isinstance(value, dict):
            if value["DefinitionRef"] == '/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu':
                keyname = value['ComPduIdRef']['ValueRef'].split('/')[-1]
            if value["DefinitionRef"] == "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu":
                ComIpdus_new[keyname] = value
            if value["DefinitionRef"] == "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignal":
                ComSignal_new[ComConfigKey] = value
            if value["DefinitionRef"] == "/AUTOSAR/EcucDefs/Com/ComConfig/ComGwMapping":
                ComGwMapping_new[ComConfigKey] = value
            if value["DefinitionRef"] == "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPduGroup":
                ComIpduGroup_new[ComConfigKey] = value
            if value["DefinitionRef"] == "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignalGroup":
                ComSignalGroup_new[ComConfigKey] = value
            if value["DefinitionRef"] == "/AUTOSAR/EcucDefs/Com/ComConfig/ComMainFunctionTx":
                ComMainFunction_tx[ComConfigKey] = value
            if value["DefinitionRef"] == "/AUTOSAR/EcucDefs/Com/ComConfig/ComMainFunctionRx":
                ComMainFunction_rx[ComConfigKey] = value
            if value["DefinitionRef"] == "/AUTOSAR/EcucDefs/Com/ComConfig/ComMainFunctionGw":
                ComMainFunction_gw[ComConfigKey] = value
            if value["DefinitionRef"] == "/AUTOSAR/EcucDefs/Com/ComConfig/ComTxSignalFifo":
                ComTxSignalFifo[ComConfigKey] = value

    ComIpdus_sort = {}
    #将ComIpdus_new按照ComIPduHandleId大小顺序排序
    ComIpdus_sort = handle_Ipdusort_Depend_ComIPduHandleId(ComIpdus_new)
    ComIpduGroup_sort = handle_Ipdusort_Depend_ComIPduGroupHandleId(ComIpduGroup_new)

    ComConfig["ComIPdus"] = {}
    # ComConfig["ComSignals"] = ComSignal_new
    ComConfig["ComGwMappings"] = ComGwMapping_new
    ComConfig["ComIPduGroups"] = ComIpduGroup_sort
    ComConfig["ComSignalGroups"] = ComSignalGroup_new
    ComConfig["ComTxSignalFifos"] = ComTxSignalFifo
    ComConfig["ComSignals_totle"] = {}
    ComConfig["ComTxSignals_totle"] = {}
    ComConfig["ComRxSignals_totle"] = {}
    ComConfig["ComIPdus_totle"] = {}

    ComConfig["ComRxIPdus_totle"] = {}
    ComConfig["ComTxIPdus_totle"] = {}
    ComConfig["ComMainFunction_tx_totle"] = {}
    ComConfig["ComMainFunction_rx_totle"] = {}
    ComConfig["ComMainFunction_gw_totle"] = {}


    # 获取Com运行的Core ID
    # 此处是Com_SendSignal 代理功能 后续需要支持多核需要考虑该处的实现
    ComGeneral["Com_CoreId"] = 0
    if "rte" in Config_dict.keys():
        Rte_dict = Config_dict['rte']
        if ComConfig["ComTxSignalFifos"] != {}:
            Com_CoreId = handle_ComMainFunctionTx_CoreId(Rte_dict)
            ComGeneral["Com_CoreId"] = Com_CoreId

    # 将ComIPduSignalRef  ComIPduGroup ComIPduSignalGroupRef全部转成list
    handle_Str_To_List(ComIpdus_sort)

    #将默认的ComSignals的信号组属性都填充为FALSE
    handle_SgInfo_Signal(ComSignal_new)

    # 获取每一个ComMainFunction关联的Core信息
    handle_mainfunction_coreId(ComMainFunction_tx, ComMainFunction_rx, ComMainFunction_gw, Config_dict['rte'])

    # 将ecuc的Ref提取出来 包括PduLength  CoreId 并填充Signal关联的CoreId
    # 获取每一个IPDU Signal关联的Core信息
    handle_ecuc_ref(ComIpdus_sort, ComSignal_new, EcuC_dict)

    # 将IPDU根据Core信息进行拆分
    ComIpdus_sort_result = split_dict_by_core_id(ComIpdus_sort)

    # # 将Signal根据Core信息进行拆分
    ComSignal_sort_result = split_dict_by_core_id(ComSignal_new)

    # 将MainFunctionTx根据Core信息进行拆分
    ComMainFunction_tx_sort_result = split_dict_by_core_id(ComMainFunction_tx)

    # 将MainFunctionRx根据Core信息进行拆分
    ComMainFunction_rx_sort_result = split_dict_by_core_id(ComMainFunction_rx)

    # 将MainFunctionGw根据Core信息进行拆分
    ComMainFunction_gw_sort_result = split_dict_by_core_id(ComMainFunction_gw)

    # 统一放到ComConfig Dict内部
    ComConfig["ComMainFunction_tx"] = ComMainFunction_tx_sort_result
    ComConfig["ComMainFunction_rx"] = ComMainFunction_rx_sort_result
    ComConfig["ComMainFunction_gw"] = ComMainFunction_gw_sort_result
    ComConfig["ComSignals"] = ComSignal_sort_result

    #将每一个Core的ComIpdus按照mainfunction ref 进行排序
    handle_Ipdusort_Depend_ComMainFunctionRef(ComConfig, ComIpdus_sort_result, ComGeneral)
    
    # 删除DefinitionRef属性
    del_useless_info(ComConfig)
    del_useless_info(ComGeneral)

   # 将Cbk File全部转成list
    ComGeneral["ComUserCbkHeaderList"] = []
    ComGeneralCbkFilelist = []
    if "ComUserCbkHeaderFile" in ComGeneral.keys():
        if "Value" in ComGeneral["ComUserCbkHeaderFile"].keys():
            if ComGeneral["ComUserCbkHeaderFile"]["Value"] != None:
                if isinstance(ComGeneral["ComUserCbkHeaderFile"]["Value"], list):
                    ComGeneral["ComUserCbkHeaderList"] = ComGeneral["ComUserCbkHeaderFile"]["Value"]
                else:
                    ComGeneralCbkFilelist.append(ComGeneral["ComUserCbkHeaderFile"]["Value"])
                    ComGeneral["ComUserCbkHeaderList"] =  ComGeneralCbkFilelist.copy()
                    ComGeneralCbkFilelist.clear()

    # 设置ComGeneral全局变量
    handle_ComGeneral_Value(ComGeneral)

    #定义信号初始值类型
    handle_Signal_InitParameter(ComGeneral)

    for coreId, com_ipdu_Info in ComConfig["ComIPdus"].items():
        # 用于记录CAN部署的core的最大coreId Idx
        ComGeneral["Com_MaxCore_Idx"] = coreId

        ComGeneral["ipdu_size"][coreId]["ipdu_size"] = 0
        ComGeneral["Txipdu_size"][coreId]["Txipdu_size"] = 0
        ComGeneral["Rxipdu_size"][coreId]["Rxipdu_size"] = 0

        ComGeneral["Txipdu_size_record"][coreId]["Txipdu_size_record"] = 0
        ComGeneral["Rxipdu_size_record"][coreId]["Rxipdu_size_record"] = 0
        
        ComGeneral["message_id"][coreId]["message_id"] = 0
        ComGeneral["message_id_tx"][coreId]["message_id_tx"] = 0
        ComGeneral["message_id_rx"][coreId]["message_id_rx"] = 0

        ComGeneral["signal_id"][coreId]["signal_id"] = 0
        ComGeneral["signal_id_tx"][coreId]["signal_id_tx"] = 0
        ComGeneral["signal_id_rx"][coreId]["signal_id_rx"] = 0
        ComGeneral["signal_endIdx_tx"][coreId]["signal_endIdx_tx"] = 0
        ComGeneral["signal_endIdx_rx"][coreId]["signal_endIdx_rx"] = 0

        ComGeneral["signal_startIdx_tx"][coreId]["signal_startIdx_tx"] = 0
        ComGeneral["signal_startIdx_rx"][coreId]["signal_startIdx_rx"] = 0

        ComGeneral["ipdu_endIdx_tx"][coreId]["ipdu_endIdx_tx"] = 0
        ComGeneral["ipdu_endIdx_rx"][coreId]["ipdu_endIdx_rx"] = 0

        ComGeneral["ipdu_startIdx_tx"][coreId]["ipdu_startIdx_tx"] = 0
        ComGeneral["ipdu_startIdx_rx"][coreId]["ipdu_startIdx_rx"] = 0


        ComGeneral["uint8_bool_signal_num"][coreId]["uint8_bool_signal_num"] = 0
        ComGeneral["uint16_signal_num"][coreId]["uint16_signal_num"] = 0   
        ComGeneral["uint32_signal_num"][coreId]["uint32_signal_num"] = 0  
        ComGeneral["uint64_signal_num"][coreId]["uint64_signal_num"] = 0
        ComGeneral["uint8_N_signal_num"][coreId]["uint8_N_signal_num"] = 0

        ComGeneral["Com_Core_Num"] += 1
        # 记录当前Core的start IPDUId
        ComGeneral["ipdu_startIdx_tx"][coreId]["ipdu_startIdx_tx"] = message_id_tx
        ComGeneral["ipdu_startIdx_rx"][coreId]["ipdu_startIdx_rx"] = message_id_rx

        ComGeneral["signal_startIdx_tx"][coreId]["signal_startIdx_tx"] = signal_id_tx
        ComGeneral["signal_startIdx_rx"][coreId]["signal_startIdx_rx"] = signal_id_rx
        for ipdu in list(com_ipdu_Info.keys()):
            # 处理报文总长度
            if "PduRef" in ComConfig["ComIPdus"][coreId][ipdu].keys():
                ComGeneral["ipdu_size"][coreId]["ipdu_size"] += ComConfig["ComIPdus"][coreId][ipdu]["PduRef"]["PduLength"]["Value"]

            TxIpdu_id = handle_message_direction(ComGeneral, ComConfig, ipdu, TxIpdus, TxIpdu_id, coreId)
            # handle tx periodic
            handle_message_periodic(ComConfig, ipdu, coreId)
            # handle message id
            message_id = handle_message_info(ComGeneral, ComConfig, ipdu, message_id, coreId)

            # handle callout function
            rx_callout_id, tx_callout_id, tt_callout_id = handle_callout(ComConfig, ipdu, rx_callout_id, tx_callout_id,
                                                                        tt_callout_id, coreId)
            # 处理信号组
            pre_handle_Signal_Group(ComGeneral, ComConfig, ipdu, signal_id, coreId)

            ComConfig["ComIPdus"][coreId][ipdu]["ComIPduSignalNum"] = len(
                ComConfig["ComIPdus"][coreId][ipdu]["ComIPduSignalRef"]["ValueRef"])
            
            # 计算IPDU中的最大信号个数
            if ComConfig["ComIPdus"][coreId][ipdu]["direction"] == "COM_RECEIVE":
                if ComGeneral["Ipdu_MaxSig_Num"] < ComConfig["ComIPdus"][coreId][ipdu]["ComIPduSignalNum"]:
                    ComGeneral["Ipdu_MaxSig_Num"] = ComConfig["ComIPdus"][coreId][ipdu]["ComIPduSignalNum"]
            
            handle_ipu_init_value(ComConfig, ipdu, coreId)

            handle_messag_Signal_StartIdx_from_direction(ComConfig, ipdu, signal_id_tx, signal_id_rx, coreId)
            # 处理信号
            for signal in list(ComConfig["ComIPdus"][coreId][ipdu]["ComIPduSignalRef"]["ValueRef"]):
                # 填充信号关联的Core和application
                ComConfig["ComSignals"][coreId][signal]["CoreRef"] = ComConfig["ComIPdus"][coreId][ipdu]["CoreRef"]
                ComConfig["ComSignals"][coreId][signal]["application"] = ComConfig["ComIPdus"][coreId][ipdu]["application"]

                ComConfig["ComSignals"][coreId][signal]["UseUpdateBit"] = "FALSE"
                handle_update_bit(ComGeneral, ComConfig, ipdu, signal, coreId)
                # handle basic info of signal
                signal_id = handle_signal_info(ComGeneral, ComConfig, ipdu, signal, signal_id, coreId)
                # register notificaiton cbk for signal
                if signal in RxCbk:
                    ComConfig["ComSignals"][coreId][signal]["ComNotification"]["Value"] = RxCbk[signal] + "_" + signal
                # handle signal notification
                signalNotification_id = handle_signal_notification(ComConfig, ipdu, signal, signalNotification_id, coreId)
                # 将tx rx signal分开
                signal_id_tx, signal_id_rx = handle_signal_from_direction(ComGeneral, ComConfig, ipdu, signal, signal_id_tx,
                                                                        signal_id_rx, coreId)

            # 将tx rx pdu分开
            message_id_tx, message_id_rx = handle_messag_from_direction(ComGeneral, ComConfig, ipdu, message_id_tx, message_id_rx, signal_id_tx, signal_id_rx, coreId)

            # IPdu Size Record记录当前信号所在的起始byte位置
            ComGeneral["Txipdu_size_record"][coreId]["Txipdu_size_record"] = ComGeneral["Txipdu_size"][coreId]["Txipdu_size"]
            ComGeneral["Rxipdu_size_record"][coreId]["Rxipdu_size_record"] = ComGeneral["Rxipdu_size"][coreId]["Rxipdu_size"]

        # hanle signal id sort
        ComConfig["ComSignals"][coreId] = handle_Signalsort_Depend_SignalId(ComConfig["ComSignals"][coreId])

        # 记录当前Core的end SignalId
        ComGeneral["signal_endIdx_tx"][coreId]["signal_endIdx_tx"] = signal_id_tx
        ComGeneral["signal_endIdx_rx"][coreId]["signal_endIdx_rx"] = signal_id_rx

        ComGeneral["ipdu_endIdx_tx"][coreId]["ipdu_endIdx_tx"] = message_id_tx
        ComGeneral["ipdu_endIdx_rx"][coreId]["ipdu_endIdx_rx"] = message_id_rx


        # 将多核的ipdu signal合并
        ComConfig["ComSignals_totle"].update(ComConfig["ComSignals"][coreId])
        ComConfig["ComTxSignals_totle"].update(ComConfig["ComTxSignals"][coreId])
        ComConfig["ComRxSignals_totle"].update(ComConfig["ComRxSignals"][coreId])

        ComConfig["ComIPdus_totle"].update(ComConfig["ComIPdus"][coreId])
        ComConfig["ComRxIPdus_totle"].update(ComConfig["ComRxIPdus"][coreId])
        ComConfig["ComTxIPdus_totle"].update(ComConfig["ComTxIPdus"][coreId])

        ComConfig["ComMainFunction_tx_totle"].update(ComConfig["ComMainFunction_tx"][coreId])
        ComConfig["ComMainFunction_rx_totle"].update(ComConfig["ComMainFunction_rx"][coreId])
        # 后面需要考虑信号网关多核
        if coreId in ComConfig["ComMainFunction_gw"].keys():
            ComConfig["ComMainFunction_gw_totle"].update(ComConfig["ComMainFunction_gw"][coreId])

    # handle IPdu Groups
    IpduGroup_Id = handle_ipdu_groups(ComConfig, IpduGroup_Id)
    # handle signal gateway
    handle_signal_gateway(ComGeneral, ComConfig)

    # handle Global Info
    handle_global_info(ComGeneral, ComConfig, TxIpdus, message_id, signal_id, IpduGroup_Id, message_id_tx,
                    message_id_rx, signal_id_tx, signal_id_rx, app_coreid_dict)

    # 将Com_MainFunctionRxStruct映射到ComRxMessageTOutInfo
    handle_MainFunctionRx_to_RxMessageTOutInfo(ComGeneral, ComConfig)

    # 将Com_MainFunctionRxStruct映射到ComRxSignalTOutInfo
    handle_MainFunctionRx_to_ComRxSignalTOutInfo(ComGeneral, ComConfig)

    # 获取Com_MainFunctionRx对应的RxNotificationRecord
    handle_MainFunctionRx_RxNotificationRecord(ComGeneral, ComConfig, app_coreid_dict)

    return {"ComConfig": ComConfig, "ComGeneral": ComGeneral}

def get_application_core_map(Os_dict, EcuC_dict):
    Os_Core_dict = {}
    app_coreid_dict = {}
    for OsKey, value in Os_dict.items():
        if OsKey == "DefinitionRef" or OsKey == "Tag":
            continue
        if isinstance(value, dict):
            if value["DefinitionRef"] == '/AUTOSAR/EcucDefs/Os/OsCore':
                Os_Core_dict[OsKey] = value
    for Os_Core, value in Os_Core_dict.items():
        app_name_value = value['OsCoreBswApplicationRef']['ValueRef']
        app_name_value = app_name_value[app_name_value.rfind("/") + 1:]
        ecuc_core_ref = Os_dict[app_name_value]['OsApplicationCoreRef']['ValueRef']
        ecuc_core_ref = ecuc_core_ref[ecuc_core_ref.rfind("/") + 1:]
        coreId = EcuC_dict['EcucHardware'][ecuc_core_ref]['EcucCoreId']['Value']
        app_coreid_dict[coreId] = app_name_value.upper()
    return app_coreid_dict

def handle_ComMainFunctionTx_CoreId(Rte_dict:dict):
    comcoreid = 0
    if "task_mappings_info" in Rte_dict.keys():
        for task_name in list(Rte_dict["task_mappings_info"].keys()):
            if "schedulable_entity_info_list" in Rte_dict["task_mappings_info"][task_name].keys():
                if isinstance(Rte_dict["task_mappings_info"][task_name]["schedulable_entity_info_list"], list):
                    for i in range(len(Rte_dict["task_mappings_info"][task_name]["schedulable_entity_info_list"])):
                        if Rte_dict["task_mappings_info"][task_name]["schedulable_entity_info_list"][i]["name"] == "ComMainFunctionTx":
                            comcoreid = int(Rte_dict["task_mappings_info"][task_name]["core_id"])
                            return comcoreid
    return comcoreid

def handle_ipu_init_value(ComConfig: dict, IPdu: str, coreId):
    # 处理IPDU中所有信号超时值是否相等  默认是全部相等 初始化IPDU超时值 默认为0
    ComConfig["ComIPdus"][coreId][IPdu]["TimeoutValue"] = 0
    ComConfig["ComIPdus"][coreId][IPdu]["FirstTimeoutValue"] = 0
    ComConfig["ComIPdus"][coreId][IPdu]["ISRXTIMEOUT"] = "FALSE"
    ComConfig["ComIPdus"][coreId][IPdu]["ISTXTIMEOUT"] = "FALSE"
    ComConfig["ComIPdus"][coreId][IPdu]["RxTimeoutIndex"] = "COM_NO_REF_INDEX"
    ComConfig["ComIPdus"][coreId][IPdu]["TxTimeoutIndex"] = "COM_NO_REF_INDEX"


    # 处理IPDU中信号是否配置notification
    ComConfig["ComIPdus"][coreId][IPdu]["IsComNotification"] = "FALSE"
    ComConfig["ComIPdus"][coreId][IPdu]["IsComTimeoutNotification"] = "FALSE"
    ComConfig["ComIPdus"][coreId][IPdu]["IsComInvalidNotification"] = "FALSE"
    ComConfig["ComIPdus"][coreId][IPdu]["IsComErrorNotification"] = "FALSE"

    ComConfig["ComIPdus"][coreId][IPdu]["StartNoti_id"] = "COM_NO_REF_INDEX"
    ComConfig["ComIPdus"][coreId][IPdu]["Noti_id_num"] = 0

    ComConfig["ComIPdus"][coreId][IPdu]["StartTimeoutNoti_id"] = "COM_NO_REF_INDEX"
    ComConfig["ComIPdus"][coreId][IPdu]["TimeoutNoti_id_num"] = 0

    ComConfig["ComIPdus"][coreId][IPdu]["StartInvalidNoti_id"] = "COM_NO_REF_INDEX"
    ComConfig["ComIPdus"][coreId][IPdu]["InvalidNoti_id_num"] = 0

    ComConfig["ComIPdus"][coreId][IPdu]["StartErrorNoti_id"] = "COM_NO_REF_INDEX"
    ComConfig["ComIPdus"][coreId][IPdu]["ErrorNoti_id_num"] = 0

# 将ComMainFunction_tx rx按照周期时间大小排序
def handle_Mainfunctionsort_Depend_Timebase(ComMainFunction: dict, MainTimeBase: str) -> dict:
    for coreId,com_mainfunction_Info in ComMainFunction.items():
        com_mainfunction_list = []
        for main_name, main_cfg in com_mainfunction_Info.items():
            main = {}
            main.update(main_cfg)
            main["Name"] = main_name
            com_mainfunction_list.append(main)
        com_mainfunction_list.sort(key=lambda main: main[MainTimeBase]["Value"])

        ComMainFunction_sort = {}
        for i in range(len(com_mainfunction_list)):
            ComMainFunction_sort[com_mainfunction_list[i].get("Name")] = com_mainfunction_list[i]
        ComMainFunction[coreId] = ComMainFunction_sort


# 找到每个MainFunction绑定的Core
def handle_mainfunction_coreId(ComMainFunction_tx:dict, ComMainFunction_rx:dict, ComMainFunction_gw:dict, Rte_dict:dict):
    comcoreid = 0
    # Com_MainFunction_Tx
    for mainfunction_tx in list(ComMainFunction_tx.keys()):
        ComMainFunction_tx[mainfunction_tx]["CoreRef"] = 0
        ComMainFunction_tx[mainfunction_tx]["application"] = "Core0"
        if "task_mappings_info" in Rte_dict.keys():
            for task_name in list(Rte_dict["task_mappings_info"].keys()):
                if "schedulable_entity_info_list" in Rte_dict["task_mappings_info"][task_name].keys():
                    if isinstance(Rte_dict["task_mappings_info"][task_name]["schedulable_entity_info_list"], list):
                        for i in range(len(Rte_dict["task_mappings_info"][task_name]["schedulable_entity_info_list"])):
                            if Rte_dict["task_mappings_info"][task_name]["schedulable_entity_info_list"][i]["name"] == mainfunction_tx:
                                comcoreid = int(Rte_dict["task_mappings_info"][task_name]["core_id"])
                                ComMainFunction_tx[mainfunction_tx]["CoreRef"] = comcoreid
                                ComMainFunction_tx[mainfunction_tx]["application"] = Rte_dict["task_mappings_info"][task_name]["application"]
    # Com_MainFunction_Rx
    for mainfunction_rx in list(ComMainFunction_rx.keys()):
        ComMainFunction_rx[mainfunction_rx]["CoreRef"] = 0
        ComMainFunction_rx[mainfunction_rx]["application"] = "Core0"
        if "task_mappings_info" in Rte_dict.keys():
            for task_name in list(Rte_dict["task_mappings_info"].keys()):
                if "schedulable_entity_info_list" in Rte_dict["task_mappings_info"][task_name].keys():
                    if isinstance(Rte_dict["task_mappings_info"][task_name]["schedulable_entity_info_list"], list):
                        for i in range(len(Rte_dict["task_mappings_info"][task_name]["schedulable_entity_info_list"])):
                            if Rte_dict["task_mappings_info"][task_name]["schedulable_entity_info_list"][i]["name"] == mainfunction_rx:
                                comcoreid = int(Rte_dict["task_mappings_info"][task_name]["core_id"])
                                ComMainFunction_rx[mainfunction_rx]["CoreRef"] = comcoreid
                                ComMainFunction_rx[mainfunction_rx]["application"] = Rte_dict["task_mappings_info"][task_name]["application"]
    # Com_MainFunction_Gw
    for mainfunction_gw in list(ComMainFunction_gw.keys()):
        ComMainFunction_gw[mainfunction_gw]["CoreRef"] = 0
        ComMainFunction_gw[mainfunction_gw]["application"] = "Core0"
        if "task_mappings_info" in Rte_dict.keys():
            for task_name in list(Rte_dict["task_mappings_info"].keys()):
                if "schedulable_entity_info_list" in Rte_dict["task_mappings_info"][task_name].keys():
                    if isinstance(Rte_dict["task_mappings_info"][task_name]["schedulable_entity_info_list"], list):
                        for i in range(len(Rte_dict["task_mappings_info"][task_name]["schedulable_entity_info_list"])):
                            if Rte_dict["task_mappings_info"][task_name]["schedulable_entity_info_list"][i]["name"] == mainfunction_gw:
                                comcoreid = int(Rte_dict["task_mappings_info"][task_name]["core_id"])
                                ComMainFunction_gw[mainfunction_gw]["CoreRef"] = comcoreid
                                ComMainFunction_gw[mainfunction_gw]["application"] = Rte_dict["task_mappings_info"][task_name]["application"]

def split_dict_by_core_id(input: dict):
    """根据 "CoreId" 拆分子字典，相同CoreId组成列表。

    Args:
        input: ComIPdu or ComSignal

    Returns:
        一个字典，键是 "CoreId" ，值是该CoreId对应的IPDU或者Signal列表。
    """
    result = {}
    for name, value in input.items():
        coreId = value["CoreRef"]
        if coreId not in result:
            result[coreId] = {}  # 创建列表存储相同CoreId的result
        result[coreId].update({name: value})  # 将result信息添加到对应列表
    # 使用 sorted() 函数对字典的键进行排序
    result_key = sorted(result.keys())

    # 创建一个新的字典来存储排序后的结果
    sorted_result = {}

    # 遍历排序后的键，将对应的键值对添加到新字典中
    for key in result_key:
        sorted_result[key] = result[key]

    # 输出排序后的字典
    return sorted_result


def handle_Ipdusort_Depend_ComMainFunctionRef(ComConfig: dict, ComIpdus_sort: dict, ComGeneral:dict) -> dict:
    
    handle_Mainfunctionsort_Depend_Timebase(ComConfig["ComMainFunction_tx"], "MainTxTimeBase")
    handle_Mainfunctionsort_Depend_Timebase(ComConfig["ComMainFunction_rx"], "MainRxTimeBase")
    handle_Mainfunctionsort_Depend_Timebase(ComConfig["ComMainFunction_gw"], "MainGwTimeBase")
    
    ComGeneral["mainfunction_num"] = {}
    # 先在这里定义 
    ComGeneral["ipdu_size"] = {}
    ComGeneral["Txipdu_size"] = {}
    ComGeneral["Rxipdu_size"] = {}
    ComGeneral["Txipdu_size_record"] = {}
    ComGeneral["Rxipdu_size_record"] = {}
    ComGeneral["message_id"] = {}
    ComGeneral["message_id_tx"] = {}
    ComGeneral["message_id_rx"] = {}
    ComGeneral["ipdu_endIdx_tx"] = {}
    ComGeneral["ipdu_endIdx_rx"] = {}
    ComGeneral["ipdu_startIdx_tx"] = {}
    ComGeneral["ipdu_startIdx_rx"] = {}

    ComGeneral["signal_id"] = {}
    ComGeneral["signal_id_tx"] = {}
    ComGeneral["signal_id_rx"] = {}
    ComGeneral["signal_endIdx_tx"] = {}
    ComGeneral["signal_endIdx_rx"] = {}
    ComGeneral["signal_startIdx_tx"] = {}
    ComGeneral["signal_startIdx_rx"] = {}

    ComConfig["ComTxIPdus"] = {}
    ComConfig["ComRxIPdus"] = {}
    ComConfig["ComTxSignals"] = {}
    ComConfig["ComRxSignals"] = {}

    ComGeneral["CoreMapInfo"] = {}

    ComGeneral["uint8_bool_signal_num"] = {}
    ComGeneral["uint16_signal_num"] = {}    
    ComGeneral["uint32_signal_num"] = {}  
    ComGeneral["uint64_signal_num"] = {}
    ComGeneral["uint8_N_signal_num"] = {}

    # 提取mainfunction_tx的IPDU信息
    StartTxIPduIdx = 0
    for coreId, com_mainfunction_Info in ComConfig["ComMainFunction_tx"].items():
        mainfunction_tx_num = 0
        TxIPduNum = 0
        if coreId in ComIpdus_sort.keys():
            ComConfig["ComIPdus"][coreId] = {}
        
        ComGeneral["mainfunction_num"][coreId] = {}
        # 先在这里定义 
        ComGeneral["ipdu_size"][coreId] = {}
        ComGeneral["Txipdu_size"][coreId] = {}  
        ComGeneral["Rxipdu_size"][coreId] = {}
        ComGeneral["Txipdu_size_record"][coreId] = {}
        ComGeneral["Rxipdu_size_record"][coreId] = {}
        ComGeneral["message_id"][coreId] = {}
        ComGeneral["message_id_tx"][coreId] = {}
        ComGeneral["message_id_rx"][coreId] = {}
        ComGeneral["ipdu_startIdx_tx"][coreId] = {}
        ComGeneral["ipdu_startIdx_rx"][coreId] = {}

        ComGeneral["ipdu_endIdx_tx"][coreId] = {}
        ComGeneral["ipdu_endIdx_rx"][coreId] = {}

        ComGeneral["signal_id"][coreId] = {}
        ComGeneral["signal_id_tx"][coreId] = {}
        ComGeneral["signal_id_rx"][coreId] = {}

        ComGeneral["signal_startIdx_tx"][coreId] = {}
        ComGeneral["signal_startIdx_rx"][coreId] = {}
        ComGeneral["signal_endIdx_tx"][coreId] = {}
        ComGeneral["signal_endIdx_rx"][coreId] = {}

        ComConfig["ComTxIPdus"][coreId] = {}
        ComConfig["ComRxIPdus"][coreId] = {}
        ComConfig["ComTxSignals"][coreId] = {}
        ComConfig["ComRxSignals"][coreId] = {}

        ComGeneral["CoreMapInfo"][coreId] = {}

        ComGeneral["uint8_bool_signal_num"][coreId] = {}
        ComGeneral["uint16_signal_num"][coreId] = {}    
        ComGeneral["uint32_signal_num"][coreId] = {}  
        ComGeneral["uint64_signal_num"][coreId] = {}
        ComGeneral["uint8_N_signal_num"][coreId] = {}

        for mainfunction_tx in list(com_mainfunction_Info.keys()):
            mainfunction_tx_num += 1
            #确认ComfirmationInfo
            com_mainfunction_Info[mainfunction_tx]["TxComfirmationInfo"] = "FALSE"

            com_mainfunction_Info[mainfunction_tx]["StartTxIPduIdx"] = StartTxIPduIdx
            if coreId in ComIpdus_sort.keys():
                for ipdu in list(ComIpdus_sort[coreId].keys()):
                    IPduMainFunctionRef = ComIpdus_sort[coreId][ipdu]['IPduMainFunctionRef']['ValueRef']
                    IPduMainFunctionRef = IPduMainFunctionRef[IPduMainFunctionRef.rfind("/") + 1:]
                    if IPduMainFunctionRef == mainfunction_tx:
                        ComIpdus_sort[coreId][ipdu]["application"] =  com_mainfunction_Info[mainfunction_tx]["application"]
                        ComConfig["ComIPdus"][coreId].update({ipdu: ComIpdus_sort[coreId][ipdu]})
                        StartTxIPduIdx += 1
                        TxIPduNum += 1
            if TxIPduNum == 0:
                com_mainfunction_Info[mainfunction_tx]["StartTxIPduIdx"] = "NO_PDU_REFERENCE"
        
            com_mainfunction_Info[mainfunction_tx]["TxIPduNum"] = TxIPduNum
            TxIPduNum = 0
        ComGeneral["mainfunction_num"][coreId]["mainfunction_tx_num"] = mainfunction_tx_num

    # 提取mainfunction_rx的IPDU信息
    StartRxIPduIdx = 0
    for coreId, com_mainfunction_Info in ComConfig["ComMainFunction_rx"].items():
        mainfunction_rx_num = 0
        RxIPduNum = 0

        for mainfunction_rx in list(com_mainfunction_Info.keys()):
            mainfunction_rx_num += 1
            # 确认RxTimeoutInfo
            com_mainfunction_Info[mainfunction_rx]["RxTimeoutInfo"] = "FALSE"

            # 映射 RxToutmessag数组
            com_mainfunction_Info[mainfunction_rx]["IsRxToutmessagStartIdx"] = "FALSE"
            com_mainfunction_Info[mainfunction_rx]["RxToutmessagStartIdx"] = "NO_PDU_REFERENCE"
            com_mainfunction_Info[mainfunction_rx]["RxToutmessagNum"] = 0

            # 映射 RxToutSignal数组
            com_mainfunction_Info[mainfunction_rx]["IsRxToutSignalStartIdx"] = "FALSE"
            com_mainfunction_Info[mainfunction_rx]["RxToutSignalStartIdx"] = "NO_PDU_REFERENCE"
            com_mainfunction_Info[mainfunction_rx]["RxToutSignalNum"] = 0

            com_mainfunction_Info[mainfunction_rx]["StartRxIPduIdx"] = StartRxIPduIdx
            if coreId in ComIpdus_sort.keys():
                for ipdu in list(ComIpdus_sort[coreId].keys()):
                    IPduMainFunctionRef = ComIpdus_sort[coreId][ipdu]['IPduMainFunctionRef']['ValueRef']
                    IPduMainFunctionRef = IPduMainFunctionRef[IPduMainFunctionRef.rfind("/") + 1:]
                    if IPduMainFunctionRef == mainfunction_rx:
                        ComIpdus_sort[coreId][ipdu]["application"] =  com_mainfunction_Info[mainfunction_rx]["application"]
                        ComConfig["ComIPdus"][coreId].update({ipdu: ComIpdus_sort[coreId][ipdu]})
                        StartRxIPduIdx += 1
                        RxIPduNum += 1
            if RxIPduNum == 0:
                com_mainfunction_Info[mainfunction_rx]["StartRxIPduIdx"] = "NO_PDU_REFERENCE"
            
            com_mainfunction_Info[mainfunction_rx]["RxIPduNum"] = RxIPduNum
            RxIPduNum = 0
        ComGeneral["mainfunction_num"][coreId]["mainfunction_rx_num"] = mainfunction_rx_num


def handle_Ipdusort_Depend_ComIPduHandleId(ComConfig: dict) -> dict:
    com_ipdus_list = []
    for ipdu_name, ipdu_cfg in ComConfig.items():
        ipdu = {}
        ipdu.update(ipdu_cfg)
        ipdu["Name"] = ipdu_name
        com_ipdus_list.append(ipdu)
    com_ipdus_list.sort(key=lambda ipdu: ipdu["ComIPduHandleId"]["Value"])

    ComIpdus_sort = {}
    for i in range(len(com_ipdus_list)):
        ComIpdus_sort[com_ipdus_list[i].get("Name")] = com_ipdus_list[i]
    return ComIpdus_sort


def handle_Ipdusort_Depend_ComIPduGroupHandleId(ComConfig: dict) -> dict:
    com_ipduGroup_list = []
    for ipdugroup_name, ipdugroup_cfg in ComConfig.items():
        ipdugroup = {}
        ipdugroup.update(ipdugroup_cfg)
        ipdugroup["Name"] = ipdugroup_name
        com_ipduGroup_list.append(ipdugroup)
    com_ipduGroup_list.sort(key=lambda ipdu: ipdu["ComIPduGroupHandleId"]["Value"])

    ComIpduGroups_sort = {}
    for i in range(len(com_ipduGroup_list)):
        ComIpduGroups_sort[com_ipduGroup_list[i].get("Name")] = com_ipduGroup_list[i]
    return ComIpduGroups_sort


def handle_Signalsort_Depend_SignalId(Com_Signal: dict) -> dict:
    com_signals_list = []
    for signal_name, signal_cfg in Com_Signal.items():
        signal = {}
        signal.update(signal_cfg)
        signal["Name"] = signal_name
        com_signals_list.append(signal)
        # stub
        if "signal_id" not in signal:
            signal["Name"] = signal_name
    com_signals_list.sort(key=lambda signal: signal["signal_id"])

    Com_Signal_sort = {}
    for i in range(len(com_signals_list)):
        Com_Signal_sort[com_signals_list[i].get("Name")] = com_signals_list[i]
    return Com_Signal_sort


def handle_Str_To_List(ComIpdus: dict):
    Templist = []
    for ipdu in list(ComIpdus.keys()):
        if isinstance(ComIpdus[ipdu]["ComIPduSignalRef"]["ValueRef"], str):
            Templist.append(ComIpdus[ipdu]["ComIPduSignalRef"]["ValueRef"])
            ComIpdus[ipdu]["ComIPduSignalRef"]["ValueRef"] = Templist.copy()
            Templist.clear()
        if isinstance(ComIpdus[ipdu]["ComIPduGroupRef"]["ValueRef"], str):
            Templist.append(ComIpdus[ipdu]["ComIPduGroupRef"]["ValueRef"])
            ComIpdus[ipdu]["ComIPduGroupRef"]["ValueRef"] = Templist.copy()
            Templist.clear()
        if isinstance(ComIpdus[ipdu]["ComIPduSignalGroupRef"]["ValueRef"], str):
            Templist.append(ComIpdus[ipdu]["ComIPduSignalGroupRef"]["ValueRef"])
            ComIpdus[ipdu]["ComIPduSignalGroupRef"]["ValueRef"] = Templist.copy()
            Templist.clear()


def handle_ecuc_ref(ComIpdus_sort: dict, ComSignal_new: dict, EcuC: dict):
    for IPdu in list(ComIpdus_sort.keys()):
        ComPduRef = ComIpdus_sort[IPdu]["ComPduIdRef"]["ValueRef"]
        ComPduRef_name = ComPduRef[ComPduRef.rfind("/") + 1:]
        # keep message name correct
        ComIpdus_sort[IPdu]["MessageName"] = ComPduRef_name
        EcucPduCoreId = 0
        
        if ComPduRef_name in EcuC["EcucConfigSet"]["EcucPduCollection"]:
            ComIpdus_sort[IPdu]["PduRef"] = EcuC["EcucConfigSet"]["EcucPduCollection"][ComPduRef_name]
            # 提取报文长度
            ComIpdus_sort[IPdu]["ComIPduSize"] = ComIpdus_sort[IPdu]["PduRef"]["PduLength"]["Value"]
        
            # 提取IPDU的CoreRef信息
            if "EcucPduDefaultCoreRef" in ComIpdus_sort[IPdu]["PduRef"].keys():
                EcucPduDefaultCoreRef = ComIpdus_sort[IPdu]["PduRef"]["EcucPduDefaultCoreRef"]["ValueRef"]
                if isinstance(EcucPduDefaultCoreRef, list):
                    EcucPduDefaultCoreRef = EcucPduDefaultCoreRef[0]
            else:
                EcucPduDefaultCoreRef = None

            if EcucPduDefaultCoreRef in {None , "" ," "}:
                # 当前CoreRef未映射 则默认是Core0
                ComIpdus_sort[IPdu]["CoreRef"] = 0
            else:
                # 提取/最后的字符
                EcucPduDefaultCore = EcucPduDefaultCoreRef[EcucPduDefaultCoreRef.rfind("/") + 1:]
                # 获取CoreId
                EcucPduCoreId = EcuC['EcucHardware'][EcucPduDefaultCore]['EcucCoreId']['Value']
                ComIpdus_sort[IPdu]["CoreRef"] = EcucPduCoreId
        if isinstance(ComIpdus_sort[IPdu]["ComIPduSignalRef"]["ValueRef"], list):
            for signal in list(ComIpdus_sort[IPdu]["ComIPduSignalRef"]["ValueRef"]):
                signal_name = signal[signal.rfind("/") + 1:]
                # Signal对应的CoreId和IPDU一致
                ComSignal_new[signal_name]["CoreRef"] = EcucPduCoreId

def del_useless_info(Parent: dict):
    if "Description" in Parent.keys():
        del Parent["Description"]
    if "Tag" in Parent.keys():
        del Parent["Tag"]
    if "DefinitionRef" in Parent.keys():
        del Parent["DefinitionRef"]
    for key in Parent.keys():
        res = isinstance(Parent[key], dict)
        # print(key, res, type(Parent[key]))
        if isinstance(Parent[key], dict):
            del_useless_info(Parent[key])


def del_json_description(Config_dict: dict):
    ComConfig = Config_dict["ComConfig"]
    ComGeneral = Config_dict["ComGeneral"]

    del_useless_info(ComConfig)
    del_useless_info(ComGeneral)


#将默认的ComSignals的信号组属性都填充为FALSE
def handle_SgInfo_Signal(ComSignal: dict):
    for signal in list(ComSignal.keys()):
        dict_tempValueTRUE = {"Value": "TRUE"}
        dict_tempValueFALSE = {"Value": "FALSE"}
        ComSignal[signal]["IsSignalGroup"] = dict_tempValueFALSE
        ComSignal[signal]["IsGroupSignal"] = dict_tempValueFALSE

def handle_invalid_Signal(ComConfig: dict, signal: str):
    """
    过滤掉无效报文（诊断报文等）
    """
    pre_condition = ["LIN"]
    invalid_signal = ["MasterReq", "SlaveResp"]
    for invalid_item in invalid_signal:
        if signal.find(invalid_item) != -1 and signal.find(pre_condition[0]) != -1:
            del ComConfig["ComSignals"][signal]


def handle_invalid_message(ComConfig: dict, IPdu: str, coreId) -> bool:
    """
    过滤掉无效报文（诊断报文等）
    """
    pre_condition = ["LIN"]
    invalid_message = ["Req", "Resp"]
    for invalid_item in invalid_message:
        if IPdu.find(invalid_item) != -1 and IPdu.find(pre_condition[0]) != -1:
            for signal in ComConfig["ComIPdus"][coreId][IPdu]["ComIPduSignalRef"]["ValueRef"]:
                signal_name = signal[signal.rfind("/") + 1:]
                del ComConfig["ComSignals"][coreId][signal_name]

            del ComConfig["ComIPdus"][coreId][IPdu]
            return True
    return False


def handle_message_direction(ComGeneral: dict, ComConfig: dict, IPdu: str, TxIpdus, TxIpdu_id, coreId) -> int:
    if ComConfig["ComIPdus"][coreId][IPdu]["ComIPduDirection"]["Value"] == "SEND":
        ComConfig["ComIPdus"][coreId][IPdu]["direction"] = "COM_SEND"
        # Tx Message Id: the serial number in TxCfg array
        ComConfig["ComIPdus"][coreId][IPdu]["TxIpdu_id"] = TxIpdu_id
        # set default value
        if ComGeneral['ComRetryFailedTransmitRequests']['Value'] == False:
            ComGeneral["COM_IPDU_RETRYFAILEDTRANS_ENABLE"] = "STD_OFF"
            ComConfig["ComIPdus"][coreId][IPdu]["ComTxIPdu"]["EnableRetryFailedTransmitRequests"] = "FALSE"
        else:
            ComGeneral["COM_IPDU_RETRYFAILEDTRANS_ENABLE"] = "STD_ON"
            ComConfig["ComIPdus"][coreId][IPdu]["ComTxIPdu"]["EnableRetryFailedTransmitRequests"] = "TRUE"
        #set ComTxIPduClearUpdateBit to 大写字母
        ComConfig["ComIPdus"][coreId][IPdu]["ComTxIPdu"]["ComTxIPduClearUpdateBit"][
                "Value"] = "COM_" + ComConfig["ComIPdus"][coreId][IPdu]["ComTxIPdu"]["ComTxIPduClearUpdateBit"]["Value"]
        ComConfig["ComIPdus"][coreId][IPdu]["ComTxIPdu"]["ComTxIPduClearUpdateBit"]["Value"] = ComConfig["ComIPdus"][coreId][IPdu][
            "ComTxIPdu"]["ComTxIPduClearUpdateBit"]["Value"].upper()
        TxIpdu_id += 1
        TxIpdus.append(IPdu)
        if "PduRef" in ComConfig["ComIPdus"][coreId][IPdu].keys():
            ComGeneral["Txipdu_size"][coreId]["Txipdu_size"] += ComConfig["ComIPdus"][coreId][IPdu]["PduRef"]["PduLength"]["Value"]
        
        ComConfig["ComIPdus"][coreId][IPdu]["PduBuffStartIdx"] = ComGeneral["Txipdu_size_record"][coreId]["Txipdu_size_record"]
        
        if ComConfig["ComIPdus"][coreId][IPdu]["ComIPduSignalProcessing"]["Value"] == "IMMEDIATE":
            ComGeneral["COM_IPDU_ALL_DEFERRED"] = "STD_OFF"

        # 记录IPDU所属的Mainfunction
        IPduMainFunctionRef = ComConfig['ComIPdus'][coreId][IPdu]['IPduMainFunctionRef']['ValueRef']
        IPduMainFunctionRef = IPduMainFunctionRef[IPduMainFunctionRef.rfind("/") + 1:]
        
        ComIPduTxTimeBase = ComConfig['ComMainFunction_tx'][coreId][IPduMainFunctionRef]['MainTxTimeBase']['Value'] * 1000
        ComConfig["ComIPdus"][coreId][IPdu]["IPduTxTimeBase"] = ComIPduTxTimeBase
    else:
        # Rx Message's TxPduId will be -1 for Rx Message will not be placed at TxCfg array
        ComConfig["ComIPdus"][coreId][IPdu]["direction"] = "COM_RECEIVE"
        ComConfig["ComIPdus"][coreId][IPdu]["TxIpdu_id"] = -1
        if ComConfig["ComIPdus"][coreId][IPdu]["ComIPduSignalProcessing"]["Value"] == "DEFERRED":
            if "PduRef" in ComConfig["ComIPdus"][coreId][IPdu].keys():
                ComGeneral["Rxipdu_size"][coreId]["Rxipdu_size"] += ComConfig["ComIPdus"][coreId][IPdu]["PduRef"]["PduLength"]["Value"]

        # COM_RECEIVE DEFERRED IPDU
        # if ComConfig["ComIPdus"][IPdu]["ComIPduSignalProcessing"]["Value"] == "DEFERRED":
        #     if "PduRef" in ComConfig["ComIPdus"][IPdu].keys():
        #         ComGeneral["ipdu_size"] += ComConfig["ComIPdus"][IPdu]["PduRef"]["PduLength"]["Value"]
        #         # ComGeneral["Rxipdu_size"] += ComConfig["ComIPdus"][IPdu]["PduRef"]["PduLength"]["Value"]
        # else:
        #     ComGeneral["COM_IPDU_ALL_DEFERRED"] = "STD_OFF"
        if ComConfig["ComIPdus"][coreId][IPdu]["ComIPduSignalProcessing"]["Value"] != "DEFERRED":
            ComGeneral["COM_IPDU_ALL_DEFERRED"] = "STD_OFF"
        
        if ComConfig["ComIPdus"][coreId][IPdu]["ComIPduSignalProcessing"]["Value"] == "DEFERRED":
            ComGeneral["COM_RXIPDU_ALL_IMMEDIATE"] = "STD_OFF"
        
        ComConfig["ComIPdus"][coreId][IPdu]["PduBuffStartIdx"] = ComGeneral["Rxipdu_size_record"][coreId]["Rxipdu_size_record"]

    if ComConfig["ComIPdus"][coreId][IPdu]["ComIPduType"]["Value"] == 'TP':
        ComGeneral["COM_IPDU_TP_ENABLE"] = "STD_ON"
    return TxIpdu_id

def handle_message_periodic(ComConfig: dict, IPdu: str, coreId):
    if ComConfig["ComIPdus"][coreId][IPdu]["ComIPduDirection"]["Value"] == "SEND":
        #ComTxModeTimePeriod
        ComTxModeTimePeriod = ComConfig['ComIPdus'][coreId][IPdu]['ComTxIPdu']['ComTxModeTrue']['ComTxMode'][
            'ComTxModeTimePeriod']['Value'] * 1000

        IPduMainFunctionRef = ComConfig['ComIPdus'][coreId][IPdu]['IPduMainFunctionRef']['ValueRef']
        IPduMainFunctionRef = IPduMainFunctionRef[IPduMainFunctionRef.rfind("/") + 1:]
        
        ComIPduTxTimeBase = ComConfig['ComMainFunction_tx'][coreId][IPduMainFunctionRef]['MainTxTimeBase']['Value'] * 1000

        ComConfig['ComIPdus'][coreId][IPdu]['ComTxIPdu']['ComTxModeTrue']['ComTxMode']['ComTxModeTimePeriod']['Value'] = int(
            ComTxModeTimePeriod / ComIPduTxTimeBase)
        
        #给RTE信号代理提供的周期信息
        ComConfig['ComIPdus'][coreId][IPdu]["Rte_GetTxModeMode"] = ComConfig['ComIPdus'][coreId][IPdu]['ComTxIPdu']['ComTxModeTrue']['ComTxMode'].get('ComTxModeMode', {}).get("Value", "NONE")
        ComConfig['ComIPdus'][coreId][IPdu]["Rte_GetPeriodTime"] = int(ComTxModeTimePeriod)
        ComConfig['ComIPdus'][coreId][IPdu]["Rte_GetMainFunctionTime"] = int(ComIPduTxTimeBase)
        ComConfig['ComIPdus'][coreId][IPdu]["Rte_GetMainFunctionName"] = IPduMainFunctionRef

        #ComTxModeTimeOffset
        ComTxModeTimeOffset = ComConfig['ComIPdus'][coreId][IPdu]['ComTxIPdu']['ComTxModeTrue']['ComTxMode'][
            'ComTxModeTimeOffset']['Value'] * 1000
        ComConfig['ComIPdus'][coreId][IPdu]['ComTxIPdu']['ComTxModeTrue']['ComTxMode']['ComTxModeTimeOffset']['Value'] = int(
            ComTxModeTimeOffset / ComIPduTxTimeBase)

        #ComTxModeRepetitionPeriod
        ComTxModeRepetitionPeriod = ComConfig['ComIPdus'][coreId][IPdu]['ComTxIPdu']['ComTxModeTrue']['ComTxMode'][
            'ComTxModeRepetitionPeriod']['Value'] * 1000
        ComConfig['ComIPdus'][coreId][IPdu]['ComTxIPdu']['ComTxModeTrue']['ComTxMode']['ComTxModeRepetitionPeriod'][
            'Value'] = int(ComTxModeRepetitionPeriod / ComIPduTxTimeBase)

        #ComMinimumDelayTime
        ComTxModeRepetitionPeriod = ComConfig['ComIPdus'][coreId][IPdu]['ComTxIPdu']['ComMinimumDelayTime']['Value'] * 1000
        ComConfig['ComIPdus'][coreId][IPdu]['ComTxIPdu']['ComMinimumDelayTime']['Value'] = int(ComTxModeRepetitionPeriod /
                                                                                       ComIPduTxTimeBase)

        #ComTxModeTimePeriod
        ComTxModeTimePeriod = ComConfig['ComIPdus'][coreId][IPdu]['ComTxIPdu']['ComTxModeFalse']['ComTxMode'][
            'ComTxModeTimePeriod']['Value'] * 1000
        ComConfig['ComIPdus'][coreId][IPdu]['ComTxIPdu']['ComTxModeFalse']['ComTxMode']['ComTxModeTimePeriod']['Value'] = int(
            ComTxModeTimePeriod / ComIPduTxTimeBase)

        #ComTxModeTimeOffset
        ComTxModeTimeOffset = ComConfig['ComIPdus'][coreId][IPdu]['ComTxIPdu']['ComTxModeFalse']['ComTxMode'][
            'ComTxModeTimeOffset']['Value'] * 1000
        ComConfig['ComIPdus'][coreId][IPdu]['ComTxIPdu']['ComTxModeFalse']['ComTxMode']['ComTxModeTimeOffset']['Value'] = int(
            ComTxModeTimeOffset / ComIPduTxTimeBase)

        #ComTxModeRepetitionPeriod
        ComTxModeRepetitionPeriod = ComConfig['ComIPdus'][coreId][IPdu]['ComTxIPdu']['ComTxModeFalse']['ComTxMode'][
            'ComTxModeRepetitionPeriod']['Value'] * 1000
        ComConfig['ComIPdus'][coreId][IPdu]['ComTxIPdu']['ComTxModeFalse']['ComTxMode']['ComTxModeRepetitionPeriod'][
            'Value'] = int(ComTxModeRepetitionPeriod / ComIPduTxTimeBase)

        #ComTxModeNumberOfRepetitions
        if (ComConfig['ComIPdus'][coreId][IPdu]['ComTxIPdu']['ComTxModeTrue']['ComTxMode']['ComTxModeNumberOfRepetitions']
            ['Value'] > 0):
            ComConfig['ComIPdus'][coreId][IPdu]['ComTxIPdu']['ComTxModeTrue']['ComTxMode']['ComTxModeNumberOfRepetitions'][
                'Value'] += 1

        if (ComConfig['ComIPdus'][coreId][IPdu]['ComTxIPdu']['ComTxModeFalse']['ComTxMode']['ComTxModeNumberOfRepetitions']
            ['Value'] > 0):
            ComConfig['ComIPdus'][coreId][IPdu]['ComTxIPdu']['ComTxModeFalse']['ComTxMode']['ComTxModeNumberOfRepetitions'][
                'Value'] += 1

def handle_messag_Signal_StartIdx_from_direction(ComConfig: dict, IPdu: str, signal_id_tx, signal_id_rx, coreId):
    if ComConfig["ComIPdus"][coreId][IPdu]["ComIPduDirection"]["Value"] == "SEND":
        ComConfig["ComIPdus"][coreId][IPdu]["Signal_StartIdx"] = signal_id_tx
    else:
        ComConfig["ComIPdus"][coreId][IPdu]["Signal_StartIdx"] = signal_id_rx

def handle_MainFunctionRx_to_RxMessageTOutInfo(ComGeneral: dict, ComConfig: dict):
    RxToutmessagStartIdx = 0
    for mainfunction_rx in list(ComConfig["ComMainFunction_rx_totle"].keys()):
        for IPdu in list(ComConfig["ComRxIPdus_totle"].keys()):
            # Com_MainFunctionRxStruct关联ComRxMessageTOutInfo的startidx
            IPduMainFunctionRef =  ComConfig["ComRxIPdus_totle"][IPdu]['IPduMainFunctionRef']['ValueRef']
            IPduMainFunctionRef = IPduMainFunctionRef[IPduMainFunctionRef.rfind("/") + 1:]
            if IPduMainFunctionRef == mainfunction_rx:
                if ComConfig["ComIPdus_totle"][IPdu]["ISRXTIMEOUT"] == "TRUE":
                    if ComConfig["ComMainFunction_rx_totle"][IPduMainFunctionRef]["IsRxToutmessagStartIdx"] == "FALSE":
                        ComConfig["ComMainFunction_rx_totle"][IPduMainFunctionRef]["RxToutmessagStartIdx"] = RxToutmessagStartIdx
                        ComConfig["ComMainFunction_rx_totle"][IPduMainFunctionRef]["IsRxToutmessagStartIdx"] = "TRUE"
                    ComConfig["ComMainFunction_rx_totle"][IPduMainFunctionRef]["RxToutmessagNum"] += 1
                    RxToutmessagStartIdx += 1

def handle_MainFunctionRx_RxNotificationRecord(ComGeneral: dict, ComConfig: dict, app_coreid_dict:dict):
    for coreId, com_mainfunction_Info in ComConfig["ComMainFunction_rx"].items():
        mainfunction_rx_num = 0
        for mainfunction_rx in list(com_mainfunction_Info.keys()):
            # MainFunction 对应的RxNotificationRecord
            com_mainfunction_Info[mainfunction_rx]["RxNotificationRecordRef"] = "RxNotificationRecordRef" + "_Core" + str(coreId) + "_Mf" + str(mainfunction_rx_num)
            com_mainfunction_Info[mainfunction_rx]["Ipdu_MaxSig_Num"] = 0
            com_mainfunction_Info[mainfunction_rx]["START_memmapstr"] = "#define COM_START_SEC_" + app_coreid_dict[coreId] + "_PRIVATE_BSS"
            com_mainfunction_Info[mainfunction_rx]["STOP_memmapstr"] = "#define COM_STOP_SEC_" + app_coreid_dict[coreId] + "_PRIVATE_BSS"
            mainfunction_rx_num += 1
            for ipdu in list(ComConfig["ComRxIPdus"][coreId].keys()):
                IPduMainFunctionRef = ComConfig["ComRxIPdus"][coreId][ipdu]['IPduMainFunctionRef']['ValueRef']
                IPduMainFunctionRef = IPduMainFunctionRef[IPduMainFunctionRef.rfind("/") + 1:]
                if IPduMainFunctionRef == mainfunction_rx:
                    # 获取该MainFunction对应的IPDU最大的Signal个数
                    MainFunction_Ipdu_MaxSig_Num = len(ComConfig["ComRxIPdus"][coreId][ipdu]["ComIPduSignalRef"]["ValueRef"])
                    if com_mainfunction_Info[mainfunction_rx]["Ipdu_MaxSig_Num"] < MainFunction_Ipdu_MaxSig_Num:
                        com_mainfunction_Info[mainfunction_rx]["Ipdu_MaxSig_Num"] = MainFunction_Ipdu_MaxSig_Num
            if com_mainfunction_Info[mainfunction_rx]["Ipdu_MaxSig_Num"] == 0:
                com_mainfunction_Info[mainfunction_rx]["RxNotificationRecordRef"] = "NULL"
            else:
                RxNotificationRecordRef = com_mainfunction_Info[mainfunction_rx]["RxNotificationRecordRef"]
                if ComGeneral["Com_Support_MutiCore"] == "STD_OFF":
                    com_mainfunction_Info[mainfunction_rx]["RxNotificationRecord"] = "VAR(uint16, COM_PRIVATE_BSS) " + RxNotificationRecordRef + "[" + str(com_mainfunction_Info[mainfunction_rx]["Ipdu_MaxSig_Num"]) + "]"
                else: 
                    com_mainfunction_Info[mainfunction_rx]["RxNotificationRecord"] = "VAR(uint16, COM_" + app_coreid_dict[coreId]  + "_PRIVATE_BSS) " + RxNotificationRecordRef + "[" + str(com_mainfunction_Info[mainfunction_rx]["Ipdu_MaxSig_Num"]) + "]"

def handle_MainFunctionRx_to_ComRxSignalTOutInfo(ComGeneral: dict, ComConfig: dict):
    RxToutSignalStartIdx = 0
    for mainfunction_rx in list(ComConfig["ComMainFunction_rx_totle"].keys()):
        for Signal in list(ComConfig["ComRxSignals_totle"].keys()):
            # Com_MainFunctionRxStruct关联ComSignalTOutInfo的startidx
            IPdu = ComConfig["ComRxSignals_totle"][Signal]["message_name"]
            IPduMainFunctionRef =  ComConfig["ComRxIPdus_totle"][IPdu]['IPduMainFunctionRef']['ValueRef']
            IPduMainFunctionRef = IPduMainFunctionRef[IPduMainFunctionRef.rfind("/") + 1:]
            if IPduMainFunctionRef == mainfunction_rx:
                if ComConfig["ComRxSignals_totle"][Signal]["isSignalTOut"] == "TRUE":
                    if ComConfig["ComMainFunction_rx_totle"][IPduMainFunctionRef]["IsRxToutSignalStartIdx"] == "FALSE":
                        ComConfig["ComMainFunction_rx_totle"][IPduMainFunctionRef]["RxToutSignalStartIdx"] = RxToutSignalStartIdx
                        ComConfig["ComMainFunction_rx_totle"][IPduMainFunctionRef]["IsRxToutSignalStartIdx"] = "TRUE"
                    ComConfig["ComMainFunction_rx_totle"][IPduMainFunctionRef]["RxToutSignalNum"] += 1
                    RxToutSignalStartIdx += 1
                       

def handle_messag_from_direction(ComGeneral: dict, ComConfig: dict, IPdu: str, message_id_tx, message_id_rx, signal_id_tx, signal_id_rx, coreId):
    if ComConfig["ComIPdus"][coreId][IPdu]["ComIPduDirection"]["Value"] == "SEND":
        ComConfig["ComIPdus"][coreId][IPdu]["Signal_EndIdx"] = signal_id_tx
        ComConfig["ComIPdus"][coreId][IPdu]["message_id_trx"] = message_id_tx
        message_id_tx += 1
        ComGeneral["message_id_tx"][coreId]["message_id_tx"] += 1
        if ComConfig["ComIPdus"][coreId][IPdu]["ISTXTIMEOUT"] == "TRUE":
            ComConfig["ComIPdus"][coreId][IPdu]["TxTimeoutIndex"] = ComGeneral["TxToutmessage_size"]
            ComGeneral["TxToutmessage_size"] += 1
        ComConfig["ComTxIPdus"][coreId].update({IPdu: ComConfig["ComIPdus"][coreId][IPdu]})
    else:
        ComConfig["ComIPdus"][coreId][IPdu]["Signal_EndIdx"] = signal_id_rx
        ComConfig["ComIPdus"][coreId][IPdu]["message_id_trx"] = message_id_rx
        message_id_rx += 1
        ComGeneral["message_id_rx"][coreId]["message_id_rx"] += 1
        if ComConfig["ComIPdus"][coreId][IPdu]["ISRXTIMEOUT"] == "TRUE":
            ComConfig["ComIPdus"][coreId][IPdu]["RxTimeoutIndex"] = ComGeneral["RxToutmessage_size"]
            ComGeneral["RxToutmessage_size"] += 1
        ComConfig["ComRxIPdus"][coreId].update({IPdu: ComConfig["ComIPdus"][coreId][IPdu]})
    return message_id_tx, message_id_rx


def handle_message_info(ComGeneral:dict, ComConfig: dict, IPdu: str, message_id, coreId):
    # every message will get an message id
    ComConfig["ComIPdus"][coreId][IPdu]["message_id"] = message_id
    # signal reference
    new_signal_ref = []
    if ComConfig["ComIPdus"][coreId][IPdu]["ComIPduSignalRef"]["ValueRef"] != None:
        for signal in list(ComConfig["ComIPdus"][coreId][IPdu]["ComIPduSignalRef"]["ValueRef"]):
            new_signal_ref.append(signal[signal.rfind("/") + 1:])
    ComConfig["ComIPdus"][coreId][IPdu]["ComIPduSignalRef"]["ValueRef"] = new_signal_ref

    # IPduGroupRefs - remove preffix

    ComConfig["ComIPdus"][coreId][IPdu]["ComIPduGroupNum"] = len(ComConfig["ComIPdus"][coreId][IPdu]["ComIPduGroupRef"]["ValueRef"])
    new_IpduGroup_ref = []
    for ComIPduGroupId in list(ComConfig["ComIPdus"][coreId][IPdu]["ComIPduGroupRef"]["ValueRef"]):
        ComIPduGroupId = ComIPduGroupId[ComIPduGroupId.rfind("/") + 1:]
        new_IpduGroup_ref.append(ComIPduGroupId)

    ComConfig["ComIPdus"][coreId][IPdu]["ComIPduGroupRef"]["ValueRef"] = new_IpduGroup_ref

    if ComConfig["ComIPdus"][coreId][IPdu]["ComIPduSignalProcessing"]["Value"][0:3] != "COM":
        ComConfig["ComIPdus"][coreId][IPdu]["ComIPduSignalProcessing"][
            "Value"] = "COM_" + ComConfig["ComIPdus"][coreId][IPdu]["ComIPduSignalProcessing"]["Value"]

    # init value - rout req
    ComConfig["ComIPdus"][coreId][IPdu]["ComIPduGwRoutingReq"] = "FALSE"
    # init value - ipducallout
    if "ComIPduCallout" in ComConfig["ComIPdus"][coreId][IPdu]:
        if ComConfig["ComIPdus"][coreId][IPdu]["ComIPduCallout"]["Value"] == "" or ComConfig["ComIPdus"][coreId][IPdu][
                "ComIPduCallout"]["Value"] == " " or ComConfig["ComIPdus"][coreId][IPdu]["ComIPduCallout"]["Value"] == None:
            ComConfig["ComIPdus"][coreId][IPdu]["ComIPduCallout"]["Value"] = "COM_NO_FUNCTION_CALLOUT"
        if ComConfig["ComIPdus"][coreId][IPdu]["ComIPduTriggerTransmitCallout"]["Value"] == "" or ComConfig["ComIPdus"][coreId][IPdu][
                "ComIPduTriggerTransmitCallout"]["Value"] == " " or ComConfig["ComIPdus"][coreId][IPdu][
                    "ComIPduTriggerTransmitCallout"]["Value"] == None:
            ComConfig["ComIPdus"][coreId][IPdu]["ComIPduTriggerTransmitCallout"]["Value"] = "COM_NO_FUNCTION_CALLOUT"
    message_id += 1
    ComGeneral["message_id"][coreId]["message_id"] += 1
    return message_id


def handle_callout(ComConfig: dict, IPdu: str, rx_callout_id: int, tx_callout_id: int, tt_callout_id: int, coreId):
    """
    handle callout
    """
    ComConfig["ComIPdus"][coreId][IPdu]["ComRxIPduCallout"] = "COM_NO_FUNCTION_CALLOUT"
    ComConfig["ComIPdus"][coreId][IPdu]["ComTxIPduCallout"] = "COM_NO_FUNCTION_CALLOUT"
    ComConfig["ComIPdus"][coreId][IPdu]["ComTriggerTransmitIPduCallout"] = "COM_NO_FUNCTION_CALLOUT"

    if "ComIPduCallout" in ComConfig["ComIPdus"][coreId][IPdu].keys():
        if ComConfig["ComIPdus"][coreId][IPdu]["ComIPduCallout"]["Value"] != "COM_NO_FUNCTION_CALLOUT":
            if ComConfig["ComIPdus"][coreId][IPdu]["direction"] == "COM_SEND":
                ComConfig["ComIPdus"][coreId][IPdu]["ComTxIPduCallout"] = ComConfig["ComIPdus"][coreId][IPdu]["ComIPduCallout"]["Value"]
                ComConfig["ComIPdus"][coreId][IPdu]["ComTxIPduCalloutId"] = tx_callout_id
                tx_callout_id += 1
            else:
                ComConfig["ComIPdus"][coreId][IPdu]["ComRxIPduCallout"] = ComConfig["ComIPdus"][coreId][IPdu]["ComIPduCallout"]["Value"]
                ComConfig["ComIPdus"][coreId][IPdu]["ComRxIPduCalloutId"] = rx_callout_id
                rx_callout_id += 1

        if ComConfig["ComIPdus"][coreId][IPdu]["ComIPduTriggerTransmitCallout"]["Value"] != "COM_NO_FUNCTION_CALLOUT":
            ComConfig["ComIPdus"][coreId][IPdu]["ComTriggerTransmitIPduCallout"] = ComConfig["ComIPdus"][coreId][IPdu][
                "ComIPduTriggerTransmitCallout"]["Value"]
            ComConfig["ComIPdus"][coreId][IPdu]["ComTriggerTransmitIPduCalloutId"] = tt_callout_id
            tt_callout_id += 1
    return rx_callout_id, tx_callout_id, tt_callout_id


# def handle_update_bit(ComConfig:dict, IPdu:str, signal:str):
#     update_bit_signal = ""

#     if signal[-6:-6] == "_":
#         # signal end with _UB
#         update_bit_signal = signal[:-6] + signal[-3:]
#     else:
#         # signal end with signalnameUB_Rx or signalnameUB_Tx
#         update_bit_signal = signal[:-5] + signal[-3:]

#     # print("signal", signal, "  update_bit_signal", update_bit_signal)
#     ComConfig["ComSignals"][update_bit_signal]["UseUpdateBit"] = "TRUE"
#     ComConfig["ComSignals"][update_bit_signal]["UpdateBitPosition"]["Value"] = ComConfig[
#         "ComSignals"][signal]["BitPosition"]["Value"]
#     del ComConfig["ComIPdus"][signal]


def handle_update_bit(ComGeneral: dict, ComConfig: dict, IPdu: str, signal: str, coreId):
    if ComConfig["ComSignals"][coreId][signal]["ComUpdateBitPosition"]["Value"] != 0:
        ComGeneral["COM_SIGNAL_UPDATEBIT_ENABLE"] = "STD_ON"
        ComConfig["ComSignals"][coreId][signal]["UseUpdateBit"] = "TRUE"

def handle_rxsignal_type_Acc(ComGeneral: dict, ComConfig: dict, IPdu: str, signal: str, coreId):
    SignalType = ComConfig["ComSignals"][coreId][signal]["ComSignalType"]["Value"]
    if SignalType == "COM_UINT8_N" or SignalType == "COM_UINT8_DYN":
        ComGeneral["COM_RXSIGNAL_UINT8_N_ENABLE"] = "STD_ON"
    elif SignalType == "COM_UINT8" or SignalType == "COM_SINT8" or SignalType == "COM_BOOLEAN":
        ComGeneral["COM_RXSIGNAL_UINT8_ENABLE"] = "STD_ON"
    elif SignalType == "COM_UINT16" or SignalType == "COM_SINT16":
        ComGeneral["COM_RXSIGNAL_UINT16_ENABLE"] = "STD_ON"
    elif SignalType == "COM_UINT32" or SignalType == "COM_SINT32" or SignalType == "COM_FLOAT32":
        ComGeneral["COM_RXSIGNAL_UINT16_ENABLE"] = "STD_ON"
    else:
        ComGeneral["COM_RXSIGNAL_UINT64_ENABLE"] = "STD_ON"
        
    

def handle_signal_from_direction(ComGeneral: dict, ComConfig: dict, IPdu: str, signal: str, signal_id_tx, signal_id_rx, coreId):
    if ComConfig["ComIPdus"][coreId][IPdu]["ComIPduDirection"]["Value"] == "SEND":
        ComConfig["ComTxSignals"][coreId].update({signal: ComConfig["ComSignals"][coreId][signal]})
        ComConfig["ComTxSignals"][coreId][signal]["signal_id_trx"] = signal_id_tx

        # 将IPDU的周期属性放到信号中  供Rte使用
        ComConfig["ComTxSignals"][coreId][signal]["Rte_GetPeriodTime"] = ComConfig['ComIPdus'][coreId][IPdu]["Rte_GetPeriodTime"]
        ComConfig["ComTxSignals"][coreId][signal]["Rte_GetTxModeMode"] = ComConfig['ComIPdus'][coreId][IPdu]["Rte_GetTxModeMode"]
        ComConfig["ComTxSignals"][coreId][signal]["Rte_GetMainFunctionTime"] = ComConfig['ComIPdus'][coreId][IPdu]["Rte_GetMainFunctionTime"]
        ComConfig["ComTxSignals"][coreId][signal]["Rte_GetMainFunctionName"] = ComConfig['ComIPdus'][coreId][IPdu]["Rte_GetMainFunctionName"]
        
        signal_id_tx += 1
        ComGeneral["signal_id_tx"][coreId]["signal_id_tx"] += 1
        handle_SignalBytePosition(ComConfig, signal, ComGeneral["Txipdu_size_record"][coreId]["Txipdu_size_record"], coreId)
        #Notification
        if ComConfig["ComTxSignals"][coreId][signal]["ComNotification"]["Value"] != "COM_NO_FUNCTION_CALLOUT":
            ComConfig["ComTxSignals"][coreId][signal]["ComNotification_id_tx"] = ComGeneral["ComNotification_id_tx"]
            # 记录IPdu中信号 对应的第一个ComNotification_id
            if ComConfig["ComIPdus"][coreId][IPdu]["IsComNotification"] == "FALSE":
                ComConfig["ComIPdus"][coreId][IPdu]["StartNoti_id"] = ComGeneral["ComNotification_id_tx"]
            ComConfig["ComIPdus"][coreId][IPdu]["Noti_id_num"] += 1

            ComGeneral["ComNotification_id_tx"] += 1
            ComConfig["ComIPdus"][coreId][IPdu]["IsComNotification"] = "TRUE"

            #确认当前的mainfunction id下有配置tx confirmation
            IPduMainFunctionRef =  ComConfig["ComIPdus"][coreId][IPdu]['IPduMainFunctionRef']['ValueRef']
            IPduMainFunctionRef = IPduMainFunctionRef[IPduMainFunctionRef.rfind("/") + 1:]
            ComConfig["ComMainFunction_tx"][coreId][IPduMainFunctionRef]["TxComfirmationInfo"] = "TRUE"
        
        #Timeout Notification
        if ComConfig["ComTxSignals"][coreId][signal]["ComTimeoutNotification"]["Value"] != "COM_NO_FUNCTION_CALLOUT":
            ComConfig["ComTxSignals"][coreId][signal]["ComTimeoutNotification_id_tx"] = ComGeneral["ComTimeoutNotification_id_tx"]
            # 记录IPdu中信号 对应的第一个ComTimeoutNotification_id
            if ComConfig["ComIPdus"][coreId][IPdu]["IsComTimeoutNotification"] == "FALSE":
                ComConfig["ComIPdus"][coreId][IPdu]["StartTimeoutNoti_id"] = ComGeneral["ComTimeoutNotification_id_tx"]
            ComConfig["ComIPdus"][coreId][IPdu]["TimeoutNoti_id_num"] += 1
            
            ComGeneral["ComTimeoutNotification_id_tx"] += 1
            ComConfig["ComIPdus"][coreId][IPdu]["IsComTimeoutNotification"] = "TRUE"

        #Invalid Notification
        if ComConfig["ComTxSignals"][coreId][signal]["ComInvalidNotification"]["Value"] != "COM_NO_FUNCTION_CALLOUT":
            ComConfig["ComTxSignals"][coreId][signal]["ComInvalidNotification_id_tx"] = ComGeneral["ComInvalidNotification_id_tx"]
            # 记录IPdu中信号 对应的第一个ComTimeoutNotification_id
            if ComConfig["ComIPdus"][coreId][IPdu]["IsComInvalidNotification"] == "FALSE":
                ComConfig["ComIPdus"][coreId][IPdu]["StartInvalidNoti_id"] = ComGeneral["ComInvalidNotification_id_tx"]
            ComConfig["ComIPdus"][coreId][IPdu]["InvalidNoti_id_num"] += 1

            ComGeneral["ComInvalidNotification_id_tx"] += 1
            ComConfig["ComIPdus"][coreId][IPdu]["IsComInvalidNotification"] = "TRUE"
        
        #ComError Notification
        if ComConfig["ComTxSignals"][coreId][signal]["ComErrorNotification"]["Value"] != "COM_NO_FUNCTION_CALLOUT":
            ComConfig["ComTxSignals"][coreId][signal]["ComErrorNotification_id_tx"] = ComGeneral["ComErrorNotification_id_tx"]
            # 记录IPdu中信号 对应的第一个ComTimeoutNotification_id
            if ComConfig["ComIPdus"][coreId][IPdu]["IsComErrorNotification"] == "FALSE":
                ComConfig["ComIPdus"][coreId][IPdu]["StartErrorNoti_id"] = ComGeneral["ComErrorNotification_id_tx"]
            ComConfig["ComIPdus"][coreId][IPdu]["ErrorNoti_id_num"] += 1

            ComGeneral["ComErrorNotification_id_tx"] += 1
            ComConfig["ComIPdus"][coreId][IPdu]["IsComErrorNotification"] = "TRUE"
        
        #ComSignals ComTransferProperty
        if ComConfig["ComTxSignals"][coreId][signal]["ComTransferProperty"]["Value"] == "COM_TRIGGERED_ON_CHANGE_WITHOUT_REPETITION" or ComConfig["ComTxSignals"][coreId][signal][
            "ComTransferProperty"]["Value"] == "COM_TRIGGERED_ON_CHANGE":
            ComGeneral["COM_SIGNAL_ONCHANGE_ENABLE"] = "STD_ON"

    else:
        ComConfig["ComRxSignals"][coreId].update({signal: ComConfig["ComSignals"][coreId][signal]})
        ComConfig["ComRxSignals"][coreId][signal]["signal_id_trx"] = signal_id_rx
        signal_id_rx += 1
        ComGeneral["signal_id_rx"][coreId]["signal_id_rx"] += 1
        # rx signal buff num
        if ComConfig["ComRxSignals"][coreId][signal]["ComSignalType"]["Value"] in {"COM_UINT8" , "COM_SINT8" ,"COM_BOOLEAN"}:
            ComConfig["ComRxSignals"][coreId][signal]["BufferIdx"] = ComGeneral["uint8_bool_signal_num"][coreId]["uint8_bool_signal_num"]
            ComGeneral["uint8_bool_signal_num"][coreId]["uint8_bool_signal_num"] += 1
        if ComConfig["ComRxSignals"][coreId][signal]["ComSignalType"]["Value"] in {"COM_UINT16", "COM_SINT16"}:
            ComConfig["ComRxSignals"][coreId][signal]["BufferIdx"] = ComGeneral["uint16_signal_num"][coreId]["uint16_signal_num"]
            ComGeneral["uint16_signal_num"][coreId]["uint16_signal_num"] += 1
        if ComConfig["ComRxSignals"][coreId][signal]["ComSignalType"]["Value"] in {"COM_UINT32", "COM_FLOAT32", "COM_SINT32"}:
            ComConfig["ComRxSignals"][coreId][signal]["BufferIdx"] = ComGeneral["uint32_signal_num"][coreId]["uint32_signal_num"]
            ComGeneral["uint32_signal_num"][coreId]["uint32_signal_num"] += 1
        if ComConfig["ComRxSignals"][coreId][signal]["ComSignalType"]["Value"] in {"COM_UINT64", "COM_FLOAT64", "COM_SINT64"}:
            ComConfig["ComRxSignals"][coreId][signal]["BufferIdx"] = ComGeneral["uint64_signal_num"][coreId]["uint64_signal_num"]
            ComGeneral["uint64_signal_num"][coreId]["uint64_signal_num"] += 1
        if ComConfig["ComRxSignals"][coreId][signal]["ComSignalType"]["Value"] in {"COM_UINT8_N", "COM_UINT8_DYN"}:
            ComConfig["ComRxSignals"][coreId][signal]["BufferIdx"] = ComGeneral["uint8_N_signal_num"][coreId]["uint8_N_signal_num"]
            ComGeneral["uint8_N_signal_num"][coreId]["uint8_N_signal_num"] += ComConfig["ComRxSignals"][coreId][signal]["ComSignalLength"]["Value"]
        handle_SignalBytePosition(ComConfig, signal, ComGeneral["Rxipdu_size_record"][coreId]["Rxipdu_size_record"], coreId)

        #Notification
        if ComConfig["ComRxSignals"][coreId][signal]["ComNotification"]["Value"] != "COM_NO_FUNCTION_CALLOUT":
            ComConfig["ComRxSignals"][coreId][signal]["ComNotification_id_rx"] = ComGeneral["ComNotification_id_rx"]
            ComGeneral["ComNotification_id_rx"] += 1
            ComConfig["ComIPdus"][coreId][IPdu]["IsComNotification"] = "TRUE"
        
        #Timeout Notification
        if ComConfig["ComRxSignals"][coreId][signal]["ComTimeoutNotification"]["Value"] != "COM_NO_FUNCTION_CALLOUT":
            ComConfig["ComRxSignals"][coreId][signal]["ComTimeoutNotification_id_rx"] = ComGeneral["ComTimeoutNotification_id_rx"]

            # 记录IPdu中信号 对应的第一个ComTimeoutNotification_id
            if ComConfig["ComIPdus"][coreId][IPdu]["IsComTimeoutNotification"] == "FALSE":
                ComConfig["ComIPdus"][coreId][IPdu]["StartTimeoutNoti_id"] = ComGeneral["ComTimeoutNotification_id_rx"]
            ComConfig["ComIPdus"][coreId][IPdu]["TimeoutNoti_id_num"] += 1

            ComGeneral["ComTimeoutNotification_id_rx"] += 1
            ComConfig["ComIPdus"][coreId][IPdu]["IsComTimeoutNotification"] = "TRUE"

            

        #Invalid Notification
        if ComConfig["ComRxSignals"][coreId][signal]["ComInvalidNotification"]["Value"] != "COM_NO_FUNCTION_CALLOUT":
            ComConfig["ComRxSignals"][coreId][signal]["ComInvalidNotification_id_rx"] = ComGeneral["ComInvalidNotification_id_rx"]
            ComGeneral["ComInvalidNotification_id_rx"] += 1
            ComConfig["ComIPdus"][coreId][IPdu]["IsComInvalidNotification"] = "TRUE"
        
        #ComError Notification
        if ComConfig["ComRxSignals"][coreId][signal]["ComErrorNotification"]["Value"] != "COM_NO_FUNCTION_CALLOUT":
            ComConfig["ComRxSignals"][coreId][signal]["ComErrorNotification_id_rx"] = ComGeneral["ComErrorNotification_id_rx"]
            ComGeneral["ComErrorNotification_id_rx"] += 1
            ComConfig["ComIPdus"][coreId][IPdu]["IsComErrorNotification"] = "TRUE"
        
        # 判断信号类型
        handle_rxsignal_type_Acc(ComGeneral, ComConfig, IPdu, signal, coreId)
    return signal_id_tx, signal_id_rx


def handle_signal_notification(ComConfig: dict, IPdu: str, signal: str, signalNotification_id, coreId) -> int:
    # Rx&Tx Notification
    if ComConfig["ComSignals"][coreId][signal]["ComNotification"]["Value"] != "COM_NO_FUNCTION_CALLOUT":
        ComConfig["ComSignals"][coreId][signal]["signalNotification_id"] = signalNotification_id
        signalNotification_id += 1
    else:
        ComConfig["ComSignals"][coreId][signal]["signalNotification_id"] = -1

    # Timeout Notification
    if ComConfig["ComSignals"][coreId][signal]["ComTimeoutNotification"]["Value"] != "COM_NO_FUNCTION_CALLOUT":
        ComConfig["ComSignals"][coreId][signal]["signalTimeoutNotification_id"] = signalNotification_id
        signalNotification_id += 1
    else:
        ComConfig["ComSignals"][coreId][signal]["signalTimeoutNotification_id"] = -1

    # Invalid Notification
    if ComConfig["ComSignals"][coreId][signal]["ComInvalidNotification"]["Value"] != "COM_NO_FUNCTION_CALLOUT":
        ComConfig["ComSignals"][coreId][signal]["signalInvalidNotification_id"] = signalNotification_id
        signalNotification_id += 1
    else:
        ComConfig["ComSignals"][coreId][signal]["signalInvalidNotification_id"] = -1

    # Error Notification
    if ComConfig["ComSignals"][coreId][signal]["ComErrorNotification"]["Value"] != "COM_NO_FUNCTION_CALLOUT":
        ComConfig["ComSignals"][coreId][signal]["signalErrorNotification_id"] = signalNotification_id
        signalNotification_id += 1
    else:
        ComConfig["ComSignals"][coreId][signal]["signalErrorNotification_id"] = -1
    return signalNotification_id

def handle_ComGeneral_Value(ComGeneral: dict):

    ComGeneral["Com_Core_Num"] = 0

    ComGeneral["Com_MaxCore_Idx"] = 0

    ComGeneral["TxNotification_size"] = 0
    
    ComGeneral["mainfunction_rx_num_totle"] = 0
    ComGeneral["mainfunction_tx_num_totle"] = 0

    ComGeneral["Com_TxNotification"] = "STD_OFF"
    
    #统计是否配置updatebit 包括TX RX
    ComGeneral["COM_SIGNAL_UPDATEBIT_ENABLE"] = "STD_OFF"

    #统计是否报文都为DEFERRED 包括TX RX
    ComGeneral["COM_IPDU_ALL_DEFERRED"] = "STD_ON"

    #统计是否报文都为IMMEDIATE 只统计RX
    ComGeneral["COM_RXIPDU_ALL_IMMEDIATE"] = "STD_ON"

    #统计是否配置错误重传
    ComGeneral["COM_IPDU_RETRYFAILEDTRANS_ENABLE"] = "STD_OFF"

    #统计是否超时后处理
    ComGeneral["COM_TIMEOUT_DATA_ACTION_REPLACE_ENABLE"] = "STD_OFF"

    #统计是否非法值处理
    ComGeneral["COM_SIGNAL_DATAINVALIDVALUE_ENABLE"] = "STD_OFF"

    #统计是否配置信号组 包括TX RX
    ComGeneral["COM_SIGNAL_GROUP_ENABLE"] = "STD_OFF"
    
    #统计是否配置TP报文
    ComGeneral["COM_IPDU_TP_ENABLE"] = "STD_OFF"

    #统计是否配置动态信号
    ComGeneral["COM_IPDU_DYNC_SIGNAL_ENABLE"] = "STD_OFF"

    #统计是否配置错误通知
    ComGeneral["COM_ERROR_NOTIFICATION_ENABLE"] = "STD_OFF"

    # 统计接收信号超时
    ComGeneral["RxToutsignal_size"] = 0
    ComGeneral["COM_RXTOU_ENABLE"] = "STD_OFF"
    ComGeneral["COM_RXIPDU_SAME_TIMEOUT"] = "STD_ON"
    ComGeneral["RxToutmessage_size"] = 0

    # 统计发送信号超时
    ComGeneral["TxToutsignal_size"] = 0
    ComGeneral["COM_TXTOU_ENABLE"] = "STD_OFF"
    ComGeneral["COM_TXIPDU_SAME_TIMEOUT"] = "STD_ON"
    ComGeneral["TxToutmessage_size"] = 0

    #signal type num
    # ComGeneral["uint8_bool_signal_num"] = 0
    # ComGeneral["uint16_signal_num"] = 0    
    # ComGeneral["uint32_signal_num"] = 0  
    # ComGeneral["uint64_signal_num"] = 0
    # ComGeneral["uint8_N_signal_num"] = 0

    #signal type
    ComGeneral["COM_RXSIGNAL_UINT8_ENABLE"] = "STD_OFF"
    ComGeneral["COM_RXSIGNAL_UINT16_ENABLE"] = "STD_OFF"
    ComGeneral["COM_RXSIGNAL_UINT32_ENABLE"] = "STD_OFF"
    ComGeneral["COM_RXSIGNAL_UINT64_ENABLE"] = "STD_OFF"
    ComGeneral["COM_RXSIGNAL_UINT8_N_ENABLE"] = "STD_OFF"

    #Tx signal fifo
    ComGeneral["COM_TXSIGNAL_UINT8_FIFO"] = "STD_OFF"
    ComGeneral["COM_TXSIGNAL_UINT16_FIFO"] = "STD_OFF"
    ComGeneral["COM_TXSIGNAL_UINT32_FIFO"] = "STD_OFF"
    ComGeneral["COM_TXSIGNAL_UINT64_FIFO"] = "STD_OFF"
    ComGeneral["COM_TXSIGNAL_UINT8_N_FIFO"] = "STD_OFF"

    #signal transferProperty ON change 
    ComGeneral["COM_SIGNAL_ONCHANGE_ENABLE"] = "STD_OFF"

    #用于计算IPDU的最大信号个数
    ComGeneral["Ipdu_MaxSig_Num"] = 0

    #用于计算Notification timeoutnotification等的idx
    #TX
    ComGeneral["ComNotification_id_tx"] = 0
    ComGeneral["ComTimeoutNotification_id_tx"] = 0
    ComGeneral["ComInvalidNotification_id_tx"] = 0
    ComGeneral["ComErrorNotification_id_tx"] = 0

    #RX
    ComGeneral["ComNotification_id_rx"] = 0
    ComGeneral["ComTimeoutNotification_id_rx"] = 0
    ComGeneral["ComInvalidNotification_id_rx"] = 0
    ComGeneral["ComErrorNotification_id_rx"] = 0

    #判断是否有初始值为0的信号
    ComGeneral["ComSignalInitValueZero"] = "STD_OFF"

    # 单核情况下存去全量数据
    ComGeneral["SingleCore"] = {}

    ComGeneral["uint8_bool_signal_num_totle"] = 0
    ComGeneral["uint16_signal_num_totle"] = 0
    ComGeneral["uint32_signal_num_totle"] = 0
    ComGeneral["uint64_signal_num_totle"] = 0
    ComGeneral["uint8_N_signal_num_totle"] = 0


def handle_Signal_InitParameter(ComGeneral: dict):
    ComGeneral["Com_ConstValueUInt8Array"] = []
    ComGeneral["Com_ConstValueUInt8Num"] = 0
    ComGeneral["Com_ConstValueUInt16Array"] = []
    ComGeneral["Com_ConstValueUInt16Num"] = 0
    ComGeneral["Com_ConstValueUInt32Array"] = []
    ComGeneral["Com_ConstValueUInt32Num"] = 0
    ComGeneral["Com_ConstValueUInt64Array"] = []
    ComGeneral["Com_ConstValueUInt64Num"] = 0
    # ComConfig["ComSignals"]["Com_ConstValueUInt8_NArray"] = []
    ComGeneral["Com_ConstValueUInt8_NNum"] = 0


def handle_signal_info(ComGeneral: dict, ComConfig: dict, IPdu: str, signal: str, signal_id, coreId) -> int:
    # signal attribute: message_name
    ComConfig["ComSignals"][coreId][signal]["message_name"] = ComConfig["ComIPdus"][coreId][IPdu]["MessageName"]
    # signal attribute: signal_id
    ComConfig["ComSignals"][coreId][signal]["signal_id"] = signal_id
    # signal name
    SignalName = {}
    SignalName["Value"] = signal
    ComConfig["ComSignals"][coreId][signal]["SignalName"] = SignalName
    # signal attribute: update position
    if "UpdateBitPosition" in ComConfig["ComSignals"][coreId][signal]:
        if ComConfig["ComSignals"][coreId][signal]["UpdateBitPosition"]["Value"] is None:
            ComConfig["ComSignals"][coreId][signal]["UpdateBitPosition"]["Value"] = 0

    # init value - signal routing request
    ComConfig["ComSignals"][coreId][signal]["ComSigGwRoutingReq"] = "FALSE"

    # signal type
    ComConfig["ComSignals"][coreId][signal]["ComSignalType"][
        "Value"] = "COM_" + ComConfig["ComSignals"][coreId][signal]["ComSignalType"]["Value"]

    # signal ComSignalEndianness
    ComConfig["ComSignals"][coreId][signal]["ComSignalEndianness"][
        "Value"] = "COM_" + ComConfig["ComSignals"][coreId][signal]["ComSignalEndianness"]["Value"]


    # signal ComTransferProperty
    if ComConfig["ComSignals"][coreId][signal]["ComTransferProperty"]["Value"] in {'' , None }:
        ComConfig["ComSignals"][coreId][signal]["ComTransferProperty"]["Value"] = "PENDING"
    ComConfig["ComSignals"][coreId][signal]["ComTransferProperty"][
        "Value"] = "COM_" + ComConfig["ComSignals"][coreId][signal]["ComTransferProperty"]["Value"]

    # signal ComDataInavalidAction
    if ComConfig["ComSignals"][coreId][signal]["ComDataInvalidAction"]["Value"] in {'' , None }:
        ComConfig["ComSignals"][coreId][signal]["ComDataInvalidAction"]["Value"] = "NOTIFY"
    ComConfig["ComSignals"][coreId][signal]["ComDataInvalidAction"][
        "Value"] = "INVALID_" + ComConfig["ComSignals"][coreId][signal]["ComDataInvalidAction"]["Value"]

    # signal ComRxDataTimeoutAction
    # 当前支持配置成NONE和REPLACE
    if ComConfig["ComSignals"][coreId][signal]["ComRxDataTimeoutAction"]["Value"] == 'NONE' or ComConfig["ComSignals"][coreId][signal][
            "ComRxDataTimeoutAction"]["Value"] == 'REPLACE':
        if ComConfig["ComSignals"][coreId][signal][
            "ComRxDataTimeoutAction"]["Value"] == 'REPLACE':
            ComGeneral["COM_TIMEOUT_DATA_ACTION_REPLACE_ENABLE"] = "STD_ON"
        ComConfig["ComSignals"][coreId][signal]["ComRxDataTimeoutAction"][
            "Value"] = "COM_TIMEOUT_DATA_ACTION_" + ComConfig["ComSignals"][coreId][signal]["ComRxDataTimeoutAction"]["Value"]
        
    else:
        ComConfig["ComSignals"][coreId][signal]["ComRxDataTimeoutAction"]["Value"] = "COM_TIMEOUT_DATA_ACTION_NONE"

    str = ''
    numbers = []
    total = 0
    # signal ComSignalLength convert ComBitSize when COM_UINT8_N COM_UINT8_DYN ComSignalType
    if ComConfig["ComSignals"][coreId][signal]["ComSignalType"]["Value"] == "COM_UINT8_N" or ComConfig["ComSignals"][coreId][signal][
            "ComSignalType"]["Value"] == "COM_UINT8_DYN":
        if ComConfig["ComSignals"][coreId][signal]["ComSignalType"]["Value"] == "COM_UINT8_DYN":
            ComGeneral["COM_IPDU_DYNC_SIGNAL_ENABLE"] = "STD_ON"
        ComConfig["ComSignals"][coreId][signal]["ComBitSize"][
            "Value"] = 8 * ComConfig["ComSignals"][coreId][signal]["ComSignalLength"]["Value"]
        str = ComConfig["ComSignals"][coreId][signal]["ComSignalInitValue"]["Value"]
        if ' ' in str:
            numbers = [int(num) for num in str.split()]
        else:
            ComSignalLength = ComConfig["ComSignals"][coreId][signal]["ComSignalLength"]["Value"]
            if "COM_BIG_ENDIAN" == ComConfig["ComSignals"][coreId][signal]["ComSignalEndianness"]["Value"]:
                for index in range(ComSignalLength):
                    numbers.append(
                        int(ComConfig["ComSignals"][coreId][signal]["ComSignalInitValue"]["Value"]) >> 8 *
                        (ComSignalLength - index - 1) & 0xFF) 
            else:
                for index in range(ComSignalLength):
                    numbers.append(
                        int(ComConfig["ComSignals"][coreId][signal]["ComSignalInitValue"]["Value"]) >> 8 * index & 0xFF)
        ComConfig["ComSignals"][coreId][signal]["ComSignalInitValueList"] = numbers
        for num in numbers:
            total += num
        if total == 0:
            ComConfig["ComSignals"][coreId][signal]["ComSignalInitValue"]["Value"] = '0'

    handle_SignalBitPosition(ComConfig, signal, coreId)
    handle_SignalInitValueType(ComGeneral, ComConfig, signal, coreId)
    handle_SignalBusAccType(ComConfig, signal, coreId)
    

    if ComConfig["ComSignals"][coreId][signal]["ComUpdateBitPosition"]["Value"] == None:
        ComConfig["ComSignals"][coreId][signal]["ComUpdateBitPosition"]["Value"] = 0

    # if signal == "PATResponseCode_oFBCM_Code_Info1_oFBCM_BDCAN1_Tx":
    #     print (ComConfig["ComSignals"][signal]["ComNotification"]["Value"])

    # init value - notification
    if ComConfig["ComSignals"][coreId][signal]["ComNotification"]["Value"] == "" or ComConfig["ComSignals"][coreId][signal][
            "ComNotification"]["Value"] == " " or ComConfig["ComSignals"][coreId][signal]["ComNotification"]["Value"] == None:
        ComConfig["ComSignals"][coreId][signal]["ComNotification"]["Value"] = "COM_NO_FUNCTION_CALLOUT"

    if ComConfig["ComSignals"][coreId][signal]["ComInvalidNotification"]["Value"] == "" or ComConfig["ComSignals"][coreId][signal][
            "ComInvalidNotification"]["Value"] == " " or ComConfig["ComSignals"][coreId][signal]["ComInvalidNotification"][
                "Value"] == None:
        ComConfig["ComSignals"][coreId][signal]["ComInvalidNotification"]["Value"] = "COM_NO_FUNCTION_CALLOUT"

    if ComConfig["ComSignals"][coreId][signal]["ComErrorNotification"]["Value"] == "" or ComConfig["ComSignals"][coreId][signal][
            "ComErrorNotification"]["Value"] == " " or ComConfig["ComSignals"][coreId][signal]["ComErrorNotification"][
                "Value"] == None:
        ComConfig["ComSignals"][coreId][signal]["ComErrorNotification"]["Value"] = "COM_NO_FUNCTION_CALLOUT"
    else:
        ComGeneral["COM_ERROR_NOTIFICATION_ENABLE"] = "STD_ON"

    if ComConfig["ComSignals"][coreId][signal]["ComTimeoutNotification"]["Value"] == "" or ComConfig["ComSignals"][coreId][signal][
            "ComTimeoutNotification"]["Value"] == " " or ComConfig["ComSignals"][coreId][signal]["ComTimeoutNotification"][
                "Value"] == None:
        ComConfig["ComSignals"][coreId][signal]["ComTimeoutNotification"]["Value"] = "COM_NO_FUNCTION_CALLOUT"

    ComConfig["ComSignals"][coreId][signal]['isSignalTOut'] = "FALSE"
    ComConfig["ComSignals"][coreId][signal]['isSignalNotification'] = "FALSE"
    # handle timeout value
    if signal[-3:] == "_Rx":
        #ComTimeout
        IPduMainFunctionRef = ComConfig['ComIPdus'][coreId][IPdu]['IPduMainFunctionRef']['ValueRef']
        IPduMainFunctionRef = IPduMainFunctionRef[IPduMainFunctionRef.rfind("/") + 1:]
        ComIPduRxTimeBase = ComConfig['ComMainFunction_rx'][coreId][IPduMainFunctionRef]['MainRxTimeBase']['Value'] * 1000

        ComTimeout = ComConfig["ComSignals"][coreId][signal]['ComTimeout']['Value'] * 1000
        ComConfig["ComSignals"][coreId][signal]['ComTimeout']['Value'] = int(ComTimeout / ComIPduRxTimeBase)

        #ComFirstTimeout
        ComFirstTimeout = ComConfig['ComSignals'][coreId][signal]['ComFirstTimeout']['Value'] * 1000
        ComConfig['ComSignals'][coreId][signal]['ComFirstTimeout']['Value'] = int(ComFirstTimeout / ComIPduRxTimeBase)

        if ComConfig["ComSignals"][coreId][signal]['ComTimeout']['Value'] > 0 or ComConfig['ComSignals'][coreId][signal]['ComFirstTimeout']['Value'] > 0:
            ComConfig["ComSignals"][coreId][signal]['isSignalTOut'] = "TRUE"
            ComGeneral["COM_RXTOU_ENABLE"] = "STD_ON"
            ComConfig["ComSignals"][coreId][signal]["signal_id_tout"] = ComGeneral["RxToutsignal_size"]
            ComGeneral["RxToutsignal_size"] += 1
            ComConfig["ComIPdus"][coreId][IPdu]["ISRXTIMEOUT"] = "TRUE"
            if ComConfig["ComSignals"][coreId][signal]['ComTimeout']['Value'] > 0:
                if ComConfig["ComIPdus"][coreId][IPdu]["TimeoutValue"] == 0:
                    ComConfig["ComIPdus"][coreId][IPdu]["TimeoutValue"] = ComConfig["ComSignals"][coreId][signal]['ComTimeout']['Value']
                if ComConfig["ComIPdus"][coreId][IPdu]["TimeoutValue"] != ComConfig["ComSignals"][coreId][signal]['ComTimeout']['Value']:
                    ComGeneral["COM_RXIPDU_SAME_TIMEOUT"] = "STD_OFF"
            
            if ComConfig['ComSignals'][coreId][signal]['ComFirstTimeout']['Value'] > 0:
                if ComConfig["ComIPdus"][coreId][IPdu]["FirstTimeoutValue"] == 0:
                    ComConfig["ComIPdus"][coreId][IPdu]["FirstTimeoutValue"] = ComConfig["ComSignals"][coreId][signal]['ComFirstTimeout']['Value']
                if ComConfig["ComIPdus"][coreId][IPdu]["FirstTimeoutValue"] != ComConfig["ComSignals"][coreId][signal]['ComFirstTimeout']['Value']:
                    ComGeneral["COM_RXIPDU_SAME_TIMEOUT"] = "STD_OFF"
            
            #确认当前的mainfunction id下有配置rx timeout
            IPduMainFunctionRef =  ComConfig["ComIPdus"][coreId][IPdu]['IPduMainFunctionRef']['ValueRef']
            IPduMainFunctionRef = IPduMainFunctionRef[IPduMainFunctionRef.rfind("/") + 1:]
            ComConfig["ComMainFunction_rx"][coreId][IPduMainFunctionRef]["RxTimeoutInfo"] = "TRUE"

    else:
        #ComTimeout
        IPduMainFunctionRef = ComConfig['ComIPdus'][coreId][IPdu]['IPduMainFunctionRef']['ValueRef']
        IPduMainFunctionRef = IPduMainFunctionRef[IPduMainFunctionRef.rfind("/") + 1:]
        ComIPduTxTimeBase = ComConfig['ComMainFunction_tx'][coreId][IPduMainFunctionRef]['MainTxTimeBase']['Value'] * 1000

        ComTimeout = ComConfig["ComSignals"][coreId][signal]['ComTimeout']['Value'] * 1000
        ComConfig["ComSignals"][coreId][signal]['ComTimeout']['Value'] = int(ComTimeout / ComIPduTxTimeBase)

        #ComFirstTimeout
        ComFirstTimeout = ComConfig['ComSignals'][coreId][signal]['ComFirstTimeout']['Value'] * 1000
        ComConfig['ComSignals'][coreId][signal]['ComFirstTimeout']['Value'] = int(ComFirstTimeout / ComIPduTxTimeBase)

        if ComConfig["ComSignals"][coreId][signal]["ComNotification"]["Value"] != "COM_NO_FUNCTION_CALLOUT":
            ComConfig["ComSignals"][coreId][signal]['isSignalNotification'] = "TRUE"
            ComGeneral["Com_TxNotification"] = "STD_ON"
            ComConfig["ComSignals"][coreId][signal]["signal_id_notification"] = ComGeneral["TxNotification_size"]
            ComGeneral["TxNotification_size"] += 1
        
        if ComConfig["ComSignals"][coreId][signal]['ComTimeout']['Value'] > 0 or ComConfig['ComSignals'][coreId][signal]['ComFirstTimeout']['Value'] > 0:
            ComConfig["ComSignals"][coreId][signal]['isSignalTOut'] = "TRUE"
            ComGeneral["COM_TXTOU_ENABLE"] = "STD_ON"
            ComConfig["ComSignals"][coreId][signal]["signal_id_tout"] = ComGeneral["TxToutsignal_size"]
            ComGeneral["TxToutsignal_size"] += 1
            ComConfig["ComIPdus"][coreId][IPdu]["ISTXTIMEOUT"] = "TRUE"
            if ComConfig["ComSignals"][coreId][signal]['ComTimeout']['Value'] > 0:
                if ComConfig["ComIPdus"][coreId][IPdu]["TimeoutValue"] == 0:
                    ComConfig["ComIPdus"][coreId][IPdu]["TimeoutValue"] = ComConfig["ComSignals"][coreId][signal]['ComTimeout']['Value']
                if ComConfig["ComIPdus"][coreId][IPdu]["TimeoutValue"] != ComConfig["ComSignals"][coreId][signal]['ComTimeout']['Value']:
                    ComGeneral["COM_TXIPDU_SAME_TIMEOUT"] = "STD_OFF"
            
            if ComConfig['ComSignals'][coreId][signal]['ComFirstTimeout']['Value'] > 0:
                if ComConfig["ComIPdus"][coreId][IPdu]["FirstTimeoutValue"] == 0:
                    ComConfig["ComIPdus"][coreId][IPdu]["FirstTimeoutValue"] = ComConfig["ComSignals"][coreId][signal]['ComFirstTimeout']['Value']
                if ComConfig["ComIPdus"][coreId][IPdu]["FirstTimeoutValue"] != ComConfig["ComSignals"][coreId][signal]['ComFirstTimeout']['Value']:
                    ComGeneral["COM_TXIPDU_SAME_TIMEOUT"] = "STD_OFF"
    
    if ComConfig["ComSignals"][coreId][signal]["ComSignalDataInvalidValue"]["Value"] != '' and ComConfig["ComSignals"][coreId][signal]["ComSignalDataInvalidValue"]["Value"] != None:
        ComGeneral["COM_SIGNAL_DATAINVALIDVALUE_ENABLE"] = "STD_ON"
    if ComConfig["ComSignals"][coreId][signal]["ComInvalidNotification"]["Value"] != "COM_NO_FUNCTION_CALLOUT":
        ComGeneral["COM_SIGNAL_DATAINVALIDVALUE_ENABLE"] = "STD_ON"
   
    ComGeneral["signal_id"][coreId]["signal_id"] += 1
    signal_id += 1
    return signal_id


def handle_SignalInitValueType(ComGeneral: dict, ComConfig: dict, signal: str, coreId):
    ComConfig["ComSignals"][coreId][signal]["ComSignalInitValueIndex"] = 0
    if (ComConfig["ComSignals"][coreId][signal]["ComSignalInitValue"]["Value"] != '0'):
        CheckType = ComConfig["ComSignals"][coreId][signal]["ComSignalType"]["Value"]
        if (CheckType == "COM_UINT8" or CheckType == "COM_BOOLEAN" or CheckType == "COM_SINT8"):
            ComGeneral["Com_ConstValueUInt8Array"].append(
                int(ComConfig["ComSignals"][coreId][signal]["ComSignalInitValue"]["Value"]))
            ComConfig["ComSignals"][coreId][signal]["ComSignalInitValueIndex"] = ComGeneral["Com_ConstValueUInt8Num"]
            ComGeneral["Com_ConstValueUInt8Num"] += 1

        if (CheckType == "COM_UINT16" or CheckType == "COM_SINT16"):
            ComGeneral["Com_ConstValueUInt16Array"].append(
                int(ComConfig["ComSignals"][coreId][signal]["ComSignalInitValue"]["Value"]))
            ComConfig["ComSignals"][coreId][signal]["ComSignalInitValueIndex"] = ComGeneral["Com_ConstValueUInt16Num"]
            ComGeneral["Com_ConstValueUInt16Num"] += 1

        if (CheckType == "COM_UINT32" or CheckType == "COM_SINT32" or CheckType == "COM_FLOAT32"):
            ComGeneral["Com_ConstValueUInt32Array"].append(
                int(ComConfig["ComSignals"][coreId][signal]["ComSignalInitValue"]["Value"]))
            ComConfig["ComSignals"][coreId][signal]["ComSignalInitValueIndex"] = ComGeneral["Com_ConstValueUInt32Num"]
            ComGeneral["Com_ConstValueUInt32Num"] += 1

        if (CheckType == "COM_UINT64" or CheckType == "COM_SINT64" or CheckType == "COM_FLOAT64"):
            ComGeneral["Com_ConstValueUInt64Array"].append(
                int(ComConfig["ComSignals"][coreId][signal]["ComSignalInitValue"]["Value"]))
            ComConfig["ComSignals"][coreId][signal]["ComSignalInitValueIndex"] = ComGeneral["Com_ConstValueUInt64Num"]
            ComGeneral["Com_ConstValueUInt64Num"] += 1

        if (CheckType == "COM_UINT8_N" or CheckType == "COM_UINT8_DYN"):
            ComConfig["ComSignals"][coreId][signal]["ComSignalInitValueIndex"] = ComGeneral["Com_ConstValueUInt8_NNum"]
            ComGeneral["Com_ConstValueUInt8_NNum"] += 1
    else:
        ComGeneral["ComSignalInitValueZero"] = "STD_ON"

#将大端序的Combitpositon的MSB转成LSB
def handle_SignalBitPosition(ComConfig: dict, signal: str, coreId):
    ComConfig["ComSignals"][coreId][signal]["ComEndBitPosition"] = 0
    if ComConfig["ComSignals"][coreId][signal]["ComSignalEndianness"]["Value"] == "COM_BIG_ENDIAN":
        # 获取ComEndBitPosition MSB
        ComBitPositionMsb = ComConfig["ComSignals"][coreId][signal]["ComBitPosition"]["Value"]
        ComConfig["ComSignals"][coreId][signal]["ComEndBitPosition"] = ComBitPositionMsb
        
        # 获取ComEndBitPosition LSB
        ComBitSize = ComConfig["ComSignals"][coreId][signal]["ComBitSize"]["Value"]
        ComLsbInx = ((ComBitPositionMsb ^ 0x7) + ComBitSize - 1) ^ 7    
        ComConfig["ComSignals"][coreId][signal]["ComBitPosition"]["Value"] = ComLsbInx

def handle_SignalBusAccType(ComConfig: dict, signal: str, coreId):
    ComSignalType = ComConfig["ComSignals"][coreId][signal]["ComSignalType"]["Value"]
    ComBitPosition = ComConfig["ComSignals"][coreId][signal]["ComBitPosition"]["Value"]
    ComBitSize = ComConfig["ComSignals"][coreId][signal]["ComBitSize"]["Value"]
    if ComBitPosition % 8 == 0 and ComBitSize == 8:
        ComConfig["ComSignals"][coreId][signal]["BusAccType"] = "COM_BYTE_SIG_DECODE"
    elif ComBitPosition % 8 == 0 and ComBitSize % 8 == 0:
        if "COM_BIG_ENDIAN" == ComConfig["ComSignals"][coreId][signal]["ComSignalEndianness"]["Value"]:
            ComConfig["ComSignals"][coreId][signal]["BusAccType"] = "COM_NBYTE_BIG_ENDIAN_SIG_DECODE"
        else:
            ComConfig["ComSignals"][coreId][signal]["BusAccType"] = "COM_NBYTE_SIG_DECODE"
    elif ComBitPosition % 8 + ComBitSize <= 8:
        ComConfig["ComSignals"][coreId][signal]["BusAccType"] = "COM_NBIT_SIG_DECODE"
    else:
        if "COM_BIG_ENDIAN" == ComConfig["ComSignals"][coreId][signal]["ComSignalEndianness"]["Value"]:
            ComConfig["ComSignals"][coreId][signal]["BusAccType"] = "COM_NBITNBYTE_BIG_ENDIAN_SIG_DECODE"
        else:
            ComConfig["ComSignals"][coreId][signal]["BusAccType"] = "COM_NBITNBYTE_SIG_DECODE"


def handle_SignalBytePosition(ComConfig: dict, signal: str, StartBytePos, coreId):
    ComBitPosition = ComConfig["ComSignals"][coreId][signal]["ComBitPosition"]["Value"]
    ComByteOffSet = int(ComBitPosition / 8)
    ComConfig["ComSignals"][coreId][signal]["ComStartIPduBuffIdx"] = StartBytePos + ComByteOffSet

def pre_handle_Signal_Group(ComGeneral: dict, ComConfig: dict, IPdu: str, signal_id, coreId):
    dict_tempValueTRUE = {"Value": "TRUE"}
    dict_tempValueFALSE = {"Value": "FALSE"}
    temp_str = ""

    ComConfig['ComIPdus'][coreId][IPdu]["ComIPduHasSignalGroup"] = "FALSE"

    if ComConfig['ComIPdus'][coreId][IPdu]['ComIPduSignalGroupRef'][
            "ValueRef"] is not None and ComConfig['ComIPdus'][coreId][IPdu]['ComIPduSignalGroupRef']['ValueRef'][
                0] != '' and ComConfig['ComIPdus'][coreId][IPdu]['ComIPduSignalGroupRef']['ValueRef'][0] != '<empty>':
        if "PduRef" in ComConfig["ComIPdus"][coreId][IPdu].keys():
            ComGeneral["ipdu_size"][coreId]["ipdu_size"] += ComConfig["ComIPdus"][coreId][IPdu]["PduRef"]["PduLength"]["Value"]
            if ComConfig["ComIPdus"][coreId][IPdu]["ComIPduDirection"]["Value"] == "SEND":
                ComGeneral["Txipdu_size"][coreId]["Txipdu_size"] += ComConfig["ComIPdus"][coreId][IPdu]["PduRef"]["PduLength"]["Value"]
            else:
                ComGeneral["Rxipdu_size"][coreId]["Rxipdu_size"] += ComConfig["ComIPdus"][coreId][IPdu]["PduRef"]["PduLength"]["Value"]
                # 当前报文是Immedite 并且该报文有信号组  需要保留IPDU Buffer
                if ComConfig["ComIPdus"][coreId][IPdu]["ComIPduSignalProcessing"]["Value"] == "COM_IMMEDIATE":
                    ComGeneral["Rxipdu_size"][coreId]["Rxipdu_size"] += ComConfig["ComIPdus"][coreId][IPdu]["PduRef"]["PduLength"]["Value"]
        for ComSignalGroupVal in list(ComConfig["ComIPdus"][coreId][IPdu]["ComIPduSignalGroupRef"]["ValueRef"]):
            ComGeneral["COM_SIGNAL_GROUP_ENABLE"] = "STD_ON"
            ComConfig['ComIPdus'][coreId][IPdu]["ComIPduHasSignalGroup"] = "TRUE"
            ComSignalGroupVal = ComSignalGroupVal[ComSignalGroupVal.rfind("/") + 1:]
            dict_Temp = {}
            list_temp = []
            ComConfig["ComSignalGroups"][ComSignalGroupVal]["IsGroupSignal"] = dict_tempValueFALSE
            ComConfig["ComSignalGroups"][ComSignalGroupVal]["IsSignalGroup"] = dict_tempValueTRUE
            ComConfig['ComSignals'][coreId][ComSignalGroupVal] = copy.deepcopy(ComConfig["ComSignalGroups"][ComSignalGroupVal])
            ComConfig["ComIPdus"][coreId][IPdu]["ComIPduSignalRef"]["ValueRef"].append(ComSignalGroupVal)
            # ComConfig['ComSignals'][ComSignalGroupVal]["signal_id"] = signal_id
            # signal_id += 1
            for ComGroupSignalVal in list(ComConfig["ComSignalGroups"][ComSignalGroupVal].keys()):
                if "ComBitPosition" in ComConfig["ComSignalGroups"][ComSignalGroupVal][ComGroupSignalVal]:
                    temp_str = ComGroupSignalVal
                    ComConfig["ComSignalGroups"][ComSignalGroupVal][ComGroupSignalVal][
                        "IsGroupSignal"] = dict_tempValueTRUE
                    ComConfig["ComSignalGroups"][ComSignalGroupVal][ComGroupSignalVal][
                        "IsSignalGroup"] = dict_tempValueFALSE
                    #执行组信号填充赋值
                    handle_groupsignal_info(ComConfig["ComSignalGroups"][ComSignalGroupVal][ComGroupSignalVal])

                    ComConfig['ComSignals'][coreId][ComGroupSignalVal] = copy.deepcopy(
                        ComConfig["ComSignalGroups"][ComSignalGroupVal][ComGroupSignalVal])
                    ComConfig["ComIPdus"][coreId][IPdu]["ComIPduSignalRef"]["ValueRef"].append(ComGroupSignalVal)
                    # ComConfig['ComSignals'][ComGroupSignalVal]["signal_id"] = signal_id
                    # signal_id += 1
                    dict_Temp[ComGroupSignalVal] = ComConfig['ComSignals'][coreId][ComGroupSignalVal]
                    list_temp.append(ComGroupSignalVal)
            #ComConfig['ComSignals'][ComSignalGroupVal]["ComSignalType"] = ComConfig['ComSignals'][ComGroupSignalVal]["ComSignalType"]
            ComConfig['ComSignals'][coreId][ComSignalGroupVal]["ComGroupSignalRef"] = dict_Temp
            ComConfig['ComSignals'][coreId][ComSignalGroupVal]["GroupSignal"] = {"Value": list_temp}
            handle_signalgroup_info(ComConfig['ComSignals'][coreId][ComSignalGroupVal], ComConfig['ComSignals'][coreId][temp_str],
                                    ComConfig['ComSignals'][coreId])
            handle_signal_group(ComConfig, IPdu, ComSignalGroupVal, coreId)
    return signal_id


def handle_signal_group(ComConfig: dict, IPdu: str, ComSignalGroupVal: str, coreId):
    # signal group attribute: signal group mask
    ShadowBufferstr = get_signal_group_mask(ComConfig["ComSignals"][coreId][ComSignalGroupVal]["ComGroupSignalRef"],
                                            ComConfig["ComSignals"][coreId], ComConfig["ComIPdus"][coreId][IPdu]["ComIPduSize"])
    ComConfig['ComSignals'][coreId][ComSignalGroupVal]["ShadowBuffer_Mask"] = {"Value": ShadowBufferstr}


def handle_groupsignal_info(gsinfo: dict):
    gsinfo["ComUpdateBitPosition"] = {"Value": 0}
    gsinfo["ComNotification"] = {"Value": ""}
    gsinfo["ComInvalidNotification"] = {"Value": ""}
    gsinfo["ComErrorNotification"] = {"Value": ""}
    gsinfo["ComTimeoutNotification"] = {"Value": ""}
    gsinfo["ComTimeoutNotification"] = {"Value": ""}
    gsinfo["ComFirstTimeout"] = {"Value": 0}
    gsinfo["ComTimeout"] = {"Value": 0}
    gsinfo["ComRxDataTimeoutAction"] = {"Value": "NONE"}
    gsinfo["ComDataInvalidAction"] = {"Value": "NOTIFY"}


def handle_signalgroup_info(sginfo: dict, gsinfo: dict, comsignal: dict):
    sginfo["ComSignalType"] = copy.deepcopy(gsinfo["ComSignalType"])
    sginfo["ComSignalEndianness"] = copy.deepcopy(gsinfo["ComSignalEndianness"])
    sginfo["ComSignalDataInvalidValue"] = copy.deepcopy(gsinfo["ComSignalDataInvalidValue"])
    sginfo["ComSignalInitValue"] = copy.deepcopy(gsinfo["ComSignalInitValue"])
    tempBitPosition = gsinfo["ComBitPosition"]["Value"]
    tempComBitSize = 0
    tempComBitSize_Val = gsinfo["ComBitSize"]["Value"]
    # dict_tempValueTRUE = {"Value":"TRUE"}
    # sginfo["ComUpdateBitPosition"] = {"Value":0}
    for group_signal in list(sginfo["ComGroupSignalRef"].keys()):
        if sginfo[group_signal]["ComBitPosition"]["Value"] < tempBitPosition:
            tempBitPosition = sginfo[group_signal]["ComBitPosition"]["Value"]
        if sginfo[group_signal]["ComBitSize"]["Value"] > tempComBitSize_Val:
            sginfo["ComSignalType"] = copy.deepcopy(sginfo[group_signal]["ComSignalType"])
        tempComBitSize += sginfo[group_signal]["ComBitSize"]["Value"]
    sginfo["ComBitPosition"] = {"Value": tempBitPosition}
    sginfo["ComBitSize"] = {"Value": tempComBitSize}
    # sginfo["ComUpdateBitPosition"] = {"Value":0}


def handle_ipdu_groups(ComConfig: dict, IpduGroup_Id) -> int:
    IpduGroups = ComConfig["ComIPduGroups"]

    # Create IPDU GROUP Object with Ipdu Group Id
    for key in IpduGroups:
        ComConfig["ComIPduGroups"][key] = {}
        ComConfig["ComIPduGroups"][key]["Id"] = IpduGroup_Id
        IpduGroup_Id += 1
    return IpduGroup_Id


def handle_global_info(ComGeneral: dict, ComConfig: dict, TxIpdus, message_id, signal_id, IpduGroup_Id,
                       message_id_tx, message_id_rx, signal_id_tx, signal_id_rx, app_coreid_dict):
    Parameters = {}
    # ComIPdus counter
    Parameters["message_Count"] = message_id
    # signals counter
    Parameters["signal_Count"] = signal_id
    # all the Tx message
    ComConfig["TxIpdus"] = TxIpdus
    # global macro
    Parameters["COM_MAX_N_IPDUS"] = message_id
    Parameters["COM_MAX_N_TXIPDUS"] = message_id_tx
    Parameters["COM_MAX_N_RXIPDUS"] = message_id_rx

    Parameters["COM_MAX_N_TXSIGNALS"] = signal_id_tx
    Parameters["COM_MAX_N_RXSIGNALS"] = signal_id_rx

    Parameters["COM_MAX_N_SIGNALS"] = signal_id
    Parameters["COM_MAX_N_GROUP_SIGNALS"] = 400
    Parameters["COM_N_SUPPORTED_IPDU_GROUPS"] = IpduGroup_Id
    Parameters["COM_MAX_N_SUPPORTED_IPDU_COUNTERS"] = 0
    Parameters["COM_OSEKNM_SUPPORT"] = "STD_OFF"
    Parameters["COM_SIG_GATEWAY_ENABLE"] = ComConfig["COM_SIG_GATEWAY_ENABLE"]
    Parameters["ComConfigurationId"] = 1
    
    if ComGeneral["Com_Core_Num"] > 1:
        ComGeneral["Com_Support_MutiCore"] = "STD_ON"
    else:
        ComGeneral["Com_Support_MutiCore"] = "STD_OFF"
    
    # 处理单核部署情况下的全局数据
    if ComGeneral["Com_Support_MutiCore"] == "STD_OFF":
        for coreId, txipdu_info in ComGeneral["Txipdu_size"].items():
            if txipdu_info != {}:
                ComGeneral["SingleCore"]["ipdu_size"] = ComGeneral["ipdu_size"][coreId]["ipdu_size"]
                ComGeneral["SingleCore"]["Txipdu_size"] = ComGeneral["Txipdu_size"][coreId]["Txipdu_size"]
                ComGeneral["SingleCore"]["Rxipdu_size"] = ComGeneral["Rxipdu_size"][coreId]["Rxipdu_size"]

                ComGeneral["SingleCore"]["uint8_bool_signal_num"] = ComGeneral["uint8_bool_signal_num"][coreId]["uint8_bool_signal_num"]
                ComGeneral["SingleCore"]["uint16_signal_num"] = ComGeneral["uint16_signal_num"][coreId]["uint16_signal_num"]
                ComGeneral["SingleCore"]["uint32_signal_num"] = ComGeneral["uint32_signal_num"][coreId]["uint32_signal_num"]
                ComGeneral["SingleCore"]["uint64_signal_num"] = ComGeneral["uint64_signal_num"][coreId]["uint64_signal_num"]
                ComGeneral["SingleCore"]["uint8_N_signal_num"] = ComGeneral["uint8_N_signal_num"][coreId]["uint8_N_signal_num"]


                ComGeneral["uint8_bool_signal_num_totle"] = ComGeneral["uint8_bool_signal_num"][coreId]["uint8_bool_signal_num"]
                ComGeneral["uint16_signal_num_totle"] = ComGeneral["uint16_signal_num"][coreId]["uint16_signal_num"]
                ComGeneral["uint32_signal_num_totle"] = ComGeneral["uint32_signal_num"][coreId]["uint32_signal_num"]
                ComGeneral["uint64_signal_num_totle"] = ComGeneral["uint64_signal_num"][coreId]["uint64_signal_num"]
                ComGeneral["uint8_N_signal_num_totle"] = ComGeneral["uint8_N_signal_num"][coreId]["uint8_N_signal_num"]

    # buffer size need to be optimized
    # Parameters["COM_MAX_BUFFER_SIZE"] = ComGeneral["ipdu_size"]

    # Parameters["COM_MAX_TXBUFFER_SIZE"] = ComGeneral["Txipdu_size"]
    # Parameters["COM_MAX_RXBUFFER_SIZE"] = ComGeneral["Rxipdu_size"]

    # 提取每个Core的Tx Rx Buffer
    for coreId, txipdu_info in ComGeneral["Txipdu_size"].items():
        if txipdu_info != {}: 
            txipdu_info["START_memmapstr"] = "#define COM_START_SEC_" + app_coreid_dict[coreId] + "_PRIVATE_BSS"
            txipdu_info["STOP_memmapstr"] = "#define COM_STOP_SEC_" + app_coreid_dict[coreId] + "_PRIVATE_BSS"
            txipdu_info["com_self_tx_buffer"] = "VAR(uint8, COM_" + app_coreid_dict[coreId] + "_PRIVATE_BSS) " + "Com_Self_TxBuffer_" + str(coreId) + "[" + str(txipdu_info["Txipdu_size"]) + "]"
            txipdu_info["com_self_tx_ipdu"] = "VAR(com_self_tx_ipdu_type, COM_" + app_coreid_dict[coreId] + "_PRIVATE_BSS) " + "Com_Self_TxIPdu_" + str(coreId) + "[" + str(ComGeneral["message_id_tx"][coreId]["message_id_tx"]) + "]"
            txipdu_info["com_self_tx_signal"] = "VAR(com_self_tx_signal_type, COM_" + app_coreid_dict[coreId] + "_PRIVATE_BSS) " + "Com_Self_TxSignal_" + str(coreId) + "[" + str(ComGeneral["signal_id_tx"][coreId]["signal_id_tx"]) + "]"
            
            # 生成Com_CoreMapTxIpduStructStruct使用
            ComGeneral["CoreMapInfo"][coreId].update({"endTxIpduIdx": ComGeneral["ipdu_endIdx_tx"][coreId]["ipdu_endIdx_tx"]})
            ComGeneral["CoreMapInfo"][coreId].update({"endTxSignalIdx": ComGeneral["signal_endIdx_tx"][coreId]["signal_endIdx_tx"]})

            ComGeneral["CoreMapInfo"][coreId].update({"startTxIpduIdx": ComGeneral["ipdu_startIdx_tx"][coreId]["ipdu_startIdx_tx"]})
            ComGeneral["CoreMapInfo"][coreId].update({"startRxIpduIdx": ComGeneral["ipdu_startIdx_rx"][coreId]["ipdu_startIdx_rx"]})

            ComGeneral["CoreMapInfo"][coreId].update({"startTxSignalIdx": ComGeneral["signal_startIdx_tx"][coreId]["signal_startIdx_tx"]})
            ComGeneral["CoreMapInfo"][coreId].update({"startRxSignalIdx": ComGeneral["signal_startIdx_rx"][coreId]["signal_startIdx_rx"]})

            if txipdu_info["Txipdu_size"] > 0 :
                ComGeneral["CoreMapInfo"][coreId].update({"txBufferRef": "Com_Self_TxBuffer_" + str(coreId)})
            else:
                ComGeneral["CoreMapInfo"][coreId].update({"txBufferRef": "NULL"})

            
            ComGeneral["CoreMapInfo"][coreId].update({"selfTxIpduRef": "Com_Self_TxIPdu_" + str(coreId)})
            ComGeneral["CoreMapInfo"][coreId].update({"selfTxSignalRef": "Com_Self_TxSignal_" + str(coreId)})

            # 生成信号Buffer
            ComGeneral["CoreMapInfo"][coreId]["START_memmapstr"] = txipdu_info["START_memmapstr"]
            ComGeneral["CoreMapInfo"][coreId]["STOP_memmapstr"] = txipdu_info["STOP_memmapstr"]

            ComGeneral["CoreMapInfo"][coreId]["uint8_bool_signal_num"] = ComGeneral["uint8_bool_signal_num"][coreId]["uint8_bool_signal_num"]
            ComGeneral["uint8_bool_signal_num_totle"] += ComGeneral["uint8_bool_signal_num"][coreId]["uint8_bool_signal_num"]
            if ComGeneral["uint8_bool_signal_num"][coreId]["uint8_bool_signal_num"] > 0:
                ComGeneral["CoreMapInfo"][coreId].update({"com_RxSigBufferUInt8Ref": "com_rx_sig_buf_u8_" + str(coreId)})
                ComGeneral["CoreMapInfo"][coreId]["uint8_bool_buff"] = "VAR(uint8, COM_" + app_coreid_dict[coreId] + "_PRIVATE_BSS) " + "com_rx_sig_buf_u8_" + str(coreId) + "[" + str(ComGeneral["uint8_bool_signal_num"][coreId]["uint8_bool_signal_num"]) + "]"
            else:
                ComGeneral["CoreMapInfo"][coreId].update({"com_RxSigBufferUInt8Ref": "NULL"})
            ComGeneral["CoreMapInfo"][coreId]["uint8_bool_signal_num"] = ComGeneral["uint8_bool_signal_num"][coreId]["uint8_bool_signal_num"]
            
            ComGeneral["uint16_signal_num_totle"] += ComGeneral["uint16_signal_num"][coreId]["uint16_signal_num"]
            if ComGeneral["uint16_signal_num"][coreId]["uint16_signal_num"] > 0:
                ComGeneral["CoreMapInfo"][coreId].update({"com_RxSigBufferUInt16Ref": "com_rx_sig_buf_u16_" + str(coreId)})
                ComGeneral["CoreMapInfo"][coreId]["uint16_buff"] = "VAR(uint16, COM_" + app_coreid_dict[coreId] + "_PRIVATE_BSS) " + "com_rx_sig_buf_u16_" + str(coreId) + "[" + str(ComGeneral["uint16_signal_num"][coreId]["uint16_signal_num"]) + "]"
            else:
                ComGeneral["CoreMapInfo"][coreId].update({"com_RxSigBufferUInt16Ref": "NULL"})
            ComGeneral["CoreMapInfo"][coreId]["uint16_signal_num"] = ComGeneral["uint16_signal_num"][coreId]["uint16_signal_num"]

            ComGeneral["uint32_signal_num_totle"] += ComGeneral["uint32_signal_num"][coreId]["uint32_signal_num"]
            if ComGeneral["uint32_signal_num"][coreId]["uint32_signal_num"] > 0:
                ComGeneral["CoreMapInfo"][coreId].update({"com_RxSigBufferUInt32Ref": "com_rx_sig_buf_u32_" + str(coreId)})
                ComGeneral["CoreMapInfo"][coreId]["uint32_buff"] = "VAR(uint32, COM_" + app_coreid_dict[coreId] + "_PRIVATE_BSS) " + "com_rx_sig_buf_u32_" + str(coreId) + "[" + str(ComGeneral["uint32_signal_num"][coreId]["uint32_signal_num"]) + "]"
            else:
                ComGeneral["CoreMapInfo"][coreId].update({"com_RxSigBufferUInt32Ref": "NULL"})
            ComGeneral["CoreMapInfo"][coreId]["uint32_signal_num"] = ComGeneral["uint32_signal_num"][coreId]["uint32_signal_num"]

            ComGeneral["uint64_signal_num_totle"] += ComGeneral["uint64_signal_num"][coreId]["uint64_signal_num"]
            if ComGeneral["uint64_signal_num"][coreId]["uint64_signal_num"] > 0:
                ComGeneral["CoreMapInfo"][coreId].update({"com_RxSigBufferUInt64Ref": "com_rx_sig_buf_u64_" + str(coreId)})
                ComGeneral["CoreMapInfo"][coreId]["uint64_buff"] = "VAR(uint64, COM_" + app_coreid_dict[coreId] + "_PRIVATE_BSS) " + "com_rx_sig_buf_u64_" + str(coreId) + "[" + str(ComGeneral["uint64_signal_num"][coreId]["uint64_signal_num"]) + "]"
            else:
                ComGeneral["CoreMapInfo"][coreId].update({"com_RxSigBufferUInt64Ref": "NULL"})
            ComGeneral["CoreMapInfo"][coreId]["uint64_signal_num"] = ComGeneral["uint64_signal_num"][coreId]["uint64_signal_num"]

            ComGeneral["uint8_N_signal_num_totle"] += ComGeneral["uint8_N_signal_num"][coreId]["uint8_N_signal_num"]
            if ComGeneral["uint8_N_signal_num"][coreId]["uint8_N_signal_num"] > 0:
                ComGeneral["CoreMapInfo"][coreId].update({"Com_RxSigBufferArrayBasedRef": "com_rx_sig_buf_u8_n_" + str(coreId)})
                ComGeneral["CoreMapInfo"][coreId]["uint8_N_buff"] = "VAR(uint8, COM_" + app_coreid_dict[coreId] + "_PRIVATE_BSS) " + "com_rx_sig_buf_u8_n_" + str(coreId) + "[" + str(ComGeneral["uint8_N_signal_num"][coreId]["uint8_N_signal_num"]) + "]"
            else:
                ComGeneral["CoreMapInfo"][coreId].update({"Com_RxSigBufferArrayBasedRef": "NULL"})
            ComGeneral["CoreMapInfo"][coreId]["uint8_N_signal_num"] = ComGeneral["uint8_N_signal_num"][coreId]["uint8_N_signal_num"]

    for coreId, rxipdu_info in ComGeneral["Rxipdu_size"].items():
        if rxipdu_info != {}:
            rxipdu_info["START_memmapstr"] = "#define COM_START_SEC_" + app_coreid_dict[coreId] + "_PRIVATE_BSS"
            rxipdu_info["STOP_memmapstr"] = "#define COM_STOP_SEC_" + app_coreid_dict[coreId] + "_PRIVATE_BSS"
            rxipdu_info["com_self_rx_buffer"] = "VAR(uint8, COM_" + app_coreid_dict[coreId] + "_PRIVATE_BSS) " + "Com_Self_RxBuffer_" + str(coreId) + "[" + str(rxipdu_info["Rxipdu_size"]) + "]"
            rxipdu_info["com_self_rx_ipdu"] = "VAR(com_self_rx_ipdu_type, COM_" + app_coreid_dict[coreId] + "_PRIVATE_BSS) " + "Com_Self_RxIPdu_" + str(coreId) + "[" + str(ComGeneral["message_id_rx"][coreId]["message_id_rx"]) + "]"
            rxipdu_info["com_self_rx_signal"] = "VAR(com_self_rx_signal_type, COM_" + app_coreid_dict[coreId] + "_PRIVATE_BSS) " + "Com_Self_RxSignal_" + str(coreId) + "[" + str(ComGeneral["signal_id_rx"][coreId]["signal_id_rx"]) + "]"

            # 生成Com_CoreMapTxIpduStructStruct使用
            ComGeneral["CoreMapInfo"][coreId].update({"endRxIpduIdx": ComGeneral["ipdu_endIdx_rx"][coreId]["ipdu_endIdx_rx"]})
            ComGeneral["CoreMapInfo"][coreId].update({"endRxSignalIdx": ComGeneral["signal_endIdx_rx"][coreId]["signal_endIdx_rx"]})

            ComGeneral["CoreMapInfo"][coreId].update({"selfRxIpduRef": "Com_Self_RxIPdu_" + str(coreId)})
            ComGeneral["CoreMapInfo"][coreId].update({"selfRxSignalRef": "Com_Self_RxSignal_" + str(coreId)})
            if rxipdu_info["Rxipdu_size"] > 0 :
                ComGeneral["CoreMapInfo"][coreId].update({"rxBufferRef": "Com_Self_RxBuffer_" + str(coreId)})
            else:
                ComGeneral["CoreMapInfo"][coreId].update({"rxBufferRef": "NULL"})


    for coreId, mainfunction_num_info in ComGeneral["mainfunction_num"].items():
        if mainfunction_num_info != {}:
            ComGeneral["mainfunction_rx_num_totle"] += ComGeneral["mainfunction_num"][coreId]["mainfunction_rx_num"]
            ComGeneral["mainfunction_tx_num_totle"] += ComGeneral["mainfunction_num"][coreId]["mainfunction_tx_num"]
    
    # 保证数组中有一个成员
    if ComGeneral["RxToutsignal_size"] == 0:
        ComGeneral["RxToutsignal_size"] = 1 
    

    if ComGeneral["TxToutsignal_size"] == 0:
        ComGeneral["TxToutsignal_size"] = 1 


    if ComGeneral["RxToutmessage_size"] == 0:
        ComGeneral["RxToutmessage_size"] = 1  
    
    if ComGeneral["TxToutmessage_size"] == 0:
        ComGeneral["TxToutmessage_size"] = 1 

    ComGeneral["Parameters"] = Parameters
    if ComGeneral['ComConfigurationUseDet']['Value'] == False:
        ComGeneral['ComConfigurationUseDet']['Value'] = 'STD_OFF'
    else:
        ComGeneral['ComConfigurationUseDet']['Value'] = 'STD_ON'
    
    # 状态初始值
    ComGeneral['ComSupportedTxSignalProx'] = 'STD_OFF'
    ComGeneral["COM_TXSIGNAL_UINT8_FIFO"] = "STD_OFF"
    ComGeneral["COM_TXSIGNAL_UINT16_FIFO"] = "STD_OFF"
    ComGeneral["COM_TXSIGNAL_UINT32_FIFO"] = "STD_OFF"
    ComGeneral["COM_TXSIGNAL_UINT64_FIFO"] = "STD_OFF"
    ComGeneral["COM_TXSIGNAL_UINT8_N_FIFO"] = "STD_OFF"

    ComGeneral['Uint8FifoDepth'] = 0
    ComGeneral['Uint16FifoDepth'] = 0
    ComGeneral['Uint32FifoDepth'] = 0
    ComGeneral['Uint8_NFifoDepth'] = 0
    ComGeneral['Uint8_NFifoSize'] = 0
    ComGeneral["MaxDequeueCntOneCycle"] = 0

    # 发送信号代理功能
    if ComConfig["ComTxSignalFifos"] != {}:
        if ComConfig['ComTxSignalFifos']["ComTxSignalFifo"]['ComSupportedTxSignalProx']['Value'] == False:
            ComGeneral['ComSupportedTxSignalProx'] = 'STD_OFF'
        else:
            ComGeneral['ComSupportedTxSignalProx'] = 'STD_ON'
        
        if ComConfig['ComTxSignalFifos']["ComTxSignalFifo"]['Uint8FifoDepth']['Value'] > 0:
            ComGeneral['Uint8FifoDepth'] = ComConfig['ComTxSignalFifos']["ComTxSignalFifo"]['Uint8FifoDepth']['Value'] + 1
            ComGeneral["COM_TXSIGNAL_UINT8_FIFO"] = "STD_ON"
        
        if ComConfig['ComTxSignalFifos']["ComTxSignalFifo"]['Uint16FifoDepth']['Value'] > 0:
            ComGeneral['Uint16FifoDepth'] = ComConfig['ComTxSignalFifos']["ComTxSignalFifo"]['Uint16FifoDepth']['Value'] + 1
            ComGeneral["COM_TXSIGNAL_UINT16_FIFO"] = "STD_ON"
        
        if ComConfig['ComTxSignalFifos']["ComTxSignalFifo"]['Uint32FifoDepth']['Value'] > 0:
            ComGeneral['Uint32FifoDepth'] = ComConfig['ComTxSignalFifos']["ComTxSignalFifo"]['Uint32FifoDepth']['Value'] + 1
            ComGeneral["COM_TXSIGNAL_UINT32_FIFO"] = "STD_ON"

        if ComConfig['ComTxSignalFifos']["ComTxSignalFifo"]['Uint8_NFifoDepth']['Value'] > 0:
            ComGeneral['Uint8_NFifoDepth'] = ComConfig['ComTxSignalFifos']["ComTxSignalFifo"]['Uint8_NFifoDepth']['Value'] +1 
            ComGeneral["COM_TXSIGNAL_UINT8_N_FIFO"] = "STD_ON"

        if ComConfig['ComTxSignalFifos']["ComTxSignalFifo"]['Uint8_NFifoSize']['Value'] > 0:
            ComGeneral['Uint8_NFifoSize'] = ComConfig['ComTxSignalFifos']["ComTxSignalFifo"]['Uint8_NFifoSize']['Value'] + 2

        fifoDepthTotle = ComGeneral['Uint8FifoDepth'] + ComGeneral['Uint16FifoDepth'] + ComGeneral['Uint32FifoDepth'] + ComGeneral['Uint8_NFifoDepth']

        if ComConfig['ComTxSignalFifos']["ComTxSignalFifo"]["MaxDequeueCntOneCycle"]['Value'] == 0 or ComConfig['ComTxSignalFifos']["ComTxSignalFifo"]["MaxDequeueCntOneCycle"]['Value'] > fifoDepthTotle:
            ComGeneral['MaxDequeueCntOneCycle'] = fifoDepthTotle
        else:
            ComGeneral['MaxDequeueCntOneCycle'] = ComConfig['ComTxSignalFifos']["ComTxSignalFifo"]["MaxDequeueCntOneCycle"]['Value']

def handle_signal_gateway(ComGeneral: dict, ComConfig: dict):
    comgwmapping_count = 0
    ComConfig["COM_SIG_GATEWAY_ENABLE"] = "STD_OFF"
    for route_item in ComConfig["ComGwMappings"].keys():
        ComConfig["COM_SIG_GATEWAY_ENABLE"] = "STD_ON"
        path_count = 0
        comgwmapping_count += 1
        route_path = ComConfig["ComGwMappings"][route_item]
        route_source = route_path["ComGwSource"]
        if "ComGwDestination" not in route_path.keys():
            route_path["ComGwDestination"] = {}
        
        index = 0
        for route_dest_key in ComConfig["ComGwMappings"][route_item].keys():
            if "ComGwDestination" in route_dest_key:
                if  route_dest_key !=  "ComGwDestination":
                    index += 1
                    index_str = str(index)
                    route_path["ComGwDestination"]["ComGwSignal" + index_str] = ComConfig["ComGwMappings"][route_item][route_dest_key]["ComGwSignal"]

        route_dests = route_path["ComGwDestination"]
        # handle source signal
        if "ComGwSignal" in route_source.keys():
            # source signal ref type

            # remove prefix
            temp_name = route_source["ComGwSignal"]["ComGwSignalRef"]["ValueRef"]

            # 判断当前是Signal还是Group Signal
            index = temp_name.find("ComConfig")
            if index != -1:
                # 从 "ComConfig" 结束的地方开始提取，并按 "/" 分割
                elements = temp_name[index:].split('/')[1:]
            else:
                elements = []

            route_source["ComGwSignal"]["ComGwSignalRef"]["ValueRef"] = temp_name[temp_name.rfind("/") + 1:]
            # IPDU Reference
            source_ipdu_name = ComConfig["ComSignals_totle"][route_source["ComGwSignal"]["ComGwSignalRef"]
                                                       ["ValueRef"]]["message_name"]
            ComConfig["ComGwMappings"][route_item]["ComGwSource"]["IPduRef"] = source_ipdu_name
            
            # set routing request
            if ComConfig["ComIPdus_totle"][source_ipdu_name]["ComIPduGwRoutingReq"] != "TRUE":
                ComConfig["ComIPdus_totle"][source_ipdu_name]["ComIPduGwRoutingReq"] = "TRUE"
                ComConfig["ComRxIPdus_totle"][source_ipdu_name]["ComIPduGwRoutingReq"] = "TRUE"

            # get signal handler
            signal_name = route_source["ComGwSignal"]["ComGwSignalRef"]["ValueRef"]
            
            # set type in cfg code
            # 组信号 Com/ComConfig/xxx_siggroup/GroupSignal
            # 信号  Com/ComConfig/Signal
            if len(elements) == 2:
                ComConfig["ComGwMappings"][route_item]["ComGwSource"]["SourceType"] = "COM_GROUP_SIGNAL_REFERENCE"
                ComConfig["ComGwMappings"][route_item]["SourceSignalHandle"] = "ComConf_ComGroupSignal_" + signal_name
            else:
                ComConfig["ComGwMappings"][route_item]["ComGwSource"]["SourceType"] = "COM_SIGNAL_REFERENCE"
                ComConfig["ComGwMappings"][route_item]["SourceSignalHandle"] = "ComConf_ComSignal_" + signal_name
            
            # open source signal routing request
            ComConfig["ComSignals_totle"][signal_name]["ComSigGwRoutingReq"] = "TRUE"
            ComConfig["ComRxSignals_totle"][signal_name]["ComSigGwRoutingReq"] = "TRUE"
        else:
            # source description signal type
            # have not supported
            logger.error(f"Description Gateway Unsupported")

        # handle dest signals
        for dest_item in route_dests.keys():
            path_count += 1
            if "ComGwSignalRef" in route_dests[dest_item].keys():
                # dest signal ref type
                # remove prefix
                temp_name = route_dests[dest_item]["ComGwSignalRef"]["ValueRef"]

                # 判断当前是Signal还是Group Signal
                # 组信号 Com/ComConfig/xxx_siggroup/GroupSignal
                # 信号  Com/ComConfig/Signal
                index = temp_name.find("ComConfig")
                if index != -1:
                    # 从 "ComConfig" 结束的地方开始提取，并按 "/" 分割
                    elements = temp_name[index:].split('/')[1:]
                else:
                    elements = []

                route_dests[dest_item]["ComGwSignalRef"]["ValueRef"] = temp_name[temp_name.rfind("/") + 1:]

                dest_ipdu_name = ComConfig["ComSignals_totle"][route_dests[dest_item]["ComGwSignalRef"]
                                                         ["ValueRef"]]["message_name"]
                ComConfig["ComGwMappings"][route_item]["ComGwDestination"][dest_item]["IPduRef"] = dest_ipdu_name
                # set type in cfg code
                
                # get dest signal handler
                signal_name = route_dests[dest_item]["ComGwSignalRef"]["ValueRef"]
                # ComConfig["ComGwMappings"][route_item]["DestSignalHandle"] = "ComConf_ComSignal_" + signal_name
                route_dests[dest_item]["DestSignalHandle"] = "ComConf_ComSignal_" + signal_name

                # set type in cfg code
                if len(elements) == 2:
                    ComConfig["ComGwMappings"][route_item]["ComGwDestination"][dest_item]["DestType"] = "COM_GROUP_SIGNAL_REFERENCE"
                    route_dests[dest_item]["DestSignalHandle"] = "ComConf_ComGroupSignal_" + signal_name
                else:
                    ComConfig["ComGwMappings"][route_item]["ComGwDestination"][dest_item]["DestType"] = "COM_SIGNAL_REFERENCE"
                    route_dests[dest_item]["DestSignalHandle"] = "ComConf_ComSignal_" + signal_name
            else:
                # dest description signal type
                logger.error(f"Description Gateway Unsupported")
        ComConfig["ComGwMappings"][route_item]["NoOfDesitnation"] = path_count
    ComGeneral["NoOfGwMappings"] = comgwmapping_count


def get_signal_group_mask(group_signal_names, com_signal, message_size) -> str:
    """
    cal shadow buffer mask for signal group
    """
    mask_str = ""
    byte_order = "big"
    source_list = [0 for i in range(message_size * 8)]
    # print("source_str:", source_str)
    for group_signal in list(group_signal_names.keys()):
        if group_signal_names[group_signal]["ComSignalEndianness"]["Value"] == 'LITTLE_ENDIAN':
            byte_order = "little"
        elif group_signal_names[group_signal]["ComSignalEndianness"]["Value"] == "OPAQUE":
            byte_order = "opaque"
    # mark group signal
    if byte_order == "big":
        for group_signal in list(group_signal_names.keys()):
            st = com_signal[group_signal]["ComBitPosition"]["Value"] ^ 7
            if com_signal[group_signal]["ComSignalType"]["Value"] == "UINT8_N" or com_signal[group_signal]["ComSignalType"]["Value"] == "UINT8_DYN":
                com_signal[group_signal]["ComBitSize"]["Value"] = 8 * com_signal[group_signal]["ComSignalLength"]["Value"]
            len = com_signal[group_signal]["ComBitSize"]["Value"]
            for i_to_len in range(len):
                source_list[st + i_to_len] |= 1

    elif byte_order == "little":
        for group_signal in list(group_signal_names.keys()):
            st = com_signal[group_signal]["ComBitPosition"]["Value"] ^ 7
            if com_signal[group_signal]["ComSignalType"]["Value"] == "UINT8_N" or com_signal[group_signal]["ComSignalType"]["Value"] == "UINT8_DYN":
                com_signal[group_signal]["ComBitSize"]["Value"] = 8 * com_signal[group_signal]["ComSignalLength"]["Value"]
            len = com_signal[group_signal]["ComBitSize"]["Value"]
            head_bit = (st // 8) * 8
            minuend = 0
            for i_to_len in range(len):
                source_list[st - minuend] |= 1
                if (st - minuend) == head_bit:
                    st += 15
                    head_bit += 8
                    minuend = 0
                else:
                    minuend += 1
    else:
        logger.info("Not supported byte order")
        return ""

    tmp_list = ["".join([str(i) for i in source_list[x * 8:(x + 1) * 8]]) for x in range(message_size)]
    # print(tmp_list)
    for x_str in tmp_list:
        if mask_str != "":
            mask_str += ", "
        mask_str += (hex(int(x_str, 2)))
    # print(mask_str)
    return mask_str


def get_unsigned_type(signal_dict, signal_value):
    """
    Get an unsigned type.

    Arg:
        signal_dict: dictionary that stores signal;
        signal_value: The single signal obtained.
    """

    if signal_value.length <= 8:
        signal_dict["signal_type"] = "COM_UINT8"
    elif signal_value.length > 8 and signal_value.length <= 16:
        signal_dict["signal_type"] = "COM_UINT16"
    elif signal_value.length > 16 and signal_value.length <= 32:
        signal_dict["signal_type"] = "COM_UINT32"
    else:
        signal_dict["signal_type"] = "COM_UINT64"


def get_signed_type(signal_dict, signal_value):
    """
    Get an signed type.

    Arg:
        signal_dict: dictionary that stores signal;
        signal_value: The single signal obtained.
    """

    if signal_value.length <= 8:
        signal_dict["signal_type"] = "COM_SINT8"
    elif signal_value.length > 8 and signal_value.length <= 16:
        signal_dict["signal_type"] = "COM_SINT16"
    elif signal_value.length > 16 and signal_value.length <= 32:
        signal_dict["signal_type"] = "COM_SINT32"
    else:
        signal_dict["signal_type"] = "COM_SINT64"


def get_signal_type(signal_dict, signal_value):
    """
    Get all signals from the dbc file.

    Arg:
        signal_dict: dictionary that stores signal;
        signal_value: The single signal obtained.
    """

    if True == signal_value.is_float:
        if signal_value.length <= 32:
            signal_dict["signal_type"] = "COM_FLOAT32"
        else:
            signal_dict["signal_type"] = "COM_FLOAT64"
    elif False == signal_value.is_signed and False == signal_value.is_float:
        get_unsigned_type(signal_dict, signal_value)
    elif True == signal_value.is_signed and False == signal_value.is_float:
        get_signed_type(signal_dict, signal_value)


def get_gen_sig_send_type(gen_sig_send_type_value, signal_dict):
    """
    Get singal send type.

    Arg:
        gen_sig_send_type_value: value from dbc file;
        signal_dict: dictionary that stores signal.
    """

    if 0 == gen_sig_send_type_value or 7 == gen_sig_send_type_value:
        signal_dict["gen_sig_send_type"] = "COM_PENDING"
    elif 1 == gen_sig_send_type_value or 5 == gen_sig_send_type_value:
        signal_dict["gen_sig_send_type"] = "COM_TRIGGERED_WITHOUT_REPETITION"
    elif 2 == gen_sig_send_type_value or 6 == gen_sig_send_type_value:
        signal_dict["gen_sig_send_type"] = "COM_TRIGGERED"
    elif 3 == gen_sig_send_type_value:
        signal_dict["gen_sig_send_type"] = "COM_TRIGGERED_ON_CHANGE_WITHOUT_REPETITION"
    elif 4 == gen_sig_send_type_value:
        signal_dict["gen_sig_send_type"] = "COM_TRIGGERED_ON_CHANGE"


def get_dbc_signals(signal_dict, signal_value):
    """
    Get all signals from the dbc file.

    Arg:
        signal_dict: dictionary that stores signal;
        signal_value: The single signal obtained.
    """

    signal_dict["signal_name"] = signal_value.signal_name
    signal_dict["bit_position"] = signal_value.start
    signal_dict["signal_size"] = signal_value.length
    gen_sig_send_type_value = signal_value.signal_attributes["GenSigSendType"].value
    get_gen_sig_send_type(gen_sig_send_type_value, signal_dict)
    if "big_endian" == signal_value.byte_order:
        signal_dict["byte_order"] = "COM_BIG_ENDIAN"
    elif "litter_endian" == signal_value.byte_order:
        signal_dict["byte_order"] = "COM_LITTLE_ENDIAN"
    else:
        signal_dict["byte_order"] = "COM_OPAQUE"
    signal_dict["init_value"] = 0
    get_signal_type(signal_dict, signal_value)


def get_gen_msg_send_type(gen_msg_send_type_value, msg_dict):
    """
    Get message send type.

    Arg:
        gen_msg_send_type_value: value from dbc file;
        msg_dict: dictionary that stores message.
    """

    if 0 == gen_msg_send_type_value or 2 == gen_msg_send_type_value:
        msg_dict["gen_msg_send_type"] = "COM_PERIODIC"
    elif 1 == gen_msg_send_type_value or 3 == gen_msg_send_type_value or 6 == gen_msg_send_type_value:
        msg_dict["gen_msg_send_type"] = "COM_DIRECT"
    elif 4 == gen_msg_send_type_value or 5 == gen_msg_send_type_value:
        msg_dict["gen_msg_send_type"] = "COM_MIXED"
    elif 7 == gen_msg_send_type_value or 8 == gen_msg_send_type_value:
        msg_dict["gen_msg_send_type"] = "COM_NONE"


def get_config_json_template_input_parameter_dict(Config_dict: dict) -> dict:
    """
    Get input parameter for template com_cfg.json.

    Arg:
        Config_dict : All config dictionary

    Return:
        Returns a dictionary of template input parameter
    """
    com_config_json_dict = {}
    com_config_json_dict["config"] = {}
    ipdu_group_refs = []
    lin_count = 0
    for dbc in Config_dict["dbc"]:
        config_json_dict = []
        for msg_key, msg_value in dbc["details"].items():
            msg_dict = {}
            msg_dict["comm_diag_type"] = "dbc"
            msg_dict["message_name"] = msg_value.message_name
            msg_dict["bus_name"] = msg_value.bus_name
            if msg_value.bus_name not in ipdu_group_refs and "GenMsgILSupport" not in msg_value.message_attributes:
                ipdu_group_refs.append(msg_value.bus_name)
            msg_dict["message_size"] = msg_value.length
            msg_dict["has_signal_groups"] = msg_value.has_signal_groups
            gen_msg_send_type_value = msg_value.message_attributes['GenMsgSendType'].value
            if 'GenMsgCycleTime' in msg_value.message_attributes:
                msg_dict["gen_msg_cycle_time"] = msg_value.message_attributes['GenMsgCycleTime'].value // 10
            else:
                msg_dict["gen_msg_cycle_time"] = 0

            if 'GenMsgCycleTimeFast' in msg_value.message_attributes:
                msg_dict["gen_msg_cycle_time_fast"] = msg_value.message_attributes['GenMsgCycleTimeFast'].value // 10
            else:
                msg_dict["gen_msg_cycle_time_fast"] = 0
            get_gen_msg_send_type(gen_msg_send_type_value, msg_dict)
            signal_groups = []
            for signal_groups_key, signal_groups_value in msg_value.signal_groups.items():
                signal_group_dict = {}
                signal_group_dict["group_name"] = signal_groups_value.signal_group_name
                signal_group_dict["repetitions"] = signal_groups_value.repetitions
                signal_group_dict["group_signals_name"] = signal_groups_value.group_signals_name
                signal_groups.append(signal_group_dict)
            msg_dict["signal_groups"] = signal_groups
            msg_dict["senders"] = msg_value.senders[0]
            signals = []
            for signal_key, signal_value in msg_value.signals.items():
                signal_dict = {}
                if "FBCM" == msg_dict["senders"]:
                    signal_dict["direction"] = "Tx"
                else:
                    signal_dict["direction"] = "Rx"
                get_dbc_signals(signal_dict, signal_value)
                signals.append(signal_dict)
            msg_dict["signals"] = signals
            config_json_dict.append(msg_dict)
        if "GenMsgILSupport" in msg_value.message_attributes and 0 == msg_value.message_attributes[
                'GenMsgILSupport'].value:
            continue
        com_config_json_dict["config"][dbc["dbc_name"]] = config_json_dict
    for ldf in Config_dict["ldf"]:
        config_json_dict = []
        for msg_key, msg_value in ldf["details"].items():
            msg_dict = {}
            msg_dict["bus_name"] = msg_value.bus_name
            if msg_value.bus_name not in ipdu_group_refs:
                ipdu_group_refs.append(msg_value.bus_name)
            msg_dict["comm_diag_type"] = "ldf"
            msg_dict["message_name"] = msg_value.message_name
            msg_dict["senders"] = msg_value.node_name
            msg_dict["has_signal_groups"] = 0
            msg_dict["signal_groups"] = []
            msg_dict["gen_msg_send_type"] = "COM_NONE"
            msg_dict["message_size"] = msg_value.length
            msg_dict["gen_msg_cycle_time"] = 0
            msg_dict["gen_msg_cycle_time_fast"] = 0
            signals = []
            for signal_key, signal_value in msg_value.signals.items():
                signal_dict = {}
                signal_dict["signal_name"] = signal_value.signal_name
                signal_dict["bit_position"] = signal_value.start
                signal_dict["signal_size"] = signal_value.length
                signal_dict["init_value"] = signal_value.initial
                if "FBCM" == msg_dict["senders"]:
                    signal_dict["direction"] = "Tx"
                else:
                    signal_dict["direction"] = "Rx"
                signal_dict["signal_type"] = "COM_UINT8"
                signal_dict["byte_order"] = "COM_LITTLE_ENDIAN"
                signal_dict["gen_sig_send_type"] = "COM_PENDING"
                signals.append(signal_dict)
            msg_dict["signals"] = signals
            config_json_dict.append(msg_dict)
        com_config_json_dict["config"][ldf["ldf_name"]] = config_json_dict
    com_config_json_dict["ipdu_group_refs"] = ipdu_group_refs

    return com_config_json_dict

# 通过definition 获取对应的dict
def com_tools_get_dict_by_definition_ref(input_dict: dict, definition_ref: str) -> dict:
    for k, v in input_dict.items():
        if isinstance(v, dict):
            if "DefinitionRef" in v.keys():
                if v["DefinitionRef"].split("/")[-1] == definition_ref:    
                    return v    
    return {}