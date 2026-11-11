#
# Copyright (C) 2015-2022 Lixiang Group Holding Limited
#
"""
整理TcpIp模块配置信息模块
"""

class TcpIpInfo:
    def __init__(self, name, config, rte_config):
        self.name = name
        self.initConfig = config
        self.rteConfig = rte_config
        self.outContainer = {}
        self.multiContainer = [
            'TcpIpCtrl',
            'TcpIpLocalAddr',
            'TcpIpArpConfig',
            'TcpIpAddrAssignment',
            'TcpIpSocketOwner',
            'TcpIpTcpConfigOptionFilter',
            'TcpIpStaticArpEntry',
            'TcpIpBufferMemoryConfig'
        ]
        self.SpecialContainer = [
            'TcpIpIpVXCtrl',
            'TcpIpSocketOwnerConfig',
        ]

    def tcpip_get_dict_by_definition_ref(self, input_dict: dict, definition_ref: str) -> dict:
        for value in input_dict.values():
            if isinstance(value, dict):
                if "DefinitionRef" in value.keys():
                    if value["DefinitionRef"].split("/")[-1] == definition_ref:
                        return value
        return {}

    def get_tcpip_mainfunction_config(self, name: str) -> dict:
        mainfunctionitem = {
            "name": name,
            "coreid": 0,
        }
        if "task_mappings_info" not in self.rteConfig.keys():
            pass
        else:
            for task_name in list(self.rteConfig["task_mappings_info"].keys()):
                if "schedulable_entity_info_list" not in self.rteConfig["task_mappings_info"][task_name].keys():
                    continue
                if isinstance(self.rteConfig["task_mappings_info"][task_name]["schedulable_entity_info_list"], list) == False:
                    continue
                for i in range(len(self.rteConfig["task_mappings_info"][task_name]["schedulable_entity_info_list"])):
                    if self.rteConfig["task_mappings_info"][task_name]["schedulable_entity_info_list"][i]["name"] == name:
                        MainfuncCoreId = int(self.rteConfig["task_mappings_info"][task_name]["core_id"])
                        mainfunctionitem['coreid'] = MainfuncCoreId
        return mainfunctionitem

    def get_tcpip_coreid_list(self, mainfunclist: list) -> dict:
        coreList = []
        maxCoreId = 0
        for items in mainfunclist:
            coreList.append(items['coreid'])
            if items['coreid'] > maxCoreId:
                maxCoreId = items['coreid']
        coreList = sorted(list(set(coreList)))
        ret = {
            "list": coreList,
            "max": maxCoreId
        }
        return ret     

    def get_template_dynamic_code_dict(self):
        self.outContainer = self.tranverse_dict(self.initConfig)
        buffer_sorted_config_list = sorted(self.outContainer['TcpIpConfig']['TcpIpBufferMemoryConfig'], key=lambda x: int(x["TcpIpBufferMemoryLen"]["Value"]))
        self.outContainer['TcpIpConfig']['TcpIpBufferMemoryConfig'] = buffer_sorted_config_list
        tcpip_mainfunc_list = []
        mainfunc = self.get_tcpip_mainfunction_config("tcpip_main_function")
        tcpip_mainfunc_list.append(mainfunc)
        self.outContainer["TcpIpCoreidList"] = self.get_tcpip_coreid_list(tcpip_mainfunc_list)
        return self.outContainer

    def tranverse_dict(self, config):
        tmp = {}
        for key, value in config.items():
            if self.iscontainer(value):
                container_name = self.get_def_name(value)
                if container_name in self.multiContainer:
                    if container_name not in tmp:
                        tmp[container_name] = []
                    new = {"ShortName": key}
                    v = self.tranverse_dict(value)
                    new.update(v)
                    tmp[container_name].append(new)
                else:
                    tmp[container_name] = self.tranverse_dict(value)
            else:
                # print(key, ": ", value)
                tmp[key] = value
        return tmp
    def get_def_name(self, v):
        if not isinstance(v, dict):
            # print('not dict', v)
            return 'NULL'
        elif 'DefinitionRef' not in v:
            print('no DefinitionRef', v)
            return 'NULL'
        else:
            return v['DefinitionRef'].split('/')[-1]

    def iscontainer(self, v):
        if (self.get_def_name(v) in self.SpecialContainer):
            return True
        if isinstance(v, dict):
            for key, value in v.items():
                if (key != "DefinitionRef") and (key != "ValueRef") and (key != "Value") and (key != "reference_item_path"):
                    return True
        return False

