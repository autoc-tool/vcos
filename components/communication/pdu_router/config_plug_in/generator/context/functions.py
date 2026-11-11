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
os模板内函数定义模块.

定义jinja2模板中所使用的函数.

用法示例:
{{get_core_id_list()}}
"""
import math

from jinja2 import pass_context

from func_module.generator.common.common import FunctionBase

class Function(FunctionBase):
    """
    Class containing all functions. Add new static functions here
    to be included in the jinja2 context
    """

    @staticmethod
    @pass_context
    def get_pdur_json_config(context):
        pdur_json_config = context["pdur_json"]
        return pdur_json_config

    @staticmethod
    @pass_context
    def get_pdur_general_config(context):
        pdur_general_config = context["pdur_json"]["PduRGeneral"]
        return pdur_general_config

    @staticmethod
    @pass_context
    def get_pdur_multicore(context):
        PduRMultiCore = context["pdur_json"]["PduRMultiCore"]
        return PduRMultiCore

    @staticmethod
    @pass_context
    def get_pdur_core_num(context):
        pdur_evcfifo_cfg = context["pdur_json"]["PduREvCFifos"]
        maxCoreId_fifo = 0
        for fifo in pdur_evcfifo_cfg:
            if maxCoreId_fifo < fifo['PduREventQueueCore']:
                maxCoreId_fifo = fifo['PduREventQueueCore']

        return int(maxCoreId_fifo) + 1

    @staticmethod
    @pass_context
    def get_pdur_if_core_num(context):
        pdur_evcfifo_cfg = context["pdur_json"]["PduREvCFifosIF"]
        maxCoreId_fifo = 0
        for fifo in pdur_evcfifo_cfg:
            if maxCoreId_fifo < fifo['coreid']:
                maxCoreId_fifo = fifo['coreid']

        return int(maxCoreId_fifo) + 1

    @staticmethod
    @pass_context
    def get_pdur_mainfunctionlist(context):
        return context["pdur_json"]["PduRMainFunctionList"]

    @staticmethod
    @pass_context
    def get_pdur_isr(context):
        return context["pdur_json"]["PduRIsr"]

    @staticmethod
    @pass_context
    def get_pdur_isr_table(context):
        pdurIsrList = context["pdur_json"]["PduRIsr"]
        output = []
        maxCoreId = 0
        for pdurIsr in pdurIsrList:
            if pdurIsr['coreid'] == '0xFFFF':
                continue
            if maxCoreId < int(pdurIsr['coreid']):
                maxCoreId = int(pdurIsr['coreid'])
        for coreid in range(maxCoreId + 1):
            for pdurIsr in pdurIsrList:
                if coreid == int(pdurIsr['coreid']):
                    pdurIsrTemp = pdurIsr
                    pdurIsrTemp['name'] = pdurIsrTemp['name']
                    output.append(pdurIsrTemp)
                    break
            else:
                temp = {'name':'', 'isrref':'', 'coreid':''}
                output.append(temp)
        listLen = maxCoreId + 1
        return output, listLen

    @staticmethod
    @pass_context
    def get_pdur_max_routing_path_cnt_config(context):
        routing_path_cnt = 0
        msg_name_list = []
        pdur_json_config = context["pdur_json"]
        for path_name, path_value in pdur_json_config["PduRRoutingPaths"].items():
            if path_value['PduRoutingPathName'] not in msg_name_list:
                msg_name_list.append(path_value['PduRoutingPathName'])
                routing_path_cnt = routing_path_cnt + 1
        return routing_path_cnt
    
    @staticmethod
    @pass_context
    def get_pdur_real_routing_path_cnt_config(context):
        routing_path_cnt = 0
        msg_name_list = []
        pdur_json_config = context["pdur_json"]
        for path_name, path_value in pdur_json_config["PduRRoutingPaths"].items():
            if path_value['PduRoutingPathName'] not in msg_name_list:
                msg_name_list.append(path_value['PduRoutingPathName'])
                routing_path_cnt = routing_path_cnt + 1
        return routing_path_cnt

    @staticmethod
    @pass_context
    def get_pdur_real_routing_path_bit_cnt_config(context):
        routing_path_cnt = 0
        routing_path_bit_cnt = 0
        msg_name_list = []
        pdur_json_config = context["pdur_json"]
        for path_name, path_value in pdur_json_config["PduRRoutingPaths"].items():
            if path_value['PduRoutingPathName'] not in msg_name_list:
                msg_name_list.append(path_value['PduRoutingPathName'])
                routing_path_cnt = routing_path_cnt + 1
        routing_path_bit_cnt = math.ceil(routing_path_cnt / 8.0)
        return routing_path_bit_cnt

    @staticmethod
    @pass_context
    def get_pdur_bsw_modules_config(context):
        output_dict = {}
        pdur_bsw_modules_config = context["pdur_json"]["PduRBswModules"]
        for key, item in pdur_bsw_modules_config.items():
            moduleNameRef = item['PduRBswModuleRef']['ValueRef'].split('/')
            if len(moduleNameRef) == 0:
                continue
            moduleName = moduleNameRef[-1]
            output_dict[moduleName] = item
        return output_dict

    @staticmethod
    @pass_context
    def get_pdur_bsw_modules_routing_config(context):
        pdur_bsw_modules_config = context["pdur_json"]["PduRBswModules"]
        cancel_recv_module_list = []
        cancel_tran_module_list = []
        LoIf_module_list = []
        LoIf_withoutIf_module_list = []
        LoIf_withIf_module_list = [] # DoIP和SoAd模块Transmit前有If字样
        LoTp_module_list = []
        LoTp_withoutTp_module_list = []
        LoTp_withTp_module_list = [] # DoIP和SoAd模块Transmit前有Tp字样
        LoTT_module_list = []
        UpIf_module_list = []
        Up_module_list = []
        UpTp_module_list = []
        UpTT_module_list = []
        TxCof_module_list = []
        TpTxCof_module_list = []
        UseTag_module_list = []
        cancel_recv_module_list, cancel_tran_module_list = get_pdur_bsw_modules_routing_config_cancel(pdur_bsw_modules_config)
        Up_module_list, LoIf_module_list, LoTp_module_list = get_pdur_bsw_modules_routing_config_lo_up_list(pdur_bsw_modules_config)
        LoIf_withIf_module_list, LoIf_withoutIf_module_list, LoTp_withTp_module_list, LoTp_withoutTp_module_list, \
            LoTT_module_list = get_pdur_bsw_modules_routing_config_lo_if_tp_list(pdur_bsw_modules_config)
        UpIf_module_list, UpTp_module_list, UpTT_module_list, TxCof_module_list, TpTxCof_module_list = \
            get_pdur_bsw_modules_routing_config_up_if_tp_list(pdur_bsw_modules_config)
        Lo_module_length = len(LoIf_withoutIf_module_list) + len(LoIf_withIf_module_list) + \
            len(LoTp_withoutTp_module_list) + len(LoTp_withTp_module_list)
        module_config_list = {
            "cancel_recv_module_list": cancel_recv_module_list,
            "cancel_tran_module_list": cancel_tran_module_list,
            'Up_module_list': Up_module_list,
            'LoIf_module_list': LoIf_module_list,
            'LoTp_module_list': LoTp_module_list,
            'LoTT_module_list': LoTT_module_list,
            'UpIf_module_list': UpIf_module_list,
            'UpTp_module_list': UpTp_module_list,
            'UpTT_module_list': UpTT_module_list,
            'TxCof_module_list': TxCof_module_list,
            'TpTxCof_module_list': TpTxCof_module_list,
            'UseTag_module_list': UseTag_module_list,
            'LoIf_withoutIf_module_list': LoIf_withoutIf_module_list,
            'LoIf_withIf_module_list': LoIf_withIf_module_list,
            'LoTp_withoutTp_module_list': LoTp_withoutTp_module_list,
            'LoTp_withTp_module_list': LoTp_withTp_module_list,
            'Lo_module_length': Lo_module_length
        }
        return module_config_list

    @staticmethod
    @pass_context
    def get_pdur_routing_path_config(context):
        return context["pdur_json"]["PduRRoutingPaths"]

    @staticmethod
    @pass_context
    def get_pdur_dest_pdu_list(context):
        pdur_dest_pdu_config = {}
        for key, item in context["pdur_json"]["PduRRoutingPaths"].items():
            for PduRDestPduName, PduRDestPdu in item['PduRDestPdus'].items():
                pdur_dest_pdu_config[PduRDestPduName] = PduRDestPdu
        return pdur_dest_pdu_config

    @staticmethod
    @pass_context
    def get_pdur_dest_max_Num(context):
        pdur_dest_Num_list = {}
        routing_path_count = 0
        pdur_dest_max_Num = 0
        pdur_json_config = context["pdur_json"]
        for path_name, path_value in pdur_json_config["PduRRoutingPaths"].items():
            pdur_dest_Num_list[routing_path_count] = len(path_value['PduRDestPdus'])
            if pdur_dest_max_Num <= len(path_value['PduRDestPdus']):
                pdur_dest_max_Num = len(path_value['PduRDestPdus'])
            routing_path_count = routing_path_count + 1
        # pdur_dest_max_Num = max(pdur_dest_Num_list)
        return pdur_dest_max_Num


    @staticmethod
    @pass_context
    def get_pdur_configuration_id(context):
        pdur_configuration_id = 0
        for key, item in context["pdur_json"].items():
            if (key == 'PduRConfigurationId'):
                pdur_configuration_id = item["Value"]
                break
        return pdur_configuration_id

    @staticmethod
    @pass_context
    def get_pdur_json_dbc_config(context):
        pdur_json_dbc_config = context["pdur_json"]["dbc"]
        return pdur_json_dbc_config

    @staticmethod
    @pass_context
    def get_pdur_json_ldf_config(context):
        pdur_json_ldf_config = context["pdur_json"]["ldf"]
        return pdur_json_ldf_config

    @staticmethod
    @pass_context
    def get_pdur_routing_groups(context):
        pdur_routing_groups = context["pdur_json"]["PduRRoutingPathGroups"]
        return pdur_routing_groups

    @staticmethod
    @pass_context
    def get_pdur_routing_group_name_by_id(context, id):
        retName = ""
        pdur_routing_groups = context["pdur_json"]["PduRRoutingPathGroups"]
        for name, group_info in pdur_routing_groups.items():
            if id == group_info['PduRRoutingPathGroupId']:
                retName = name
                break
        return retName

    @staticmethod
    @pass_context
    def get_pdur_routing_groups_dest_pdu_list(context):
        pdur_routing_groups_dest_pdu_list = context["pdur_json"]["PduRRoutingPathPduMacroList"]
        return pdur_routing_groups_dest_pdu_list

    @staticmethod
    @pass_context
    def get_pdur_tx_buffers(context):
        pdur_tx_buffers = context["pdur_json"]["PduRTxBuffers"]
        return pdur_tx_buffers

    @staticmethod
    @pass_context
    def get_pdur_evcfifo_cfg(context):
        mainfuncList = context["pdur_json"]["PduRMainFunctionList"]
        pdur_evcfifo_cfg = context["pdur_json"]["PduREvCFifos"]
        maxCoreId = 0
        fifoOutput = []
        for fifo in pdur_evcfifo_cfg:
            fifo['SpinLockName'] = 'EMPTY'
            if maxCoreId < fifo['PduREventQueueCore']:
                maxCoreId = fifo['PduREventQueueCore']
        for itr in range(maxCoreId + 1):
            for fifo in pdur_evcfifo_cfg:
                if fifo['PduREventQueueCore'] == itr:
                    fifoOutput.append(fifo)
                    break
            else:
                temp = {'PduREvCFifoLength': 0, 'PduREventQueueCore': itr, 'SpinLockName': 'EMPTY'}
                fifoOutput.append(temp)
        for item in fifoOutput:
            for mainfunc in mainfuncList:
                if item['PduREventQueueCore'] == mainfunc['coreid']:
                    item['SpinLockName'] = mainfunc['name'].upper()
            
        return fifoOutput # 每个核FIFO的长度，如果不需要布置，则为0

    @staticmethod
    @pass_context
    def get_pdur_if_evcfifo_cfg(context):
        mainfuncList = context["pdur_json"]["PduRMainFunctionList"]
        isrList = context["pdur_json"]["PduRIsr"]
        pdur_evcfifo_cfg = context["pdur_json"]["PduREvCFifosIF"]
        NumTab = {}
        tempList = []
        PduREvCFifoList_IF = []
        for IFRoutingPath in pdur_evcfifo_cfg:
            if str(IFRoutingPath['coreid']) not in NumTab.keys():
                NumTab[str(IFRoutingPath['coreid'])] = 1
            else:
                NumTab[str(IFRoutingPath['coreid'])] = NumTab[str(IFRoutingPath['coreid'])] + 1
        maxCoreId = 0
        for coreId, fifolen in NumTab.items():
            if maxCoreId < int(coreId):
                maxCoreId =  int(coreId)
            IfFifolen = fifolen // 2 + 1 # 暂时定IF类FIFO长度为需求量的1/2
            PduREvCFifo = {"PduREvCFifoLength": IfFifolen, "PduREventQueueCore": int(coreId), 'SpinLockName': 'EMPTY'}
            tempList.append(PduREvCFifo)
        for itr in range(maxCoreId+1):
            for PduREvCFifo in tempList:
                if itr == PduREvCFifo['PduREventQueueCore']:
                    PduREvCFifoList_IF.append(PduREvCFifo)
                    break
            else:
                temp = {"PduREvCFifoLength": 0, "PduREventQueueCore": itr, "SpinLockName": "EMPTY"}
                PduREvCFifoList_IF.append(temp)
        for item in PduREvCFifoList_IF:
            for isr in isrList:
                if str(item['PduREventQueueCore']) == isr['coreid']:
                    item['SpinLockName'] = isr['name'].upper()
        return PduREvCFifoList_IF # 每个核FIFO的长度，如果不需要布置，则为0


    @staticmethod
    @pass_context
    def get_src_dest_msg_name(context):
        src_dest_msg_list = {}
        src_msg_list = {}
        dest_msg_list = {}
        src_dest_count = 0
        pdur_json_config = context["pdur_json"]
        for path_name, path_value in pdur_json_config["PduRRoutingPaths"].items():
            for pdurdestpdukey, pdurdestpduvalue in path_value['PduRDestPdus'].items():
                src_msg_list[src_dest_count] = path_value['PduRoutingPathName']
                dest_msg_list[src_dest_count] = pdurdestpdukey
                src_dest_count = src_dest_count + 1
        src_dest_msg_list ={
            "src_msg_list": src_msg_list,
            "dest_msg_list": dest_msg_list,
        }
        return src_dest_msg_list

    @staticmethod
    @pass_context
    def get_srcpduid_by_destpduid(context, name: str):
        src_msg_list = {}
        dest_msg_list = {}
        src_dest_count = 0
        src_msg_mapping_list = []
        
        pdur_json_config = context["pdur_json"]
        for path_name, path_value in pdur_json_config["PduRRoutingPaths"].items():
            for pdurdestpdukey, pdurdestpduvalue in path_value['PduRDestPdus'].items():
                src_msg_list[src_dest_count] = path_value['PduRoutingPathName']
                dest_msg_list[src_dest_count] = pdurdestpdukey
                src_dest_count = src_dest_count + 1

        for src_msg in range(src_dest_count):
            if dest_msg_list[src_msg] == name and src_msg_list[src_msg] not in src_msg_mapping_list:
                src_msg_mapping_list.append(src_msg_list[src_msg])

        return src_msg_mapping_list

    @staticmethod
    @pass_context
    def get_msg_src_pduid_config(context):
        src_pduid_list = []
        pdur_json_config = context["pdur_json"]
        for path_name, path_value in pdur_json_config["PduRRoutingPaths"].items():
            src_pduid_list.append(path_value['PduRoutingPathName'])
        return src_pduid_list

    @staticmethod
    @pass_context
    def get_msg_dest_pduid_no_duplicate_config(context):
        dest_pduid_no_duplicate_list = []
        pdur_json_config = context["pdur_json"]
        for path_name, path_value in pdur_json_config["PduRRoutingPaths"].items():
            for destPduId, value in path_value['PduRDestPdus'].items():
                dest_pduid_no_duplicate_list.append(destPduId)
        dest_pduid_no_duplicate_list = list(dict.fromkeys(dest_pduid_no_duplicate_list))
        return dest_pduid_no_duplicate_list
    
    @staticmethod
    @pass_context
    def get_pdur_bufferpool(context):
        return context["pdur_json"]['PduRBufferPools']

    @staticmethod
    @pass_context
    def get_pdur_txbuffer(context):
        return context["pdur_json"]['PduRTxBuffers']

    @staticmethod
    @pass_context
    def get_pdur_bufferpool_and_buffer(context):
        pdur_buffer_pool_list = Function.get_pdur_bufferpool(context)

    @staticmethod
    @pass_context
    def get_pdur_routing_path_group_idx_list(context):
        group_list = context["pdur_json"]['PduRRoutingPathGroupList']
        return group_list

    @staticmethod
    @pass_context
    def get_pdur_routing_path_group_idx_list_empty(context):
        group_list = context["pdur_json"]['PduRRoutingPathGroupList']
        if len(group_list) == 0:
            return 'True'
        else:
            return 'False'

    @staticmethod
    @pass_context
    def to_hex(self, input):
        return hex(input)
    @staticmethod
    @pass_context
    def get_interface_file_config(context):
        return context["moduleConfig"]

def get_pdur_bsw_modules_routing_config_cancel(pdur_bsw_modules_config):
    cancel_recv_module_list = []
    cancel_tran_module_list = []
    for _, bsw_module in pdur_bsw_modules_config.items():
        bsw_module_name = bsw_module['PduRBswModuleRef']['ValueRef'].split('/')[-1]
        if 'PduRLowerModule' in bsw_module and bsw_module['PduRLowerModule']['Value'] is True:
            if 'PduRCancelReceive' in bsw_module and bsw_module['PduRCancelReceive']['Value'] is True:
                cancel_recv_module_list.append(bsw_module_name)
            if 'PduRCancelTransmit' in bsw_module and bsw_module['PduRCancelTransmit']['Value'] is True:
                cancel_tran_module_list.append(bsw_module_name)
    return cancel_recv_module_list, cancel_tran_module_list

def get_pdur_bsw_modules_routing_config_lo_up_list(pdur_bsw_modules_config):
    Up_module_list = []
    LoIf_module_list = []
    LoTp_module_list = []
    for _, bsw_module in pdur_bsw_modules_config.items():
        bsw_module_name = bsw_module['PduRBswModuleRef']['ValueRef'].split('/')[-1]
        if 'PduRLowerModule' in bsw_module and bsw_module['PduRLowerModule']['Value'] is True:
            if 'PduRCommunicationInterface' in bsw_module and bsw_module['PduRCommunicationInterface']['Value'] is True:
                LoIf_module_list.append(bsw_module_name)
            if 'PduRTransportProtocol' in bsw_module and bsw_module['PduRTransportProtocol']['Value'] is True:
                LoTp_module_list.append(bsw_module_name)
        if 'PduRUpperModule' in bsw_module and bsw_module['PduRUpperModule']['Value'] is True:
            Up_module_list.append(bsw_module_name)
    return Up_module_list, LoIf_module_list, LoTp_module_list

def get_pdur_bsw_modules_routing_config_lo_if_tp_list(pdur_bsw_modules_config):
    LoIf_withoutIf_module_list = []
    LoIf_withIf_module_list = [] # DoIP和SoAd模块Transmit前有If字样
    LoTT_module_list = []
    LoTp_withoutTp_module_list = []
    LoTp_withTp_module_list = [] # DoIP和SoAd模块Transmit前有Tp字样
    for _, bsw_module in pdur_bsw_modules_config.items():
        bsw_module_name = bsw_module['PduRBswModuleRef']['ValueRef'].split('/')[-1]
        # RxIndication/TpRxIndication
        if 'PduRLowerModule' in bsw_module and bsw_module['PduRLowerModule']['Value'] is True:
            # RxIndication
            if 'PduRCommunicationInterface' in bsw_module and bsw_module['PduRCommunicationInterface']['Value'] is True:
                if bsw_module_name == "DoIP" or bsw_module_name == "SoAd":
                    temp_name = bsw_module_name + "_If"
                    LoIf_withIf_module_list.append(temp_name)
                else:
                    LoIf_withoutIf_module_list.append(bsw_module_name)
            # TpRxIndication
            if 'PduRTransportProtocol' in bsw_module and bsw_module['PduRTransportProtocol']['Value'] is True:
                if bsw_module_name == "DoIP" or bsw_module_name == "SoAd":
                    temp_name = bsw_module_name + "_Tp"
                    LoTp_withTp_module_list.append(temp_name)
                else:
                    LoTp_withoutTp_module_list.append(bsw_module_name)
            # TriggerTransmit
            if 'PduRTriggertransmit' in bsw_module and bsw_module['PduRTriggertransmit']['Value'] is True:
                LoTT_module_list.append(bsw_module_name)
    return LoIf_withIf_module_list, LoIf_withoutIf_module_list, LoTp_withTp_module_list, LoTp_withoutTp_module_list, \
        LoTT_module_list

def get_pdur_bsw_modules_routing_config_up_if_tp_list(pdur_bsw_modules_config):
    UpIf_module_list = []
    UpTp_module_list = []
    UpTT_module_list = []
    TxCof_module_list = []
    TpTxCof_module_list = []
    for _, bsw_module in pdur_bsw_modules_config.items():
        bsw_module_name = bsw_module['PduRBswModuleRef']['ValueRef'].split('/')[-1]
        if 'PduRUpperModule' in bsw_module and bsw_module['PduRUpperModule']['Value'] is True:
            # Transmit
            if 'PduRCommunicationInterface' in bsw_module and bsw_module['PduRCommunicationInterface']['Value'] is True:
                UpIf_module_list.append(bsw_module_name)
                if 'PduRTxConfirmation' in bsw_module and bsw_module['PduRTxConfirmation']['Value'] is True:
                    TxCof_module_list.append(bsw_module_name)
            # CopyRxData/CopyTxData/StartOfReception
            if 'PduRTransportProtocol' in bsw_module and bsw_module['PduRTransportProtocol']['Value'] is True:
                UpTp_module_list.append(bsw_module_name)
                if 'PduRTxConfirmation' in bsw_module and bsw_module['PduRTxConfirmation']['Value'] is True:
                    TpTxCof_module_list.append(bsw_module_name)
            # TriggerTransmit
            if 'PduRTriggertransmit' in bsw_module and bsw_module['PduRTriggertransmit']['Value'] is True:
                UpTT_module_list.append(bsw_module_name)
    return UpIf_module_list, UpTp_module_list, UpTT_module_list, TxCof_module_list, TpTxCof_module_list