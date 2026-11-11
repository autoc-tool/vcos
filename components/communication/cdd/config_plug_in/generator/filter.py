#
# Copyright (C) 2015-2022 Lixiang Group Holding Limited
#
"""
配置代码生成模块.
"""


def get_template_input_parameter_dict(config_dict: dict) -> dict:
    """
    Get input parameter for template generated code.
    Arg:
      config_dict : All config dictionary
    Return:
      Returns a dictionary of template input parameter
    """
    return config_dict


def get_template_dynamic_code_dict(config_dict: dict) -> dict:
    pduList = {}

    for key, value in config_dict.items():
        if 'ArPackages' not in value:
            continue
        if 'ActiveEcuC' not in  value['ArPackages']:
            continue
        activeecuc = value['ArPackages']['ActiveEcuC']
        for moduleName, module in activeecuc.items():
            if 'DefinitionRef' not in module:
                continue
            definitionRef = module['DefinitionRef']
            if definitionRef.split("/")[-1] == 'Cdd': # 确认是CDD模块
                cdd_section = moduleName
                cdd_pdu_list = get_com_stack_info(value, cdd_section)
                pduList[key] = cdd_pdu_list
                pduList[key]['cdd_prefix'] = cdd_section # 方法一：直接在pduList[key]加一个键值对，和cdd_pdu_list字典内容并列

    config_dict_canif = config_dict.get('canif', None)
    if config_dict_canif is not None:
        pduList = get_function_name(config_dict_canif, pduList)
    return pduList


def get_com_stack_info(config_dict: dict, cdd_section: str) -> dict:
    comStackInPackage = config_dict['ArPackages']['ActiveEcuC'][cdd_section]['CddComStackContribution']
    comStacks = {}
    # comStacks = {"cdd_prefix": cdd_section}  # 方法二：初始时包含cdd_section字段
    for comStackName, comStackValue in comStackInPackage.items():
        if comStackName == 'DefinitionRef':
            continue
        stackName = comStackName
        # comStackName：CddPduRUpperLayerContribution->CddComStackContribution的下一级
        # comStackValue：CddPduRUpperLayerTxPdu等信息，包含If、Tp、PDU id等信息
        stackType = get_stack_type(comStackValue)
        stackPdu = get_pdu_name_and_pdu_id(comStackValue)
        comStacks[stackName] = {"stackType": stackType, "stackPdu": stackPdu}
    return comStacks

# 获取API的类型(If or Tp)：ComIfUpper、PduRTpUpper、PduRIfUpper、SoAdTpUpper、SoAdIfUpper
def get_stack_type(comStackValue: dict) -> dict:
    stackType = ""
    if comStackValue['DefinitionRef'].split("/")[-1] == 'CddComIfUpperLayerContribution':
        stackType = "ComIfUpper"
    elif comStackValue['DefinitionRef'].split("/")[-1] == 'CddPduRUpperLayerContribution':
        for key, value in comStackValue.items():
            if key == 'DefinitionRef':
                continue
            if value['DefinitionRef'].split("/")[-1] == 'CddPduRUpperLayerTxPdu':
                if value['CddPduRApiType']['Value'] == "TP":
                    stackType = "PduRTpUpper"
                elif value['CddPduRApiType']['Value'] == "IF":
                    stackType = "PduRIfUpper"
                else:
                    stackType = "Error"
            elif value['DefinitionRef'].split("/")[-1] == 'CddPduRUpperLayerRxPdu':
                if value['CddPduRApiType']['Value'] == "TP":
                    stackType = "PduRTpUpper"
                elif value['CddPduRApiType']['Value'] == "IF":
                    stackType = "PduRIfUpper"
                else:
                    stackType = "Error"
            else:
                stackType = "Error"


    elif comStackValue['DefinitionRef'].split("/")[-1] == 'CddPduRLowerLayerContribution':
        for key, value in comStackValue.items():
            if key == 'DefinitionRef':
                continue
            if value['DefinitionRef'].split("/")[-1] == 'CddPduRLowerLayerTxPdu':
                if value['CddPduRApiType']['Value'] == "TP":
                    stackType = "PduRTpLower"
                elif value['CddPduRApiType']['Value'] == "IF":
                    stackType = "PduRIfLower"
                else:
                    stackType = "Error"
            elif value['DefinitionRef'].split("/")[-1] == 'CddPduRLowerLayerRxPdu':
                if value['CddPduRApiType']['Value'] == "TP":
                    stackType = "PduRTpLower"
                elif value['CddPduRApiType']['Value'] == "IF":
                    stackType = "PduRIfLower"
                else:
                    stackType = "Error"
            else:
                stackType = "Error"


    elif comStackValue['DefinitionRef'].split("/")[-1] == 'CddSoAdUpperLayerContribution':
        for key, value in comStackValue.items():
            if key == 'DefinitionRef':
                continue
            if value['DefinitionRef'].split("/")[-1] == 'CddSoAdUpperLayerTxPdu':
                if value['CddPduRApiType']['Value'] == "TP":
                    stackType = "SoAdTpUpper"
                elif value['CddPduRApiType']['Value'] == "IF":
                    stackType = "SoAdIfUpper"
                else:
                    stackType = "Error"
            elif value['DefinitionRef'].split("/")[-1] == 'CddSoAdUpperLayerRxPdu':
                if value['CddPduRApiType']['Value'] == "TP":
                    stackType = "SoAdTpUpper"
                elif value['CddPduRApiType']['Value'] == "IF":
                    stackType = "SoAdIfUpper"
                else:
                    stackType = "Error"
            else:
                stackType = "Error"

    else:  # 其他类型暂不支持
        stackType = "Error"
    return stackType