def set_default_ref_value(d, def_ref_name, config_name):
    if (def_ref_name not in d) or ('ValueRef' not in d[def_ref_name]) or\
        (d[def_ref_name]['ValueRef'] == '<empty>') or (d[def_ref_name]['ValueRef'] == None):
        d[def_ref_name] = {'ValueRef' : '-1'}
    else:
        value = d[def_ref_name]['ValueRef']
        d[def_ref_name] = {'ValueRef' : config_name + value.split('/')[-1]}

def set_default_value(d, def_ref_name, default_value):
    if (def_ref_name not in d) or ('Value' not in d[def_ref_name]) or\
        (d[def_ref_name]['Value'] == 'null') or (d[def_ref_name]['Value'] == 'NULL') or\
        (not d[def_ref_name]['Value']) or (d[def_ref_name]['Value'] == None):
        d[def_ref_name] = {'Value' : default_value}

def set_TcpIpSocketOwner_default(config):
    TcpIpSocketOwner = config['TcpIpConfig']['TcpIpSocketOwnerConfig']['TcpIpSocketOwner']
    definition = [
        'TcpIpSocketOwnerCopyTxDataName',
        'TcpIpSocketOwnerHeaderFileName',
        'TcpIpSocketOwnerLocalIpAddrAssignmentChgName',
        'TcpIpSocketOwnerRxIndicationName',
        'TcpIpSocketOwnerTcpAcceptedName',
        'TcpIpSocketOwnerTcpConnectedName',
        'TcpIpSocketOwnerTcpIpEventName',
        'TcpIpSocketOwnerTxConfirmationName',
        'TcpIpSocketOwnerUpperLayerType',
    ]
    for v in TcpIpSocketOwner:
        for d in definition:
            set_default_value(v, d, 'NULL_PTR')
        socket_owner_name = 'TCPIP_SOCKETOWNER_' + v['ShortName'].upper()
        get_socket_name = v['ShortName']
        v['ShortName'] = socket_owner_name
        v['GetSocket'] = get_socket_name

def set_TcpIpCtrl_default(config):
    TcpIpCtrl = config['TcpIpConfig']['TcpIpCtrl']
    for v in TcpIpCtrl:
        if ('TcpIpIpFramePrioDefault' not in v):
            v['TcpIpIpFramePrioDefault'] = {'Value': 0}
        set_default_ref_value(v, 'TcpIpEthIfCtrlRef', 'EthIfConf_EthIfController_')
        if ('TcpIpIpVXCtrl' not in v):
            v['TcpIpIpVXCtrl'] = {'TcpIpIpV4Ctrl' : {'TcpIpArpConfigRef' : {'ValueRef' : 0}}}

def set_TcpIpBufferMemory_default(config):
    BufferMemory = config['TcpIpConfig']['TcpIpBufferMemoryConfig']
    for v in BufferMemory:
        set_default_value(v,'TcpIpBufferMemoryLen',0)
        set_default_value(v,'TcpIpBufferMemoryNum',0)

def set_TcpIpLocalAddr_default(config):
    TcpIpLocalAddr = config['TcpIpConfig']['TcpIpLocalAddr']
    AddrId = []
    for v in TcpIpLocalAddr:
        if v['TcpIpAddrId']['Value'] != None:
            AddrId.append(v['TcpIpAddrId']['Value'])

    for v in TcpIpLocalAddr:
        set_default_ref_value(v, 'TcpIpCtrlRef', 'TcpIpConf_TcpIpCtrl_')
        if 'TcpIpStaticIpAddressConfig' in v:
            d = v['TcpIpStaticIpAddressConfig']
            set_default_value(d, 'TcpIpNetmask', 0)
            set_default_value(d, 'TcpIpDefaultRouter', '0.0.0.0')
            set_default_value(d, 'TcpIpStaticIpAddress', '0.0.0.0')
        if 'TcpIpAddrAssignment' in v:
            d = v['TcpIpAddrAssignment']
            for addr in d:
                set_default_value(addr, 'TcpIpAssignmentLifetime', 'TCPIP_FORGET')
                set_default_value(addr, 'TcpIpAssignmentMethod', 'TCPIP_STATIC')
                set_default_value(addr, 'TcpIpAssignmentTrigger', 'TCPIP_MANUAL')
                set_default_value(addr, 'TcpIpAssignmentPriority', 3)
        if v['TcpIpAddrId']['Value'] == None:
            i = 0
            while (i in AddrId) and (i < 65535):
                i = i + 1
            AddrId.append(int(i))
            v['TcpIpAddrId']['Value'] = int(i)


