#
# Copyright (C) 2015-2022 Lixiang Group Holding Limited
#
"""
os模板内函数定义模块.

定义jinja2模板中所使用的函数.

用法示例:
{{get_core_id_list()}}
"""

from jinja2 import pass_context
from func_module.generator.common.common import FunctionBase


class Function(FunctionBase):
    """
    Class containing all functions. Add new static functions here
    to be included in the jinja2 context
    """

    @staticmethod
    @pass_context
    def get_cdd_com_stack(context):
        PduRIfUpper = {'stackIsExist': 'False'}
        PduRTpUpper = {'stackIsExist': 'False'}
        PduRIfLower = {'stackIsExist': 'False', 'TxPduExisted': 'False', 'RxPduExisted': 'False'}
        PduRTpLower = {'stackIsExist': 'False', 'TxPduExisted': 'False', 'RxPduExisted': 'False'}
        ComIfUpper = {'stackIsExist': 'False'}
        SoAdIfUpper = {'stackIsExist': 'False'}
        SoAdTpUpper = {'stackIsExist': 'False'}
        for _, itemValue in context['cddInfo'].items():
            if not isinstance(itemValue, dict): # 如果itemValue不是dict类型, 则跳过处理
                continue
            if itemValue['stackType'] == 'PduRIfUpper':
                PduRIfUpper['stackIsExist'] = 'True'
                PduRIfUpper['TxPduExisted'] = itemValue['stackPdu']['TxPduExisted']
                PduRIfUpper['RxPduExisted'] = itemValue['stackPdu']['RxPduExisted']
            elif itemValue['stackType'] == 'PduRTpUpper':
                PduRTpUpper['stackIsExist'] = 'True'
                PduRTpUpper['TxPduExisted'] = itemValue['stackPdu']['TxPduExisted']
                PduRTpUpper['RxPduExisted'] = itemValue['stackPdu']['RxPduExisted']

            if itemValue['stackType'] == 'PduRIfLower':
                PduRIfLower['stackIsExist'] = 'True'
                PduRIfLower['TxPduExisted'] = itemValue['stackPdu']['TxPduExisted']
                PduRIfLower['RxPduExisted'] = itemValue['stackPdu']['RxPduExisted']
            elif itemValue['stackType'] == 'PduRTpLower':
                PduRTpLower['stackIsExist'] = 'True'
                PduRTpLower['TxPduExisted'] = itemValue['stackPdu']['TxPduExisted']
                PduRTpLower['RxPduExisted'] = itemValue['stackPdu']['RxPduExisted']

            elif itemValue['stackType'] == 'SoAdIfUpper':
                SoAdIfUpper['stackIsExist'] = 'True'
                SoAdIfUpper['TxPduExisted'] = itemValue['stackPdu']['TxPduExisted']
                SoAdIfUpper['RxPduExisted'] = itemValue['stackPdu']['RxPduExisted']

            elif itemValue['stackType'] == 'SoAdTpUpper':
                SoAdTpUpper['stackIsExist'] = 'True'
                SoAdTpUpper['TxPduExisted'] = itemValue['stackPdu']['TxPduExisted']
                SoAdTpUpper['RxPduExisted'] = itemValue['stackPdu']['RxPduExisted']

            elif itemValue['stackType'] == 'ComIfUpper':
                ComIfUpper['stackIsExist'] = 'True'
                ComIfUpper['TxPduExisted'] = itemValue['stackPdu']['TxPduExisted']
                ComIfUpper['RxPduExisted'] = itemValue['stackPdu']['RxPduExisted']
        isExist = {'PduRIfUpper': PduRIfUpper, 'PduRTpUpper': PduRTpUpper,
                   'PduRIfLower': PduRIfLower, 'PduRTpLower': PduRTpLower, 'ComIfUpper': ComIfUpper,
                   'SoAdIfUpper': SoAdIfUpper, 'SoAdTpUpper': SoAdTpUpper}
        return isExist

    @staticmethod
    @pass_context
    def get_cdd_pdu(context):
        PduRUpperLayer = {'stackIsExist': 'False'}
        PduRLowerLayer = {'stackIsExist': 'False'}
        ComIfUpperLayer = {'stackIsExist': 'False'}
        SoAdUpperLayer = {'stackIsExist': 'False'}
        for itemName, itemValue in context['cddInfo'].items():
            if not isinstance(itemValue, dict): # 如果itemValue不是dict类型, 则跳过处理
                continue
            if itemName == 'CddPduRUpperLayerContribution':
                PduRUpperLayer['stackIsExist'] = 'True'
                PduRUpperLayer['PduInfo'] = itemValue['stackPdu']
            if itemName == 'CddPduRLowerLayerContribution':
                PduRLowerLayer['stackIsExist'] = 'True'
                PduRLowerLayer['PduInfo'] = itemValue['stackPdu']
            if itemName == 'CddComIfUpperLayerContribution':
                ComIfUpperLayer['stackIsExist'] = 'True'
                ComIfUpperLayer['PduInfo'] = itemValue['stackPdu']
            if itemName == 'CddSoAdUpperLayerContribution':
                SoAdUpperLayer['stackIsExist'] = 'True'
                SoAdUpperLayer['PduInfo'] = itemValue['stackPdu']
        stackPdus = {'PduRUpperLayer':PduRUpperLayer, 'PduRLowerLayer':PduRLowerLayer,
                     'ComIfUpperLayer':ComIfUpperLayer, 'SoAdUpperLayer':SoAdUpperLayer}
        return stackPdus

    @staticmethod
    @pass_context
    def get_canif_pdu_name(context):
        PduRIfUpper = {'stackIsExist': 'False'}
        canif_pduname = {}
        Canif_Rx_PduName = ''
        Canif_Tx_PduName = ''
        stackTxFunNameList = [] # 定义一个空列表用于存放Txfunname信息
        stackRxFunNameList = [] # 定义一个空列表用于存放Rxfunname信息
        for itemName, itemValue in context['cddInfo'].items():
            if not isinstance(itemValue, dict): # 如果itemValue不是dict类型, 则跳过处理
                continue
            if itemValue['stackType'] == 'PduRIfUpper':
                PduRIfUpper['stackIsExist'] = 'True'
                PduRIfUpper['TxPduExisted'] = itemValue['stackPdu']['TxPduExisted']
                PduRIfUpper['RxPduExisted'] = itemValue['stackPdu']['RxPduExisted']
            if itemValue['stackType'] == 'ComIfUpper':
                if itemValue['stackPdu']['TxPduExisted'] == 'True':
                    for tx_fun_name in itemValue['stackPdu']['TxPdu']:
                        for key, value in tx_fun_name.items():
                            if key == 'FuncName':
                                Canif_Tx_PduName = value
                                stackTxFunNameList.append(Canif_Tx_PduName)
                stackTxFunNameList = list(set(stackTxFunNameList)) # 去除重名的name
                canif_pduname['Canif_Tx_Pduname'] = stackTxFunNameList

                if itemValue['stackPdu']['RxPduExisted'] == 'True':
                    for rx_fun_name in itemValue['stackPdu']['RxPdu']:
                        for key, value in rx_fun_name.items():
                            if key == 'FuncName':
                                Canif_Rx_PduName = value
                                stackRxFunNameList.append(Canif_Rx_PduName)
                stackRxFunNameList = list(set(stackRxFunNameList))
                canif_pduname['Canif_Rx_PduName'] = stackRxFunNameList
        canif_pduinfo = {'PduRIfUpper':PduRIfUpper, 'canif_pduname':canif_pduname}
        return canif_pduinfo

    @staticmethod
    @pass_context
    def get_cdd_module_info(context):
        for itemName, itemValue in context['cddInfo'].items():
            if itemName == 'cdd_prefix':
                cdd_prefix_name = itemValue
                cdd_module_info = {'cdd_prefix_name': cdd_prefix_name}
        return cdd_module_info