def get_pdu_name_and_pdu_id(comStackValue: dict) -> dict:
    stackPdu = {}
    isExist = {'TxPduExisted': 'False', 'RxPduExisted': 'False'}
    stackTxPduList = []  # 定义一个空列表用于保存TxPdu信息
    stackRxPduList = []  # 定义一个空列表用于保存RxPdu信息
    for itemsInComStackName, itemsInComStackValue in comStackValue.items():
        if itemsInComStackName == 'DefinitionRef':
            continue
        for itemInComStackNameR, itemsInComStackValueR in itemsInComStackValue.items():
            if itemInComStackNameR == 'DefinitionRef':
                itemsInComStackNameRe = itemsInComStackValueR.split("/")[-1]
                break
        if "TxPdu" in itemsInComStackNameRe:
            isExist['TxPduExisted'] = 'True'
            for itemsInPduDefName, itemsInPduDefValue in itemsInComStackValue.items():
                if "HandleId" in itemsInPduDefName:
                    pduHandleId = itemsInPduDefValue['Value']
                if "PduRef" in itemsInPduDefName:
                    if itemsInPduDefValue['ValueRef'] is not None:
                        pduName = itemsInPduDefValue['ValueRef'].split('/')[-1]
                        if pduName == "<empty>":
                            pduName = ''
                    else:
                        pduName = ''
                    pduRef = itemsInPduDefValue
                    stackTxPdu = {'PduHandleId': pduHandleId, 'PduName': pduName, 'PduRef': pduRef['ValueRef']}
            stackTxPduList.append(stackTxPdu)  # 将每次循环得到的TxPdu信息追加到列表中
        stackPdu['TxPdu'] = stackTxPduList  # 将TxPdu列表赋值给stackPdu字典中的'TxPdu'键
        if "RxPdu" in itemsInComStackNameRe:
            isExist['RxPduExisted'] = 'True'
            stackRxPdu = {}
            for itemsInPduDefName, itemsInPduDefValue in itemsInComStackValue.items():
                if "HandleId" in itemsInPduDefName:
                    pduHandleId = itemsInPduDefValue['Value']
                    stackRxPdu['PduHandleId'] = pduHandleId
                if "PduRef" in itemsInPduDefName:
                    if itemsInPduDefValue['ValueRef'] is not None:
                        pduName = itemsInPduDefValue['ValueRef'].split('/')[-1]
                        if pduName == "<empty>":
                            pduName = ''
                    else:
                        pduName = ''
                    pduRef = itemsInPduDefValue
                    stackRxPdu['PduRef'] = pduRef['ValueRef']
            stackPdu['RxPdu'] = stackRxPdu
            stackRxPdu = {'PduHandleId': pduHandleId, 'PduName': pduName, 'PduRef': pduRef['ValueRef']}
            stackRxPduList.append(stackRxPdu)  # 将每次循环得到的TxPdu信息追加到列表中
        stackPdu['RxPdu'] = stackRxPduList  # 将TxPdu列表赋值给stackPdu字典中的'TxPdu'键
    stackPdu['TxPduExisted'] = isExist['TxPduExisted']
    stackPdu['RxPduExisted'] = isExist['RxPduExisted']
    return stackPdu


def get_function_name(config_dict_raw: dict, pduList: dict) -> dict:
    config_dict_canif = config_dict_raw["ArPackages"]["ActiveEcuC"]["CanIf"]
    pduListAfter = pduList
    CanIfPdus = config_dict_canif['CanIfInitCfg']

    for cdd_key, cdd_value in pduListAfter.items():
        if 'CddComIfUpperLayerContribution' in cdd_value:
            stackItem = cdd_value['CddComIfUpperLayerContribution']  # 获取特定的字典项
            stackPdu = stackItem['stackPdu']
            TxPduRef = ''
            RxPduRef = ''
            if "TxPdu" in stackPdu:
                # 此段处理逻辑：如果CDD和CanIf的Ref能对的上，则将canif那边的CanIfTxPduUserTxConfirmationName赋值给tx_pduvalue['FuncName']
                for tx_pduvalue in stackPdu['TxPdu']:
                    if stackItem['stackPdu']['TxPduExisted'] == 'True':
                            tx_pduvalue['FuncName'] = 'Cdd_TxConfirmation'
                    for key, value in tx_pduvalue.items():
                        if key == 'PduRef':
                            TxPduRef = value

                    for CanIfPduName, CanIfPduItem in CanIfPdus.items():
                        if CanIfPduName == 'DefinitionRef':
                            continue
                        if 'CanIfTxPduRef' in CanIfPduItem:
                            if CanIfPduItem['CanIfTxPduRef']['ValueRef'] == TxPduRef:
                                tx_pduvalue['FuncName'] = CanIfPduItem['CanIfTxPduUserTxConfirmationName']['Value']
            if "RxPdu" in stackPdu:
                for rx_pduvalue in stackPdu['RxPdu']:
                    if stackItem['stackPdu']['RxPduExisted'] == 'True':
                            rx_pduvalue['FuncName'] = 'Cdd_RxIndication'
                    for key, value in rx_pduvalue.items():
                        if key == 'PduRef':
                            RxPduRef = value

                    for CanIfPduName, CanIfPduItem in CanIfPdus.items():
                        if CanIfPduName == 'DefinitionRef':
                            continue
                        if 'CanIfRxPduRef' in CanIfPduItem:
                            if CanIfPduItem['CanIfRxPduRef']['ValueRef'] == RxPduRef:
                                rx_pduvalue['FuncName'] = CanIfPduItem['CanIfRxPduUserRxIndicationName']['Value']
    return pduListAfter