def set_TcpIpIpConfigs_default(config):
    if 'TcpIpIpConfig' not in config['TcpIpConfig']:
        config['TcpIpConfig']['TcpIpIpConfig'] = {}
    TcpIpIpConfig = config['TcpIpConfig']['TcpIpIpConfig']

    if 'TcpIpIpV4Config' not in TcpIpIpConfig:
        TcpIpIpConfig['TcpIpIpV4Config'] = {}
    TcpIpIpV4Config = TcpIpIpConfig['TcpIpIpV4Config']

    if 'TcpIpArpConfig' not in TcpIpIpV4Config:
        TcpIpIpV4Config['TcpIpArpConfig'] = []
    TcpIpArpConfig = TcpIpIpV4Config['TcpIpArpConfig']

    for v in TcpIpArpConfig:
        if 'TcpIpStaticArpEntry' not in v:
            v['TcpIpStaticArpEntry'] = []
        for entry in v['TcpIpStaticArpEntry']:
            set_default_value(entry, 'TcpIpStaticArpEntryIpAddr', '0.0.0.0')
            set_default_value(entry, 'TcpIpStaticArpEntryPhysAddr', '00-00-00-00-00-00')
            addr = entry['TcpIpStaticArpEntryIpAddr']['Value']
            ip = addr.split('.')
            if len(ip) != 4:
                print('wrong ip addr', addr)
                while(len(ip) < 4):
                    ip.append('0')
            # print(ip)
            entry['TcpIpStaticArpEntryIpAddr']['Value'] = ip[0] + ',' + ip[1] + ',' + ip[2] + ',' + ip[3]

            addr = entry['TcpIpStaticArpEntryPhysAddr']['Value']
            mac = addr.split('-')
            if len(mac) != 6:
                print('wrong ip addr', addr)
                while(len(mac) < 6):
                    mac.append('00')
            # print(mac)
            entry['TcpIpStaticArpEntryPhysAddr']['Value'] = mac

def set_TcpIpGeneral(config):
    TcpIpGeneral = config['TcpIpGeneral']
    # enable_configs = [
    #     'TcpIpDevErrorDetect',
    #     'TcpIpResetIpAssignmentApi',
    #     'TcpIpTcpEnabled',
    #     'TcpIpUdpEnabled',
    #     'TcpIpVersionInfoApi',
    #     'TcpIpGetAndResetMeasurementDataApi'
    # ]
    # for definition in enable_configs:
    #     if (definition in TcpIpGeneral) and (TcpIpGeneral[definition]['Value'] == True):
    #         TcpIpGeneral[definition]= {'Value' : 'STD_ON'}
    #     else:
    #         TcpIpGeneral[definition]= {'Value' : 'STD_OFF'}
    main_function = TcpIpGeneral['TcpIpMainFunctionPeriod']
    main_function['Value'] = int(main_function['Value'] * 1000)

def set_TcpIpTcpConfig(config):
    if 'TcpIpTcpConfig' in config['TcpIpConfig']:
        set_default_value(config['TcpIpConfig']['TcpIpTcpConfig'], 'TcpIpTcpSynReceivedTimeout', 0)
        set_default_value(config['TcpIpConfig']['TcpIpTcpConfig'], 'TcpIpTcpSynMaxRtx', 0)
        set_default_value(config['TcpIpConfig']['TcpIpTcpConfig'], 'TcpIpTcpSendBufferMax', 2)
        # config['TcpIpConfig']['TcpIpTcpConfig']['TcpIpTcpSynReceivedTimeout'] = {'Value' : 0}
        # config['TcpIpConfig']['TcpIpTcpConfig']['TcpIpTcpSynMaxRtx'] = {'Value' : 0}
        if config['TcpIpConfig']['TcpIpTcpConfig']['TcpIpTcpRetransmissionTimeout']['Value'] == None:
            config['TcpIpConfig']['TcpIpTcpConfig'].pop('TcpIpTcpRetransmissionTimeout')
        if config['TcpIpConfig']['TcpIpTcpConfig']['TcpIpTcpMaxRtx']['Value'] == None:
            config['TcpIpConfig']['TcpIpTcpConfig'].pop('TcpIpTcpMaxRtx')

def set_tcpip_default_value(config):
    set_TcpIpCtrl_default(config)
    set_TcpIpLocalAddr_default(config)
    set_TcpIpSocketOwner_default(config)
    set_TcpIpIpConfigs_default(config)
    set_TcpIpGeneral(config)
    set_TcpIpTcpConfig(config)
    set_TcpIpBufferMemory_default(config)